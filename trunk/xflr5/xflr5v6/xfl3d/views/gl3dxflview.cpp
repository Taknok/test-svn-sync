/****************************************************************************

    gl3dXflView Class
    Copyright (C) André Deperrois

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*****************************************************************************/

#include <QOpenGLFunctions>
#include <QDir>

#include "gl3dxflview.h"

#include <xfl3d/controls/w3dprefs.h>
#include <xflanalysis/plane_analysis/lltanalysis.h>
#include <xflcore/displayoptions.h>
#include <xflcore/units.h>
#include <xflcore/xflcore.h>
#include <xflgeom/geom3d/vector3d.h>
#include <xflobjects/objects3d/body.h>
#include <xflobjects/objects3d/plane.h>
#include <xflobjects/objects3d/pointmass.h>
#include <xflobjects/objects3d/surface.h>
#include <xflobjects/objects3d/wing.h>
#include <xflobjects/objects3d/wpolar.h>

Miarex *gl3dXflView::s_pMiarex(nullptr);
MainFrame *gl3dXflView::s_pMainFrame(nullptr);

gl3dXflView::gl3dXflView(QWidget *pParent) : gl3dView(pParent)
{
    m_WingMeshIndicesArray = nullptr;

    m_pLeftBodyTexture = m_pRightBodyTexture= nullptr;
    for(int iw=0; iw<MAXWINGS; iw++)
    {
        m_pWingTopLeftTexture[iw] = m_pWingTopRightTexture[iw] = nullptr;
        m_pWingBotLeftTexture[iw] = m_pWingBotRightTexture[iw] = nullptr;
    }

    m_bOutline    = true;
    m_bSurfaces   = true;
    m_bVLMPanels  = false;
    m_bAxes       = true;
    m_bShowMasses = false;
    m_bFoilNames  = false;

    for(int iw=0; iw<MAXWINGS; iw++) m_iWingElems[iw]=0;
    m_iWingMeshElems = 0;


    m_nHighlightLines = m_HighlightLineSize = 0;
    memset(m_Ny, 0, sizeof(m_Ny));
}


gl3dXflView::~gl3dXflView()
{
    for(int iWing=0; iWing<MAXWINGS; iWing++)
    {
        m_vboEditWingMesh[iWing].destroy();
    }

    if(m_WingMeshIndicesArray) delete[] m_WingMeshIndicesArray;


    m_vboBody.destroy();
    m_vboEditBodyMesh.destroy();

    for(int iWing=0; iWing<MAXWINGS; iWing++)
    {
        m_vboWingSurface[iWing].destroy();
        m_vboWingOutline[iWing].destroy();
    }

    makeCurrent();
    if(m_pLeftBodyTexture)     delete m_pLeftBodyTexture;
    if(m_pRightBodyTexture)    delete m_pRightBodyTexture;

    for(int iw=0; iw<MAXWINGS; iw++)
    {
        if(m_pWingBotLeftTexture[iw])  delete m_pWingBotLeftTexture[iw];
        if(m_pWingTopLeftTexture[iw])  delete m_pWingTopLeftTexture[iw];
        if(m_pWingBotRightTexture[iw]) delete m_pWingBotRightTexture[iw];
        if(m_pWingTopRightTexture[iw]) delete m_pWingTopRightTexture[iw];
        m_pWingBotLeftTexture[iw] = nullptr;
        m_pWingTopLeftTexture[iw] = nullptr;
        m_pWingBotRightTexture[iw] = nullptr;
        m_pWingTopRightTexture[iw] = nullptr;
    }
    doneCurrent();
}


void gl3dXflView::onSurfaces(bool bChecked)
{
    m_bSurfaces = bChecked;
    update();
}


void gl3dXflView::onOutline(bool bChecked)
{
    m_bOutline = bChecked;
    update();
}


void gl3dXflView::onPanels(bool bChecked)
{
    m_bVLMPanels = bChecked;
    update();
}


void gl3dXflView::onFoilNames(bool bChecked)
{
    m_bFoilNames = bChecked;
    update();
}


void gl3dXflView::onShowMasses(bool bChecked)
{
    m_bShowMasses = bChecked;
    update();
}


void gl3dXflView::glMakeFuseFlatPanels(Body const*pBody)
{
    Vector3d P1, P2, P3, P4, N, P1P3, P2P4, Tj, Tjp1;

    if(m_pLeftBodyTexture)  delete m_pLeftBodyTexture;
    if(m_pRightBodyTexture) delete m_pRightBodyTexture;

    QImage leftTexture  = QImage(QString(m_TexturePath+QDir::separator()+"body_left.png"));
    if(leftTexture.isNull()) leftTexture = QImage(QString(":/resources/default_textures/body_left.png"));
    m_pLeftBodyTexture  = new QOpenGLTexture(leftTexture);
    QImage rightTexture  = QImage(QString(m_TexturePath+QDir::separator()+"body_right.png"));
    if(rightTexture.isNull()) rightTexture = QImage(QString(":/resources/default_textures/body_right.png"));
    m_pRightBodyTexture  = new QOpenGLTexture(rightTexture);


    int buffersize = (pBody->sideLineCount()-1) * (pBody->frameCount()-1); //quads
    buffersize *= 2;    //two triangles per quad
    buffersize *= 3;    //three vertices per triangle
    buffersize *= 8;    // 3 position + 3 normal + 2 UV components

    QVector<float>pBodyVertexArray(buffersize);

    int iv=0;

    float fnh = pBody->sideLineCount();
    float fLength = float(pBody->length());

    float tip = 0.0;
    if(pBody->frameCount()) tip = pBody->frameAt(0)->position().xf();

    //surfaces
    for (int k=0; k<pBody->sideLineCount()-1;k++)
    {
        for (int j=0; j<pBody->frameCount()-1;j++)
        {
            Tj.set(pBody->frameAt(j)->position().x,     0.0, 0.0);
            Tjp1.set(pBody->frameAt(j+1)->position().x, 0.0, 0.0);

            P1 = pBody->frameAt(j  )->ctrlPointAt(k);       P1.x = pBody->frameAt(j  )->position().x;
            P2 = pBody->frameAt(j+1)->ctrlPointAt(k);       P2.x = pBody->frameAt(j+1)->position().x;
            P3 = pBody->frameAt(j+1)->ctrlPointAt(k+1);     P3.x = pBody->frameAt(j+1)->position().x;
            P4 = pBody->frameAt(j  )->ctrlPointAt(k+1);     P4.x = pBody->frameAt(j  )->position().x;

            P1P3 = P3-P1;
            P2P4 = P4-P2;
            N = (P1P3 * P2P4).normalized(); // flat shading

            // first triangle
            pBodyVertexArray[iv++] = P1.xf();
            pBodyVertexArray[iv++] = P1.yf();
            pBodyVertexArray[iv++] = P1.zf();
            pBodyVertexArray[iv++] = N.xf();
            pBodyVertexArray[iv++] = N.yf();
            pBodyVertexArray[iv++] = N.zf();
            pBodyVertexArray[iv++] = 1.0f-(P1.xf()-tip)/fLength;
            pBodyVertexArray[iv++] = float(k)/fnh;

            pBodyVertexArray[iv++] = P2.xf();
            pBodyVertexArray[iv++] = P2.yf();
            pBodyVertexArray[iv++] = P2.zf();
            pBodyVertexArray[iv++] = N.xf();
            pBodyVertexArray[iv++] = N.yf();
            pBodyVertexArray[iv++] = N.zf();
            pBodyVertexArray[iv++] = 1.0f-(P2.xf()-tip)/fLength;
            pBodyVertexArray[iv++] = float(k)/fnh;

            pBodyVertexArray[iv++] = P3.xf();
            pBodyVertexArray[iv++] = P3.yf();
            pBodyVertexArray[iv++] = P3.zf();
            pBodyVertexArray[iv++] = N.xf();
            pBodyVertexArray[iv++] = N.yf();
            pBodyVertexArray[iv++] = N.zf();
            pBodyVertexArray[iv++] = 1.0f-(P3.xf()-tip)/fLength;
            pBodyVertexArray[iv++] = float(k+1)/fnh;

            //second triangle
            pBodyVertexArray[iv++] = P1.xf();
            pBodyVertexArray[iv++] = P1.yf();
            pBodyVertexArray[iv++] = P1.zf();
            pBodyVertexArray[iv++] = N.xf();
            pBodyVertexArray[iv++] = N.yf();
            pBodyVertexArray[iv++] = N.zf();
            pBodyVertexArray[iv++] = 1.0f-(P1.xf()-tip)/fLength;
            pBodyVertexArray[iv++] = float(k)/fnh;

            pBodyVertexArray[iv++] = P3.xf();
            pBodyVertexArray[iv++] = P3.yf();
            pBodyVertexArray[iv++] = P3.zf();
            pBodyVertexArray[iv++] = N.xf();
            pBodyVertexArray[iv++] = N.yf();
            pBodyVertexArray[iv++] = N.zf();
            pBodyVertexArray[iv++] = 1.0f-(P3.xf()-tip)/fLength;
            pBodyVertexArray[iv++] = float(k+1)/fnh;

            pBodyVertexArray[iv++] = P4.xf();
            pBodyVertexArray[iv++] = P4.yf();
            pBodyVertexArray[iv++] = P4.zf();
            pBodyVertexArray[iv++] = N.xf();
            pBodyVertexArray[iv++] = N.yf();
            pBodyVertexArray[iv++] = N.zf();
            pBodyVertexArray[iv++] = 1.0f-(P4.xf()-tip)/fLength;
            pBodyVertexArray[iv++] = float(k+1)/fnh;
        }
    }
    Q_ASSERT(iv==buffersize);

    m_vboFuseRight.destroy();
    m_vboFuseRight.create();
    m_vboFuseRight.bind();
    m_vboFuseRight.allocate(pBodyVertexArray.data(), buffersize * int(sizeof(GLfloat)));
    m_vboFuseRight.release();

    // left side
    iv=0;
    for (int k=0; k<pBody->sideLineCount()-1;k++)
    {
        for (int j=0; j<pBody->frameCount()-1;j++)
        {
            Tj.set(pBody->frameAt(j)->position().x,     0.0, 0.0);
            Tjp1.set(pBody->frameAt(j+1)->position().x, 0.0, 0.0);

            P1 = pBody->frameAt(j  )->ctrlPointAt(k);       P1.x = pBody->frameAt(j  )->position().x;
            P2 = pBody->frameAt(j+1)->ctrlPointAt(k);       P2.x = pBody->frameAt(j+1)->position().x;
            P3 = pBody->frameAt(j+1)->ctrlPointAt(k+1);     P3.x = pBody->frameAt(j+1)->position().x;
            P4 = pBody->frameAt(j  )->ctrlPointAt(k+1);     P4.x = pBody->frameAt(j  )->position().x;

            P1P3 = P3-P1;
            P2P4 = P4-P2;
            N = P1P3 * P2P4;
            N.normalize();

            P1.y = -P1.y;
            P2.y = -P2.y;
            P3.y = -P3.y;
            P4.y = -P4.y;
            N.y = -N.y;

            //first triangle
            pBodyVertexArray[iv++] = P1.xf();
            pBodyVertexArray[iv++] = P1.yf();
            pBodyVertexArray[iv++] = P1.zf();
            pBodyVertexArray[iv++] = N.xf();
            pBodyVertexArray[iv++] = N.yf();
            pBodyVertexArray[iv++] = N.zf();
            pBodyVertexArray[iv++] = (P1.xf()-tip)/fLength;
            pBodyVertexArray[iv++] = float(k)/fnh;

            pBodyVertexArray[iv++] = P3.xf();
            pBodyVertexArray[iv++] = P3.yf();
            pBodyVertexArray[iv++] = P3.zf();
            pBodyVertexArray[iv++] = N.xf();
            pBodyVertexArray[iv++] = N.yf();
            pBodyVertexArray[iv++] = N.zf();
            pBodyVertexArray[iv++] = (P3.xf()-tip)/fLength;
            pBodyVertexArray[iv++] = float(k+1)/fnh;

            pBodyVertexArray[iv++] = P2.xf();
            pBodyVertexArray[iv++] = P2.yf();
            pBodyVertexArray[iv++] = P2.zf();
            pBodyVertexArray[iv++] = N.xf();
            pBodyVertexArray[iv++] = N.yf();
            pBodyVertexArray[iv++] = N.zf();
            pBodyVertexArray[iv++] = (P2.xf()-tip)/fLength;
            pBodyVertexArray[iv++] = float(k)/fnh;

            //second triangle
            pBodyVertexArray[iv++] = P1.xf();
            pBodyVertexArray[iv++] = P1.yf();
            pBodyVertexArray[iv++] = P1.zf();
            pBodyVertexArray[iv++] = N.xf();
            pBodyVertexArray[iv++] = N.yf();
            pBodyVertexArray[iv++] = N.zf();
            pBodyVertexArray[iv++] = (P1.xf()-tip)/fLength;
            pBodyVertexArray[iv++] = float(k)/fnh;

            pBodyVertexArray[iv++] = P4.xf();
            pBodyVertexArray[iv++] = P4.yf();
            pBodyVertexArray[iv++] = P4.zf();
            pBodyVertexArray[iv++] = N.xf();
            pBodyVertexArray[iv++] = N.yf();
            pBodyVertexArray[iv++] = N.zf();
            pBodyVertexArray[iv++] = (P4.xf()-tip)/fLength;
            pBodyVertexArray[iv++] = float(k+1)/fnh;

            pBodyVertexArray[iv++] = P3.xf();
            pBodyVertexArray[iv++] = P3.yf();
            pBodyVertexArray[iv++] = P3.zf();
            pBodyVertexArray[iv++] = N.xf();
            pBodyVertexArray[iv++] = N.yf();
            pBodyVertexArray[iv++] = N.zf();
            pBodyVertexArray[iv++] = (P3.xf()-tip)/fLength;
            pBodyVertexArray[iv++] = float(k+1)/fnh;

        }
    }
    Q_ASSERT(iv==buffersize);

    m_vboFuseLeft.destroy();
    m_vboFuseLeft.create();
    m_vboFuseLeft.bind();
    m_vboFuseLeft.allocate(pBodyVertexArray.data(), buffersize * int(sizeof(GLfloat)));
    m_vboFuseLeft.release();
}



void gl3dXflView::glMakeFuseFlatPanelsOutline(const Body *pBody)
{
    Vector3d P1, P2;

    int buffersize = 0;
    buffersize += pBody->frameCount()             //
                 * (pBody->sideLineCount()-1) *2  // number of segments/frame
                 * 2                              // 2 vertices
                 * 3;                             // 3 components


    buffersize += pBody->sideLineCount()          //
                 * (pBody->frameCount()-1) *2     // number of segments/frame
                 * 2                              // 2 vertices
                 * 3;                             // 3 components

    QVector<float>OutlineVertexArray(buffersize);

    int iv=0;
    for (int j=0; j<pBody->frameCount();j++)
    {
        for (int k=0; k<pBody->sideLineCount()-1; k++)
        {
            P1 = pBody->frameAt(j)->ctrlPointAt(k);       P1.x = pBody->frameAt(j)->position().x;
            P2 = pBody->frameAt(j)->ctrlPointAt(k+1);     P2.x = pBody->frameAt(j)->position().x;

            // right side segment
            OutlineVertexArray[iv++] = P1.xf();
            OutlineVertexArray[iv++] = P1.yf();
            OutlineVertexArray[iv++] = P1.zf();

            OutlineVertexArray[iv++] = P2.xf();
            OutlineVertexArray[iv++] = P2.yf();
            OutlineVertexArray[iv++] = P2.zf();

            // left side segment
            OutlineVertexArray[iv++] =  P1.xf();
            OutlineVertexArray[iv++] = -P1.yf();
            OutlineVertexArray[iv++] =  P1.zf();

            OutlineVertexArray[iv++] =  P2.xf();
            OutlineVertexArray[iv++] = -P2.yf();
            OutlineVertexArray[iv++] =  P2.zf();
        }
    }

    for (int k=0; k<pBody->sideLineCount(); k++)
    {
        for (int j=0; j<pBody->frameCount()-1; j++)
        {
            P1 = pBody->frameAt(j  )->ctrlPointAt(k);       P1.x = pBody->frameAt(j  )->position().x;
            P2 = pBody->frameAt(j+1)->ctrlPointAt(k);       P2.x = pBody->frameAt(j+1)->position().x;

            // right side segment
            OutlineVertexArray[iv++] = P1.xf();
            OutlineVertexArray[iv++] = P1.yf();
            OutlineVertexArray[iv++] = P1.zf();

            OutlineVertexArray[iv++] = P2.xf();
            OutlineVertexArray[iv++] = P2.yf();
            OutlineVertexArray[iv++] = P2.zf();

            // left side segment
            OutlineVertexArray[iv++] =  P1.xf();
            OutlineVertexArray[iv++] = -P1.yf();
            OutlineVertexArray[iv++] =  P1.zf();

            OutlineVertexArray[iv++] =  P2.xf();
            OutlineVertexArray[iv++] = -P2.yf();
            OutlineVertexArray[iv++] =  P2.zf();
        }
    }

    Q_ASSERT(iv==buffersize);

    m_vboFuseOutline.destroy();
    m_vboFuseOutline.create();
    m_vboFuseOutline.bind();
    m_vboFuseOutline.allocate(OutlineVertexArray.data(), buffersize * sizeof(GLfloat));
    m_vboFuseOutline.release();
}


void gl3dXflView::glMakeFuseSplines(Body const *pBody)
{
    int NXXXX = W3dPrefs::bodyAxialRes();
    int NHOOOP = W3dPrefs::bodyHoopRes();
    QVector<Vector3d> T((NXXXX+1)*(NHOOOP+1)), N((NXXXX+1)*(NHOOOP+1));
    Vector3d TALB, LATB;

    if(!pBody)return;

    Vector3d Point;
    Vector3d Normal;

    if(m_pLeftBodyTexture)  delete m_pLeftBodyTexture;
    if(m_pRightBodyTexture) delete m_pRightBodyTexture;

    QImage leftTexture  = QImage(QString(m_TexturePath+QDir::separator()+"body_left.png"));
    if(leftTexture.isNull()) leftTexture = QImage(QString(":/resources/default_textures/body_left.png"));
    m_pLeftBodyTexture  = new QOpenGLTexture(leftTexture);
    QImage rightTexture  = QImage(QString(m_TexturePath+QDir::separator()+"body_right.png"));
    if(rightTexture.isNull()) rightTexture = QImage(QString(":/resources/default_textures/body_right.png"));
    m_pRightBodyTexture  = new QOpenGLTexture(rightTexture);

    //vertices array size:
    // surface:
    //     (NX+1)*(NH+1) : from 0 to NX, and from 0 to NH
    //     x2 : 2 sides
    // outline:
    //     frameSize()*(NH+1)*2 : frames
    //     (NX+1) + (NX+1)      : top and bottom lines
    //
    // x8 : 3 vertices components, 3 normal components, 2 texture componenents
    int FuseVertexSize(0);

    int nTriangles =  NXXXX*NHOOOP*2;           // quads x2 triangles/quad
    FuseVertexSize  =  nTriangles
                       *3                        // 3 vertices/triangle
                       *8;                       // 3 vertex components, 3 normal components, 2 uv components

    QVector<float> FuseVertexArray(FuseVertexSize);

    int p = 0;
    double ud(0), vd(0);
    for (int k=0; k<=NXXXX; k++)
    {
        ud = double(k) / double(NXXXX);
        for (int l=0; l<=NHOOOP; l++)
        {
            vd = double(l) / double(NHOOOP);
            pBody->getPoint(ud,  vd, true, T[p]);
            pBody->nurbs().getNormal(ud, vd, N[p]);

            p++;
        }
    }

    int nla(0), nlb(0), nta(0), ntb(0);

    int iv=0;
    //right side first;
    p=0;
    for (int k=0; k<NXXXX; k++)
    {
        for (int l=0; l<NHOOOP; l++)
        {
            nta =  k   *(NHOOOP+1)+l;
            ntb =  k   *(NHOOOP+1)+l+1;
            nla = (k+1)*(NHOOOP+1)+l;
            nlb = (k+1)*(NHOOOP+1)+l+1;

            //first triangle
            FuseVertexArray[iv++] = T.at(nta).xf();
            FuseVertexArray[iv++] = T.at(nta).yf();
            FuseVertexArray[iv++] = T.at(nta).zf();
            FuseVertexArray[iv++] = N.at(nta).xf();
            FuseVertexArray[iv++] = N.at(nta).yf();
            FuseVertexArray[iv++] = N.at(nta).zf();
            FuseVertexArray[iv++] = float(NXXXX-k)/float(NXXXX);
            FuseVertexArray[iv++] = float(l)/float(NHOOOP);

            FuseVertexArray[iv++] = T.at(nla).xf();
            FuseVertexArray[iv++] = T.at(nla).yf();
            FuseVertexArray[iv++] = T.at(nla).zf();
            FuseVertexArray[iv++] = N.at(nla).xf();
            FuseVertexArray[iv++] = N.at(nla).yf();
            FuseVertexArray[iv++] = N.at(nla).zf();
            FuseVertexArray[iv++] = float(NXXXX-(k+1))/float(NXXXX);
            FuseVertexArray[iv++] = float(l)/float(NHOOOP);

            FuseVertexArray[iv++] = T.at(nlb).xf();
            FuseVertexArray[iv++] = T.at(nlb).yf();
            FuseVertexArray[iv++] = T.at(nlb).zf();
            FuseVertexArray[iv++] = N.at(nlb).xf();
            FuseVertexArray[iv++] = N.at(nlb).yf();
            FuseVertexArray[iv++] = N.at(nlb).zf();
            FuseVertexArray[iv++] = float(NXXXX-(k+1))/float(NXXXX);
            FuseVertexArray[iv++] = float(l+1)/float(NHOOOP);

            //second triangle
            FuseVertexArray[iv++] = T.at(nta).xf();
            FuseVertexArray[iv++] = T.at(nta).yf();
            FuseVertexArray[iv++] = T.at(nta).zf();
            FuseVertexArray[iv++] = N.at(nta).xf();
            FuseVertexArray[iv++] = N.at(nta).yf();
            FuseVertexArray[iv++] = N.at(nta).zf();
            FuseVertexArray[iv++] = float(NXXXX-k)/float(NXXXX);
            FuseVertexArray[iv++] = float(l)/float(NHOOOP);

            FuseVertexArray[iv++] = T.at(nlb).xf();
            FuseVertexArray[iv++] = T.at(nlb).yf();
            FuseVertexArray[iv++] = T.at(nlb).zf();
            FuseVertexArray[iv++] = N.at(nlb).xf();
            FuseVertexArray[iv++] = N.at(nlb).yf();
            FuseVertexArray[iv++] = N.at(nlb).zf();
            FuseVertexArray[iv++] = float(NXXXX-(k+1))/float(NXXXX);
            FuseVertexArray[iv++] = float(l+1)/float(NHOOOP);

            FuseVertexArray[iv++] = T.at(ntb).xf();
            FuseVertexArray[iv++] = T.at(ntb).yf();
            FuseVertexArray[iv++] = T.at(ntb).zf();
            FuseVertexArray[iv++] = N.at(ntb).xf();
            FuseVertexArray[iv++] = N.at(ntb).yf();
            FuseVertexArray[iv++] = N.at(ntb).zf();
            FuseVertexArray[iv++] = float(NXXXX-k)/float(NXXXX);
            FuseVertexArray[iv++] = float(l+1)/float(NHOOOP);

            p++;
        }
    }

    Q_ASSERT(iv==FuseVertexSize);

    m_vboFuseRight.destroy();
    m_vboFuseRight.create();
    m_vboFuseRight.bind();
    m_vboFuseRight.allocate(FuseVertexArray.data(), FuseVertexSize * int(sizeof(GLfloat)));
    m_vboFuseRight.release();

    //left side next;
    iv=0;
    p=0;
    for (int k=0; k<NXXXX; k++)
    {
        for (int l=0; l<NHOOOP; l++)
        {
            nta =  k   *(NHOOOP+1)+l;
            ntb =  k   *(NHOOOP+1)+l+1;
            nla = (k+1)*(NHOOOP+1)+l;
            nlb = (k+1)*(NHOOOP+1)+l+1;

            //first triangle
            FuseVertexArray[iv++] =  T.at(nta).xf();
            FuseVertexArray[iv++] = -T.at(nta).yf();
            FuseVertexArray[iv++] =  T.at(nta).zf();
            FuseVertexArray[iv++] =  N.at(nta).xf();
            FuseVertexArray[iv++] = -N.at(nta).yf();
            FuseVertexArray[iv++] =  N.at(nta).zf();
            FuseVertexArray[iv++] = float(k)/float(NXXXX);
            FuseVertexArray[iv++] = float(l)/float(NHOOOP);

            FuseVertexArray[iv++] =  T.at(nlb).xf();
            FuseVertexArray[iv++] = -T.at(nlb).yf();
            FuseVertexArray[iv++] =  T.at(nlb).zf();
            FuseVertexArray[iv++] =  N.at(nlb).xf();
            FuseVertexArray[iv++] = -N.at(nlb).yf();
            FuseVertexArray[iv++] =  N.at(nlb).zf();
            FuseVertexArray[iv++] = float(k+1)/float(NXXXX);
            FuseVertexArray[iv++] = float(l+1)/float(NHOOOP);

            FuseVertexArray[iv++] =  T.at(nla).xf();
            FuseVertexArray[iv++] = -T.at(nla).yf();
            FuseVertexArray[iv++] =  T.at(nla).zf();
            FuseVertexArray[iv++] =  N.at(nla).xf();
            FuseVertexArray[iv++] = -N.at(nla).yf();
            FuseVertexArray[iv++] =  N.at(nla).zf();
            FuseVertexArray[iv++] = float(k+1)/float(NXXXX);
            FuseVertexArray[iv++] = float(l)/float(NHOOOP);


            //second triangle
            FuseVertexArray[iv++] =  T.at(nta).xf();
            FuseVertexArray[iv++] = -T.at(nta).yf();
            FuseVertexArray[iv++] =  T.at(nta).zf();
            FuseVertexArray[iv++] =  N.at(nta).xf();
            FuseVertexArray[iv++] = -N.at(nta).yf();
            FuseVertexArray[iv++] =  N.at(nta).zf();
            FuseVertexArray[iv++] = float(k)/float(NXXXX);
            FuseVertexArray[iv++] = float(l)/float(NHOOOP);

            FuseVertexArray[iv++] =  T.at(ntb).xf();
            FuseVertexArray[iv++] = -T.at(ntb).yf();
            FuseVertexArray[iv++] =  T.at(ntb).zf();
            FuseVertexArray[iv++] =  N.at(ntb).xf();
            FuseVertexArray[iv++] = -N.at(ntb).yf();
            FuseVertexArray[iv++] =  N.at(ntb).zf();
            FuseVertexArray[iv++] = float(k)/float(NXXXX);
            FuseVertexArray[iv++] = float(l+1)/float(NHOOOP);

            FuseVertexArray[iv++] =  T.at(nlb).xf();
            FuseVertexArray[iv++] = -T.at(nlb).yf();
            FuseVertexArray[iv++] =  T.at(nlb).zf();
            FuseVertexArray[iv++] =  N.at(nlb).xf();
            FuseVertexArray[iv++] = -N.at(nlb).yf();
            FuseVertexArray[iv++] =  N.at(nlb).zf();
            FuseVertexArray[iv++] = float(k+1)/float(NXXXX);
            FuseVertexArray[iv++] = float(l+1)/float(NHOOOP);

            p++;
        }
    }

    Q_ASSERT(iv==FuseVertexSize);

    m_vboFuseLeft.destroy();
    m_vboFuseLeft.create();
    m_vboFuseLeft.bind();
    m_vboFuseLeft.allocate(FuseVertexArray.data(), FuseVertexSize * int(sizeof(GLfloat)));
    m_vboFuseLeft.release();
}


void gl3dXflView::glMakeFuseSplinesOutline(Body const*pBody)
{
    if(!pBody) return;

    int NXXXX = W3dPrefs::bodyAxialRes();
    int NHOOOP = W3dPrefs::bodyHoopRes();


    Vector3d Point;
    double hinc(0), u(0), v(0);

    //OUTLINE
    // outline:
    //     frameSize()*(NH+1)*2 : frames
    //     (NX+1) + (NX+1)      : top and bottom lines
    //
    int outlinesize =   pBody->frameCount()*(NHOOOP+1)*2 // frames
                      + (NXXXX+1)                       // top outline
                      + (NXXXX+1);                      // bot outline
    outlinesize *=3; // x3 vertices components

    std::vector<float> OutlineVertexArray(outlinesize);

    hinc=1./(double)NHOOOP;

    int iv=0;
    // frames : frameCount() x (NH+1)
    for (int iFr=0; iFr<pBody->frameCount(); iFr++)
    {
        u = pBody->getu(pBody->frameAt(iFr)->position().x);
        for (int j=0; j<=NHOOOP; j++)
        {
            v = (double)j*hinc;
            pBody->getPoint(u,v,true, Point);
            OutlineVertexArray[iv++] = Point.x;
            OutlineVertexArray[iv++] = Point.y;
            OutlineVertexArray[iv++] = Point.z;
        }

        for (int j=NHOOOP; j>=0; j--)
        {
            v = (double)j*hinc;
            pBody->getPoint(u,v,false, Point);
            OutlineVertexArray[iv++] = Point.x;
            OutlineVertexArray[iv++] = Point.y ;
            OutlineVertexArray[iv++] = Point.z;
        }
    }

    //top line: NX+1
    v = 0.0;
    for (int iu=0; iu<=NXXXX; iu++)
    {
        pBody->getPoint((double)iu/(double)NXXXX,v, true, Point);
        OutlineVertexArray[iv++] = Point.x;
        OutlineVertexArray[iv++] = Point.y;
        OutlineVertexArray[iv++] = Point.z;
    }

    //bottom line: NX+1
    v = 1.0;
    for (int iu=0; iu<=NXXXX; iu++)
    {
        pBody->getPoint((double)iu/(double)NXXXX,v, true, Point);
        OutlineVertexArray[iv++] = Point.x;
        OutlineVertexArray[iv++] = Point.y;
        OutlineVertexArray[iv++] = Point.z;
    }
    Q_ASSERT(iv==outlinesize);

    m_vboFuseOutline.destroy();
    m_vboFuseOutline.create();
    m_vboFuseOutline.bind();
    m_vboFuseOutline.allocate(OutlineVertexArray.data(), outlinesize * sizeof(GLfloat));
    m_vboFuseOutline.release();
}


/** used in GL3DWingDlg and gl3dBodyView*/
void gl3dXflView::paintSectionHighlight()
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    m_shadLine.bind();
    m_shadLine.enableAttributeArray(m_locLine.m_attrVertex);
    m_vboHighlight.bind();
    m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3, 3 * sizeof(GLfloat));
    m_shadLine.setUniformValue(m_locLine.m_UniColor, QColor(255,0,0));
    glLineWidth(5);

    int pos = 0;
    for(int iLines=0; iLines<m_nHighlightLines; iLines++)
    {
        glDrawArrays(GL_LINE_STRIP, pos, m_HighlightLineSize);
        pos += m_HighlightLineSize;
    }

    m_shadLine.disableAttributeArray(m_locLine.m_attrVertex);
    m_vboHighlight.release();
    m_shadLine.release();
}



/** Default mesh, if no polar has been defined */
void gl3dXflView::paintEditWingMesh(QOpenGLBuffer &vbo)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*m_matModel);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj*m_matView*m_matModel);
        m_shadLine.setUniformValue(m_locLine.m_UniColor, W3dPrefs::s_VLMStyle.m_Color);
        m_shadLine.setUniformValue(m_locLine.m_Pattern, GLStipple(W3dPrefs::s_VLMStyle.m_Stipple));
        m_shadLine.setUniformValue(m_locLine.m_Thickness, W3dPrefs::s_VLMStyle.m_Width);

        vbo.bind();
        {
            m_shadLine.enableAttributeArray(m_locLine.m_attrVertex);
            m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3);

            int nTriangles = vbo.size()/3/3/int(sizeof(float)); // three vertices and three components

            f->glLineWidth(W3dPrefs::s_VLMStyle.m_Width);
            int pos = 0;
            for(int p=0; p<nTriangles; p++)
            {
                f->glDrawArrays(GL_LINE_STRIP, pos, 3);
                pos +=3;
            }

            m_shadLine.setUniformValue(m_locLine.m_UniColor, DisplayOptions::backgroundColor());

            f->glEnable(GL_POLYGON_OFFSET_FILL);
            f->glPolygonOffset(DEPTHFACTOR, DEPTHUNITS);
            f->glDrawArrays(GL_TRIANGLES, 0, nTriangles*3);
            f->glDisable(GL_POLYGON_OFFSET_FILL);
        }
        vbo.release();
        m_shadLine.disableAttributeArray(m_locLine.m_attrVertex);
    }
    m_shadLine.release();

    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix, m_matView*m_matModel);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, m_matProj*m_matView*m_matModel);
        m_shadSurf.setUniformValue(m_locSurf.m_UniColor, W3dPrefs::s_VLMStyle.m_Color);

        vbo.bind();
        {
            m_shadSurf.enableAttributeArray(m_locSurf.m_attrVertex);
            m_shadSurf.setAttributeBuffer(m_locSurf.m_attrVertex, GL_FLOAT, 0, 3);

            int nTriangles = vbo.size()/3/3/int(sizeof(float)); // three vertices and three components

            m_shadSurf.setUniformValue(m_locSurf.m_UniColor, DisplayOptions::backgroundColor());

            f->glEnable(GL_POLYGON_OFFSET_FILL);
            f->glPolygonOffset(DEPTHFACTOR, DEPTHUNITS);
            f->glDrawArrays(GL_TRIANGLES, 0, nTriangles*3);
            f->glDisable(GL_POLYGON_OFFSET_FILL);
        }
        vbo.release();
        m_shadSurf.disableAttributeArray(m_locSurf.m_attrVertex);
    }
    m_shadSurf.release();
    f->glDisable(GL_POLYGON_OFFSET_FILL);
}


void gl3dXflView::setSpanStations(Plane const *pPlane, WPolar const *pWPolar, PlaneOpp const*pPOpp)
{
    if(!pPlane || !pWPolar || !pPOpp) return;
    Wing const *pWing = nullptr;

    if(pWPolar->isLLTMethod())
    {
        if(pPOpp)
        {
            m_Ny[0] = pPOpp->m_pWOpp[0]->m_NStation-1;
        }
        else
        {
            m_Ny[0] = LLTAnalysis::nSpanStations();
        }

        m_Ny[1] = m_Ny[2] = m_Ny[3] = 0;
    }
    else
    {
        for(int iWing=0; iWing<MAXWINGS; iWing++)
        {
            pWing = pPlane->wingAt(iWing);
            if(pWing)
            {
                m_Ny[iWing]=0;
                for (int j=0; j<pWing->m_Surface.size(); j++)
                {
                    m_Ny[iWing] += pWing->m_Surface[j]->NYPanels();
                }
            }
        }
    }
}


void gl3dXflView::paintWing(int iWing, Wing const *pWing)
{
    if(!pWing) return;

    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    int CHORDPOINTS = W3dPrefs::chordwiseRes();

    QMatrix4x4 vmMat(m_matView*m_matModel);
    QMatrix4x4 pvmMat(m_matProj*vmMat);

    if(m_bSurfaces)
    {
        QVector<ushort> const &wingIndicesArray = m_WingIndicesArray[iWing];

        int pos = 0;

        bool bTextures = pWing->textures() &&
                (m_pWingBotLeftTexture[iWing] && m_pWingBotRightTexture[iWing] && m_pWingTopLeftTexture[iWing] && m_pWingTopRightTexture[iWing]);

        m_shadSurf.bind();
        {
            m_vboWingSurface[iWing].bind();
            {
                m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix, vmMat);
                m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMat);

                if(s_Light.m_bIsLightOn) m_shadSurf.setUniformValue(m_locSurf.m_Light, 1);
                else                     m_shadSurf.setUniformValue(m_locSurf.m_Light, 0);

                if(bTextures)
                {
                    m_shadSurf.setUniformValue(m_locSurf.m_HasTexture, 1);
//                    m_shadSurf.setUniformValue(m_locSurf.m_TexSampler, 0); //TEXTURE0  is the default anyway
                }
                else
                {
                    m_shadSurf.setUniformValue(m_locSurf.m_HasTexture, 0);
                    m_shadSurf.setUniformValue(m_locSurf.m_UniColor, pWing->wingColor());
                    m_shadSurf.setUniformValue(m_locSurf.m_HasUniColor, 1);
                }

                m_shadSurf.setUniformValue(m_locSurf.m_TwoSided, 1);
                glDisable(GL_CULL_FACE);

                m_shadSurf.enableAttributeArray(m_locSurf.m_attrVertex);
                m_shadSurf.enableAttributeArray(m_locSurf.m_attrNormal);

                int stride = 8;
                m_shadSurf.setAttributeBuffer(m_locSurf.m_attrVertex, GL_FLOAT, 0,                  3, stride * sizeof(GLfloat));
                m_shadSurf.setAttributeBuffer(m_locSurf.m_attrNormal, GL_FLOAT, 3* sizeof(GLfloat), 3, stride * sizeof(GLfloat));
                if(bTextures)
                {
                    m_shadSurf.enableAttributeArray(m_locSurf.m_attrUV);
                    m_shadSurf.setAttributeBuffer(m_locSurf.m_attrUV, GL_FLOAT, 6*sizeof(GLfloat), 2, stride *sizeof(GLfloat));
                }

                glEnable(GL_POLYGON_OFFSET_FILL);
                glPolygonOffset(DEPTHFACTOR, DEPTHUNITS);

                //indices array size:
                //  Top & bottom surfaces
                //      NSurfaces
                //      x (ChordPoints-1)quads
                //      x2 triangles per/quad
                //      x2 top and bottom surfaces
                //      x3 indices/triangle

                pos = 0;
                for (int j=0; j<pWing->m_Surface.count(); j++)
                {
                    Surface const *pSurf = pWing->m_Surface.at(j);
                    //top surface
                    if(bTextures)
                    {
                        if(pSurf->isLeftSurf()) m_pWingTopLeftTexture[iWing]->bind();
                        else                    m_pWingTopRightTexture[iWing]->bind();
                    }
                    glDrawElements(GL_TRIANGLES, (CHORDPOINTS-1)*2*3, GL_UNSIGNED_SHORT, wingIndicesArray.data()+pos);
                    if(bTextures)
                    {
                        if(pSurf->isLeftSurf()) m_pWingTopLeftTexture[iWing]->release();
                        else                    m_pWingTopRightTexture[iWing]->release();
                    }
                    pos += (CHORDPOINTS-1)*2*3;
                    // bottom surface
                    if(bTextures)
                    {
                        if(pSurf->isLeftSurf()) m_pWingBotLeftTexture[iWing]->bind();
                        else                    m_pWingBotRightTexture[iWing]->bind();
                    }
                    glDrawElements(GL_TRIANGLES, (CHORDPOINTS-1)*2*3, GL_UNSIGNED_SHORT, wingIndicesArray.data()+pos);
                    if(bTextures)
                    {
                        if(pSurf->isLeftSurf()) m_pWingBotLeftTexture[iWing]->release();
                        else                    m_pWingBotRightTexture[iWing]->release();
                    }
                    pos += (CHORDPOINTS-1)*2*3;
                }

                for (int j=0; j<pWing->m_Surface.count(); j++)
                {
                    Surface const *pSurf = pWing->m_Surface.at(j);
                    //tip ssurface
                    if(pSurf->isTipLeft())
                    {
                        glDrawElements(GL_TRIANGLES, (CHORDPOINTS-1)*2*3, GL_UNSIGNED_SHORT, wingIndicesArray.data()+pos);
                        pos += (CHORDPOINTS-1)*2*3;
                    }

                    if(pSurf->isTipRight())
                    {
                        glDrawElements(GL_TRIANGLES, (CHORDPOINTS-1)*2*3, GL_UNSIGNED_SHORT, wingIndicesArray.data()+pos);
                        pos += (CHORDPOINTS-1)*2*3;
                    }
                }

                glDisable(GL_POLYGON_OFFSET_FILL);
            }

            m_vboWingSurface[iWing].release();

            m_shadSurf.disableAttributeArray(m_locSurf.m_attrVertex);
            m_shadSurf.disableAttributeArray(m_locSurf.m_attrNormal);
        }
        m_shadSurf.release();
    }

    if(m_bOutline)
    {
        m_shadLine.bind();
        {
            m_shadLine.setUniformValue(m_locLine.m_UniColor, W3dPrefs::s_OutlineStyle.m_Color);
            m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*m_matModel);
            m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj*m_matView*m_matModel);

            m_shadLine.setUniformValue(m_locLine.m_Pattern, GLStipple(W3dPrefs::s_OutlineStyle.m_Stipple));
            m_shadLine.setUniformValue(m_locLine.m_Thickness, W3dPrefs::s_OutlineStyle.m_Width);

            m_vboWingOutline[iWing].bind();
            {
                m_shadLine.enableAttributeArray(m_locLine.m_attrVertex);
                m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3, 3* sizeof(GLfloat));

                glLineWidth(W3dPrefs::s_OutlineStyle.m_Width);
                glEnable (GL_LINE_STIPPLE);
                GLLineStipple(W3dPrefs::s_OutlineStyle.m_Stipple);

                glDrawArrays(GL_LINES, 0, m_iWingOutlinePoints[iWing]);
            }
            m_vboWingOutline[iWing].release();

            m_shadLine.disableAttributeArray(m_locLine.m_attrVertex);
        }
        m_shadLine.release();
    }
    glDisable(GL_LINE_STIPPLE);
}



void gl3dXflView::paintFoilNames(Wing const *pWing)
{
    int j=0;
    Foil const *pFoil=nullptr;

    QColor clr(105,105,195);
    if(DisplayOptions::isLightTheme()) clr = clr.darker();
    else                               clr = clr.lighter();

    for(j=0; j<pWing->m_Surface.size(); j++)
    {
        pFoil = pWing->m_Surface.at(j)->foilA();

        if(pFoil) glRenderText(pWing->m_Surface.at(j)->m_TA.x, pWing->m_Surface.at(j)->m_TA.y, pWing->m_Surface.at(j)->m_TA.z,
                               pFoil->name(),
                               clr);
    }

    j = pWing->m_Surface.size()-1;
    pFoil = pWing->m_Surface.at(j)->foilB();
    if(pFoil) glRenderText(pWing->m_Surface.at(j)->m_TB.x, pWing->m_Surface.at(j)->m_TB.y, pWing->m_Surface.at(j)->m_TB.z,
                           pFoil->name(),
                           clr);
}


void gl3dXflView::paintMasses(double volumeMass, const Vector3d &pos, const QString &tag, const QVector<PointMass*> &ptMasses)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    QColor massclr = W3dPrefs::s_MassColor;
    if(DisplayOptions::isLightTheme()) massclr = massclr.darker();
    else                               massclr = massclr.lighter();

    double delta = 0.02/m_glScalef;
    if(qAbs(volumeMass)>PRECISION)
    {
        glRenderText(pos.x, pos.y, pos.z + delta,
                     tag + QString(" (%1").arg(volumeMass*Units::kgtoUnit(), 0,'g',3) + Units::weightUnitLabel()+")",
                     massclr);
    }

    for(int im=0; im<ptMasses.size(); im++)
    {
        paintSphere(ptMasses[im]->position() +pos,
                    W3dPrefs::s_MassRadius/m_glScalef,
                    massclr,
                    true);
        glRenderText(ptMasses[im]->position().x + pos.x,
                     ptMasses[im]->position().y + pos.y,
                     ptMasses[im]->position().z + pos.z + delta,
                     ptMasses[im]->tag()+QString(" (%1").arg(ptMasses[im]->mass()*Units::kgtoUnit(), 0,'g',3)+Units::weightUnitLabel()+")",
                     massclr);
    }
}


/**
 * Draws the point masses, the object masses, and the CG position in the OpenGL viewport
*/
void gl3dXflView::paintMasses(Plane const *pPlane)
{
    if(!pPlane) return;
    double delta = 0.02/m_glScalef;

    for(int iw=0; iw<MAXWINGS; iw++)
    {
        if(pPlane->wingAt(iw))
        {
            paintMasses(pPlane->wingAt(iw)->m_VolumeMass, pPlane->wingLE(iw),
                        pPlane->wingAt(iw)->m_WingName,   pPlane->wingAt(iw)->m_PointMass);
        }
    }

    paintMasses(0.0, Vector3d(0.0,0.0,0.0),"",pPlane->m_PointMass);


    if(pPlane->body())
    {
        Body const *pCurBody = pPlane->body();

        paintMasses(pCurBody->m_VolumeMass,
                    pPlane->bodyPos(),
                    pCurBody->m_BodyName,
                    pCurBody->m_PointMass);
    }

    QColor massclr = W3dPrefs::s_MassColor;
    if(DisplayOptions::isLightTheme()) massclr = massclr.darker();
    else                               massclr = massclr.lighter();

    //plot CG
    Vector3d Place(pPlane->CoG().x, pPlane->CoG().y, pPlane->CoG().z);
    paintSphere(Place, W3dPrefs::s_MassRadius*2.0/m_glScalef,
                massclr);

    glRenderText(pPlane->CoG().x, pPlane->CoG().y, pPlane->CoG().z + delta,
                 "CoG "+QString("%1").arg(pPlane->totalMass()*Units::kgtoUnit(), 7,'g',3)
                 +Units::weightUnitLabel(), massclr);
}


void gl3dXflView::glMakeWingGeometry(int iWing, Wing const *pWing, Body const *pBody)
{
    ushort CHORDPOINTS = ushort(W3dPrefs::chordwiseRes());

    Vector3d N, Pt;
    QVector<Vector3d>NormalA(CHORDPOINTS);
    QVector<Vector3d>NormalB(CHORDPOINTS);
    QVector<Vector3d>PtBotLeft(pWing->m_Surface.count() * CHORDPOINTS);
    QVector<Vector3d>PtBotRight(pWing->m_Surface.count() * CHORDPOINTS);
    QVector<Vector3d>PtTopLeft(pWing->m_Surface.count() * CHORDPOINTS);
    QVector<Vector3d>PtTopRight(pWing->m_Surface.count() * CHORDPOINTS);

    QVector<double>leftV(CHORDPOINTS);
    QVector<double>rightV(CHORDPOINTS);
    double leftU=0.0, rightU=1.0;
    memset(NormalA.data(), 0, sizeof(CHORDPOINTS*sizeof(Vector3d)));
    memset(NormalB.data(), 0, sizeof(CHORDPOINTS*sizeof(Vector3d)));
    //vertices array size:
    // surface:
    //     pWing->NSurfaces
    //     xCHORDPOINTS : from 0 to CHORDPOINTS
    //     x2  for A and B sides
    //     x2  for top and bottom
    // outline
    //     2 points mLA & mLB for leading edge
    //     2 points mTA & mTB for trailing edge
    //
    // x8  : for 3 vertex components, 3 normal components, 2 texture components

    int bufferSize = pWing->m_Surface.count()*CHORDPOINTS*2*2 ;
    bufferSize *= 8;

    QVector<float>wingVertexArray(bufferSize);

    N.set(0.0, 0.0, 0.0);
    int iv=0; //index of vertex components

    //SURFACE
    for (int j=0; j<pWing->m_Surface.size(); j++)
    {
        Surface const *pSurf = pWing->surface(j);
        if(!pSurf) continue;

        //top surface
        pSurf->getSidePoints(TOPSURFACE, pBody, PtTopLeft, PtTopRight, NormalA, NormalB, CHORDPOINTS);
        pWing->getTextureUV(j, leftV.data(), rightV.data(), leftU, rightU, CHORDPOINTS);

        //left side vertices
        for (int l=0; l<CHORDPOINTS; l++)
        {
            wingVertexArray[iv++] = PtTopLeft.at(l).xf();
            wingVertexArray[iv++] = PtTopLeft.at(l).yf();
            wingVertexArray[iv++] = PtTopLeft.at(l).zf();
            wingVertexArray[iv++] = NormalA.at(l).xf();
            wingVertexArray[iv++] = NormalA.at(l).yf();
            wingVertexArray[iv++] = NormalA.at(l).zf();
            wingVertexArray[iv++] = float(leftU);
            wingVertexArray[iv++] = float(leftV.at(l));
        }
        //right side vertices
        for (int l=0; l<CHORDPOINTS; l++)
        {
            wingVertexArray[iv++] = PtTopRight.at(l).xf();
            wingVertexArray[iv++] = PtTopRight.at(l).yf();
            wingVertexArray[iv++] = PtTopRight.at(l).zf();
            wingVertexArray[iv++] = NormalB.at(l).xf();
            wingVertexArray[iv++] = NormalB.at(l).yf();
            wingVertexArray[iv++] = NormalB.at(l).zf();
            wingVertexArray[iv++] = float(rightU);
            wingVertexArray[iv++] = float(rightV.at(l));
        }


        //bottom surface
        pSurf->getSidePoints(BOTSURFACE, pBody, PtBotLeft, PtBotRight,
                                              NormalA, NormalB, CHORDPOINTS);

        //left side vertices
        for (int l=0; l<CHORDPOINTS; l++)
        {
            wingVertexArray[iv++] = PtBotLeft.at(l).xf();
            wingVertexArray[iv++] = PtBotLeft.at(l).yf();
            wingVertexArray[iv++] = PtBotLeft.at(l).zf();
            wingVertexArray[iv++] = NormalA.at(l).xf();
            wingVertexArray[iv++] = NormalA.at(l).yf();
            wingVertexArray[iv++] = NormalA.at(l).zf();
            wingVertexArray[iv++] = float(1.0-leftU);
            wingVertexArray[iv++] = float(leftV.at(l));
        }

        //right side vertices
        for (int l=0; l<CHORDPOINTS; l++)
        {
            wingVertexArray[iv++] = PtBotRight.at(l).xf();
            wingVertexArray[iv++] = PtBotRight.at(l).yf();
            wingVertexArray[iv++] = PtBotRight.at(l).zf();
            wingVertexArray[iv++] = NormalB.at(l).xf();
            wingVertexArray[iv++] = NormalB.at(l).yf();
            wingVertexArray[iv++] = NormalB.at(l).zf();
            wingVertexArray[iv++] = float(1.0-rightU);
            wingVertexArray[iv++] = float(rightV.at(l));
        }
    }


    Q_ASSERT(iv==bufferSize);

    //indices array size:
    //  Top & bottom surfaces
    //      NSurfaces
    //      x (ChordPoints-1)quads
    //      x2 triangles per/quad
    //      x2 top and bottom surfaces
    //      x3 indices/triangle
    //  Tip patches
    //      (CHORDPOINTS-1) quads
    //      x2 triangles per/quad
    //      x2 tip patches
    //      x3 indices/triangle

    m_iWingElems[iWing] =  pWing->m_Surface.count()* (CHORDPOINTS-1) *2 *2 *3
                           + (CHORDPOINTS-1) *2 *2 *3;

    m_WingIndicesArray[iWing].resize(m_iWingElems[iWing]);
    QVector<ushort> &wingIndicesArray = m_WingIndicesArray[iWing];
    int ii = 0;
    ushort nV=0;
    for (int j=0; j<pWing->m_Surface.count(); j++)
    {
        Surface const *pSurf = pWing->surface(j);
        if(!pSurf) continue;

        //topsurface
        for (int l=0; l<CHORDPOINTS-1; l++)
        {
            Q_ASSERT(ii < m_iWingElems[iWing]);
            //first triangle
            wingIndicesArray[ii]   = nV;
            wingIndicesArray[ii+1] = nV+1;
            wingIndicesArray[ii+2] = nV+CHORDPOINTS;
            //second triangle
            wingIndicesArray[ii+3] = nV+CHORDPOINTS;
            wingIndicesArray[ii+4] = nV+1;
            wingIndicesArray[ii+5] = nV+CHORDPOINTS+1;
            ii += 6;
            nV++;
        }
        nV +=CHORDPOINTS+1;

        //botsurface
        for (int l=0; l<CHORDPOINTS-1; l++)
        {
            Q_ASSERT(ii < m_iWingElems[iWing]);
            //first triangle
            wingIndicesArray[ii]   = nV;
            wingIndicesArray[ii+1] = nV+1;
            wingIndicesArray[ii+2] = nV+CHORDPOINTS;
            //second triangle
            wingIndicesArray[ii+3] = nV+CHORDPOINTS;
            wingIndicesArray[ii+4] = nV+1;
            wingIndicesArray[ii+5] = nV+CHORDPOINTS+1;
            ii += 6;
            nV++;
        }
        nV +=CHORDPOINTS+1;
    }

    //TIP PATCHES
    nV=0;
    for (int j=0; j<pWing->m_Surface.count(); j++)
    {
        Surface const *pSurf = pWing->surface(j);
        if(!pSurf) continue;

        if(pSurf->isTipLeft())
        {
            Q_ASSERT(ii+5 < m_iWingElems[iWing]);
            for (int l=0; l<CHORDPOINTS-1; l++)
            {
                //first triangle
                wingIndicesArray[ii]   = nV;
                wingIndicesArray[ii+1] = nV+1;
                wingIndicesArray[ii+2] = nV+2*CHORDPOINTS;
                //second triangle
                wingIndicesArray[ii+3] = nV+2*CHORDPOINTS;
                wingIndicesArray[ii+4] = nV+1;
                wingIndicesArray[ii+5] = nV+2*CHORDPOINTS+1;
                ii += 6;
                nV++; //move one vertex
            }
            nV++; //skip the last vertex
        }

        if(pSurf->isTipRight())
        {
            if(!pSurf->isTipLeft()) nV += CHORDPOINTS;

            Q_ASSERT(ii+5 < m_iWingElems[iWing]);
            for (int l=0; l<CHORDPOINTS-1; l++)
            {
                //first triangle
                wingIndicesArray[ii]   = nV;
                wingIndicesArray[ii+1] = nV+1;
                wingIndicesArray[ii+2] = nV+2*CHORDPOINTS;

                //second triangle
                wingIndicesArray[ii+3] = nV+2*CHORDPOINTS;
                wingIndicesArray[ii+4] = nV+1;
                wingIndicesArray[ii+5] = nV+2*CHORDPOINTS+1;
                ii += 6;
                nV++; //move one vertex
            }
            nV++; //skip the last vertex;
            nV += CHORDPOINTS; //skip the bottom line of this wing section
        }

        if(pSurf->isTipLeft())
        {
            nV+= 3*CHORDPOINTS;
        }
        else if(pWing->m_Surface.at(j)->isTipRight())
        {
        }
        else
        {
            nV +=4*CHORDPOINTS;
        }
    }
    Q_ASSERT(ii==m_iWingElems[iWing]);

    if(m_pWingBotLeftTexture[iWing])  delete m_pWingBotLeftTexture[iWing];
    if(m_pWingTopLeftTexture[iWing])  delete m_pWingTopLeftTexture[iWing];
    if(m_pWingBotRightTexture[iWing]) delete m_pWingBotRightTexture[iWing];
    if(m_pWingTopRightTexture[iWing]) delete m_pWingTopRightTexture[iWing];


    QString textureName;
    switch(pWing->wingType())
    {
        case xfl::MAINWING:
            textureName = "wing_";
            break;
        case xfl::SECONDWING:
            textureName = "wing2_";
            break;
        case xfl::ELEVATOR:
            textureName = "elevator_";
            break;
        case xfl::FIN:
            textureName = "fin_";
            break;
        default:
            textureName="wing_";
            break;
    }


    QImage topLeftTexture;
    getTextureFile(textureName+"top_left", topLeftTexture);
    m_pWingTopLeftTexture[iWing] = new QOpenGLTexture(topLeftTexture);

    QImage botLeftTexture;
    getTextureFile(textureName+"bottom_left", botLeftTexture);
    m_pWingBotLeftTexture[iWing] = new QOpenGLTexture(botLeftTexture);

    QImage topRightTexture;
    getTextureFile(textureName+"top_right", topRightTexture);
    m_pWingTopRightTexture[iWing] = new QOpenGLTexture(topRightTexture);

    QImage botRightTexture;
    getTextureFile(textureName+"bottom_right", botRightTexture);
    m_pWingBotRightTexture[iWing] = new QOpenGLTexture(botRightTexture);

    m_vboWingSurface[iWing].destroy();
    m_vboWingSurface[iWing].create();
    m_vboWingSurface[iWing].bind();
    m_vboWingSurface[iWing].allocate(wingVertexArray.data(), bufferSize * int(sizeof(GLfloat)));
    m_vboWingSurface[iWing].release();


    //make OUTLINE
    //vertices array size:
    // surface:
    //     pWing->NSurfaces
    //     x(CHORDPOINTS-1)*2 : segments from i to i+1, times two vertices
    //                          so that we can make only one call to GL_LINES later on
    //     x2  for A and B sides
    //     x2  for top and bottom
    // flaps
    m_iWingOutlinePoints[iWing]  = pWing->m_Surface.count()*(CHORDPOINTS-1)*2*2*2;

    // outline
    //     2 points mLA & mLB for leading edge
    //     2 points mTA & mTB for trailing edge
    m_iWingOutlinePoints[iWing] += pWing->m_Surface.size()*2*2;

    //TE flap outline....
    for (int j=0; j<pWing->m_Surface.size(); j++)
    {
        Foil const *pFoilA = pWing->m_Surface.at(j)->m_pFoilA;
        Foil const *pFoilB = pWing->m_Surface.at(j)->m_pFoilB;
        if(pFoilA && pFoilB && pFoilA->m_bTEFlap && pFoilB->m_bTEFlap)
        {
            m_iWingOutlinePoints[iWing] += 4;//two vertices for the top line and two for the bottom line
        }
    }
    //LE flap outline....
    for (int j=0; j<pWing->m_Surface.size(); j++)
    {
        Foil const *pFoilA = pWing->m_Surface.at(j)->m_pFoilA;
        Foil const *pFoilB = pWing->m_Surface.at(j)->m_pFoilB;
        if(pFoilA && pFoilB && pFoilA->m_bLEFlap && pFoilB->m_bLEFlap)
        {
            m_iWingOutlinePoints[iWing] += 4;//two vertices for the top line and two for the bottom line
        }
    }

    // x3  : for 3 vertex components
    QVector<float> wingOutlineVertexArray(m_iWingOutlinePoints[iWing]*3, 0);

    iv=0; //index of vertex components

    //SECTIONS OUTLINE
    for (int j=0; j<pWing->m_Surface.size(); j++)
    {
        Surface const *pSurf = pWing->surface(j);
        pSurf->getSidePoints(TOPSURFACE, pBody, PtTopLeft, PtTopRight, NormalA, NormalB, CHORDPOINTS);
        //top surface
        //left side vertices
        for (int l=0; l<CHORDPOINTS-1; l++)
        {
            wingOutlineVertexArray[iv++] = PtTopLeft.at(l).xf();
            wingOutlineVertexArray[iv++] = PtTopLeft.at(l).yf();
            wingOutlineVertexArray[iv++] = PtTopLeft.at(l).zf();
            wingOutlineVertexArray[iv++] = PtTopLeft.at(l+1).xf();
            wingOutlineVertexArray[iv++] = PtTopLeft.at(l+1).yf();
            wingOutlineVertexArray[iv++] = PtTopLeft.at(l+1).zf();
        }
        //right side vertices
        for (int l=0; l<CHORDPOINTS-1; l++)
        {
            wingOutlineVertexArray[iv++] = PtTopRight.at(l).xf();
            wingOutlineVertexArray[iv++] = PtTopRight.at(l).yf();
            wingOutlineVertexArray[iv++] = PtTopRight.at(l).zf();
            wingOutlineVertexArray[iv++] = PtTopRight.at(l+1).xf();
            wingOutlineVertexArray[iv++] = PtTopRight.at(l+1).yf();
            wingOutlineVertexArray[iv++] = PtTopRight.at(l+1).zf();
        }


        //bottom surface

        //left side vertices
        for (int l=0; l<CHORDPOINTS-1; l++)
        {
            wingOutlineVertexArray[iv++] = PtBotLeft.at(l).xf();
            wingOutlineVertexArray[iv++] = PtBotLeft.at(l).yf();
            wingOutlineVertexArray[iv++] = PtBotLeft.at(l).zf();
            wingOutlineVertexArray[iv++] = PtBotLeft.at(l+1).xf();
            wingOutlineVertexArray[iv++] = PtBotLeft.at(l+1).yf();
            wingOutlineVertexArray[iv++] = PtBotLeft.at(l+1).zf();
        }

        //right side vertices
        for (int l=0; l<CHORDPOINTS-1; l++)
        {
            wingOutlineVertexArray[iv++] = PtBotRight.at(l).xf();
            wingOutlineVertexArray[iv++] = PtBotRight.at(l).yf();
            wingOutlineVertexArray[iv++] = PtBotRight.at(l).zf();
            wingOutlineVertexArray[iv++] = PtBotRight.at(l+1).xf();
            wingOutlineVertexArray[iv++] = PtBotRight.at(l+1).yf();
            wingOutlineVertexArray[iv++] = PtBotRight.at(l+1).zf();
        }

        //Leading edge
        wingOutlineVertexArray[iv++] = PtTopLeft.first().xf();
        wingOutlineVertexArray[iv++] = PtTopLeft.first().yf();
        wingOutlineVertexArray[iv++] = PtTopLeft.first().zf();
        wingOutlineVertexArray[iv++] = PtTopRight.first().xf();
        wingOutlineVertexArray[iv++] = PtTopRight.first().yf();
        wingOutlineVertexArray[iv++] = PtTopRight.first().zf();

        //trailing edge
        wingOutlineVertexArray[iv++] = PtTopLeft.at(CHORDPOINTS-1).xf();
        wingOutlineVertexArray[iv++] = PtTopLeft.at(CHORDPOINTS-1).yf();
        wingOutlineVertexArray[iv++] = PtTopLeft.at(CHORDPOINTS-1).zf();
        wingOutlineVertexArray[iv++] = PtTopRight.at(CHORDPOINTS-1).xf();
        wingOutlineVertexArray[iv++] = PtTopRight.at(CHORDPOINTS-1).yf();
        wingOutlineVertexArray[iv++] = PtTopRight.at(CHORDPOINTS-1).zf();
    }

    //TE flap outline
    for (int j=0; j<pWing->m_Surface.size(); j++)
    {
        Surface const *pSurf =  pWing->m_Surface[j];
        Foil const *pFoilA =pSurf->m_pFoilA;
        Foil const *pFoilB =pSurf->m_pFoilB;
        if(pFoilA && pFoilB && pFoilA->m_bTEFlap && pFoilB->m_bTEFlap)
        {
           pSurf->getSurfacePoint(pSurf->m_pFoilA->m_TEXHinge/100.0,
                                  pFoilA->m_TEXHinge/100.0,
                                  0.0, TOPSURFACE, Pt, N);
            wingOutlineVertexArray[iv++] = Pt.xf();
            wingOutlineVertexArray[iv++] = Pt.yf();
            wingOutlineVertexArray[iv++] = Pt.zf();

           pSurf->getSurfacePoint(pSurf->m_pFoilB->m_TEXHinge/100.0,
                                  pFoilB->m_TEXHinge/100.0,
                                  1.0, TOPSURFACE, Pt, N);
            wingOutlineVertexArray[iv++] = Pt.xf();
            wingOutlineVertexArray[iv++] = Pt.yf();
            wingOutlineVertexArray[iv++] = Pt.zf();


           pSurf->getSurfacePoint(pSurf->m_pFoilA->m_TEXHinge/100.0,
                                  pFoilA->m_TEXHinge/100.0,
                                  0.0, BOTSURFACE, Pt, N);
            wingOutlineVertexArray[iv++] = Pt.xf();
            wingOutlineVertexArray[iv++] = Pt.yf();
            wingOutlineVertexArray[iv++] = Pt.zf();


           pSurf->getSurfacePoint(pSurf->m_pFoilB->m_TEXHinge/100.0,
                                  pFoilB->m_TEXHinge/100.0,
                                  1.0, BOTSURFACE, Pt, N);
            wingOutlineVertexArray[iv++] = Pt.xf();
            wingOutlineVertexArray[iv++] = Pt.yf();
            wingOutlineVertexArray[iv++] = Pt.zf();
        }
    }
    //LE flap outline....
    for (int j=0; j<pWing->m_Surface.size(); j++)
    {
        Surface const *pSurf =  pWing->m_Surface.at(j);
        Foil const *pFoilA = pSurf->m_pFoilA;
        Foil const *pFoilB = pSurf->m_pFoilB;
        if(pFoilA && pFoilB && pFoilA->m_bLEFlap && pFoilB->m_bLEFlap)
        {
            pSurf->getSurfacePoint(pFoilA->m_LEXHinge/100.0,
                                   pFoilA->m_LEXHinge/100.0,
                                   0.0, TOPSURFACE, Pt, N);
            wingOutlineVertexArray[iv++] = Pt.xf();
            wingOutlineVertexArray[iv++] = Pt.yf();
            wingOutlineVertexArray[iv++] = Pt.zf();

            pSurf->getSurfacePoint(pFoilB->m_LEXHinge/100.0,
                                   pFoilB->m_LEXHinge/100.0,
                                   1.0, TOPSURFACE, Pt, N);
            wingOutlineVertexArray[iv++] = Pt.xf();
            wingOutlineVertexArray[iv++] = Pt.yf();
            wingOutlineVertexArray[iv++] = Pt.zf();

            pSurf->getSurfacePoint(pFoilA->m_LEXHinge/100.0,
                                   pFoilA->m_LEXHinge/100.0,
                                   0.0, BOTSURFACE, Pt, N);
            wingOutlineVertexArray[iv++] = Pt.xf();
            wingOutlineVertexArray[iv++] = Pt.yf();
            wingOutlineVertexArray[iv++] = Pt.zf();

            pSurf->getSurfacePoint(pFoilB->m_LEXHinge/100.0,
                                   pFoilB->m_LEXHinge/100.0,
                                   1.0, BOTSURFACE, Pt, N);
            wingOutlineVertexArray[iv++] = Pt.xf();
            wingOutlineVertexArray[iv++] = Pt.yf();
            wingOutlineVertexArray[iv++] = Pt.zf();
        }
    }

    Q_ASSERT(iv==m_iWingOutlinePoints[iWing] * 3);

    m_vboWingOutline[iWing].destroy();
    m_vboWingOutline[iWing].create();
    m_vboWingOutline[iWing].bind();
    m_vboWingOutline[iWing].allocate(wingOutlineVertexArray.data(), m_iWingOutlinePoints[iWing] * 3 * int(sizeof(GLfloat)));
    m_vboWingOutline[iWing].release();
}


void gl3dXflView::getTextureFile(QString const &surfaceName, QImage &textureImage)
{
    QString texture = m_TexturePath+QDir::separator()+surfaceName;

    textureImage =  QImage(QString(texture+".png"));
    if(textureImage.isNull())
    {
        textureImage  = QImage(QString(texture+".jpg"));
        if(textureImage.isNull())
        {
            textureImage  = QImage(QString(texture+".jpeg"));
            if(textureImage.isNull())
            {
                textureImage = QImage(QString(":/resources/default_textures/"+surfaceName+".png"));
            }
        }
    }
}


/** Default mesh, if no polar has been defined */
void gl3dXflView::glMakeWingEditMesh(QOpenGLBuffer &vbo, Wing const *pWing)
{    //not necessarily the same Nx for all surfaces, so we need to count the quad panels
    int bufferSize = 0;
    for (int j=0; j<pWing->surfaceCount(); j++)
    {
        Surface *pSurf = pWing->m_Surface[j];
        //tip patches
        if(pSurf->isTipLeft())  bufferSize += (pSurf->NXPanels());
        if(pSurf->isTipRight()) bufferSize += (pSurf->NXPanels());

        // top and bottom surfaces
        bufferSize += pSurf->NXPanels()*2 * (pSurf->NYPanels());
    }
    bufferSize *=2;    // 2 triangles/quad
    bufferSize *=3;    // 3 vertex for each triangle
    bufferSize *=3;    // 3 components for each node

    QVector<float> meshVertexArray(bufferSize);

    int iv=0;

    Vector3d A,B,C,D;

    //tip patches
    for (int j=0; j<pWing->m_Surface.size(); j++)
    {
        Surface const*pSurf = pWing->surface(j);
        if(pSurf->isTipLeft())
        {
            for (int l=0; l<pSurf->NXPanels(); l++)
            {
                pSurf->getPanel(0,l,TOPSURFACE);
                A = pSurf->TA;
                B = pSurf->LA;
                pSurf->getPanel(0,l,BOTSURFACE);
                C = pSurf->LA;
                D = pSurf->TA;

                //first triangle
                meshVertexArray[iv++] = A.xf();
                meshVertexArray[iv++] = A.yf();
                meshVertexArray[iv++] = A.zf();
                meshVertexArray[iv++] = B.xf();
                meshVertexArray[iv++] = B.yf();
                meshVertexArray[iv++] = B.zf();
                meshVertexArray[iv++] = C.xf();
                meshVertexArray[iv++] = C.yf();
                meshVertexArray[iv++] = C.zf();

                //second triangle
                meshVertexArray[iv++] = C.xf();
                meshVertexArray[iv++] = C.yf();
                meshVertexArray[iv++] = C.zf();
                meshVertexArray[iv++] = D.xf();
                meshVertexArray[iv++] = D.yf();
                meshVertexArray[iv++] = D.zf();
                meshVertexArray[iv++] = A.xf();
                meshVertexArray[iv++] = A.yf();
                meshVertexArray[iv++] = A.zf();
            }
        }
        if(pSurf->isTipRight())
        {
            for (int l=0; l<pSurf->NXPanels(); l++)
            {
                pSurf->getPanel(pSurf->NYPanels()-1,l,TOPSURFACE);
                A = pSurf->TB;
                B = pSurf->LB;
                pSurf->getPanel(pSurf->NYPanels()-1,l,BOTSURFACE);
                C = pSurf->LB;
                D = pSurf->TB;

                //first triangle
                meshVertexArray[iv++] = C.xf();
                meshVertexArray[iv++] = C.yf();
                meshVertexArray[iv++] = C.zf();
                meshVertexArray[iv++] = B.xf();
                meshVertexArray[iv++] = B.yf();
                meshVertexArray[iv++] = B.zf();
                meshVertexArray[iv++] = A.xf();
                meshVertexArray[iv++] = A.yf();
                meshVertexArray[iv++] = A.zf();

                //second triangle
                meshVertexArray[iv++] = A.xf();
                meshVertexArray[iv++] = A.yf();
                meshVertexArray[iv++] = A.zf();
                meshVertexArray[iv++] = D.xf();
                meshVertexArray[iv++] = D.yf();
                meshVertexArray[iv++] = D.zf();
                meshVertexArray[iv++] = C.xf();
                meshVertexArray[iv++] = C.yf();
                meshVertexArray[iv++] = C.zf();
            }
        }
    }

    //background surface
    for (int j=0; j<pWing->m_Surface.size(); j++)
    {
        Surface *pSurf = pWing->m_Surface[j];
        for(int k=0; k<pSurf->NYPanels(); k++)
        {
            for (int l=0; l<pSurf->NXPanels(); l++)
            {
                pSurf->getPanel(k,l,TOPSURFACE);

                // first triangle
                meshVertexArray[iv++] = pSurf->LA.xf();
                meshVertexArray[iv++] = pSurf->LA.yf();
                meshVertexArray[iv++] = pSurf->LA.zf();
                meshVertexArray[iv++] = pSurf->TA.xf();
                meshVertexArray[iv++] = pSurf->TA.yf();
                meshVertexArray[iv++] = pSurf->TA.zf();
                meshVertexArray[iv++] = pSurf->TB.xf();
                meshVertexArray[iv++] = pSurf->TB.yf();
                meshVertexArray[iv++] = pSurf->TB.zf();

                //second triangle
                meshVertexArray[iv++] = pSurf->TB.xf();
                meshVertexArray[iv++] = pSurf->TB.yf();
                meshVertexArray[iv++] = pSurf->TB.zf();
                meshVertexArray[iv++] = pSurf->LB.xf();
                meshVertexArray[iv++] = pSurf->LB.yf();
                meshVertexArray[iv++] = pSurf->LB.zf();
                meshVertexArray[iv++] = pSurf->LA.xf();
                meshVertexArray[iv++] = pSurf->LA.yf();
                meshVertexArray[iv++] = pSurf->LA.zf();
            }

            for (int l=0; l<pSurf->NXPanels(); l++)
            {
                pSurf->getPanel(k,l,BOTSURFACE);
                //first triangle
                meshVertexArray[iv++] = pSurf->TB.xf();
                meshVertexArray[iv++] = pSurf->TB.yf();
                meshVertexArray[iv++] = pSurf->TB.zf();
                meshVertexArray[iv++] = pSurf->TA.xf();
                meshVertexArray[iv++] = pSurf->TA.yf();
                meshVertexArray[iv++] = pSurf->TA.zf();
                meshVertexArray[iv++] = pSurf->LA.xf();
                meshVertexArray[iv++] = pSurf->LA.yf();
                meshVertexArray[iv++] = pSurf->LA.zf();

                //second triangle
                meshVertexArray[iv++] = pSurf->LA.xf();
                meshVertexArray[iv++] = pSurf->LA.yf();
                meshVertexArray[iv++] = pSurf->LA.zf();
                meshVertexArray[iv++] = pSurf->LB.xf();
                meshVertexArray[iv++] = pSurf->LB.yf();
                meshVertexArray[iv++] = pSurf->LB.zf();
                meshVertexArray[iv++] = pSurf->TB.xf();
                meshVertexArray[iv++] = pSurf->TB.yf();
                meshVertexArray[iv++] = pSurf->TB.zf();
            }
        }
    }


    Q_ASSERT(iv==bufferSize);


    Q_ASSERT(iv==bufferSize);

    //    m_iWingMeshElems = ii/3;
    vbo.destroy();
    vbo.create();
    vbo.bind();
    vbo.allocate(meshVertexArray.data(), bufferSize * int(sizeof(GLfloat)));
    vbo.release();
}


void gl3dXflView::glMakeBodyFrameHighlight(const Body *pBody, const Vector3d &bodyPos, int iFrame)
{
    //    int NXXXX = W3dPrefsDlg::bodyAxialRes();
    int NHOOOP = W3dPrefs::bodyHoopRes();

    Vector3d Point;
    if(iFrame<0) return;

    Frame const*pFrame = pBody->frameAt(iFrame);
    //    xinc = 0.1;
    double hinc = 1.0/double(NHOOOP-1);

    int bufferSize = 0;
    QVector<float>pHighlightVertexArray;

    m_nHighlightLines = 2; // left and right - could make one instead

    //create 3D Splines or Lines to overlay on the body
    int iv = 0;

    if(pBody->isFlatPanelType())
    {
        m_HighlightLineSize = pFrame->pointCount();
        bufferSize = m_nHighlightLines * m_HighlightLineSize *3 ;
        pHighlightVertexArray.resize(bufferSize);
        for (int k=0; k<pFrame->pointCount();k++)
        {
            pHighlightVertexArray[iv++] = pFrame->m_Position.xf()+bodyPos.xf();
            pHighlightVertexArray[iv++] = pFrame->m_CtrlPoint[k].yf();
            pHighlightVertexArray[iv++] = pFrame->m_CtrlPoint[k].zf()+bodyPos.zf();
        }

        for (int k=0; k<pFrame->pointCount();k++)
        {
            pHighlightVertexArray[iv++] =  pFrame->m_Position.xf()+bodyPos.xf();
            pHighlightVertexArray[iv++] = -pFrame->m_CtrlPoint[k].yf();
            pHighlightVertexArray[iv++] =  pFrame->m_CtrlPoint[k].zf()+bodyPos.zf();
        }
    }
    else if(pBody->isSplineType())
    {
        m_HighlightLineSize = NHOOOP;
        bufferSize = m_nHighlightLines * m_HighlightLineSize *3 ;
        pHighlightVertexArray.resize(bufferSize);

        if(pBody->activeFrame())
        {
            double u = pBody->getu(pFrame->m_Position.x);
            double v = 0.0;
            for (int k=0; k<NHOOOP; k++)
            {
                pBody->getPoint(u,v,true, Point);
                pHighlightVertexArray[iv++] = Point.xf()+bodyPos.xf();
                pHighlightVertexArray[iv++] = Point.yf();
                pHighlightVertexArray[iv++] = Point.zf()+bodyPos.zf();
                v += hinc;
            }

            v = 1.0;
            for (int k=0; k<NHOOOP; k++)
            {
                pBody->getPoint(u,v,false, Point);
                pHighlightVertexArray[iv++] = Point.xf()+bodyPos.xf();
                pHighlightVertexArray[iv++] = Point.yf();
                pHighlightVertexArray[iv++] = Point.zf()+bodyPos.zf();
                v -= hinc;
            }
        }
    }
    Q_ASSERT(iv==bufferSize);

    m_vboHighlight.destroy();
    m_vboHighlight.create();
    m_vboHighlight.bind();
    m_vboHighlight.allocate(pHighlightVertexArray.data(), bufferSize*int(sizeof(float)));
    m_vboHighlight.release();
}



/** Default mesh, if no polar has been defined */
void gl3dXflView::glMakeEditBodyMesh(Body *pBody, Vector3d const&pos)
{
    if(!pBody) return;
    QVector<Panel> panels;
    QVector<Vector3d> nodes;
    pBody->makePanels(0, pos, panels, nodes);
}


void gl3dXflView::paintNormals(QOpenGLBuffer &vbo)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_UniColor, QColor(135,105,35));

        if(m_bUse120StyleShaders) glLineWidth(2);
        else m_shadLine.setUniformValue(m_locLine.m_Thickness, 1);

        m_shadLine.setUniformValue(m_locLine.m_Pattern, GLStipple(Line::SOLID));

        vbo.bind();
        {
            m_shadLine.enableAttributeArray(m_locLine.m_attrVertex);
            m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3, 3*sizeof(GLfloat));


            int nNormals = vbo.size()/2/3/int(sizeof(float)); //  (two vertices) x (x,y,z) = 6

            //    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDrawArrays(GL_LINES, 0, nNormals*2);

            m_shadLine.disableAttributeArray(m_locLine.m_attrVertex);
        }
        vbo.release();
    }
    m_shadLine.release();
}


void gl3dXflView::paintMesh(QOpenGLBuffer &vbo, bool bBackGround)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    m_shadLine.bind();
    {
        m_shadLine.enableAttributeArray(m_locLine.m_attrVertex);
        m_shadLine.setUniformValue(m_locLine.m_UniColor, W3dPrefs::s_VLMStyle.m_Color);
        m_shadLine.setUniformValue(m_locLine.m_Pattern, GLStipple(W3dPrefs::s_VLMStyle.m_Stipple));
        m_shadLine.setUniformValue(m_locLine.m_Thickness, W3dPrefs::s_VLMStyle.m_Width);

        vbo.bind();
        {
            m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3, 6 * sizeof(GLfloat));

            int nPanels = vbo.size()/3/6/int(sizeof(float)); // three vertices and 6 components

            glLineWidth(W3dPrefs::s_VLMStyle.m_Width);
            glEnable(GL_LINE_STIPPLE);
            GLLineStipple(W3dPrefs::s_VLMStyle.m_Stipple);
            int pos = 0;
            for(int p=0; p<nPanels*2; p++)
            {
                glDrawArrays(GL_LINE_STRIP, pos, 3);
                pos +=3 ;
            }
            glDisable (GL_LINE_STIPPLE);

/*            m_shadLine.setUniformValue(m_locLine.m_UniColor, DisplayOptions::backgroundColor());

            if(bBackGround)
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                glEnable(GL_POLYGON_OFFSET_FILL);
                glPolygonOffset(DEPTHFACTOR, DEPTHUNITS);
                glDrawArrays(GL_TRIANGLES, 0, nPanels*3);
                glDisable(GL_POLYGON_OFFSET_FILL);

            }
            m_shadLine.disableAttributeArray(m_locLine.m_attrVertex);*/
        }
        vbo.release();
    }
    m_shadLine.release();

    if(bBackGround)
    {
        m_shadSurf.bind();
        {
            m_shadSurf.setUniformValue(m_locSurf.m_UniColor, DisplayOptions::backgroundColor());
            m_shadSurf.setUniformValue(m_locSurf.m_HasUniColor, 1);

            vbo.bind();
            {
                m_shadSurf.enableAttributeArray(m_locSurf.m_attrVertex);
                m_shadSurf.setAttributeBuffer(m_locSurf.m_attrVertex, GL_FLOAT, 0, 3, 6 * sizeof(GLfloat));


                int nPanels = vbo.size()/3/6/int(sizeof(float)); // three vertices and 6 components
                glEnable(GL_POLYGON_OFFSET_FILL);
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                glPolygonOffset(DEPTHFACTOR, DEPTHUNITS);
                glDrawArrays(GL_TRIANGLES, 0, nPanels*3);
                glDisable(GL_POLYGON_OFFSET_FILL);


                m_shadSurf.disableAttributeArray(m_locSurf.m_attrVertex);
            }
            vbo.release();
        }
        m_shadSurf.release();
    }
}



