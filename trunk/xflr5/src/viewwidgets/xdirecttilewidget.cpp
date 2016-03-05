/****************************************************************************

	XDirectTileWidget Class
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

#include <mainframe.h>
#include "xdirecttilewidget.h"
#include "xdirect/XDirect.h"
#include <viewwidgets/oppointwidget.h>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSplitter>
#include <QAction>
#include <QtDebug>


XDirectTileWidget::XDirectTileWidget(QWidget *pParent) :  GraphTileWidget(pParent)
{
	for(int iGraph=0; iGraph<MAXPOLARGRAPHS; iGraph++)
	{
		m_GraphWidget.append(new GraphWidget(this));
	}


	m_xflr5App = XFLR5::XFOILANALYSIS;

	m_pLegendWidget = new LegendWidget(this);
	m_pOpPointWidget = new OpPointWidget(this);
//	m_pLegendStack = new QStackedWidget(this);

	m_nGraphWidgets = MAXPOLARGRAPHS;

	m_iActiveGraphWidget = 0;
	m_SingleGraphOrientation = Qt::Horizontal;

	setupMainLayout();

}

XDirectTileWidget::~XDirectTileWidget()
{

}


void XDirectTileWidget::connectSignals()
{
	MainFrame*pMainFrame = (MainFrame*)s_pMainFrame;
	connect(pMainFrame->m_pResetFoilScale,  SIGNAL(triggered()), m_pOpPointWidget, SLOT(onResetFoilScale()));
	connect(pMainFrame->m_pXDirectStyleAct, SIGNAL(triggered()), m_pOpPointWidget, SLOT(onXDirectStyle()));
	connect(pMainFrame->m_pShowNeutralLine, SIGNAL(triggered()), m_pOpPointWidget, SLOT(onShowNeutralLine()));
	connect(pMainFrame->m_pShowPanels,      SIGNAL(triggered()), m_pOpPointWidget, SLOT(onShowPanels()));

	for(int igw=0; igw<m_GraphWidget.count(); igw++)
	{
		connect(m_GraphWidget.at(igw), SIGNAL(graphChanged(QGraph*)), this, SLOT(onResetCurves(QGraph*)));
	}

	connect(m_GraphWidget.at(0), SIGNAL(graphResized(QGraph*)), m_pOpPointWidget, SLOT(onResetFoilScale()));

}


void XDirectTileWidget::setupMainLayout()
{
	m_pMainGridLayout = new QGridLayout;
	{
		for(int i=0; i<2; i++)
			for(int j=0; j<3; j++)
				if(3*i+j<m_GraphWidget.count()) m_pMainGridLayout->addWidget(m_GraphWidget.at(3*i+j),i,j);

		m_pMainGridLayout->setSpacing(0);
		m_pMainGridLayout->setMargin(0);
	}

	setLayout(m_pMainGridLayout);
}



void XDirectTileWidget::adjustLayout()
{
	QXDirect *pXDirect = (QXDirect*)s_pXDirect;

	blockSignals(true);

	for(int ig=0; ig<MAXPOLARGRAPHS; ig++)
		m_pMainGridLayout->removeWidget(m_GraphWidget.at(ig));

	m_pMainGridLayout->removeWidget(m_pOpPointWidget);
	m_pMainGridLayout->removeWidget(m_pLegendWidget);

	if(!pXDirect->bPolarView())
	{
		m_iActiveGraphWidget = 0;
		for(int igw=0; igw<m_GraphWidget.count(); igw++)
			m_GraphWidget.at(igw)->setVisible(false);

		m_pOpPointWidget->setVisible(true);
		m_pLegendWidget->setVisible(false);

//		m_pMainGridLayout->addWidget(m_GraphWidget.at(0),1,1);
		m_pMainGridLayout->addWidget(m_pOpPointWidget,1,1,2,3);
/*		m_pMainGridLayout->setRowStretch(1,5);
		m_pMainGridLayout->setRowStretch(2,3);
		m_pMainGridLayout->setColumnStretch(1,1);
		m_pMainGridLayout->setColumnStretch(2,0);
		m_pMainGridLayout->setColumnStretch(3,0);*/
	}
	else
	{
		m_pOpPointWidget->setVisible(false);
		m_pLegendWidget->setVisible(true);

		m_pMainGridLayout->setRowStretch(1,1);
		m_pMainGridLayout->setRowStretch(2,1);
		m_pMainGridLayout->setColumnStretch(1,1);
		m_pMainGridLayout->setColumnStretch(2,1);
		m_pMainGridLayout->setColumnStretch(3,1);
		if(m_nGraphWidgets==1)
		{
			for(int igw=0; igw<m_GraphWidget.count(); igw++)
				m_GraphWidget.at(igw)->setVisible(igw==m_iActiveGraphWidget);

			m_pMainGridLayout->addWidget(m_GraphWidget.at(m_iActiveGraphWidget),1,1);
			m_pMainGridLayout->addWidget(m_pLegendWidget,1,2);

			m_pMainGridLayout->setRowStretch(1,1);
			m_pMainGridLayout->setRowStretch(2,0);
			m_pMainGridLayout->setColumnStretch(1,15);
			m_pMainGridLayout->setColumnStretch(2,5);
			m_pMainGridLayout->setColumnStretch(3,0);

		}

		else if(m_nGraphWidgets==2)
		{
			for(int igw=0; igw<m_GraphWidget.count(); igw++)
				m_GraphWidget.at(igw)->setVisible(igw<2);
			m_pMainGridLayout->addWidget(m_GraphWidget.at(0),1,1);
			m_pMainGridLayout->addWidget(m_GraphWidget.at(1),1,2);
			m_pMainGridLayout->addWidget(m_pLegendWidget,2,1,1,2);

			m_pMainGridLayout->setRowStretch(1,17);
			m_pMainGridLayout->setRowStretch(2,5);
			m_pMainGridLayout->setColumnStretch(1,1);
			m_pMainGridLayout->setColumnStretch(2,1);
			m_pMainGridLayout->setColumnStretch(3,0);
		}
		else if(m_nGraphWidgets==4)
		{
			for(int igw=0; igw<m_GraphWidget.count(); igw++)
				m_GraphWidget.at(igw)->setVisible(igw==0 || igw==1 || igw==2 || igw==3);

			m_pMainGridLayout->addWidget(m_GraphWidget.at(0),1,1);
			m_pMainGridLayout->addWidget(m_GraphWidget.at(1),1,2);
			m_pMainGridLayout->addWidget(m_GraphWidget.at(2),2,1);
			m_pMainGridLayout->addWidget(m_GraphWidget.at(3),2,2);

			m_pMainGridLayout->addWidget(m_pLegendWidget,1,3,2,1);

			m_pMainGridLayout->setRowStretch(1,1);
			m_pMainGridLayout->setRowStretch(2,1);
			m_pMainGridLayout->setColumnStretch(1,3);
			m_pMainGridLayout->setColumnStretch(2,3);
			m_pMainGridLayout->setColumnStretch(3,2);
		}
		else
		{
			for(int igw=0; igw<m_GraphWidget.count(); igw++) m_GraphWidget.at(igw)->setVisible(true);
			m_pMainGridLayout->addWidget(m_GraphWidget.at(0),1,1);
			m_pMainGridLayout->addWidget(m_GraphWidget.at(1),1,2);
			m_pMainGridLayout->addWidget(m_GraphWidget.at(2),1,3);
			m_pMainGridLayout->addWidget(m_GraphWidget.at(3),2,2);
			m_pMainGridLayout->addWidget(m_GraphWidget.at(4),2,3);
			m_pMainGridLayout->addWidget(m_pLegendWidget,2,1);

			m_pMainGridLayout->setRowStretch(1,1);
			m_pMainGridLayout->setRowStretch(2,1);
			m_pMainGridLayout->setColumnStretch(1,2);
			m_pMainGridLayout->setColumnStretch(2,1);
			m_pMainGridLayout->setColumnStretch(3,1);
		}
	}

	blockSignals(false);
}



void XDirectTileWidget::setGraphList(QList<QGraph*>pGraphList, int nGraphs, int iGraphWidget, Qt::Orientation orientation)
{
	MainFrame*pMainFrame = (MainFrame*)s_pMainFrame;
	QXDirect *pXDirect = (QXDirect*)s_pXDirect;
	m_xflr5App = pMainFrame->xflr5App();
	m_nGraphWidgets = qMin(nGraphs,MAXPOLARGRAPHS);
	m_iActiveGraphWidget = iGraphWidget;

	if(!pXDirect->m_bPolarView)
	{
		for(int ig=0; ig<qMin(MAXPOLARGRAPHS, pGraphList.count()); ig++)
			m_GraphWidget.at(ig)->setGraph(NULL);
	}
	else
	{
		for(int ig=0; ig<qMin(MAXPOLARGRAPHS, pGraphList.count()); ig++)
			m_GraphWidget.at(ig)->setGraph(pGraphList.at(ig));
	}
	m_pLegendWidget->setGraph(pGraphList.at(0));
	m_SingleGraphOrientation = orientation;

	m_pOpPointWidget->setGraph(&pXDirect->m_CpGraph);
//	m_pOpPointWidget->setFoilScale();

	adjustLayout();

	m_GraphWidget.at(0)->setFocus();

	update();
}




void XDirectTileWidget::onResetCurGraphScales()
{
	if(!isVisible()) return;
	QXDirect *pXDirect = (QXDirect*)s_pXDirect;
	if(!pXDirect->bPolarView())
	{
		m_pOpPointWidget->resetGraphScale();
	}
	else if(activeGraphWidget())
	{
		activeGraphWidget()->onResetGraphScales();
	}
	setFocus();
}

















