/****************************************************************************

	GL3Widget Class
	Copyright (C) 2016 Andre Deperrois adeperrois@xflr5.com

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

#include <QOpenGLShaderProgram>
#include <QMouseEvent>
#include "gl3widget.h"
#include "CVector.h"
#include "Settings.h"

GL3Widget::GL3Widget(QWidget *pParent) : QOpenGLWidget(pParent)
{
	m_clearColor = Qt::black;
	m_xRot= 0;
	m_yRot= 0;
	m_zRot= 0;
	m_glScaled = m_glScaledRef = 1.0f;

	m_bArcball = m_bCrossPoint = true;
	arcBallArray = arcPointArray = NULL;
}


GL3Widget::~GL3Widget()
{
	if(arcBallArray)  delete[] arcBallArray;
	if(arcPointArray) delete[] arcPointArray;

	// release all OpenGL resources.
	makeCurrent();
	m_vboArcball.destroy();
	doneCurrent();
}



void GL3Widget::mousePressEvent(QMouseEvent *event)
{
	m_lastPos = event->pos();
}



void GL3Widget::mouseMoveEvent(QMouseEvent *event)
{
	int dx = event->x() - m_lastPos.x();
	int dy = event->y() - m_lastPos.y();

	if (event->buttons() & Qt::LeftButton)
	{
		rotateBy( dy,  dx, 0);
	}
	else if (event->buttons() & Qt::RightButton)
	{
		rotateBy( dy, 0,  dx);
	}
	m_lastPos = event->pos();
}



void GL3Widget::mouseReleaseEvent(QMouseEvent * /* event */)
{
	emit clicked();
}



/**
*Overrides the wheelEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void GL3Widget::wheelEvent(QWheelEvent *event)
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
	m_glScaled *= zoomFactor;
	update();

/*	if(m_iView == GLMIAREXVIEW)
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
	}*/
}

void GL3Widget::rotateOneStep()
{
	int xRot = 3, yRot = 2, zRot = 1;
	rotateBy(xRot, yRot, zRot);
}



void GL3Widget::resizeGL(int width, int height)
{
	int side = qMin(width, height);
	glViewport((width - side) / 2, (height - side) / 2, side, side);
}



static GLfloat const x_axis[] = {
	-1.0f, 0.0f, 0.0f,
	 1.0f, 0.0f, 0.0f,
	 1.0f,   0.0f,   0.0f,
	 0.95f,  0.025f, 0.025f,
	 1.0f,  0.0f,    0.0f,
	 0.95f,-0.025f,-0.025f
};

static GLfloat const y_axis[] = {
	  0.0f,    -1.0f,    0.0f,
	  0.0f,     1.0f,    0.0f,
	  0.f,      1.0f,    0.0f,
	  0.025f,   0.95f,   0.025f,
	  0.f,      1.0f,    0.0f,
	 -0.025f,   0.95f,  -0.025f
};

static GLfloat const z_axis[] = {
	 0.0f,    0.0f,   -1.0f,
	 0.0f,    0.0f,    1.0f,
	 0.0f,    0.0f,    1.0f,
	 0.025f,  0.025f,  0.95f,
	 0.0f,    0.0f,    1.0f,
	-0.025f, -0.025f,  0.95f
};



void GL3Widget::rotateBy(int xAngle, int yAngle, int zAngle)
{
	m_xRot += xAngle;
	m_yRot += yAngle;
	m_zRot += zAngle;
	update();
}

void GL3Widget::setClearColor(const QColor &color)
{
	m_clearColor = color;
	update();
}

void GL3Widget::initializeGL()
{
//	m_pTimer = new QTimer(this);
//	connect(m_pTimer, SIGNAL(timeout()), this, SLOT(rotateOneStep()));
//	m_pTimer->start(73);

	glMakeArcballArrays();

	m_ShaderProgram.addShaderFromSourceCode(QOpenGLShader::Vertex,
		"attribute highp vec4 vertex;\n"
		"uniform highp mat4 matrix;\n"
		"void main(void)\n"
		"{\n"
		"   gl_Position = matrix * vertex;\n"
		"}");
	m_ShaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment,
		"uniform mediump vec4 color;\n"
		"void main(void)\n"
		"{\n"
		"   gl_FragColor = color;\n"
		"}");
	m_ShaderProgram.link();
	m_ShaderProgram.bind();

	m_VertexLocation = m_ShaderProgram.attributeLocation("vertex");
	m_MatrixLocation = m_ShaderProgram.uniformLocation("matrix");
	m_ColorLocation = m_ShaderProgram.uniformLocation("color");
}



void GL3Widget::paintGL()
{
	glClearColor(m_clearColor.redF(), m_clearColor.greenF(), m_clearColor.blueF(), m_clearColor.alphaF());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	QMatrix4x4 m;
	m.ortho(-0, 120.0, -100, 100, -100, 150.0f);
//	m.ortho(rect());
	m.translate(50.0f, 0.0f, 0.0f);
	m.rotate(m_xRot / 16.0f, 1.0f, 0.0f, 0.0f);
	m.rotate(m_yRot / 16.0f, 0.0f, 1.0f, 0.0f);
	m.rotate(m_zRot / 16.0f, 0.0f, 0.0f, 1.0f);
	m.scale(50.0f*m_glScaled);


	QMatrix4x4 pmvMatrix;
	pmvMatrix.ortho(rect());

	m_ShaderProgram.enableAttributeArray(m_VertexLocation);
	m_ShaderProgram.setUniformValue(m_MatrixLocation, m);

	QColor color(30, 200, 130, 255);
	m_ShaderProgram.setUniformValue(m_ColorLocation, color);


	glEnable (GL_LINE_STIPPLE);
	glLineStipple (1, 0xFF18);

	m_ShaderProgram.setAttributeArray(m_VertexLocation, x_axis, 3);
	glDrawArrays(GL_LINE_STRIP, 0, 6);
	m_ShaderProgram.setAttributeArray(m_VertexLocation, y_axis, 3);
	glDrawArrays(GL_LINE_STRIP, 0, 6);
	m_ShaderProgram.setAttributeArray(m_VertexLocation, z_axis, 3);
	glDrawArrays(GL_LINE_STRIP, 0, 6);

	glDisable (GL_LINE_STIPPLE);

//	drawArcBall();
	paintArcBall();

	m_ShaderProgram.disableAttributeArray(m_VertexLocation);
}


void GL3Widget::paintArcBall()
{
	m_ShaderProgram.enableAttributeArray(m_VertexLocation);
	m_vboArcball.bind();
	m_ShaderProgram.setAttributeBuffer(m_VertexLocation, GL_FLOAT, 0, 3, 0);
	QColor(155,0,0,255);
	m_ShaderProgram.setUniformValue(m_ColorLocation, QColor(155,75,75,255));

	int NumCircles =  6;
	int NumAngles  = 10;
	for (int col=0; col<NumCircles*2; col++)
	{
		glDrawArrays(GL_LINE_STRIP, col*(NumAngles-2), NumAngles-2);
	}
	glDrawArrays(GL_LINE_STRIP, 2*NumCircles*(NumAngles-2),    34);
	glDrawArrays(GL_LINE_STRIP, 2*NumCircles*(NumAngles-2)+34, 34);

	m_vboArcball.release();

	m_ShaderProgram.disableAttributeArray(m_VertexLocation);
}



/**
*Creates the OpenGL List for the ArcBall.
*@param ArcBall the ArcBall object associated to the view
*@param GLScale the overall scaling factor for the view
*/
void GL3Widget::glMakeArcballArrays()
{
	float GLScale = 3.0f;
	int row, col, NumAngles, NumCircles;
	double Radius=0.1, lat_incr, lon_incr, phi, theta;
//	ArcBall.GetMatrix();
	CVector eye(0.0, 0.0, 1.0);
	CVector up(0.0, 1.0, 0.0);
	m_ArcBall.setZoom(0.45, eye, up);

	Radius = m_ArcBall.ab_sphere;

	NumAngles  = 10;
	NumCircles =  6;
	lat_incr =  90.0 / (double)NumAngles;
	lon_incr = 360.0 / (double)NumCircles;

	int iv=0;

	if(arcBallArray!=NULL)  delete [] arcBallArray;
	if(arcPointArray!=NULL) delete [] arcPointArray;

	arcBallArray  = new float[3000];
	arcPointArray = new float[3000];
	//ARCBALL
	for (col=0; col<NumCircles; col++)
	{
		//first
		phi = (col * lon_incr) * PI/180.0;
		for (row=1; row<NumAngles-1; row++)
		{
			theta = (row * lat_incr) * PI/180.0;
			arcBallArray[iv++] = Radius*cos(phi)*cos(theta)*GLScale;
			arcBallArray[iv++] = Radius*sin(theta)*GLScale;
			arcBallArray[iv++] = Radius*sin(phi)*cos(theta)*GLScale;
		}
	}

	for (col=0; col<NumCircles; col++)
	{
		//Second
		phi = (col * lon_incr ) * PI/180.0;
		for (row=1; row<NumAngles-1; row++)
		{
			theta = -(row * lat_incr) * PI/180.0;
			arcBallArray[iv++] = Radius*cos(phi)*cos(theta)*GLScale;
			arcBallArray[iv++] = Radius*sin(theta)*GLScale;
			arcBallArray[iv++] = Radius*sin(phi)*cos(theta)*GLScale;
		}
	}

	theta = 0.;
	for(col=1; col<35; col++)
	{
		phi = (0.0 + (double)col*360.0/72.0) * PI/180.0;
		arcBallArray[iv++] = Radius * cos(phi) * cos(theta)*GLScale;
		arcBallArray[iv++] = Radius * sin(theta)*GLScale;
		arcBallArray[iv++] = Radius * sin(phi) * cos(theta)*GLScale;
	}

	theta = 0.;
	for(col=1; col<35; col++)
	{
		phi = (0.0 + (double)col*360.0/72.0) * PI/180.0;
		arcBallArray[iv++] = Radius * cos(-phi) * cos(theta)*GLScale;
		arcBallArray[iv++] = Radius * sin(theta)*GLScale;
		arcBallArray[iv++] = Radius * sin(-phi) * cos(theta)*GLScale;
	}

	/*
	//ARCPOINT
	NumAngles  = 10;

	lat_incr = 30.0 / NumAngles;
	lon_incr = 30.0 / NumAngles;
	iv=0;

	phi = 0.0;//longitude
	for (row = -NumAngles; row < NumAngles; row++)
	{
		theta = (row * lat_incr) * PI/180.0;
		arcPointArray[iv++] = Radius*cos(phi)*cos(theta)*GLScale;
		arcPointArray[iv++] = Radius*sin(theta)*GLScale;
		arcPointArray[iv++] = Radius*sin(phi)*cos(theta)*GLScale;
	}
	qDebug()<<"firstarcpoint"<<iv;

	theta = 0.;
	for(col=-NumAngles; col<NumAngles; col++)
	{
		phi = (0.0 + (double)col*30.0/NumAngles) * PI/180.0;
		arcPointArray[iv++] = Radius * cos(phi) * cos(theta)*GLScale;
		arcPointArray[iv++] = Radius * sin(theta)*GLScale;
		arcPointArray[iv++] = Radius * sin(phi) * cos(theta)*GLScale;
	}
	qDebug()<<"secarcpoint"<<iv;*/


	m_vboArcball.create();
	m_vboArcball.bind();
	m_vboArcball.allocate(arcBallArray, iv * sizeof(GLfloat));
	m_vboArcball.release();

}




void GL3Widget::drawArcBall()
{
/*	first 576
	sec 1440
	third 1542
	four 1644

	firstpp 60
	secpp 120
	1645 121*/
	QColor color;
	color.setRgb(130, 0, 230);
	m_ShaderProgram.setUniformValue(m_ColorLocation, color);
	m_ShaderProgram.setAttributeArray(m_VertexLocation, arcBallArray, 3);
	glDrawArrays(GL_LINE_STRIP, 0, 8);

/*	color.setRgb(230, 0, 130);
	m_ShaderProgram.setUniformValue(colorLocation, color);

	m_ShaderProgram.setAttributeArray(vertexLocation, arcPointArray, 3);
	glDrawArrays(GL_LINE_STRIP, 0, 60);
	m_ShaderProgram.setAttributeArray(vertexLocation, arcPointArray, 3);
	glDrawArrays(GL_LINE_STRIP, 60, 60);
*/
}
