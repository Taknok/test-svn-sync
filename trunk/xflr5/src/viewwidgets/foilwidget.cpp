/****************************************************************************

	FoilWidget Class
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

#include "foilwidget.h"
#include "xdirect/XDirectStyleDlg.h"
#include "mainframe.h"
#include <Foil.h>
#include <OpPoint.h>
#include <Settings.h>
#include <globals.h>
#include <QStatusBar>
#include <QtDebug>

void *FoilWidget::s_pMainFrame= NULL;

FoilWidget::FoilWidget(QWidget *pParent) : QWidget(pParent)
{
	setMouseTracking(true);
	m_bTrans       = false;
	m_bAnimate     = false;
	m_bBL          = false;
	m_bPressure    = false;
	m_bNeutralLine = true;
	m_bShowPanels  = false;

	m_iNeutralStyle = 2;
	m_iNeutralWidth = 1;
	m_crNeutralColor = QColor(200,200,255);

	m_crBLColor = QColor(200,70,70);
	m_iBLStyle = 1;
	m_iBLWidth = 1;
	m_crPressureColor= QColor(100,150,100);
	m_iPressureStyle = 0;
	m_iPressureWidth = 1;

	m_fScale = 1.0;
	m_pCpGraph = NULL;
}



FoilWidget::~FoilWidget()
{
}


void FoilWidget::contextMenuEvent (QContextMenuEvent *event)
{
//	MainFrame *pMainFrame = (MainFrame*)s_pMainFrame;
//	pMainFrame->m_pOperFoilCtxMenu->exec(event->globalPos());
	event->ignore();
}

void FoilWidget::keyPressEvent(QKeyEvent *event)
{
	switch (event->key())
	{
		case Qt::Key_R:
			setFoilScale();
			update();
			break;

		default:event->ignore();

	}
}



void FoilWidget::mouseDoubleClickEvent (QMouseEvent *event)
{
	event->ignore();
}



/**
 * The user has toggled the display of the pressure arrows
 */
void FoilWidget::onShowPressure(bool bPressure)
{
    showPressure(bPressure);
    update();
}



/**
 * The user has toggled the display of the boundary layer
 */
void FoilWidget::onShowBL(bool bBL)
{
    showBL(bBL);
    update();
}




/**
 * Overrides the QWidget's mouseMoveEvent method.
 * Dispatches the event
 * @param event the QMouseEvent
 */
void FoilWidget::mouseMoveEvent(QMouseEvent *event)
{
	QPoint pt;
	double scale;
	double a;
	MainFrame* pMainFrame;
	pMainFrame = (MainFrame*)s_pMainFrame;

	pt.setX(event->x());
	pt.setY(event->y()); //client coordinates
	setFocus();

	if ((event->buttons() & Qt::LeftButton) && m_bTrans)
	{

		// we translate the airfoil
		m_FoilOffset.rx() += pt.x()-m_PointDown.x();
		m_FoilOffset.ry() += pt.y()-m_PointDown.y();

		if(!m_bAnimate) update();

		m_PointDown = pt;
	}
	else if (Foil::curFoil() && ((event->buttons() & Qt::MidButton) || event->modifiers().testFlag(Qt::AltModifier)))
	{
		// we zoom the graph or the foil
		if(Foil::curFoil())
		{
			//zoom the foil
			scale = m_fScale;

			if(pt.y()-m_PointDown.y()<0) m_fScale /= 1.02;
			else                         m_fScale *= 1.02;

			a = rect().center().x();

			m_FoilOffset.rx() = a + (m_FoilOffset.x()-a)/scale*m_fScale;

			if(!m_bAnimate) update();

		}
		m_PointDown = pt;
	}
	else
	{
		if(!Foil::curFoil()) return;

		//convert screen coordinates to foil coordinates
		CVector real = MousetoReal((event->pos()));
		pMainFrame->statusBar()->showMessage(QString("X = %1, Y = %2").arg(real.x).arg(real.y));
	}

	event->accept();

}



/**
 * Overrides the QWidget's mousePressEvent method.
 * Dispatches the event
 * @param event the QMouseEvent
 */
void FoilWidget::mousePressEvent(QMouseEvent *event)
{
	QPoint pt(event->x(), event->y()); //client coordinates

	if(event->buttons() & Qt::LeftButton)
	{
		m_PointDown.setX(pt.x());
		m_PointDown.setY(pt.y());

		m_bTrans = true;
		setCursor(Qt::ClosedHandCursor);
		if(!m_bAnimate) update();
	}

	event->accept();
}


/**
 * Overrides the QWidget's mouseReleasEvent method.
 * Dispatches the event
 * @param event the QMouseEvent
 */
void FoilWidget::mouseReleaseEvent(QMouseEvent *event)
{
	m_bTrans = false;
	setCursor(Qt::CrossCursor);
	event->accept();
}



void FoilWidget::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);
	QPainter painter(this);
	painter.fillRect(rect(), Settings::backgroundColor());

	paintOpPoint(painter);
}



/**
 * The user has requested the launch of the interface used to define the display style of the Foil
 */
void FoilWidget::onXDirectStyle()
{
	XDirectStyleDlg xdsDlg(this);
	xdsDlg.m_iBLStyle         = m_iBLStyle;
	xdsDlg.m_iBLWidth         = m_iBLWidth;
	xdsDlg.m_crBLColor        = m_crBLColor;
	xdsDlg.m_iPressureStyle   = m_iPressureStyle;
	xdsDlg.m_iPressureWidth   = m_iPressureWidth;
	xdsDlg.m_crPressureColor  = m_crPressureColor;
	xdsDlg.m_iNeutralStyle    = m_iNeutralStyle;
	xdsDlg.m_iNeutralWidth    = m_iNeutralWidth;
	xdsDlg.m_crNeutralColor   = m_crNeutralColor;

	if(xdsDlg.exec() == QDialog::Accepted)
	{
		m_iBLStyle         = xdsDlg.m_iBLStyle;
		m_iBLWidth         = xdsDlg.m_iBLWidth;
		m_crBLColor        = xdsDlg.m_crBLColor;
		m_iPressureStyle   = xdsDlg.m_iPressureStyle;
		m_iPressureWidth   = xdsDlg.m_iPressureWidth;
		m_crPressureColor  = xdsDlg.m_crPressureColor;
		m_iNeutralStyle    = xdsDlg.m_iNeutralStyle;
		m_iNeutralWidth    = xdsDlg.m_iNeutralWidth;
		m_crNeutralColor   = xdsDlg.m_crNeutralColor;
	}
	update();
}


/**
 * Converts screen coordinates to viewport coordinates
 * @param point the screen coordinates
 * @return the viewport coordinates
 */
CVector FoilWidget::MousetoReal(QPoint point)
{
	CVector Real;

	Real.x =  (point.x() - m_FoilOffset.x())/m_fScale;
	Real.y = -(point.y() - m_FoilOffset.y())/m_fScale;
	Real.z = 0.0;

	return Real;
}


/**
 * Overrides the QWidget's wheelEvent method.
 * Dispatches the event
 * @param event the QWheelEvent
 */
void FoilWidget::wheelEvent (QWheelEvent *event)
{
	if(Foil::curFoil())
	{
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

		double scale = m_fScale;

		m_fScale *= zoomFactor;

		int a = (int)((rect().right()+rect().left())/2);

		m_FoilOffset.rx() = a + (int)((m_FoilOffset.x()-a)/scale*m_fScale);

//		if(!m_bAnimate)
			update();
	}
}



/**
 * Loads the user's default settings from the application QSettings object
 * @param pSettings a pointer to the QSettings object
 */
void FoilWidget::loadSettings(QSettings *pSettings)
{
	pSettings->beginGroup("FoilWidget");
	{
		m_bBL             = pSettings->value("BoundaryLayer").toBool();
		m_bPressure       = pSettings->value("Pressure").toBool();

		m_crBLColor = pSettings->value("BLColor").value<QColor>();
		m_iBLStyle = pSettings->value("BLStyle").toInt();
		m_iBLWidth = pSettings->value("BLWidth").toInt();

		m_crPressureColor = pSettings->value("PressureColor").value<QColor>();
		m_iPressureStyle = pSettings->value("PressureStyle").toInt();
		m_iPressureWidth = pSettings->value("PressureWidth").toInt();

		m_crNeutralColor = pSettings->value("NeutralColor").value<QColor>();
		m_iNeutralStyle = pSettings->value("NeutralStyle").toInt();
		m_iNeutralWidth = pSettings->value("NeutralWidth").toInt();
	}
}


/**
 * Saves the user-defined settings
 * @param pSettings a pointer to the QSetting object.
 */
void FoilWidget::saveSettings(QSettings *pSettings)
{
	pSettings->beginGroup("FoilWidget");
	{
		pSettings->setValue("BoundaryLayer", m_bBL);
		pSettings->setValue("Pressure", m_bPressure);
		pSettings->setValue("BLColor", m_crBLColor);
		pSettings->setValue("BLStyle", m_iBLStyle);
		pSettings->setValue("BLWidth", m_iBLWidth);
		pSettings->setValue("PressureColor", m_crPressureColor);
		pSettings->setValue("PressureStyle", m_iPressureStyle);
		pSettings->setValue("PressureWidth", m_iPressureWidth);
		pSettings->setValue("NeutralColor", m_crNeutralColor);
		pSettings->setValue("NeutralStyle", m_iNeutralStyle);
		pSettings->setValue("NeutralWidth", m_iNeutralWidth);
	}
}

/**
 * The method which draws the pressure arrows in the OpPoint view.
 * @param painter a reference to the QPainter object with which to draw
 * @param pOpPoint the OpPoint object to draw
 * @param scale the scale of the view
 */
void FoilWidget::paintPressure(QPainter &painter, double scale)
{
	if(!Foil::curFoil()) return;
	if(!OpPoint::curOpp()) return;
	if(!OpPoint::curOpp()->bViscResults()) return;

	int i;
	double alpha = -OpPoint::curOpp()->Alpha*PI/180.0;
	double cosa = cos(alpha);
	double sina = sin(alpha);
	double x, y ,xs, ys, xe, ye, dx, dy, x1, x2, y1, y2, r2;
	double cp;
	QPointF offset = m_FoilOffset;

	painter.save();

	QPen CpvPen(m_crPressureColor);
	CpvPen.setStyle(getStyle(m_iPressureStyle));
	CpvPen.setWidth(m_iPressureWidth);
	painter.setPen(CpvPen);


	for(i=0; i<Foil::curFoil()->n; i++)
	{
		if(OpPoint::curOpp()->m_bViscResults) cp = OpPoint::curOpp()->Cpv[i];
		else                                  cp = OpPoint::curOpp()->Cpi[i];
		x = Foil::curFoil()->x[i];
		y = Foil::curFoil()->y[i];

		xs = (x-0.5)*cosa - y*sina + 0.5;
		ys = (x-0.5)*sina + y*cosa;

		if(cp>0)
		{
			x += Foil::curFoil()->nx[i] * cp * 0.05;
			y += Foil::curFoil()->ny[i] * cp * 0.05;

			xe = (x-0.5)*cosa - y*sina + 0.5;
			ye = (x-0.5)*sina + y*cosa;
			painter.drawLine(xs*scale + offset.x(), -ys*scale + offset.y(),
							 xe*scale + offset.x(), -ye*scale + offset.y());

			dx = xe - xs;
			dy = ye - ys;
			r2 = sqrt(dx*dx + dy*dy);
			if(r2!=0.0) //you can never be sure...
			{
				dx = dx/r2;
				dy = dy/r2;
			}

			x1 = xs + 0.0085*dx + 0.005*dy;
			y1 = ys + 0.0085*dy - 0.005*dx;
			x2 = xs + 0.0085*dx - 0.005*dy;
			y2 = ys + 0.0085*dy + 0.005*dx;

			painter.drawLine( xs*scale + offset.x(), -ys*scale + offset.y(),
							  x1*scale + offset.x(), -y1*scale + offset.y());

			painter.drawLine( xs*scale + offset.x(), -ys*scale + offset.y(),
							  x2*scale + offset.x(), -y2*scale + offset.y());
		}
		else
		{

			x += -Foil::curFoil()->nx[i] * cp *0.05;
			y += -Foil::curFoil()->ny[i] * cp *0.05;

			xe = (x-0.5)*cosa - y*sina+ 0.5;
			ye = (x-0.5)*sina + y*cosa;
			painter.drawLine( xs*scale + offset.x(), -ys*scale + offset.y(),
							  xe*scale + offset.x(), -ye*scale + offset.y());

			dx = xe - xs;
			dy = ye - ys;
			r2 = sqrt(dx*dx + dy*dy);
			if(r2!=0.0) //you can never be sure...
			{
				dx = -dx/r2;
				dy = -dy/r2;
			}

			x1 = xe + 0.0085*dx + 0.005*dy;
			y1 = ye + 0.0085*dy - 0.005*dx;
			x2 = xe + 0.0085*dx - 0.005*dy;
			y2 = ye + 0.0085*dy + 0.005*dx;

			painter.drawLine( xe*scale + offset.x(), -ye*scale + offset.y(),
							  x1*scale + offset.x(), -y1*scale + offset.y());

			painter.drawLine( xe*scale + offset.x(), -ye*scale + offset.y(),
							  x2*scale + offset.x(), -y2*scale + offset.y());
		}
	}
	//last draw lift at XCP position
	QPen LiftPen(m_crPressureColor);
	LiftPen.setStyle(getStyle(m_iPressureStyle));
	LiftPen.setWidth(m_iPressureWidth+1);
	painter.setPen(LiftPen);

	xs =  (OpPoint::curOpp()->m_XCP-0.5)*cosa  + 0.5;
	ys = -(OpPoint::curOpp()->m_XCP-0.5)*sina ;

	xe = xs;
	ye = ys - OpPoint::curOpp()->Cl/10.0;

	painter.drawLine( xs*scale + offset.x(), ys*scale + offset.y(),
					  xs*scale + offset.x(), ye*scale + offset.y());

	dx = xe - xs;
	dy = ye - ys;
	r2 = sqrt(dx*dx + dy*dy);
	dx = -dx/r2;
	dy = -dy/r2;

	x1 = xe + 0.0085*dx + 0.005*dy;
	y1 = ye + 0.0085*dy - 0.005*dx;
	x2 = xe + 0.0085*dx - 0.005*dy;
	y2 = ye + 0.0085*dy + 0.005*dx;

	painter.drawLine( xe*scale + offset.x(), ye*scale + offset.y(),
					  x1*scale + offset.x(), y1*scale + offset.y());

	painter.drawLine( xe*scale + offset.x(), ye*scale + offset.y(),
					  x2*scale + offset.x(), y2*scale + offset.y());

	painter.restore();
}




/**
 * Draws the Cp Graph and the foil
 * @param painter a reference to the QPainter object with which to draw
 */
void FoilWidget::paintOpPoint(QPainter &painter)
{
	if (!Foil::curFoil() || !Foil::curFoil()->foilName().length())
		return;

	double Alpha, FoilScale;
	FoilScale = m_fScale;
	QString Result, str, str1;

	if (rect().width()<150 || rect().height()<150) return;

	painter.save();

	if(m_bNeutralLine)
	{
		QPen NeutralPen(m_crNeutralColor);
		NeutralPen.setStyle(getStyle(m_iNeutralStyle));
		NeutralPen.setWidth(m_iNeutralWidth);
		painter.setPen(NeutralPen);
		painter.drawLine(rect().left(),  m_FoilOffset.y(),
						 rect().right(), m_FoilOffset.y());
	}

	Alpha = 0.0;
	if(OpPoint::curOpp()) Alpha = OpPoint::curOpp()->Alpha;

	Foil::curFoil()->m_bPoints = m_bShowPanels;
	Foil::curFoil()->drawFoil(painter, -Alpha, m_fScale, FoilScale, m_FoilOffset);

	if(m_bPressure && OpPoint::curOpp()) paintPressure(painter, m_fScale);
	if(m_bBL && OpPoint::curOpp())       paintBL(painter, OpPoint::curOpp(), m_fScale);


	// Write Titles and results
	QString strong;

	painter.setFont(Settings::s_TextFont);
	int D = 0;
	int ZPos = rect().bottom();
	int XPos = rect().right()-10;
	QPen WritePen(Settings::s_TextColor);
	painter.setPen(WritePen);

	QFontMetrics fm(Settings::s_TextFont);
	int dD = fm.height();

	//write the foil's properties

	int Back = 5;

	if(Foil::curFoil()->m_bTEFlap) Back +=3;

	int LeftPos = rect().left()+10;
	ZPos = rect().bottom() - 10 - Back*dD;

	D = 0;
	str1 = Foil::curFoil()->m_FoilName;
	painter.drawText(LeftPos,ZPos+D, str1+str);
	D += dD;

	str = "%";
	str1 = QString(tr("Thickness         = %1")).arg(Foil::curFoil()->m_fThickness*100.0, 6, 'f', 2);
	painter.drawText(LeftPos,ZPos+D, str1+str);
	D += dD;

	str1 = QString(tr("Max. Thick.pos.   = %1")).arg(Foil::curFoil()->m_fXThickness*100.0, 6, 'f', 2);
	painter.drawText(LeftPos,ZPos+D, str1+str);
	D += dD;

	str1 = QString(tr("Max. Camber       = %1")).arg( Foil::curFoil()->m_fCamber*100.0, 6, 'f', 2);
	painter.drawText(LeftPos,ZPos+D, str1+str);
	D += dD;

	str1 = QString(tr("Max. Camber pos.  = %1")).arg(Foil::curFoil()->m_fXCamber*100.0, 6, 'f', 2);
	painter.drawText(LeftPos,ZPos+D, str1+str);
	D += dD;

	str1 = QString(tr("Number of Panels  =  %1")).arg( Foil::curFoil()->n);
	painter.drawText(LeftPos,ZPos+D, str1);
	D += dD;

	if(Foil::curFoil()->m_bTEFlap)
	{
		str1 = QString(tr("Flap Angle = %1")+QString::fromUtf8("°")).arg( Foil::curFoil()->m_TEFlapAngle, 7, 'f', 2);
		painter.drawText(LeftPos,ZPos+D, str1);
		D += dD;

		str1 = QString(tr("XHinge     = %1")).arg( Foil::curFoil()->m_TEXHinge, 6, 'f', 1);
		strong="%";
		painter.drawText(LeftPos,ZPos+D, str1+strong);
		D += dD;

		str1 = QString(tr("YHinge     = %1")).arg( Foil::curFoil()->m_TEYHinge, 6, 'f', 1);
		strong="%";
		painter.drawText(LeftPos,ZPos+D, str1+strong);
		D += dD;
	}


	D = 0;
	Back = 6;

	if(OpPoint::curOpp())
	{
		Back = 12;
		if(OpPoint::curOpp()->m_bTEFlap) Back++;
		if(OpPoint::curOpp()->m_bLEFlap) Back++;
		if(OpPoint::curOpp()->m_bViscResults && qAbs(OpPoint::curOpp()->Cd)>0.0) Back++;
		if(Polar::curPolar()->polarType()==XFLR5::FIXEDLIFTPOLAR) Back++;
		if(Polar::curPolar()->polarType()!=XFLR5::FIXEDSPEEDPOLAR && Polar::curPolar()->polarType()!=XFLR5::FIXEDAOAPOLAR) Back++;
	}

	int dwidth = fm.width(tr("TE Hinge Moment/span = 0123456789"));

	ZPos = rect().bottom()-Back*dD - 10;
	XPos = rect().right()-dwidth-20;
	D=0;


	if(Polar::curPolar())
	{
		if(Polar::curPolar()->polarType()==XFLR5::FIXEDSPEEDPOLAR)       str1 = tr("Fixed speed polar");
		else if(Polar::curPolar()->polarType()==XFLR5::FIXEDLIFTPOLAR)   str1 = tr("Fixed lift polar");
		else if(Polar::curPolar()->polarType()==XFLR5::RUBBERCHORDPOLAR) str1 = tr("Rubber chord polar");
		else if(Polar::curPolar()->polarType()==XFLR5::FIXEDAOAPOLAR)    str1 = tr("Fixed a.o.a. polar");

		painter.drawText(XPos,ZPos, dwidth, dD, Qt::AlignRight | Qt::AlignTop, str1);
		D += dD;
		if(Polar::curPolar()->polarType() ==XFLR5::FIXEDSPEEDPOLAR)
		{
			ReynoldsFormat(strong, Polar::curPolar()->m_Reynolds);
			strong ="Reynolds = " + strong;
			painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, strong);
			D += dD;
			strong = QString("Mach = %1").arg( Polar::curPolar()->m_Mach,9,'f',3);
			painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, strong);
			D += dD;
		}
		else if(Polar::curPolar()->polarType()==XFLR5::FIXEDLIFTPOLAR)
		{
			ReynoldsFormat(strong, Polar::curPolar()->m_Reynolds);
			strong = tr("Re.sqrt(Cl) = ") + strong;
			painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, strong);
			D += dD;

			strong = QString(tr("M.sqrt(Cl) = %1")).arg(Polar::curPolar()->m_Mach,9,'f',3);
			painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, strong);
			D += dD;
		}
		else if(Polar::curPolar()->polarType()==XFLR5::RUBBERCHORDPOLAR)
		{
			ReynoldsFormat(strong, Polar::curPolar()->m_Reynolds);
			strong = tr("Re.sqrt(Cl) = ") + strong;
			painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, strong);
			D += dD;

			strong = QString("Mach = %1").arg(Polar::curPolar()->m_Mach,9,'f',3);
			painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, strong);
			D += dD;
		}
		else if(Polar::curPolar()->polarType()==XFLR5::FIXEDAOAPOLAR)
		{
			strong = QString("Alpha = %1 "+QString::fromUtf8("°")).arg(Polar::curPolar()->m_ASpec,10,'f',2);
			painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, strong);
			D += dD;
			strong = QString("Mach = %1").arg(Polar::curPolar()->m_Mach,9,'f',3);
			painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, strong);
			D += dD;
		}

		strong = QString("NCrit = %1").arg(Polar::curPolar()->m_ACrit,9,'f',3);
		painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, strong);
		D += dD;

		strong = QString(tr("Forced Upper Trans. = %1")).arg(Polar::curPolar()->m_XTop,9,'f',3);
		painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, strong);
		D += dD;
		strong = QString(tr("Forced Lower Trans. = %1")).arg(Polar::curPolar()->m_XBot, 9, 'f', 3);
		painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, strong);
		D += dD;

		if(OpPoint::curOpp())
		{
			if(Polar::curPolar()->polarType()!=XFLR5::FIXEDSPEEDPOLAR)
			{
				ReynoldsFormat(Result, OpPoint::curOpp()->Reynolds);
				Result = "Re = "+ Result;
				painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, Result);
				D += dD;
			}
			if(Polar::curPolar()->polarType()==XFLR5::FIXEDLIFTPOLAR)
			{
				Result = QString("Ma = %1").arg(OpPoint::curOpp()->Mach, 9, 'f', 4);
				painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, Result);
				D += dD;
			}
			if(Polar::curPolar()->polarType()!=XFLR5::FIXEDAOAPOLAR)
			{
				Result = QString(tr("Alpha = %1")+QString::fromUtf8("°")).arg(OpPoint::curOpp()->Alpha, 8, 'f', 2);
				painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, Result);
				D += dD;
			}
			Result = QString(tr("Cl = %1")).arg(OpPoint::curOpp()->Cl, 9, 'f', 3);
			painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, Result);
			D += dD;

			Result = QString(tr("Cm = %1")).arg(OpPoint::curOpp()->Cm, 9, 'f', 3);
			painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, Result);
			D += dD;

			Result = QString(tr("Cd = %1")).arg(OpPoint::curOpp()->Cd, 9, 'f', 3);
			painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, Result);
			D += dD;

			if(OpPoint::curOpp()->m_bViscResults && qAbs(OpPoint::curOpp()->Cd)>0.0)
			{
				Result = QString(tr("L/D = %1")).arg(OpPoint::curOpp()->Cl/OpPoint::curOpp()->Cd, 9, 'f', 3);
				painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, Result);
				D += dD;
			}

			Result = QString(tr("Upper Trans. = %1")).arg(OpPoint::curOpp()->Xtr1, 9, 'f', 3);
			painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, Result);
			D += dD;

			Result = QString(tr("Lower Trans. = %1")).arg(OpPoint::curOpp()->Xtr2, 9, 'f', 3);
			painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, Result);
			D += dD;

			if(OpPoint::curOpp()->m_bTEFlap)
			{
				Result = QString(tr("TE Hinge Moment/span = %1")).arg(OpPoint::curOpp()->m_TEHMom, 9, 'e', 2);
				painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, Result);
				D += dD;
			}

			if(OpPoint::curOpp()->m_bLEFlap)
			{
				Result = QString(tr("LE Hinge Moment/span = %1")).arg(OpPoint::curOpp()->m_LEHMom, 9, 'e', 2);
				painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, Result);
				D += dD;
			}
		}
	}
	painter.restore();
}







/**
 * The method which draws the boundary layer in the OpPoint view.
 * @param painter a reference to the QPainter object with which to draw
 * @param pOpPoint the OpPoint object to draw
 * @param scale the scale of the view
 */
void FoilWidget::paintBL(QPainter &painter, OpPoint* pOpPoint, double scale)
{
	if(!Foil::curFoil() || !pOpPoint) return;

	QPointF offset, From, To;
	double x,y;
	int i;
	double alpha = -pOpPoint->Alpha*PI/180.0;
	double cosa = cos(alpha);
	double sina = sin(alpha);

	if(!pOpPoint->m_bViscResults || !pOpPoint->m_bBL) return;

	painter.save();

	offset = m_FoilOffset;
	scale  = scale;
	scale  = scale;

	QPen WakePen(m_crBLColor);
	WakePen.setStyle(getStyle(m_iBLStyle));
	WakePen.setWidth(m_iBLWidth);

	painter.setPen(WakePen);

	x = (pOpPoint->xd1[1]-0.5)*cosa - pOpPoint->yd1[1]*sina + 0.5;
	y = (pOpPoint->xd1[1]-0.5)*sina + pOpPoint->yd1[1]*cosa;
	From.rx() =  x*scale + offset.x();
	From.ry() = -y*scale + offset.y();
	for (i=2; i<=pOpPoint->nd1; i++)
	{
		x = (pOpPoint->xd1[i]-0.5)*cosa - pOpPoint->yd1[i]*sina + 0.5;
		y = (pOpPoint->xd1[i]-0.5)*sina + pOpPoint->yd1[i]*cosa;
		To.rx() =  x*scale + offset.x();
		To.ry() = -y*scale + offset.y();
		painter.drawLine(From, To);
		From = To;
	}

	x = (pOpPoint->xd2[0]-0.5)*cosa - pOpPoint->yd2[0]*sina + 0.5;
	y = (pOpPoint->xd2[0]-0.5)*sina + pOpPoint->yd2[0]*cosa;
	From.rx() =  x*scale + offset.x();
	From.ry() = -y*scale + offset.y();
	for (i=1; i<pOpPoint->nd2; i++)
	{
		x = (pOpPoint->xd2[i]-0.5)*cosa - pOpPoint->yd2[i]*sina + 0.5;
		y = (pOpPoint->xd2[i]-0.5)*sina + pOpPoint->yd2[i]*cosa;
		To.rx() =  x*scale + offset.x();
		To.ry() = -y*scale + offset.y();
		painter.drawLine(From, To);
		From = To;
	}

	x = (pOpPoint->xd3[0]-0.5)*cosa - pOpPoint->yd3[0]*sina + 0.5;
	y = (pOpPoint->xd3[0]-0.5)*sina + pOpPoint->yd3[0]*cosa;
	From.rx() =  x*scale + offset.x();
	From.ry() = -y*scale + offset.y();
	for (i=1; i<pOpPoint->nd3; i++)
	{
		x = (pOpPoint->xd3[i]-0.5)*cosa - pOpPoint->yd3[i]*sina + 0.5;
		y = (pOpPoint->xd3[i]-0.5)*sina + pOpPoint->yd3[i]*cosa;
		To.rx() =  x*scale + offset.x();
		To.ry() = -y*scale + offset.y();
		painter.drawLine(From, To);
		From = To;
	}
	painter.restore();
}



/**
 * Sets the Foil scale in the OpPoint view.
 */
void FoilWidget::setFoilScale()
{
	int iMargin = 53;
	if(m_pCpGraph) iMargin = m_pCpGraph->margin();
	m_FoilOffset.rx() = rect().left() + iMargin;
	m_FoilOffset.ry() = rect().center().y();

	m_fScale = (rect().width()-2.0*iMargin);
	if(m_pCpGraph && m_pCpGraph->yVariable()>=2)
	{
		double p0  = m_pCpGraph->xToClient(0.0);
		double p1  = m_pCpGraph->xToClient(1.0);
		m_fScale =  (p1-p0);
	}
}




/**
 * The user has requested to reset the scale of the foil to its automatic default value
 */
void FoilWidget::onResetFoilScale()
{
	setFoilScale();
//	if(!m_pFoilWidget->m_bAnimate)
	update();

}


/**
 * The user has toggled the display of the neutral line y=0.
 */
void FoilWidget::onShowNeutralLine()
{
	m_bNeutralLine = !m_bNeutralLine;
	MainFrame *pMainFrame = (MainFrame*)s_pMainFrame;
	pMainFrame->m_pShowNeutralLine->setChecked(m_bNeutralLine);
	update();
}


/**
 * The user has toggled the display of the Foil's panels.
 */
void FoilWidget::onShowPanels()
{
	m_bShowPanels = !m_bShowPanels;
	MainFrame *pMainFrame = (MainFrame*)s_pMainFrame;
	pMainFrame->m_pShowPanels->setChecked(m_bShowPanels);
	update();
}









