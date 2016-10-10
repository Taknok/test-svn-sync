/****************************************************************************

    Spline  Class
	Copyright (C) 1996 Paul Bourke	http://astronomy.swin.edu.au/~pbourke/curves/spline/
	Copyright (C) 2003-2016 Andre Deperrois adeperrois@xflr5.com

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

#include <engine_globals.h>
#include "Spline.h"
#include "math.h"


/**
*The public constructor
*/
Spline::Spline()
{
	m_Style = 0;
	m_Width = 1;
	m_Color.setHsv((int)(((double)qrand()/(double)RAND_MAX)*360),
					 (int)(((double)qrand()/(double)RAND_MAX)*155)+100,
					 (int)(((double)qrand()/(double)RAND_MAX)*155)+100,
					  255);

	m_CtrlPoint.clear();
#if QT_VERSION >= 0x040700
    m_CtrlPoint.reserve(50);
#endif

	m_knot.clear();
#if QT_VERSION >= 0x040700
    m_knot.reserve(100);
#endif

	m_iHighlight  = -10;
	m_iSelect     = -10;
	m_iDegree     =  3;
	m_iRes        = 79;

	m_PtWeight = 1.0;

	memset(m_Output, 0, sizeof(m_Output));
}

/**
* Copies the data from an existing Spline.
* @param pSpline a pointer to an existing Spline object.
*/
void Spline::copy(Spline *pSpline)
{
	if(!pSpline) return;
	
	m_CtrlPoint.clear();
	for(int ic=0; ic<pSpline->m_CtrlPoint.size(); ic++)
	{
		m_CtrlPoint.append(pSpline->m_CtrlPoint.at(ic));
	}

	m_iDegree     = pSpline->m_iDegree;
	m_iHighlight  = pSpline->m_iHighlight;
	m_iRes        = pSpline->m_iRes;
	m_iSelect     = pSpline->m_iSelect;

	splineKnots();
	splineCurve();
}


/**
* Creates a symetric spline w.r.t. the axis y=0, from an existing Spline.
* @param pSpline a pointer to an existing Spline object.
*/
void Spline::copySymetric(Spline *pSpline)
{
	if(!pSpline) return;
	
	m_CtrlPoint.clear();
	for(int ic=0; ic<pSpline->m_CtrlPoint.size(); ic++)
	{
		m_CtrlPoint.append(pSpline->m_CtrlPoint.at(ic));
		m_CtrlPoint[ic].y = -m_CtrlPoint[ic].y;
	}

	m_iDegree     = pSpline->m_iDegree;
	m_iHighlight  = pSpline->m_iHighlight;
	m_iRes        = pSpline->m_iRes;
	m_iSelect     = pSpline->m_iSelect;
	for(int i=0; i<m_iRes; i++)
	{
		m_Output[i].x =  pSpline->m_Output[i].x;
		m_Output[i].y = -pSpline->m_Output[i].y;
		m_Output[i].z =  pSpline->m_Output[i].z;
	}

	m_knot.clear();
	for(int i=0; i<pSpline->m_knot.size(); i++)
	{
		m_knot.append(pSpline->m_knot[i]);
	}
}



/** Returns the y-coordinate of the spline at the specified x-coordinate
 *@param x the x-coordinate
 *@return the y-value
 */
double Spline::getY(double const &x)
{
	int i;
	double y;

	if(x<=0.0 || x>=1.0) return 0.0;

	for (i=0; i<m_iRes-1; i++)
	{
		if (m_Output[i].x <m_Output[i+1].x  && m_Output[i].x <= x && x<=m_Output[i+1].x )
		{
			y = (m_Output[i].y 	+ (m_Output[i+1].y-m_Output[i].y)
			/(m_Output[i+1].x-m_Output[i].x)*(x-m_Output[i].x));
			return y;
		}
	}
	return 0.0;
}


/**
* Inserts a new point in the array of control points, using crescending x values as the key for insertion
* @param x the x-coordinate of the point to insert
* @param y the y-coordinate of the point to insert
* @return true unless the max number of points has been reached
*/
bool Spline::insertPoint(double const &x, double const &y)
{
	int k;

	if (x>=0.0 && x<=1.0)
	{ 
		//No points yet
		if(m_CtrlPoint.size()==0)
		{
			m_CtrlPoint.append(CVector(x,y,0.0));
		}
		else 
		{
			if(x<m_CtrlPoint.first().x)
			{
				// if we're the new minimum point
				m_CtrlPoint.prepend(CVector(x,y,0.0));
				m_iSelect = 0;
			}
			else if(x>=m_CtrlPoint.last().x)
			{
				// if we're the new maximum point
				m_CtrlPoint.append(CVector(x,y,0.0));
				m_iSelect = m_CtrlPoint.size();
			}
			else
			{
				// else if we're in between
				for (k=0; k<m_CtrlPoint.size()-1; k++)
				{
					if (x>=m_CtrlPoint[k].x && x<m_CtrlPoint[k+1].x)
					{
						m_CtrlPoint.insert(k+1, CVector(x,y,0.0));
						m_iSelect = k+1;
						break;
					}
				}
			}
		}
	}
	splineKnots();
	splineCurve();
	return true;
}

/**
* Checks if an input point matches with a control point
*@param Real the input point to compare with the control points
*@return the index of the first control point which matches, or -10 if none matches.
*/
int Spline::isControlPoint(CVector const &Real)
{
	int k;
	for (k=0; k<m_CtrlPoint.size(); k++)
	{
		if(qAbs(Real.x-m_CtrlPoint[k].x)<0.005 && qAbs(Real.y-m_CtrlPoint[k].y)<0.005) return k;
	}
	return -10;
}


/**
* Checks if an input point matches with a control point
*@param Real the input point to compare with the control points
*@param ZoomFactor the scaling factor to withdraw from the input point @todo withdrawal to be performed from within the calling function.
*@return the index of the first control point which matches, or -10 if none matches.
*/
int Spline::isControlPoint(CVector const &Real, double const &ZoomFactor)
{
	int k;
	for (k=0; k<m_CtrlPoint.size(); k++)
	{
		if (qAbs(Real.x-m_CtrlPoint[k].x)<0.006/ZoomFactor && qAbs(Real.y-m_CtrlPoint[k].y)<0.006/ZoomFactor) return k;
	}
	return -10;
}



/**
* Checks if an input point matches with a control point
*@param Real the input point to compare with the control points
*@param zx the scaling factor of the x-scale, to withdraw from the input point @todo withdrawal to be performed from within the calling function.
*@param zy the scaling factor of the y-scale, to withdraw from the input point @todo withdrawal to be performed from within the calling function.
*@return the index of the first control point which matches, or -10 if none matches.
*/
int Spline::isControlPoint(double const &x, double const &y, double const &zx, double const &zy)
{
	int k;
	for (k=0; k<m_CtrlPoint.size(); k++)
	{
		if(qAbs((x-m_CtrlPoint[k].x)/zx)<11.0 && qAbs((y-m_CtrlPoint[k].y)/zy)<11.0) return k;
	}
	return -10;
}

/**
* Removes a point from the array of control points, only if the remaining number of points is strictly greater than the spline's degree
*@param k the index of the control point to remove in the array
*@return false if the remaining number of points is equal or less than the spline's degree, true otherwise.
*/
bool Spline::removePoint(int const &k)
{
	if(m_CtrlPoint.size()<=m_iDegree+1) return false; // no less...

	if (k>0 && k<m_CtrlPoint.size())
	{
		m_CtrlPoint.removeAt(k);
		splineKnots();
		splineCurve();
	}
	return true;
}




/**	
 * Calculates the blending value. This is done recursively.
   If the numerator and denominator are 0 the expression is 0.
   If the denominator is 0 the expression is 0 
 *
 * @param  i   the control point's index
 * @param  p   the spline's degree 	
 * @param  t   the spline parameter
 * @return the blending value for this control point and the pair of degree and parameter values.
*/
double Spline::splineBlend(int const &i,  int const &p, double const &t)
{

	double pres = 1.e-6; //same for all the recursive calls...

	if(i+p+1>=m_knot.size())
	{
//		qDebug()<<"Error here";
		return 0.0;
	}

	if (p == 0) 
	{
		if ((m_knot[i] <= t) && (t < m_knot[i+1]) )  return  1.0;
//		else if (qAbs(m_knot[i]-m_knot[i+1])<pres)   return  0.0;
		else                                         return  0.0;
	} 
	else
	{
		if (qAbs(m_knot[i+p] - m_knot[i])<pres && qAbs(m_knot[i+p+1] - m_knot[i+1])<pres)
			return  0.0;

		else if (qAbs(m_knot[i+p] - m_knot[i])<pres)
			return  (m_knot[i+p+1]-t) / (m_knot[i+p+1]-m_knot[i+1]) * splineBlend(i+1, p-1, t);

		else if (qAbs(m_knot[i+p+1]-m_knot[i+1])<pres)
			return  (t - m_knot[i])   / (m_knot[i+p] - m_knot[i])   * splineBlend(i,   p-1, t);

		else
			return  (t - m_knot[i])   / (m_knot[i+p]-m_knot[i])	    * splineBlend(i,   p-1, t) +
					(m_knot[i+p+1]-t) / (m_knot[i+p+1]-m_knot[i+1]) * splineBlend(i+1 ,p-1, t);
	}
}


/**
* Calculates the spline's output points 
*/
void Spline::splineCurve()
{
	double t, increment, b, w;
	int i,j;

	if (m_CtrlPoint.size()>=3)
	{
		t = 0;
		increment = 1.0/(double)(m_iRes - 1);
		
		for (j=0;j<m_iRes;j++)
		{
			m_Output[j].x = 0;
			m_Output[j].y = 0;
			w=0.0;

			for (i=0; i<m_CtrlPoint.size(); i++)
			{
				b = splineBlend(i, m_iDegree, t);
				if(i!=0 && i!=m_CtrlPoint.size()-1) b *= m_PtWeight;

				m_Output[j].x += m_CtrlPoint[i].x * b;
				m_Output[j].y += m_CtrlPoint[i].y * b;
				w += b;
			}
			m_Output[j] *= 1.0/w;

			t += increment;
		}

		m_Output[m_iRes-1] = m_CtrlPoint.last();
	}
}

/**
*Generates an array of standard knot values for this spline.
*/
void Spline::splineKnots()
{
	double a,b;
	int j, iDegree;

	iDegree = qMin(m_iDegree, m_CtrlPoint.size());

	double nKnots  = iDegree + m_CtrlPoint.size() + 1;
	m_knot.clear();

	for (j=0; j<nKnots; j++)
	{
		if (j<iDegree+1)  m_knot.append(0.0);
		else 
		{
			if(j<m_CtrlPoint.size())
			{
				a = (double)(j-iDegree);
				b = (double)(nKnots-2*iDegree-1);
				if(qAbs(b)>0.0) m_knot.append(a/b);
				else            m_knot.append(1.0);
			}
			else m_knot.append(1.0);
		}
	}
}




