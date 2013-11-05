/****************************************************************************

    Spline Foil Class
	Copyright (C) 2003-2010 Andre Deperrois adeperrois@xflr5.com

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
 *@file This class defines the splined foil object used in foil design.
 */


#ifndef SPLINEFOIL_H
#define SPLINEFOIL_H




#include "Foil.h"
#include "Spline.h"



struct Picture
{
	public:
		double xExt[SPLINECONTROLSIZE];
		double yExt[SPLINECONTROLSIZE];
		double xInt[SPLINECONTROLSIZE];
		double yInt[SPLINECONTROLSIZE];
		CVector ExtRearPt, IntRearPt;
		int m_iExt, m_iInt;
};


/**
*@class SplineFoil
*@brief  The class which defines the splined foil object.

The foil is contructed based on one spline for the upper surface and one spline for the lower surface.
@todo This class mixes the construction methods and the MMI; would be better to move the MMI to a derived child class for polymorphism.
*/
class SplineFoil
{
	friend class QAFoil;
	friend class MainFrame;
	friend class SplineCtrlsDlg;
	friend class FoilTableDelegate;


public:
	SplineFoil();

private:
	bool CompMidLine();

	bool InitSplineFoil();

	bool Serialize(QDataStream &ar, bool bIsStoring);

	void Copy(SplineFoil* pSF);
	void CopyFromPicture(Picture *pPic);
	void CopyToPicture(Picture *pPic);
	void DrawCtrlPoints(QPainter &painter, double scalex, double scaley, QPoint Offset);
	void DrawMidLine(QPainter &painter, double scalex, double scaley, QPoint Offset);
	void DrawFoil(QPainter &painter, double scalex, double scaley, QPoint Offset);
	void DrawOutPoints(QPainter &painter, double scalex, double scaley, QPoint Offset);
	void ExportToBuffer(Foil *pFoil);
	void ExportToFile(QTextStream &out);
	void UpdateSplineFoil();
	void UpdateSelected(double x, double y);
	void SetCurveParams(int style, int width, QColor color);

	bool m_bModified;
	bool m_bVisible, m_bOutPoints, m_bCenterLine;

	bool m_bSymetric;
	
	int m_OutPoints;
	int	m_FoilStyle;
	int m_FoilWidth;
	double m_fCamber;
	double m_fThickness;
	double m_fxCambMax, m_fxThickMax;
	QString m_strFoilName;
	QColor m_FoilColor;
	Spline m_Extrados;
	Spline m_Intrados;
	CVector m_rpMid[1001];
};
#endif
