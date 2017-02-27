/****************************************************************************

	Techwing Application

	Copyright (C) Andre Deperrois adeperrois@xflr5.com

	All rights reserved.

*****************************************************************************/

#ifndef MATRIX_H
#define MATRIX_H

#include <objects2d/Vector3d.h>
#include <complex>

using namespace std;

typedef Vector3d Vector;


void transpose33(double *l);
bool invert33(double *l);
void AV33(double *A, double *v, double *p);


double det33(double *aij);
complex<double> det33(complex<double> *aij);


double det44(double *aij);

complex<double> det44(complex<double> *aij);
complex<double> cofactor44(complex<double> *aij, int &i, int &j);

bool Invert44(complex<double> *ain, complex<double> *aout);


bool Gauss(double *A, int n, double *B, int m, bool *pbCancel);


bool Crout_LU_Decomposition_with_Pivoting(double *A, int pivot[], int n, bool *pbCancel, double TaskSize, double &Progress);
bool Crout_LU_with_Pivoting_Solve(double *LU, double B[], int pivot[], double x[], int n, bool *pbCancel);

bool Crout_LU_Decomposition_with_Pivoting(float *A, int pivot[], int n, bool *pbCancel, double TaskSize, double &Progress);
bool Crout_LU_with_Pivoting_Solve(float *LU, double B[], int pivot[], double x[], int Size, bool *pbCancel);


void TestEigen();
void CharacteristicPol(double m[][4], double p[5]);
bool LinBairstow(double *p, complex<double> *root, int n);
bool Eigenvector(double a[][4], complex<double> lambda, complex<double> *V);


int Compare(complex<double> a, complex<double>b);
void ComplexSort(complex<double>*array, int ub);



#endif // MATRIX_H
