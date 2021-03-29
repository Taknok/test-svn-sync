/****************************************************************************

    OpPointWidget Class
    Copyright (C) 2016-2016 Andre Deperrois 

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

#include "oppointwt.h"



#include <QAction>



#include <xflcore/xflcore.h>
#include <globals/mainframe.h>
#include <graph/graph.h>
#include <graph/graph_globals.h>
#include <graph/graphdlg.h>
#include <misc/options/settings.h>
#include <xdirect/xdirect.h>
#include <xdirect/xdirectstyledlg.h>
#include <xflcore/constants.h>




MainFrame *OpPointWidget::s_pMainFrame = nullptr;

/**
*The public constructor
*/
OpPointWidget::OpPointWidget(QWidget *parent) : QWidget(parent)
{
    setMouseTracking(true);
    setCursor(Qt::CrossCursor);

    QSizePolicy sizepol;
    sizepol.setHorizontalPolicy(QSizePolicy::Expanding);
    sizepol.setVerticalPolicy(QSizePolicy::Expanding);
    setSizePolicy(sizepol);

    m_bTransFoil   = false;
    m_bTransGraph  = false;
    m_bAnimate     = false;
    m_bBL          = false;
    m_bPressure    = false;
    m_bNeutralLine = true;
//    m_bShowPanels  = false;
    m_bXPressed = m_bYPressed = false;

    m_NeutralStyle  = {true, Line::DASHDOT, 1, QColor(155, 155,155), Line::NOSYMBOL};
    m_BLStyle       = {true, Line::DASH,    2, QColor(255, 55,  55), Line::NOSYMBOL};
    m_PressureStyle = {true, Line::SOLID,   2, QColor( 95, 155, 95), Line::NOSYMBOL};

    m_fScale = m_fYScale = 1.0;
    m_pCpGraph = nullptr;
}


/**
*Overrides the keyPressEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void OpPointWidget::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_V:
        {
            if(m_pCpGraph->isInDrawRect(m_LastPoint))
            {
                GraphDlg::setActivePage(0);
                onGraphSettings();
            }
            pEvent->accept();
            break;
        }
        case Qt::Key_G:
        {
            if(m_pCpGraph->isInDrawRect(m_LastPoint))
            {
                onGraphSettings();
            }
            pEvent->accept();
            break;
        }
        case Qt::Key_R:
            if(m_pCpGraph->isInDrawRect(m_LastPoint))
                m_pCpGraph->setAuto(true);
            else setFoilScale();
            update();
            break;

        case Qt::Key_X:
            m_bXPressed = true;
            break;

        case Qt::Key_Y:
            m_bYPressed = true;
            break;

        default:pEvent->ignore();

    }
}


/**
*Overrides the keyReleaseEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void OpPointWidget::keyReleaseEvent(QKeyEvent *pEvent)
{
    Q_UNUSED(pEvent);
    m_bXPressed = m_bYPressed = false;
}



/**
*Overrides the mousePressEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void OpPointWidget::mousePressEvent(QMouseEvent *pEvent)
{
    QPoint pt(pEvent->x(), pEvent->y()); //client coordinates

    if(pEvent->buttons() & Qt::LeftButton)
    {
        if (m_pCpGraph->isInDrawRect(pEvent->pos()))
        {
            m_bTransGraph = true;
        }
        else
        {
            m_bTransFoil = true;
        }
        m_LastPoint.setX(pt.x());
        m_LastPoint.setY(pt.y());
        setCursor(Qt::ClosedHandCursor);
//        if(!m_bAnimate) update();
    }

    pEvent->accept();
}


/**
*Overrides the mouseReleaseEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void OpPointWidget::mouseReleaseEvent(QMouseEvent *pEvent)
{
    m_bTransGraph = false;
    m_bTransFoil  = false;
    setCursor(Qt::CrossCursor);
    pEvent->accept();
}


/**
*Overrides the mouseMoveEvent method of the base class.
*/
void OpPointWidget::mouseMoveEvent(QMouseEvent *pEvent)
{
    QPoint pt;
    double scale;
    double a;

    pt.setX(pEvent->x());
    pt.setY(pEvent->y()); //client coordinates
    setFocus();

    if (pEvent->buttons() & Qt::LeftButton)
    {
        if(m_bTransGraph)
        {
            QPoint point;
            double xu, yu, x1, y1, xmin, xmax, ymin, ymax;

            point = pEvent->pos();

            // we translate the curves inside the graph
            m_pCpGraph->setAuto(false);
            x1 =  m_pCpGraph->clientTox(m_LastPoint.x()) ;
            y1 =  m_pCpGraph->clientToy(m_LastPoint.y()) ;

            xu = m_pCpGraph->clientTox(point.x());
            yu = m_pCpGraph->clientToy(point.y());

            xmin = m_pCpGraph->xMin() - xu+x1;
            xmax = m_pCpGraph->xMax() - xu+x1;
            ymin = m_pCpGraph->yMin() - yu+y1;
            ymax = m_pCpGraph->yMax() - yu+y1;

            m_pCpGraph->setWindow(xmin, xmax, ymin, ymax);
        }
        else if(m_bTransFoil)
        {
            // we translate the airfoil
            m_FoilOffset.rx() += pt.x()-m_LastPoint.x();
            m_FoilOffset.ry() += pt.y()-m_LastPoint.y();
        }
    }
    else if (XDirect::curFoil() && ((pEvent->buttons() & Qt::MidButton) || pEvent->modifiers().testFlag(Qt::AltModifier)))
    {
        // we zoom the graph or the foil
        if(XDirect::curFoil())
        {
            //zoom the foil
            scale = m_fScale;

            if(pt.y()-m_LastPoint.y()<0) m_fScale /= 1.02;
            else                         m_fScale *= 1.02;

            a = rect().center().x();

            m_FoilOffset.rx() = a + (m_FoilOffset.x()-a)/scale*m_fScale;
        }
    }

    m_LastPoint = pt;
    if(!m_bAnimate) update();

    pEvent->accept();

}


void OpPointWidget::mouseDoubleClickEvent (QMouseEvent *pEvent)
{
    Q_UNUSED(pEvent);
    setCursor(Qt::CrossCursor);
    if (m_pCpGraph->isInDrawRect(pEvent->pos()))
    {
        onGraphSettings();
        update();
    }
}


/**
 * The user has requested an edition of the settings of the active graph
 */
void OpPointWidget::onGraphSettings()
{
    GraphDlg grDlg(this);
    grDlg.setGraph(m_pCpGraph);

//    QAction *action = qobject_cast<QAction *>(sender());
//    grDlg.setActivePage(0);

    if(grDlg.exec() == QDialog::Accepted)
    {
        emit graphChanged(m_pCpGraph);
    }
    update();
}


/**
*Overrides the resizeEvent function of the base class.
*Dispatches the handling to the active child application.
*/
void OpPointWidget::resizeEvent(QResizeEvent *pEvent)
{
    Q_UNUSED(pEvent);
    if(m_pCpGraph)
    {
        int h = rect().height();
        int h4 = h/3;
        QRect rGraphRect = QRect(0, 0, + rect().width(), rect().height()-h4);
        m_pCpGraph->setMargin(50);
        m_pCpGraph->setDrawRect(rGraphRect);
        m_pCpGraph->initializeGraph();
    }
    setFoilScale();
}

/**
 * Sets the Foil scale in the OpPoint view.
 */
void OpPointWidget::resetGraphScale()
{
    if(m_pCpGraph)
    {
        m_pCpGraph->setAuto(true);
        update();
    }
}

/**
 * Sets the Foil scale in the OpPoint view.
 */
void OpPointWidget::setFoilScale()
{
    int iMargin = 53;
    if(m_pCpGraph)
    {
        iMargin = m_pCpGraph->margin();
        int h =  m_pCpGraph->clientRect()->height();
        m_FoilOffset.rx() = rect().left() + iMargin;
        m_FoilOffset.ry() = (rect().height()+h)/2;
//        m_fScale = rect().width()-2.0*iMargin;
//        if(m_pCpGraph && m_pCpGraph->yVariable()<2)
        {
            double p0  = m_pCpGraph->xToClient(0.0);
            double p1  = m_pCpGraph->xToClient(1.0);
            m_fScale =  (p1-p0);
        }
    }
    else
    {
        m_FoilOffset.rx() = rect().left() + iMargin;
        m_FoilOffset.ry() = rect().center().y();

        m_fScale = rect().width()-2.0*iMargin;
    }
    m_fYScale = 1.0;
}


/**
*Overrides the wheelEvent function of the base class.
*Dispatches the handling to the active child application.
*/
void OpPointWidget::wheelEvent(QWheelEvent *pEvent)
{
    QPoint pt;
#if QT_VERSION >= 0x050F00
    pt = pEvent->position().toPoint();
#else
    pt = pEvent->pos();
#endif

    if(m_pCpGraph && m_pCpGraph->isInDrawRect(pt))
    {
        double zoomFactor=1.0;

        if(pEvent->angleDelta().y()>0)
        {
            if(!Settings::s_bReverseZoom) zoomFactor = 1./1.06;
            else                          zoomFactor = 1.06;
        }
        else
        {
            if(!Settings::s_bReverseZoom) zoomFactor = 1.06;
            else                          zoomFactor = 1./1.06;
        }

        if (m_bXPressed)
        {
            //zoom x scale
            m_pCpGraph->setAutoX(false);
            m_pCpGraph->scaleXAxis(1./zoomFactor);
        }
        else if(m_bYPressed)
        {
            //zoom y scale
            m_pCpGraph->setAutoY(false);
            m_pCpGraph->scaleYAxis(1./zoomFactor);
        }
        else
        {
            //zoom both
            m_pCpGraph->setAuto(false);
            m_pCpGraph->scaleAxes(1./zoomFactor);
        }

        m_pCpGraph->setAutoXUnit();
        m_pCpGraph->setAutoYUnit();
        update();
    }
    else if(XDirect::curFoil())
    {
        double zoomFactor=1.0;

        if(pEvent->angleDelta().y()>0)
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


        if(m_bYPressed)
        {
            m_fYScale *= zoomFactor;
        }
        else
        {
            m_fScale *= zoomFactor;
        }

        int a = int((rect().right()+rect().left())/2);

        m_FoilOffset.rx() = a + int((m_FoilOffset.x()-a)/scale*m_fScale);

//        if(!m_bAnimate)
            update();
    }
}


/**
*Overrides the paintEvent function of the base class.
*Dispatches the handling to the active child application.
*/
void OpPointWidget::paintEvent(QPaintEvent *pEvent)
{
    if(s_pMainFrame->m_iApp == Xfl::XFOILANALYSIS)
    {
        QPainter painter(this);
        painter.save();

        painter.fillRect(rect(), Settings::s_BackgroundColor);
        paintGraph(painter);
        paintOpPoint(painter);

        painter.restore();
    }
    else
    {
        QPainter painter(this);
        painter.fillRect(rect(), Settings::s_BackgroundColor);
    }
    pEvent->accept();
}



/**
 * Draws the graph
 * @param painter a reference to the QPainter object with which to draw
 */
void OpPointWidget::paintGraph(QPainter &painter)
{
    if(!m_pCpGraph) return;

    painter.save();

    QFontMetrics fm(Settings::textFont());
    int fmheight = fm.height();
    int fmWidth  = fm.averageCharWidth();
//  draw  the graph
    if(m_pCpGraph->clientRect()->width()>200 && m_pCpGraph->clientRect()->height()>150)
    {
        m_pCpGraph->drawGraph(painter);
        QPoint Place(m_pCpGraph->clientRect()->right()-73*fmWidth, m_pCpGraph->clientRect()->top()+fmheight);
        m_pCpGraph->drawLegend(painter, Place, Settings::s_TextFont, Settings::s_TextColor, Settings::backgroundColor());
    }


    if(m_pCpGraph->isInDrawRect(m_LastPoint) && Settings::bMousePos())
    {
        QPen textPen(Settings::textColor());

        painter.setPen(textPen);

        painter.drawText(m_pCpGraph->clientRect()->width()-14*fm.averageCharWidth(), m_pCpGraph->clientRect()->top()+  fmheight, QString("x = %1").arg(m_pCpGraph->clientTox(m_LastPoint.x()),9,'f',5));
        painter.drawText(m_pCpGraph->clientRect()->width()-14*fm.averageCharWidth(), m_pCpGraph->clientRect()->top()+2*fmheight, QString("y = %1").arg(m_pCpGraph->clientToy(m_LastPoint.y()),9,'f',5));
    }
    painter.restore();
}



/**
 * Draws the Cp Graph and the foil
 * @param painter a reference to the QPainter object with which to draw
 */
void OpPointWidget::paintOpPoint(QPainter &painter)
{
    if (!XDirect::curFoil() || !XDirect::curFoil()->name().length())
        return;

    double Alpha;
    QString Result, str, str1;

    if (rect().width()<150 || rect().height()<150) return;

    painter.save();

    if(m_bNeutralLine)
    {
        QPen NeutralPen(m_NeutralStyle.m_Color);
        NeutralPen.setStyle(getStyle(m_NeutralStyle.m_Stipple));
        NeutralPen.setWidth(m_NeutralStyle.m_Width);
        painter.setPen(NeutralPen);
        painter.drawLine(rect().left(),  int(m_FoilOffset.y()),
                         rect().right(), int(m_FoilOffset.y()));
    }
    if(!m_pCpGraph->isInDrawRect(m_LastPoint) && Settings::bMousePos())
    {
        QPen textPen(Settings::textColor());
        QFontMetrics fm(Settings::textFont());
        int fmheight  = fm.height();
        painter.setPen(textPen);

        Vector3d real = mousetoReal(m_LastPoint);
        painter.drawText(m_pCpGraph->clientRect()->width()-14*fm.averageCharWidth(),
                         m_pCpGraph->clientRect()->height() + fmheight, QString("x = %1")
                         .arg(real.x,9,'f',5));
        painter.drawText(m_pCpGraph->clientRect()->width()-14*fm.averageCharWidth(),
                         m_pCpGraph->clientRect()->height() + 2*fmheight, QString("y = %1")
                         .arg(real.y,9,'f',5));
    }


    Alpha = 0.0;
    if(XDirect::curOpp()) Alpha = XDirect::curOpp()->aoa();


    drawFoil(painter, XDirect::curFoil(), -Alpha, m_fScale, m_fScale*m_fYScale, m_FoilOffset);
    if(XDirect::curFoil()->pointStyle()>0)
        drawPoints(painter, XDirect::curFoil(), -Alpha, m_fScale,m_fScale*m_fYScale, m_FoilOffset, Settings::s_BackgroundColor);


/*    if(m_bShowPanels)
    {
        int memPts = QXDirect::curFoil()->foilPointStyle();
        QXDirect::curFoil()->foilPointStyle() = qMax(memPts, 1);
        drawPoints(painter, QXDirect::curFoil(), -Alpha, m_fScale, m_fScale*m_fYScale, m_FoilOffset);
        QXDirect::curFoil()->foilPointStyle() = memPts;
    }*/

    if(m_bPressure && XDirect::curOpp()) paintPressure(painter, m_fScale, m_fScale*m_fYScale);
    if(m_bBL && XDirect::curOpp())       paintBL(painter, XDirect::curOpp(), m_fScale, m_fScale*m_fYScale);


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

    if(XDirect::curFoil()->m_bTEFlap) Back +=3;

    int LeftPos = rect().left()+10;
    ZPos = rect().bottom() - 10 - Back*dD;

    D = 0;
    str1 = XDirect::curFoil()->name();
    painter.drawText(LeftPos,ZPos+D, str1+str);
    D += dD;

    str = "%";
    str1 = QString(tr("Thickness         = %1")).arg(XDirect::curFoil()->thickness()*100.0, 6, 'f', 2);
    painter.drawText(LeftPos,ZPos+D, str1+str);
    D += dD;

    str1 = QString(tr("Max. Thick.pos.   = %1")).arg(XDirect::curFoil()->xThickness()*100.0, 6, 'f', 2);
    painter.drawText(LeftPos,ZPos+D, str1+str);
    D += dD;

    str1 = QString(tr("Max. Camber       = %1")).arg( XDirect::curFoil()->camber()*100.0, 6, 'f', 2);
    painter.drawText(LeftPos,ZPos+D, str1+str);
    D += dD;

    str1 = QString(tr("Max. Camber pos.  = %1")).arg(XDirect::curFoil()->xCamber()*100.0, 6, 'f', 2);
    painter.drawText(LeftPos,ZPos+D, str1+str);
    D += dD;

    str1 = QString(tr("Number of Panels  =  %1")).arg( XDirect::curFoil()->n);
    painter.drawText(LeftPos,ZPos+D, str1);
    D += dD;

    if(XDirect::curFoil()->m_bTEFlap)
    {
        str1 = QString(tr("Flap Angle = %1")+QChar(0260)).arg( XDirect::curFoil()->m_TEFlapAngle, 7, 'f', 2);
        painter.drawText(LeftPos,ZPos+D, str1);
        D += dD;

        str1 = QString(tr("XHinge     = %1")).arg( XDirect::curFoil()->m_TEXHinge, 6, 'f', 1);
        strong="%";
        painter.drawText(LeftPos,ZPos+D, str1+strong);
        D += dD;

        str1 = QString(tr("YHinge     = %1")).arg( XDirect::curFoil()->m_TEYHinge, 6, 'f', 1);
        strong="%";
        painter.drawText(LeftPos,ZPos+D, str1+strong);
        D += dD;
    }


    D = 0;
    Back = 6;
    if(XDirect::curPolar() && XDirect::curOpp())
    {
        Back = 12;
        if(XDirect::curOpp()->m_bTEFlap) Back++;
        if(XDirect::curOpp()->m_bLEFlap) Back++;
        if(XDirect::curOpp()->m_bViscResults && qAbs(XDirect::curOpp()->Cd)>0.0) Back++;
        if(XDirect::curPolar()->polarType()==Xfl::FIXEDLIFTPOLAR) Back++;
        if(XDirect::curPolar()->polarType()!=Xfl::FIXEDSPEEDPOLAR && XDirect::curPolar()->polarType()!=Xfl::FIXEDAOAPOLAR) Back++;
    }

    int dwidth = fm.horizontalAdvance(tr("TE Hinge Moment/span = 0123456789"));

    ZPos = rect().bottom()-Back*dD - 10;
    XPos = rect().right()-dwidth-20;
    D=0;


    if(XDirect::curPolar())
    {
        if(XDirect::curPolar()->polarType()==Xfl::FIXEDSPEEDPOLAR)       str1 = tr("Fixed speed polar");
        else if(XDirect::curPolar()->polarType()==Xfl::FIXEDLIFTPOLAR)   str1 = tr("Fixed lift polar");
        else if(XDirect::curPolar()->polarType()==Xfl::RUBBERCHORDPOLAR) str1 = tr("Rubber chord polar");
        else if(XDirect::curPolar()->polarType()==Xfl::FIXEDAOAPOLAR)    str1 = tr("Fixed a.o.a. polar");

        painter.drawText(XPos,ZPos, dwidth, dD, Qt::AlignRight | Qt::AlignTop, str1);
        D += dD;
        if(XDirect::curPolar()->polarType() ==Xfl::FIXEDSPEEDPOLAR)
        {
            ReynoldsFormat(strong, XDirect::curPolar()->Reynolds());
            strong ="Reynolds = " + strong;
            painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, strong);
            D += dD;
            strong = QString("Mach = %1").arg( XDirect::curPolar()->Mach(),9,'f',3);
            painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, strong);
            D += dD;
        }
        else if(XDirect::curPolar()->polarType()==Xfl::FIXEDLIFTPOLAR)
        {
            ReynoldsFormat(strong, XDirect::curPolar()->Reynolds());
            strong = tr("Re.sqrt(Cl) = ") + strong;
            painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, strong);
            D += dD;

            strong = QString(tr("M.sqrt(Cl) = %1")).arg(XDirect::curPolar()->Mach(),9,'f',3);
            painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, strong);
            D += dD;
        }
        else if(XDirect::curPolar()->polarType()==Xfl::RUBBERCHORDPOLAR)
        {
            ReynoldsFormat(strong, XDirect::curPolar()->Reynolds());
            strong = tr("Re.sqrt(Cl) = ") + strong;
            painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, strong);
            D += dD;

            strong = QString("Mach = %1").arg(XDirect::curPolar()->Mach(),9,'f',3);
            painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, strong);
            D += dD;
        }
        else if(XDirect::curPolar()->polarType()==Xfl::FIXEDAOAPOLAR)
        {
            strong = QString("Alpha = %1 ").arg(XDirect::curPolar()->aoa(),10,'f',2)+QChar(0260);
            painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, strong);
            D += dD;
            strong = QString("Mach = %1").arg(XDirect::curPolar()->Mach(),9,'f',3);
            painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, strong);
            D += dD;
        }

        strong = QString("NCrit = %1").arg(XDirect::curPolar()->NCrit(),9,'f',3);
        painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, strong);
        D += dD;

        strong = QString(tr("Forced Upper Trans. = %1")).arg(XDirect::curPolar()->XtrTop(),9,'f',3);
        painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, strong);
        D += dD;
        strong = QString(tr("Forced Lower Trans. = %1")).arg(XDirect::curPolar()->XtrBot(), 9, 'f', 3);
        painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, strong);
        D += dD;

        if(XDirect::curOpp())
        {
            if(XDirect::curPolar()->polarType()!=Xfl::FIXEDSPEEDPOLAR)
            {
                ReynoldsFormat(Result, XDirect::curOpp()->Reynolds());
                Result = "Re = "+ Result;
                painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, Result);
                D += dD;
            }
            if(XDirect::curPolar()->polarType()==Xfl::FIXEDLIFTPOLAR)
            {
                Result = QString("Ma = %1").arg(XDirect::curOpp()->m_Mach, 9, 'f', 4);
                painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, Result);
                D += dD;
            }
            if(XDirect::curPolar()->polarType()!=Xfl::FIXEDAOAPOLAR)
            {
                Result = QString(tr("Alpha = %1")+QChar(0260)).arg(XDirect::curOpp()->m_Alpha, 8, 'f', 2);
                painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, Result);
                D += dD;
            }
            Result = QString(tr("Cl = %1")).arg(XDirect::curOpp()->Cl, 9, 'f', 3);
            painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, Result);
            D += dD;

            Result = QString(tr("Cm = %1")).arg(XDirect::curOpp()->Cm, 9, 'f', 3);
            painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, Result);
            D += dD;

            Result = QString(tr("Cd = %1")).arg(XDirect::curOpp()->Cd, 9, 'f', 3);
            painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, Result);
            D += dD;

            if(XDirect::curOpp()->m_bViscResults && qAbs(XDirect::curOpp()->Cd)>0.0)
            {
                Result = QString(tr("L/D = %1")).arg(XDirect::curOpp()->Cl/XDirect::curOpp()->Cd, 9, 'f', 3);
                painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, Result);
                D += dD;
            }

            Result = QString(tr("Upper Trans. = %1")).arg(XDirect::curOpp()->Xtr1, 9, 'f', 3);
            painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, Result);
            D += dD;

            Result = QString(tr("Lower Trans. = %1")).arg(XDirect::curOpp()->Xtr2, 9, 'f', 3);
            painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, Result);
            D += dD;

            if(XDirect::curOpp()->m_bTEFlap)
            {
                Result = QString(tr("TE Hinge Moment/span = %1")).arg(XDirect::curOpp()->m_TEHMom, 9, 'e', 2);
                painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, Result);
                D += dD;
            }

            if(XDirect::curOpp()->m_bLEFlap)
            {
                Result = QString(tr("LE Hinge Moment/span = %1")).arg(XDirect::curOpp()->m_LEHMom, 9, 'e', 2);
                painter.drawText(XPos,ZPos+D, dwidth, dD, Qt::AlignRight | Qt::AlignTop, Result);
                D += dD;
            }
        }
    }


    painter.restore();
}




/**
 * The method which draws the pressure arrows in the OpPoint view.
 * @param painter a reference to the QPainter object with which to draw
 * @param pOpPoint the OpPoint object to draw
 * @param scale the scale of the view
 */
void OpPointWidget::paintPressure(QPainter &painter, double scalex, double scaley)
{
    if(!XDirect::curFoil()) return;
    if(!XDirect::curOpp()) return;
    if(!XDirect::curOpp()->bViscResults()) return;

    double alpha = -XDirect::curOpp()->m_Alpha*PI/180.0;
    double cosa = cos(alpha);
    double sina = sin(alpha);
    double x, y ,xs, ys, xe, ye, dx, dy, x1, x2, y1, y2, r2;
    double cp;
    QPointF offset = m_FoilOffset;

    painter.save();

    QPen CpvPen(m_PressureStyle.m_Color);
    CpvPen.setStyle(getStyle(m_PressureStyle.m_Stipple));
    CpvPen.setWidth(m_PressureStyle.m_Width);
    painter.setPen(CpvPen);


    for(int i=0; i<XDirect::curFoil()->n; i++)
    {
        if(XDirect::curOpp()->m_bViscResults) cp = XDirect::curOpp()->Cpv[i];
        else                                  cp = XDirect::curOpp()->Cpi[i];
        x = XDirect::curFoil()->x[i];
        y = XDirect::curFoil()->y[i];

        xs = (x-0.5)*cosa - y*sina + 0.5;
        ys = (x-0.5)*sina + y*cosa;

        if(cp>0)
        {
            x += XDirect::curFoil()->nx[i] * cp * 0.05;
            y += XDirect::curFoil()->ny[i] * cp * 0.05;

            xe = (x-0.5)*cosa - y*sina + 0.5;
            ye = (x-0.5)*sina + y*cosa;
            painter.drawLine(int(xs*scalex + offset.x()), int(-ys*scaley + offset.y()),
                             int(xe*scalex + offset.x()), int(-ye*scaley + offset.y()));

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

            painter.drawLine( int(xs*scalex + offset.x()), int(-ys*scaley + offset.y()),
                              int(x1*scalex + offset.x()), int(-y1*scaley + offset.y()));

            painter.drawLine( int(xs*scalex + offset.x()), int(-ys*scaley + offset.y()),
                              int(x2*scalex + offset.x()), int(-y2*scaley + offset.y()));
        }
        else
        {

            x += -XDirect::curFoil()->nx[i] * cp *0.05;
            y += -XDirect::curFoil()->ny[i] * cp *0.05;

            xe = (x-0.5)*cosa - y*sina+ 0.5;
            ye = (x-0.5)*sina + y*cosa;
            painter.drawLine( int(xs*scalex + offset.x()), int(-ys*scaley + offset.y()),
                              int(xe*scalex + offset.x()), int(-ye*scaley + offset.y()));

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

            painter.drawLine( int(xe*scalex + offset.x()), int(-ye*scaley + offset.y()),
                              int(x1*scalex + offset.x()), int(-y1*scaley + offset.y()));

            painter.drawLine( int(xe*scalex + offset.x()), int(-ye*scaley + offset.y()),
                              int(x2*scalex + offset.x()), int(-y2*scaley + offset.y()));
        }
    }
    //last draw lift at XCP position
    QPen LiftPen(m_PressureStyle.m_Color);
    LiftPen.setStyle(getStyle(m_PressureStyle.m_Stipple));
    LiftPen.setWidth(m_PressureStyle.m_Width+1);
    painter.setPen(LiftPen);

    xs =  (XDirect::curOpp()->m_XCP-0.5)*cosa  + 0.5;
    ys = -(XDirect::curOpp()->m_XCP-0.5)*sina ;

    xe = xs;
    ye = ys - XDirect::curOpp()->Cl/10.0;

    painter.drawLine( int(xs*scalex + offset.x()), int(ys*scaley + offset.y()),
                      int(xs*scalex + offset.x()), int(ye*scaley + offset.y()));

    dx = xe - xs;
    dy = ye - ys;
    r2 = sqrt(dx*dx + dy*dy);
    dx = -dx/r2;
    dy = -dy/r2;

    x1 = xe + 0.0085*dx + 0.005*dy;
    y1 = ye + 0.0085*dy - 0.005*dx;
    x2 = xe + 0.0085*dx - 0.005*dy;
    y2 = ye + 0.0085*dy + 0.005*dx;

    painter.drawLine( int(xe*scalex + offset.x()), int(ye*scaley + offset.y()),
                      int(x1*scalex + offset.x()), int(y1*scaley + offset.y()));

    painter.drawLine( int(xe*scalex + offset.x()), int(ye*scaley + offset.y()),
                      int(x2*scalex + offset.x()), int(y2*scaley + offset.y()));

    painter.restore();
}


/**
 * The method which draws the boundary layer in the OpPoint view.
 * @param painter a reference to the QPainter object with which to draw
 * @param pOpPoint the OpPoint object to draw
 * @param scale the scale of the view
 */
void OpPointWidget::paintBL(QPainter &painter, OpPoint* pOpPoint, double scalex, double scaley)
{
    if(!XDirect::curFoil() || !pOpPoint) return;

    QPointF offset, From, To;
    double x=0,y=0;
    double alpha = -pOpPoint->aoa()*PI/180.0;
    double cosa = cos(alpha);
    double sina = sin(alpha);

    if(!pOpPoint->m_bViscResults || !pOpPoint->m_bBL) return;

    painter.save();

    offset = m_FoilOffset;

    QPen WakePen(m_BLStyle.m_Color);
    WakePen.setStyle(getStyle(m_BLStyle.m_Stipple));
    WakePen.setWidth(m_BLStyle.m_Width);

    painter.setPen(WakePen);

    x = (pOpPoint->blx.xd1[1]-0.5)*cosa - pOpPoint->blx.yd1[1]*sina + 0.5;
    y = (pOpPoint->blx.xd1[1]-0.5)*sina + pOpPoint->blx.yd1[1]*cosa;
    From.rx() =  x*scalex + offset.x();
    From.ry() = -y*scaley + offset.y();
    for (int i=2; i<=pOpPoint->blx.nd1; i++)
    {
        x = (pOpPoint->blx.xd1[i]-0.5)*cosa - pOpPoint->blx.yd1[i]*sina + 0.5;
        y = (pOpPoint->blx.xd1[i]-0.5)*sina + pOpPoint->blx.yd1[i]*cosa;
        To.rx() =  x*scalex + offset.x();
        To.ry() = -y*scaley + offset.y();
        painter.drawLine(From, To);
        From = To;
    }

    x = (pOpPoint->blx.xd2[0]-0.5)*cosa - pOpPoint->blx.yd2[0]*sina + 0.5;
    y = (pOpPoint->blx.xd2[0]-0.5)*sina + pOpPoint->blx.yd2[0]*cosa;
    From.rx() =  x*scalex + offset.x();
    From.ry() = -y*scaley + offset.y();
    for (int i=1; i<pOpPoint->blx.nd2; i++)
    {
        x = (pOpPoint->blx.xd2[i]-0.5)*cosa - pOpPoint->blx.yd2[i]*sina + 0.5;
        y = (pOpPoint->blx.xd2[i]-0.5)*sina + pOpPoint->blx.yd2[i]*cosa;
        To.rx() =  x*scalex + offset.x();
        To.ry() = -y*scaley + offset.y();
        painter.drawLine(From, To);
        From = To;
    }

    x = (pOpPoint->blx.xd3[0]-0.5)*cosa - pOpPoint->blx.yd3[0]*sina + 0.5;
    y = (pOpPoint->blx.xd3[0]-0.5)*sina + pOpPoint->blx.yd3[0]*cosa;
    From.rx() =  x*scalex + offset.x();
    From.ry() = -y*scaley + offset.y();
    for (int i=1; i<pOpPoint->blx.nd3; i++)
    {
        x = (pOpPoint->blx.xd3[i]-0.5)*cosa - pOpPoint->blx.yd3[i]*sina + 0.5;
        y = (pOpPoint->blx.xd3[i]-0.5)*sina + pOpPoint->blx.yd3[i]*cosa;
        To.rx() =  x*scalex + offset.x();
        To.ry() = -y*scaley + offset.y();
        painter.drawLine(From, To);
        From = To;
    }
    painter.restore();
}


/**
 * The user has requested the launch of the interface used to define the display style of the Foil
 */
void OpPointWidget::onXDirectStyle()
{
    XDirectStyleDlg xdsDlg(this);
    xdsDlg.m_BLStyle       = m_BLStyle;
    xdsDlg.m_PressureStyle = m_PressureStyle;
    xdsDlg.m_NeutralStyle   = m_NeutralStyle;

    if(xdsDlg.exec() == QDialog::Accepted)
    {
        m_BLStyle       = xdsDlg.m_BLStyle;
        m_PressureStyle = xdsDlg.m_PressureStyle;
        m_NeutralStyle  = xdsDlg.m_NeutralStyle;
    }
    update();
}


/**
 * Converts screen coordinates to viewport coordinates
 * @param point the screen coordinates
 * @return the viewport coordinates
 */
Vector3d OpPointWidget::mousetoReal(const QPoint &point)
{
    Vector3d Real;

    Real.x =  (point.x() - m_FoilOffset.x())/m_fScale;
    Real.y = -(point.y() - m_FoilOffset.y())/m_fScale;
    Real.z = 0.0;

    return Real;
}


/**
 * The user has requested to reset the scale of the foil to its automatic default value
 */
void OpPointWidget::onResetFoilScale()
{
    setFoilScale();
//    if(!m_bAnimate)
    update();
}


/**
 * The user has toggled the display of the neutral line y=0.
 */
void OpPointWidget::onShowNeutralLine()
{
    m_bNeutralLine = !m_bNeutralLine;
    s_pMainFrame->m_pShowNeutralLine->setChecked(m_bNeutralLine);
    update();
}


/**
 * The user has toggled the display of the Foil's panels.
 */
/*void OpPointWidget::onShowPanels()
{
    m_bShowPanels = !m_bShowPanels;
    MainFrame *pMainFrame = (MainFrame*)s_pMainFrame;
    pMainFrame->m_pShowPanels->setChecked(m_bShowPanels);
    update();
}*/


/**
 * The user has toggled the display of the pressure arrows
 */
void OpPointWidget::onShowPressure(bool bPressure)
{
    showPressure(bPressure);
    update();
}


/**
 * The user has toggled the display of the boundary layer
 */
void OpPointWidget::onShowBL(bool bBL)
{
    showBL(bBL);
    update();
}


void OpPointWidget::saveSettings(QSettings &settings)
{
    settings.beginGroup("OpPointSettings");
    {
        m_BLStyle.saveSettings(      settings, "BLStyle");
        m_PressureStyle.saveSettings(settings, "PressureStyle");
        m_NeutralStyle.saveSettings( settings, "NeutralStyle");
    }
    settings.endGroup();
}


void OpPointWidget::loadSettings(QSettings &settings)
{
    settings.beginGroup("OpPointSettings");
    {
        m_BLStyle.loadSettings(      settings, "BLStyle");
        m_PressureStyle.loadSettings(settings, "PressureStyle");
        m_NeutralStyle.loadSettings( settings, "NeutralStyle");
    }
    settings.endGroup();
}

