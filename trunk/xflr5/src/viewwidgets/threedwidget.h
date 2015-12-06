/****************************************************************************

	GLWidget Class
	Copyright (C) 2009-2012 Andre Deperrois adeperrois@xflr5.com

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


/**
*@file threedwidget.h
*@brief This file implements the declaration of the class ThreeDWidget,
*Used for 3d openGL-based drawing in the central area of the application's MainFrame.
*/

#ifndef THREEDWIDGET_H
#define THREEDWIDGET_H

#include <QOpenGLWidget>
#include <QPixmap>
#include "params.h"
#include "misc/GLLightDlg.h"
#include "objects/CRectangle.h"
#include <ArcBall.h>
#include <PointMass.h>
#include "misc/GLLightDlg.h"


#define GLLISTSPHERE             531

#define VLMSTREAMLINES          1220
#define SURFACESPEEDS           1221
#define LIFTFORCE               1222
#define VLMMOMENTS              1223
#define VLMWINGLIFT             1232
#define VLMWINGDRAG             1236
#define VLMWINGWASH             1241
#define VLMWINGTOPTRANS         1245
#define VLMWINGBOTTRANS         1249
#define WINGSURFACES            1300
#define WINGOUTLINE             1304

#define MESHPANELS              1372
#define MESHBACK                1373

#define WINGLEGEND              1376
#define WOPPLEGEND              1377
#define WOPPCPLEGENDCLR         1378
#define WOPPCPLEGENDTXT         1379

#define WINGWAKEPANELS          1383

#define VLMCTRLPTS              1385
#define VLMVORTICES             1386

#define PANELCP                 1390
#define PANELFORCEARROWS        1391
#define PANELFORCELEGENDTXT     1392

#define ARCBALLLIST             1414
#define ARCPOINTLIST            1415


#define BODYGEOMBASE            4321
#define BODYMESHBASE            5322
#define BODYCPBASE              6173
#define BODYFORCELISTBASE       6245

/** @enum This enumeration lists the different 3D views used in the program, i.e. the view in Miarex, in Body edition and in Wing edition.*/
typedef enum {GLMIAREXVIEW, GLBODYVIEW, GLWINGVIEW, GLPLANEVIEW, GLEDITBODYVIEW} enumGLView;



/**
*@class ThreeDWidget 
* This class is used for 3d OpenGl-based drawing in the central area of the application's MainFrame.
* There are three instances of this class, one attached to the QStackedWidget of the MainFrame, one for Wing edition, and one for Body edition.
* Depending on the active application, this class calls the drawings methods in QMiarex, in the GLBodyDlg or in GLWingDlg.
* All Qt events received by this widget are sent to the child windows for handling.
*/
class ThreeDWidget : public QOpenGLWidget
{
    Q_OBJECT

	friend class QMiarex;
	friend class GL3dBodyDlg;
	friend class GL3dWingDlg;
	friend class MainFrame;
	friend class ArcBall;
	friend class EditPlaneDlg;
	friend class EditBodyDlg;

public:
	ThreeDWidget(QWidget *parent = 0);
	void glCreateArcballList(ArcBall &ArcBall, double GLScale);
	void glDrawAxes(double length, QColor AxisColor, int AxisStyle, int AxisWidth);
	void glInverseMatrix();
	void glCreateUnitSphere();
	void glRenderView();
	void glRenderSphere(double radius);
	void glRenderText(double x, double y, double z, const QString & str, QColor textColor = QColor(Qt::white));
	void glRenderText(int x, int y, const QString & str, QColor textColor = QColor(Qt::white));
	void glSetupLight(double Offset_y, double LightFactor);
	void normalVector(GLdouble p1[3], GLdouble p2[3],  GLdouble p3[3], GLdouble n[3]);
	enumGLView &iView(){return m_iView;}

	void setScale(double refLength);

	void screenToViewport(QPoint const &point, CVector &real);
	void viewportToScreen(CVector const &real, QPoint &point);
	void viewportToScreen(double const &x, double const &y, QPoint &point);

	void worldToViewport(const CVector &V, double &Vx, double &Vy);
	void viewportToWorld(CVector vp, CVector &w);


private slots:
	void on3DIso();
	void on3DTop();
	void on3DLeft();
	void on3DFront();
	void on3DReset();
	void onClipPlane(int pos);

	void onAxes(bool bChecked);
	void onSurfaces(bool bChecked);
	void onPanels(bool bChecked);
	void onOutline(bool bChecked);
	void onFoilNames(bool bChecked);
	void onShowMasses(bool bChecked);



private:
	void contextMenuEvent (QContextMenuEvent * event);
	void keyPressEvent(QKeyEvent *event);
	void keyReleaseEvent(QKeyEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseDoubleClickEvent (QMouseEvent *event);
	void paintGL();//virtual override
	void resizeGL(int width, int height);
	void wheelEvent (QWheelEvent *event );
	void paintEvent(QPaintEvent *event);
	QSize sizeHint() const;
	QSize minimumSizeHint() const;

	void glDrawFoils(void *pWing);
	void glDrawMasses(double volumeMass, CVector pos, QString tag, QList<PointMass*> ptMasses);
	QString glError();

	void setupViewPort(int width, int height);
	void reset3DRotationCenter();
	void set3DRotationCenter(QPoint point);



private:
	static void *s_pMiarex;     /**< A void pointer to the instance of the QMiarex widget.*/
	static void *s_pMainFrame;  /**< A void pointer to the instance of the MainFrame widget.*/
	void *m_pParent;            /**< A void pointer to the parent widget. */

	 bool s_bOutline;                   /**< true if the surface outlines are to be displayed in the 3D view*/
	 bool s_bSurfaces;                  /**< true if the surfaces are to be displayed in the 3D view*/
	 bool s_bVLMPanels;                 /**< true if the panels are to be displayed in the 3D view*/
	 bool s_bAxes;                      /**< true if the axes are to be displayed in the 3D view*/
	 bool s_bShowMasses;                /**< true if the point masses are to be displayed on the openGL 3D view */
	 bool s_bFoilNames;                 /**< true if the foil names are to be displayed on the openGL 3D view */


	GLLightDlg m_glLightDlg;


	QRect m_rCltRect;           /**< The client window rectangle  */
	CRectangle m_GLViewRect;    /**< The OpenGl viewport.*/

	enumGLView m_iView;         /**< The identification of the type of the calling parent widget*/

	bool m_bTrans;
	bool m_bDragPoint;
	bool m_bArcball;			//true if the arcball is to be displayed
	bool m_bCrossPoint;			//true if the control point on the arcball is to be displayed


	double m_ClipPlanePos;      /**< the z-position of the clip plane in viewport coordinates */
	double m_glScaled, m_glScaledRef;
	double MatIn[4][4], MatOut[4][4];


	QPoint m_MousePos;
	QPoint m_ptPopUp;
	QPoint m_LastPoint;


	CVector m_RealPopUp;
	CVector m_glViewportTrans;// the translation vector in gl viewport coordinates
	CVector m_glRotCenter;    // the center of rotation in object coordinates... is also the opposite of the translation vector

	ArcBall m_ArcBall;
	QPixmap m_PixText;
};

#endif
