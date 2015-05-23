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
#include <QSplitter>
#include <QtDebug>



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

	m_splitWOpp = 5./8.;
	m_splitWPolar = 4./5.;

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
		connect(m_GraphWidget.at(igw), SIGNAL(isActive(QGraph*)), this, SLOT(onSetActiveGraph(QGraph*)));
	}
}


void MiarexTileWidget::setupMainLayout()
{
	QHBoxLayout *pMainLayout = new QHBoxLayout;
	{
		m_pMainSplitter = new QSplitter(Qt::Horizontal, this);
		{
			QWidget *p4GraphWidget = new QWidget;
			{
				QGridLayout *pGridLayout = new QGridLayout;
				{
					for(int i=0; i<2; i++)
						for(int j=0; j<3; j++)
							if(3*i+j<m_GraphWidget.count()) pGridLayout->addWidget(m_GraphWidget.at(3*i+j),i,j);

					pGridLayout->setSpacing(0);
					pGridLayout->setMargin(0);
				}
				p4GraphWidget->setLayout(pGridLayout);
			}
			m_pMainSplitter->addWidget(p4GraphWidget);

			m_pLegendStack = new QStackedWidget;
			m_pLegendStack->addWidget(m_pLegendWidget);
			m_pLegendStack->addWidget(m_pWingWidget);
			m_pMainSplitter->addWidget(m_pLegendStack);

			QList<int> splitSizes;
			splitSizes.append(5);
			splitSizes.append(3);
			m_pMainSplitter->setSizes(splitSizes);
		}

		pMainLayout->setSpacing(0);
		pMainLayout->setMargin(0);
		pMainLayout->addWidget(m_pMainSplitter);
	}
	setLayout(pMainLayout);
}




void MiarexTileWidget::adjustLayout()
{
	if(m_nGraphWidgets==1)
	{
		if(m_MiarexView==XFLR5::WOPPVIEW)
		{
			m_pMainSplitter->setOrientation(Qt::Vertical);
			m_pLegendStack->setCurrentWidget(m_pWingWidget);
			QList<int> splitSizes;
			splitSizes.append((int)(m_splitWOpp*100.0));
			splitSizes.append(100 - (int)(m_splitWOpp*100.0));
			m_pMainSplitter->setSizes(splitSizes);
		}
		else
		{
			m_pMainSplitter->setOrientation(Qt::Horizontal);
			m_pLegendStack->setCurrentWidget(m_pLegendWidget);
			QList<int> splitSizes;
			splitSizes.append((int)(m_splitWPolar*100.0));
			splitSizes.append(100 - (int)(m_splitWPolar*100.0));
			m_pMainSplitter->setSizes(splitSizes);
		}
		for(int igw=0; igw<m_GraphWidget.count(); igw++)
			m_GraphWidget.at(igw)->setVisible(igw==m_iActiveGraphWidget);
	}
	else if(m_nGraphWidgets==2)
	{
		QList<int> splitSizes;
		splitSizes.append((int)(m_splitWPolar*100.0));
		splitSizes.append(100 - (int)(m_splitWPolar*100.0));
		m_pMainSplitter->setSizes(splitSizes);
		m_pMainSplitter->setOrientation(Qt::Vertical);
		m_pLegendStack->setCurrentWidget(m_pLegendWidget);
		for(int igw=0; igw<m_GraphWidget.count(); igw++)
			m_GraphWidget.at(igw)->setVisible(igw<2);
	}
	else if(m_nGraphWidgets==4)
	{
		QList<int> splitSizes;
		splitSizes.append((int)(m_splitWPolar*100.0));
		splitSizes.append(100 - (int)(m_splitWPolar*100.0));
		m_pMainSplitter->setSizes(splitSizes);
		m_pMainSplitter->setOrientation(Qt::Horizontal);
		m_pLegendStack->setCurrentWidget(m_pLegendWidget);
		for(int igw=0; igw<m_GraphWidget.count(); igw++)
			m_GraphWidget.at(igw)->setVisible(igw==0 || igw==1 || igw==3 || igw==4);
	}
	else
	{
		QList<int> splitSizes;
		splitSizes.append((int)(m_splitWPolar*100.0));
		splitSizes.append(100 - (int)(m_splitWPolar*100.0));
		m_pMainSplitter->setSizes(splitSizes);
		m_pMainSplitter->setOrientation(Qt::Horizontal);
		m_pLegendStack->setCurrentWidget(m_pLegendWidget);
		for(int igw=0; igw<m_GraphWidget.count(); igw++)
			m_GraphWidget.at(igw)->setVisible(true);
	}
}



void MiarexTileWidget::setMiarexGraphList(XFLR5::enumMiarexViews miarexView, QList<QGraph*>pGraphList, int nGraphs, int iGraphWidget, Qt::Orientation orientation)
{
	m_nGraphWidgets = qMin(nGraphs,MAXGRAPHS);
	m_iActiveGraphWidget = iGraphWidget;
	m_MiarexView = miarexView;

	if(miarexView==XFLR5::WOPPVIEW) m_pWingWidget->setWingGraph(m_GraphWidget.at(0)->graph());

	for(int ig=0; ig<qMin(MAXGRAPHS, pGraphList.count()); ig++)
		m_GraphWidget.at(ig)->setGraph(pGraphList.at(ig));

	m_pLegendWidget->setGraph(pGraphList.at(0));
	m_pWingWidget->setWingGraph(pGraphList.at(0));
	m_SingleGraphOrientation = orientation;

	adjustLayout();

	update();
}



