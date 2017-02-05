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

#ifndef ENGINE_GLOBALS_H
#define ENGINE_GLOBALS_H


#include <QFile>
#include <QList>
#include <QString>
#include <QTextStream>
#include <complex>


#include <analysis3d_params.h>
#include <Vector3d.h>
#include <objects2d/Foil.h>
#include <objects2d/Polar.h>
#include <objects3d/PointMass.h>
#include <objects3d/Body.h>
#include <analysis3d_enums.h>


using namespace std;
/*
void readCString(QDataStream &ar, QString &strong);
void writeCString(QDataStream &ar, QString const &strong);
void readCOLORREF(QDataStream &ar, QColor &color);
void WriteCOLORREF(QDataStream &ar, QColor const &color);
void WriteCOLORREF(QDataStream &ar, int r, int g, int b);

void readqColor(QDataStream &ar, int &r, int &g, int &b, int &a);
void writeqColor(QDataStream &ar, int r, int g, int b, int a);
bool ReadAVLString(QTextStream &in, int &Line, QString &strong);
*/

bool IsEven(int n);
bool IsBetween(int f, int f1, int f2);
bool IsBetween(int f, double f1, double f2);


bool Gauss(double *A, int n, double *B, int m, bool *pbCancel);

double det33(double *aij);
complex<double> det33(complex<double> *aij);


double det44(double *aij);

complex<double> det44(complex<double> *aij);
complex<double> cofactor44(complex<double> *aij, int &i, int &j);

bool Invert44(complex<double> *ain, complex<double> *aout);




void TestEigen();
void CharacteristicPol(double m[][4], double p[5]);
bool LinBairstow(double *p, complex<double> *root, int n);
bool Eigenvector(double a[][4], complex<double> lambda, complex<double> *V);


bool Crout_LU_Decomposition_with_Pivoting(double *A, int pivot[], int n, bool *pbCancel, double TaskSize, double &Progress);
bool Crout_LU_with_Pivoting_Solve(double *LU, double B[], int pivot[], double x[], int n, bool *pbCancel);

bool Crout_LU_Decomposition_with_Pivoting(float *A, int pivot[], int n, bool *pbCancel, double TaskSize, double &Progress);
bool Crout_LU_with_Pivoting_Solve(float *LU, double B[], int pivot[], double x[], int Size, bool *pbCancel);


double GetPlrPointFromCl(Foil *pFoil, double Re, double Cl, int PlrVar, bool &bOutRe, bool &bError);
double GetPlrPointFromAlpha(Foil *pFoil, double Re, double Alpha, int PlrVar, bool &bOutRe, bool &bError);

bool SplineInterpolation(int n, double *x, double *y,  double *a, double *b, double *c, double *d);
double splineBlend(int const &index, int const &p, double const &t, double *knots);


int Compare(complex<double> a, complex<double>b);
void ComplexSort(complex<double>*array, int ub);



#endif // ENGINE_GLOBALS_H
 
