/****************************************************************************

	BodyFrameWidget Class
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

#include "Settings.h"
#include "BodyFrameWidget.h"
#include <QPainter>
#include <QtDebug>


BodyFrameWidget::BodyFrameWidget(QWidget *pParent, Body *pBody)
	:Section2dWidget(pParent)
{
	m_pBody = pBody;
}


void BodyFrameWidget::setScale()
{
	if(!m_pBody)
	{
		//scale is set by user zooming
		m_fRefScale = (double)rect().width();
		m_fScale = m_fRefScale;
	}
	else
	{
		m_fRefScale = ((double)rect().width())/(m_pBody->Length()/15.0);
		m_fScale = m_fRefScale;

	}

	m_ptOffset.rx() = rect().width()/2;
	m_ptOffset.ry() = rect().height()/2;

	m_ViewportTrans = QPoint(0,0);
}


/**
*Overrides the resizeEvent function of the base class.
*Dispatches the handling to the active child application.
*/
void BodyFrameWidget::resizeEvent (QResizeEvent *event)
{
	setScale();
}



void BodyFrameWidget::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	painter.save();
	painter.fillRect(rect(), Settings::s_BackgroundColor);

	drawScaleLegend(painter);
	drawBackImage(painter);

	paintGrids(painter);
	drawFrameLines();
	drawFramePoints();

	painter.restore();
}




void BodyFrameWidget::drawFrameLines()
{
	if(!m_pBody) return;

	int k;
	CVector Point;
	double hinc, u, v;
	int nh;

	QPainter painter(this);
	painter.save();
	nh = 23;
//	xinc = 0.1;
	hinc = 1.0/(double)(nh-1);

	QPen framePen(m_pBody->bodyColor());
	framePen.setWidth(2);
	painter.setPen(framePen);

	QPolygonF rightPolyline, leftPolyline;

	if(m_pBody->m_LineType ==XFLR5::BODYSPLINETYPE)
	{
		if(m_pBody->activeFrame())
		{
			u = m_pBody->getu(m_pBody->activeFrame()->m_Position.x);

			v = 0.0;
			for (k=0; k<nh; k++)
			{
				m_pBody->getPoint(u,v,true, Point);
				rightPolyline.append(QPointF(Point.y*m_fScale+m_ptOffset.x(), Point.z* -m_fScale + m_ptOffset.y()));
				leftPolyline.append(QPointF(-Point.y*m_fScale+m_ptOffset.x(), Point.z* -m_fScale + m_ptOffset.y()));
				v += hinc;
			}
		}
	}

	painter.drawPolyline(rightPolyline);
	painter.drawPolyline(leftPolyline);

	framePen.setStyle(Qt::DashLine);
	framePen.setWidth(1);
	painter.setPen(framePen);

	for(int j=0; j<m_pBody->frameSize(); j++)
	{
		if(m_pBody->frame(j)!=m_pBody->activeFrame())
		{
			rightPolyline.clear();
			leftPolyline.clear();

			if(m_pBody->m_LineType ==XFLR5::BODYSPLINETYPE)
			{
				u = m_pBody->getu(m_pBody->frame(j)->m_Position.x);

				v = 0.0;
				for (k=0; k<nh; k++)
				{
					m_pBody->getPoint(u,v,true, Point);
					rightPolyline.append(QPointF(Point.y*m_fScale+m_ptOffset.x(), Point.z* -m_fScale + m_ptOffset.y()));
					leftPolyline.append(QPointF(-Point.y*m_fScale+m_ptOffset.x(), Point.z* -m_fScale + m_ptOffset.y()));
					v += hinc;
				}

			}
			else
			{

				for (k=0; k<m_pBody->sideLineCount();k++)
				{
					rightPolyline.append(QPointF( m_pBody->frame(j)->m_CtrlPoint[k].y*m_fScale+m_ptOffset.x(), m_pBody->frame(j)->m_CtrlPoint[k].z* -m_fScale + m_ptOffset.y()));
					leftPolyline.append( QPointF(-m_pBody->frame(j)->m_CtrlPoint[k].y*m_fScale+m_ptOffset.x(), m_pBody->frame(j)->m_CtrlPoint[k].z* -m_fScale + m_ptOffset.y()));
				}
			}

			painter.drawPolyline(rightPolyline);
			painter.drawPolyline(leftPolyline);
		}
	}
	painter.restore();
}




void BodyFrameWidget::drawFramePoints()
{
	if(!m_pBody->activeFrame()) return;

	Frame *m_pFrame = m_pBody->activeFrame();
	QPainter painter(this);
	painter.save();

	QPen pointPen(m_pBody->bodyColor());


	for (int k=0; k<m_pFrame->PointCount();k++)
	{
		if(Frame::s_iSelect==k)
		{
			pointPen.setWidth(4);
			pointPen.setColor(Qt::red);
		}
		else if(Frame::s_iHighlight==k)
		{
			pointPen.setWidth(4);
			pointPen.setColor(m_pBody->bodyColor().lighter());
		}
		else
		{
			pointPen.setWidth(2);
			pointPen.setColor(m_pBody->bodyColor());
		}

		painter.setPen(pointPen);
		QRectF rectF( m_pFrame->m_CtrlPoint[k].y *  m_fScale -3 +m_ptOffset.x(),
					  m_pFrame->m_CtrlPoint[k].z * -m_fScale -3 +m_ptOffset.y(),
					  7,7);
		painter.drawEllipse(rectF);
	}
	painter.restore();
}



void BodyFrameWidget::setBody(Body *pBody)
{
	m_pBody = pBody;
	setScale();
}



void BodyFrameWidget::onInsertPt()
{
	CVector Real = mousetoReal(m_PointDown);
	if(Real.y<0.0) return;
	if(m_pBody->activeFrame())
	{
		m_pBody->InsertPoint(Real);
		emit objectModified();
	}
}


void BodyFrameWidget::onRemovePt()
{

	if(m_pBody->activeFrame())
	{
		CVector Real = mousetoReal(m_PointDown);

		int n =   m_pBody->activeFrame()->isPoint(Real, m_fScale/m_fRefScale);
		if (n>=0)
		{
			for (int i=0; i<m_pBody->frameSize();i++)
			{
				m_pBody->frame(i)->removePoint(n);
			}
			m_pBody->setNURBSKnots();
			emit objectModified();
		}
	}
}



int BodyFrameWidget::highlightPoint(CVector real)
{
	if(!m_pBody->activeFrame()) Frame::s_iHighlight = -1;
	else
	{
		real.z = real.y;
		real.y = real.x;
		real.x = m_pBody->activeFrame()->position().x;
		Frame::s_iHighlight =   m_pBody->activeFrame()->isPoint(real, m_fScale/m_fRefScale);
	}
	return Frame::s_iHighlight;
}



int BodyFrameWidget::selectPoint(CVector real)
{
	if(!m_pBody->activeFrame()) Frame::s_iSelect = -1;
	else
	{
		real.z = real.y;
		real.y = real.x;
		real.x = m_pBody->activeFrame()->position().x;
		Frame::s_iSelect =   m_pBody->activeFrame()->isPoint(real, m_fScale/m_fRefScale);
	}
	return Frame::s_iSelect;
}



void BodyFrameWidget::dragSelectedPoint(double x, double y)
{
	if (!m_pBody->activeFrame() || (Frame::s_iSelect<0) ||  (Frame::s_iSelect > m_pBody->activeFrame()->PointCount())) return;

	m_pBody->activeFrame()->selectedPoint().set(m_pBody->activeFrame()->position().x, qMax(x,0.0), y);

}







