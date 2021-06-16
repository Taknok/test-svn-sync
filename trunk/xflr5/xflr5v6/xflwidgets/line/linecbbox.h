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


#pragma once

#include <QComboBox>
#include <xflcore/ls2.h>



class LineCbBox : public QComboBox
{
    public:
        LineCbBox(QWidget *pParent=nullptr);

        QSize sizeHint() const override;
        void paintEvent (QPaintEvent *pEvent) override;

        void setLine(int const &style, int const &width, QColor const &color, const int &pointStyle);
        void setLine(LS2 const &lineStyle);
        void showPoints(bool bShow){m_bShowPoints=bShow;}

        bool points() const {return m_bShowPoints; }

    private:
        LS2 m_LineStyle;
        bool m_bShowPoints;
};

