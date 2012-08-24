/****************************************************************************

	AboutQ5 Class
	Copyright (C) 2008-2009 Andre Deperrois xflr5@yahoo.com

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

//Global functions

#ifndef GLOBALS_H
#define GLOBALS_H

#include <Qt> 
#include <QFile>
#include <QString>
#include <QTextStream>
#include <QColor>
#include <complex>
#include "Params.h"
#include "Objects/CVector.h"
#include "Objects/Foil.h"

using namespace std;

bool IsEven(int n);
bool IsBetween(int f, int f1, int f2);
bool IsBetween(int f, double f1, double f2);
bool ReadAVLString(QTextStream &in, int &Line, QString &strong);
bool Rewind1Line(QTextStream &in, int &Line, QString &strong);

bool ludcmp(int n, double *a, int *indx, bool *pbCancel=false);
bool baksub(int n, double *a, int *indx, double *b,           bool *pbCancel=false);
bool baksub(int n, double *a, int *indx, double *RHS, int nb, bool *pbCancel=false);

void ReadValues(QString line, int &res, double &x, double &y, double &z);

void ExpFormat(double &f, int &exp);
void ReynoldsFormat(QString &str, double f);

bool Gauss(double *A, int n, double *B, int m, bool *pbCancel);

void GetSpeedUnit(QString &str, int unit);
void GetWeightUnit(QString &str, int unit);
void GetAreaUnit(QString &str, int unit);
void GetLengthUnit(QString &str, int unit);
void GetMomentUnit(QString &str, int unit);
void GetForceUnit(QString &str, int unit);

double GLGetRed(double tau);
double GLGetGreen(double tau);
double GLGetBlue(double tau);

double IntegralC2(double y1, double y2, double c1, double c2);
double IntegralCy(double y1, double y2, double c1, double c2);

void ReadCString(QDataStream &ar, QString &strong);
void WriteCString(QDataStream &ar, QString const &strong);
void ReadCOLORREF(QDataStream &ar, QColor &color);
void WriteCOLORREF(QDataStream &ar, QColor const &color);
void VLMQmn(CVector const &LA, CVector const &LB, CVector const &TA, CVector const &TB, CVector const &C, CVector &V);
void VLMCmn(CVector const &A, CVector const &B, CVector const &C, CVector &V, bool bAll=true);

void Trace(int n);
void Trace(QString msg);
void Trace(QString msg, int n);
void Trace(QString msg, double f);

void SetUnits(int LUnit, int AUnit, int SUnit, int WUnit, int FUnit, int MUnit,
			  double &mtoUnit, double &m2toUnit, double &mstoUnit,  double &kgtoUnit,
			  double &NtoUnit, double &NmtoUnit);

Qt::PenStyle GetStyle(int s);


double Det44(double *aij);
double Det33(double *aij);
complex<double> Det33(complex<double> *aij);
complex<double> Det44(complex<double> *aij);
complex<double> Cofactor44(complex<double> *aij, int &i, int &j);
bool Invert44(complex<double> *ain, complex<double> *aout);


void TestEigen();
void CharacteristicPol(double m[][4], double p[5]);
bool LinBairstow(double *p, complex<double> *root, int n);
bool Eigenvector(double a[][4], complex<double> lambda, complex<double> *V);


#endif // FUNCTIONS_H