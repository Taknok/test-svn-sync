/****************************************************************************

	Objects2D    Copyright (C) 2016-2016 Andre Deperrois adeperrois@xflr5.com

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
#ifndef OBJECTS2D_H
#define OBJECTS2D_H

/**
  * @file This file implements the variables and methods used to manage 3D objects
  */

#include <QList>
#include <objects2d/Foil.h>
#include <objects2d/Polar.h>
#include <objects2d/OpPoint.h>

class Objects2D
{
public:
	Objects2D();

	static Foil *    addFoil(Foil *pFoil);
	static void      addPolar(Polar *pPolar);
	static void      deleteObjects();
	static void      deleteFoil(Foil *pFoil);
	static void      deleteFoilResults(Foil *pFoil, bool bDeletePolars=false);
	static void      deletePolar(Polar *pPolar);
	static Foil *    duplicateFoil(Foil *pFoil);
	static Foil*     getFoil(QString FoilName);
	static OpPoint*  getFoilOpp(Foil *pFoil, Polar *pPolar, double x);
	static Polar*    getPolar(Foil *pFoil, QString PolarName);
	static void      insertPOpp(OpPoint *pPOpp);
	static Polar *   insertNewPolar(Polar *pModPolar, Foil *pCurFoil);
	static bool      FoilExists(QString FoilName);
	static void      renameFoil(QString FoilName);
	static Foil *    setModFoil(Foil *pModFoil);
	static void      setStaticPointers();

public:
	// object variable lists

	static QList <void *> s_oaFoil;   /**< The array of void pointers to the Foil objects. */
	static QList <void *> s_oaPolar;  /**< The array of void pointers to the Polar objects. */
	static QList <void *> s_oaOpp;    /**< The array of void pointers to the OpPoint objects. */
};

#endif // OBJECTS2D_H
