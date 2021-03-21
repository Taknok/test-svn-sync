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

#include "ls2.h"


/** @deprecated use ls2 and conversion functions instead */
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

    // Conversion functions to the new struct
    void fromLS2(LS2 const &ls2)
    {
        m_bIsVisible = ls2.m_bIsVisible;
        m_Width      = ls2.m_Width;
        m_Color      = ls2.m_Color;

        switch(ls2.m_Stipple)
        {
            case Line::SOLID:      m_Stipple = 0;   break;
            case Line::DASH:       m_Stipple = 1;   break;
            case Line::DOT:        m_Stipple = 2;   break;
            case Line::DASHDOT:    m_Stipple = 3;   break;
            case Line::DASHDOTDOT: m_Stipple = 4;   break;
            case Line::NOLINE:     m_Stipple = 5;   break;
        }

        switch(ls2.m_PointStyle)
        {
            default:
            case Line::NOSYMBOL:       m_PointStyle = 0;    break;
            case Line::LITTLECIRCLE:   m_PointStyle = 1;    break;
            case Line::BIGCIRCLE:      m_PointStyle = 2;    break;
            case Line::LITTLESQUARE:   m_PointStyle = 3;    break;
            case Line::BIGSQUARE:      m_PointStyle = 4;    break;
        }
    }

    LS2 toLS2() const
    {
        LS2 ls2;
        ls2.m_bIsVisible = m_bIsVisible;
        ls2.m_Width      = m_Width;
        ls2.m_Color      = m_Color;
        ls2.m_Stipple    = lineStipple2();
        ls2.m_PointStyle = pointStyle2();
        return ls2;
    }

    Line::enumLineStipple lineStipple2() const
    {
        switch(m_Stipple)
        {
            default:
            case 0: return Line::SOLID;
            case 1: return Line::DASH;
            case 2: return Line::DOT;
            case 3: return Line::DASHDOT;
            case 4: return Line::DASHDOTDOT;
            case 5: return Line::NOLINE;
        }
    }


    Line::enumPointStyle pointStyle2() const
    {
        switch(m_PointStyle)
        {
            default:
            case 0: return Line::NOSYMBOL;
            case 1: return Line::LITTLECIRCLE;
            case 2: return Line::BIGCIRCLE;
            case 3: return Line::LITTLESQUARE;
            case 4: return Line::BIGSQUARE;
        }
    }

    void loadSettings(QSettings &settings, QString const &name)
    {
        if(settings.contains(name+"_visible")) m_bIsVisible = settings.value(name+"_visible", true).toBool();
        if(settings.contains(name+"_color"))   m_Color      = settings.value(name+"_color", QColor(205,205,205)).value<QColor>();
        if(settings.contains(name+"_width"))   m_Width      = settings.value(name+"_width", 1).toInt();
        if(settings.contains(name+"_line"))    m_Stipple    = settings.value(name+"_style", 0).toInt();
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
    int m_Stipple=0;              /**< the index of the style with which to draw the curve >*/
    int m_Width=1;                /**< the width with which to draw the curve >*/
    QColor m_Color=Qt::black;     /**< the color with which to draw the curve >*/

    int m_PointStyle=0;        /**< defines the point display. O = no points, 1 = small circles, 2 = large circles,3 = small squares, 4 = large squares >*/
};


