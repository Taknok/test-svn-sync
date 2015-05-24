/****************************************************************************

	ThreeDWidget Class
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


#include <QtOpenGL>
#include "mainframe.h"
#include "misc/Settings.h"
#include "miarex/Miarex.h"
#include <math.h>

#include "graph/Curve.h"
#include "miarex/design/GL3dBodyDlg.h"
#include "miarex/design/EditPlaneDlg.h"
#include "miarex/design/EditBodyDlg.h"
#include "miarex/design/GL3dWingDlg.h"
#include "misc/W3dPrefsDlg.h"
#include "threedwidget.h"

void *ThreeDWidget::s_pMiarex;
void *ThreeDWidget::s_pMainFrame;


bool ThreeDWidget::s_bOutline = true;
bool ThreeDWidget::s_bSurfaces = true;
bool ThreeDWidget::s_bVLMPanels = false;
bool ThreeDWidget::s_bAxes = true;
bool ThreeDWidget::s_bShowMasses = false;
bool ThreeDWidget::s_bFoilNames = false;



/**
*The public constructor
*/
ThreeDWidget::ThreeDWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
	if(MainFrame::s_bTrace)
	{
		Trace(".........OpenGL::Resulting_Format");

		Trace("OpenGl::majorversion", format().majorVersion());
		Trace("OpenGl::minorversion", format().minorVersion());
		Trace("OpenGl::depth buffer size", format().depthBufferSize());
		Trace("OpenGl::stereo", format().stereo());
		Trace("OpenGl::swapInterval", format().swapInterval());

		if(format().profile() == QSurfaceFormat::NoProfile)                 Trace("Opengl::CompatibilityProfile::NoProfile");
		else if(format().profile() == QSurfaceFormat::CoreProfile)          Trace("Opengl::CompatibilityProfile::CoreProfile");
		else if(format().profile()== QSurfaceFormat::CompatibilityProfile)  Trace("Opengl::CompatibilityProfile::CompatibilityProfile");

		Trace("\n\n");

		qDebug() << "Current OpenGL context format:" << format();
	}

	m_pParent = parent;
	m_iView = GLMIAREXVIEW;


	setAutoFillBackground(false);

	setMouseTracking(true);
	setCursor(Qt::CrossCursor);

	m_glViewportTrans.x  = 0.0;
	m_glViewportTrans.y  = 0.0;
	m_glViewportTrans.z  = 0.0;

	m_ClipPlanePos  = 5.0;
	m_glScaled      = 1.0;

	m_bTrans                   = false;
	m_bArcball                 = false;
	m_bCrossPoint              = false;

	m_LastPoint.setX(0);
	m_LastPoint.setY(0);

	m_PixText = QPixmap(107, 97);
	m_PixText.fill(Qt::transparent);

	memset(MatIn,  0, 16*sizeof(double));
	memset(MatOut, 0, 16*sizeof(double));

	m_glLightDlg.m_p3DWidget = this;

	m_ArcBall.m_p3dWidget = this;
}


/**
*Overrides the contextMenuEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void ThreeDWidget::contextMenuEvent (QContextMenuEvent * event)
{
	MainFrame *pMainFrame = (MainFrame*)s_pMainFrame;
	QPoint ScreenPt = event->globalPos();
	if(m_iView == GLMIAREXVIEW)
	{
		QMiarex *pMiarex = (QMiarex *)s_pMiarex;
		if (pMiarex->m_iView==XFLR5::W3DVIEW)
		{
			if(pMiarex->m_pCurWPolar && pMiarex->m_pCurWPolar->polarType()==XFLR5::STABILITYPOLAR)
				pMainFrame->m_pW3DStabCtxMenu->exec(ScreenPt);
			else pMainFrame->m_pW3DCtxMenu->exec(ScreenPt);
		}
	}
	else  if(m_iView ==GLBODYVIEW)
	{
		GL3dBodyDlg *pDlg = (GL3dBodyDlg*)m_pParent;
		pDlg->ShowContextMenu(event);
	}
	else if(m_iView == GLWINGVIEW)
	{
//		GL3dWingDlg *pDlg = (GL3dWingDlg*)m_pParent;
//		pDlg->ShowContextMenu(event);
	}
}


/**
*Overrides the mousePressEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void ThreeDWidget::mousePressEvent(QMouseEvent *event)
{
	if(m_iView == GLBODYVIEW)
	{
		GL3dBodyDlg *pDlg = (GL3dBodyDlg*)m_pParent;
		pDlg->mousePressEvent(event);
	}
	else if(m_iView == GLMIAREXVIEW || m_iView == GLWINGVIEW || m_iView == GLPLANEVIEW || m_iView == GLEDITBODYVIEW)
	{
		QPoint point(event->pos().x(), event->pos().y());
		QPoint glPoint(event->pos().x() + geometry().x(), event->pos().y()+geometry().y());

		CVector Real;
		bool bCtrl = false;
		if(event->modifiers() & Qt::ControlModifier) bCtrl =true;

		screenToViewport(point, Real);

		if(geometry().contains(glPoint)) setFocus();

		if (event->buttons() & Qt::MidButton)
		{
			m_bArcball = true;
			m_ArcBall.Start(event->pos().x(), m_rCltRect.height()-event->pos().y());
			m_bCrossPoint = true;

			reset3DRotationCenter();
			update();
		}
		else if (event->buttons() & Qt::LeftButton)
		{
			{
				m_bTrans=true;

				if(geometry().contains(glPoint))
				{
					m_ArcBall.Start(point.x(), m_rCltRect.height()-point.y());
					m_bCrossPoint = true;
					reset3DRotationCenter();
					if (!bCtrl)
					{
						m_bTrans = true;
						setCursor(Qt::ClosedHandCursor);
					}
					else
					{
						m_bArcball = true;
					}
					update();
				}
			}
		}

		m_LastPoint = point;
	}
}


/**
*Overrides the mouseReleaseEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void ThreeDWidget::mouseReleaseEvent(QMouseEvent *event)
{
	if(m_iView == GLBODYVIEW)
	{
		GL3dBodyDlg *pDlg = (GL3dBodyDlg*)m_pParent;
		pDlg->mouseReleaseEvent(event);
	}
	else if(m_iView == GLMIAREXVIEW || m_iView == GLWINGVIEW || m_iView == GLPLANEVIEW || m_iView == GLEDITBODYVIEW)
	{
		setCursor(Qt::CrossCursor);

		m_bTrans      = false;
		m_bDragPoint  = false;
		m_bArcball    = false;
		m_bCrossPoint = false;

	//	We need to re-calculate the translation vector
		int i,j;
		for(i=0; i<4; i++)
			for(j=0; j<4; j++)
				MatIn[i][j] =  m_ArcBall.ab_quat[i*4+j];

		GLInverseMatrix();
		m_glViewportTrans.x =  (MatOut[0][0]*m_glRotCenter.x + MatOut[0][1]*m_glRotCenter.y + MatOut[0][2]*m_glRotCenter.z);
		m_glViewportTrans.y = -(MatOut[1][0]*m_glRotCenter.x + MatOut[1][1]*m_glRotCenter.y + MatOut[1][2]*m_glRotCenter.z);
		m_glViewportTrans.z =  (MatOut[2][0]*m_glRotCenter.x + MatOut[2][1]*m_glRotCenter.y + MatOut[2][2]*m_glRotCenter.z);


		update();
		event->accept();
	}
}


/**
*Overrides the mouseMoveEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void ThreeDWidget::mouseMoveEvent(QMouseEvent *event)
{
	if(m_iView == GLBODYVIEW)
	{
		GL3dBodyDlg *pDlg = (GL3dBodyDlg*)m_pParent;
		pDlg->mouseMoveEvent(event);
	}
	else if(m_iView == GLMIAREXVIEW || m_iView == GLWINGVIEW || m_iView == GLPLANEVIEW || m_iView == GLEDITBODYVIEW)
	{
		QPoint point(event->pos().x(), event->pos().y());
		QPoint glPoint(event->pos().x() + geometry().x(), event->pos().y()+geometry().y());
		m_MousePos = event->pos();
		CVector Real;

		QPoint Delta(point.x()-m_LastPoint.x(), point.y()-m_LastPoint.y());
		screenToViewport(point, Real);

	//	if(!hasFocus()) setFocus();

		bool bCtrl = false;

		if (event->modifiers() & Qt::ControlModifier) bCtrl =true;

		if (event->buttons()   & Qt::LeftButton)
		{
			if(bCtrl&& geometry().contains(glPoint))
			{
				//rotate
				m_ArcBall.Move(point.x(), m_rCltRect.height()-point.y());
				update();
			}
			else if(m_bTrans)
			{
				//translate
				if(geometry().contains(glPoint))
				{
					int side = qMax(geometry().width(), geometry().height());

					m_glViewportTrans.x += (GLfloat)(Delta.x()*2.0/m_glScaled/side);
					m_glViewportTrans.y += (GLfloat)(Delta.y()*2.0/m_glScaled/side);

					m_glRotCenter.x = MatOut[0][0]*(m_glViewportTrans.x) + MatOut[0][1]*(-m_glViewportTrans.y) + MatOut[0][2]*m_glViewportTrans.z;
					m_glRotCenter.y = MatOut[1][0]*(m_glViewportTrans.x) + MatOut[1][1]*(-m_glViewportTrans.y) + MatOut[1][2]*m_glViewportTrans.z;
					m_glRotCenter.z = MatOut[2][0]*(m_glViewportTrans.x) + MatOut[2][1]*(-m_glViewportTrans.y) + MatOut[2][2]*m_glViewportTrans.z;

					update();
				}
			}
		}

		else if (event->buttons() & Qt::MidButton)
		{
				m_ArcBall.Move(point.x(), m_rCltRect.height()-point.y());
				update();
		}

		m_LastPoint = point;
	}
}


/**
*Overrides the mouseDoubleClickEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void ThreeDWidget::mouseDoubleClickEvent ( QMouseEvent * event )
{
	if(m_iView == GLMIAREXVIEW)
	{
//		QMiarex* pMiarex = (QMiarex*)s_pMiarex;
		set3DRotationCenter(event->pos());
	}
	else if(m_iView == GLBODYVIEW)
	{
		GL3dBodyDlg *pDlg = (GL3dBodyDlg*)m_pParent;
		pDlg->doubleClickEvent(event->pos());
	}
	else if(m_iView == GLWINGVIEW)
	{
//		GL3dWingDlg *pDlg = (GL3dWingDlg*)m_pParent;
		set3DRotationCenter(event->pos());
	}
	else if(m_iView == GLPLANEVIEW)
	{
//		ViewObjectDlg *pDlg = (ViewObjectDlg*)m_pParent;
		set3DRotationCenter(event->pos());
	}
	else if(m_iView == GLEDITBODYVIEW)
	{
//		ViewObjectDlg *pDlg = (ViewObjectDlg*)m_pParent;
		set3DRotationCenter(event->pos());
	}
}


/**
*Overrides the wheelEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void ThreeDWidget::wheelEvent(QWheelEvent *event)
{
	double zoomFactor=1.0;
	if(event->delta()>0)
	{
		if(!Settings::s_bReverseZoom) zoomFactor = 1./1.06;
		else                          zoomFactor = 1.06;
	}
	else
	{
		if(!Settings::s_bReverseZoom) zoomFactor = 1.06;
		else                          zoomFactor = 1./1.06;
	}

	if(m_iView == GLMIAREXVIEW)
	{
		m_glScaled *= zoomFactor;
		update();
	}
	else if(m_iView == GLBODYVIEW)
	{
		GL3dBodyDlg *pDlg = (GL3dBodyDlg*)m_pParent;
		pDlg->zoomEvent(event->pos(), zoomFactor);
	}
	else if(m_iView == GLWINGVIEW || m_iView==GLPLANEVIEW || m_iView==GLEDITBODYVIEW)
	{
		QPoint glPoint(event->pos().x() + geometry().x(), event->pos().y()+geometry().y());

		if(geometry().contains(glPoint))
		{
			setFocus();	//The mouse button has been wheeled
			m_glScaled *= zoomFactor;
		}
		update();
	}
}


/*void ThreeDWidget::initializeGL()
{
	glClearColor(.1, 0.0784, 0.1569, 1.0);
}*/

/**
*Overrides the keyPressEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void ThreeDWidget::keyPressEvent(QKeyEvent *event)
{
	if(m_iView == GLMIAREXVIEW)
	{
		QMiarex* pMiarex = (QMiarex*)s_pMiarex;
		pMiarex->keyPressEvent(event);
	}
	else if(m_iView == GLBODYVIEW)
	{
		GL3dBodyDlg *pDlg = (GL3dBodyDlg*)m_pParent;
		pDlg->keyPressEvent(event);
	}
	else if(m_iView == GLWINGVIEW || m_iView == GLPLANEVIEW || m_iView == GLEDITBODYVIEW)
	{
		switch (event->key())
		{
			case Qt::Key_Control:
			{
				m_bArcball = true;
				update();
				break;
			}
			case Qt::Key_R:
			{
				on3DReset();
				event->accept();
				break;
			}
			default:
				event->ignore();
		}
	}
}


/**
*Overrides the keyReleaseEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void ThreeDWidget::keyReleaseEvent(QKeyEvent *event)
{
	if(m_iView == GLBODYVIEW)
	{
		GL3dBodyDlg *pDlg = (GL3dBodyDlg*)m_pParent;
		pDlg->keyReleaseEvent(event);
	}
	else if(m_iView ==GLMIAREXVIEW || m_iView == GLWINGVIEW || m_iView == GLPLANEVIEW || m_iView == GLEDITBODYVIEW)
	{
		switch (event->key())
		{
			case Qt::Key_Control:
			{
				m_bArcball = false;
				update();
				break;
			}

			default:
				event->ignore();
		}
	}
}





/**
* Overrides the virtual paintEvent method of the base class.
* Calls the paintGL() method, then overlays the text using the widget's QPainter
* @param event
*/
void ThreeDWidget::paintEvent(QPaintEvent *event)
{
	paintGL();
	QPainter painter(this);

	if(m_iView==GLMIAREXVIEW)
	{
		QMiarex* pMiarex = (QMiarex*)s_pMiarex;
		if(pMiarex->m_bResetTextLegend) pMiarex->drawTextLegend();

		painter.drawPixmap(0,0, pMiarex->m_PixText);
		painter.drawPixmap(0,0, m_PixText);
		m_PixText.fill(Qt::transparent);
	}
	else if(m_iView == GLBODYVIEW)
	{
		GL3dBodyDlg *pDlg = (GL3dBodyDlg*)m_pParent;
		pDlg->PaintBodyLegend(painter);
	}
	else if(m_iView == GLWINGVIEW)
	{
		painter.drawPixmap(0,0, m_PixText);
		m_PixText.fill(Qt::transparent);
	}
	else if(m_iView == GLPLANEVIEW)
	{
		EditPlaneDlg *pDlg = (EditPlaneDlg*)m_pParent;
		painter.drawPixmap(0,0, pDlg->m_PixText);
		painter.drawPixmap(0,0, m_PixText);
		m_PixText.fill(Qt::transparent);
	}
	else if(m_iView == GLEDITBODYVIEW)
	{
		EditBodyDlg *pDlg = (EditBodyDlg*)m_pParent;
		painter.drawPixmap(0,0, pDlg->m_PixText);
		painter.drawPixmap(0,0, m_PixText);
		m_PixText.fill(Qt::transparent);
	}
	event->accept();
}



/**
*Overrides the paintGL method of the base class.
*Dispatches the handling to the active child application.
*/
void ThreeDWidget::paintGL()
{
	if(m_iView==GLMIAREXVIEW)
	{
		QMiarex* pMiarex = (QMiarex*)s_pMiarex;
		pMiarex->GLDraw3D();
		GLRenderView();
	}
	else if(m_iView == GLBODYVIEW)
	{
		GL3dBodyDlg *pDlg = (GL3dBodyDlg*)m_pParent;
		pDlg->GLDraw3D();
		pDlg->GLRenderBody();
	}
	else if(m_iView == GLWINGVIEW)
	{
		GL3dWingDlg *pDlg = (GL3dWingDlg*)m_pParent;
		pDlg->GLDraw3D();
		GLRenderView();
	}
	else if(m_iView == GLPLANEVIEW)
	{
		EditPlaneDlg *pDlg = (EditPlaneDlg*)m_pParent;
		pDlg->GLDraw3D();
		GLRenderView();
	}
	else if(m_iView == GLEDITBODYVIEW)
	{
		EditBodyDlg *pDlg = (EditBodyDlg*)m_pParent;
		pDlg->GLDraw3D();
		GLRenderView();
	}
}


/**
*Overrides the resizeGL method of the base class.
* Sets the GL viewport to fit in the client area.
* Sets the scaling factors for the objects to be drawn in the viewport.
*@param width the width in pixels of the client area
*@param height the height in pixels of the client area
*/
void ThreeDWidget::resizeGL(int width, int height)
{
	double w, h, s;
	m_rCltRect = geometry();
	w = (double)width;
	h = (double)height;
	s = 1.0;


	glLoadIdentity();
	if(w>h)	m_GLViewRect.SetRect(-s, s*h/w, s, -s*h/w);
	else    m_GLViewRect.SetRect(-s*w/h, s, s*w/h, -s);

	m_PixText = m_PixText.scaled(rect().size());

	qreal pixelRatio = 1;
#if QT_VERSION >= 0x050000
	pixelRatio = devicePixelRatio();
#endif

	setupViewPort(geometry().width() * pixelRatio, geometry().height() * pixelRatio);

	GLCreateArcballList(m_ArcBall, 1.0);
	GLCreateUnitSphere();

	if(m_iView == GLMIAREXVIEW)
	{
		if(s_pMiarex)
		{
			QMiarex* pMiarex = (QMiarex*)s_pMiarex;
	//		pMiarex->m_ArcBall.GetMatrix();
			pMiarex->m_bIs3DScaleSet = false;
			pMiarex->m_bResetTextLegend = true;
			pMiarex->Set3DScale();
		}
	}
	else if(m_iView == GLWINGVIEW)
	{
	}
	else if(m_iView == GLBODYVIEW)
	{
		if(m_pParent)
		{
			GL3dBodyDlg *pDlg = (GL3dBodyDlg*)m_pParent;
			pDlg->m_bIs3DScaleSet = false;
			pDlg->SetRectangles();
			pDlg->SetBodyScale();
			pDlg->SetBodyLineScale();
			pDlg->SetFrameScale();
		}
	}
}



void ThreeDWidget::setupViewPort(int width, int height)
{
//	makeCurrent();

	glViewport(0,0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	double s = 1.0;
	glOrtho(-s,s,-(height*s)/width,(height*s)/width,-100.0*s,100.0*s);

	glMatrixMode(GL_MODELVIEW);
}



/**
*Creates the OpenGL List for the ArcBall.
*@param ArcBall the ArcBall object associated to the view 
*@param GLScale the overall scaling factor for the view @deprecated and unused
*/
void ThreeDWidget::GLCreateArcballList(ArcBall &ArcBall, double GLScale)
{
	int row, col, NumAngles, NumCircles;
	double Radius, lat_incr, lon_incr, phi, theta;
//	ArcBall.GetMatrix();
	CVector eye(0.0,0.0,1.0);
	CVector up(0.0,1.0,0.0);
	ArcBall.SetZoom(0.45,eye,up);

	if(!glIsList(ARCBALLLIST))
	{
		glNewList(ARCBALLLIST,GL_COMPILE);
		{
			glColor3d(0.3,0.3,.5);
			glLineWidth(1.0);

			Radius = ArcBall.ab_sphere;

			NumAngles  = 50;
			NumCircles =  6;
			lat_incr =  90.0 / NumAngles;
			lon_incr = 360.0 / NumCircles;

			for (col = 0; col < NumCircles; col++)
			{
				glBegin(GL_LINE_STRIP);
				{
					phi = (col * lon_incr) * PI/180.0;

					for (row = 1; row < NumAngles-1; row++)
					{
						theta = (row * lat_incr) * PI/180.0;
						glVertex3d(Radius*cos(phi)*cos(theta)*GLScale, Radius*sin(theta)*GLScale, Radius*sin(phi)*cos(theta)*GLScale);
					}
				}
				glEnd();
				glBegin(GL_LINE_STRIP);
				{
					phi = (col * lon_incr ) * PI/180.0;

					for (row = 1; row < NumAngles-1; row++)
					{
						theta = -(row * lat_incr) * PI/180.0;
						glVertex3d(Radius*cos(phi)*cos(theta)*GLScale, Radius*sin(theta)*GLScale, Radius*sin(phi)*cos(theta)*GLScale);
					}
				}
				glEnd();
			}


			glBegin(GL_LINE_STRIP);
			{
				theta = 0.;
				for(col=1; col<35; col++)
				{
					phi = (0.0 + (double)col*360.0/72.0) * PI/180.0;
					glVertex3d(Radius * cos(phi) * cos(theta)*GLScale, Radius * sin(theta)*GLScale, Radius * sin(phi) * cos(theta)*GLScale);
				}
			}
			glEnd();

			glBegin(GL_LINE_STRIP);
			{
				theta = 0.;
				for(col=1; col<35; col++)
				{
					phi = (0.0 + (double)col*360.0/72.0) * PI/180.0;
					glVertex3d(Radius * cos(-phi) * cos(theta)*GLScale, Radius * sin(theta)*GLScale, Radius * sin(-phi) * cos(theta)*GLScale);
				}
			}
			glEnd();
		}
		glEndList();
	}

	if(!glIsList(ARCPOINTLIST))
	{
		glNewList(ARCPOINTLIST,GL_COMPILE);
		{
			glColor3d(0.3,0.1,.2);
			glLineWidth(2.0);

			NumAngles  = 10;

			lat_incr = 30.0 / NumAngles;
			lon_incr = 30.0 / NumAngles;

			glBegin(GL_LINE_STRIP);
			{
				phi = 0.0;//longitude

				for (row = -NumAngles; row < NumAngles; row++)
				{
					theta = (row * lat_incr) * PI/180.0;
					glVertex3d(Radius*cos(phi)*cos(theta)*GLScale, Radius*sin(theta)*GLScale, Radius*sin(phi)*cos(theta)*GLScale);
				}
			}
			glEnd();

			glBegin(GL_LINE_STRIP);
			{
				theta = 0.;
				for(col=-NumAngles; col<NumAngles; col++)
				{
					phi = (0.0 + (double)col*30.0/NumAngles) * PI/180.0;
					glVertex3d(Radius * cos(phi) * cos(theta)*GLScale, Radius * sin(theta)*GLScale, Radius * sin(phi) * cos(theta)*GLScale);
				}
			}
			glEnd();
		}
		glEndList();
	}
}



/**
* Draws the axis in the OpenGL view
*@param length the length of the axis
*@param color the color to use to draw the axis
*@param style the index of the style to use to draw the axis
*@param width the width to use to draw the axis
*/
void ThreeDWidget::GLDrawAxes(double length, QColor AxisColor, int AxisStyle, int AxisWidth)
{
	double l = .6*length;

	glLineWidth((GLfloat)(AxisWidth));

	glColor3d(AxisColor.redF(),AxisColor.greenF(),AxisColor.blueF());

	glEnable (GL_LINE_STIPPLE);

	if     (AxisStyle == 1) glLineStipple (1, 0xCFCF);
	else if(AxisStyle == 2) glLineStipple (1, 0x6666);
	else if(AxisStyle == 3) glLineStipple (1, 0xFF18);
	else if(AxisStyle == 4) glLineStipple (1, 0x7E66);
	else                    glLineStipple (1, 0xFFFF);

	// X axis____________
	glBegin(GL_LINES);
		glVertex3d(-l, 0.0, 0.0);
		glVertex3d( l, 0.0, 0.0);
	glEnd();
	//Arrow
	glBegin(GL_LINES);
		glVertex3d( 1.0*l,   0.0,   0.0);
		glVertex3d( 0.98*l,  0.015*l, 0.015*l);
	glEnd();
	glBegin(GL_LINES);
		glVertex3d( 1.0*l,  0.0,    0.0);
		glVertex3d( 0.98*l,-0.015*l,-0.015*l);
	glEnd();
	glDisable (GL_LINE_STIPPLE);
	//XLabel
	GLRenderText( l, 0.015, 0.015, "X");


	// Y axis____________
	glEnable (GL_LINE_STIPPLE);
	glBegin(GL_LINES);
		glVertex3d(0.0, -l, 0.0);
		glVertex3d(0.0,  l, 0.0);
	glEnd();

	//Arrow
	glBegin(GL_LINES);
		glVertex3d( 0.0,     1.0*l,  0.0);
		glVertex3d( 0.015*l, 0.98*l, 0.015*l);
	glEnd();
	glBegin(GL_LINES);
		glVertex3d( 0.0,     1.0*l,  0.0);
		glVertex3d(-0.015*l, 0.98*l,-0.015*l);
	glEnd();
	glDisable (GL_LINE_STIPPLE);
	//Y Label
	GLRenderText( 0.015, l, 0.015, "Y");


	// Z axis____________
	glEnable (GL_LINE_STIPPLE);
	glBegin(GL_LINES);
		glVertex3d(0.0, 0.0, -l);
		glVertex3d(0.0, 0.0,  l);
	glEnd();

	//Arrow
	glBegin(GL_LINES);
		glVertex3d(  0.0,   0.0, 1.0*l);
		glVertex3d( 0.015*l,  0.015*l,  0.98*l);
	glEnd();
	glBegin(GL_LINES);
		glVertex3d( 0.0,    0.0, 1.0*l);
		glVertex3d(-0.015*l, -0.015*l,  0.98*l);
	glEnd();
	glDisable (GL_LINE_STIPPLE);
	//ZLabel
	GLRenderText( 0.015, 0.015, l, "Z");

	glDisable (GL_LINE_STIPPLE);
}



/**
Creates a list for a sphere with unit radius
*/
void ThreeDWidget::GLCreateUnitSphere()
{
	double start_lat, start_lon,lat_incr, lon_incr, R;
	double phi1, phi2, theta1, theta2;
	GLdouble u[3], v[3], w[3], n[3];
	int row, col;

	int NumLongitudes, NumLatitudes;
	NumLongitudes = NumLatitudes = 19;

	glNewList(GLLISTSPHERE, GL_COMPILE);
	{
		glDisable(GL_TEXTURE_2D);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glBegin(GL_TRIANGLES);
//		glColor3d(cr.redF(),cr.greenF(),cr.blueF());

		start_lat = -90;
		start_lon = 0.0;
		R = 1.0;

		lat_incr = 180.0 / NumLatitudes;
		lon_incr = 360.0 / NumLongitudes;

		for (col = 0; col < NumLongitudes; col++)
		{
			phi1 = (start_lon + col * lon_incr) * PI/180.0;
			phi2 = (start_lon + (col + 1) * lon_incr) * PI/180.0;

			for (row = 0; row < NumLatitudes; row++)
			{
				theta1 = (start_lat + row * lat_incr) * PI/180.0;
				theta2 = (start_lat + (row + 1) * lat_incr) * PI/180.0;

				u[0] = R * cos(phi1) * cos(theta1);//x
				u[1] = R * sin(theta1);//y
				u[2] = R * sin(phi1) * cos(theta1);//z

				v[0] = R * cos(phi1) * cos(theta2);//x
				v[1] = R * sin(theta2);//y
				v[2] = R * sin(phi1) * cos(theta2);//z

				w[0] = R * cos(phi2) * cos(theta2);//x
				w[1] = R * sin(theta2);//y
				w[2] = R * sin(phi2) * cos(theta2);//z

				NormalVector(u,v,w,n);

				glNormal3dv(n);
				glVertex3dv(u);
				glVertex3dv(v);
				glVertex3dv(w);

				v[0] = R * cos(phi2) * cos(theta1);//x
				v[1] = R * sin(theta1);//y
				v[2] = R * sin(phi2) * cos(theta1);//z

				NormalVector(u,w,v,n);
				glNormal3dv(n);
				glVertex3dv(u);
				glVertex3dv(w);
				glVertex3dv(v);
			}
		}
		glEnd();

		glDisable(GL_DEPTH_TEST);
	}
	glEndList();
}



void ThreeDWidget::GLRenderView()
{
//	makeCurrent();
	// Clear the viewport
	glFlush();
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



	static GLdouble pts[4];
	pts[0]= 0.0; pts[1]=0.0; pts[2]=-1.0; pts[3]= m_ClipPlanePos;  //x=m_VerticalSplit
	glClipPlane(GL_CLIP_PLANE1, pts);


	glPushMatrix();
	{
		if(m_ClipPlanePos>4.9999) 	glDisable(GL_CLIP_PLANE1);
		else						glEnable(GL_CLIP_PLANE1);

		GLSetupLight(0.0, 1.0);
		glDisable(GL_LIGHTING);
		glDisable(GL_LIGHT0);
		if(m_glLightDlg.isVisible())
		{
			glDisable(GL_LIGHTING);
			glDisable(GL_LIGHT0);
			glPushMatrix();
			{
				glTranslated( m_glLightDlg.s_XLight, m_glLightDlg.s_YLight, m_glLightDlg.s_ZLight);
				double radius = (m_glLightDlg.s_ZLight+2.0)/73.0;
				glColor3d(m_glLightDlg.s_Red, m_glLightDlg.s_Green, m_glLightDlg.s_Blue);
				GLRenderSphere(radius/m_glScaled);
			}
			glPopMatrix();
		}

		glLoadIdentity();
		if(m_bCrossPoint && m_bArcball)
		{
			glPushMatrix();
			{
				m_ArcBall.RotateCrossPoint();
				glRotated(m_ArcBall.angle, m_ArcBall.p.x, m_ArcBall.p.y, m_ArcBall.p.z);
				glCallList(ARCPOINTLIST);
			}
			glPopMatrix();
		}
		if(m_bArcball)
		{
			glPushMatrix();
			{
				m_ArcBall.Rotate();
				glCallList(ARCBALLLIST);
			}
			glPopMatrix();
		}

		m_ArcBall.Rotate();

		glScaled(m_glScaled, m_glScaled, m_glScaled);
		glTranslated(m_glRotCenter.x, m_glRotCenter.y, m_glRotCenter.z);
		if(s_bAxes) GLDrawAxes(1.0, W3dPrefsDlg::s_3DAxisColor, W3dPrefsDlg::s_3DAxisStyle, W3dPrefsDlg::s_3DAxisWidth);

		if(m_iView==GLMIAREXVIEW)
		{
			QMiarex* pMiarex = (QMiarex*)s_pMiarex;
			if(pMiarex->m_iView==XFLR5::W3DVIEW) pMiarex->GLRenderView();
		}

		else if(m_iView == GLWINGVIEW)
		{
			GL3dWingDlg *pDlg = (GL3dWingDlg*)m_pParent;
			pDlg->GLRenderView();
		}
		else if(m_iView == GLPLANEVIEW)
		{
			EditPlaneDlg *pDlg = (EditPlaneDlg*)m_pParent;
			pDlg->GLRenderView();
		}
		else if(m_iView == GLEDITBODYVIEW)
		{
			EditBodyDlg *pDlg = (EditBodyDlg*)m_pParent;
			pDlg->GLRenderView();
		}

		glDisable(GL_LIGHTING);
		glDisable(GL_LIGHT0);
	}
	glPopMatrix();

	glDisable(GL_CLIP_PLANE1);
	glFinish();

}


/**
*Renders a sphere in the viewport. Used to draw the point masses and the light.
*@param radius the sphere's radius, in IS units
*/
void ThreeDWidget::GLRenderSphere(double radius)
{
	if(radius>0)
	{
		glPushMatrix();
		glScaled(radius, radius, radius);
		glCallList(GLLISTSPHERE);
		glScaled(1./radius, 1./radius, 1./radius);
		glPopMatrix();
	}
}


/**
* Calculates two vectors, using the middle point as the common origin
*/
void ThreeDWidget::NormalVector(GLdouble p1[3], GLdouble p2[3],  GLdouble p3[3], GLdouble n[3])
{

	GLdouble v1[3], v2[3], d;
	v1[0] = p3[0] - p1[0];
	v1[1] = p3[1] - p1[1];
	v1[2] = p3[2] - p1[2];
	v2[0] = p3[0] - p2[0];
	v2[1] = p3[1] - p2[1];
	v2[2] = p3[2] - p2[2];

	// calculate the cross product of the two vectors
	n[0] = v1[1] * v2[2] - v2[1] * v1[2];
	n[1] = v1[2] * v2[0] - v2[2] * v1[0];
	n[2] = v1[0] * v2[1] - v2[0] * v1[1];

	// normalize the vector
	d = ( n[0] * n[0] + n[1] * n[1] + n[2] * n[2] );
	// try to catch very small vectors
	if (d < (GLdouble)0.00000001)
	{
		d = (GLdouble)100000000.0;
	}
	else
	{
		d = (GLdouble)1.0 / sqrt(d);
	}

	n[0] *= d;
	n[1] *= d;
	n[2] *= d;
}

/**
*Initializes the light parameters of the GL Viewport
*@param pglLightParams a pointer to the instance of the GLLightDlg which holds the user-defined settings for the light.
*@param Offset_y
*@param LightFactor a global factor for all light intensities.
*/
void ThreeDWidget::GLSetupLight(double Offset_y, double LightFactor)
{
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);    // the ambient light
	glDisable(GL_LIGHT1);
	glDisable(GL_LIGHT2);
	glDisable(GL_LIGHT3);
	glDisable(GL_LIGHT4);
	glDisable(GL_LIGHT5);
	glDisable(GL_LIGHT6);
	glDisable(GL_LIGHT7);

	float fLightAmbient0[4];
	float fLightDiffuse0[4];
	float fLightSpecular0[4];
	float fLightPosition0[4];

	if(LightFactor>1.0) LightFactor = 1.0f;

	// the ambient light conditions.
	fLightAmbient0[0] = LightFactor*GLLightDlg::s_Ambient * GLLightDlg::s_Red; // red component
	fLightAmbient0[1] = LightFactor*GLLightDlg::s_Ambient * GLLightDlg::s_Green; // green component
	fLightAmbient0[2] = LightFactor*GLLightDlg::s_Ambient * GLLightDlg::s_Blue; // blue component
	fLightAmbient0[3] = 1.0; // alpha

	fLightDiffuse0[0] = LightFactor*GLLightDlg::s_Diffuse * GLLightDlg::s_Red; // red component
	fLightDiffuse0[1] = LightFactor*GLLightDlg::s_Diffuse * GLLightDlg::s_Green; // green component
	fLightDiffuse0[2] = LightFactor*GLLightDlg::s_Diffuse * GLLightDlg::s_Blue; // blue component
	fLightDiffuse0[3] = 1.0; // alpha

	fLightSpecular0[0] = LightFactor*GLLightDlg::s_Specular * GLLightDlg::s_Red; // red component
	fLightSpecular0[1] = LightFactor*GLLightDlg::s_Specular * GLLightDlg::s_Green; // green component
	fLightSpecular0[2] = LightFactor*GLLightDlg::s_Specular * GLLightDlg::s_Blue; // blue component
	fLightSpecular0[3] = 1.0; // alpha

	// And finally, its position

	fLightPosition0[0] = (GLfloat)((GLLightDlg::s_XLight));
	fLightPosition0[1] = (GLfloat)((GLLightDlg::s_YLight + Offset_y));
	fLightPosition0[2] = (GLfloat)((GLLightDlg::s_ZLight));
	fLightPosition0[3] = 1.0; // W (positional light)


	// Enable the basic light
	glLightfv(GL_LIGHT0, GL_AMBIENT,  fLightAmbient0);
	glLightfv(GL_LIGHT0, GL_DIFFUSE,  fLightDiffuse0);
	glLightfv(GL_LIGHT0, GL_SPECULAR, fLightSpecular0);
	glLightfv(GL_LIGHT0, GL_POSITION, fLightPosition0);


	float fMatAmbient[4]   = {GLLightDlg::s_MatAmbient,  GLLightDlg::s_MatAmbient,   GLLightDlg::s_MatAmbient,  1.0f};
	float fMatSpecular[4]  = {GLLightDlg::s_MatSpecular, GLLightDlg::s_MatSpecular,  GLLightDlg::s_MatSpecular, 1.0f};
	float fMatDiffuse[4]   = {GLLightDlg::s_MatDiffuse,  GLLightDlg::s_MatDiffuse,   GLLightDlg::s_MatDiffuse,  1.0f};
	float fMatEmission[4]  = {GLLightDlg::s_MatEmission, GLLightDlg::s_MatEmission,  GLLightDlg::s_MatEmission, 1.0f};

	if(GLLightDlg::s_bColorMaterial)
	{
		glEnable(GL_COLOR_MATERIAL);
		glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
//		glColorMaterial(GL_FRONT, GL_AMBIENT);
//		glColorMaterial(GL_FRONT, GL_DIFFUSE);
//		glColorMaterial(GL_FRONT, GL_SPECULAR);

	}
	else
	{
		glDisable(GL_COLOR_MATERIAL);

	}
	glMaterialfv(GL_FRONT, GL_SPECULAR,  fMatSpecular);
	glMaterialfv(GL_FRONT, GL_AMBIENT,   fMatAmbient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE,   fMatDiffuse);
	glMaterialfv(GL_FRONT, GL_EMISSION,  fMatEmission);
	glMateriali( GL_FRONT, GL_SHININESS, GLLightDlg::s_iMatShininess);

	if(GLLightDlg::s_bDepthTest)  glEnable(GL_DEPTH_TEST);     else glDisable(GL_DEPTH_TEST);
	if(GLLightDlg::s_bCullFaces)  glEnable(GL_CULL_FACE);      else glDisable(GL_CULL_FACE);
	if(GLLightDlg::s_bSmooth)     glEnable(GL_POLYGON_SMOOTH); else glDisable(GL_POLYGON_SMOOTH);
	if(GLLightDlg::s_bShade)      glShadeModel(GL_SMOOTH);     else glShadeModel(GL_FLAT);

	if(GLLightDlg::s_bLocalView) glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER ,0);
	else                           glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER ,1);

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,0);
}


/**
*Converts screen coordinates to OpenGL Viewport coordinates.
*@param point the screen coordinates.
*@param real the viewport coordinates.
*/
void ThreeDWidget::screenToViewport(QPoint const &point, CVector &real)
{
	//
	// In input, takes the 2D point in screen client area coordinates
	// In output, returns the 2D OpenGL point
	//
	static double h2, w2;
	h2 = (double)geometry().height() /2.0;
	w2 = (double)geometry().width()  /2.0;

	if(w2>h2)
	{
		real.x =  ((double)point.x() - w2) / w2;
		real.y = -((double)point.y() - h2) / w2;
	}
	else
	{
		real.x =  ((double)point.x() - w2) / h2;
		real.y = -((double)point.y() - h2) / h2;
	}
}



/**
*Converts OpenGL Viewport coordinates to screen coordinates
*@param real the viewport coordinates.
*@param point the screen coordinates.
*/
void ThreeDWidget::viewportToScreen(CVector const &real, QPoint &point)
{
	//
	//converts an opengl 2D vector to screen client coordinates
	//
	static double dx, dy, h2, w2;

	h2 = m_GLViewRect.height() /2.0;
	w2 = m_GLViewRect.width()  /2.0;

	dx = ( real.x + w2)/2.0;
	dy = (-real.y + h2)/2.0;

	if(w2>h2)
	{
		point.setX((int)(dx * (double)geometry().width()));
		point.setY((int)(dy * (double)geometry().width()));
	}
	else
	{
		point.setX((int)(dx * (double)geometry().height()));
		point.setY((int)(dy * (double)geometry().height()));
	}
}




/**
*Converts OpenGL Viewport coordinates to client coordinates
*@param x the viewport x-coordinate.
*@param y the viewport y-coordinate.
*@param point the client coordinates.
*/
void ThreeDWidget::viewportToScreen(double const &x, double const &y, QPoint &point)
{
    //
    //converts an opengl 2D vector to screen client coordinates
    //
    static double dx, dy, h2, w2;

    h2 = m_GLViewRect.height() /2.0;
    w2 = m_GLViewRect.width()  /2.0;

    dx = ( x + w2)/2.0;
    dy = (-y + h2)/2.0;

    if(w2>h2)
    {
        point.setX((int)(dx * (double)geometry().width()));
        point.setY((int)(dy * (double)geometry().width()));
    }
    else
    {
        point.setX((int)(dx * (double)geometry().height()));
        point.setY((int)(dy * (double)geometry().height()));
    }
}




void ThreeDWidget::GLInverseMatrix()
{
	//Step 1. Transpose the 3x3 rotation portion of the 4x4 matrix to get the inverse rotation
	int i,j;

	for(i=0 ; i<3; i++)
	{
		for(j=0; j<3; j++)
		{
			MatOut[j][i] = MatIn[i][j];
		}
	}
}


void ThreeDWidget::reset3DRotationCenter()
{
	//adjust the new rotation center after a translation or a rotation

	int i,j;

	for(i=0; i<4; i++)
		for(j=0; j<4; j++)
			MatOut[i][j] =  m_ArcBall.ab_quat[i*4+j];

	m_glRotCenter.x = MatOut[0][0]*(m_glViewportTrans.x) + MatOut[0][1]*(-m_glViewportTrans.y) + MatOut[0][2]*m_glViewportTrans.z;
	m_glRotCenter.y = MatOut[1][0]*(m_glViewportTrans.x) + MatOut[1][1]*(-m_glViewportTrans.y) + MatOut[1][2]*m_glViewportTrans.z;
	m_glRotCenter.z = MatOut[2][0]*(m_glViewportTrans.x) + MatOut[2][1]*(-m_glViewportTrans.y) + MatOut[2][2]*m_glViewportTrans.z;
}


void ThreeDWidget::set3DRotationCenter(QPoint point)
{
	//adjusts the new rotation center after the user has picked a point on the screen
	//finds the closest panel under the point,
	//and changes the rotation vector and viewport translation
	CVector I, A, B, AA, BB, PP, U;

	screenToViewport(point, B);
	B.z = -1.0;
	A.Set(B.x, B.y, +1.0);

	viewportToWorld(A, AA);
	viewportToWorld(B, BB);


	U.Set(BB.x-AA.x, BB.y-AA.y, BB.z-AA.z);
	U.Normalize();

	bool bIntersect = false;

	if(m_iView == GLWINGVIEW)
	{
		GL3dWingDlg *pDlg = (GL3dWingDlg*)m_pParent;
		if(pDlg->IntersectObject(AA, U, I))
		{
			bIntersect = true;
			PP.Set(I);
		}
	}
	else if(m_iView == GLPLANEVIEW)
	{
		EditPlaneDlg *pDlg = (EditPlaneDlg*)m_pParent;
		if(pDlg->IntersectObject(AA, U, I))
		{
			bIntersect = true;
			PP.Set(I);
		}
	}
	else if(m_iView == GLEDITBODYVIEW)
	{
		EditBodyDlg *pDlg = (EditBodyDlg*)m_pParent;
		if(pDlg->IntersectObject(AA, U, I))
		{
			bIntersect = true;
			PP.Set(I);
		}
	}
	else if(m_iView == GLMIAREXVIEW)
	{
		QMiarex *pMiarex = (QMiarex*)s_pMiarex;
		if(pMiarex->IntersectObject(AA, U, I))
		{
			bIntersect = true;
			PP.Set(I);
		}
	}

	if(bIntersect)
	{
//		instantaneous visual transition
//		m_glRotCenter -= PP * m_glScaled;

//		smooth visual transition
		GLInverseMatrix();

		U.x = (-PP.x -m_glRotCenter.x)/30.0;
		U.y = (-PP.y -m_glRotCenter.y)/30.0;
		U.z = (-PP.z -m_glRotCenter.z)/30.0;

		for(int i=0; i<30; i++)
		{
			m_glRotCenter +=U;
			m_glViewportTrans.x =  (MatOut[0][0]*m_glRotCenter.x + MatOut[0][1]*m_glRotCenter.y + MatOut[0][2]*m_glRotCenter.z);
			m_glViewportTrans.y = -(MatOut[1][0]*m_glRotCenter.x + MatOut[1][1]*m_glRotCenter.y + MatOut[1][2]*m_glRotCenter.z);
			m_glViewportTrans.z=   (MatOut[2][0]*m_glRotCenter.x + MatOut[2][1]*m_glRotCenter.y + MatOut[2][2]*m_glRotCenter.z);

			update();
		}
	}
}



void ThreeDWidget::GLDrawFoils(void *pWingPtr)
{
	int j;
	Foil *pFoil;
	Wing *pWing = (Wing*)pWingPtr;

	for(j=0; j<pWing->m_Surface.size(); j++)
	{
		pFoil = pWing->m_Surface.at(j)->foilA();

		if(pFoil) GLRenderText(pWing->m_Surface.at(j)->m_TA.x, pWing->m_Surface.at(j)->m_TA.y, pWing->m_Surface.at(j)->m_TA.z,
							   pFoil->foilName(),
							   QColor(Qt::yellow).lighter(175));

	}

	j = pWing->m_Surface.size()-1;
	pFoil = pWing->m_Surface.at(j)->foilB();
	if(pFoil) GLRenderText(pWing->m_Surface.at(j)->m_TB.x, pWing->m_Surface.at(j)->m_TB.y, pWing->m_Surface.at(j)->m_TB.z,
						 pFoil->foilName(),
						 QColor(Qt::yellow).lighter(175));
}



void ThreeDWidget::GLDrawMasses(double volumeMass, CVector pos, QString tag, QList<PointMass*> ptMasses)
{
	if(qAbs(volumeMass)>PRECISION)
	{
		glPushMatrix();
		{
			glTranslated(pos.x,
						 pos.y,
						 pos.z);
			GLRenderText(0.0,0.0,0.0, tag + QString(" (%1").arg(volumeMass*Units::kgtoUnit(), 0,'g',3) + Units::weightUnitLabel()+")", W3dPrefsDlg::s_MassColor.lighter(125));
		}
		glPopMatrix();
	}

	for(int im=0; im<ptMasses.size(); im++)
	{
		glPushMatrix();
		{
			glTranslated(ptMasses[im]->position().x,
						 ptMasses[im]->position().y,
						 ptMasses[im]->position().z);
			glColor3d(W3dPrefsDlg::s_MassColor.redF(), W3dPrefsDlg::s_MassColor.greenF(), W3dPrefsDlg::s_MassColor.blueF());
			GLRenderSphere(W3dPrefsDlg::s_MassRadius/m_glScaled);
			GLRenderText(0.0, 0.0, 0.0 +.02/m_glScaled,
						 ptMasses[im]->tag()+QString(" (%1").arg(ptMasses[im]->mass()*Units::kgtoUnit(), 0,'g',3)+Units::weightUnitLabel()+")", W3dPrefsDlg::s_MassColor.lighter(125));
		}
		glPopMatrix();
	}
}



void ThreeDWidget::onSurfaces(bool bChecked)
{
	ThreeDWidget::s_bSurfaces = bChecked;
	update();
}


void ThreeDWidget::onOutline(bool bChecked)
{
	ThreeDWidget::s_bOutline = bChecked;
	update();
}


void ThreeDWidget::onPanels(bool bChecked)
{
	ThreeDWidget::s_bVLMPanels = bChecked;
	update();
}

void ThreeDWidget::onAxes(bool bChecked)
{
	ThreeDWidget::s_bAxes = bChecked;
	update();

}


void ThreeDWidget::onFoilNames(bool bChecked)
{
	ThreeDWidget::s_bFoilNames = bChecked;
	update();

}



void ThreeDWidget::onShowMasses(bool bChecked)
{
	ThreeDWidget::s_bShowMasses = bChecked;
	update();

}


void ThreeDWidget::on3DIso()
{
	m_ArcBall.ab_quat[0]	= -0.65987748f;
	m_ArcBall.ab_quat[1]	=  0.38526487f;
	m_ArcBall.ab_quat[2]	= -0.64508355f;
	m_ArcBall.ab_quat[3]	=  0.0f;
	m_ArcBall.ab_quat[4]	= -0.75137258f;
	m_ArcBall.ab_quat[5]	= -0.33720365f;
	m_ArcBall.ab_quat[6]	=  0.56721509f;
	m_ArcBall.ab_quat[7]	=  0.0f;
	m_ArcBall.ab_quat[8]	=  0.000f;
	m_ArcBall.ab_quat[9]	=  0.85899049f;
	m_ArcBall.ab_quat[10]	=  0.51199043f;
	m_ArcBall.ab_quat[11]	=  0.0f;
	m_ArcBall.ab_quat[12]	=  0.0f;
	m_ArcBall.ab_quat[13]	=  0.0f;
	m_ArcBall.ab_quat[14]	=  0.0f;
	m_ArcBall.ab_quat[15]	=  1.0f;

	reset3DRotationCenter();
	update();
}



void ThreeDWidget::on3DTop()
{
	m_ArcBall.SetQuat(sqrt(2.0)/2.0, 0.0, 0.0, -sqrt(2.0)/2.0);
	reset3DRotationCenter();
	update();
}


void ThreeDWidget::on3DLeft()
{
	m_ArcBall.SetQuat(sqrt(2.0)/2.0, -sqrt(2.0)/2.0, 0.0, 0.0);// rotate by 90° around x
	reset3DRotationCenter();
	update();
}


void ThreeDWidget::on3DFront()
{
	Quaternion Qt1(sqrt(2.0)/2.0, 0.0,           -sqrt(2.0)/2.0, 0.0);// rotate by 90° around y
	Quaternion Qt2(sqrt(2.0)/2.0, -sqrt(2.0)/2.0, 0.0,           0.0);// rotate by 90° around x

	m_ArcBall.SetQuat(Qt1 * Qt2);
	reset3DRotationCenter();
	update();
}


void ThreeDWidget::on3DReset()
{
	m_glViewportTrans.Set(0.0, 0.0, 0.0);

	reset3DRotationCenter();
	update();
}



QSize ThreeDWidget::sizeHint() const
{
	return QSize(640, 480);
}




QSize ThreeDWidget::minimumSizeHint() const
{
	return QSize(250, 200);
}



void ThreeDWidget::GLRenderText(double x, double y, double z, const QString & str, QColor textColor)
{
	QPoint point;
	double Vx, Vy;

	if(z>m_ClipPlanePos) return;

	worldToViewport(CVector(x,y,z), Vx, Vy);
	viewportToScreen(Vx, Vy, point);

	QPainter paint(&m_PixText);
	paint.save();
	QPen textPen(textColor);
	paint.setPen(textPen);
	paint.drawText(point, str);
	paint.restore();
}




void ThreeDWidget::GLRenderText(int x, int y, const QString & str, QColor textColor)
{
	QPainter paint(&m_PixText);
	paint.save();
	QPen textPen(textColor);
	paint.setPen(textPen);

	paint.drawText(x,y, str);
	paint.restore();
}




void ThreeDWidget::viewportToWorld(CVector vp, CVector &w)
{
	//un-translate
	vp.x += - m_glViewportTrans.x*m_glScaled;
	vp.y += + m_glViewportTrans.y*m_glScaled;

	//un-scale
	vp.x *= 1.0/m_glScaled;
	vp.y *= 1.0/m_glScaled;
	vp.z *= 1.0/m_glScaled;


	//un-rotate
	w.x = m_ArcBall.ab_quat[0]*vp.x + m_ArcBall.ab_quat[1]*vp.y + m_ArcBall.ab_quat[2]*vp.z;
	w.y = m_ArcBall.ab_quat[4]*vp.x + m_ArcBall.ab_quat[5]*vp.y + m_ArcBall.ab_quat[6]*vp.z;
	w.z = m_ArcBall.ab_quat[8]*vp.x + m_ArcBall.ab_quat[9]*vp.y + m_ArcBall.ab_quat[10]*vp.z;
}





void ThreeDWidget::worldToViewport(CVector const &V, double &Vx, double &Vy)
{
	float m_modelViewMat[16], m_projMat[16];
	float m_V[16];
	m_V[0] = (float)V.x;
	m_V[1] = (float)V.y;
	m_V[2] = (float)V.z;
	m_V[3] = 1.0f;

	glGetFloatv(GL_MODELVIEW_MATRIX, m_modelViewMat);
	glGetFloatv(GL_PROJECTION_MATRIX, m_projMat);

	glPushMatrix();
	{
		glLoadMatrixf(m_projMat);
		glMultMatrixf(m_modelViewMat);
		glMultMatrixf(m_V);
		glGetFloatv(GL_MODELVIEW_MATRIX,  m_V);
	}
	glPopMatrix();
	Vx = m_V[0];
	Vy = m_V[1];

	if(height()<width())  Vy *= (double)height() / (double)width();
	else                  Vx *= (double)width()  / (double)height();
}




void ThreeDWidget::setScale(double refLength)
{
	m_glScaled = 1.5/refLength;
}





/**
* The user has modified the position of the clip plane in the 3D view
*@param pos the new z position in viewport coordinates of the clipping plane
*/
void ThreeDWidget::onClipPlane(int pos)
{
	double planepos =  (double)pos/100.0;
	m_ClipPlanePos = sinh(planepos) * 0.5;
	update();
}



QString ThreeDWidget::GLError()
{
	GLenum err = glGetError();
	QString strange;
	switch(err)
	{
		case GL_NO_ERROR:
		strange = "GL_NO_ERROR: No error has been recorded. The value of this symbolic constant is guaranteed to be 0.";
		break;

		case GL_INVALID_ENUM:
			strange = "GL_INVALID_ENUM: An unacceptable value is specified for an enumerated argument. The offending command is ignored and has no other side effect than to set the error flag.";
			break;

		case GL_INVALID_VALUE:
			strange = "GL_INVALID_VALUE: A numeric argument is out of range. The offending command is ignored and has no other side effect than to set the error flag.";
			break;
		case GL_INVALID_OPERATION:
			strange = "GL_INVALID_OPERATION: The specified operation is not allowed in the current state. The offending command is ignored and has no other side effect than to set the error flag.";
			break;

		case GL_INVALID_FRAMEBUFFER_OPERATION:
			strange = "GL_INVALID_FRAMEBUFFER_OPERATION: The framebuffer object is not complete. The offending command is ignored and has no other side effect than to set the error flag.";
			break;

		case GL_OUT_OF_MEMORY:
			strange = "GL_OUT_OF_MEMORY: There is not enough memory left to execute the command. The state of the GL is undefined, except for the state of the error flags, after this error is recorded.";
			break;

		case GL_STACK_UNDERFLOW:
			strange = "GL_STACK_UNDERFLOW: An attempt has been made to perform an operation that would cause an internal stack to underflow.";
			break;

		case GL_STACK_OVERFLOW:
			strange = "GL_STACK_OVERFLOW: An attempt has been made to perform an operation that would cause an internal stack to overflow.";
			break;
	}
	return strange;
}




