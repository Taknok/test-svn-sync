#ifndef WINGWIDGET_H
#define WINGWIDGET_H

#include <QWidget>
#include <QSettings>
#include <QPixmap>
#include "Plane.h"
#include "PlaneOpp.h"
#include <QGraph.h>


class WingWidget : public QWidget
{
	Q_OBJECT
public:
	WingWidget(QWidget *pParent = 0);
	~WingWidget();


	void contextMenuEvent (QContextMenuEvent *event);
	void keyPressEvent(QKeyEvent *event);
	void mouseDoubleClickEvent (QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void paintEvent(QPaintEvent *event);
	void resizeEvent (QResizeEvent *event);
	void wheelEvent (QWheelEvent *event);

	void setWingScale();
	void setWingGraph(QGraph *pGraph);
	void loadSettings(QSettings *pSettings);
	void saveSettings(QSettings *pSettings);

private:
	void paintXCmRef(QPainter &painter, QPointF ORef, double scale);
	void paintXCP(QPainter &painter, QPointF ORef, double scale);
	void paintXTr(QPainter &painter, QPointF ORef, double scale);
	void paintWing(QPainter &painter, QPointF ORef, double scale);


signals:

public slots:
	void onResetWingScale();

public:
	static void *s_pMainFrame;
	static void *s_pMiarex;

private:

	bool m_bXBot;                      /**< true if the transition on the bottom surface should be displayed in the operating point or in 3D view*/
	bool m_bXCmRef; 	               /**< true if the position of the reference point for the moments should be displayed in the operating point view*/
	bool m_bXCP;                       /**< true if the lift curve should be displayed in the operating point or in the 3D view*/
	bool m_bXTop;                      /**< true if the transition on the top surface should be displayed in the operating point or in 3D view */

	bool m_bTrans;
	bool m_bIs2DScaleSet;

	double m_WingScale;
	QPointF m_ptOffset;              /**< client offset position for wing display */
	QPoint m_LastPoint;           /**< The client position of the previous mousepress event */
	QGraph * m_pGraph;
	QPixmap m_PixText;
};

#endif // WINGWIDGET_H
