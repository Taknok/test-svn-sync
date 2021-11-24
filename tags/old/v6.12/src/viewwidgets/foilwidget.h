/****************************************************************************

	FoilWidget Class
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

#ifndef FOILWIDGET_H
#define FOILWIDGET_H

#include <QWidget>
#include <QSettings>
#include <Polar.h>
#include <CVector.h>
#include <QGraph.h>

class FoilWidget : public QWidget
{
	Q_OBJECT
	friend class XDirectStyleDlg;

public:
	FoilWidget(QWidget *pParent=NULL);
	~FoilWidget();


	void contextMenuEvent (QContextMenuEvent *event);
	void keyPressEvent(QKeyEvent *event);
	void mouseDoubleClickEvent (QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void paintEvent(QPaintEvent *event);
	void wheelEvent (QWheelEvent *event);

	void loadSettings(QSettings *pSettings);
	void saveSettings(QSettings *pSettings);
	void setFoilScale();
	void setGraph(QGraph *pGraph){m_pCpGraph =pGraph;}

	void showPressure(bool bPressure){m_bPressure = bPressure;}
	void showBL(bool bBL){m_bBL = bBL;}


private:
	void paintPressure(QPainter &painter, double scale);
	void paintOpPoint(QPainter &painter);
	void paintBL(QPainter &painter, OpPoint* pOpPoint, double scale);

	CVector MousetoReal(QPoint point);

public slots:
	void onXDirectStyle();
	void onShowNeutralLine();
	void onShowPanels();
	void onResetFoilScale();

public slots:
    void onShowPressure(bool bPressure);
    void onShowBL(bool bBL);

public:
	static void *s_pMainFrame;

private:
	double m_fScale;
	QPointF m_FoilOffset;

	QColor m_crBLColor;         /**< the color used to draw the boundary layer */
	QColor m_crPressureColor;   /**< the color used to draw the pressure arrows */
	QColor m_crNeutralColor;    /**< the color used to draw the neutral line */
	int m_iBLStyle;             /**< the index of the style used to draw the boundary layer */
	int m_iBLWidth;             /**< the width of the line used to draw the boundary layer */
	int m_iPressureStyle;       /**< the index of the style used to draw the pressure arrows*/
	int m_iPressureWidth;       /**< the width of the line used to draw the pressure arrows */
	int m_iNeutralStyle;        /**< the index of the style used to draw the neutral line */
	int m_iNeutralWidth;        /**< the width of the line used to draw the neutral line */

	bool m_bTrans;
	bool m_bAnimate;
	bool m_bBL;                /**< true if the Boundary layer shoud be displayed */
	bool m_bPressure;          /**< true if the pressure distirbution should be displayed */
	bool m_bNeutralLine;
	bool m_bShowPanels;

	QGraph *m_pCpGraph;

	QPoint m_PointDown;
};

#endif // FOILWIDGET_H
