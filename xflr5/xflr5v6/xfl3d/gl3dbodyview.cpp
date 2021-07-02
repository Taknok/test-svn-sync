/****************************************************************************

    gl3dBodyView Class
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

#include <QOpenGLPaintDevice>
#include <QPainter>

#include "gl3dbodyview.h"
#include <misc/options/settings.h>
#include <xfl3d/controls/w3dprefsdlg.h>
#include <miarex/design/gl3dbodydlg.h>
#include <globals/mainframe.h>
#include <xflobjects/objects3d/body.h>


gl3dBodyView::gl3dBodyView(QWidget *pParent) : gl3dXflView(pParent)
{
//    m_pglBodyDlg = dynamic_cast<GL3dBodyDlg*>(pParent);
    m_pBody = nullptr;
    m_bResetglFrameHighlight   = true;
    m_bResetglBody        = true;//otherwise endless repaint if no body present

    //create a default pix from a random image - couldn't find a better way to do this
/*    m_pixTextLegend = QPixmap(":/resources/images/xflr5_64.png");
    m_pixTextLegend.fill(Qt::transparent);

    QFontMetrics fm(DisplayOptions::textFont());
    int w = fm.averageCharWidth()*19;
    int h = fm.height()*5;
    QRect rect(0,0,w,h);
    m_pixTextLegend = m_pixTextLegend.scaled(rect.size());
*/
}


void gl3dBodyView::glRenderView()
{
    if(m_pBody)
    {
        if(m_bVLMPanels) paintEditBodyMesh(m_pBody);
        paintBody(m_pBody);

        if(m_pBody->activeFrame()) paintSectionHighlight();
        if(m_bShowMasses)
            paintMasses(m_pBody->volumeMass(), Vector3d(0.0,0.0,0.0), "Structural mass", m_pBody->m_PointMass);
    }
}


/**
* Overrides the contextMenuEvent method of the base class.
* Dispatches the handling to the active child application.
*/
void gl3dBodyView::contextMenuEvent (QContextMenuEvent * event)
{
    Q_UNUSED(event);
    m_bArcball = false;
    update();

    //    GL3dBodyDlg *pDlg = (GL3dBodyDlg*)m_pParent;
    //    pDlg->showContextMenu(event);
}


void gl3dBodyView::on3dReset()
{
    startResetTimer(m_pBody->length());
}


/**
* Creates the VertexBufferObjects for OpenGL 3.0
*/
void gl3dBodyView::glMake3dObjects()
{
    if(m_bResetglFrameHighlight || m_bResetglBody)
    {
        if(m_pBody->activeFrame())
        {
            glMakeBodyFrameHighlight(m_pBody,Vector3d(0.0,0.0,0.0), m_pBody->m_iActiveFrame);
            m_bResetglFrameHighlight = false;
        }
    }

    if(m_bResetglBody)
    {
        m_bResetglBody = false;
        if(m_pBody->isSplineType())         glMakeBodySplines(m_pBody);
        else if(m_pBody->isFlatPanelType()) glMakeBody3DFlatPanels(m_pBody);
        glMakeEditBodyMesh(m_pBody, Vector3d(0.0,0.0,0.0));
    }
}


bool gl3dBodyView::intersectTheObject(Vector3d const &AA,  Vector3d const &BB, Vector3d &I)
{
    return m_pBody->intersectFlatPanels(AA, BB, I);
}





