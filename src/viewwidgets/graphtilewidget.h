/****************************************************************************

	GraphTileWidget Class
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


#ifndef GRAPHTILEWIDGET_H
#define GRAPHTILEWIDGET_H

#include <QWidget>
#include <QGraph.h>
#include <QSplitter>
#include <graphwidget.h>
#include "legendwidget.h"
#include <xflr5.h>



class GraphTileWidget : public QWidget
{
	friend class XDirectTileWidget;
	friend class MiarexTileWidget;

	Q_OBJECT
public:
	GraphTileWidget(QWidget *parent = 0);
	~GraphTileWidget();

	QGraph *graph(int iGraph);
	GraphWidget *graphWidget(int iGraph);

	XFLR5::enumApp xflr5App(){return m_xflr5App;}

	int graphWidgetCount(){return m_GraphWidget.count();}
	int activeGraph(){return m_iActiveGraphWidget;}
	void setActiveGraph(int iGraphWidget);
	void setGraphCount(int nGraphs);

	void keyPressEvent(QKeyEvent *event);
	void contextMenuEvent (QContextMenuEvent *event);

	virtual void setGraphList(QList<QGraph*>pGraphList, int nGraphs, int iGraphWidget, Qt::Orientation orientation =Qt::Horizontal);
	virtual void Connect() = 0;


	GraphWidget *graphWidget(QGraph *pGraph);
	void setCurGraph(QGraph *pGraph){m_pCurGraph = pGraph;}

private:
	virtual void adjustLayout() = 0;
	virtual void setupMainLayout() = 0;


public slots:
	void onSetActiveGraph(QGraph* pGraph = NULL);
	void onResetCurves(QGraph *pGraph = NULL);

	void onSingleGraph();
	void onAllGraphSettings();
	void onAllGraphScales();


	void onTwoGraphs();
	void onFourGraphs();
	void onAllGraphs();
	void onGraphSettings();
	void onResetCurGraphScales();
	void onExportCurGraph();


public:
	static void *s_pMainFrame;   /**< A void pointer to the instance of the MainFrame object. */
	static void *s_pMiarex;      /**< A void pointer to the instance of the QMiarex object. */
	static void *s_pXDirect;     /**< A void pointer to the instance of the QXDirect object. */


private:
	QList<GraphWidget*>m_GraphWidget;
	LegendWidget *m_pLegendWidget;
	QSplitter *m_pMainSplitter;

	int m_nGraphWidgets;
	int m_iActiveGraphWidget;

	XFLR5::enumApp m_xflr5App;
	XFLR5::enumMiarexViews m_MiarexView;

	Qt::Orientation m_SingleGraphOrientation;

	QGraph *m_pCurGraph;
};

#endif // GRAPHTILEWIDGET_H
