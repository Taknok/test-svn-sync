/****************************************************************************

	Body Class
	Copyright (C) 2007-2016 Andre Deperrois adeperrois@xflr5.com

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

/** @file
 * This file implements the definition of the Body class.
 */



#ifndef BODY_H
#define BODY_H

#include "../xflr5.h"
#include "Panel.h"
#include "NURBSSurface.h"
#include "PointMass.h"
#include <QTextStream>
#include <QVarLengthArray>
#include <QColor>

/**
 * This class :
 *	 - defines the body object,
 * 	 - provides the methods for the calculation of the plane's geometric properties,
 *   - porvides methods for the panel calculations.
 * The data is stored in International Standard Units, i.e. meters, kg, and seconds.
 * Angular data is stored in degrees.
 */
class Body
{
public:
	Body();
	~Body();

	bool isInNURBSBody(double x, double z);
	bool isInNURBSBodyOld(CVector Pt);
	bool intersect(CVector A, CVector B, CVector &I, bool bRight);
	bool intersectFlatPanels(CVector A, CVector B, CVector &I);
	bool intersectNURBS(CVector A, CVector B, CVector &I, bool bRight);
	bool serializeBodyWPA(QDataStream &ar, bool bIsStoring);
	bool serializeBodyXFL(QDataStream &ar, bool bIsStoring);
	bool importDefinition(QTextStream &inStream, double mtoUnit, QString &errorMessage);
	bool exportBodyDefinition(QTextStream &outStream, double mtoUnit);

	int insertFrame(CVector Real);
	int insertFrameBefore(int iFrame);
	int insertFrameAfter(int iFrame);
	int InsertPoint(CVector Real);
	int isFramePos(CVector Real, double ZoomFactor);
	int removeFrame(int n);
	int readFrame(QTextStream &in, int &Line, Frame *pFrame, double const &Unit);

	double Length();

	double getu(double x);
	double getv(double u, CVector r, bool bRight);
	double getSectionArcLength(double x);

	CVector centerPoint(double u);
	CVector leadingPoint();

	void clearPointMasses();
	void computeAero(double *Cp, double &XCP, double &YCP, double &ZCP,
				  double &GCm, double &GRm, double &GYm, double &Alpha, CVector &CoG);
	void duplicate(Body *pBody);
	void exportGeometry(QTextStream &outStream, int type, double mtoUnit, int nx, int nh);
	void getPoint(double u, double v, bool bRight, CVector &Pt);
	CVector Point(double u, double v, bool bRight);
	void removeActiveFrame();
	void removeSideLine(int SideLine);
	void scale(double XFactor, double YFactor, double ZFactor, bool bFrameOnly=false, int FrameID=0);
	void translate(double XTrans, double, double ZTrans, bool bFrameOnly=false, int FrameID=0);
	void translate(CVector T, bool bFrameOnly=false, int FrameID=0);
	void setNURBSKnots();
	void setPanelPos();
	void setEdgeWeight(double uw, double vw);

	Frame *frame(int iFrame);
	Frame *activeFrame();

	int setActiveFrame(Frame *pFrame);
	Frame *setActiveFrame(int iFrame);


	double framePosition(int iFrame);
	int frameSize()       {return m_SplineSurface.frameCount();}
	int framePointCount() {return m_SplineSurface.FramePointCount();}
	int sideLineCount()   {return m_SplineSurface.FramePointCount();}// same as FramePointCount();

	void computeBodyAxisInertia();
	void computeVolumeInertia(CVector &CoG, double &CoGIxx, double &CoGIyy, double &CoGIzz, double &CoGIxz);
	double totalMass();
	double &volumeMass(){return m_VolumeMass;}

	QString &bodyName(){return m_BodyName;}
	QString &bodyDescription() {return m_BodyDescription;}
	QColor &bodyColor(){return m_BodyColor;}

	XFLR5::enumBodyLineType &bodyType(){return m_LineType;}
	NURBSSurface *splineSurface() {return &m_SplineSurface;}

	//____________________VARIABLES_____________________________________________

	QString m_BodyName;                       /**< the Body's name, used as its reference */
	QString m_BodyDescription;                /**< a free description for the Body */


	NURBSSurface m_SplineSurface;             /**< the spline surface which defines the left (port) side of the body */

	XFLR5::enumBodyLineType m_LineType;              /**< the type of body surfaces 1=PANELS  2=NURBS */

	int m_iActiveFrame;		                  /**< the currently selected frame for display */
	int m_iHighlight;                         /**< the currently selected point to highlight */
	int m_iRes;                               /**< the number of output points in one direction of the NURBS surface */
	int m_NElements;                          /**< the number of mesh elements for this Body object = m_nxPanels * m_nhPanels *2 */
	int m_nxPanels;                           /**< For a NURBS body, the number of mesh elements in the direction of the x-axis */
	int m_nhPanels;                           /**< For a NURBS body, the number of mesh elements in the hoop direction */

	int m_BodyStyle;                          /**< the index of the spline's style */
	int m_BodyWidth;                          /**< the width of the spline */
	QColor m_BodyColor;                       /**< the Body's display color */

	double m_Bunch;                            /**< a bunch parameter to set the density of the points of the NURBS surface; unused */

	double m_VolumeMass;                       /**< the mass of the Body's structure, excluding point masses */
	double m_TotalMass;                        /**< the wing's total mass, i.e. the sum of the volume mass and of the point masses */
	QList<PointMass*> m_PointMass;             /**< the array of PointMass objects */

	double m_CoGIxx;                           /**< the Ixx component of the inertia tensor, calculated at the CoG */
	double m_CoGIyy;                           /**< the Ixx component of the inertia tensor, calculated at the CoG */
	double m_CoGIzz;                           /**< the Ixx component of the inertia tensor, calculated at the CoG */
	double m_CoGIxz;                           /**< the Ixx component of the inertia tensor, calculated at the CoG */
	CVector m_CoG;                             /**< the position of the CoG */


	QVarLengthArray<int> m_xPanels;              /**< the number of mesh panels between two frames */
	QVarLengthArray<int> m_hPanels;              /**< the number of mesh panels in the hoop direction between two sidelines */


	Panel *m_pBodyPanel;                       /** A pointer to the first body panel in the array */

	//allocate temporary variables to
	//avoid lengthy memory allocation times on the stack
	double value, bs, cs;
	CVector t_R, t_Prod, t_Q, t_r, t_N;
//	CVector P0, P1, P2, PI;
	static double s_XPanelPos[300];

};
#endif

