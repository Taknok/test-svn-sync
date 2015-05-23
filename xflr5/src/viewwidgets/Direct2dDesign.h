/****************************************************************************

	DirectDesignView Class
	Copyright (C) 2015 Andre Deperrois adeperrois@xflr5.com

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

#ifndef DIRECT2DDESIGN_H
#define DIRECT2DDESIGN_H

#include "section2dwidget.h"
#include "SplineFoil.h"
#include "Foil.h"

class Direct2dDesign : public Section2dWidget
{
public:
	Direct2dDesign(QWidget *pParent=NULL);

	void setObjects(Foil *pBufferFoil, SplineFoil *pSF, QList<void*> *poaFoil);

	void setScale();
	void paintEvent(QPaintEvent *event);
	void resizeEvent (QResizeEvent *event);

	int highlightPoint(CVector real);
	int selectPoint(CVector real);
	void dragSelectedPoint(double x, double y);

private slots:
	void onInsertPt();
	void onRemovePt();

private:
	void paintSplines(QPainter &painter);
	void paintFoils(QPainter &painter);
	void paintLegend(QPainter &painter);


private:
	QList<void *> *m_poaFoil;   /**< a pointer to the array of Foil objects */
	SplineFoil *m_pSF;          /**< a pointer to the SplineFoil object */
	Foil *m_pBufferFoil;

};

#endif // DIRECT2DDESIGN_H
