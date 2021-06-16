/****************************************************************************

    LineCbBox Class
    Copyright (C) 2009-2019 Andre Deperrois

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

#include <iostream>

#include <QtDebug>
#include <QPainter>
#include <QPaintEvent>


#include "linecbbox.h"

#include <xflcore/xflcore.h>
#include <xflgraph/graph_globals.h>
#include <misc/options/settings.h>



LineCbBox::LineCbBox(QWidget *pParent)
    :QComboBox(pParent)
{
    setParent(pParent);
    m_LineStyle = {true, Line::SOLID, 1, QColor(255,100,50), Line::NOSYMBOL};
    m_bShowPoints = false;

    QSizePolicy szPolicyExpanding;
    szPolicyExpanding.setHorizontalPolicy(QSizePolicy::MinimumExpanding);
    szPolicyExpanding.setVerticalPolicy(QSizePolicy::Minimum);
    setSizePolicy(szPolicyExpanding);
}


QSize LineCbBox::sizeHint() const
{
    QFontMetrics fm(Settings::s_TextFont);
    int w = 7 * fm.averageCharWidth();
    int h = fm.height();
    return QSize(w, h);
}


void LineCbBox::setLine(int const &style, int const &width, QColor const &color, int const &pointStyle)
{
    m_LineStyle.setStipple(style);
    m_LineStyle.m_Width = width;
    m_LineStyle.m_Color = color;
    m_LineStyle.setPointStyle(pointStyle);
}


void LineCbBox::setLine(const LS2 &lineStyle)
{
    m_LineStyle = lineStyle;
}


void LineCbBox::paintEvent (QPaintEvent *)
{
    QStyleOption opt;
    opt.initFrom(this);

    QPainter painter(this);
//    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

    painter.save();
    QColor clr = m_LineStyle.m_Color;
    if(!isEnabled())
    {
        QPalette pal;
        clr = pal.color(QPalette::Disabled, QPalette::Button);
    }

    QRect r = opt.rect;

    painter.setBrush(Qt::NoBrush);
    painter.setBackgroundMode(Qt::TransparentMode);

    QPen LinePen(clr);
    LinePen.setStyle(getStyle(m_LineStyle.m_Stipple));
    LinePen.setWidth(m_LineStyle.m_Width);
    painter.setPen(LinePen);
    painter.drawLine(r.left()+5, r.center().y(), r.width()-10, r.center().y());

    if(m_bShowPoints)
    {
        LinePen.setStyle(Qt::SolidLine);
        painter.setPen(LinePen);

        QPalette palette;
        drawPoint(painter, m_LineStyle.m_PointStyle, palette.window().color(), r.center());
    }

    painter.restore();
}

