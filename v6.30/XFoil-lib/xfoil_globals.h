
/****************************************************************************

	Global Functions
	Copyright (C) 2008-2016 Andre Deperrois adeperrois@xflr5.com

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
* @file
* This file contains the declaration of methods used throughout the program and not specific to one application.
*/


#ifndef XFOIL_GLOBALS_H
#define XFOIL_GLOBALS_H

#include <QFile>
#include <QList>
#include <QString>
#include <QTextStream>
#include <complex>
#include <xfoil_params.h>
#include <objects/CVector.h>
#include <objects/Foil.h>
#include <objects/Polar.h>


void readCString(QDataStream &ar, QString &strong);
void writeCString(QDataStream &ar, QString const &strong);
void readCOLORREF(QDataStream &ar, int &r, int &g, int &b);
void WriteCOLORREF(QDataStream &ar, int r, int g, int b);

void readqColor(QDataStream &ar, int &r, int &g, int &b, int &a);
void writeqColor(QDataStream &ar, int r, int g, int b, int a);



#endif // XFOIL_GLOBALS_H