/****************************************************************************

    Objects2D    Copyright (C) 2016-2019 Andre Deperrois

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

/**
  * @file This file implements the variables and methods used to manage 3D objects
  */

#include <QVector>

#include <xflcore/core_enums.h>


class Foil;
class Polar;
class OpPoint;

namespace Objects2d
{
    // object arrays
    extern QVector<Foil *> s_oaFoil;   /**< The array of pointers to the Foil objects. */
    extern QVector<Polar *> s_oaPolar;  /**< The array of pointers to the Polar objects. */
    extern QVector<OpPoint *> s_oaOpp;    /**< The array of pointers to the OpPoint objects. */

    void      deleteObjects();

    void      deleteFoilResults(Foil *pFoil, bool bDeletePolars=false);

    Foil*     duplicateFoil(Foil *pFoil);
    Foil*     foil(const QString &strFoilName);
    Foil*     foilAt(int iFoil);
    Foil*     deleteFoil(Foil *pFoil);
    Foil*     deleteThisFoil(Foil *pFoil);
    void      deleteAllFoils();
    void      insertThisFoil(Foil *pFoil);
    Foil *    addFoil(Foil *pFoil);
    inline void appendFoil(Foil *pFoil) {s_oaFoil.append(pFoil);}
    bool      foilExists(QString const &FoilName, Qt::CaseSensitivity cs=Qt::CaseInsensitive);
    void      renameFoil(QString const &FoilName);
    void      renameThisFoil(Foil *pFoil, QString const &newFoilName);
    Foil *    setModFoil(Foil *pModFoil);

    void      addPolar(Polar *pPolar);
    inline void appendPolar(Polar *pPolar) {s_oaPolar.append(pPolar);}
    Polar*    getPolar(Foil const *pFoil, QString const &PolarName);
    Polar*    getPolar(QString const &FoilName, QString const &PolarName);
    Polar*    polarAt(int index);
    void      deletePolar(Polar *pPolar);
    void      deletePolarAt(int index);

    OpPoint*  oppAt(int index);
    OpPoint*  getOpp(Foil *pFoil, Polar *pPolar, double Alpha);
    OpPoint*  getFoilOpp(Foil *pFoil, Polar *pPolar, double x);
    void      insertOpPoint(OpPoint *pNewPoint);
    inline void appendOpp(OpPoint *pOpp) {s_oaOpp.append(pOpp);}
    bool      deleteOpp(OpPoint *pOpp);
    void      deleteOppAt(int index);
    OpPoint*  addOpPoint(const Foil *pFoil, Polar *pPolar, OpPoint *pOpPoint, bool bStoreOpp);

    inline int foilCount() {return s_oaFoil.size();}
    inline int polarCount() {return s_oaPolar.size();}
    inline int oppCount() {return s_oaOpp.size();}

    void setFoilChildrenStyle(Foil *pFoil);
    void setPolarChildrenStyle(const Polar *pPolar);

    inline QVector<Foil*> * pOAFoil() {return &s_oaFoil;}
    inline QVector<Polar*> * pOAPolar() {return &s_oaPolar;}
    inline QVector<OpPoint*> * pOAOpp() {return &s_oaOpp;}

    Polar* createPolar(Foil *pFoil, xfl::enumPolarType PolarType, double Spec, double Mach, double NCrit, double m_XTop, double m_XBot);
};

