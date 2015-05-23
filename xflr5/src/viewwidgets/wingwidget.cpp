/****************************************************************************

	WingWidget Class
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

#include <QPainter>
#include "wingwidget.h"
#include "Settings.h"
#include "miarex/Objects3D.h"
#include "misc/W3dPrefsDlg.h"
#include "miarex/Miarex.h"

#include <QtDebug>
void *WingWidget::s_pMainFrame = NULL;
void *WingWidget::s_pMiarex = NULL;

WingWidget::WingWidget(QWidget *pParent) : QWidget(pParent)
{
	setMouseTracking(true);
	m_bXCP               = false;
	m_bXTop              = false;
	m_bXBot              = false;
	m_bXCmRef            = true;

	m_bTrans = false;
	m_bIs2DScaleSet = false;

	m_pGraph = NULL;

	m_WingScale = 1.0;
	m_ptOffset.rx() = 0;
	m_ptOffset.ry() = 0;
}



WingWidget::~WingWidget()
{
}



void WingWidget::contextMenuEvent (QContextMenuEvent *event)
{
}


void WingWidget::setWingGraph(QGraph *pGraph)
{
	m_pGraph = pGraph;
}


void WingWidget::keyPressEvent(QKeyEvent *event)
{
	bool bShift = false;
	if(event->modifiers() & Qt::ShiftModifier)   bShift =true;

	switch (event->key())
	{
		case Qt::Key_R:
			onResetWingScale();
			event->accept();
			return;


		default:
			QWidget::keyPressEvent(event);
	}

	event->ignore();
}


void WingWidget::mouseDoubleClickEvent (QMouseEvent *event)
{

}


void WingWidget::mouseMoveEvent(QMouseEvent *event)
{
	setFocus();

	// we translate the Plane
	if (event->buttons() & Qt::LeftButton)
	{
		QPointF Delta;
		Delta.setX(event->pos().x() - m_LastPoint.x());
		Delta.setY(event->pos().y() - m_LastPoint.y());
		m_ptOffset.rx() += Delta.x();
		m_ptOffset.ry() += Delta.y();
		m_bIs2DScaleSet = false;
		update();
	}
	m_LastPoint = event->pos();
}


void WingWidget::mousePressEvent(QMouseEvent *event)
{
	if (event->buttons() & Qt::LeftButton)
	{
		m_LastPoint = event->pos();
	}
}


void WingWidget::mouseReleaseEvent(QMouseEvent *event)
{
	m_bTrans = false;
}


void WingWidget::paintEvent(QPaintEvent *event)
{
	QMiarex *pMiarex=(QMiarex*)s_pMiarex;

	QPainter painter(this);
	painter.save();
	painter.fillRect(rect(), Settings::backgroundColor());
	painter.setFont(Settings::s_TextFont);

	QPen TextPen(Settings::s_TextColor);
	TextPen.setWidth(1);
	painter.setPen(TextPen);

	if(pMiarex->m_pCurPlane)
	{
		paintWing(painter, m_ptOffset, m_WingScale);
		if(pMiarex->m_pCurPOpp && pMiarex->m_pCurPOpp->isVisible())
		{
			QPointF PtLegend;
			PtLegend.rx() = rect().width()/2;
			PtLegend.ry() = rect().bottom();
			paintXTr(painter, m_ptOffset, m_WingScale);
			if (m_bXCP)    paintXCP(painter, m_ptOffset, m_WingScale);
			if (m_bXCmRef) paintXCmRef(painter, m_ptOffset, m_WingScale);
		}

		painter.setBackgroundMode(Qt::TransparentMode);
		painter.setOpacity(1);
		painter.drawPixmap(0,0, m_PixText);
	}
	painter.restore();
}



/**
 * Draws the wing in the 2D operating point view
 * @param painter a reference to the QPainter object on which the view shall be drawn
 * @param ORef the origin of the tip of the root chord, in client coordinates
 * @param scale the scaling factor with which to draw the wing
 */
void WingWidget::paintWing(QPainter &painter, QPointF ORef, double scale)
{
	QMiarex *pMiarex=(QMiarex*)s_pMiarex;
	if(!pMiarex->m_pCurPlane)	return;
	static int i;
	static double scalex, scaley;

	scalex  = scale;
	scaley  = scale;

	Wing *pWing = pMiarex->m_pCurPlane->m_Wing;

	painter.save();
	QPen WingPen(W3dPrefsDlg::s_OutlineColor);
	WingPen.setStyle(getStyle(W3dPrefsDlg::s_OutlineStyle));
	WingPen.setWidth(W3dPrefsDlg::s_OutlineWidth);

	painter.setPen(WingPen);

	QPointF O(ORef);

	//Right Wing
	O.rx() = ORef.x();
	O.ry() = ORef.y();
	for (i=0; i<pWing->NWingSection()-1;i++)
	{
		O.rx() +=(int)(pWing->Length(i)*scalex);
		painter.drawLine(O.x(),                                   O.y()+(int)(pWing->Offset(i)*scaley),
						 O.x()+(int)(pWing->Length(i+1)*scalex),  O.y()+(int)(pWing->Offset(i+1)*scaley));

		painter.drawLine(O.x()+(int)(pWing->Length(i+1)*scalex),  O.y()+(int)(pWing->Offset(i+1)*scaley),
						 O.x()+(int)(pWing->Length(i+1)*scalex),  O.y()+(int)((pWing->Offset(i+1)+pWing->Chord(i+1))*scaley));

		painter.drawLine(O.x()+(int)(pWing->Length(i+1)*scalex),  O.y()+(int)((pWing->Offset(i+1)+pWing->Chord(i+1))*scaley),
						 O.x(),                                   O.y() +(int)((pWing->Offset(i)+pWing->Chord(i))*scaley));

		painter.drawLine(O.x(),                                   O.y() +(int)((pWing->Offset(i)+pWing->Chord(i))*scaley),
						 O.x(),                                   O.y()+(int)(pWing->Offset(i)*scaley));
	}


	//LeftWing
	O.rx() = ORef.x();
	O.ry() = ORef.y();

	for (i=0; i<pWing->NWingSection()-1;i++)
	{
		O.rx() -= (int)(pWing->Length(i)*scalex);
		painter.drawLine(O.x(),						                     O.y()+(int)(pWing->Offset(i)*scaley),
						 O.x()-(int)(pWing->Length(i+1)*scalex), O.y()+(int)(pWing->Offset(i+1)*scaley));

		painter.drawLine(O.x()-(int)(pWing->Length(i+1)*scalex), O.y()+(int)(pWing->Offset(i+1)*scaley),
						 O.x()-(int)(pWing->Length(i+1)*scalex), O.y()+(int)((pWing->Offset(i+1)+pWing->Chord(i+1))*scaley));

		painter.drawLine(O.x()-(int)(pWing->Length(i+1)*scalex), O.y()+(int)((pWing->Offset(i+1)+pWing->Chord(i+1))*scaley),
						 O.x(),                                        O.y() +(int)((pWing->Offset(i)+pWing->Chord(i))*scaley));

		painter.drawLine(O.x(),                                        O.y() +(int)((pWing->Offset(i)+pWing->Chord(i))*scaley),
						 O.x(),                                        O.y()+(int)(pWing->Offset(i)*scaley));
	}


	QPen SymPen(QColor(155,128,190));
	painter.setPen(SymPen);
	painter.setBackgroundMode(Qt::TransparentMode);

	painter.drawLine(ORef.x(), ORef.y()-20, ORef.x(), ORef.y()+75);
	painter.restore();
}



/**
 * Draws the position of the reference point for the moments in the operating view
 * @param painter a reference to the QPainter object on which the view shall be drawn
 * @param ORef the origin of the tip of the root chord, in client coordinates
 * @param scale the scaling factor with which to draw the wing
 */
void WingWidget::paintXCmRef(QPainter & painter, QPointF ORef, double scale)
{
	//Draws the moment reference point on the 2D view
	QMiarex *pMiarex=(QMiarex*)s_pMiarex;
	if(!pMiarex->m_pCurPlane || !pMiarex->m_pCurWPolar)	return;

	painter.save();
	QPointF O(ORef);
	QPointF offset;

	double scaley;

	offset.rx() = ORef.x();
	offset.ry() = ORef.y();
//	scalex  = scale;
	scaley  = scale;
	O.rx() = offset.x();
	O.ry() = offset.y();

	QPen XCmRefPen(Settings::s_TextColor);
	painter.setPen(XCmRefPen);

	double XCm = O.x() ;
	double YCm = O.y() + pMiarex->m_pCurWPolar->CoG().x*scaley;
	int size = 3;
	QRect CM(XCm-size, YCm-size, 2*size, 2*size);
	painter.drawEllipse(CM);

	painter.drawText(XCm+10, YCm-5, tr("Moment ref. location"));

	painter.restore();
}





/**
 * Draws the lift line and the position of the center of pressure in the operating view
 * @param painter a reference to the QPainter object on which the view shall be drawn
 * @param ORef the origin of the tip of the root chord, in client coordinates
 * @param scale the scaling factor with which to draw the wing
 */
void WingWidget::paintXCP(QPainter & painter, QPointF ORef, double scale)
{
	//Draws the lift line and center of pressure position on the the 2D view
	QMiarex *pMiarex = (QMiarex*)s_pMiarex;
	if(!pMiarex->m_pCurPlane)	return;

	Wing *pWing = pMiarex->m_pCurPlane->m_Wing;

	painter.save();

	QPointF From, To;

	double y;
	int nStart;
	double offLE;
	if(pMiarex->m_pCurPOpp->analysisMethod()==XFLR5::LLTMETHOD) nStart = 1; else nStart = 0;

	QPointF O(ORef);
	QPointF offset;

	double scalex, scaley;
	offset.rx() = ORef.x();
	offset.ry() = ORef.y();
	scalex  = scale;
	scaley  = scale;
	O.rx() = offset.x();
	O.ry() = offset.y();

	QPen XCPPen(W3dPrefsDlg::s_XCPColor);
	XCPPen.setWidth(W3dPrefsDlg::s_XCPWidth);
	XCPPen.setStyle(getStyle(W3dPrefsDlg::s_XCPStyle));
	painter.setPen(XCPPen);

	double XCp = O.x() + pMiarex->m_pCurPOpp->m_pPlaneWOpp[0]->m_CP.y*scalex;
	double YCp = O.y() + pMiarex->m_pCurPOpp->m_pPlaneWOpp[0]->m_CP.x*scaley;
//    int ZCp = O.z() + (int)(pMiarex->m_pCurPOpp->m_pPlaneWOpp[0]->m_ZCP*scalez);
	int size = 3;
	QRect CP(XCp-size, YCp-size, 2*size, 2*size);
	painter.drawEllipse(CP);

	offLE = pWing->getOffset(pMiarex->m_pCurPOpp->m_pPlaneWOpp[0]->m_SpanPos[nStart]*2.0/pMiarex->m_pCurPOpp->m_pPlaneWOpp[0]->m_Span);
	y = (offLE+pMiarex->m_pCurPOpp->m_pPlaneWOpp[0]->m_Chord[nStart]*pMiarex->m_pCurPOpp->m_pPlaneWOpp[0]->m_XCPSpanRel[nStart])*scaley;
	From = QPoint(O.x()+pMiarex->m_pCurPOpp->m_pPlaneWOpp[0]->m_SpanPos[nStart]*scalex,	O.y()+y );

	for (int m=nStart; m<pMiarex->m_pCurPOpp->m_pPlaneWOpp[0]->m_NStation; m++)
	{
		offLE = pWing->getOffset(pMiarex->m_pCurPOpp->m_pPlaneWOpp[0]->m_SpanPos[m]*2.0/pMiarex->m_pCurPOpp->m_pPlaneWOpp[0]->m_Span);
		y = (offLE+pMiarex->m_pCurPOpp->m_pPlaneWOpp[0]->m_Chord[m]*pMiarex->m_pCurPOpp->m_pPlaneWOpp[0]->m_XCPSpanRel[m])*scaley;
		To = QPoint(O.x()+pMiarex->m_pCurPOpp->m_pPlaneWOpp[0]->m_SpanPos[m]*scalex,	O.y()+y );
		painter.drawLine(From, To);
		From = To;
	}

	int x = (int)(rect().width()/2);
	int y1 = rect().bottom();
	painter.drawLine(x-60,  y1- 20, x-40,  y1 - 20);
	painter.drawText(x-35, y1 - 18, tr("Centre of Pressure"));
	painter.restore();
}



/**
 * Draws the laminar to turbulent translition lines in the operating view
 * @param painter a reference to the QPainter object on which the view shall be drawn
 * @param ORef the origin of the tip of the root chord, in client coordinates
 * @param scale the scaling factor with which to draw the wing
 */
void WingWidget::paintXTr(QPainter & painter, QPointF ORef, double scale)
{
	//Draws the transition lines on the 2D view
	QMiarex *pMiarex = (QMiarex*)s_pMiarex;
	if(!pMiarex->m_pCurPlane)	return;

	Wing *pWing = pMiarex->m_pCurPlane->wing();
	painter.save();

	double y;
	int m,nStart;
	if(pMiarex->m_pCurPOpp->analysisMethod()==XFLR5::LLTMETHOD) nStart = 1; else nStart = 0;

	QPointF O(ORef);
	QPointF offset, From, To;

	double offLE;
	double scalex, scaley;
	offset.rx() = ORef.x();
	offset.ry() = ORef.y();
	scalex  = scale;
	scaley  = scale;

	O.rx() = offset.x();
	O.ry() = offset.y();

	QPen TopPen(W3dPrefsDlg::s_TopColor);
	TopPen.setStyle(getStyle(W3dPrefsDlg::s_TopStyle));
	TopPen.setWidth(W3dPrefsDlg::s_TopWidth);
	painter.setPen(TopPen);

	if (m_bXTop)
	{
		offLE = pWing->getOffset(pMiarex->m_pCurPOpp->m_pPlaneWOpp[0]->m_SpanPos[nStart]*2.0/pMiarex->m_pCurPOpp->m_pPlaneWOpp[0]->m_Span);
		y = (offLE+pMiarex->m_pCurPOpp->m_pPlaneWOpp[0]->m_Chord[nStart]*pMiarex->m_pCurPOpp->m_pPlaneWOpp[0]->m_XTrTop[nStart])*scaley;
		From = QPoint(O.x()+pMiarex->m_pCurPOpp->m_pPlaneWOpp[0]->m_SpanPos[nStart]*scalex,	O.y()+y);

		for (m=nStart; m<pMiarex->m_pCurPOpp->m_pPlaneWOpp[0]->m_NStation; m++)
		{
			offLE = pWing->getOffset(pMiarex->m_pCurPOpp->m_pPlaneWOpp[0]->m_SpanPos[m]*2.0/pMiarex->m_pCurPOpp->m_pPlaneWOpp[0]->m_Span);
			y = (offLE+pMiarex->m_pCurPOpp->m_pPlaneWOpp[0]->m_Chord[m]*pMiarex->m_pCurPOpp->m_pPlaneWOpp[0]->m_XTrTop[m])*scaley;

			To = QPoint(O.x()+pMiarex->m_pCurPOpp->m_pPlaneWOpp[0]->m_SpanPos[m]*scalex, O.y()+y );
			painter.drawLine(From, To);
			From  = To;
		}


		int x = (int)(rect().width()/2);
		int y = rect().bottom();
		painter.drawLine(x-60,  y - 50, x-40,  y - 50);
		painter.drawText(x-35, y - 48, tr("Top transition"));

	}


	QPen BotPen(W3dPrefsDlg::s_BotColor);
	BotPen.setStyle(getStyle(W3dPrefsDlg::s_BotStyle));
	BotPen.setWidth(W3dPrefsDlg::s_BotWidth);

	painter.setPen(BotPen);
	if (m_bXBot)
	{
		offLE = pWing->getOffset(pMiarex->m_pCurPOpp->m_pPlaneWOpp[0]->m_SpanPos[nStart]*2.0/pMiarex->m_pCurPOpp->m_pPlaneWOpp[0]->m_Span);
		y = (offLE+pMiarex->m_pCurPOpp->m_pPlaneWOpp[0]->m_Chord[nStart]*pMiarex->m_pCurPOpp->m_pPlaneWOpp[0]->m_XTrBot[nStart])*scaley;
		From = QPoint(O.x() +pMiarex->m_pCurPOpp->m_pPlaneWOpp[0]->m_SpanPos[nStart]*scalex, O.y()+y );
		for (m=nStart; m<pMiarex->m_pCurPOpp->m_pPlaneWOpp[0]->m_NStation; m++)
		{
			offLE = pWing->getOffset(pMiarex->m_pCurPOpp->m_pPlaneWOpp[0]->m_SpanPos[m]*2.0/pMiarex->m_pCurPOpp->m_pPlaneWOpp[0]->m_Span);
			y = (offLE+pMiarex->m_pCurPOpp->m_pPlaneWOpp[0]->m_Chord[m]*pMiarex->m_pCurPOpp->m_pPlaneWOpp[0]->m_XTrBot[m])*scaley;
			To = QPoint(O.x()+pMiarex->m_pCurPOpp->m_pPlaneWOpp[0]->m_SpanPos[m]*scalex, O.y()+y );
			painter.drawLine(From, To);
			From  = To;
		}

		int x = (int)(rect().width()/2);
		int y = rect().bottom();
		painter.drawLine(x-60,  y - 35, x-40,  y - 35);
		painter.drawText(x-35, y - 33, tr("Bottom transition"));

	}
	painter.restore();
}





void WingWidget::resizeEvent (QResizeEvent *event)
{
	setWingScale();
	update();
}


void WingWidget::wheelEvent (QWheelEvent *event)
{
qDebug()<<"WingWidget wheel event"<<m_WingScale	;
	double zoomFactor=1.0;

	QPoint pt(event->x(), event->y()); //client coordinates

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

	m_WingScale *= zoomFactor;

	m_bIs2DScaleSet = false;

	update();
}



void WingWidget::setWingScale()
{
	if(m_bIs2DScaleSet) return;
	QMiarex *pMiarex=(QMiarex*)s_pMiarex;

	m_ptOffset.rx() = rect().width()/2.0;
	m_ptOffset.ry() = rect().height()/4.0;

	if(pMiarex->m_pCurPlane && m_pGraph)
	{
//		qDebug()<<"graphmargin"<<m_pGraph->margin()<<pMiarex->m_pCurPlane->planformSpan();
		m_WingScale = (rect().width()-2*m_pGraph->margin())/pMiarex->m_pCurPlane->planformSpan();
		m_bIs2DScaleSet = true;
	}
}



void WingWidget::onResetWingScale()
{
	setWingScale();
	update();
}



/**
 * Loads the user's default settings from the application QSettings object
 * @param pSettings a pointer to the QSettings object
 */
void WingWidget::loadSettings(QSettings *pSettings)
{
	pSettings->beginGroup("WingWidget");
	{
		m_bXCmRef       = pSettings->value("bXCmRef", true).toBool();

	}
}


/**
 * Saves the user-defined settings
 * @param pSettings a pointer to the QSetting object.
 */
void WingWidget::saveSettings(QSettings *pSettings)
{
	pSettings->beginGroup("WingWidget");
	{
		pSettings->setValue("bXCmRef", m_bXCmRef);

	}
}
