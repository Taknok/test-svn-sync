/****************************************************************************

    Global functions

    Copyright (C) 2008-2017 Andre Deperrois

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

#pragma once

#include <QString>
#include <QColor>
#include <QFile>
#include <QPainter>


#include <xflcore/core_enums.h>
#include <xflobjects/objectcolor.h>


using namespace std;

class Foil;
class Polar;
class OpPoint;
class Plane;
class WPolar;

void findFiles(const QString &startDir, QStringList filters, bool bRecursive, QStringList &filepathnames);
bool findFile(QString const &filename, QString const &startDir, QStringList filters, bool bRecursive, QString &filePathName);

int readValues(QString line, double &x, double &y, double &z);

void ExpFormat(double &f, int &exp);
void ReynoldsFormat(QString &str, double f);


float GLGetRed(float tau);
float GLGetGreen(float tau);
float GLGetBlue(float tau);


QColor randomColor(bool bLightColor);


Xfl::enumPanelDistribution distributionType(QString const &strDist);
QString distributionType(Xfl::enumPanelDistribution dist);

Xfl::enumBodyLineType bodyPanelType(QString const &strPanelType);
QString bodyPanelType(Xfl::enumBodyLineType panelType);


Xfl::enumPolarType polarType(QString const &strPolarType);
QString polarType(Xfl::enumPolarType polarType);

Xfl::enumPolarType WPolarType(QString const &strPolarType);
QString WPolarType(Xfl::enumPolarType polarType);


Xfl::enumAnalysisMethod analysisMethod(QString const &strAnalysisMethod);
QString analysisMethod(Xfl::enumAnalysisMethod analysisMethod);


Xfl::enumBC boundaryCondition(QString const &strBC);
QString boundaryCondition(Xfl::enumBC boundaryCondition);

Foil *readFoilFile(QFile &xFoilFile);
Foil *readPolarFile(QFile &plrFile, QVector<Polar*> &polarList);

void drawFoil(QPainter &painter, const Foil *pFoil, double alpha, double scalex, double scaley, QPointF const &Offset);
void drawMidLine(QPainter &painter, Foil const*pFoil, double scalex, double scaley, QPointF const &Offset);
void drawPoints(QPainter &painter, Foil const*pFoil, double alpha, double scalex, double scaley, QPointF const &Offset, QColor const &backColor);
void drawPoint(QPainter &painter, int pointStyle, QColor const &bkColor, QPoint const &pt);

bool stringToBool(QString str);
QString referenceDimension(Xfl::enumRefDimension refDimension);
Xfl::enumRefDimension referenceDimension(QString const &strRefDimension);

Xfl::enumWingType wingType(QString const &strWingType);
QString wingType(Xfl::enumWingType wingType);

void setAutoWPolarName(WPolar * pWPolar, Plane *pPlane);


void ReynoldsFormat(QString &str, double f);

QColor getColor(int r, int g, int b, int a=255);
QColor colour(const OpPoint *pOpp);
QColor colour(const Polar *pPolar);
QColor colour(const Foil *pFoil);
void setRandomFoilColor(Foil *pFoil, bool bLightTheme);




void readString(QDataStream &ar, QString &strong);
void writeString(QDataStream &ar, QString const &strong);
void readColor(QDataStream &ar, int &r, int &g, int &b);
void writeColor(QDataStream &ar, int r, int g, int b);

void readColor(QDataStream &ar, int &r, int &g, int &b, int &a);
void writeColor(QDataStream &ar, int r, int g, int b, int a);


bool serializeFoil(Foil*pFoil, QDataStream &ar, bool bIsStoring);
bool serializePolar(Polar *pPolar, QDataStream &ar, bool bIsStoring);

QColor color(ObjectColor clr);
