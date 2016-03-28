/****************************************************************************

	MiarexTileWidget Class
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

#include "miarextilewidget.h"

#include <QHBoxLayout>


MiarexTileWidget::MiarexTileWidget(QWidget *pParent) : GraphTileWidget(pParent)
{
	for(int iGraph=0; iGraph<MAXGRAPHS; iGraph++)
	{
		m_GraphWidget.append(new GraphWidget(this));
	}

	m_xflr5App = XFLR5::MIAREX;
	m_MiarexView = XFLR5::WPOLARVIEW;

	m_pLegendWidget = new LegendWidget(this);
	m_pLegendWidget->setMiarexView(m_MiarexView);
	m_pWingWidget = new WingWidget(this);


	m_nGraphWidgets = MAXGRAPHS;

	m_iActiveGraphWidget = 0;
	m_SingleGraphOrientation = Qt::Horizontal;

	setupMainLayout();
}


MiarexTileWidget::~MiarexTileWidget()
{

}


void MiarexTileWidget::Connect()
{
	for(int igw=0; igw<m_GraphWidget.count(); igw++)
	{
		connect(m_GraphWidget.at(igw), SIGNAL(graphChanged(QGraph*)), this, SLOT(onResetCurves(QGraph*)));
	}
}



void MiarexTileWidget::setupMainLayout()
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



void MiarexTileWidget::adjustLayout()
{
	blockSignals(true);

	for(int ig=0; ig<MAXGRAPHS; ig++)
		m_pMainGridLayout->removeWidget(m_GraphWidget.at(ig));

	m_pMainGridLayout->removeWidget(m_pWingWidget);
	m_pMainGridLayout->removeWidget(m_pLegendWidget);


	m_pWingWidget->setVisible(false);
	m_pLegendWidget->setVisible(true);

	if(m_nGraphWidgets==1)
	{
		if(m_MiarexView==XFLR5::WOPPVIEW)
		{
			for(int igw=0; igw<m_GraphWidget.count(); igw++)
				m_GraphWidget.at(igw)->setVisible(igw==m_iActiveGraphWidget);

			m_pWingWidget->setVisible(true);
			m_pLegendWidget->setVisible(false);

			m_pMainGridLayout->addWidget(m_GraphWidget.at(m_iActiveGraphWidget),1,1);
			m_pMainGridLayout->addWidget(m_pWingWidget,2,1);
			m_pMainGridLayout->setRowStretch(1,5);
			m_pMainGridLayout->setRowStretch(2,3);
			m_pMainGridLayout->setColumnStretch(1,1);
			m_pMainGridLayout->setColumnStretch(2,0);
			m_pMainGridLayout->setColumnStretch(3,0);
			m_pMainGridLayout->setColumnStretch(4,0);
		}
		else
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
			m_pMainGridLayout->setColumnStretch(4,0);
		}
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
		m_pMainGridLayout->setColumnStretch(4,0);
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
		m_pMainGridLayout->setColumnStretch(4,0);
	}
	else
	{
		for(int igw=0; igw<m_GraphWidget.count(); igw++) m_GraphWidget.at(igw)->setVisible(true);
		m_pMainGridLayout->addWidget(m_GraphWidget.at(0),1,1);
		m_pMainGridLayout->addWidget(m_GraphWidget.at(1),1,2);
		m_pMainGridLayout->addWidget(m_GraphWidget.at(2),1,3);
		m_pMainGridLayout->addWidget(m_GraphWidget.at(3),2,1);
		m_pMainGridLayout->addWidget(m_GraphWidget.at(4),2,2);
		m_pMainGridLayout->addWidget(m_GraphWidget.at(5),2,3);
		m_pMainGridLayout->addWidget(m_pLegendWidget,1,4,2,1);

		m_pMainGridLayout->setRowStretch(1,1);
		m_pMainGridLayout->setRowStretch(2,1);
		m_pMainGridLayout->setColumnStretch(1,1);
		m_pMainGridLayout->setColumnStretch(2,1);
		m_pMainGridLayout->setColumnStretch(3,1);
		m_pMainGridLayout->setColumnStretch(4,1);
	}


	blockSignals(false);
}




void MiarexTileWidget::setMiarexGraphList(XFLR5::enumMiarexViews miarexView, QList<QGraph*>pGraphList, int nGraphs, int iGraphWidget, Qt::Orientation orientation)
{
	m_nGraphWidgets = qMin(nGraphs,MAXGRAPHS);
	m_iActiveGraphWidget = iGraphWidget;
	m_MiarexView = miarexView;
	m_pLegendWidget->setMiarexView(m_MiarexView);

	if(miarexView==XFLR5::WOPPVIEW) m_pWingWidget->setWingGraph(m_GraphWidget.at(0)->graph());

	for(int ig=0; ig<qMin(MAXGRAPHS, pGraphList.count()); ig++)
		m_GraphWidget.at(ig)->setGraph(pGraphList.at(ig));

	m_pLegendWidget->setGraph(pGraphList.at(0));
	m_pWingWidget->setWingGraph(pGraphList.at(0));
	m_SingleGraphOrientation = orientation;

	adjustLayout();

	m_GraphWidget.at(0)->setFocus();
	update();
}


void MiarexTileWidget::onSplitterMoved(int pos, int index)
{
	if(m_MiarexView==XFLR5::WOPPVIEW)
	{
		m_pWingWidget->setWingScale();
//		update();
	}
}




