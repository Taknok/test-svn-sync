/****************************************************************************

	AboutQ5 Class
	Copyright (C) 2008-2008 Andre Deperrois xflr5@yahoo.com

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


#include "Globals.h"
#include <QPen>
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QDateTime> 
#include <QByteArray>
#include <math.h>
#include <QtDebug>


void ExpFormat(double &f, int &exp)
{
	if (f==0.0)
	{
		exp = 0;
		f = 0.0;
		return;
	}
	double f1 = fabs(f);
//	int sgn = int(f/f1);
	if(f1<1)
		exp = (int)log10(f1)-1;
	else
		exp = (int)log10(f1);

	f = f/pow(10.0,exp);

}



bool Gauss(double *A, int n, double *B, int m, bool *pbCancel)
{
	int row, i, j, pivot_row, k;
	double max, dum, *pa, *pA, *A_pivot_row;
	// for each variable find pivot row and perform forward substitution
	pa = A;
	for (row = 0; row < (n - 1); row++, pa += n)
	{
//		qApp->processEvents();
		if(*pbCancel) return false;
		//  find the pivot row
		A_pivot_row = pa;
		max = fabs(*(pa + row));
		pA = pa + n;
		pivot_row = row;
		for (i=row+1; i < n; pA+=n, i++)
		{
			if ((dum = fabs(*(pA+row))) > max)
			{
				max = dum;
				A_pivot_row = pA;
				pivot_row = i;
			}
		}
		if (max <= 0.0)
		{
			return false;                // the matrix A is singular
		}
			// and if it differs from the current row, interchange the two rows.

		if (pivot_row != row)
		{
			for (i = row; i < n; i++)
			{
				dum = *(pa + i);
				*(pa + i) = *(A_pivot_row + i);
				*(A_pivot_row + i) = dum;
			}
			for(k=0; k<=m; k++)
			{
				dum = B[row+k*n];
				B[row+k*n] = B[pivot_row+k*n];
				B[pivot_row+k*n] = dum;
			}
		}

		// Perform forward substitution
		for (i = row+1; i<n; i++)
		{
			pA = A + i * n;
			dum = - *(pA + row) / *(pa + row);
			*(pA + row) = 0.0;
			for (j=row+1; j<n; j++) *(pA+j) += dum * *(pa + j);
			for (k=0; k<=m; k++)
				B[i+k*n] += dum * B[row+k*n];
		}
	}
	// Perform backward substitution

	pa = A + (n - 1) * n;
	for (row = n - 1; row >= 0; pa -= n, row--)
	{
//		qApp->processEvents();
		if(*pbCancel) return false;

		if ( *(pa + row) == 0.0 )
		{
			return false;           // matrix is singular
		}
		dum = 1.0 / *(pa + row);
		for ( i = row + 1; i < n; i++) *(pa + i) *= dum;
		for(k=0; k<=m; k++) B[row+k*n] *= dum;
		for ( i = 0, pA = A; i < row; pA += n, i++)
		{
			dum = *(pA + row);
			for ( j = row + 1; j < n; j++) *(pA + j) -= dum * *(pa + j);
			for(k=0; k<=m; k++)
				B[i+k*n] -= dum * B[row+k*n];
		}
	}
	return true;
}



void GetAreaUnit(QString &str, int unit)
{
	switch(unit)
	{
		case 0:
		{
			str="mm2";
			break;
		}
		case 1:
		{
			str="cm2";
			break;
		}
		case 2:
		{
			str="dm2";
			break;
		}
		case 3:
		{
			str="m2";
			break;
		}
		case 4:
		{
			str="in2";
			break;
		}
		case 5:
		{
			str="ft2";
			break;
		}
		default:
		{
			str=" ";
			break;
		}
	}
}
void GetLengthUnit(QString &str, int unit)
{
	switch(unit)
	{
		case 0:
		{
			str="mm";
			break;
		}
		case 1:
		{
			str="cm";
			break;
		}
		case 2:
		{
			str="dm";
			break;
		}
		case 3:
		{
			str="m";
			break;
		}
		case 4:
		{
			str="in";
			break;
		}
		case 5:
		{
			str="ft";
			break;
		}
		default:
		{
			str=" ";
			break;
		}
	}
}

void GetForceUnit(QString &str, int unit)
{
	switch(unit){
		case 0:{
			str="N.m";
			break;
		}
		case 1:{
			str="lbf";
			break;
		}

		default:{
			str=" ";
			break;
		}
	}
}

void GetMomentUnit(QString &str, int unit)
{
	switch(unit)
	{
		case 0:
		{
			str="N.m";
			break;
		}
		case 1:
		{
			str="lbf.in";
			break;
		}
		case 2:
		{
			str="lbf.ft";
			break;
		}
		default:
		{
			str=" ";
			break;
		}
	}
}


void GetSpeedUnit(QString &str, int unit)
{
	switch(unit){
		case 0:{
			str="m/s";
			break;
		}
		case 1:{
			str="km/h";
			break;
		}
		case 2:{
			str="ft/s";
			break;
		}
		case 3:{
			str="kt";
			break;
		}
		case 4:{
			str="mph";
			break;
		}
		default:{
			str=" ";
			break;
		}
	}
}


void GetWeightUnit(QString &str, int unit)
{
	switch(unit){
		case 0:{
			str="g";
			break;
		}
		case 1:{
			str="kg";
			break;
		}
		case 2:{
			str="oz";
			break;
		}
		case 3:{
			str="lb";
			break;
		}
		default:{
			str=" ";
			break;
		}
	}
}

bool IsBetween(int f, int f1, int f2)
{
	if (f2 < f1)
	{
		int tmp = f2;
		f2 = f1;
		f1 = tmp;
	}
	if(f<f1) return false;
	else if(f>f2) return false;
	return true;
}


bool IsEven(int n)
{
	if(n%2==0) return true;
	else return false;
}

bool IsBetween(int f, double f1, double f2)
{
	double ff = f;
	if (f2 < f1)
	{
		double tmp = f2;
		f2 = f1;
		f1 = tmp;
	}
	if(ff<f1)       return false;
	else if(ff>f2) return false;
	return true;
}


Qt::PenStyle GetStyle(int s)
{
	if(s==0)      return Qt::SolidLine;
	else if(s==1) return Qt::DashLine;
	else if(s==2) return Qt::DotLine;
	else if(s==3) return Qt::DashDotLine;
	else if(s==4) return Qt::DashDotDotLine;
	return Qt::SolidLine;
}



bool ReadAVLString(QTextStream &in, int &Line, QString &strong)
{
	bool bComment = true;
	int pos;
	QString str;

	while(bComment && !in.atEnd())
	{
		bComment = false;

		strong = in.readLine();
//		bRead = pXFile->ReadString(strong);
		if(in.atEnd()) return false;


		pos = strong.indexOf("#",0);
		if(pos==0)		bComment = true;

		pos = strong.indexOf("!",0);
		if(pos==0)		bComment = true;

		str = strong;
		str.replace(" ", "");
		if(str.isEmpty())	bComment = true;

		Line++;
	}

	if(in.atEnd())
	{
		return false;
	}
	return true;
}


void ReadCOLORREF(QDataStream &ar, QColor &color)
{
	qint32 colorref;
	int r,g,b;

	ar >> colorref;
	b = (int)(colorref/256/256);
	colorref -= b*256*256;
	g = (int)(colorref/256);
	r = colorref - g*256;
	color = QColor(r,g,b,255);

}

void ReadCString(QDataStream &ar, QString &strong)
{
	qint8 qi, ch;
	char c;

	ar >> qi;
	strong.clear();
	for(int j=0; j<qi;j++)
	{
		strong += " ";
		ar >> ch;
		c = char(ch);
		strong[j] = c;
	}
}


bool Rewind1Line(QTextStream &in, int &Line, QString &strong)
{
	int length = strong.length() * 1+2;//1 char takes one byte in the file ?

	int pos = in.pos();
	in.seek(pos-length);
	Line--;
	return true;
}


void ReynoldsFormat(QString &str, double f)
{
	int i, q, r, exp;
	exp = (int)log10(f);
	r = exp%3;
	q = (exp-r)/3;

	QString strong;
	strong = QString("%1").arg(f,0,'f',0);

	int l = strong.length();

	for (i=0; i<q; i++){
		strong.insert(l-3*(i+1)-i," ");
		l++;
	}

	for (i=strong.length(); i<9; i++){
		strong = " "+strong;
	}

	str = strong;
}


void SetUnits(int LUnit, int AUnit, int SUnit, int WUnit, int FUnit, int MUnit,
			  double &mtoUnit, double &m2toUnit, double &mstoUnit,  double &kgtoUnit, double &NtoUnit, double &NmtoUnit)
{
	switch(LUnit)
	{
		case 0:
		{//mdm
			mtoUnit  = 1000.0;
			break;
		}
		case 1:{//cm
			mtoUnit  = 100.0;
			break;
		}
		case 2:{//dm
			mtoUnit  = 10.0;
			break;
		}
		case 3:{//m
			mtoUnit  = 1.0;
			break;
		}
		case 4:{//in
			mtoUnit  = 1000.0/25.4;
			break;
		}
		case 5:{///ft
			mtoUnit  = 1000.0/25.4/12.0;
			break;
		}
		default:{//m
			mtoUnit  = 1.0;
			break;
		}
	}
	switch(AUnit)
	{
		case 0:{//mm²
			m2toUnit = 1000000.0;
			break;
		}
		case 1:{//cm²
			m2toUnit = 10000.0;
			break;
		}
		case 2:{//dm²
			m2toUnit = 100.0;
			break;
		}
		case 3:{//m²
			m2toUnit = 1.0;
			break;
		}
		case 4:{//in²
			m2toUnit = 1./0.254/0.254*100.0;
			break;
		}
		case 5:{//ft²
			m2toUnit = 1./0.254/0.254/144.0*100.0;
			break;
		}
		default:{
			m2toUnit = 1.0;
			break;
		}
	}

	switch(WUnit){
		case 0:{///g
			kgtoUnit = 1000.0;
			break;
		}
		case 1:{//kg
			kgtoUnit = 1.0;

			break;
		}
		case 2:{//oz
			kgtoUnit = 1./ 2.83495e-2;
			break;
		}
		case 3:{//lb
			kgtoUnit = 1.0/0.45359237;
			break;
		}
		default:{
			kgtoUnit = 1.0;
			break;
		}
	}
	switch(SUnit){
		case 0:{// m/s
			mstoUnit = 1.0;
			break;
		}
		case 1:{// km/h
			mstoUnit = 3600.0/1000.0;
			break;
		}
		case 2:{// ft/s
			mstoUnit = 100.0/2.54/12.0;
			break;
		}
		case 3:{// kt (int.)
			mstoUnit = 1.0/0.514444;
			break;
		}
		case 4:{// mph
			mstoUnit = 3600.0/1609.344;
			break;
		}
		default:{
			mstoUnit = 1.0;
			break;
		}
	}

	switch(FUnit){
		case 0:{//N
			NtoUnit = 1.0;
			break;
		}
		case 1:{//lbf
			NtoUnit = 1.0/4.44822;
			break;
		}
		default:{
			NtoUnit = 1.0;
			break;
		}
	}
	switch(MUnit){
		case 0:{//N.m
			NmtoUnit = 1.0;
			break;
		}
		case 1:{//lbf.in
			NmtoUnit = 1.0/4.44822/0.0254;
			break;
		}
		case 2:{//lbf.0t
			NmtoUnit = 1.0/4.44822/12.0/0.0254;
			break;
		}
		default:{
			NmtoUnit = 1.0;
			break;
		}
	}
}


void Trace(int n)
{
	QString FileName = QDir::tempPath() + "/Trace.log";
	QFile *tf = new QFile(FileName);
	if (!tf->open(QIODevice::ReadWrite | QIODevice::Text)) return;

	QString strong;
	QTextStream ts(tf);
	while(!ts.atEnd()) ts>>strong;


	QDateTime dt = QDateTime::currentDateTime();
	QString str = dt.toString("dd.MM.yyyy  hh:mm:ss");
	ts << str << n;

	tf->close();

}


void Trace(QString msg)
{
//	if(!((CXFLR5App*)AfxGetApp())->bTrace) return;
	QString FileName = QDir::tempPath() + "/Trace.log";
	QFile *tf = new QFile(FileName);
	if (!tf->open(QIODevice::ReadWrite | QIODevice::Text)) return;

	QString strong;
	QTextStream ts(tf);
	while(!ts.atEnd()) ts>>strong;



	QDateTime dt = QDateTime::currentDateTime();
	QString str = dt.toString("dd.MM.yyyy  hh:mm:ss");
	str += msg;
	str += "\n";

	ts << str;

	tf->close();

}


void Trace(QString msg, int n)
{
//	if(!((CXFLR5App*)AfxGetApp())->bTrace) return;
	QString FileName = QDir::tempPath() + "/Trace.log";
	QFile *tf = new QFile(FileName);
	if (!tf->open(QIODevice::ReadWrite | QIODevice::Text)) return;

	QString strong;
	QTextStream ts(tf);
	while(!ts.atEnd()) ts>>strong;



	strong = QString("  %1").arg(n);
	strong = msg + strong;

	QDateTime dt = QDateTime::currentDateTime();
	QString str = dt.toString("dd.MM.yyyy  hh:mm:ss");
	str += strong;
	str += "\n";
	ts << str;;
	tf->close();
}


void Trace(QString msg, double f)
{
//	if(!((CXFLR5App*)AfxGetApp())->bTrace) return;
	QString FileName = QDir::tempPath() + "/Trace.log";
	QFile *tf = new QFile(FileName);
	if (!tf->open(QIODevice::ReadWrite | QIODevice::Text)) return;

	QString strong;
	QTextStream ts(tf);
	while(!ts.atEnd()) ts>>strong;


	strong = QString("  %1").arg(f);
	strong = msg + strong;


	QDateTime dt = QDateTime::currentDateTime();
	QString str = dt.toString("dd.MM.yyyy  hh:mm:ss");

	str += strong;
	str += "\n";
	ts << str;


	tf->close();
}

void VLMQmn(CVector const &LA, CVector const &LB, CVector const &TA, CVector const &TB, CVector const &C, CVector &V)
{
	//Quadrilateral VLM FORMULATION
	// LA, LB, TA, TB are the vortex's four corners
	// LA and LB are at the 3/4 point of panel nx
	// TA and TB are at the 3/4 point of panel nx+1
	//
	//    LA__________LB               |
	//    |           |                |
	//    |           |                | freestream speed
	//    |           |                |
	//    |           |                \/
	//    |           |
	//    TA__________TB
	//
	//
	//
	// C is the point where the induced speed is calculated
	// V is the resulting speed
	//
	// Vectorial operations are written explicitly to save computing times (4x more efficient)
	//
	double CoreSize = 0.000001;
//	if(fabs(*m_pCoreSize)>1.e-10) CoreSize = *m_pCoreSize;

	static int i;
	static CVector R[5];
	static CVector r0, r1, r2, Psi, t;
	static double r1v,r2v,ftmp, Omega;

	V.x = 0.0;
	V.y = 0.0;
	V.z = 0.0;

	R[0].x = LB.x;
	R[0].y = LB.y;
	R[0].z = LB.z;
	R[1].x = TB.x;
	R[1].y = TB.y;
	R[1].z = TB.z;
	R[2].x = TA.x;
	R[2].y = TA.y;
	R[2].z = TA.z;
	R[3].x = LA.x;
	R[3].y = LA.y;
	R[3].z = LA.z;
	R[4].x = LB.x;
	R[4].y = LB.y;
	R[4].z = LB.z;

	for (i=0; i<4; i++)
	{
		r0.x = R[i+1].x - R[i].x;
		r0.y = R[i+1].y - R[i].y;
		r0.z = R[i+1].z - R[i].z;
		r1.x = C.x - R[i].x;
		r1.y = C.y - R[i].y;
		r1.z = C.z - R[i].z;
		r2.x = C.x - R[i+1].x;
		r2.y = C.y - R[i+1].y;
		r2.z = C.z - R[i+1].z;

		Psi.x = r1.y*r2.z - r1.z*r2.y;
		Psi.y =-r1.x*r2.z + r1.z*r2.x;
		Psi.z = r1.x*r2.y - r1.y*r2.x;

		ftmp = Psi.x*Psi.x + Psi.y*Psi.y + Psi.z*Psi.z;

		r1v = sqrt((r1.x*r1.x + r1.y*r1.y + r1.z*r1.z));
		r2v = sqrt((r2.x*r2.x + r2.y*r2.y + r2.z*r2.z));

		//get the distance of the TestPoint to the panel's side
		t.x =  r1.y*r0.z - r1.z*r0.y;
		t.y = -r1.x*r0.z + r1.z*r0.x;
		t.z =  r1.x*r0.y - r1.y*r0.x;

		if ((t.x*t.x+t.y*t.y+t.z*t.z)/(r0.x*r0.x+r0.y*r0.y+r0.z*r0.z) > CoreSize * CoreSize)
		{
			Psi.x /= ftmp;
			Psi.y /= ftmp;
			Psi.z /= ftmp;

			Omega = (r0.x*r1.x + r0.y*r1.y + r0.z*r1.z)/r1v - (r0.x*r2.x + r0.y*r2.y + r0.z*r2.z)/r2v;
			V.x += Psi.x * Omega/4.0/PI;
			V.y += Psi.y * Omega/4.0/PI;
			V.z += Psi.z * Omega/4.0/PI;
		}
	}
}



void VLMCmn(CVector const &A, CVector const &B, CVector const &C, CVector &V, bool bAll)
{
	// CLASSIC VLM FORMULATION/
	//
	//    LA__________LB               |
	//    |           |                |
	//    |           |                | freestream speed
	//    |           |                |
	//    |           |                \/
	//    |           |
	//    \/          \/
	//
	// Note : the geometry has been rotated by sideslip, hence, there is no need to align the trailing vortices with sideslip
	//
	// V is the resulting speed at point C
	//
	// Vectorial operations are written inline to save computing times
	// -->longer code, but 4x more efficient....

	static CVector r0, r1, r2, Psi, t, h, Far;
	static double  ftmp, Omega;
	static double CoreSize = 0.000001;
//	if(fabs(*m_pCoreSize)>1.e-10) CoreSize = *m_pCoreSize;

	V.Set(0.0,0.0,0.0);

	if(bAll)
	{
		r0.x = B.x - A.x;
		r0.y = B.y - A.y;
		r0.z = B.z - A.z;

		r1.x = C.x - A.x;
		r1.y = C.y - A.y;
		r1.z = C.z - A.z;

		r2.x = C.x - B.x;
		r2.y = C.y - B.y;
		r2.z = C.z - B.z;

		Psi.x = r1.y*r2.z - r1.z*r2.y;
		Psi.y =-r1.x*r2.z + r1.z*r2.x;
		Psi.z = r1.x*r2.y - r1.y*r2.x;

		ftmp = Psi.x*Psi.x + Psi.y*Psi.y + Psi.z*Psi.z;

		//get the distance of the TestPoint to the panel's side
		t.x =  r1.y*r0.z - r1.z*r0.y;
		t.y = -r1.x*r0.z + r1.z*r0.x;
		t.z =  r1.x*r0.y - r1.y*r0.x;

		if ((t.x*t.x+t.y*t.y+t.z*t.z)/(r0.x*r0.x+r0.y*r0.y+r0.z*r0.z) >CoreSize * CoreSize)
		{
			Psi.x /= ftmp;
			Psi.y /= ftmp;
			Psi.z /= ftmp;

			Omega = (r0.x*r1.x + r0.y*r1.y + r0.z*r1.z)/sqrt((r1.x*r1.x + r1.y*r1.y + r1.z*r1.z))
				   -(r0.x*r2.x + r0.y*r2.y + r0.z*r2.z)/sqrt((r2.x*r2.x + r2.y*r2.y + r2.z*r2.z));

			V.x = Psi.x * Omega/4.0/PI;
			V.y = Psi.y * Omega/4.0/PI;
			V.z = Psi.z * Omega/4.0/PI;
		}
	}

	// We create Far points to align the trailing vortices with the reference axis
	// The trailing vortex legs are not aligned with the free-stream, i.a.w. the small angle approximation
	// If this approximation is not valid, then the geometry should be tilted in the polar definition

	// calculate left contribution
	Far.x = A.x +  10000.0; //10 km... should be enough
	Far.y = A.y;
	Far.z = A.z;// + (Far.x-A.x) * tan(m_Alpha*PI/180.0);

	r0.x = A.x - Far.x;
	r0.y = A.y - Far.y;
	r0.z = A.z - Far.z;

	r1.x = C.x - A.x;
	r1.y = C.y - A.y;
	r1.z = C.z - A.z;

	r2.x = C.x - Far.x;
	r2.y = C.y - Far.y;
	r2.z = C.z - Far.z;

	Psi.x = r1.y*r2.z - r1.z*r2.y;
	Psi.y =-r1.x*r2.z + r1.z*r2.x;
	Psi.z = r1.x*r2.y - r1.y*r2.x;

	ftmp = Psi.x*Psi.x + Psi.y*Psi.y + Psi.z*Psi.z;

	t.x=1.0; t.y=0.0; t.z=0.0;

	h.x =  r1.y*t.z - r1.z*t.y;
	h.y = -r1.x*t.z + r1.z*t.x;
	h.z =  r1.x*t.y - r1.y*t.x;

	//Next add 'left' semi-infinite contribution
	//eq.6-56

	if ((h.x*h.x+h.y*h.y+h.z*h.z) > CoreSize * CoreSize)
	{
		Psi.x /= ftmp;
		Psi.y /= ftmp;
		Psi.z /= ftmp;

		Omega =  (r0.x*r1.x + r0.y*r1.y + r0.z*r1.z)/sqrt((r1.x*r1.x + r1.y*r1.y + r1.z*r1.z))
				-(r0.x*r2.x + r0.y*r2.y + r0.z*r2.z)/sqrt((r2.x*r2.x + r2.y*r2.y + r2.z*r2.z));

		V.x += Psi.x * Omega/4.0/PI;
		V.y += Psi.y * Omega/4.0/PI;
		V.z += Psi.z * Omega/4.0/PI;
	}

	// calculate right vortex contribution
	Far.x = B.x +  10000.0;
	Far.y = B.y ;
	Far.z = B.z;// + (Far.x-B.x) * tan(m_Alpha*PI/180.0);

	r0.x = Far.x - B.x;
	r0.y = Far.y - B.y;
	r0.z = Far.z - B.z;

	r1.x = C.x - Far.x;
	r1.y = C.y - Far.y;
	r1.z = C.z - Far.z;

	r2.x = C.x - B.x;
	r2.y = C.y - B.y;
	r2.z = C.z - B.z;

	Psi.x = r1.y*r2.z - r1.z*r2.y;
	Psi.y =-r1.x*r2.z + r1.z*r2.x;
	Psi.z = r1.x*r2.y - r1.y*r2.x;

	ftmp = Psi.x*Psi.x + Psi.y*Psi.y + Psi.z*Psi.z;

	//Last add 'right' semi-infinite contribution
	//eq.6-57
	h.x =  r2.y*t.z - r2.z*t.y;
	h.y = -r2.x*t.z + r2.z*t.x;
	h.z =  r2.x*t.y - r2.y*t.x;

	if ((h.x*h.x+h.y*h.y+h.z*h.z) > CoreSize * CoreSize)
	{
		Psi.x /= ftmp;
		Psi.y /= ftmp;
		Psi.z /= ftmp;

		Omega =  (r0.x*r1.x + r0.y*r1.y + r0.z*r1.z)/sqrt((r1.x*r1.x + r1.y*r1.y + r1.z*r1.z))
				-(r0.x*r2.x + r0.y*r2.y + r0.z*r2.z)/sqrt((r2.x*r2.x + r2.y*r2.y + r2.z*r2.z));

		V.x += Psi.x * Omega/4.0/PI;
		V.y += Psi.y * Omega/4.0/PI;
		V.z += Psi.z * Omega/4.0/PI;
	}
}



void WriteCOLORREF(QDataStream &ar, QColor const &color)
{
	qint32 colorref;
	int r,g,b;

	color.getRgb(&r,&g,&b);

	colorref = b*256*256+g*256+r;
	ar << colorref;
}



void WriteCString(QDataStream &ar, QString const &strong)
{
	qint8 qi = strong.length();

	QByteArray textline;
	char *text;
	textline = strong.toAscii();
	text = textline.data();

	ar << qi;
	ar.writeRawData(text, qi);
}



double GLGetRed(double tau)
{
	if(tau>2.0/3.0)       return 1.0;
	else if(tau>1.0/3.0)  return (3.0*(tau-1.0/3.0));
	else				  return 0.0;
}

double GLGetGreen(double tau)
{
	if(tau<0.f || tau>1.0) 	return 0.0;
	else if(tau<1.0/4.0) 	return (4.0*tau);
	else if(tau>3.0/4.0) 	return (1.0-4.0*(tau-3.0/4.0));
	else					return 1.0;
}

double GLGetBlue(double tau)
{
	if(tau>2.0/3.0) 		return 0.0;
	else if(tau>1.0/3.0)	return (1.0-3.0*(tau-1.0/3.0));
	else		    		return 1.0;
}


double IntegralC2(double y1, double y2, double c1, double c2)
{
	// Auxiliary integral used in LLT calculations

	double res = 0.0;

	if (fabs(y2-y1)<1.e-5) return 0.0;
	double g = (c2-c1)/(y2-y1);

	res = (c1-g*y1)*(c1-g*y1)*(y2-y1) +
		  g * (c1-g*y1)      *(y2*y2-y1*y1)+
		  g*g/3.0            *(y2*y2*y2-y1*y1*y1);

	return res;
}


double IntegralCy(double y1, double y2, double c1, double c2)
{
	// Auxiliary integral used in LLT calculations

	double res = 0.0;
	if (fabs(y2-y1)<1.e-5) return (y1+y2)/2.0 * (c1+c2)/2.0;

	double g = (c2-c1)/(y2-y1);
	res = (c1-g*y1)/2.0 *(y2*y2 - y1*y1) + g/3.0 * (y2*y2*y2-y1*y1*y1);
	return res;
}


void ReadValues(QString line, int &res, double &x, double &y, double &z)
{
/*	char *sx = new char[30];
	char *sy = new char[30];
	char *text;*/
	QString str;
	bool bOK;

	line = line.simplified();
	int pos = line.indexOf(" ");
	res = 0;
	if(pos>0)
	{
		str = line.left(pos);
		line = line.right(line.length()-pos);
	}
	else
	{
		str = line;
		line = "";
	}
	x = str.toDouble(&bOK);
	if(bOK) res++;
	else
	{
		y=z=0.0;
		return;
	}

	line = line.trimmed();
	pos = line.indexOf(" ");
	if(pos>0)
	{
		str = line.left(pos);
		line = line.right(line.length()-pos);
	}
	else
	{
		str = line;
		line = "";
	}
	y = str.toDouble(&bOK);
	if(bOK) res++;
	else
	{
		z=0.0;
		return;
	}

	line = line.trimmed();
	if(!line.isEmpty())
	{
		z = line.toDouble(&bOK);
		if(bOK) res++;
	}
	else z=0.0;
}




bool ludcmp(int n, double *a, int *indx, bool *pbCancel)
{
	//    *******************************************************
	//    *                                                     *
	//    *   factors a full nxn matrix into an lu form.        *
	//    *   subr. baksub can back-substitute it with some rhs.*
	//    *   assumes matrix is non-singular...                 *
	//    *    ...if it isn"t, a divide by zero will result.    *
	//    *                                                     *
	//    *   a is the matrix...                                *
	//    *     ...replaced with its lu factors.                *
	//    *                                                     *
	//    *                              mark drela  1988       *
	//    *                                                     *
	//    *  Translated to C - Andre Deperrois  2003-2009       *
	//    *******************************************************
	//
	//
	bool bimaxok = false;
	int imax = -1;
	int i, j, k;
	static double vv[VLMMATSIZE];//use temporary global array
	double dum, sum, aamax;

	if(n>IQX)
	{
//		QString str("Stop ludcmp: array overflow. Increase nvx");
//		WriteString(str, true);
		return false;
	}

	for (i=1; i<=n; i++)
	{
		aamax = 0.0;
		for (j=1; j<=n; j++) aamax = qMax(fabs(*(a+i*n+j)), aamax);
		vv[i] = 1.0/aamax;
	}

	for(j=1;j<= n;j++)
	{
//		qApp->processEvents();
		if(*pbCancel) return false;
		for(i=1; i<=j-1; i++)
		{
			sum = *(a+i*n+j);
			for (k=1;k<= i-1;k++) sum -= *(a+i*n+k)* *(a+k*n+j);
			*(a+i*n+j) = sum;
		}

		aamax = 0.0;

		for (i=j; i<=n;i++)
		{
			sum = *(a+i*n+j);
			for (k=1;k<= j-1;k++) sum -= *(a+i*n+k)* *(a+k*n+j);
			*(a+i*n+j) = sum ;
			dum = (vv[i]*fabs(sum));
			if(dum>=aamax)
			{
				imax = i;
				aamax = dum;
				bimaxok = true;
			}
		}

		if(imax<0) return false;

		if(j!=imax)
		{
			for (k=1; k<= n; k++)
			{
				dum = *(a+imax*n+k);
				*(a+imax*n+k) = *(a+j*n+k);
				*(a+j*n+k)    = dum;
			}
			vv[imax] = vv[j];
		}

		indx[j] = imax;
		if(j!=n)
		{
			dum /= *(a+j*n+j);
			for(i=j+1; i<= n;i++) *(a+i*n+j) *= dum;
		}
	}
	return true;
}


bool baksub(int n, double *a, int *indx, double *b, bool *pbCancel)
{
	double sum;
	int i, ii, ll, j;
	ii = 0;
	for (i=1;i<= n;i++)
	{
//		qApp->processEvents();
		if(*pbCancel) return false;
		ll = indx[i];
		sum = b[ll];
		b[ll] = b[i];
		if(ii!=0)  
			for (j=ii; j<=i-1; j++) sum -= *(a+i*n+j) * b[j];
		else
			if(sum!=0.0) ii = i;

		b[i] = sum;
	}

	for (i=n; i>=1; i--)
	{
//		qApp->processEvents();
		if(*pbCancel) return false;
		sum = b[i];
		if(i<n)
		for (j=i+1; j<=n; j++) sum -= *(a+i*n+j) * b[j];
		b[i] = sum/ *(a+i*n+i);
	}
	return true;
}


bool baksub(int n, double *a, int *indx, double *RHS, int nb, bool *pbCancel)
{
	//same as baksub, except that we solve for nb RHS simultaneously

	double sum;
	int i, ii, ll, j, kb;
	ii = 0;
	for (i=1;i<= n;i++)
	{
//		qApp->processEvents();
		if(*pbCancel) return false;
		ll = indx[i];

		for(kb=0; kb<nb; kb++)
		{
			sum = RHS[ll+kb*n];
			RHS[ll+kb*n] = RHS[i+kb*n];
			if(ii!=0)  
				for (j=ii; j<=i-1; j++) sum -= *(a+i*n+j) * RHS[j+kb*n];
			else
				if(sum!=0.0) ii = i;

			RHS[i+kb*n] = sum;
		}
	}

	for (i=n; i>=1; i--)
	{
//		qApp->processEvents();
		if(*pbCancel) return false;

		for(kb=0; kb<nb; kb++)
		{
			sum = RHS[i+kb*n];
			if(i<n)
			for (j=i+1; j<=n; j++) sum -= *(a+i*n+j) * RHS[j+kb*n];
			RHS[i+kb*n] = sum/ *(a+i*n+i);
		}
	}
	return true;
}


double Det44(double *aij)
{
//	returns the determinant of a 4x4 matrix

	static int i,j,k,l,p,q;
	static double det, sign, a33[9];

	det = 0.0;
	for(i=0; i<4; i++)
	{
		for(j=0; j<4; j++)
		{
			p = 0;
			for(k=0; k<4 && k!=i; k++)
			{
				q = 0;
				for(l=0; l<4 && l!=j; l++)
				{
					*(a33+p*3+q) = *(aij+4*k+l);// could also do it by address, to be a little faster
					q++;
				}
				p++;
			}
			sign = pow(-1.0,i+j);
			det += sign * Det33(a33);
		}
	}
	return det;
}


double Det33(double *aij)
{
	//returns the determinant of a 3x3 matrix

	double det;

	det  = aij[0]*aij[4]*aij[8];
	det -= aij[0]*aij[5]*aij[7];

	det -= aij[1]*aij[3]*aij[8];
	det += aij[1]*aij[5]*aij[6];

	det += aij[2]*aij[3]*aij[7];
	det -= aij[2]*aij[4]*aij[6];

	return det;
}



complex<double> Det33(complex<double> *aij)
{
	//returns the determinant of a 3x3 matrix
	complex<double> det;

	det  = aij[0]*aij[4]*aij[8];
	det -= aij[0]*aij[5]*aij[7];

	det -= aij[1]*aij[3]*aij[8];
	det += aij[1]*aij[5]*aij[6];

	det += aij[2]*aij[3]*aij[7];
	det -= aij[2]*aij[4]*aij[6];

	return det;
}


complex<double> Cofactor44(complex<double> *aij, int &i, int &j)
{
	//returns the complex cofactor	of element i,j, in the 4x4 matrix aij
	static int k,l,p,q;
	static complex<double> a33[9];

	p = 0;
	for(k=0; k<4; k++)
	{
		if(k!=i)
		{
			q = 0;
			for(l=0; l<4; l++)
			{
				if(l!=j)
				{
					a33[p*3+q] = *(aij+4*k+l);
					q++;
				}
			}
			p++;
		}
	}
	return Det33(a33);
}

complex<double> Det44(complex<double> *aij)
{
//	returns the determinant of a 4x4 matrix

	static int i,j,k,l,p,q;
	static double sign;
	static complex<double> det, a33[9];
	det = 0.0;

	i=0;
	for(j=0; j<4; j++)
	{
		p = 0;
		for(k=0; k<4; k++)
		{
			if(k!=i)
			{
				q = 0;
				for(l=0; l<4; l++)
				{
					if(l!=j)
					{
						a33[p*3+q] = aij[4*k+l];
						q++;
					}
				}
				p++;
			}
		}
		sign = pow(-1.0,i+j);
		det += sign * aij[4*i+j] * Det33(a33);
	}

	return det;
}


bool Invert44(complex<double> *ain, complex<double> *aout)
{
	//small size, use the direct method
	static int i,j;
	static complex<double> det;
	static double sign;
	complex<double>nada;
	det = Det44(ain);

	if(abs(det)<PRECISION) return false;

	for(i=0; i<4; i++)
	{
		for(j=0; j<4; j++)
		{
			sign = pow(-1.0,i+j);
			aout[4*j+i] = sign * Cofactor44(ain, i, j)/det;
		}
	}
	return true;
}






void CharacteristicPol(double m[][4], double p[5])
{
	// input : 4x4 matrix m[][]
	// output : coefficients p[] of the matrix characteristic polynomial
	// thanks mathlab !
	// polynom can then be solved for complex roots using Bairstow's algorithm


	// lambda^4
	p[4] =  1;

	// lambda^3
	p[3] =  - m[0][0]- m[1][1]-m[2][2]- m[3][3];

	// lambda^2
	p[2] =
		+ m[0][0] * m[1][1]
		+ m[0][0] * m[2][2]
		+ m[0][0] * m[3][3]
		+ m[1][1] * m[3][3]
		+ m[1][1] * m[2][2]
		+ m[2][2] * m[3][3]
		- m[1][0] * m[0][1]
		- m[2][1] * m[1][2]
		- m[2][0] * m[0][2]
		- m[2][3] * m[3][2]
		- m[3][1] * m[1][3]
		- m[3][0] * m[0][3];

	// lambda^1
	p[1] =
		+ m[2][1] * m[1][2] * m[3][3]
		+ m[0][0] * m[2][1] * m[1][2]
		- m[3][1] * m[1][2] * m[2][3]
		+ m[3][1] * m[1][3] * m[2][2]
		+ m[1][0] * m[0][1] * m[3][3]
		+ m[1][0] * m[0][1] * m[2][2]
		- m[2][0] * m[0][1] * m[1][2]
		- m[1][0] * m[3][1] * m[0][3]
		- m[1][0] * m[2][1] * m[0][2]
		+ m[3][0] * m[1][1] * m[0][3]
		- m[3][0] * m[0][1] * m[1][3]
		+ m[2][0] * m[0][2] * m[3][3]
		- m[2][0] * m[3][2] * m[0][3]
		+ m[2][0] * m[1][1] * m[0][2]
		- m[3][0] * m[0][2] * m[2][3]
		+ m[3][0] * m[0][3] * m[2][2]
		- m[2][1] * m[3][2] * m[1][3]
		- m[0][0] * m[1][1] * m[2][2]
		+ m[0][0] * m[2][3] * m[3][2]
		+ m[1][1] * m[2][3] * m[3][2]
		- m[0][0] * m[2][2] * m[3][3]
		+ m[0][0] * m[3][1] * m[1][3]
		- m[1][1] * m[2][2] * m[3][3]
		- m[0][0] * m[1][1] * m[3][3];

	// lambda^0
	p[0] =
		+ m[2][0] * m[0][1] * m[1][2] * m[3][3]
		- m[2][0] * m[1][1] * m[0][2] * m[3][3]
		+ m[2][0] * m[1][1] * m[3][2] * m[0][3]
		- m[2][0] * m[0][1] * m[3][2] * m[1][3]
		+ m[1][0] * m[3][1] * m[0][3] * m[2][2]
		- m[1][0] * m[3][1] * m[0][2] * m[2][3]
		+ m[1][0] * m[2][1] * m[0][2] * m[3][3]
		- m[1][0] * m[2][1] * m[3][2] * m[0][3]
		- m[3][0] * m[1][1] * m[0][3] * m[2][2]
		+ m[3][0] * m[0][1] * m[1][3] * m[2][2]
		- m[3][0] * m[0][1] * m[1][2] * m[2][3]
		- m[2][0] * m[3][1] * m[1][2] * m[0][3]
		+ m[2][0] * m[3][1] * m[0][2] * m[1][3]
		- m[3][0] * m[2][1] * m[0][2] * m[1][3]
		+ m[3][0] * m[1][1] * m[0][2] * m[2][3]
		+ m[3][0] * m[2][1] * m[1][2] * m[0][3]
		- m[0][0] * m[2][1] * m[1][2] * m[3][3]
		+ m[0][0] * m[2][1] * m[3][2] * m[1][3]
		- m[1][0] * m[0][1] * m[2][2] * m[3][3]
		+ m[1][0] * m[0][1] * m[2][3] * m[3][2]
		+ m[0][0] * m[3][1] * m[1][2] * m[2][3]
		- m[0][0] * m[3][1] * m[1][3] * m[2][2]
		+ m[0][0] * m[1][1] * m[2][2] * m[3][3]
		- m[0][0] * m[1][1] * m[2][3] * m[3][2];
}


#define POLYNOMORDER    6


void TestEigen()
{
	double A[4][4];
	double p[5];
/*	A[0][0] = -0.0069; 	A[1][0] =  0.0139; 	A[2][0] =   0.0;	A[3][0] = -9.81;
	A[0][1] = -0.0905; 	A[1][1] = -0.3149; 	A[2][1] = 235.8928; A[3][1] =  0.0;
	A[0][2] =  0.0004;	A[1][2] = -0.0034;	A[2][2] = -0.4282; 	A[3][2] = 0.0;
	A[0][3] =  0.0000;	A[1][3] =  0.0000;	A[2][3] =  1.0;		A[3][3] = 0.0;*/

	A[0][0] =-1.00; A[0][1] =  1.0;	A[0][2] =  1.0;	A[0][3] = -1.0;
	A[1][0] = 1.00; A[1][1] =  1.0;	A[1][2] =  2.0; A[1][3] = -1.0;
	A[2][0] = 3.00;	A[2][1] = -2.0;	A[2][2] =  1.0; A[2][3] =  1.0;
	A[3][0] = 1.00;	A[3][1] =  1.0;	A[3][2] =  2.0;	A[3][3] =  1.0;
	complex<double> AC[16];
//	complex<double> V[4];
	complex <double> lambda(2.0, 0.0);
	for(int i=0; i<4; i++)
	{
		for(int j=0; j<4;j++)
		{
			AC[i*4+j] = complex<double>(A[i][j],0.0);
		}
	}

	CharacteristicPol(A, p);

	complex<double> roots[POLYNOMORDER];

	if(LinBairstow(p, roots, 4))
	{
	}
	else
	{
	}
}


bool Eigenvector(double a[][4], complex<double> lambda, complex<double> *V)
{
	//________________________________________________________________________
	//Solves the system A.V = lambda.V for a 4x4 complex matrix
	// in input :
	//    - matrix A
	//    - the array of complex eigenvalues
	// in output
	//    - the array of complex eigenvectors
	//
	// eigen vector is solved by direct matrix solution
	// one of the vector's component is set to 1, to avoid the trivial solution V=0;
	//
	// Andre Deperrois October 2009
	//
	//________________________________________________________________________

	static complex<double> detm, detr;
	static complex<double> r[9], m[9];
	int ii, jj, i, j, kp;

	// first find a pivot for which the  associated n-1 determinant is not zero
	bool bFound = false;
	kp=0;
	do
	{
		V[kp] = 1.0;
		ii= 0;
		for(i=0;i<4 ; i++)
		{
			if(i!=kp)
			{
				jj=0;
				for(j=0; j<4; j++)
				{
					if(j!=kp)
					{
						m[ii*3+jj] = a[i][j];
						jj++;
					}
				}
				m[ii*3+ii] -= lambda;
				ii++;
			}
		}
		detm = Det33(m);
		bFound = std::abs(detm)>0.0;
		if(bFound || kp>=3) break;
		kp++;
	}while(true);

	if(!bFound) return false;

	// at this point we have identified pivot kp
	// with a non-zero subdeterminant.
	// so solve for the other 3 eigenvector components.
	// using Cramer's rule

	//create rhs determinant
	jj=0;
	for(j=0; j<4; j++)
	{
		memcpy(r,m, 9*sizeof(complex<double>));
		if(j!=kp)
		{
			ii= 0;
			for(i=0; i<4; i++)
			{
				if(i!=kp)
				{
					r[ii*3+jj] = - a[i][kp];
					ii++;
				}
			}
			detr  = Det33(r);
			V[j] = detr/detm;
			jj++;
		}
	}

	return true;
}

#define TOLERANCE   0.0000000001

bool LinBairstow(double *p, complex<double> *root, int n)
{
	//________________________________________________________________________
	//
	// Finds the complex roots of a polynom P(x) using Lin-Bairstow's method
	// P(x) = Sum p_i x^i	    i = 0..n;
	// The polynoms coefficient are in array p
	//
	// Andre Deperrois October 2009
	//
	//________________________________________________________________________

	double b[POLYNOMORDER], c[POLYNOMORDER];
	int i, k, nn, iter;
	double r,s,d0,d1,d2;
	double Delta;

	memset(b, 0, POLYNOMORDER*sizeof(double));
	memset(c, 0, POLYNOMORDER*sizeof(double));

	//T(x) = x2 -rx -s;
	//R(x) = u(x-r)+v //remainder of deivision of by Q
	//Q(x) = Sum b_i x^(i-2)

	//P(x) = (x2-rx+s) Q(x) + u(x-r) +v

	nn=n ;//initial order is polynom order

	r=-2.0;//initial guesses
	s=-1.0;

	do
	{
		iter = 0;
		do
		{
			//compute recursively the coefs b_i of polynom Q
			b[nn]   = p[nn];
			b[nn-1] = p[nn-1] + r * b[nn];
			for(k=nn-2; k>=0; k--) b[k] = p[k] + r*b[k+1] + s*b[k+2];

			//build the partial derivatives c_i
			c[nn]   = b[nn];
			c[nn-1] = b[nn-1] + r * c[nn];
			for(k=nn-2; k>=1; k--) c[k] = b[k] + r*c[k+1] + s*c[k+2];

			d0 = c[1]*c[3] - c[2]*c[2];
			d1 = (-b[0]*c[3]+b[1]*c[2])/d0;
			d2 = (-b[1]*c[1]+b[0]*c[2])/d0;
			r+=d1;
			s+=d2;
		} while((std::abs(d1)> TOLERANCE || std::abs(d2)> TOLERANCE) && iter <10);

		//we have a division
		//so find the roots of the remainder R
		Delta = r*r+4.0*s;
		if(Delta<0.0)
		{
			//complex roots
			root[nn-1] = complex<double>(r/2.0,  sqrt(fabs(Delta))/2.0);
			root[nn-2] = complex<double>(r/2.0, -sqrt(fabs(Delta))/2.0);
		}
		else
		{
			//real roots
			root[nn-1]   = complex<double>(r/2.0 + sqrt(Delta)/2.0, 0.0);
			root[nn-2] = complex<double>(r/2.0 - sqrt(Delta)/2.0, 0.0);
		}

		//deflate polynom order
		for(i=nn; i>=2; i--)
		{
			p[i-2] = b[i];
		}
		nn-=2;
		if(nn==2)
		{
			//last two roots, solve directly
			if(fabs(p[2])<PRECISION)
			{
				// one last root, but we should never get here
				if(fabs(p[1])>PRECISION)
				{
					//last single root, real
					root[0] = -p[0]/p[1];
				}
				else return false;
			}
			else
			{
				Delta = p[1]*p[1]-4.0*p[0]*p[2];
				if(Delta<0)
				{
					//complex roots
					root[nn-1] = complex<double>(-p[1]/2.0/p[2],  sqrt(fabs(Delta))/2.0/p[2]);
					root[nn-2] = complex<double>(-p[1]/2.0/p[2], -sqrt(fabs(Delta))/2.0/p[2]);
				}
				else
				{
					//real roots
					root[nn-1] = complex<double>((-p[1]+sqrt(Delta))/2.0/p[2],  0.0);
					root[nn-2] = complex<double>((-p[1]-sqrt(Delta))/2.0/p[2],  0.0);
				}
			}
			break;
		}
		if(nn==1)
		{
			if(fabs(p[1])>PRECISION)
			{
				//last single root, real
				root[0] = -p[0]/p[1];
			}
			else return false;
			break;
		}

	}while(nn>2);
	return true;
}






