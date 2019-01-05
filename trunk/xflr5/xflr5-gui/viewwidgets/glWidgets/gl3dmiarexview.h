/****************************************************************************

    gl3dMiarexView Class
    Copyright (C) 2003-2019 Andre Deperrois

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


#ifndef GL3DMIAREXVIEW_H
#define GL3DMIAREXVIEW_H


#include <gl3dview.h>

class gl3dMiarexView : public gl3dView
{
public:
	gl3dMiarexView(QWidget *parent = 0);
	~gl3dMiarexView();

private:
	void glRenderView();
	void contextMenuEvent (QContextMenuEvent * event);
	void paintGL();
	void paintOverlay();
	void set3DRotationCenter(QPoint point);
	void resizeGL(int width, int height);

public:
	void glMakeCpLegendClr();
	bool glMakeStreamLines(Wing *PlaneWing[MAXWINGS], Vector3d *pNode, WPolar *pWPolar, PlaneOpp *pPOpp, int nPanels);
	void glMakeSurfVelocities(Panel *pPanel, WPolar *pWPolar, PlaneOpp *pPOpp, int nPanels);
	void glMakeTransitions(int iWing, Wing *pWing, WPolar *pWPolar, WingOpp *pWOpp);
	void glMakeLiftStrip(int iWing, Wing *pWing, WPolar *pWPolar, WingOpp *pWOpp);
	void glMakeLiftForce(WPolar *pWPolar, PlaneOpp *pPOpp);
	void glMakeMoments(Wing *pWing, WPolar *pWPolar, PlaneOpp *pPOpp);
	void glMakeDownwash(int iWing, Wing *pWing, WPolar *pWPolar, WingOpp *pWOpp);
	void glMakeDragStrip(int iWing, Wing *pWing, WPolar *pWPolar, WingOpp *pWOpp, double beta);
	void glMakePanelForces(int nPanels, Panel *pPanel, WPolar *pWPolar, PlaneOpp *pPOpp);
	void glMakePanels(QOpenGLBuffer &vbo, int nPanels, int nNodes, Vector3d *pNode, Panel *pPanel, PlaneOpp *pPOpp);

	void paintLift(int iWing);
	void paintMoments();
	void paintDrag(int iWing);
	void paintDownwash(int iWing);
	void paintStreamLines();
	void paintSurfaceVelocities(int nPanels);
	void paintTransitions(int iWing);
	void paintCpLegendClr();
	void paintPanelCp(int nPanels);
	void paintPanelForces(int nPanels);
	void paintMesh(int nPanels);

public slots:
	void on3DReset();

public:
	QOpenGLBuffer m_vboSurfaceVelocities, m_vboPanelCp, m_vboPanelForces, m_vboStreamLines;
	QOpenGLBuffer m_vboLiftForce, m_vboMoments;
	QOpenGLBuffer m_vboICd[MAXWINGS], m_vboVCd[MAXWINGS], m_vboLiftStrips[MAXWINGS], m_vboTransitions[MAXWINGS], m_vboDownwash[MAXWINGS];
	QOpenGLBuffer m_vboMesh, m_vboLegendColor;

};

#endif // GL3DMIAREXVIEW_H
