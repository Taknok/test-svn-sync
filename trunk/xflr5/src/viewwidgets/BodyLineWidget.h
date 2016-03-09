/****************************************************************************

	BodyLineWidget Class
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

#ifndef BODYLINEWIDGET_H
#define BODYLINEWIDGET_H

#include "section2dwidget.h"
#include <Body.h>


class BodyLineWidget : public Section2dWidget
{
public:
	BodyLineWidget(QWidget *pParent, Body *pBody=NULL);
	void setBody(Body *pBody);
	void drawBodyLines();
	void drawBodyPoints();
	void drawScaleLegend(QPainter &painter);

	void paintEvent(QPaintEvent *event);
	void resizeEvent (QResizeEvent *event);
	void setScale();

	int highlightPoint(CVector real);
	int selectPoint(CVector real);
	void dragSelectedPoint(double x, double y);

private slots:
	void onInsertPt();
	void onRemovePt();


private:
	Body *m_pBody;
};

#endif // BODYLINEWIDGET_H
