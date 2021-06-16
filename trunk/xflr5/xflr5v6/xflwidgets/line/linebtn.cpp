/****************************************************************************

    LineBtn Class
    Copyright (C) 2014 Andre Deperrois 

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


#include <QMouseEvent>
#include <QPen>
#include <QPainter>

#include <misc/options/settings.h>
#include "linebtn.h"
#include <xflcore/xflcore.h>
#include <xflgraph/graph_globals.h>


LineBtn::LineBtn(QWidget *parent)
    : QAbstractButton(parent)
{
    QSizePolicy szPolicyExpanding;
    szPolicyExpanding.setHorizontalPolicy(QSizePolicy::MinimumExpanding);
    szPolicyExpanding.setVerticalPolicy(QSizePolicy::Minimum);
    setSizePolicy(szPolicyExpanding);

    m_LineStyle = {true, Line::SOLID, 1, Qt::darkGray, Line::NOSYMBOL};
}


void LineBtn::mouseReleaseEvent(QMouseEvent *pEvent)
{
    if (pEvent->button() == Qt::LeftButton)
    {
        emit clickedLB();
    }
    else
        QWidget::mouseReleaseEvent(pEvent);
}


QSize LineBtn::sizeHint() const
{
    QFontMetrics fm(Settings::s_TextFont);
    int w = 7 * fm.averageCharWidth();
    int h = fm.height();
    return QSize(w, h);
}


void LineBtn::setColor(QColor const & color)
{
    m_LineStyle.m_Color = color;
    update();
}


void LineBtn::setStipple(int style)
{
    m_LineStyle.setStipple(style);
    update();
}


void LineBtn::setWidth(int width)
{
    m_LineStyle.m_Width = width;
    update();
}


void LineBtn::setPointStyle(int pointStyle)
{
    m_LineStyle.setPointStyle(pointStyle);
    update();
}


void LineBtn::setTheStyle(int style, int width, QColor const & color, int pointStyle)
{
    m_LineStyle.setStipple(style);
    m_LineStyle.m_Width = width;
    m_LineStyle.m_Color = color;
    m_LineStyle.setPointStyle(pointStyle);
    update();
}


void LineBtn::paintEvent(QPaintEvent *pEvent)
{
    QPainter painter(this);
    painter.save();

    QRect r = rect();

    if(isEnabled())
    {
    //    painter.setBrush(Qt::DiagCrossPattern);
        painter.setBackgroundMode(Qt::TransparentMode);

        QPen LinePen(m_LineStyle.m_Color);
        LinePen.setStyle(getStyle(m_LineStyle.m_Stipple));
        LinePen.setWidth(m_LineStyle.m_Width);
        painter.setPen(LinePen);
        painter.drawLine(r.left()+5, r.height()/2, r.width()-5, r.height()/2);
    }

    painter.restore();
    pEvent->accept();
}
