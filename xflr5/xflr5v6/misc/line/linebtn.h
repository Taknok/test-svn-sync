/****************************************************************************

    LineBtn Class
    Copyright (C) 2013 Andre Deperrois 

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



#pragma once


#include <QAbstractButton>
#include <xflcore/ls2.h>

class LineBtn : public QAbstractButton
{
    Q_OBJECT

    public:
        LineBtn(QWidget *parent = nullptr);

        void setTheStyle(int lineStyle, int width, QColor const & color, int pointStyle);
        void setTheStyle(LS2 const &ls) {m_LineStyle = ls;}
        LS2 const &theStyle() const {return m_LineStyle;}

        void setColor(QColor const & color);
        void setStipple(int lineStyle);
        void setWidth(int width);
        void setPointStyle(int pointStyle);

        QColor color()   const {return m_LineStyle.m_Color;}
        int lineStyle()  const {return m_LineStyle.m_Stipple;}
        int lineWidth()  const {return m_LineStyle.m_Width;}
        int pointStyle() const {return m_LineStyle.m_PointStyle;}

    signals:
        void clickedLB();

    public:
        void mouseReleaseEvent(QMouseEvent *pEvent) override;
        void paintEvent(QPaintEvent *pEvent) override;
        QSize sizeHint() const override;

    private:
        LS2 m_LineStyle;
};

