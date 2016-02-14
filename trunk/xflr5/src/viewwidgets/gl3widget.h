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

#ifndef GL3WIDGET_H
#define GL3WIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QTimer>

#include "ArcBall.h"

QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram);
QT_FORWARD_DECLARE_CLASS(QOpenGLTexture)

class GL3Widget : public QOpenGLWidget
{
	Q_OBJECT
public:
	GL3Widget(QWidget *pParent = NULL);
	~GL3Widget();
	void rotateBy(int xAngle, int yAngle, int zAngle);
	void setClearColor(const QColor &color);

signals:
	void clicked();

protected:
	void initializeGL() Q_DECL_OVERRIDE;
	void paintGL() Q_DECL_OVERRIDE;
	void resizeGL(int width, int height) Q_DECL_OVERRIDE;
	void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
	void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
	void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
	void wheelEvent(QWheelEvent *event);

private slots:
	void rotateOneStep();

private:
	void glMakeArcballArrays();
	void drawArcBall();
	void paintArcBall();

	QColor m_clearColor;
	QPoint m_lastPos;
	int m_xRot;
	int m_yRot;
	int m_zRot;
	QOpenGLShaderProgram m_ShaderProgram;
	QOpenGLBuffer m_vboArcball;

	int m_VertexLocation, m_MatrixLocation, m_ColorLocation;

	bool m_bArcball;			//true if the arcball is to be displayed
	bool m_bCrossPoint;			//true if the control point on the arcball is to be displayed
	ArcBall m_ArcBall;
	float *arcBallArray, *arcPointArray;

	double m_glScaled, m_glScaledRef;

};

#endif // GL3WIDGET_H
