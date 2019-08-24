/****************************************************************************

    LineStyle Class
    Copyright (C) 2009-2018 Andre Deperrois

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

#include <QColor>
#include <QSettings>
#include <QDataStream>

struct LineStyle
{
    LineStyle()
    {
        m_bIsVisible=true;
        m_Stipple=0;
        m_Width=1;
        m_Color = Qt::gray;
        m_PointStyle=0;
    }

    LineStyle(bool bVisible, int style, int width, QColor color, int pointstyle)
    {
        m_bIsVisible = bVisible;
        m_Stipple = style;
        m_Width = width;
        m_Color = color;
        m_PointStyle = pointstyle;
    }


    void loadSettings(QSettings &settings, QString const &name)
    {
        if(settings.contains(name+"_visible")) m_bIsVisible = settings.value(name+"_visible", true).toBool();
        if(settings.contains(name+"_color"))   m_Color      = settings.value(name+"_color", QColor(205,205,205)).value<QColor>();
        if(settings.contains(name+"_width"))   m_Width      = settings.value(name+"_width", 1).toInt();
        if(settings.contains(name+"_line"))    m_Stipple      = settings.value(name+"_style", 0).toInt();
        if(settings.contains(name+"_pts"))     m_PointStyle = settings.value(name+"_pts", 0).toInt();

    }

    void saveSettings(QSettings &settings, QString const &name) const
    {
        settings.setValue(name+"_visible", m_bIsVisible);
        settings.setValue(name+"_color",   m_Color);
        settings.setValue(name+"_width",   m_Width);
        settings.setValue(name+"_style",   m_Stipple);
        settings.setValue(name+"_pts",     m_PointStyle);
    }

    void serializeXfl(QDataStream &ar, bool bIsStoring)
    {
        if(bIsStoring)
        {
            ar << m_Stipple;
            ar << m_Width;
            ar << m_PointStyle;
            ar << m_Color;
            ar << m_bIsVisible;
        }
        else
        {
            ar >> m_Stipple;
            ar >> m_Width;
            ar >> m_PointStyle;
            ar >> m_Color;
            ar >> m_bIsVisible;
        }
    }


public:
    bool m_bIsVisible=true;       /**< true if the curve is visible in the active view >*/
    int m_Stipple=0;             /**< the index of the style with which to draw the curve >*/
    int m_Width=1;             /**< the width with which to draw the curve >*/
    QColor m_Color=Qt::black;          /**< the color with which to draw the curve >*/

    int m_PointStyle=0;        /**< defines the point display. O = no points, 1 = small circles, 2 = large circles,3 = small squares, 4 = large squares >*/
};


