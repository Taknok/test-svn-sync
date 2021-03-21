/****************************************************************************

    OpPoint Class
    Copyright (C) 2003 Andre Deperrois 

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


/**
 *@file
 *
 * This class implements the surface object on which the panels are constructed for the VLM and 3d-panel calculations.
 *
 */


#pragma once

#include <QString>
#include <QTextStream>
#include <QDataStream>


#include <xfoil_params.h>
#include <xflobjects/objects2d/blxfoil.h>
#include <xflcore/ls2.h>

class Foil;
class Polar;

/**
*@class OpPoint
*@brief
 * The class which defines the operating point associated to Foil objects.

An OpPoint object stores the results of an XFoil calculation.
Each instance of an OpPoint is uniquely attached to a Polar object, which is itself attached uniquely to a Foil object.
The identification of the parent Polar and Foil are made using the QString names of the objects.
*/
class OpPoint
{
    friend class Polar;
    friend class XDirect;
    friend class OpPointWidget;

public:
    OpPoint();

    void setHingeMoments(const Foil *pFoil);

    void exportOpp(QTextStream &out, QString Version, bool bCSV, Foil *pFoil, bool bDataOnly=false) const;

    bool serializeOppWPA(QDataStream &ar, bool bIsStoring, int ArchiveFormat=0);
    bool serializeOppXFL(QDataStream &ar, bool bIsStoring);
    void getOppProperties(QString &OpPointProperties, Foil *pFoil, bool bData=false) const;


    QString foilName()  const {return m_FoilName;}
    QString polarName() const {return m_PlrName;}
    QString opPointName() const;

    void setFoilName(QString newFoilName) {m_FoilName = newFoilName;}
    void setPolarName(QString newPlrName) {m_PlrName = newPlrName;}

    bool bViscResults() const {return m_bViscResults;}
    bool bBL()          const {return m_bBL;}


    double aoa()      const {return m_Alpha;}
    double Reynolds() const {return m_Reynolds;}
    double Mach()     const {return m_Mach; }


    LS2 &theStyle() {return m_theStyle;}
    LS2 const &theStyle() const {return m_theStyle;}
    void setTheStyle(LS2 const &style) {m_theStyle=style;}
    void setTheStyle(Line::enumLineStipple stipple, int w, const QColor &clr, Line::enumPointStyle pointstyle);
    int width() const {return m_theStyle.m_Width;}
    int pointStyle() const {return m_theStyle.m_PointStyle;}
    Line::enumPointStyle pointStyle2() const {return m_theStyle.m_PointStyle;}

    void setLineStipple(Line::enumLineStipple s) {m_theStyle.m_Stipple=s;}
    void setLineWidth(int w)   {m_theStyle.m_Width=w;}

    void setPointStyle(int n) {m_theStyle.setPointStyle(n);}
    void setPointStyle2(Line::enumPointStyle pts) {m_theStyle.m_PointStyle=pts;}

    QColor const &color() const {return m_theStyle.m_Color;}
    void setColor(QColor const &clr) {m_theStyle.m_Color=clr;}
    void setColor(int r, int g, int b, int a) {m_theStyle.m_Color = {r,g,b,a};}

    int red() const {return m_theStyle.m_Color.red();}
    int green() const {return m_theStyle.m_Color.green();}
    int blue() const {return m_theStyle.m_Color.blue();}
    int alphaChannel() const {return m_theStyle.m_Color.alpha();}
    Line::enumLineStipple polarStyle() const     {return m_theStyle.m_Stipple;}
    int lineWidth() const     {return m_theStyle.m_Width;}
    bool isVisible() const     {return m_theStyle.m_bIsVisible;}

    void setStipple(int n) {m_theStyle.setStipple(n);} // conversion
    void setStipple2(Line::enumLineStipple s) {m_theStyle.m_Stipple=s;}
    void setWidth(int w) {m_theStyle.m_Width=w;}
    void setVisible(bool bVisible) {m_theStyle.m_bIsVisible=bVisible;}


public:
    bool m_bViscResults;        /**< true if viscous results are stored in this OpPoint */
    bool m_bBL;                 /**< true if a boundary layer is stored in this OpPoint */
    bool m_bTEFlap;             /**< true if the parent foil has a flap on the trailing edge */
    bool m_bLEFlap;             /**< true if the parent foil has a flap on the leading edge */

    LS2 m_theStyle;

    int n;                          /**< the number of foil surface points */


    double m_Reynolds;            /**< the Re number of the OpPoint */
    double m_Mach;                /**< the Mach number of the OpPoint */
    double m_Alpha;               /**< the aoa*/
    double Cl;                  /**< the lift coefficient */
    double Cm;                  /**< the pitching moment coefficient */
    double Cd;                  /**< the drag coefficient - viscous only, since we are dealing with 2D analysis */
    double Cdp;                 /**< @todo check significance in XFoil doc */
    double Xtr1;                /**< the laminar to turbulent transition point on the upper surface */
    double Xtr2;                /**< the laminar to turbulent transition point on the lower surface */
    double ACrit;               /**< the NCrit parameter which defines turbulent transition */
    double m_XCP;               /**< the x-position of the centre of pressure */

    double Cpv[IQX];            /**< the distribution of Cp on the surfaces for a viscous analysis */
    double Cpi[IQX];            /**< the distribution of Cp on the surfaces for an inviscid analysis */
    double Qv[IQX];             /**< the distribution of stream velocity on the surfaces for a viscous analysis */
    double Qi[IQX];             /**< the distribution of stream velocity on the surfaces for an inviscid analysis */

    double m_TEHMom;            /**< the moment on the foil's trailing edge flap */
    double m_LEHMom;            /**< the moment on the foil's leading edge flap */
    double XForce;              /**< the y-component of the pressure forces */
    double YForce;              /**< the y-component of the pressure forces */
    double Cpmn;                /**< @todo check significance in XFoil doc */

    BLXFoil blx;          /**< BL data from an XFoil analysis */


    QString m_FoilName;      /**< the name of the parent Foil */
    QString m_PlrName;       /**< the name of the parent Polar */
};

