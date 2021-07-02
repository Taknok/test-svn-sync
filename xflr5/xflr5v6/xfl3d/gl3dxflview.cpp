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

#include "gl3dxflview.h"


#include <globals/mainframe.h>
#include <miarex/design/editbodydlg.h>
#include <miarex/design/editplanedlg.h>
#include <miarex/design/gl3dbodydlg.h>
#include <miarex/design/gl3dwingdlg.h>
#include <miarex/miarex.h>
#include <miarex/objects3d.h>
#include <miarex/view/gl3dscales.h>
#include <xfl3d/controls/w3dprefsdlg.h>
#include <misc/options/settings.h>
#include <xflanalysis/plane_analysis/lltanalysis.h>
#include <xflcore/displayoptions.h>
#include <xflcore/units.h>
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

    m_iBodyElems = 0;
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


void gl3dXflView::glMakeBody3DFlatPanels(Body const *pBody)
{
    Vector3d P1, P2, P3, P4, N, P1P3, P2P4, Tj, Tjp1;

    if(m_pLeftBodyTexture)  delete m_pLeftBodyTexture;
    if(m_pRightBodyTexture) delete m_pRightBodyTexture;

    QString projectPath = Settings::s_LastDirName + QDir::separator() + MainFrame::s_ProjectName+ "_textures";
    QString planeName;
    if(s_pMiarex && s_pMiarex->m_pCurPlane)
    {
        planeName = s_pMiarex->m_pCurPlane->planeName();
    }
    QString texturePath = projectPath+QDir::separator()+planeName+QDir::separator();

    QImage leftTexture  = QImage(QString(texturePath+"body_left.png"));
    if(leftTexture.isNull()) leftTexture = QImage(QString(":/resources/default_textures/body_left.png"));
    m_pLeftBodyTexture  = new QOpenGLTexture(leftTexture);
    QImage rightTexture  = QImage(QString(texturePath+"body_right.png"));
    if(rightTexture.isNull()) rightTexture = QImage(QString(":/resources/default_textures/body_right.png"));
    m_pRightBodyTexture  = new QOpenGLTexture(rightTexture);


    int bufferSize = (pBody->sideLineCount()-1) * (pBody->frameCount()-1); //quads
    bufferSize *= 2;  // two sides
    bufferSize *= 4;  // four vertices per quad
    bufferSize *= 8;  // 8 components per vertex
    QVector<float>pBodyVertexArray(bufferSize);

    //Create triangles
    //  indices array size:
    //    NX*NH
    //    2 triangles per/quad
    //    3 indices/triangle
    //    2 sides
    m_iBodyElems = (pBody->sideLineCount()-1) * (pBody->frameCount()-1); //quads
    m_iBodyElems *= 2;    //two sides
    m_iBodyElems *= 2;    //two triangles per quad
    m_iBodyElems *= 3;    //three vertex per triangle

    m_BodyIndicesArray.resize(m_iBodyElems);
    m_BodyIndicesArray.fill(0);

    int iv=0;
    int ii=0;

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

            P1 = pBody->frameAt(j)->m_CtrlPoint[k];       P1.x = pBody->frameAt(j)->position().x;
            P2 = pBody->frameAt(j+1)->m_CtrlPoint[k];     P2.x = pBody->frameAt(j+1)->position().x;
            P3 = pBody->frameAt(j+1)->m_CtrlPoint[k+1];   P3.x = pBody->frameAt(j+1)->position().x;
            P4 = pBody->frameAt(j)->m_CtrlPoint[k+1];     P4.x = pBody->frameAt(j)->position().x;

            P1P3 = P3-P1;
            P2P4 = P4-P2;
            N = P1P3 * P2P4;
            N.normalize();

            int i1 = iv/8;
            int i2 = i1+1;
            int i3 = i2+1;
            int i4 = i3+1;

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
            pBodyVertexArray[iv++] = float(k)/fnh;
            pBodyVertexArray[iv++] = P4.xf();
            pBodyVertexArray[iv++] = P4.yf();
            pBodyVertexArray[iv++] = P4.zf();
            pBodyVertexArray[iv++] = N.xf();
            pBodyVertexArray[iv++] = N.yf();
            pBodyVertexArray[iv++] = N.zf();
            pBodyVertexArray[iv++] = 1.0f-(P4.xf()-tip)/fLength;
            pBodyVertexArray[iv++] = float(k)/fnh;

            //first triangle
            m_BodyIndicesArray[ii]   = ushort(i1);
            m_BodyIndicesArray[ii+1] = ushort(i2);
            m_BodyIndicesArray[ii+2] = ushort(i3);

            //second triangle
            m_BodyIndicesArray[ii+3] = ushort(i3);
            m_BodyIndicesArray[ii+4] = ushort(i4);
            m_BodyIndicesArray[ii+5] = ushort(i1);
            ii += 6;
        }
    }
    for (int k=0; k<pBody->sideLineCount()-1;k++)
    {
        for (int j=0; j<pBody->frameCount()-1;j++)
        {
            Tj.set(pBody->frameAt(j)->position().x,     0.0, 0.0);
            Tjp1.set(pBody->frameAt(j+1)->position().x, 0.0, 0.0);

            P1 = pBody->frameAt(j)->m_CtrlPoint[k];       P1.x = pBody->frameAt(j)->position().x;
            P2 = pBody->frameAt(j+1)->m_CtrlPoint[k];     P2.x = pBody->frameAt(j+1)->position().x;
            P3 = pBody->frameAt(j+1)->m_CtrlPoint[k+1];   P3.x = pBody->frameAt(j+1)->position().x;
            P4 = pBody->frameAt(j)->m_CtrlPoint[k+1];     P4.x = pBody->frameAt(j)->position().x;

            P1P3 = P3-P1;
            P2P4 = P4-P2;
            N = P1P3 * P2P4;
            N.normalize();

            P1.y = -P1.y;
            P2.y = -P2.y;
            P3.y = -P3.y;
            P4.y = -P4.y;
            N.y = -N.y;

            int i1 = iv/8;
            int i2 = i1+1;
            int i3 = i2+1;
            int i4 = i3+1;

            pBodyVertexArray[iv++] = P1.xf();
            pBodyVertexArray[iv++] = P1.yf();
            pBodyVertexArray[iv++] = P1.zf();
            pBodyVertexArray[iv++] = N.xf();
            pBodyVertexArray[iv++] = N.yf();
            pBodyVertexArray[iv++] = N.zf();
            pBodyVertexArray[iv++] = (P1.xf()-tip)/fLength;
            pBodyVertexArray[iv++] = float(k)/fnh;
            pBodyVertexArray[iv++] = P2.xf();
            pBodyVertexArray[iv++] = P2.yf();
            pBodyVertexArray[iv++] = P2.zf();
            pBodyVertexArray[iv++] = N.xf();
            pBodyVertexArray[iv++] = N.yf();
            pBodyVertexArray[iv++] = N.zf();
            pBodyVertexArray[iv++] = (P2.xf()-tip)/fLength;
            pBodyVertexArray[iv++] = float(k)/fnh;
            pBodyVertexArray[iv++] = P3.xf();
            pBodyVertexArray[iv++] = P3.yf();
            pBodyVertexArray[iv++] = P3.zf();
            pBodyVertexArray[iv++] = N.xf();
            pBodyVertexArray[iv++] = N.yf();
            pBodyVertexArray[iv++] = N.zf();
            pBodyVertexArray[iv++] = (P3.xf()-tip)/fLength;
            pBodyVertexArray[iv++] = float(k+1)/fnh;
            pBodyVertexArray[iv++] = P4.xf();
            pBodyVertexArray[iv++] = P4.yf();
            pBodyVertexArray[iv++] = P4.zf();
            pBodyVertexArray[iv++] = N.xf();
            pBodyVertexArray[iv++] = N.yf();
            pBodyVertexArray[iv++] = N.zf();
            pBodyVertexArray[iv++] = (P4.xf()-tip)/fLength;
            pBodyVertexArray[iv++] = float(k+1)/fnh;

            //first triangle
            m_BodyIndicesArray[ii]   = ushort(i1);
            m_BodyIndicesArray[ii+1] = ushort(i2);
            m_BodyIndicesArray[ii+2] = ushort(i3);

            //second triangle
            m_BodyIndicesArray[ii+3] = ushort(i3);
            m_BodyIndicesArray[ii+4] = ushort(i4);
            m_BodyIndicesArray[ii+5] = ushort(i1);
            ii += 6;
        }
    }
    Q_ASSERT(iv==bufferSize);
    Q_ASSERT(ii==m_iBodyElems);

    m_vboBody.destroy();
    m_vboBody.create();
    m_vboBody.bind();
    m_vboBody.allocate(pBodyVertexArray.data(), bufferSize * int(sizeof(GLfloat)));
    m_vboBody.release();
}


void gl3dXflView::glMakeBodySplines(Body const *pBody)
{
    int NXXXX = W3dPrefsDlg::bodyAxialRes();
    int NHOOOP = W3dPrefsDlg::bodyHoopRes();
    QVector<Vector3d> m_T((NXXXX+1)*(NHOOOP+1));
    Vector3d TALB, LATB;
    int j=0, k=0, l=0, p=0;

    if(!pBody)
    {
        return;
    }

    Vector3d Point;

    Vector3d N;

    if(m_pLeftBodyTexture)  delete m_pLeftBodyTexture;
    if(m_pRightBodyTexture) delete m_pRightBodyTexture;

    QString projectPath = Settings::s_LastDirName + QDir::separator() + MainFrame::s_ProjectName+ "_textures";
    QString planeName;
    if(s_pMiarex && s_pMiarex->m_pCurPlane)
    {
        planeName = s_pMiarex->m_pCurPlane->planeName();
    }
    QString texturePath = projectPath+QDir::separator()+planeName+QDir::separator();

    QImage leftTexture  = QImage(QString(texturePath+"body_left.png"));
    if(leftTexture.isNull()) leftTexture = QImage(QString(":/resources/default_textures/body_left.png"));
    m_pLeftBodyTexture  = new QOpenGLTexture(leftTexture);
    QImage rightTexture  = QImage(QString(texturePath+"body_right.png"));
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
    int bodyVertexSize;
    bodyVertexSize  =   (NXXXX+1)*(NHOOOP+1) *2             // side surfaces
            + pBody->frameCount()*(NHOOOP+1)*2 // frames
            + (NXXXX+1)                       // top outline
            + (NXXXX+1);                      // bot outline

    bodyVertexSize *= 8; // 3 vertex components, 3 normal components, 2 uv components

    QVector<float> pBodyVertexArray(bodyVertexSize);

    p = 0;
    double ud=0, vd=0;
    for (k=0; k<=NXXXX; k++)
    {
        ud = double(k) / double(NXXXX);
        for (l=0; l<=NHOOOP; l++)
        {
            vd = double(l) / double(NHOOOP);
            pBody->getPoint(ud,  vd, true, m_T[p]);
            p++;
        }
    }

    int iv=0; //index of vertex components

    //right side first;
    p=0;
    for (k=0; k<=NXXXX; k++)
    {
        for (l=0; l<=NHOOOP; l++)
        {
            pBodyVertexArray[iv++] = m_T[p].xf();
            pBodyVertexArray[iv++] = m_T[p].yf();
            pBodyVertexArray[iv++] = m_T[p].zf();

            if(k==0)       N.set(-1.0, 0.0, 0.0);
            else if(k==NXXXX) N.set(1.0, 0.0, 0.0);
            else if(l==0)                N.set(0.0, 0.0, 1.0);
            else if(l==NHOOOP)                N.set(0.0,0.0, -1.0);
            else
            {
                LATB = m_T[p+NHOOOP+1] - m_T[p+1];     //    LATB = TB - LA;
                TALB = m_T[p]  - m_T[p+NHOOOP+2];      //    TALB = LB - TA;
                N = TALB * LATB;
                N.normalize();
            }

            pBodyVertexArray[iv++] =  N.xf();
            pBodyVertexArray[iv++] =  N.yf();
            pBodyVertexArray[iv++] =  N.zf();

            pBodyVertexArray[iv++] = float(NXXXX-k)/float(NXXXX);
            pBodyVertexArray[iv++] = float(l)/float(NHOOOP);
            p++;
        }
    }


    //left side next;
    p=0;
    for (k=0; k<=NXXXX; k++)
    {
        for (l=0; l<=NHOOOP; l++)
        {
            pBodyVertexArray[iv++] =  m_T[p].xf();
            pBodyVertexArray[iv++] = -m_T[p].yf();
            pBodyVertexArray[iv++] =  m_T[p].zf();

            if(k==0) N.set(-1.0, 0.0, 0.0);
            else if(k==NXXXX) N.set(1.0, 0.0, 0.0);
            else if(l==0)  N.set(0.0, 0.0, 1.0);
            else if(l==NHOOOP) N.set(0.0,0.0, -1.0);
            else
            {
                LATB = m_T[p+NHOOOP+1] - m_T[p+1];     //    LATB = TB - LA;
                TALB = m_T[p]  - m_T[p+NHOOOP+2];      //    TALB = LB - TA;
                N = TALB * LATB;
                N.normalize();
            }
            pBodyVertexArray[iv++] =  N.xf();
            pBodyVertexArray[iv++] = -N.yf();
            pBodyVertexArray[iv++] =  N.zf();

            pBodyVertexArray[iv++] = float(k)/float(NXXXX);
            pBodyVertexArray[iv++] = float(l)/float(NHOOOP);
            p++;
        }
    }

    //OUTLINE
    double hinc=1./double(NHOOOP);
    double u=0, v=0;
    u=0.0; v = 0.0;

    // frames : frameCount() x (NH+1)
    for (int iFr=0; iFr<pBody->frameCount(); iFr++)
    {
        u = pBody->getu(pBody->frameAt(iFr)->m_Position.x);
        for (j=0; j<=NHOOOP; j++)
        {
            v = double(j)*hinc;
            pBody->getPoint(u,v,true, Point);
            pBodyVertexArray[iv++] = Point.xf();
            pBodyVertexArray[iv++] = Point.yf();
            pBodyVertexArray[iv++] = Point.zf();

            N = Vector3d(0.0, Point.y, Point.z);
            N.normalize();
            pBodyVertexArray[iv++] =  N.xf();
            pBodyVertexArray[iv++] =  N.yf();
            pBodyVertexArray[iv++] =  N.zf();

            pBodyVertexArray[iv++] = float(u);
            pBodyVertexArray[iv++] = float(v);
        }

        for (j=NHOOOP; j>=0; j--)
        {
            v = double(j)*hinc;
            pBody->getPoint(u,v,false, Point);
            pBodyVertexArray[iv++] = Point.xf();
            pBodyVertexArray[iv++] = Point.yf();
            pBodyVertexArray[iv++] = Point.zf();
            N = Vector3d(0.0, Point.y, Point.z);
            N.normalize();
            pBodyVertexArray[iv++] =  N.xf();
            pBodyVertexArray[iv++] =  N.yf();
            pBodyVertexArray[iv++] =  N.zf();

            pBodyVertexArray[iv++] = float(u);
            pBodyVertexArray[iv++] = float(v);
        }
    }

    //top line: NX+1
    v = 0.0;
    for (int iu=0; iu<=NXXXX; iu++)
    {
        pBody->getPoint(double(iu)/double(NXXXX),v, true, Point);
        pBodyVertexArray[iv++] = Point.xf();
        pBodyVertexArray[iv++] = Point.yf();
        pBodyVertexArray[iv++] = Point.zf();

        pBodyVertexArray[iv++] = N.xf();
        pBodyVertexArray[iv++] = N.yf();
        pBodyVertexArray[iv++] = N.zf();

        pBodyVertexArray[iv++] = float(iu)/float(NXXXX);
        pBodyVertexArray[iv++] = float(v);
    }

    //bottom line: NX+1
    v = 1.0;
    for (int iu=0; iu<=NXXXX; iu++)
    {
        pBody->getPoint(double(iu)/double(NXXXX),v, true, Point);
        pBodyVertexArray[iv++] = Point.xf();
        pBodyVertexArray[iv++] = Point.yf();
        pBodyVertexArray[iv++] = Point.zf();
        pBodyVertexArray[iv++] = N.xf();
        pBodyVertexArray[iv++] = N.yf();
        pBodyVertexArray[iv++] = N.zf();

        pBodyVertexArray[iv++] = float(iu)/float(NXXXX);
        pBodyVertexArray[iv++] = float(v);
    }
    Q_ASSERT(iv==bodyVertexSize);


    //Create triangles
    //  indices array size:
    //    NX*NH
    //    2 triangles per/quad
    //    3 indices/triangle
    //    2 sides
    m_BodyIndicesArray.resize(NXXXX*NHOOOP*2*3*2);

    int ii=0;
    int nV=0;

    //left side;
    for (int k=0; k<NXXXX; k++)
    {
        for (int l=0; l<NHOOOP; l++)
        {
            nV = k*(NHOOOP+1)+l; // id of the vertex at the bottom left of the quad
            //first triangle
            m_BodyIndicesArray[ii]   = ushort(nV);
            m_BodyIndicesArray[ii+1] = ushort(nV+NHOOOP+1);
            m_BodyIndicesArray[ii+2] = ushort(nV+1);

            //second triangle
            m_BodyIndicesArray[ii+3] = ushort(nV+NHOOOP+1);
            m_BodyIndicesArray[ii+4] = ushort(nV+1);
            m_BodyIndicesArray[ii+5] = ushort(nV+NHOOOP+1+1);
            ii += 6;
        }
    }

    //right side
    for (k=0; k<NXXXX; k++)
    {
        for (l=0; l<NHOOOP; l++)
        {
            nV = (NXXXX+1)*(NHOOOP+1) + k*(NHOOOP+1)+l; // id of the vertex at the bottom left of the quad
            //first triangle
            m_BodyIndicesArray[ii]   = ushort(nV);
            m_BodyIndicesArray[ii+1] = ushort(nV+NHOOOP+1);
            m_BodyIndicesArray[ii+2] = ushort(nV+1);

            //second triangle
            m_BodyIndicesArray[ii+3] = ushort(nV+NHOOOP+1);
            m_BodyIndicesArray[ii+4] = ushort(nV+1);
            m_BodyIndicesArray[ii+5] = ushort(nV+NHOOOP+1+1);
            ii += 6;
        }
    }
    m_iBodyElems = ii;

    pBody = nullptr;

    m_vboBody.destroy();
    m_vboBody.create();
    m_vboBody.bind();
    m_vboBody.allocate(pBodyVertexArray.data(), bodyVertexSize * int(sizeof(GLfloat)));
    m_vboBody.release();
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
        m_shadLine.setUniformValue(m_locLine.m_UniColor, W3dPrefsDlg::s_VLMStyle.m_Color);
        m_shadLine.setUniformValue(m_locLine.m_Pattern, GLStipple(W3dPrefsDlg::s_VLMStyle.m_Stipple));
        m_shadLine.setUniformValue(m_locLine.m_Thickness, W3dPrefsDlg::s_VLMStyle.m_Width);

        vbo.bind();
        {
            m_shadLine.enableAttributeArray(m_locLine.m_attrVertex);
            m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3);

            int nTriangles = vbo.size()/3/3/int(sizeof(float)); // three vertices and three components

            f->glLineWidth(W3dPrefsDlg::s_VLMStyle.m_Width);
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
        m_shadSurf.setUniformValue(m_locSurf.m_UniColor, W3dPrefsDlg::s_VLMStyle.m_Color);

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


void gl3dXflView::paintBody(Body const *pBody)
{
    if(!pBody) return;
    bool bTextures = pBody->hasTextures() && (m_pLeftBodyTexture && m_pRightBodyTexture);

    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    QMatrix4x4 vmMat(m_matView*m_matModel);
    QMatrix4x4 pvmMat(m_matProj*vmMat);

    int pos = 0;
    int NXXXX = W3dPrefsDlg::bodyAxialRes();
    int NHOOOP = W3dPrefsDlg::bodyHoopRes();

    m_shadSurf.bind();
    {
        m_vboBody.bind();
        {
            m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix, vmMat);
            m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMat);

            if(bTextures)
            {
                m_shadSurf.setUniformValue(m_locSurf.m_HasTexture, 1);
            }
            else
            {
                m_shadSurf.setUniformValue(m_locSurf.m_HasTexture, 0);
                m_shadSurf.setUniformValue(m_locSurf.m_UniColor, pBody->bodyColor());
                if(s_Light.m_bIsLightOn) m_shadSurf.setUniformValue(m_locSurf.m_Light, 1);
                else                     m_shadSurf.setUniformValue(m_locSurf.m_Light, 0);
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

            if(m_bSurfaces)
            {
                glEnable(GL_POLYGON_OFFSET_FILL);
                glPolygonOffset(DEPTHFACTOR, DEPTHUNITS);

                if(bTextures) m_pRightBodyTexture->bind();
                glDrawElements(GL_TRIANGLES, m_iBodyElems/2, GL_UNSIGNED_SHORT, m_BodyIndicesArray.data());
                if(bTextures) m_pRightBodyTexture->release();
                if(bTextures) m_pLeftBodyTexture->bind();
                glDrawElements(GL_TRIANGLES, m_iBodyElems/2, GL_UNSIGNED_SHORT, m_BodyIndicesArray.data()+m_iBodyElems/2);
                if(bTextures) m_pLeftBodyTexture->release();

                glDisable(GL_POLYGON_OFFSET_FILL);
            }

            m_vboBody.release();

            // leave things as they were
            m_shadSurf.disableAttributeArray(m_locSurf.m_attrVertex);
            m_shadSurf.disableAttributeArray(m_locSurf.m_attrNormal);
        }
        m_shadSurf.release();
    }


    if(m_bOutline)
    {
        m_shadLine.bind();
        {
            m_vboBody.bind();
            {
                m_shadLine.enableAttributeArray(m_locLine.m_attrVertex);
                m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3, 8 * sizeof(GLfloat));
                m_shadLine.setUniformValue(m_locLine.m_UniColor, W3dPrefsDlg::s_OutlineStyle.m_Color);

                m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*m_matModel);
                m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj*m_matView*m_matModel);

                glLineWidth(W3dPrefsDlg::s_OutlineStyle.m_Width);

                if(pBody->isSplineType())
                {
                    pos = (NXXXX+1) * (NHOOOP+1) * 2;
                    for(int iFr=0; iFr<pBody->frameCount(); iFr++)
                    {
                        glDrawArrays(GL_LINE_STRIP, pos, (NHOOOP+1)*2);
                        pos += (NHOOOP+1)*2;
                    }
                    glDrawArrays(GL_LINE_STRIP, pos, NXXXX+1);
                    pos += NXXXX+1;
                    glDrawArrays(GL_LINE_STRIP, pos, NXXXX+1);
                }
                else if(pBody->isFlatPanelType())
                {
                    int pos=0;
                    for(int i=0; i<m_iBodyElems/2; i++)
                    {
                        glDrawArrays(GL_LINE_STRIP, pos, 4);
                        pos +=4;
                    }
                }

                m_shadLine.disableAttributeArray(m_locLine.m_attrVertex);
            }
            m_vboBody.release();
        }
        m_shadLine.release();
    }
}


/** Default mesh, if no polar has been defined */
void gl3dXflView::paintEditBodyMesh(const Body *pBody)
{
    if(!pBody) return;

    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*m_matModel);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj*m_matView*m_matModel);
        m_shadLine.enableAttributeArray(m_locLine.m_attrVertex);
        m_vboEditBodyMesh.bind();
        {
            m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3, 3*sizeof(GLfloat));
            m_shadLine.setUniformValue(m_locLine.m_UniColor, W3dPrefsDlg::s_VLMStyle.m_Color);
            //    m_ShaderProgramLine.setUniformValue(m_locLine.m_UniColor, Qt::red);

            if(pBody->isFlatPanelType())
            {
                f->glLineWidth(W3dPrefsDlg::s_VLMStyle.m_Width);

                //        f->ffset(DEPTHFACTOR, DEPTHUNITS);
                f->glDrawArrays(GL_LINES, 0, m_iBodyMeshLines*2);
            }
            else if(pBody->isSplineType())
            {
                int pos=0;
                int NXXXX = W3dPrefsDlg::bodyAxialRes();
                int NHOOOP = W3dPrefsDlg::bodyHoopRes();
                f->glLineWidth(W3dPrefsDlg::s_VLMStyle.m_Width);

                pos=0;
                //x-lines
                for (int l=0; l<2*pBody->m_nhPanels; l++)
                {
                    f->glDrawArrays(GL_LINE_STRIP, pos, NXXXX);
                    pos += NXXXX;
                }

                //hoop lines;
                for (int k=0; k<2*pBody->m_nxPanels; k++)
                {
                    f->glDrawArrays(GL_LINE_STRIP, pos, NHOOOP);
                    pos += NHOOOP;
                }
            }
        }
        m_vboEditBodyMesh.release();

        m_shadLine.disableAttributeArray(m_locLine.m_attrVertex);
    }
    m_shadLine.release();

    QMatrix4x4 vmMat(m_matView*m_matModel);
    QMatrix4x4 pvmMat(m_matProj*vmMat);

    //mesh background
    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix, vmMat);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMat);

        m_shadSurf.setUniformValue(m_locSurf.m_UniColor, pBody->bodyColor());
        if(s_Light.m_bIsLightOn) m_shadSurf.setUniformValue(m_locSurf.m_Light, 1);
        else                     m_shadSurf.setUniformValue(m_locSurf.m_Light, 0);
        m_shadSurf.setUniformValue(m_locSurf.m_HasUniColor, 1);

        m_shadSurf.setUniformValue(m_locSurf.m_TwoSided, 1);
        glDisable(GL_CULL_FACE);

        m_shadSurf.enableAttributeArray(m_locSurf.m_attrVertex);
        m_shadSurf.enableAttributeArray(m_locSurf.m_attrNormal);

        m_vboEditBodyMesh.bind();
        {
            m_shadSurf.setAttributeBuffer(m_locSurf.m_attrVertex, GL_FLOAT, 0,                  3, 6 * sizeof(GLfloat));
            m_shadSurf.setAttributeBuffer(m_locSurf.m_attrNormal, GL_FLOAT, 3* sizeof(GLfloat), 3, 6 * sizeof(GLfloat));

            f->glEnable(GL_POLYGON_OFFSET_FILL);
            f->glPolygonOffset(DEPTHFACTOR, DEPTHUNITS);
            f->glDrawElements(GL_TRIANGLES, m_iBodyElems/2, GL_UNSIGNED_SHORT, m_BodyIndicesArray.data());
            f->glDrawElements(GL_TRIANGLES, m_iBodyElems/2, GL_UNSIGNED_SHORT, m_BodyIndicesArray.data()+m_iBodyElems/2);
            f->glDisable(GL_POLYGON_OFFSET_FILL);
        }
        m_vboEditBodyMesh.release();

        m_shadSurf.disableAttributeArray(m_locSurf.m_attrVertex);
        m_shadSurf.disableAttributeArray(m_locSurf.m_attrNormal);
    }
    m_shadSurf.release();
}


void gl3dXflView::paintWing(int iWing, Wing const *pWing)
{
    if(!pWing) return;

    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    int CHORDPOINTS = W3dPrefsDlg::chordwiseRes();

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
            m_shadLine.setUniformValue(m_locLine.m_UniColor, W3dPrefsDlg::s_OutlineStyle.m_Color);
            m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*m_matModel);
            m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj*m_matView*m_matModel);

            m_shadLine.setUniformValue(m_locLine.m_Pattern, GLStipple(W3dPrefsDlg::s_OutlineStyle.m_Stipple));
            m_shadLine.setUniformValue(m_locLine.m_Thickness, W3dPrefsDlg::s_OutlineStyle.m_Width);

            m_vboWingOutline[iWing].bind();
            {
                m_shadLine.enableAttributeArray(m_locLine.m_attrVertex);
                m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3, 3* sizeof(GLfloat));

                glLineWidth(W3dPrefsDlg::s_OutlineStyle.m_Width);
                glEnable (GL_LINE_STIPPLE);
                GLLineStipple(W3dPrefsDlg::s_OutlineStyle.m_Stipple);

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
    if(Settings::isLightTheme()) clr = clr.darker();
    else                         clr = clr.lighter();

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

    QColor massclr = W3dPrefsDlg::s_MassColor;
    if(Settings::isLightTheme()) massclr = massclr.darker();
    else                         massclr = massclr.lighter();

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
                    W3dPrefsDlg::s_MassRadius/m_glScalef,
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

    QColor massclr = W3dPrefsDlg::s_MassColor;
    if(Settings::isLightTheme()) massclr = massclr.darker();
    else                         massclr = massclr.lighter();

    //plot CG
    Vector3d Place(pPlane->CoG().x, pPlane->CoG().y, pPlane->CoG().z);
    paintSphere(Place, W3dPrefsDlg::s_MassRadius*2.0/m_glScalef,
                massclr);

    glRenderText(pPlane->CoG().x, pPlane->CoG().y, pPlane->CoG().z + delta,
                 "CoG "+QString("%1").arg(pPlane->totalMass()*Units::kgtoUnit(), 7,'g',3)
                 +Units::weightUnitLabel(), massclr);
}


void gl3dXflView::glMakeWingGeometry(int iWing, Wing const *pWing, Body const *pBody)
{
    ushort CHORDPOINTS = ushort(W3dPrefsDlg::chordwiseRes());

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


    QString planeName;
    QString textureName;

    if(s_pMiarex && s_pMiarex->m_pCurPlane)
    {
        planeName = s_pMiarex->m_pCurPlane->planeName();
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
    }
    else
    {
        textureName="wing_";
    }

    QImage topLeftTexture;
    getTextureFile(planeName, textureName+"top_left", topLeftTexture);
    m_pWingTopLeftTexture[iWing] = new QOpenGLTexture(topLeftTexture);

    QImage botLeftTexture;
    getTextureFile(planeName, textureName+"bottom_left", botLeftTexture);
    m_pWingBotLeftTexture[iWing] = new QOpenGLTexture(botLeftTexture);

    QImage topRightTexture;
    getTextureFile(planeName, textureName+"top_right", topRightTexture);
    m_pWingTopRightTexture[iWing] = new QOpenGLTexture(topRightTexture);

    QImage botRightTexture;
    getTextureFile(planeName, textureName+"bottom_right", botRightTexture);
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
        Foil const *pFoilA = pWing->m_Surface[j]->m_pFoilA;
        Foil const *pFoilB = pWing->m_Surface[j]->m_pFoilB;
        if(pFoilA && pFoilB && pFoilA->m_bTEFlap && pFoilB->m_bTEFlap)
        {
            m_iWingOutlinePoints[iWing] += 4;//two vertices for the top line and two for the bottom line
        }
    }
    //LE flap outline....
    for (int j=0; j<pWing->m_Surface.size(); j++)
    {
        Foil const *pFoilA = pWing->m_Surface[j]->m_pFoilA;
        Foil const *pFoilB = pWing->m_Surface[j]->m_pFoilB;
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


void gl3dXflView::getTextureFile(QString const &planeName, QString const &surfaceName, QImage &textureImage)
{
    QString projectPath = Settings::s_LastDirName + QDir::separator() + MainFrame::s_ProjectName+ "_textures";
    QString texturePath = projectPath+QDir::separator()+planeName+QDir::separator()+surfaceName;

    textureImage =  QImage(QString(texturePath+".png"));
    if(textureImage.isNull())
    {
        textureImage  = QImage(QString(texturePath+".jpg"));
        if(textureImage.isNull())
        {
            textureImage  = QImage(QString(texturePath+".jpeg"));
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
    for (int j=0; j<pWing->m_Surface.size(); j++)
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
        Surface *pSurf = pWing->m_Surface[j];
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


void gl3dXflView::glMakeBodyFrameHighlight(const Body *pBody, Vector3d bodyPos, int iFrame)
{
    //    int NXXXX = W3dPrefsDlg::bodyAxialRes();
    int NHOOOP = W3dPrefsDlg::bodyHoopRes();

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
void gl3dXflView::glMakeEditBodyMesh(Body *pBody, Vector3d BodyPosition)
{
    if(!pBody) return;
    int NXXXX = W3dPrefsDlg::bodyAxialRes();
    int NHOOOP = W3dPrefsDlg::bodyHoopRes();
    int nx=0, nh=0;
    Vector3d Pt;
    Vector3d P1, P2, P3, P4, PStart, PEnd;
    QVector<float>meshVertexArray;
    int bufferSize = 0;
    m_iBodyMeshLines = 0;

    float dx = BodyPosition.xf();
    float dy = BodyPosition.yf();
    float dz = BodyPosition.zf();

    int iv=0;

    if(pBody->isFlatPanelType()) //LINES
    {
        bufferSize = 0;
        for (int j=0; j<pBody->frameCount()-1;j++)
        {
            for (int k=0; k<pBody->sideLineCount()-1;k++)
            {
                for(int jp=0; jp<=pBody->m_xPanels[j]; jp++)
                {
                    bufferSize += 6;
                }
                for(int kp=0; kp<=pBody->m_hPanels[k]; kp++)
                {
                    bufferSize += 6;
                }
            }
        }
        bufferSize *=2;

        meshVertexArray.resize(bufferSize);

        for (int j=0; j<pBody->frameCount()-1;j++)
        {
            for (int k=0; k<pBody->sideLineCount()-1;k++)
            {
                P1 = pBody->frameAt(j)->m_CtrlPoint[k];       P1.x = pBody->frameAt(j)->m_Position.x;
                P2 = pBody->frameAt(j+1)->m_CtrlPoint[k];     P2.x = pBody->frameAt(j+1)->m_Position.x;
                P3 = pBody->frameAt(j+1)->m_CtrlPoint[k+1];   P3.x = pBody->frameAt(j+1)->m_Position.x;
                P4 = pBody->frameAt(j)->m_CtrlPoint[k+1];     P4.x = pBody->frameAt(j)->m_Position.x;

                P1.x+=double(dx);   P2.x+=double(dx);   P3.x+=double(dx);   P4.x+=double(dx);
                P1.y+=double(dy);   P2.y+=double(dy);   P3.y+=double(dy);   P4.y+=double(dy);
                P1.z+=double(dz);   P2.z+=double(dz);   P3.z+=double(dz);   P4.z+=double(dz);

                //left side panels
                for(int jp=0; jp<=pBody->m_xPanels[j]; jp++)
                {
                    PStart = P1 + (P2-P1) * double(jp)/double(pBody->m_xPanels[j]);
                    PEnd   = P4 + (P3-P4) * double(jp)/double(pBody->m_xPanels[j]);
                    meshVertexArray[iv++] = PStart.xf();
                    meshVertexArray[iv++] = PStart.yf();
                    meshVertexArray[iv++] = PStart.zf();
                    meshVertexArray[iv++] = PEnd.xf();
                    meshVertexArray[iv++] = PEnd.yf();
                    meshVertexArray[iv++] = PEnd.zf();
                    m_iBodyMeshLines++;
                }
                for(int kp=0; kp<=pBody->m_hPanels[k]; kp++)
                {
                    PStart = P1 + (P4-P1) * double(kp)/double(pBody->m_hPanels[k]);
                    PEnd   = P2 + (P3-P2) * double(kp)/double(pBody->m_hPanels[k]);
                    meshVertexArray[iv++] = PStart.xf();
                    meshVertexArray[iv++] = PStart.yf();
                    meshVertexArray[iv++] = PStart.zf();
                    meshVertexArray[iv++] = PEnd.xf();
                    meshVertexArray[iv++] = PEnd.yf();
                    meshVertexArray[iv++] = PEnd.zf();
                    m_iBodyMeshLines++;
                }

                //right side panels
                for(int jp=0; jp<=pBody->m_xPanels[j]; jp++)
                {
                    PStart = P1 + (P2-P1) * double(jp)/double(pBody->m_xPanels[j]);
                    PEnd   = P4 + (P3-P4) * double(jp)/double(pBody->m_xPanels[j]);
                    meshVertexArray[iv++] =  PStart.xf();
                    meshVertexArray[iv++] = -PStart.yf();
                    meshVertexArray[iv++] =  PStart.zf();
                    meshVertexArray[iv++] =  PEnd.xf();
                    meshVertexArray[iv++] = -PEnd.yf();
                    meshVertexArray[iv++] =  PEnd.zf();
                    m_iBodyMeshLines++;
                }
                for(int kp=0; kp<=pBody->m_hPanels[k]; kp++)
                {
                    PStart = P1 + (P4-P1) * double(kp)/double(pBody->m_hPanels[k]);
                    PEnd   = P2 + (P3-P2) * double(kp)/double(pBody->m_hPanels[k]);
                    meshVertexArray[iv++] =  PStart.xf();
                    meshVertexArray[iv++] = -PStart.yf();
                    meshVertexArray[iv++] =  PStart.zf();
                    meshVertexArray[iv++] =  PEnd.xf();
                    meshVertexArray[iv++] = -PEnd.yf();
                    meshVertexArray[iv++] =  PEnd.zf();
                    m_iBodyMeshLines++;
                }
            }
        }
        Q_ASSERT(m_iBodyMeshLines*6==bufferSize);
        Q_ASSERT(iv==bufferSize);
    }
    else if(pBody->isSplineType()) //NURBS
    {
        pBody->setPanelPos();

        nx = pBody->nxPanels();
        nh = pBody->nhPanels();

        bufferSize = 0;
        bufferSize += nh * NXXXX; // nh longitudinal lines
        bufferSize += nx * NHOOOP; // nx hoop line
        bufferSize *= 2;       // two sides
        bufferSize *= 3;       // 3 components/vertex;

        meshVertexArray.resize(bufferSize);

        //x-lines;
        for (int l=0; l<nh; l++)
        {
            double v = double(l)/double(nh-1);
            for (int k=0; k<NXXXX; k++)
            {
                double u = double(k)/double(NXXXX-1);
                pBody->getPoint(u,  v, true, Pt);
                meshVertexArray[iv++] = Pt.xf() + dx;
                meshVertexArray[iv++] = Pt.yf() + dy;
                meshVertexArray[iv++] = Pt.zf() + dz;
            }
        }
        for (int l=0; l<nh; l++)
        {
            double v = double(l)/double(nh-1);
            for (int k=0; k<NXXXX; k++)
            {
                double u = double(k)/double(NXXXX-1);
                pBody->getPoint(u,  v, false, Pt);
                meshVertexArray[iv++] = Pt.xf() + dx;
                meshVertexArray[iv++] = Pt.yf() + dy;
                meshVertexArray[iv++] = Pt.zf() + dz;
            }
        }

        //hoop lines;
        for (int k=0; k<nx; k++)
        {
            double uk = pBody->m_XPanelPos[k];
            for (int l=0; l<NHOOOP; l++)
            {
                double v = double(l)/double(NHOOOP-1);
                pBody->getPoint(uk,  v, true, Pt);
                meshVertexArray[iv++] = Pt.xf() + dx;
                meshVertexArray[iv++] = Pt.yf() + dy;
                meshVertexArray[iv++] = Pt.zf() + dz;
            }
        }
        for (int k=0; k<nx; k++)
        {
            double uk = pBody->m_XPanelPos[k];
            for (int l=0; l<NHOOOP; l++)
            {
                double v = double(l)/double(NHOOOP-1);
                pBody->getPoint(uk,  v, false, Pt);
                meshVertexArray[iv++] = Pt.xf() + dx;
                meshVertexArray[iv++] = Pt.yf() + dy;
                meshVertexArray[iv++] = Pt.zf() + dz;
            }
        }
    }
    Q_ASSERT(iv==bufferSize);

    m_vboEditBodyMesh.destroy();
    m_vboEditBodyMesh.create();
    m_vboEditBodyMesh.bind();
    m_vboEditBodyMesh.allocate(meshVertexArray.data(), bufferSize * int(sizeof(GLfloat)));
    m_vboEditBodyMesh.release();
}
