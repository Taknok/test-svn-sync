/****************************************************************************

    FoilWt Class
    Copyright (C) 2015 Andre Deperrois

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
#include <QMessageBox>


#include "foilwt.h"
#include <xflcore/xflcore.h>
#include <globals/mainframe.h>
#include <graph/graph_globals.h>
#include <gui_objects/splinefoil.h>
#include <misc/options/settings.h>
#include <xdirect/xdirect.h>
#include <xflobjects/objects2d/foil.h>

FoilWt::FoilWt(QWidget *pParent) : Section2dWidget(pParent)
{
    m_bNeutralLine = true;

    m_pBufferFoil = nullptr;
    createContextMenu();
}


void FoilWt::setBufferFoil(Foil *pBufferFoil)
{
    m_pBufferFoil = pBufferFoil;
}


void FoilWt::setScale()
{
    //scale is set by user zooming
    m_fRefScale = rect().width()*6.0/8.0;
    m_fScale = m_fRefScale;


    m_ptOffset.rx() = rect().width()/8;
    m_ptOffset.ry() = rect().height()/2;

    m_ViewportTrans = QPoint(0,0);
}


void FoilWt::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.fillRect(rect(), Settings::s_BackgroundColor);

    drawScaleLegend(painter);
    drawBackImage(painter);


    paintGrids(painter);
    paintSplines(painter);
    paintFoils(painter);
    paintLegend(painter);
}



/**
 * Draws the SplineFoil object.
 * @param painter a reference to the QPainter object with which to draw.
 */
void FoilWt::paintSplines(QPainter &painter)
{
    painter.save();

    QPen CtrlPen;

    QBrush FillBrush(Settings::s_BackgroundColor);
    painter.setBrush(FillBrush);
    painter.restore();
}


/**
 * Draws the visible Foil objects.
 * @param painter a reference to the QPainter object with which to draw.
 */
void FoilWt::paintFoils(QPainter &painter)
{
    painter.save();
    QPen FoilPen, CenterPen, CtrlPen;

    QBrush FillBrush(Settings::s_BackgroundColor);
    painter.setBrush(FillBrush);

    for (int k=0; k<m_oaFoil.size(); k++)
    {
        Foil const*pFoil = m_oaFoil.at(k);
        if (pFoil->isVisible())
        {
            FoilPen.setStyle(getStyle(pFoil->lineStyle()));
            FoilPen.setWidth(pFoil->lineWidth());
            FoilPen.setColor(colour(pFoil));
            painter.setPen(FoilPen);

            drawFoil(painter, pFoil, 0.0, m_fScale, m_fScale*m_fScaleY,m_ptOffset);
            if (pFoil->bCenterLine())
            {
                CenterPen.setColor(colour(pFoil));
                CenterPen.setStyle(Qt::DashLine);
                painter.setPen(CenterPen);
                drawMidLine(painter, pFoil, m_fScale, m_fScale*m_fScaleY, m_ptOffset);
            }

            drawPoints(painter, pFoil, 0.0, m_fScale,m_fScale*m_fScaleY, m_ptOffset, Settings::s_BackgroundColor);
        }
    }
    if (m_pBufferFoil && m_pBufferFoil->isVisible())
    {
        drawFoil(painter, m_pBufferFoil, 0.0, m_fScale, m_fScale*m_fScaleY,m_ptOffset);

        if (m_pBufferFoil->bCenterLine())
        {
            CenterPen.setColor(colour(m_pBufferFoil));
            CenterPen.setStyle(Qt::DashLine);
            painter.setPen(CenterPen);
            drawMidLine(painter, m_pBufferFoil, m_fScale, m_fScale*m_fScaleY, m_ptOffset);
        }

        CtrlPen.setColor(colour(m_pBufferFoil));
        painter.setPen(CtrlPen);
        drawPoints(painter, m_pBufferFoil, 0.0, m_fScale,m_fScale*m_fScaleY, m_ptOffset, Settings::s_BackgroundColor);

    }
    painter.restore();
}


/**
 * Draws the legend.
 * @param painter a reference to the QPainter object with which to draw.
 */
void FoilWt::paintLegend(QPainter &painter)
{
    painter.save();

    painter.setFont(Settings::s_TextFont);

    if(m_bShowLegend)
    {
        QFont fnt(Settings::s_TextFont); //valgrind
        QFontMetrics fm(fnt);
        int fmw = fm.averageCharWidth();

        Foil const* pRefFoil;
        QString strong;
        QPoint Place(rect().right()-35*fmw, 10);

        int LegendSize = 10*fmw;
        int ypos = 15;
        int delta = 5;

        painter.setBackgroundMode(Qt::TransparentMode);

        QPen TextPen(Settings::s_TextColor);
        painter.setPen(TextPen);

        QBrush FillBrush(Settings::s_BackgroundColor);
        painter.setBrush(FillBrush);

        QPen LegendPen;

        int k=0;
        k++;

        for (int n=0; n<m_oaFoil.size(); n++)
        {
            pRefFoil = m_oaFoil.at(n);
            if(pRefFoil && pRefFoil->isVisible())
            {
                strong = pRefFoil->name();
                if(strong.length())
                {
                    LegendPen.setColor(colour(pRefFoil));
                    LegendPen.setStyle(getStyle(pRefFoil->lineStyle()));
                    LegendPen.setWidth(pRefFoil->lineWidth());

                    painter.setPen(LegendPen);
                    painter.drawLine(Place.x(), Place.y() + ypos*k, Place.x() + LegendSize, Place.y() + ypos*k);

                    int x1 = Place.x() + int(0.5*LegendSize);

                    drawPoint(painter, pRefFoil->pointStyle(), Settings::s_BackgroundColor, QPoint(x1, Place.y() + ypos*k));
                    painter.setPen(TextPen);
                    painter.drawText(Place.x() + LegendSize + fmw, Place.y() + ypos*k+delta, pRefFoil->name());
                    k++;
                }
            }
        }
    }
    painter.restore();
}


void FoilWt::resizeEvent (QResizeEvent *event)
{
    setScale();
    event->accept();
}











