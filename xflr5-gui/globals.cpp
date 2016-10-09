/****************************************************************************

	Globals Class
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

//Global functions

/**@file This file contains the definitions of methods used throughout the program and not specific to one application. */

#include <globals.h>
#include <mainframe.h>
#include <QtDebug>
#include <QPen>
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QDateTime> 
#include <QByteArray>
#include <math.h>
#include <qopengl.h>
#include <objects/WPolar.h>
#include <misc/Units.h>


/** 
* Returns a double number as its root and its base 10 exponent
* @param f the double number to reformat; is returned as f = f/pow(10.0,exp);
* @param exp the base 10 exponent of f.
*/
void ExpFormat(double &f, int &exp)
{
	if (f==0.0)
	{
		exp = 0;
		f = 0.0;
		return;
	}
	double f1 = qAbs(f);
//	int sgn = int(f/f1);
	if(f1<1)
		exp = (int)log10(f1)-1;
	else
		exp = (int)log10(f1);

	f = f/pow(10.0,exp);

}



/**
* Returns the index of a Qt-style based on the index of the style in the array 
*@param s the index of the style
*@return The index of the Qt-style 
*/
Qt::PenStyle getStyle(int s)
{
	if(s==0)      return Qt::SolidLine;
	else if(s==1) return Qt::DashLine;
	else if(s==2) return Qt::DotLine;
	else if(s==3) return Qt::DashDotLine;
	else if(s==4) return Qt::DashDotDotLine;
	return Qt::SolidLine;
}




/**
* Outputs in a debug file the current time and the value of the integer passed as an input parameter.
* The file is in the user's default temporary directory with the name Trace.log
* Used for debugging.
*@param n the integer to output
*/
void Trace(int n)
{
	if(!MainFrame::s_bTrace) return;

	if(MainFrame::s_pTraceFile && MainFrame::s_pTraceFile->isOpen())
	{
		QTextStream ts(MainFrame::s_pTraceFile);
		ts << "Int value=" << n << "\n";
	}
	qDebug()<<"Int value="<<n;
}


void Trace(QString msg, bool b)
{
	if(!MainFrame::s_bTrace) return;
	QString str;
	if(b) str += msg + "=true";
	else  str += msg + "=false";

	if(MainFrame::s_pTraceFile && MainFrame::s_pTraceFile->isOpen())
	{
		QTextStream ts(MainFrame::s_pTraceFile);
		ts << str << "\n";
	}
	qDebug()<<str;
}

/**
* Outputs in a debug file the current time and a string message passed as an input parameter.
* The file is in the user's default temporary directory with the name Trace.log. 
* Used for debugging.
*@param msg the message to output
*/
void Trace(QString msg)
{
	if(!MainFrame::s_bTrace) return;

	if(MainFrame::s_pTraceFile && MainFrame::s_pTraceFile->isOpen())
	{
		QTextStream ts(MainFrame::s_pTraceFile);
		ts<<msg<<"\n";
		ts.flush();
	}
	qDebug()<<msg;
}


/**
* Outputs in a debug file the current time, a string message and the value of the integer passed as an input parameter.
* The file is in the user's default temporary directory with the name Trace.log. 
* Used for debugging.
*@param msg the message to output
*@param n the integer to output
*/
void Trace(QString msg, int n)
{
	if(!MainFrame::s_bTrace) return;

	QString strong;
	strong = QString("  %1").arg(n);
	strong = msg + strong;



	if(MainFrame::s_pTraceFile && MainFrame::s_pTraceFile->isOpen())
	{
		QTextStream ts(MainFrame::s_pTraceFile);
		ts << strong << "\n";
	}
	qDebug()<<strong;
}


/**
* Outputs in a debug file the current time, a string message and the value of the floating number passed as an input parameter.
* The file is in the user's default temporary directory with the name Trace.log. 
* Used for debugging.
*@param msg the message to output
*@param f the float number to output
*/
void Trace(QString msg, double f)
{
	if(!MainFrame::s_bTrace) return;

	QString strong;
	strong = QString("  %1").arg(f);
	strong = msg + strong;


	if(MainFrame::s_pTraceFile && MainFrame::s_pTraceFile->isOpen())
	{
		QTextStream ts(MainFrame::s_pTraceFile);
		ts << strong << "\n";
	}
	qDebug()<<strong;
}


/**
* Returns the red component of a color scale depending on an input parameter with value between 0 and 1.
* Used to draw a color scale between 0=blue and 1=red
*@param tau the input parameter between 0 and 1.
*@return the red component of the color
*/
float GLGetRed(double tau)
{
	if(tau>2.0/3.0)      return 1.0;
	else if(tau>1.0/3.0) return (3.0*(tau-1.0/3.0));
	else                 return 0.0;
}


/**
* Returns the green component of a color scale depending on an input parameter with value between 0 and 1.
* Used to draw a color scale between 0=blue and 1=red
*@param tau the input parameter between 0 and 1.
*@return the green component of the color
*/
float GLGetGreen(double tau)
{
	if(tau<0.f || tau>1.0) 	return 0.0;
	else if(tau<1.0/4.0) 	return (4.0*tau);
	else if(tau>3.0/4.0) 	return (1.0-4.0*(tau-3.0/4.0));
	else                    return 1.0;
}


/**
* Returns the blue component of a color scale depending on an input parameter with value between 0 and 1.
* Used to draw a color scale between 0=blue and 1=red
*@param tau the input parameter between 0 and 1.
*@return the blue component of the color
*/
float GLGetBlue(double tau)
{
	if(tau>2.0/3.0)      return 0.0;
	else if(tau>1.0/3.0) return (1.0-3.0*(tau-1.0/3.0));
	else                 return 1.0;
}


/**
* Extracts three double values from a QString, and returns the number of extracted values.
*/
int readValues(QString line, double &x, double &y, double &z)
{
/*	char *sx = new char[30];
	char *sy = new char[30];
	char *text;*/
	int res=0;

	QString str;
	bool bOK;

	line = line.simplified();
	int pos = line.indexOf(" ");


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
		return res;
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
		return res;
	}

	line = line.trimmed();
	if(!line.isEmpty())
	{
		z = line.toDouble(&bOK);
		if(bOK) res++;
	}
	else z=0.0;

	return res;
}



/** 
*Returns the determinant of a 4x4 matrix
*@param aij a pointer to a one-dimensional array holding the 16 double values of the matrix
*@return the matrix's determinant
*/
double Det44(double *aij)
{
//	returns the determinant of a 4x4 matrix

	int i,j,k,l,p,q;
	double det, sign, a33[16];

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



/** 
*Returns the cofactor of an element in a 4x4 matrix of complex values.
*@param aij a pointer to a one-dimensional array holding the 16 complex values of the matrix.
*@param i the number of the element's line, starting at 0.
*@param j the number of the element's column, starting at 0.
*@return the cofactor of element (i,j).
*/
complex<double> Cofactor44(complex<double> *aij, int &i, int &j)
{
	//returns the complex cofactor	of element i,j, in the 4x4 matrix aij
	int k,l,p,q;
	complex<double> a33[9];

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

/** 
*Returns the determinant of a complex 4x4 matrix
*@param aij a pointer to a one-dimensional array holding the 16 complex double values of the matrix
*@return the matrix's determinant
*/
complex<double> Det44(complex<double> *aij)
{
//	returns the determinant of a 4x4 matrix

	int i,j,k,l,p,q;
	double sign;
	complex<double> det, a33[16];
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

/** 
*Inverts a complex 4x4 matrix
*@param ain in input, a pointer to a one-dimensional array holding the 16 complex values of the input matrix
*@param aout in output, a pointer to a one-dimensional array holding the 16 complex values of the inverted matrix
*@return if the inversion was successful
*/
bool Invert44(complex<double> *ain, complex<double> *aout)
{
	//small size, use the direct method
	int i,j;
	complex<double> det;
	double sign;

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




QColor randomColor()
{
	QColor clr;
	clr.setHsv((int)(((double)qrand()/(double)RAND_MAX)*360),
			   (int)(((double)qrand()/(double)RAND_MAX)*101)+ 99,
			   (int)(((double)qrand()/(double)RAND_MAX)*100)+155,
				255);
	return clr;
}




void GLLineStipple(int style)
{
	if     (style == Qt::DashLine)       glLineStipple (1, 0xCFCF);
	else if(style == Qt::DotLine)        glLineStipple (1, 0x6666);
	else if(style == Qt::DashDotLine)    glLineStipple (1, 0xFF18);
	else if(style == Qt::DashDotDotLine) glLineStipple (1, 0x7E66);
	else                                 glLineStipple (1, 0xFFFF);
}

//-----------------------------------------------------------------------------------------------



XFLR5::enumPanelDistribution distributionType(QString strDist)
{
	if(strDist.compare("COSINE",           Qt::CaseInsensitive)==0) return XFLR5::COSINE;
	else if(strDist.compare("UNIFORM",     Qt::CaseInsensitive)==0) return XFLR5::UNIFORM;
	else if(strDist.compare("SINE",        Qt::CaseInsensitive)==0) return XFLR5::SINE;
	else if(strDist.compare("INVERSESINE", Qt::CaseInsensitive)==0) return XFLR5::INVERSESINE;
	else return XFLR5::UNIFORM;
}


QString distributionType(XFLR5::enumPanelDistribution dist)
{
	switch(dist)
	{
		case XFLR5::COSINE: return "COSINE"; break;
		case XFLR5::UNIFORM: return "UNIFORM"; break;
		case XFLR5::SINE: return "SINE"; break;
		case XFLR5::INVERSESINE: return "INVERSE SINE"; break;
		default: return "";
	}
}



XFLR5::enumBodyLineType bodyPanelType(QString strPanelType)
{
	if(strPanelType.compare("FLATPANELS", Qt::CaseInsensitive)==0) return XFLR5::BODYPANELTYPE;
	else                                                           return XFLR5::BODYSPLINETYPE;
}

QString bodyPanelType(XFLR5::enumBodyLineType panelType)
{
	switch(panelType)
	{
		case XFLR5::BODYPANELTYPE:  return "FLATPANELS"; break;
		case XFLR5::BODYSPLINETYPE: return "NURBS"; break;
		default: return "";
	}
}


XFOIL::enumPolarType polarType(QString strPolarType)
{
	if     (strPolarType.compare("FIXEDSPEEDPOLAR",   Qt::CaseInsensitive)==0) return XFOIL::FIXEDSPEEDPOLAR;
	else if(strPolarType.compare("FIXEDLIFTPOLAR",    Qt::CaseInsensitive)==0) return XFOIL::FIXEDLIFTPOLAR;
	else if(strPolarType.compare("RUBBERCHORDPOLAR",  Qt::CaseInsensitive)==0) return XFOIL::RUBBERCHORDPOLAR;
	else if(strPolarType.compare("FIXEDAOAPOLAR",     Qt::CaseInsensitive)==0) return XFOIL::FIXEDAOAPOLAR;
	else return XFOIL::FIXEDSPEEDPOLAR;
}

QString polarType(XFOIL::enumPolarType polarType)
{
	switch(polarType)
	{
		case XFOIL::FIXEDSPEEDPOLAR:  return "FIXEDSPEEDPOLAR";   break;
		case XFOIL::FIXEDLIFTPOLAR:   return "FIXEDLIFTPOLAR";    break;
		case XFOIL::RUBBERCHORDPOLAR: return "RUBBERCHORDPOLAR";  break;
		case XFOIL::FIXEDAOAPOLAR:    return "FIXEDAOAPOLAR";     break;
		default: return "";
	}
}



XFLR5::enumPolarType WPolarType(QString strPolarType)
{
	if     (strPolarType.compare("FIXEDSPEEDPOLAR", Qt::CaseInsensitive)==0) return XFLR5::FIXEDSPEEDPOLAR;
	else if(strPolarType.compare("FIXEDLIFTPOLAR",  Qt::CaseInsensitive)==0) return XFLR5::FIXEDLIFTPOLAR;
	else if(strPolarType.compare("FIXEDAOAPOLAR",   Qt::CaseInsensitive)==0) return XFLR5::FIXEDAOAPOLAR;
	else if(strPolarType.compare("STABILITYPOLAR",  Qt::CaseInsensitive)==0) return XFLR5::STABILITYPOLAR;
	else if(strPolarType.compare("BETAPOLAR",       Qt::CaseInsensitive)==0) return XFLR5::BETAPOLAR;
	else return XFLR5::FIXEDSPEEDPOLAR;
}

QString WPolarType(XFLR5::enumPolarType polarType)
{
	switch(polarType)
	{
		case XFLR5::FIXEDSPEEDPOLAR:  return "FIXEDSPEEDPOLAR"; break;
		case XFLR5::FIXEDLIFTPOLAR:   return "FIXEDLIFTPOLAR";  break;
		case XFLR5::FIXEDAOAPOLAR:    return "FIXEDAOAPOLAR";   break;
		case XFLR5::STABILITYPOLAR:   return "STABILITYPOLAR";  break;
		case XFLR5::BETAPOLAR:        return "BETAPOLAR";       break;
		default: return "";
	}
}


XFLR5::enumAnalysisMethod analysisMethod(QString strAnalysisMethod)
{
	if     (strAnalysisMethod.compare("LLTMETHOD",   Qt::CaseInsensitive)==0) return XFLR5::LLTMETHOD;
	else if(strAnalysisMethod.compare("VLMMETHOD",   Qt::CaseInsensitive)==0) return XFLR5::VLMMETHOD;
	else if(strAnalysisMethod.compare("PANELMETHOD", Qt::CaseInsensitive)==0) return XFLR5::PANELMETHOD;
	else return XFLR5::VLMMETHOD;
}


QString analysisMethod(XFLR5::enumAnalysisMethod analysisMethod)
{
	switch(analysisMethod)
	{
		case XFLR5::LLTMETHOD:   return "LLTMETHOD";   break;
		case XFLR5::VLMMETHOD:   return "VLMMETHOD";   break;
		case XFLR5::PANELMETHOD: return "PANELMETHOD"; break;
		default: return "";
	}
}



bool stringToBool(QString str)
{
	return str.compare("true", Qt::CaseInsensitive)==0 ? true : false;
}

QString referenceDimension(XFLR5::enumRefDimension refDimension)
{
	switch(refDimension)
	{
		case XFLR5::PLANFORMREFDIM:  return "PLANFORMREFDIM";
		case XFLR5::PROJECTEDREFDIM: return "PROJECTEDREFDIM";
		case XFLR5::MANUALREFDIM:    return "MANUALREFDIM";
		default: return "";
	}
}


XFLR5::enumRefDimension referenceDimension(QString strRefDimension)
{
	if     (strRefDimension.compare("PLANFORMREFDIM",  Qt::CaseInsensitive)==0) return XFLR5::PLANFORMREFDIM;
	else if(strRefDimension.compare("PROJECTEDREFDIM", Qt::CaseInsensitive)==0) return XFLR5::PROJECTEDREFDIM;
	else if(strRefDimension.compare("MANUALREFDIM",    Qt::CaseInsensitive)==0) return XFLR5::MANUALREFDIM;
	else return XFLR5::PLANFORMREFDIM;
}



void * readFoilFile(QFile &xFoilFile)
{
	QString strong;
	QString tempStr;
	QString FoilName;

	Foil* pFoil = NULL;
	int pos, i, ip;
	pos = 0;
	double x, y, z, area;
	double xp, yp;
	bool bRead;
	QTextStream in(&xFoilFile);

	QString fileName = xFoilFile.fileName();
	int idx = fileName.lastIndexOf(QDir::separator());
	if(idx>0)
	{
		fileName = fileName.right(fileName.length()-idx-1);
		if(fileName.endsWith(".dat", Qt::CaseInsensitive))
		{
			fileName = fileName.left(fileName.length()-4);
		}
	}

	pFoil = new Foil();
	if(!pFoil)	return NULL;

	while(tempStr.length()==0 && !in.atEnd())
	{
		strong = in.readLine();
		pos = strong.indexOf("#",0);
		// ignore everything after # (including #)
		if(pos>0)strong.truncate(pos);
		tempStr = strong;
		tempStr.remove(" ");
		FoilName = strong;
	}

	if(!in.atEnd())
	{
		// FoilName contains the last comment

		if(readValues(strong,x,y,z)==2)
		{
			//there isn't a name on the first line, use the file's name
			FoilName = fileName;
			{
				pFoil->xb[0] = x;
				pFoil->yb[0] = y;
				pFoil->nb=1;
				xp = x;
				yp = y;
			}
		}
		else FoilName = strong;
		// remove fore and aft spaces
		FoilName = FoilName.trimmed();
	}

	bRead = true;
	xp=-9999.0;
	yp=-9999.0;
	do
	{
		strong = in.readLine();
		pos = strong.indexOf("#",0);
		// ignore everything after # (including #)
		if(pos>0)strong.truncate(pos);
		tempStr = strong;
		tempStr.remove(" ");
		if (!strong.isNull() && bRead && tempStr.length())
		{
			if(readValues(strong, x,y,z)==2)
			{
				//add values only if the point is not coincident with the previous one
				double dist = sqrt((x-xp)*(x-xp) + (y-yp)*(y-yp));
				if(dist>0.000001)
				{
					pFoil->xb[pFoil->nb] = x;
					pFoil->yb[pFoil->nb] = y;
					pFoil->nb++;
					if(pFoil->nb>IQX)
					{
						delete pFoil;
						return NULL;
					}
					xp = x;
					yp = y;
				}
			}
			else bRead = false;
		}
	}while (bRead && !strong.isNull());

	pFoil->setFoilName(FoilName);

	// Check if the foil was written clockwise or counter-clockwise

	area = 0.0;
	for (i=0; i<pFoil->nb; i++)
	{
		if(i==pFoil->nb-1)	ip = 0;
		else				ip = i+1;
		area +=  0.5*(pFoil->yb[i]+pFoil->yb[ip])*(pFoil->xb[i]-pFoil->xb[ip]);
	}

	if(area < 0.0)
	{
		//reverse the points order
		double xtmp, ytmp;
		for (int i=0; i<pFoil->nb/2; i++)
		{
			xtmp         = pFoil->xb[i];
			ytmp         = pFoil->yb[i];
			pFoil->xb[i] = pFoil->xb[pFoil->nb-i-1];
			pFoil->yb[i] = pFoil->yb[pFoil->nb-i-1];
			pFoil->xb[pFoil->nb-i-1] = xtmp;
			pFoil->yb[pFoil->nb-i-1] = ytmp;
		}
	}

	memcpy(pFoil->x, pFoil->xb, sizeof(pFoil->xb));
	memcpy(pFoil->y, pFoil->yb, sizeof(pFoil->yb));
	pFoil->n = pFoil->nb;

	QColor clr = randomColor();
	pFoil->setColor(clr.red(), clr.green(), clr.blue(), clr.alpha());
	pFoil->initFoil();

	return pFoil;
}





/**
 *Reads a Foil and its related Polar objects from a binary stream associated to a .plr file.
 * @param ar the binary stream
 * @return the pointer to the Foil object which has been created, or NULL if failure.
 */
void * readPolarFile(QFile &plrFile, QList<Polar*> &polarList)
{
	Foil* pFoil = NULL;
	Polar *pPolar = NULL;
	Polar * pOldPolar;
	int i, n, l;

	QDataStream ar(&plrFile);
	ar.setVersion(QDataStream::Qt_4_5);
	ar.setByteOrder(QDataStream::LittleEndian);

	ar >> n;

	if(n<100000)
	{
		//old format
		return NULL;
	}
	else if (n >=100000)
	{
		//new format XFLR5 v1.99+
		//first read all available foils
		ar>>n;
		for (i=0;i<n; i++)
		{
			pFoil = new Foil();
			if (!pFoil->serialize(ar, false))
			{
				delete pFoil;
				return NULL;
			}
		}

		//next read all available polars

		ar>>n;
		for (i=0;i<n; i++)
		{
			pPolar = new Polar();

			if (!pPolar->serialize(ar, false))
			{
				delete pPolar;
				return NULL;
			}
			for (l=0; l<polarList.size(); l++)
			{
				pOldPolar = polarList.at(l);
				if (pOldPolar->foilName()  == pPolar->foilName() &&
					pOldPolar->polarName() == pPolar->polarName())
				{
					//just overwrite...
					polarList.removeAt(l);
					delete pOldPolar;
					//... and continue to add
				}
			}
			polarList.append(pPolar);
		}
	}
	return pFoil;
}







/**
 * Draws the foil in the client area.
 * @param painter a reference to the QPainter object on which the foil will be drawn
 * @param alpha the rotation angle in degrees of the foil
 * @param scalex the scaling factor in the x-direction
 * @param scaley the scaling factor in the y-direction
 * @param Offset the foil offset in the client area
 */
void drawFoil(QPainter &painter, Foil*pFoil, double const &alpha, double const &scalex, double const &scaley, QPointF const &Offset)
{
	double xa, ya, sina, cosa;
	QPointF From, To;
	QRectF R;
	int k;
	QPen FoilPen, HighPen;

	FoilPen.setColor(colour(pFoil));
	FoilPen.setWidth(pFoil->foilWidth());
	FoilPen.setStyle(getStyle(pFoil->foilStyle()));
	painter.setPen(FoilPen);

	HighPen.setColor(QColor(255,0,0));

	cosa = cos(alpha*PI/180.0);
	sina = sin(alpha*PI/180.0);

	xa = (pFoil->x[0]-0.5)*cosa - pFoil->y[0]*sina + 0.5;
	ya = (pFoil->x[0]-0.5)*sina + pFoil->y[0]*cosa;
	From.rx() = ( xa*scalex + Offset.x());
	From.ry() = (-ya*scaley + Offset.y());

	if(pFoil->showPoints())
	{
		R.setLeft(( xa*scalex) + Offset.x() -2);
		R.setTop( (-ya*scaley) + Offset.y() -2);
		R.setWidth(4);
		R.setHeight(4);
		painter.drawRect(R);
	}
	if(pFoil->showPoints() && pFoil->iHighLight()==0)
	{
		HighPen.setWidth(2);
		painter.setPen(HighPen);
		painter.drawRect(R);
		painter.setPen(FoilPen);
	}

	for (k=1; k<pFoil->n; k++)
	{
		xa = (pFoil->x[k]-0.5)*cosa - pFoil->y[k]*sina+ 0.5;
		ya = (pFoil->x[k]-0.5)*sina + pFoil->y[k]*cosa;
		To.rx() =  xa*scalex+Offset.x();
		To.ry() = -ya*scaley+Offset.y();

		painter.drawLine(From,To);

		if(pFoil->showPoints())
		{
			R.setLeft(  xa*scalex + Offset.x() -2);
			R.setTop(  -ya*scaley + Offset.y() -2);
			R.setWidth(3);
			R.setHeight(3);
			painter.drawRect(R);
		}
		if(pFoil->showPoints() && pFoil->iHighLight()==k)
		{
			HighPen.setWidth(2);
			painter.setPen(HighPen);
			painter.drawRect(R);
			painter.setPen(FoilPen);
		}

		From = To;
	}
}


/**
 * Draws the foil's mid line in the client area.
 * @param painter a refernce to the QPainter object on which the foil will be drawn
 * @param alpha the rotation angle in degrees of the foil
 * @param scalex the scaling factor in the x-direction
 * @param scaley the scaling factor in the y-direction
 * @param Offset the foil offset in the client area
 */
void drawMidLine(QPainter &painter, Foil*pFoil, double const &scalex, double const &scaley, QPointF const &Offset)
{
	QPointF From, To;
	int k;
	QPen FoilPen;

	FoilPen.setColor(colour(pFoil));
	FoilPen.setWidth(pFoil->foilWidth());
	FoilPen.setStyle(Qt::DashLine);
	painter.setPen(FoilPen);


	From.rx() = ( pFoil->m_rpMid[0].x*scalex)  +Offset.x();
	From.ry() = (-pFoil->m_rpMid[0].y*scaley)  +Offset.y();


	for (k=0; k<MIDPOINTCOUNT; k++)
	{
		To.rx() = ( pFoil->m_rpMid[k].x*scalex)+Offset.x();
		To.ry() = (-pFoil->m_rpMid[k].y*scaley)+Offset.y();

		painter.drawLine(From, To);
		From = To;
	}
}





/**
 * Draws the foil's points in the client area.
 * @param painter a refernce to the QPainter object on which the foil will be drawn
 * @param alpha the rotation angle in degrees of the foil
 * @param scalex the scaling factor in the x-direction
 * @param scaley the scaling factor in the y-direction
 * @param Offset the foil offset in the client area
 */
void drawPoints(QPainter &painter, Foil*pFoil, double const &scalex, double const &scaley, QPointF const &Offset)
{
	int width;
	QPoint pt1;

	width = 2;

	QPen FoilPen, HighPen;
	FoilPen.setColor(colour(pFoil));
	FoilPen.setWidth(pFoil->foilWidth());
	FoilPen.setStyle(Qt::SolidLine);
	painter.setPen(FoilPen);


	HighPen.setColor(QColor(255,0,0));

	for (int i=0; i<pFoil->n;i++)
	{
		pt1.rx() = ( pFoil->x[i]*scalex + Offset.x() - width);
		pt1.ry() = (-pFoil->y[i]*scaley + Offset.y() - width);

		painter.drawRect(pt1.x(), pt1.y(), 4, 4) ;
	}
	if(pFoil->iHighLight()>=0)
	{
		HighPen.setWidth(2);
		painter.setPen(HighPen);

		pt1.rx() = ( pFoil->x[pFoil->iHighLight()]*scalex + Offset.x() - width);
		pt1.ry() = (-pFoil->y[pFoil->iHighLight()]*scaley + Offset.y() - width);

		painter.drawRect(pt1.x(), pt1.y(), 4, 4);
	}
}




void setAutoWPolarName(void * ptrWPolar, void *ptrPlane)
{
	if(!ptrPlane) return;
	QString str, strong;
	QString strSpeedUnit;
	Units::getSpeedUnitLabel(strSpeedUnit);

	int i, nCtrl;

	WPolar *pWPolar =(WPolar*)ptrWPolar;
	Plane *pPlane = (Plane*)ptrPlane;

	Units::getSpeedUnitLabel(str);

	switch(pWPolar->polarType())
	{
		case XFLR5::FIXEDSPEEDPOLAR:
		{
			pWPolar->polarName() = QString("T1-%1 ").arg(pWPolar->velocity() * Units::mstoUnit(),0,'f',1);
			pWPolar->polarName() += strSpeedUnit;
			break;
		}
		case XFLR5::FIXEDLIFTPOLAR:
		{
			pWPolar->polarName() = QString("T2");
			break;
		}
		case XFLR5::FIXEDAOAPOLAR:
		{
			pWPolar->polarName() = QString(QString::fromUtf8("T4-%1°")).arg(pWPolar->Alpha(),0,'f',3);
			break;
		}
		case XFLR5::BETAPOLAR:
		{
			pWPolar->polarName() = QString(QString::fromUtf8("T5-a%1°-%2"))
							  .arg(pWPolar->Alpha(),0,'f',1)
							  .arg(pWPolar->velocity() * Units::mstoUnit(),0,'f',1);
			pWPolar->polarName() += strSpeedUnit;
			break;
		}
		case XFLR5::STABILITYPOLAR:
		{
			pWPolar->polarName() = QString("T7");
			break;
		}
		default:
		{
			pWPolar->polarName() = "Tx";
			break;
		}
	}

	switch(pWPolar->analysisMethod())
	{
		case XFLR5::LLTMETHOD:
		{
			pWPolar->polarName() += "-LLT";
			break;
		}
		case XFLR5::VLMMETHOD:
		{
			if(pWPolar->bVLM1()) pWPolar->polarName() += "-VLM1";
			else                 pWPolar->polarName() += "-VLM2";
			break;
		}
		case XFLR5::PANELMETHOD:
		{
			if(!pWPolar->bThinSurfaces()) pWPolar->polarName() += "-Panel";
			else
			{
				if(pWPolar->bVLM1()) pWPolar->polarName() += "-VLM1";
				else                 pWPolar->polarName() += "-VLM2";
			}
			break;
		}
	}

	nCtrl = 0;



	if(pWPolar->isStabilityPolar())
	{
		QString pm=  QString::fromUtf8("\u2213");

		if(!pPlane->isWing())
		{
			if(pWPolar->m_ControlGain.size()>0 && qAbs(pWPolar->m_ControlGain[0])>PRECISION)
			{
				strong = QString::fromUtf8("-Wing(g%1)")
								   .arg(pWPolar->m_ControlGain[0],0,'f',1);
				pWPolar->polarName() += strong;
			}
			nCtrl++;
		}

		if(pPlane->stab())
		{
			if(pWPolar->m_ControlGain.size()>1 && qAbs(pWPolar->m_ControlGain[1])>PRECISION)
			{
				strong = QString::fromUtf8("-Elev(g%1)").arg(pWPolar->m_ControlGain[1],0,'f',1);
				pWPolar->polarName() += strong;
			}
			nCtrl++;
		}

		for(i=0; i<pPlane->wing()->nFlaps(); i++)
		{
			if(pWPolar->m_ControlGain.size()>i+nCtrl && qAbs(pWPolar->m_ControlGain[i+nCtrl])>PRECISION)
			{
				strong = QString::fromUtf8("-WF%1(g%2)")
						 .arg(i+1)
						 .arg(pWPolar->m_ControlGain[i+nCtrl],0,'f',1);
				pWPolar->polarName() += strong;
			}
		}
		nCtrl += pPlane->wing()->nFlaps();

		if(pPlane->stab())
		{
			for(i=0; i<pPlane->stab()->nFlaps(); i++)
			{
				if(pWPolar->m_ControlGain.size()>i+nCtrl && qAbs(pWPolar->m_ControlGain[i+nCtrl])>PRECISION)
				{
					strong = QString::fromUtf8("-EF%1(g%2)").arg(i+1).arg(pWPolar->m_ControlGain[i+nCtrl]);
					pWPolar->polarName() += strong;
				}
			}
			nCtrl += pPlane->stab()->nFlaps();
		}

		if(pPlane->fin())
		{
			for(i=0; i<pPlane->fin()->nFlaps(); i++)
			{
				if(pWPolar->m_ControlGain.size()>i+nCtrl && qAbs(pWPolar->m_ControlGain[i+nCtrl])>PRECISION)
				{
					strong = QString::fromUtf8("-FF%1(g%2)").arg(i+1).arg(pWPolar->m_ControlGain[i+nCtrl]);
					pWPolar->polarName() += strong;
				}
			}
		}
	}


	if(qAbs(pWPolar->Beta()) > .001)
	{
		strong = QString(QString::fromUtf8("-b%1°")).arg(pWPolar->Beta(),0,'f',1);
		pWPolar->polarName() += strong;
	}

	if(qAbs(pWPolar->Phi()) > .001)
	{
		strong = QString(QString::fromUtf8("-B%1°")).arg(pWPolar->Phi(),0,'f',1);
		pWPolar->polarName() += strong;
	}

	if(!pWPolar->bAutoInertia())
	{
		strong.sprintf("-%.1f", pWPolar->mass()*Units::kgtoUnit());
		if(pWPolar->isStabilityPolar()&&fabs(pWPolar->m_inertiaGain[0])>PRECISION)
			str.sprintf("\u2213%0.2f", pWPolar->m_inertiaGain[0]*Units::kgtoUnit());
		else str.clear();
		pWPolar->polarName() += strong + str + Units::weightUnitLabel();

		strong.sprintf("-x%.1f", pWPolar->CoG().x*Units::mtoUnit());
		if(pWPolar->isStabilityPolar()&&fabs(pWPolar->m_inertiaGain[1])>PRECISION)
			str.sprintf("\u2213%0.2f", pWPolar->m_inertiaGain[1]*Units::mtoUnit());
		else str.clear();
		pWPolar->polarName() += strong + str + Units::lengthUnitLabel();

		if(qAbs(pWPolar->CoG().z)>=.000001)
		{
			strong.sprintf("-z%.1f", pWPolar->CoG().z*Units::mtoUnit());
			if(pWPolar->isStabilityPolar()&&fabs(pWPolar->m_inertiaGain[2])>PRECISION)
				str.sprintf("\u2213%0.2f", pWPolar->m_inertiaGain[2]*Units::mtoUnit());
			else str.clear();
			pWPolar->polarName() += strong + str + Units::lengthUnitLabel();
		}
	}

	if(!pWPolar->bViscous())
	{
		pWPolar->polarName() += "-Inviscid";
	}
	if(pWPolar->bIgnoreBodyPanels())
	{
		pWPolar->polarName() += "-NoBodyPanels";
	}
	if(pWPolar->referenceDim()==XFLR5::PROJECTEDREFDIM) pWPolar->polarName() += "-proj_area";

	for(int i=0; i<MAXEXTRADRAG; i++)
	{
		if(fabs(pWPolar->m_ExtraDragCoef[i])>PRECISION && fabs(pWPolar->m_ExtraDragArea[i])>PRECISION)
		{
			pWPolar->polarName()+="+Drag";
			break;
		}
	}
}




XFLR5::enumWingType wingType(QString strWingType)
{
	if     (strWingType.compare("MAINWING",   Qt::CaseInsensitive)==0) return XFLR5::MAINWING;
	else if(strWingType.compare("SECONDWING", Qt::CaseInsensitive)==0) return XFLR5::SECONDWING;
	else if(strWingType.compare("ELEVATOR",   Qt::CaseInsensitive)==0) return XFLR5::ELEVATOR;
	else if(strWingType.compare("FIN",        Qt::CaseInsensitive)==0) return XFLR5::FIN;
	else                                                               return XFLR5::OTHERWING;
}

QString wingType(XFLR5::enumWingType wingType)
{
	switch(wingType)
	{
		case XFLR5::MAINWING:   return "MAINWING";
		case XFLR5::SECONDWING: return "SECONDWING";
		case XFLR5::ELEVATOR:   return "ELEVATOR";
		case XFLR5::FIN:        return "FIN";
		case XFLR5::OTHERWING:  return "OTHERWING";
	}
	return "OTHERWING";
}




/**
* Takes a double number holding the value of a Reynolds number and returns a string.
*@param str the return string  with the formatted number
*@param f the Reynolds number to be formatted
*/
void ReynoldsFormat(QString &str, double f)
{
	int i, q, r, exp;
	f = (int(f/1000.0))*1000.0;

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


QColor getColor(int r, int g, int b, int a)
{
	r = min(r, 255);
	r = max(r, 0);
	g = min(g, 255);
	g = max(g, 0);
	b = min(b, 255);
	b = max(b, 0);
	a = min(a, 255);
	a = max(a, 0);
	return QColor(r,g,b,a);
}

QColor colour(OpPoint *pOpp)
{
	return QColor(pOpp->red(), pOpp->green(), pOpp->blue(), pOpp->alphaChannel());
}

QColor colour(Polar *pPolar)
{
	return QColor(pPolar->red(), pPolar->green(), pPolar->blue(), pPolar->alphaChannel());
}

QColor colour(Foil *pFoil)
{
	return QColor(pFoil->red(), pFoil->green(), pFoil->blue(), pFoil->alphaChannel());
}


void setRandomFoilColor(Foil *pFoil)
{
	QColor clr = randomColor();
	pFoil->setColor(clr.red(), clr.green(), clr.blue(), clr.alpha());
}
