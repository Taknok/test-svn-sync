/****************************************************************************

	BodyLineWidget Class
	Copyright (C) 2015 Andre Deperrois adeperrois@xflr5.com

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

#include "globals.h"
#include "BodyLineWidget.h"
#include "Settings.h"
#include <QPainter>
#include <QPaintEvent>
#include <QtDebug>


BodyLineWidget::BodyLineWidget(QWidget *pParent, Body *pBody)
		:Section2dWidget(pParent)
{
	m_pBody = pBody;
}


/**
*Overrides the resizeEvent function of the base class.
*Dispatches the handling to the active child application.
*/
void BodyLineWidget::resizeEvent (QResizeEvent *event)
{
	setScale();
	event->accept();
}




void BodyLineWidget::setScale()
{
	if(!m_pBody)
	{
		//scale is set by user zooming
		m_fRefScale = (double)rect().width()-150.0;
		m_fScale = m_fRefScale;
	}
	else
	{
		m_fRefScale = ((double)rect().width()-150.0)/m_pBody->Length();
		m_fScale = m_fRefScale;

	}

	m_ptOffset.rx() = rect().width()/2;
	m_ptOffset.ry() = rect().height()/2;

	m_ViewportTrans = QPoint(0,0);
}


void BodyLineWidget::drawBodyLines()
{
	if(!m_pBody) return;
	QPainter painter(this);
	painter.save();
	int i,k;
	double zpos;


	QPen linePen(m_pBody->m_BodyColor);
	linePen.setWidth(1);
	painter.setPen(linePen);
	linePen.setStyle(Qt::DashLine);

	QPolygonF midLine,topLine, botLine;
	//Middle Line
	for (k=0; k<m_pBody->frameSize();k++)
	{
		zpos = (m_pBody->frame(k)->m_CtrlPoint.first().z +m_pBody->frame(k)->m_CtrlPoint.last().z )/2.0;
		midLine.append(QPointF(m_pBody->frame(k)->m_Position.x*m_fScale + m_ptOffset.x(), zpos*-m_fScale + m_ptOffset.y()));
	}

	if(m_pBody->m_LineType==XFLR5::BODYPANELTYPE)
	{
		//Top Line
		for (k=0; k<m_pBody->frameSize();k++)
		{
			topLine.append(QPointF(m_pBody->frame(k)->m_Position.x*m_fScale + m_ptOffset.x(), m_pBody->frame(k)->m_CtrlPoint[0].z*-m_fScale+ m_ptOffset.y()));
		}

		//Bottom Line
		for (k=0; k<m_pBody->frameSize();k++)
		{
			botLine.append(QPointF(m_pBody->frame(k)->m_Position.x*m_fScale + m_ptOffset.x(),
								   m_pBody->frame(k)->m_CtrlPoint[ m_pBody->frame(k)->PointCount()-1].z*-m_fScale + m_ptOffset.y()));
		}
	}
	else
	{
		CVector Point;
		double xinc, u, v;

		int nh = 50;
		xinc = 1./(double)(nh-1);
		u=0.0; v = 0.0;

		//top line
		u=0.0;
		v = 0.0;
		for (i=0; i<=nh; i++)
		{
			m_pBody->getPoint(u,v,true, Point);
			topLine.append(QPointF(Point.x*m_fScale + m_ptOffset.x(), Point.z*-m_fScale + m_ptOffset.y()));
			u += xinc;
		}

		//bottom line
		u=0.0;
		v = 1.0;
		for (i=0; i<=nh; i++)
		{
			m_pBody->getPoint(u,v,true, Point);
			botLine.append(QPointF(Point.x*m_fScale + m_ptOffset.x(), Point.z*-m_fScale + m_ptOffset.y()));
			u += xinc;
		}
	}

	QRect r(rect());
	painter.drawPolyline(midLine);
	painter.drawPolyline(topLine);
	painter.drawPolyline(botLine);

	painter.restore();
}


void BodyLineWidget::drawBodyPoints()
{
	if(!m_pBody) return;

	QPainter painter(this);
	painter.save();
	QPen pointPen;

	for (int k=0; k<m_pBody->frameSize();k++)
	{
		if(m_pBody->m_iActiveFrame==k)
		{
			pointPen.setWidth(4);
			pointPen.setColor(Qt::red);
		}
		else if(m_pBody->m_iHighlight==k)
		{
			pointPen.setWidth(4);
			pointPen.setColor(m_pBody->m_BodyColor.lighter());
		}
		else
		{
			pointPen.setColor(m_pBody->m_BodyColor);
			pointPen.setWidth(2);
		}

		painter.setPen(pointPen);
		QRectF rectF( m_pBody->frame(k)->m_Position.x * m_fScale - 3 + m_ptOffset.x(),
					 (m_pBody->frame(k)->m_CtrlPoint.first().z + m_pBody->frame(k)->m_CtrlPoint.last().z ) /2.0* -m_fScale -3+ m_ptOffset.y(),
					  7,7);
		painter.drawEllipse(rectF);

	}
	painter.restore();
}


void BodyLineWidget::setBody(Body *pBody)
{
	m_pBody = pBody;
	setScale();
}


void BodyLineWidget::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	painter.save();
	painter.fillRect(rect(), Settings::s_BackgroundColor);

	drawScaleLegend(painter);
	drawBackImage(painter);

	paintGrids(painter);
	drawBodyLines();
	drawBodyPoints();

	painter.restore();
}



void BodyLineWidget::onInsertPt()
{
	CVector Real = mousetoReal(m_PointDown);
	m_pBody->insertFrame(Real);
	emit objectModified();
}


void BodyLineWidget::onRemovePt()
{
	CVector Real = mousetoReal(m_PointDown);

	int n =  m_pBody->isFramePos(CVector(Real.x, 0.0, Real.y), m_fScale/m_fRefScale);
	if (n>=0)
	{
		n = m_pBody->removeFrame(n);
		emit objectModified();
	}
}



int BodyLineWidget::highlightPoint(CVector real)
{
	m_pBody->m_iHighlight = m_pBody->isFramePos(CVector(real.x, 0.0, real.y), m_fScale/m_fRefScale);
	return m_pBody->m_iHighlight;
}



int BodyLineWidget::selectPoint(CVector real)
{
	m_pBody->m_iActiveFrame = m_pBody->isFramePos(CVector(real.x, 0.0, real.y), m_fScale/m_fRefScale);
	m_pBody->setActiveFrame(m_pBody->m_iActiveFrame);
	return m_pBody->m_iActiveFrame;
}


void BodyLineWidget::dragSelectedPoint(double x, double y)
{
	if(!m_pBody->activeFrame())return;
	m_pBody->activeFrame()->setPosition(CVector(x,0,y));
}






