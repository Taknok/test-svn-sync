/****************************************************************************

	QXDirect Class
	Copyright (C) 2008-2014 Andre Deperrois adeperrois@xflr5.com

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

#include <QCheckBox>
#include <QTimer>
#include <QAction>
#include <QMenu>
#include <QStatusBar>
#include <QMessageBox>
#include <QColorDialog>
#include <QFileDialog>
#include <QGroupBox>
#include <QThread>
#include <QtDebug>
#include <math.h>

#include "../globals.h"
#include "../mainframe.h"
#include "../misc/Settings.h"
#include "XDirect.h"

#include "../graph/GraphDlg.h"

#include "../misc/PolarFilterDlg.h"
#include "../misc/ObjectPropsDlg.h"
#include "../misc/RenameDlg.h"
#include "../misc/EditPlrDlg.h"

#include "analysis/XFoilAnalysisDlg.h"
#include "analysis/XFoilAdvancedDlg.h"
#include "analysis/FoilPolarDlg.h"
#include "analysis/BatchThreadDlg.h"
#include "analysis/BatchDlg.h"

#include "geometry/TwoDPanelDlg.h"
#include "geometry/InterpolateFoilsDlg.h"
#include "geometry/NacaFoilDlg.h"
#include "geometry/FoilCoordDlg.h"
#include "geometry/FoilGeomDlg.h"
#include "geometry/TEGapDlg.h"
#include "geometry/LEDlg.h"
#include "geometry/FlapDlg.h"
#include "geometry/CAddDlg.h"

#include "XDirectStyleDlg.h"

Polar QXDirect::s_refPolar;

QList<double> QXDirect::s_ReList;
QList<double> QXDirect::s_MachList;
QList<double> QXDirect::s_NCritList;

bool QXDirect::s_bViscous = true;
bool QXDirect::s_bAlpha = true;
bool QXDirect::s_bInitBL = true;
bool QXDirect::s_bKeepOpenErrors = true;
bool QXDirect::s_bFromZero = false;

int QXDirect::s_TimeUpdateInterval = 100;

void *QXDirect::s_pMainFrame;

/** @todo is there any use for a buffer foil at all ? */


/**
*The public constructor.
*/
QXDirect::QXDirect(QWidget *parent) : QWidget(parent)
{
	setAttribute(Qt::WA_DeleteOnClose);

	setupLayout();

	m_pXFoil = new XFoil();

	m_LineStyle.m_PointStyle = 0;
	m_LineStyle.m_Width = 1;
	m_LineStyle.m_Color = QColor(0,0,0);
	m_LineStyle.m_PointStyle = 0;

	m_pAnimateTimer = new QTimer(this);
	m_posAnimate = 0; // no animation to start with
	connectSignals();

	fillComboBoxes(false);

	m_bAnimate        = false;
	m_bAnimatePlus    = false;
	m_bCpGraph        = true;
	m_bShowPanels     = false;
	m_bShowUserGraph  = true;
	m_bSequence       = false;
	m_bHighlightOpp   = false;

	m_bXPressed = m_bYPressed = false;

	m_bResetCurves    = true;

	m_bTrans          = false;
	m_bType1          = true;
	m_bType2          = true;
	m_bType3          = true;
	m_bType4          = true;
	m_bFromList       = false;
	m_bShowTextOutput = true;
	m_bNeutralLine    = true;
	m_bShowInviscid   = false;
	m_bCurOppOnly     = true;

	m_bPolarView          = true;
	m_iPlrGraph = 0;
	m_iPlrView  = XFLR5::ALLGRAPHS;
	m_XFoilVar  = 0;
	m_FoilYPos  = 150;

	m_PointDown.setX(0);
	m_PointDown.setY(0);

	m_posAnimate = 0;

	Polar::setCurPolar(NULL);
	OpPoint::setCurOpp(NULL);

	m_Alpha      = 0.0;
	m_AlphaMax   = 1.0;
	m_AlphaDelta = 0.5;
	m_Cl         = 0.0;
	m_ClMax      = 1.0;
	m_ClDelta    = 0.1;
	m_Reynolds      = 100000.0;
	m_ReynoldsMax   = 150000.0;
	m_ReynoldsDelta =  10000.0;

	for(int ig=0; ig<MAXPOLARGRAPHS; ig++)
	{
		m_PlrGraph.append(new QGraph);
		m_PlrGraph.at(ig)->setGraphName(QString("Polar_Graph_%1").arg(ig));
		m_PlrGraph.at(ig)->graphType() = QGRAPH::POLARGRAPH;
		m_PlrGraph[ig]->setXMin(0.0);
		m_PlrGraph[ig]->setXMax(0.1);
		m_PlrGraph[ig]->setYMin(-0.1);
		m_PlrGraph[ig]->setYMax(0.1);
		m_PlrGraph[ig]->setType(2);
		m_PlrGraph[ig]->setBorderColor(QColor(200,200,200));
		m_PlrGraph[ig]->setBorder(true);
		m_PlrGraph[ig]->setBorderStyle(0);
		m_PlrGraph[ig]->setBorderWidth(3);
		m_PlrGraph[ig]->setMargin(50);
		if(ig==0) m_PlrGraph[ig]->setVariables(2,1);
		if(ig==1) m_PlrGraph[ig]->setVariables(0,1);
		if(ig==2) m_PlrGraph[ig]->setVariables(0,5);
		if(ig==3) m_PlrGraph[ig]->setVariables(6,1);
		if(ig==4) m_PlrGraph[ig]->setVariables(0,10);
		if(ig==5) m_PlrGraph[ig]->setVariables(0,11);
	}

	for(int ig=0; ig<MAXPOLARGRAPHS; ig++) setGraphTitles(m_PlrGraph[ig]);

	m_CpGraph.graphType() = QGRAPH::OPPGRAPH;
	m_CpGraph.setXTitle(tr("X"));
	m_CpGraph.setYTitle(tr("Cp"));
	m_CpGraph.setInverted(true);
	m_CpGraph.setXMin(0.0);
	m_CpGraph.setXMax(1.0);
	m_CpGraph.setYMin(-0.1);
	m_CpGraph.setYMax(0.1);
	m_CpGraph.setMargin(50);
	m_CpGraph.setBorderColor(QColor(200,200,200));
	m_CpGraph.setBorder(true);
	m_CpGraph.setBorderStyle(0);
	m_CpGraph.setGraphName("Cp_Graph");
	m_CpGraph.setVariables(0,0);

	s_ReList.clear();
	s_MachList.clear();
	s_NCritList.clear();

	for(int iRe=0; iRe<12; iRe++)
	{
		s_ReList.append(0.0);
		s_MachList.append(0.0);
		s_NCritList.append(9.0);
	}

	s_ReList[0]  =   30000.0;
	s_ReList[1]  =   40000.0;
	s_ReList[2]  =   60000.0;
	s_ReList[3]  =   80000.0;
	s_ReList[4]  =  100000.0;
	s_ReList[5]  =  130000.0;
	s_ReList[6]  =  160000.0;
	s_ReList[7]  =  200000.0;
	s_ReList[8]  =  300000.0;
	s_ReList[9]  =  500000.0;
	s_ReList[10] = 1000000.0;
	s_ReList[11] = 3000000.0;
}


/**
 * The public destructor.
 */
QXDirect::~QXDirect()
{
	for(int ig=m_PlrGraph.count()-1; ig>=0; ig--)
	{
		delete m_PlrGraph.at(ig);
		m_PlrGraph.removeAt(ig);
	}
}


/** Sets the state of the window's widgets i.a.w. the state of the active ojbects and views. */
void QXDirect::setControls()
{
	setAttribute(Qt::WA_DeleteOnClose);

	MainFrame* pMainFrame = (MainFrame*)s_pMainFrame;

	if(m_bPolarView) m_pctrlMiddleControls->setCurrentIndex(1);
	else             m_pctrlMiddleControls->setCurrentIndex(0);

	if(Polar::curPolar())
	{
		QString PolarProps;
		Polar::curPolar()->getPolarProperties(PolarProps);
		m_pctrlPolarProps->setText(PolarProps);
	}
	else m_pctrlPolarProps->clear();

	pMainFrame->OpPointsAct->setChecked(!m_bPolarView);
	pMainFrame->PolarsAct->setChecked(m_bPolarView);

	pMainFrame->m_pShowPanels->setChecked(m_bShowPanels);
	pMainFrame->m_pShowNeutralLine->setChecked(m_bNeutralLine);
	pMainFrame->showInviscidCurve->setChecked(m_bShowInviscid);
	pMainFrame->showCurOppOnly->setChecked(m_bCurOppOnly);

	pMainFrame->setCpVarGraph->setChecked(m_CpGraph.yVariable()==0);
	pMainFrame->setQVarGraph->setChecked(m_CpGraph.yVariable()==1);

	int OppVar = m_CpGraph.yVariable();
	pMainFrame->CurXFoilCtPlot->setChecked(!m_bPolarView  && OppVar==2 && m_XFoilVar ==1);
	pMainFrame->CurXFoilDbPlot->setChecked(!m_bPolarView  && OppVar==2 && m_XFoilVar ==2);
	pMainFrame->CurXFoilDtPlot->setChecked(!m_bPolarView  && OppVar==2 && m_XFoilVar ==3);
	pMainFrame->CurXFoilRtLPlot->setChecked(!m_bPolarView && OppVar==2 && m_XFoilVar ==4);
	pMainFrame->CurXFoilRtPlot->setChecked(!m_bPolarView  && OppVar==2 && m_XFoilVar ==5);
	pMainFrame->CurXFoilNPlot->setChecked(!m_bPolarView   && OppVar==2 && m_XFoilVar ==6);
	pMainFrame->CurXFoilCdPlot->setChecked(!m_bPolarView  && OppVar==2 && m_XFoilVar ==7);
	pMainFrame->CurXFoilCfPlot->setChecked(!m_bPolarView  && OppVar==2 && m_XFoilVar ==8);
	pMainFrame->CurXFoilUePlot->setChecked(!m_bPolarView  && OppVar==2 && m_XFoilVar ==9);
	pMainFrame->CurXFoilHPlot->setChecked(!m_bPolarView   && OppVar==2 && m_XFoilVar ==10);

	m_pctrlShowPressure->setEnabled(!m_bPolarView && OpPoint::curOpp());
	m_pctrlShowBL->setEnabled(!m_bPolarView && OpPoint::curOpp());
	m_pctrlAnimate->setEnabled(!m_bPolarView && OpPoint::curOpp());
	m_pctrlAnimateSpeed->setEnabled(!m_bPolarView && OpPoint::curOpp());
//	m_pctrlHighlightOpp->setEnabled(m_bPolar);

	pMainFrame->m_pCurrentFoilMenu->setEnabled(Foil::curFoil());
	pMainFrame->m_pCurrentPolarMenu->setEnabled(Polar::curPolar());

	pMainFrame->renameCurFoil->setEnabled(Foil::curFoil());
	pMainFrame->pDirectDuplicateCurFoil->setEnabled(Foil::curFoil());
	pMainFrame->deleteCurFoil->setEnabled(Foil::curFoil());
	pMainFrame->exportCurFoil->setEnabled(Foil::curFoil());
	pMainFrame->renameCurFoil->setEnabled(Foil::curFoil());
	pMainFrame->setCurFoilStyle->setEnabled(Foil::curFoil());
	pMainFrame->m_pDefinePolarAct->setEnabled(Foil::curFoil());
	pMainFrame->m_pBatchAnalysisAct->setEnabled(Foil::curFoil());
	pMainFrame->deleteFoilOpps->setEnabled(Foil::curFoil());
	pMainFrame->deleteFoilPolars->setEnabled(Foil::curFoil());

	pMainFrame->editCurPolar->setEnabled(Polar::curPolar());
	pMainFrame->deletePolar->setEnabled(Polar::curPolar());
	pMainFrame->exportCurPolar->setEnabled(Polar::curPolar());
	pMainFrame->hidePolarOpps->setEnabled(Polar::curPolar());
	pMainFrame->showPolarOpps->setEnabled(Polar::curPolar());
	pMainFrame->deletePolarOpps->setEnabled(Polar::curPolar());

	pMainFrame->DerotateFoil->setEnabled(Foil::curFoil());
	pMainFrame->NormalizeFoil->setEnabled(Foil::curFoil());
	pMainFrame->RefineLocalFoil->setEnabled(Foil::curFoil());
	pMainFrame->RefineGlobalFoil->setEnabled(Foil::curFoil());
	pMainFrame->EditCoordsFoil->setEnabled(Foil::curFoil());
	pMainFrame->ScaleFoil->setEnabled(Foil::curFoil());
	pMainFrame->SetLERadius->setEnabled(Foil::curFoil());
	pMainFrame->SetTEGap->setEnabled(Foil::curFoil());
	pMainFrame->SetFlap->setEnabled(Foil::curFoil());
	pMainFrame->InterpolateFoils->setEnabled(Foil::curFoil());

	pMainFrame->m_pCurrentOppMenu->setEnabled(OpPoint::curOpp());
	pMainFrame->deleteCurOpp->setEnabled(OpPoint::curOpp());
	pMainFrame->exportCurOpp->setEnabled(OpPoint::curOpp());

	pMainFrame->checkGraphActions();
}


/**
* Connects signals and slots
*/
void QXDirect::connectSignals()
{
	MainFrame *pMainFrame = (MainFrame*)s_pMainFrame;


	connect(this, SIGNAL(projectModified()), pMainFrame, SLOT(onProjectModified()));
	connect(m_pctrlSpec1, SIGNAL(clicked()), this, SLOT(onSpec()));
	connect(m_pctrlSpec2, SIGNAL(clicked()), this, SLOT(onSpec()));
	connect(m_pctrlSpec3, SIGNAL(clicked()), this, SLOT(onSpec()));
	connect(m_pctrlAnalyze, SIGNAL(clicked()), this, SLOT(onAnalyze()));
	connect(m_pctrlAlphaMin, SIGNAL(editingFinished()), this, SLOT(onInputChanged()));
	connect(m_pctrlAlphaMax, SIGNAL(editingFinished()), this, SLOT(onInputChanged()));
	connect(m_pctrlAlphaDelta, SIGNAL(editingFinished()), this, SLOT(onInputChanged()));
	connect(m_pctrlCurveStyle, SIGNAL(activated(int)), this, SLOT(onCurveStyle(int)));
	connect(m_pctrlCurveWidth, SIGNAL(activated(int)), this, SLOT(onCurveWidth(int)));
	connect(m_pctrlPointStyle, SIGNAL(activated(int)), this, SLOT(onCurvePoints(int)));
	connect(m_pctrlCurveColor, SIGNAL(clickedLB()), this, SLOT(onCurveColor()));
	connect(m_pctrlSequence, SIGNAL(clicked()), this, SLOT(onSequence()));
	connect(m_pctrlViscous, SIGNAL(clicked()), this, SLOT(onViscous()));
	connect(m_pctrlStoreOpp, SIGNAL(clicked()), this, SLOT(onStoreOpp()));
//	connect(m_pctrlShowPoints, SIGNAL(clicked()), this, SLOT(onShowCurvePoints()));
	connect(m_pctrlShowCurve, SIGNAL(clicked()), this, SLOT(onShowCurve()));
//	connect(m_pctrlHighlightOpp, SIGNAL(clicked()), this, SLOT(OnHighlightOpp()));

	connect(m_pctrlAnimate, SIGNAL(clicked(bool)), this, SLOT(onAnimate(bool)));
	connect(m_pctrlAnimateSpeed, SIGNAL(sliderMoved(int)), this, SLOT(onAnimateSpeed(int)));
	connect(m_pAnimateTimer, SIGNAL(timeout()), this, SLOT(onAnimateSingle()));


    connect(m_pctrlShowBL,       SIGNAL(clicked(bool)), pMainFrame->m_pXDirectTileWidget->foilWidget(), SLOT(onShowBL(bool)));
    connect(m_pctrlShowPressure, SIGNAL(clicked(bool)), pMainFrame->m_pXDirectTileWidget->foilWidget(), SLOT(onShowPressure(bool)));
}


/**
* Creates a curve of the Cp graph for a specified OpPoint instance, or for all the instances of OpPoint.
* @param pOpp a pointer to the instance of the operating point, the data of which is used to build the CCurve objects
*/
void QXDirect::createOppCurves(OpPoint *pOpp)
{
	OpPoint *pOpPoint = NULL;
	if(pOpp) pOpPoint = pOpp; else pOpPoint = OpPoint::curOpp();

	Curve *pCurve1;
	QString str;
	int k;

	m_CpGraph.deleteCurves();

	if(m_bCurOppOnly && pOpPoint)
	{
		if(!pOpPoint || !pOpPoint->lineStyle().m_bIsVisible) return;
		pCurve1    = m_CpGraph.addCurve();
		
		pCurve1->setLineStyle(pOpPoint->lineStyle());

		str = QString("-Re=%1-Alpha=%2").arg(pOpPoint->Reynolds,8,'f',0).arg(pOpPoint->Alpha,5,'f',2);
		str = pOpPoint->m_strFoilName+str;
		pCurve1->setCurveName(str);

		fillOppCurve(pOpPoint, &m_CpGraph, pCurve1);

		if(m_bShowInviscid && pOpPoint)
		{
			Curve *pCpi = m_CpGraph.addCurve();
			pCpi->setPoints(pOpPoint->lineStyle().m_PointStyle);
			pCpi->setStyle(1);
			pCpi->setColor(pOpPoint->lineStyle().m_Color.darker(150));
			pCpi->setWidth(pOpPoint->lineStyle().m_Width);
			str= QString("-Re=%1-Alpha=%2_Inviscid").arg(pOpPoint->Reynolds,8,'f',0).arg(pOpPoint->Alpha,5,'f',2);
			str = pOpPoint->m_strFoilName+str;
			pCpi->setCurveName(str);
			fillOppCurve(pOpPoint, &m_CpGraph, pCpi, true);
		}
	}
	else if(!m_bCurOppOnly)
	{
		for (k=0; k<m_poaOpp->size(); k++)
		{
			pOpp = (OpPoint*)m_poaOpp->at(k);
			if (pOpp && pOpp->lineStyle().m_bIsVisible)
			{
				pCurve1    = m_CpGraph.addCurve();

//				pCurve1->setPoints(pOpp->lineStyle().m_PointStyle);
				pCurve1->setLineStyle(pOpp->lineStyle());

				str= QString("-Re=%1-Alpha=%2").arg(pOpp->Reynolds,8,'f',0).arg(pOpp->Alpha,5,'f',2);
				str = pOpp->m_strFoilName+str;
				pCurve1->setCurveName(str);

				fillOppCurve(pOpp, &m_CpGraph, pCurve1);
			}
		}
	}
}


/**
*Creates the curves of the graphs for all the visible polars.
*/
void QXDirect::createPolarCurves()
{
	// curves must be entirely reconstructed each time from the
	// operating points database, since user may have added
	// or deleted points & polars
	int k;
	Polar *pPolar;
	QString str;

	for(int ig=0; ig<MAXPOLARGRAPHS; ig++) m_PlrGraph[ig]->deleteCurves();

	for (k=0; k<m_poaPolar->size(); k++)
	{
		pPolar = (Polar*)m_poaPolar->at(k);

		if (pPolar->lineStyle().m_bIsVisible && pPolar->m_Alpha.size()>0)
		{
			if ((pPolar->m_PolarType==XFLR5::FIXEDSPEEDPOLAR  && m_bType1) ||
			    (pPolar->m_PolarType==XFLR5::FIXEDLIFTPOLAR   && m_bType2) ||
			    (pPolar->m_PolarType==XFLR5::RUBBERCHORDPOLAR && m_bType3) ||
			    (pPolar->m_PolarType==XFLR5::FIXEDAOAPOLAR    && m_bType4))
			{

				Curve* pCurve[MAXPOLARGRAPHS];
				Curve* pTr2Curve = NULL;
				for(int ig=0; ig<MAXPOLARGRAPHS; ig++)
				{
					pCurve[ig] = m_PlrGraph[ig]->addCurve();
//					pCurve[ig]->showPoints(pPolar->showPoints());
//					pCurve[ig]->setStyle(pPolar->polarStyle());
//					pCurve[ig]->setWidth(pPolar->polarWidth());
//					pCurve[ig]->setColor(pPolar->polarColor());
					pCurve[ig]->setLineStyle(pPolar->lineStyle());
					fillPolarCurve(pCurve[ig], pPolar, m_PlrGraph[ig]->xVariable(), m_PlrGraph[ig]->yVariable());
					pCurve[ig]->setCurveName(pPolar->polarName());

					if(m_PlrGraph[ig]->yVariable() == 6)	pTr2Curve = m_PlrGraph[ig]->addCurve();
					else                                    pTr2Curve = NULL;
					if(pTr2Curve)
					{
/*						pTr2Curve->showPoints(pPolar->showPoints());
						pTr2Curve->setStyle(1);
						pTr2Curve->setWidth(pPolar->polarWidth());
						pTr2Curve->setColor(pPolar->polarColor());*/
						pTr2Curve->setLineStyle(pPolar->lineStyle());
						fillPolarCurve(pTr2Curve, pPolar, m_PlrGraph[ig]->xVariable(), 7);

						str = pPolar->polarName() + " / Xtr1";
						pCurve[ig]->setCurveName(str);
						str = pPolar->polarName() + " / Xtr2";
						pTr2Curve->setCurveName(str);
					}
				}
			}
		}
	}
}





/**
* Initializes the comboboxes with the active OpPoint or Polar line style
* @param bEnable true if the comboboxes should be enable as a result
*/
void QXDirect::fillComboBoxes(bool bEnable)
{
	m_pctrlCurveColor->setEnabled(bEnable);
	m_pctrlCurveStyle->setEnabled(bEnable);
	m_pctrlCurveWidth->setEnabled(bEnable);
	m_pctrlShowCurve->setEnabled(bEnable);
	m_pctrlPointStyle->setEnabled(bEnable);

	int LineWidth[5];
	int LineStyle[5];
	int PointStyle[5];

	for (int i=0; i<5;i++)
	{
		LineStyle[i] = m_LineStyle.m_Style;
		LineWidth[i]  = m_LineStyle.m_Width;
		PointStyle[i] = m_LineStyle.m_PointStyle;
	}
	m_pStyleDelegate->setLineWidth(LineWidth); // the same selected width for all styles
	m_pStyleDelegate->setPointStyle(PointStyle); // the same selected width for all styles
	m_pStyleDelegate->setLineColor(m_LineStyle.m_Color);

	m_pWidthDelegate->setLineStyle(LineStyle); //the same selected style for all widths
	m_pWidthDelegate->setPointStyle(PointStyle); // the same selected width for all styles
	m_pWidthDelegate->setLineColor(m_LineStyle.m_Color);

	m_pPointDelegate->setLineStyle(LineStyle); //the same selected style for all widths
	m_pPointDelegate->setLineWidth(LineWidth); // the same selected width for all styles
	m_pPointDelegate->setLineColor(m_LineStyle.m_Color);

	m_pctrlCurveStyle->setLine(m_LineStyle.m_Style, m_LineStyle.m_Width, m_LineStyle.m_Color, m_LineStyle.m_PointStyle);
	m_pctrlCurveWidth->setLine(m_LineStyle.m_Style, m_LineStyle.m_Width, m_LineStyle.m_Color, m_LineStyle.m_PointStyle);
	m_pctrlPointStyle->setLine(m_LineStyle.m_Style, m_LineStyle.m_Width, m_LineStyle.m_Color, m_LineStyle.m_PointStyle);

	m_pctrlCurveColor->setColor(m_LineStyle.m_Color);
	m_pctrlCurveColor->setStyle(m_LineStyle.m_Style);
	m_pctrlCurveColor->setWidth(m_LineStyle.m_Width);
	m_pctrlCurveColor->setPointStyle(m_LineStyle.m_PointStyle);

	m_pctrlCurveStyle->update();
	m_pctrlCurveWidth->update();
	m_pctrlPointStyle->update();
	m_pctrlCurveColor->update();

	m_pctrlCurveStyle->setCurrentIndex(m_LineStyle.m_Style);
	m_pctrlCurveWidth->setCurrentIndex(m_LineStyle.m_Width-1);
	m_pctrlPointStyle->setCurrentIndex(m_LineStyle.m_PointStyle);
}


/**
* Fills the Cp graph curve with the data from the OpPoint.
*@param pOpp a pointer to the OpPoint for which the curve is drawn
*@param pGraph a pointer to the Graph to which the curve belongs
*@param pCurve a pointer to the CCurve which will be filled with the data from the OpPoint
*@param bInviscid true if the inviscid resutls should be displayed, false if the viscous results should be displayed
*/
void QXDirect::fillOppCurve(OpPoint *pOpp, Graph *pGraph, Curve *pCurve, bool bInviscid)
{
	int j;

	Foil *pOpFoil = Foil::foil(pOpp->m_strFoilName);

	switch(m_CpGraph.yVariable())
	{
		case 0:
		{
			for (j=0; j<pOpp->n; j++)
			{
				if(!bInviscid)
				{
					if(pOpp->m_bViscResults) pCurve->appendPoint(pOpFoil->x[j], pOpp->Cpv[j]);
				}
				else
				{
					pCurve->appendPoint(pOpFoil->x[j], pOpp->Cpi[j]);
				}
			}
			pGraph->setYTitle(tr("Cp"));
			break;
		}
		case 1:
		{
			for (j=0; j<pOpp->n; j++)
			{
				if(!bInviscid)
				{
					if(pOpp->m_bViscResults) pCurve->appendPoint(pOpFoil->x[j], pOpp->Qv[j]);
				}
				else
				{
					pCurve->appendPoint(pOpFoil->x[j], pOpp->Qi[j]);
				}
			}
			pGraph->setYTitle(tr("Q"));
			break;
		}
		default:
		{
			for (j=0; j<pOpp->n; j++)
			{
				if(!bInviscid)
				{
					if(pOpp->m_bViscResults) pCurve->appendPoint(pOpFoil->x[j], pOpp->Cpv[j]);
				}
				else{
					pCurve->appendPoint(pOpFoil->x[j], pOpp->Cpi[j]);
				}
			}
			pGraph->setYTitle(tr("Cp"));
			break;
		}
	}
}


/**
*Fills a CCurve object with data from a Polar object
* @param pCurve a pointer to the CCurve object to be filled with the data from the Polar object
* @param pPolar a pointer to the Polar object from which the data will extracted
* @param XVar the index of the variable for the curve's x-axis
* @param YVar the index of the variable for the curve's y-axis
*/
void QXDirect::fillPolarCurve(Curve *pCurve, Polar *pPolar, int XVar, int YVar)
{
	int i;
	QList <double> *pX;
	QList <double> *pY;

	pX = (QList <double> *) getVariable(pPolar, XVar);
	pY = (QList <double> *) getVariable(pPolar, YVar);
	double fx = 1.0;
	double fy = 1.0;

	pCurve->setSelected(-1);

	if(XVar == 3) fx = 10000.0;
	if(YVar == 3) fy = 10000.0;

	for (i=0; i<pPolar->m_Alpha.size(); i++)
	{
		if (XVar==12)
		{
			if((*pX)[i]>0.0)
			{
				if (YVar==12)
				{
					if((*pY)[i]>0.0)
					{
						pCurve->appendPoint(1.0/sqrt((*pX)[i]), 1.0/sqrt((*pY)[i]));
					}
				}
				else
				{
					pCurve->appendPoint(1.0/sqrt((*pX)[i]), (*pY)[i]*fy);
				}
			}
		}
		else{
			if (YVar==12)
			{
				if((*pY)[i]>0.0)
				{
					pCurve->appendPoint((*pX)[i]*fx, 1.0/sqrt((*pY)[i]));
				}
			}
			else
			{
				pCurve->appendPoint((*pX)[i]*fx, (*pY)[i]*fy);
			}
		}
		if(OpPoint::curOpp() && m_bHighlightOpp)
		{
			if(qAbs(pPolar->m_Alpha[i]-OpPoint::curOpp()->Alpha)<0.0001)
			{
				if(pPolar->m_PlrName==OpPoint::curOpp()->m_strPlrName  && Foil::curFoil()->foilName()==pPolar->foilName())
				{

					pCurve->setSelected(i);
				}
			}
		}
	}
}



/**
* Returns a void pointer to the array of the specified variable of the input Polar
* @param pPolar a pointer to the Polar object
* @param iVar the index of the variable for which a pointer is requested
* @return a pointer to the array of the requested variable
*/
void * QXDirect::getVariable(Polar *pPolar, int iVar)
{
	void * pVar;
	switch (iVar){
		case 0:
			pVar = &pPolar->m_Alpha;
			break;
		case 1:
			pVar = &pPolar->m_Cl;
			break;
		case 2:
			pVar = &pPolar->m_Cd;
			break;
		case 3:
			pVar = &pPolar->m_Cd;
			break;
		case 4:
			pVar = &pPolar->m_Cdp;
			break;
		case 5:
			pVar = &pPolar->m_Cm;
			break;
		case 6:
			pVar = &pPolar->m_XTr1;
			break;
		case 7:
			pVar = &pPolar->m_XTr2;
			break;
		case 8:
			pVar = &pPolar->m_HMom;
			break;
		case 9:
			pVar = &pPolar->m_Cpmn;
			break;
		case 10:
			pVar = &pPolar->m_ClCd;
			break;
		case 11:
			pVar = &pPolar->m_Cl32Cd;
			break;
		case 12:
			pVar = &pPolar->m_Cl;
			break;
		case 13:
			pVar = &pPolar->m_Re;
			break;
		case 14:
			pVar = &pPolar->m_XCp;
			break;
		default:
			pVar = &pPolar->m_Alpha;
			break;
	}
	return pVar;
}


/**
 * Overrides the QWidget's keyPressEvent method.
 * Dispatches the key press event
 * @param event the QKeyEvent
 */
void QXDirect::keyPressEvent(QKeyEvent *event)
{
	MainFrame *pMainFrame = (MainFrame*)s_pMainFrame;
	bool bShift = false;
	if(event->modifiers() & Qt::ShiftModifier)   bShift =true;
	bool bCtrl = false;
	if(event->modifiers() & Qt::ControlModifier)   bCtrl =true;

	switch (event->key())
	{
		case Qt::Key_Return:
		case Qt::Key_Enter:
			if (event->modifiers().testFlag(Qt::AltModifier) & event->modifiers().testFlag(Qt::ShiftModifier))
			{
				onOpPointProps();
				break;
			}
			else if (event->modifiers().testFlag(Qt::AltModifier))
			{
				onPolarProps();
				break;
			}

			readParams();
			if(m_pctrlAnalyze->hasFocus())  onAnalyze();
			else
			{
				activateWindow();
				m_pctrlAnalyze->setFocus();
			}
			break;
		case Qt::Key_Tab:
			readParams();
			break;
		case Qt::Key_Escape:
			stopAnimate();
			updateView();
			break;
		case Qt::Key_H:
		{
			if(m_bPolarView && event->modifiers().testFlag(Qt::ControlModifier))
			{
				onHighlightOpp();
			}
			break;
		}
		case Qt::Key_L:
			pMainFrame->onLogFile();
			break;
		case Qt::Key_X:
			m_bXPressed = true;
			break;
		case Qt::Key_Y:
			m_bYPressed = true;
			break;
		case Qt::Key_1:
			if(bCtrl)
			{
				pMainFrame->onAFoil();
				event->accept();
				return;
			}
		case Qt::Key_2:
			if(bCtrl)
			{
				pMainFrame->onAFoil();
				event->accept();
				return;
			}

		case Qt::Key_3:
			if(bCtrl)
			{
				pMainFrame->onXInverse();
				event->accept();
				return;
			}
		case Qt::Key_4:
			if(bCtrl)
			{
				pMainFrame->onXInverseMixed();
				event->accept();
				return;
			}
		case Qt::Key_5:
			break;
		case Qt::Key_6:

			if(bCtrl)
			{
				pMainFrame->onMiarex();
				event->accept();
				return;
			}
		case Qt::Key_F2:
		{
			onRenameCurFoil();
			break;
		}
		case Qt::Key_F3:
		{
			if(bShift) onCadd();
			else       onRefinePanelsGlobally();
			break;
		}
		case Qt::Key_F5:
		{
			if(!m_bPolarView) return;
			onOpPointView();
			break;
		}
		case Qt::Key_F6:
		{
			if (event->modifiers().testFlag(Qt::ShiftModifier))        onBatchAnalysis();
			else if (event->modifiers().testFlag(Qt::ControlModifier)) onMultiThreadedBatchAnalysis();
			else                                                       onDefinePolar();
			break;
		}
		case Qt::Key_F8:
		{
			if(m_bPolarView) return;
			onPolarView();
			break;
		}
		case Qt::Key_F9:
		{
			onFoilGeom();
			break;
		}
		case Qt::Key_F10:
		{
			onSetFlap();
			break;
		}
		case Qt::Key_F11:
		{
			onInterpolateFoils();
			break;
		}
		default:
			QWidget::keyPressEvent(event);
	}

	event->accept();
}



/**
 * Overrides the QWidget's keyReleaseEvent method.
 * Dispatches the key release event
 * @param event the QKeyEvent
 */
void QXDirect::keyReleaseEvent(QKeyEvent *event)
{
	switch (event->key())
	{
		case Qt::Key_X:
			if(!event->isAutoRepeat()) m_bXPressed = false;
			break;
		case Qt::Key_Y:
			if(!event->isAutoRepeat()) m_bYPressed = false;
			break;
		default:
			QWidget::keyReleaseEvent(event);
	}

	event->accept();
}


/**
 * Loads the user's default settings from the application QSettings object
 * @param pSettings a pointer to the QSettings object
 */
void QXDirect::loadSettings(QSettings *pSettings)
{
	QString str1, str2, str3;
	int b;

	pSettings->beginGroup("XDirect");
	{
		OpPoint::s_bStoreOpp       = pSettings->value("StoreOpp").toBool();
		s_bAlpha          = pSettings->value("AlphaSpec").toBool();
		s_bViscous        = pSettings->value("ViscousAnalysis").toBool();
		s_bInitBL         = pSettings->value("InitBL").toBool();
		m_bPolarView      = pSettings->value("PolarView").toBool();
		m_bShowUserGraph  = pSettings->value("UserGraph").toBool();
		m_bShowPanels     = pSettings->value("ShowPanels").toBool();
		m_bType1          = pSettings->value("Type1").toBool();
		m_bType2          = pSettings->value("Type2").toBool();
		m_bType3          = pSettings->value("Type3").toBool();
		m_bType4          = pSettings->value("Type4").toBool();
		m_bFromList       = pSettings->value("FromList").toBool();
		s_bFromZero       = pSettings->value("FromZero").toBool();
		m_bShowTextOutput = pSettings->value("TextOutput").toBool();
		m_bNeutralLine    = pSettings->value("NeutralLine").toBool();
		m_bCurOppOnly     = pSettings->value("CurOppOnly").toBool();
		m_bShowInviscid   = pSettings->value("ShowInviscid", false).toBool();
		m_bCpGraph        = pSettings->value("ShowCpGraph", true).toBool();
		m_bSequence       = pSettings->value("Sequence", false).toBool();
		m_bHighlightOpp   = pSettings->value("HighlightOpp", false).toBool();
		m_bHighlightOpp = false;

		m_XFoilVar       = pSettings->value("XFoilVar").toInt();
		s_TimeUpdateInterval = pSettings->value("TimeUpdateInterval",100).toInt();

		BatchThreadDlg::s_bUpdatePolarView = pSettings->value("BatchUpdatePolarView", false).toBool();


		m_iPlrGraph      = pSettings->value("PlrGraph").toInt();

		switch(pSettings->value("PlrView").toInt())
		{
			case 1:
				m_iPlrView = XFLR5::ONEGRAPH;
				break;
			case 2:
				m_iPlrView = XFLR5::TWOGRAPHS;
				break;
			case 4:
				m_iPlrView = XFLR5::FOURGRAPHS;
				break;
			default:
				m_iPlrView = XFLR5::ALLGRAPHS;
				break;
		}

		m_Alpha           = pSettings->value("AlphaMin").toDouble();
		m_AlphaMax        = pSettings->value("AlphaMax").toDouble();
		m_AlphaDelta      = pSettings->value("AlphaDelta").toDouble();
		m_Cl              = pSettings->value("ClMin").toDouble();
		m_ClMax           = pSettings->value("ClMax").toDouble();
		m_ClDelta         = pSettings->value("ClDelta").toDouble();
		m_Reynolds        = pSettings->value("ReynoldsMin").toDouble();
		m_ReynoldsMax     = pSettings->value("ReynoldsMax").toDouble();
		m_ReynoldsDelta   = pSettings->value("ReynolsDelta").toDouble();
		m_pXFoil->vaccel  = pSettings->value("VAccel").toDouble();
		s_bKeepOpenErrors = pSettings->value("KeepOpenErrors").toBool();

		XFoilTask::s_bAutoInitBL    = pSettings->value("AutoInitBL").toBool();
		XFoilTask::s_IterLim        = pSettings->value("IterLim", 100).toInt();

		XFoil::s_bFullReport = pSettings->value("FullReport").toBool();

		s_refPolar.m_ACrit    = pSettings->value("NCrit").toDouble();
		s_refPolar.m_XTop     = pSettings->value("XTopTr").toDouble();
		s_refPolar.m_XBot     = pSettings->value("XBotTr").toDouble();
		s_refPolar.m_Mach     = pSettings->value("Mach").toDouble();
		s_refPolar.m_ASpec    = pSettings->value("ASpec").toDouble();

		b = pSettings->value("Type").toInt();
		if(b==1)      s_refPolar.setPolarType(XFLR5::FIXEDSPEEDPOLAR);
		else if(b==2) s_refPolar.setPolarType(XFLR5::FIXEDLIFTPOLAR);
		else if(b==3) s_refPolar.setPolarType(XFLR5::RUBBERCHORDPOLAR);
		else if(b==4) s_refPolar.setPolarType(XFLR5::FIXEDAOAPOLAR);


		int NRe = pSettings->value("NReynolds").toInt();
		s_ReList.clear();
		s_MachList.clear();
		s_NCritList.clear();
		for (int i=0; i<NRe; i++)
		{
			str1 = QString("ReList%1").arg(i);
			str2 = QString("MaList%1").arg(i);
			str3 = QString("NcList%1").arg(i);
			s_ReList.append(pSettings->value(str1).toDouble());
			s_MachList.append(pSettings->value(str2).toDouble());
			s_NCritList.append(pSettings->value(str3).toDouble());
		}
	}
	pSettings->endGroup();

	for(int ig=0; ig<m_PlrGraph.count(); ig++) m_PlrGraph[ig]->loadSettings(pSettings);

	m_CpGraph.loadSettings(pSettings);

	if(m_CpGraph.yVariable() == 0 || m_CpGraph.yVariable()>=2)
	{
		m_CpGraph.setYTitle(tr("Cp"));
		m_CpGraph.setInverted(true);
	}
	else
	{
		m_CpGraph.setYTitle(tr("Q"));
		m_CpGraph.setInverted(false);
	}

	for(int ig=0; ig<MAXPOLARGRAPHS; ig++) setGraphTitles(m_PlrGraph.at(ig));
}




/**
 * The user has changed one of the analysis parameters. Reads all the data and maps it.
 */
void QXDirect::onInputChanged()
{
	readParams();
}


/**
 * The user has clicked the animate checkcbox
 * @param bChecked the new state of the checkbox
 */
void QXDirect::onAnimate(bool bChecked)
{
	if(!Foil::curFoil() || !Polar::curPolar())
	{
		m_bAnimate = false;
		return;
	}

	OpPoint* pOpPoint;
	int l;

	if(bChecked)
	{
		for (l=0; l< m_poaOpp->size(); l++)
		{
			pOpPoint = (OpPoint*)m_poaOpp->at(l);

			if (pOpPoint &&
				pOpPoint->m_strPlrName  == Polar::curPolar()->polarName() &&
				pOpPoint->m_strFoilName == Foil::curFoil()->foilName())
			{
					if(qAbs(OpPoint::curOpp()->Alpha - pOpPoint->Alpha)<0.0001)
						m_posAnimate = l-1;
			}
		}
		m_bAnimate  = true;
		int speed = m_pctrlAnimateSpeed->value();
		m_pAnimateTimer->setInterval(800-speed);
		m_pAnimateTimer->start();
	}
	else
	{
		m_pAnimateTimer->stop();
		m_bAnimate = false;
		if(m_posAnimate<0 || m_posAnimate>=m_poaOpp->size()) return;
		OpPoint* pOpPoint = (OpPoint*)m_poaOpp->at(m_posAnimate);
		if(pOpPoint) setOpp(pOpPoint->Alpha);
//		UpdateView();
		return;
	}
}


/**
 * Called by the animation timer.
 * Updates the display with the data of the next OpPoint.
 */
void QXDirect::onAnimateSingle()
{
	static int indexCbBox;
	static QString str;
	bool bIsValid = false;
	MainFrame *pMainFrame = (MainFrame*)s_pMainFrame;

	OpPoint* pOpPoint;

	if(m_poaOpp->size()<=1) return;

	// find the next oppoint related to this foil and polar pair
	while(!bIsValid)
	{
		if(m_bAnimatePlus)
		{
			m_posAnimate++;
			if (m_posAnimate >= m_poaOpp->size())
			{
				m_posAnimate = m_poaOpp->size()-2;
				m_bAnimatePlus = false;
			}
		}
		else
		{
			m_posAnimate--;
			if (m_posAnimate <0)
			{
				m_posAnimate = 1;
				m_bAnimatePlus = true;
			}
		}
		if(m_posAnimate<0 || m_posAnimate>=m_poaOpp->size()) return;

		pOpPoint = (OpPoint*)m_poaOpp->at(m_posAnimate);

		if (pOpPoint &&
			pOpPoint->m_strPlrName  == Polar::curPolar()->polarName() &&
			pOpPoint->m_strFoilName == Foil::curFoil()->foilName() &&
			pOpPoint != OpPoint::curOpp())
		{
			bIsValid = true;
			createOppCurves(pOpPoint);
			OpPoint::setCurOpp(pOpPoint);

			//select current OpPoint in Combobox
			if(Polar::curPolar()->polarType()!=XFLR5::FIXEDAOAPOLAR) str = QString("%1").arg(OpPoint::curOpp()->Alpha,8,'f',2);
			else                                              str = QString("%1").arg(OpPoint::curOpp()->Reynolds,8,'f',2);
			indexCbBox = pMainFrame->m_pctrlOpPoint->findText(str);
			if(indexCbBox>=0) pMainFrame->m_pctrlOpPoint->setCurrentIndex(indexCbBox);

			updateView();
		}
	}
}


/**
 * the user has moved the slider which defines the animation speed
 * @param val the slider's new position
 */
void QXDirect::onAnimateSpeed(int val)
{
	if(m_pAnimateTimer->isActive())
	{
		m_pAnimateTimer->setInterval(1000-val);
	}
}



/**
 * The user has clicked the analyze button.
 *
 * Reads the input parameters, initializes the analysis dialog box, and starts the analysis.
 */
void QXDirect::onAnalyze()
{
	if(!Foil::curFoil() || !Polar::curPolar()) return;

	MainFrame *pMainFrame = (MainFrame*)s_pMainFrame;

	readParams();

	m_pctrlAnalyze->setEnabled(false);

	bool bHigh = m_bHighlightOpp;
	m_bHighlightOpp = false;

	XFoilAnalysisDlg *pXFADlg = new XFoilAnalysisDlg;

	if(m_bSequence)
	{
		pXFADlg->SetAlpha(m_Alpha, m_AlphaMax, m_AlphaDelta);
		pXFADlg->SetCl(m_Cl, m_ClMax, m_ClDelta);
		pXFADlg->SetRe(m_Reynolds, m_ReynoldsMax, m_ReynoldsDelta);
	}
	else
	{
		pXFADlg->SetAlpha(m_Alpha, m_Alpha, m_AlphaDelta);
		pXFADlg->SetCl(m_Cl, m_Cl, m_ClDelta);
		pXFADlg->SetRe(m_Reynolds, m_Reynolds, m_ReynoldsDelta);
	}

	pXFADlg->m_bAlpha = s_bAlpha;

	pXFADlg->InitDialog();
	pXFADlg->show();
	pXFADlg->Analyze();

	if(!s_bKeepOpenErrors || !pXFADlg->m_bErrors) pXFADlg->hide();

	delete pXFADlg;

	// and update window
	emit projectModified();

	m_pctrlAnalyze->setEnabled(true);

	s_bInitBL = !m_pXFoil->lblini;
	m_pctrlInitBL->setChecked(s_bInitBL);;

	pMainFrame->updateOppListBox();

	setOpp();

	m_bHighlightOpp = bHigh;

	m_bResetCurves = true;

	emit projectModified();

	setControls();
	updateView();
}



/**
 * Launches a single-threaded batch analysis
 */
void QXDirect::onBatchAnalysis()
{
	MainFrame* pMainFrame = (MainFrame*)s_pMainFrame;
	if(!Foil::curFoil()) return;

	m_bPolarView = true;
	onPolarView();
	updateView();

	m_pctrlAnalyze->setEnabled(false);

	BatchDlg *pBatchDlg = new BatchDlg;
	pBatchDlg->m_pFoil     = Foil::curFoil();
	pBatchDlg->m_bAlpha    = true;

	pBatchDlg->m_SpMin     = m_Alpha;
	pBatchDlg->m_SpMax     = m_AlphaMax;
	pBatchDlg->m_SpInc     = m_AlphaDelta;
	pBatchDlg->m_AlphaMin  = m_Alpha;
	pBatchDlg->m_AlphaMax  = m_AlphaMax;
	pBatchDlg->m_AlphaInc  = m_AlphaDelta;
	pBatchDlg->m_ClMin     = m_Cl;
	pBatchDlg->m_ClMax     = m_ClMax;
	pBatchDlg->m_ClInc     = m_ClDelta;
	pBatchDlg->m_ReMin     = m_Reynolds;
	pBatchDlg->m_ReMax     = m_ReynoldsMax;
	pBatchDlg->m_ReInc     = m_ReynoldsDelta;

	pBatchDlg->m_bFromList = m_bFromList;
	pBatchDlg->m_bFromZero = s_bFromZero;
	pBatchDlg->InitDialog();

	if(pBatchDlg->exec()==QDialog::Accepted) emit projectModified();

	m_Reynolds         = pBatchDlg->m_ReMin;
	m_ReynoldsMax      = pBatchDlg->m_ReMax;
	m_ReynoldsDelta    = pBatchDlg->m_ReInc;
	m_Alpha            = pBatchDlg->m_AlphaMin;
	m_AlphaMax         = pBatchDlg->m_AlphaMax;
	m_AlphaDelta       = pBatchDlg->m_AlphaInc;
	m_Cl               = pBatchDlg->m_ClMin;
	m_ClMax            = pBatchDlg->m_ClMax;
	m_ClDelta          = pBatchDlg->m_ClInc;
	s_bAlpha           = pBatchDlg->m_bAlpha;
	m_bFromList        = pBatchDlg->m_bFromList;
	s_bFromZero        = pBatchDlg->m_bFromZero;

	delete pBatchDlg;

	setPolar();
	pMainFrame->updatePolarListBox();

	m_pctrlAnalyze->setEnabled(true);

	setOpp();

	emit projectModified();

	setControls();
	updateView();
}


/**
 * Launches a multi-threaded batch analysis
 *
 */
void QXDirect::onMultiThreadedBatchAnalysis()
{
	MainFrame* pMainFrame = (MainFrame*)s_pMainFrame;
	if(!Foil::curFoil()) 		return;

	if(QThread::idealThreadCount()<2)
	{
		QString strange = tr("Not enough threads available for multithreading");
		QMessageBox::warning(pMainFrame, tr("Warning"), strange);
		return;
	}

	m_bPolarView = true;
	onPolarView();
	updateView();

	m_pctrlAnalyze->setEnabled(false);

	BatchThreadDlg *pBatchThreadDlg   = new BatchThreadDlg;

	pBatchThreadDlg->m_pCurFoil  = Foil::curFoil();

	pBatchThreadDlg->m_bAlpha    = true;
	pBatchThreadDlg->m_AlphaMin  = m_Alpha;
	pBatchThreadDlg->m_AlphaMax  = m_AlphaMax;
	pBatchThreadDlg->m_AlphaInc  = m_AlphaDelta;
	pBatchThreadDlg->m_ClMin     = m_Cl;
	pBatchThreadDlg->m_ClMax     = m_ClMax;
	pBatchThreadDlg->m_ClInc     = m_ClDelta;
	pBatchThreadDlg->m_ReMin     = m_Reynolds;
	pBatchThreadDlg->m_ReMax     = m_ReynoldsMax;
	pBatchThreadDlg->m_ReInc     = m_ReynoldsDelta;

	pBatchThreadDlg->m_bFromList = m_bFromList;
	pBatchThreadDlg->m_bFromZero = s_bFromZero;
	pBatchThreadDlg->InitDialog();

	pBatchThreadDlg->exec();

	m_Reynolds         = pBatchThreadDlg->m_ReMin;
	m_ReynoldsMax      = pBatchThreadDlg->m_ReMax;
	m_ReynoldsDelta    = pBatchThreadDlg->m_ReInc;
	m_Alpha            = pBatchThreadDlg->m_AlphaMin;
	m_AlphaMax         = pBatchThreadDlg->m_AlphaMax;
	m_AlphaDelta       = pBatchThreadDlg->m_AlphaInc;
	m_Cl               = pBatchThreadDlg->m_ClMin;
	m_ClMax            = pBatchThreadDlg->m_ClMax;
	m_ClDelta          = pBatchThreadDlg->m_ClInc;
	s_bAlpha           = pBatchThreadDlg->m_bAlpha;
	m_bFromList        = pBatchThreadDlg->m_bFromList;
	s_bFromZero        = pBatchThreadDlg->m_bFromZero;

	delete pBatchThreadDlg;

	setPolar();
	pMainFrame->updatePolarListBox();

	m_pctrlAnalyze->setEnabled(true);


	pMainFrame->updateOppListBox();

	setOpp();


	setControls();
	updateView();

	emit projectModified();

}



/**
 * The user has requested the plot of the Cf variable using current XFoil results.
 */
void QXDirect::onCfPlot()
{
	if(!m_pXFoil->lvconv) return;
	int i;
	double x[IVX][3],y[IVX][3];
	int nside1, nside2, ibl;

	m_CpGraph.setYVariable(2);
	m_XFoilVar = 8;
	m_CpGraph.deleteCurves();
	m_CpGraph.resetLimits();
	m_CpGraph.setAuto(true);
	m_CpGraph.setInverted(false);
	m_CpGraph.setYTitle(tr("Cf"));
	Curve * pTopCurve = m_CpGraph.addCurve();
	Curve * pBotCurve = m_CpGraph.addCurve();
	pTopCurve->setCurveName(tr("Top"));
	pBotCurve->setCurveName(tr("Bot"));

	double que = 0.5*m_pXFoil->qinf*m_pXFoil->qinf;

	m_pXFoil->CreateXBL(x, nside1, nside2);
	//---- fill compressible ue arrays
	for (ibl=2; ibl<= nside1;ibl++)
	{
		y[ibl][1] = m_pXFoil->tau[ibl][1] / que;
	}
	for ( ibl=2; ibl<= nside2;ibl++)
	{
		y[ibl][2] = m_pXFoil->tau[ibl][2] / que;
	}

	for (i=2; i<=nside1-1; i++)
	{
		pTopCurve->appendPoint(x[i][1], y[i][1]);
	}
	for (i=2; i<=nside2-1; i++)
	{
		pBotCurve->appendPoint(x[i][2], y[i][2]);
	}
	m_CpGraph.SetXScale();
	setFoilScale();
	setControls();
	updateView();
}


/**
 * The user has requested the plot of the Ct variable using current XFoil results.
 */
void QXDirect::onCtPlot()
{
	if(!m_pXFoil->lvconv) return;
	int i;

	m_CpGraph.setYVariable(2);
	m_XFoilVar=1;
	m_CpGraph.deleteCurves();
	m_CpGraph.resetLimits();
	m_CpGraph.setAuto(true);
	m_CpGraph.setInverted(false);
	m_CpGraph.setYTitle(tr("Max Shear"));
	Curve * pCurve0 = m_CpGraph.addCurve();
	Curve * pCurve1 = m_CpGraph.addCurve();
	Curve * pCurve2 = m_CpGraph.addCurve();
	Curve * pCurve3 = m_CpGraph.addCurve();
	pCurve0->setCurveName(tr("Top Shear"));
	pCurve1->setCurveName(tr("Top Shear eq"));
	pCurve2->setCurveName(tr("Bot Shear"));
	pCurve3->setCurveName(tr("Bot Shear eq"));

	double x[IVX][3];
	int nside1, nside2;

	m_pXFoil->CreateXBL(x, nside1, nside2);

	int it1 = m_pXFoil->itran[1];
	int it2 = m_pXFoil->itran[2];

	for (i=it1; i<=nside1-1; i++)	pCurve0->appendPoint(x[i][1], m_pXFoil->ctau[i][1]);
	for (i=2; i<=nside1-1; i++)		pCurve1->appendPoint(x[i][1], m_pXFoil->ctq[i][1]);

	for (i=it2; i<=nside2-1; i++)	pCurve2->appendPoint(x[i][2], m_pXFoil->ctau[i][2]);
	for (i=2; i<=nside2-1; i++)		pCurve3->appendPoint(x[i][2], m_pXFoil->ctq[i][2]);

	m_CpGraph.SetXScale();
	setFoilScale();
	setControls();
	updateView();

}


/**
 * The user has requested the plot of the Dt variable using current XFoil results.
 */
void QXDirect::onDtPlot()
{
	if(!m_pXFoil->lvconv) return;
	int i;

	m_CpGraph.setYVariable(2);
	m_XFoilVar=3;
	m_CpGraph.deleteCurves();
	m_CpGraph.resetLimits();
	m_CpGraph.setAuto(true);
	m_CpGraph.setInverted(false);
	m_CpGraph.setYTitle(" ");


	double x[IVX][3];
	int nside1, nside2;

	Curve * pCurve1 = m_CpGraph.addCurve();
	Curve * pCurve2 = m_CpGraph.addCurve();

	pCurve1->setCurveName("D*");
	pCurve2->setCurveName("Theta");
	m_pXFoil->CreateXBL(x, nside1, nside2);

	for (i=2; i<nside1; i++){
		pCurve1->appendPoint(x[i][1], m_pXFoil->dstr[i][1]);
		pCurve2->appendPoint(x[i][1], m_pXFoil->thet[i][1]);
	}

	m_CpGraph.SetXScale();
	setFoilScale();
	setControls();
	updateView();
}


/**
 * The user has requested the plot of the Db variable using current XFoil results.
 */
void QXDirect::onDbPlot()
{
	if(!m_pXFoil->lvconv) return;
	int i;

	m_CpGraph.setYVariable(2);
	m_XFoilVar = 2;
	m_CpGraph.deleteCurves();
	m_CpGraph.resetLimits();
	m_CpGraph.setAuto(true);
	m_CpGraph.setInverted(false);
	m_CpGraph.setYTitle(" ");


	double x[IVX][3];
	int nside1, nside2;

	Curve * pCurve1 = m_CpGraph.addCurve();
	Curve * pCurve2 = m_CpGraph.addCurve();

	pCurve1->setCurveName("D*");
	pCurve2->setCurveName("Theta");
	m_pXFoil->CreateXBL(x, nside1, nside2);

	for (i=2; i<nside2; i++)
	{
		pCurve1->appendPoint(x[i][2], m_pXFoil->dstr[i][2]);
		pCurve2->appendPoint(x[i][2], m_pXFoil->thet[i][2]);
	}

	m_CpGraph.SetXScale();
	setFoilScale();
	setControls();
	updateView();
}


/**
 * The user has requested the plot of the Cd variable using current XFoil results.
 */
void QXDirect::onCdPlot()
{
	if(!m_pXFoil->lvconv) return;
	double x[IVX][3],y[IVX][3];
	int nside1, nside2, ibl;
	int i;

	m_CpGraph.setYVariable(2);
	m_XFoilVar = 7;
	m_CpGraph.deleteCurves();
	m_CpGraph.resetLimits();
	m_CpGraph.setAuto(true);
	m_CpGraph.setInverted(false);
	m_CpGraph.setYTitle(tr("Cd'"));
	Curve * pTopCurve = m_CpGraph.addCurve();
	Curve * pBotCurve = m_CpGraph.addCurve();
	pTopCurve->setCurveName(tr("Top"));
	pBotCurve->setCurveName(tr("Bot"));

	double qrf = m_pXFoil->qinf;

	m_pXFoil->CreateXBL(x, nside1, nside2);
	//---- fill compressible ue arrays
	for (ibl=2; ibl<= nside1;ibl++)
	{
		y[ibl][1] = m_pXFoil->dis[ibl][1] / qrf/ qrf/ qrf;
	}
	for ( ibl=2; ibl<= nside2;ibl++)
	{
		y[ibl][2] = m_pXFoil->dis[ibl][2] / qrf/ qrf/ qrf;
	}

	for (i=2; i<=nside1-1; i++)
	{
		pTopCurve->appendPoint(x[i][1], y[i][1]);
	}
	for (i=2; i<=nside2-1; i++)
	{
		pBotCurve->appendPoint(x[i][2], y[i][2]);
	}
	m_CpGraph.SetXScale();
	setFoilScale();
	setControls();
	updateView();
}


/**
 * The user has requested the plot of the Hk variable using current XFoil results.
 */
void QXDirect::onHPlot()
{
	if(!m_pXFoil->lvconv) return;
	int i;

	m_CpGraph.setYVariable(2);
	m_XFoilVar = 10;
	m_CpGraph.deleteCurves();
	m_CpGraph.resetLimits();
	m_CpGraph.setAuto(true);
	m_CpGraph.setInverted(false);
	m_CpGraph.setYTitle("Hk");
	Curve * pTopCurve = m_CpGraph.addCurve();
	Curve * pBotCurve = m_CpGraph.addCurve();
	pTopCurve->setCurveName(tr("Top"));
	pBotCurve->setCurveName(tr("Bot"));

	double x[IVX][3],y[IVX][3];
	int nside1, nside2;

	m_pXFoil->CreateXBL(x, nside1, nside2);
	m_pXFoil->FillHk(y, nside1, nside2);

	for (i=2; i<=nside1-1; i++)
	{
		pTopCurve->appendPoint(x[i][1], y[i][1]);
	}
	for (i=2; i<=nside2-1; i++)
	{
		pBotCurve->appendPoint(x[i][2], y[i][2]);
	}

	m_CpGraph.SetXScale();
	setFoilScale();
	setControls();
	updateView();
}


/**
 * The user has requested the plot of the Rt variable using current XFoil results.
 */
void QXDirect::onRtPlot()
{
	if(!m_pXFoil->lvconv) return;
	int i;

	m_CpGraph.setYVariable(2);
	m_XFoilVar=5;
	m_CpGraph.deleteCurves();
	m_CpGraph.resetLimits();
	m_CpGraph.setAuto(true);
	m_CpGraph.setInverted(false);
	m_CpGraph.setYTitle("Re_Theta");
	Curve * pTopCurve = m_CpGraph.addCurve();
	Curve * pBotCurve = m_CpGraph.addCurve();
	pTopCurve->setCurveName(tr("Top"));
	pBotCurve->setCurveName(tr("Bot"));

	double x[IVX][3],y[IVX][3];
	int nside1, nside2;

	m_pXFoil->CreateXBL(x, nside1, nside2);
	m_pXFoil->FillRTheta(y, nside1, nside2);

	for (i=2; i<=nside1-1; i++)	pTopCurve->appendPoint(x[i][1], y[i][1]);
	for (i=2; i<=nside2-1; i++) pBotCurve->appendPoint(x[i][2], y[i][2]);

	m_CpGraph.SetXScale();
	setFoilScale();
	setControls();
	updateView();
}


/**
 * The user has requested the plot of the RtL variable using current XFoil results.
 */
void QXDirect::onRtLPlot()
{
	if(!m_pXFoil->lvconv) return;
	int i;

	m_CpGraph.setYVariable(2);
	m_XFoilVar=4;
	m_CpGraph.deleteCurves();
	m_CpGraph.resetLimits();
	m_CpGraph.setAuto(true);
	m_CpGraph.setInverted(false);
	m_CpGraph.setYTitle("Re_Theta");
	Curve * pTopCurve = m_CpGraph.addCurve();
	Curve * pBotCurve = m_CpGraph.addCurve();
	pTopCurve->setCurveName(tr("Top"));
	pBotCurve->setCurveName(tr("Bot"));

	double x[IVX][3],y[IVX][3];
	int nside1, nside2;

	m_pXFoil->CreateXBL(x, nside1, nside2);
	m_pXFoil->FillRTheta(y, nside1, nside2);

	for (i=2; i<=nside1-1; i++){
		if (y[i][1]>0.0) y[i][1] = log10( y[i][1] );
		else             y[i][1] = 0.0;
		pTopCurve->appendPoint(x[i][1], y[i][1]);
	}
	for (i=2; i<=nside2-1; i++){
		if (y[i][2]>0.0) y[i][2] = log10( y[i][2] );
		else             y[i][2] = 0.0;
		pBotCurve->appendPoint(x[i][2], y[i][2]);
	}
	m_CpGraph.SetXScale();
	setFoilScale();
	setControls();
	updateView();
}


/**
 * The user has requested the plot of the Ue variable using current XFoil results.
 */
void QXDirect::onUePlot()
{
	if(!m_pXFoil->lvconv) return;
	int i;
	double x[IVX][3],y[IVX][3];
	double uei;
	int nside1, nside2, ibl;

	m_CpGraph.setYVariable(2);
	m_XFoilVar = 9;
	m_CpGraph.deleteCurves();
	m_CpGraph.resetLimits();
	m_CpGraph.setAuto(true);
	m_CpGraph.setInverted(false);
	m_CpGraph.setYTitle("Ue/Vinf");
	Curve * pTopCurve = m_CpGraph.addCurve();
	Curve * pBotCurve = m_CpGraph.addCurve();
	pTopCurve->setCurveName(tr("Top"));
	pBotCurve->setCurveName(tr("Bot"));

	m_pXFoil->CreateXBL(x, nside1, nside2);
	//---- fill compressible ue arrays
	for (ibl=2; ibl<= nside1;ibl++)
	{
		uei = m_pXFoil->uedg[ibl][1];
		y[ibl][1] = uei * (1.0-m_pXFoil->tklam)
						/ (1.0-m_pXFoil->tklam*(uei/m_pXFoil->qinf)*(uei/m_pXFoil->qinf));
	}
	for (ibl=2; ibl<= nside2;ibl++)
	{
		uei = m_pXFoil->uedg[ibl][2];
		y[ibl][2] = uei * (1.0-m_pXFoil->tklam)
						/ (1.0-m_pXFoil->tklam*(uei/m_pXFoil->qinf)*(uei/m_pXFoil->qinf));
	}

	for (i=2; i<=nside1-1; i++)
	{
		pTopCurve->appendPoint(x[i][1], y[i][1]);
	}
	for (i=2; i<=nside2-1; i++)
	{
		pBotCurve->appendPoint(x[i][2], y[i][2]);
	}
	m_CpGraph.SetXScale();
	setFoilScale();
	setControls();
	updateView();
}




/**
 * The user has requested to switch to the Cp graph view
 */
void QXDirect::onCpGraph()
{
	m_bPolarView = false;
	if(m_CpGraph.yVariable()!=0)
	{
//		m_pCpGraph->ResetLimits();
		m_CpGraph.setAuto(true);
		m_CpGraph.setYVariable(0);
	}

	m_CpGraph.setInverted(true);
	m_bResetCurves = true;
	m_CpGraph.setYTitle(tr("Cp"));

	setControls();
	m_CpGraph.SetXScale();
	setFoilScale();
	updateView();
}


/**
 * The user has toggled the request for the display of the inviscid Cp curve
 */
void QXDirect::onCpi()
{
	m_bShowInviscid = !m_bShowInviscid;

	m_bResetCurves = true;
	setControls();
	updateView();
}


/**
 * The user has toggled the switch for the display of the current OpPoint only
 */
void QXDirect::onCurOppOnly()
{
	m_bCurOppOnly = !m_bCurOppOnly;
	((MainFrame*)s_pMainFrame)->showCurOppOnly->setChecked(m_bCurOppOnly);

	if(OpPoint::curOpp()) OpPoint::curOpp()->lineStyle().m_bIsVisible = true;
	m_bResetCurves = true;
	setAnalysisParams();
	updateView();
}


/**
 * The user has changed the color of the current curve
 */
void QXDirect::onCurveColor()
{
	QColor Color = QColorDialog::getColor(m_LineStyle.m_Color);
	if(Color.isValid()) m_LineStyle.m_Color = Color;

	fillComboBoxes();
	updateCurveStyle();
}


/**
 * The user has changed the style of the current curve
 */
void QXDirect::onCurveStyle(int index)
{
	m_LineStyle.m_Style = index;
	fillComboBoxes();
	updateCurveStyle();
}


/**
 * The user has changed the width of the current curve
 */
void QXDirect::onCurveWidth(int index)
{
	m_LineStyle.m_Width = index+1;
	fillComboBoxes();
	updateCurveStyle();
}


void QXDirect::onCurvePoints(int index)
{
	m_LineStyle.m_PointStyle = index;
	fillComboBoxes();
	updateCurveStyle();
}


/**
 * The user has requested to define a new polar
 */
void QXDirect::onDefinePolar()
{
	MainFrame* pMainFrame = (MainFrame*)s_pMainFrame;
	if(!Foil::curFoil()) return;

	FoilPolarDlg fpDlg(pMainFrame);

	fpDlg.InitDialog();

	int res = fpDlg.exec();
	if (res == QDialog::Accepted)
	{
		Polar::setCurPolar(new Polar());
		Polar::curPolar()->m_FoilName = Foil::curFoil()->foilName();
		Polar::curPolar()->m_PlrName = fpDlg.m_PlrName;
		Polar::curPolar()->lineStyle().m_bIsVisible = true;
		Polar::curPolar()->copySpecification(&s_refPolar);

		Polar::curPolar()->setPolarType(fpDlg.m_PolarType);

		Polar::addPolar(Polar::curPolar());
		setPolar(Polar::curPolar());

		pMainFrame->updatePolarListBox();
		setBufferFoil();
		updateView();
		emit projectModified();
	}
	setControls();
}


/**
 * The user has requested the deletion of the current Foil.
 * Deletes the Foil, and selects the next one in the array, if any.
 */
void QXDirect::onDeleteCurFoil()
{
	MainFrame* pMainFrame = (MainFrame*)s_pMainFrame;

	QString strong;
	strong = tr("Are you sure you want to delete")  +"\n"+ Foil::curFoil()->foilName() +"\n";
	strong+= tr("and all associated OpPoints and Polars ?");

	int resp = QMessageBox::question(pMainFrame, tr("Question"), strong,  QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
	if(resp != QMessageBox::Yes) return;


	Foil*pNextFoil = Foil::deleteFoil(Foil::curFoil());
	Foil::setCurFoil(pNextFoil);
	OpPoint::setCurOpp(NULL);
	Polar::setCurPolar(NULL);

	pMainFrame->updateFoilListBox();

	m_bResetCurves = true;

	emit projectModified();

	setControls();
	updateView();
}



/**
 * The user has requested the deletion of the current OpPoint.
 */
void QXDirect::onDelCurOpp()
{
	MainFrame* pMainFrame = (MainFrame*)s_pMainFrame;
	OpPoint* pOpPoint = OpPoint::curOpp();
	stopAnimate();

	if (!pOpPoint) return;
	QString strong,str;
	strong = tr("Are you sure you want to delete the Operating Point\n");
	if(Polar::curPolar()->polarType()!=XFLR5::FIXEDAOAPOLAR) str = QString("Alpha = %1").arg(pOpPoint->Alpha,0,'f',2);
	else                                        str = QString("Reynolds = %1").arg(pOpPoint->Reynolds,0,'f',0);
	strong += str;
	strong += "  ?";

	if (QMessageBox::Yes == QMessageBox::question(pMainFrame, tr("Question"), strong,
		QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel))
	{
		OpPoint::deleteOpp(OpPoint::curOpp());
		pMainFrame->updateOppListBox();
		setOpp();
		updateView();
	}
	setControls();
}


/**
 * The user has requested the deletion of the current Polar.
 */
void QXDirect::onDeleteCurPolar()
{
	if(!Polar::curPolar()) return;
	MainFrame* pMainFrame = (MainFrame*)s_pMainFrame;
	OpPoint *pOpPoint;
	int l;
	QString str;

	str = tr("Are you sure you want to delete the polar :\n  ") + Polar::curPolar()->polarName();
	str += tr("\n and all the associated OpPoints ?");

	if (QMessageBox::Yes == QMessageBox::question(pMainFrame, tr("Question"), str,
		QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel))
	{
		// start by removing all OpPoints
		for (l=m_poaOpp->size()-1; l>=0; l--)
		{
			pOpPoint = (OpPoint*)m_poaOpp->at(l);
			if (pOpPoint->m_strPlrName  == Polar::curPolar()->polarName() &&
				pOpPoint->m_strFoilName == Foil::curFoil()->foilName())
			{
				m_poaOpp->removeAt(l);
				delete pOpPoint;
			}
		}
		// then remove the CPolar and update views
		for (l=m_poaPolar->size()-1; l>=0; l--)
		{
			if(Polar::curPolar() == m_poaPolar->at(l))
			{
				m_poaPolar->removeAt(l);
				delete Polar::curPolar();
				break;
			}
		}
		OpPoint::setCurOpp(NULL);
		Polar::setCurPolar(NULL);
	}

	pMainFrame->updatePolarListBox();
	setPolar();

	emit projectModified();
	updateView();
}


/**
 * The user has requested the deletion of the OpPoints associated to the current Polar.
 */
void QXDirect::onDeletePolarOpps()
{
	if(!Foil::curFoil() || !Polar::curPolar()) return;

	MainFrame* pMainFrame = (MainFrame*)s_pMainFrame;
	OpPoint *pOpp;

	for(int i=m_poaOpp->size()-1; i>=0; i--)
	{
		pOpp = (OpPoint*)m_poaOpp->at(i);
		if(pOpp->m_strFoilName==Foil::curFoil()->foilName() && pOpp->m_strPlrName==Polar::curPolar()->polarName())
		{
			m_poaOpp->removeAt(i);
			delete pOpp;
		}
	}

	OpPoint::setCurOpp(NULL);
	emit projectModified();

	pMainFrame->updateOppListBox();
	m_bResetCurves = true;
	setCurveParams();
	setControls();
	updateView();
}


/**
 * The user has requested the deletion of the OpPoints associated to the current Foil.
 */
void QXDirect::onDeleteFoilOpps()
{
	if(!Foil::curFoil() || !Polar::curPolar()) return;

	MainFrame* pMainFrame = (MainFrame*)s_pMainFrame;
	OpPoint *pOpp;

	for(int i=m_poaOpp->size()-1; i>=0; i--)
	{
		pOpp = (OpPoint*)m_poaOpp->at(i);
		if(pOpp->m_strFoilName==Foil::curFoil()->foilName())
		{
			m_poaOpp->removeAt(i);
			delete pOpp;
		}
	}
	OpPoint::setCurOpp(NULL);

	pMainFrame->updateOppListBox();
	m_bResetCurves = true;
	setCurveParams();
	setControls();
	updateView();

	emit projectModified();
}


/**
 * The user has requested the deletion of the Polars associated to the current Foil.
 */
void QXDirect::onDeleteFoilPolars()
{
	if(!Foil::curFoil()) return;
	MainFrame * pMainFrame = (MainFrame*) s_pMainFrame;
	int l;
	OpPoint *pOpPoint;
	stopAnimate();

	QString strong;

	strong = tr("Are you sure you want to delete polars and OpPoints\n");
	strong +=tr("associated to ")+Foil::curFoil()->foilName()  + " ?";
	if (QMessageBox::Yes == QMessageBox::question(pMainFrame, tr("Question"), strong,
		QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel))

	{
		// start by removing all OpPoints
		for (l=m_poaOpp->size()-1; l>=0; l--)
		{
			pOpPoint = (OpPoint*)m_poaOpp->at(l);
			if (pOpPoint->m_strFoilName == Foil::curFoil()->foilName())
			{
				m_poaOpp->removeAt(l);
				delete pOpPoint;
			}
		}
		// then remove CPolar and update views
		Polar* pPolar;
		for (l=m_poaPolar->size()-1; l>=0; l--)
		{
			pPolar = (Polar*)m_poaPolar->at(l);
			if (pPolar->foilName() == Foil::curFoil()->foilName())
			{
				m_poaPolar->removeAt(l);
				delete pPolar;
			}
		}
		OpPoint::setCurOpp(NULL);

	}
	Polar::setCurPolar(NULL);
	setPolar();

	m_bResetCurves = true;

	pMainFrame->updatePolarListBox();

	emit projectModified();

	setControls();
	updateView();
}


/**
 * The user has requested a local refinement of the panels of the current foil
 */
void QXDirect::onCadd()
{
	stopAnimate();
	MainFrame* pMainFrame = (MainFrame*)s_pMainFrame;
	if(!Foil::curFoil())		return;

	void* ptr = OpPoint::curOpp();
	OpPoint::setCurOpp(NULL);
	m_bResetCurves = true;

	bool bState = m_bShowPanels;
	onOpPointView();

	CAddDlg caDlg(pMainFrame);
	caDlg.m_pBufferFoil = &m_BufferFoil;
	caDlg.m_pMemFoil    = Foil::curFoil();
	caDlg.InitDialog();


	m_bShowPanels = true;
	updateView();

	if(QDialog::Accepted == caDlg.exec())
	{
		Foil *pNewFoil = new Foil();
		pNewFoil->copyFoil(&m_BufferFoil);
		pNewFoil->m_FoilColor  = MainFrame::getColor(0);
		pNewFoil->m_FoilStyle = 1;
		pNewFoil->m_FoilWidth = 1;
		pNewFoil->m_bPoints = false;
		OpPoint::setCurOpp((OpPoint*)ptr);

		if(!addNewFoil(pNewFoil)) setBufferFoil();
		else                      setFoil(pNewFoil);

		pMainFrame->updateFoilListBox();
	}
	else
	{
		OpPoint::setCurOpp((OpPoint*)ptr);
		setBufferFoil();
		m_pXFoil->InitXFoilGeometry(Foil::curFoil());
	}

	m_bShowPanels = bState;

	updateView();
}


/**
 * The user has requested that the foil be derotated
 */
void QXDirect::onDerotateFoil()
{
	if(!Foil::curFoil()) return;
	QString str;
	MainFrame* pMainFrame = (MainFrame*)s_pMainFrame;
	stopAnimate();

	Foil *pNewFoil = new Foil;
	pNewFoil->copyFoil(Foil::curFoil());
	pNewFoil->m_FoilStyle = 0;
	pNewFoil->m_FoilWidth = 1;
	pNewFoil->m_bPoints = false;
	pNewFoil->m_FoilColor  = MainFrame::getColor(0);

	double angle = pNewFoil->derotate();
	str = QString(tr("The foil has been de-rotated by %1 degrees")).arg(angle,6,'f',3);
	pMainFrame->statusBar()->showMessage(str);


	if(!addNewFoil(pNewFoil)) setBufferFoil();
	else                      setFoil(pNewFoil);

	pMainFrame->updateFoilListBox();

	emit projectModified();
	updateView();
}


/**
 * The user has requested to modify the parameters of the active polar
 */
void QXDirect::onEditCurPolar()
{
	if (!Polar::curPolar()) return;

	Polar *pMemPolar = new Polar;
	pMemPolar->copyPolar(Polar::curPolar());

	EditPlrDlg epDlg((MainFrame*)s_pMainFrame);
	epDlg.move(EditPlrDlg::s_Position);
	epDlg.resize(EditPlrDlg::s_WindowSize);
	if(EditPlrDlg::s_bWindowMaximized) epDlg.setWindowState(Qt::WindowMaximized);

	epDlg.initDialog(this, Polar::curPolar(), NULL, NULL);

	LineStyle style = Polar::curPolar()->lineStyle();
	Polar::curPolar()->lineStyle().m_PointStyle = 1;

	m_bResetCurves = true;
	updateView();

	if(epDlg.exec() == QDialog::Accepted)
	{
		emit projectModified();
	}
	else
	{
		Polar::curPolar()->copyPolar(pMemPolar);
	}
	Polar::curPolar()->lineStyle() = style;
	m_bResetCurves = true;
	updateView();

	delete pMemPolar;
}


/**
 * The user has requested the export of the current results stored in the XFoil object to a text file
 */
void QXDirect::onExportCurXFoilResults()
{
	if(!m_pXFoil->lvconv) return;
	if(!Foil::curFoil())		  return;

	QString FileName,  OutString, strong;

	double x[IVX][3],Hk[IVX][3],UeVinf[IVX][3], Cf[IVX][3], Cd[IVX][3], AA0[IVX][3];
	double RTheta[IVX][3], DStar[IVX][3], Theta[IVX][3];
	double uei;
	double que = 0.5*m_pXFoil->qinf*m_pXFoil->qinf;
	double qrf = m_pXFoil->qinf;
	int nside1, nside2, ibl;
	int type = 1;

	FileName = Foil::curFoil()->foilName();
	FileName.replace("/", " ");

	FileName = QFileDialog::getSaveFileName(this, tr("Export Current XFoil Results"),
											Settings::s_LastDirName,
											tr("Text File (*.txt);;Comma Separated Values (*.csv)"));

	if(!FileName.length()) return;
	int pos = FileName.lastIndexOf("/");
	if(pos>0) Settings::s_LastDirName = FileName.left(pos);

	pos  = FileName.lastIndexOf(".csv");
	if(pos>0) type = 2;

	QFile DestFile(FileName);

	if (!DestFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

	QTextStream out(&DestFile);

	out << VERSIONNAME;
	out << ("\n");
	strong = Foil::curFoil()->m_FoilName+ "\n";
	out << (strong);

	if(type==1)
		strong = QString("Alpha = %1,  Re = %2,  Ma= %3,  ACrit=%4\n\n")
						 .arg(m_pXFoil->alfa*180./PI, 5, 'f',1)
						 .arg(m_pXFoil->reinf1, 8, 'f',0)
						 .arg(m_pXFoil->minf1, 6, 'f',4)
						 .arg(m_pXFoil->acrit, 4, 'f',1);
	else
		strong = QString("Alpha =, %1,Re =, %3,Ma=, %3,ACrit =,%4\n\n")
						 .arg(m_pXFoil->alfa*180./PI, 5, 'f',1)
						 .arg(m_pXFoil->reinf1, 8, 'f',0)
						 .arg(m_pXFoil->minf1, 6, 'f',4)
						 .arg(m_pXFoil->acrit, 4, 'f',1);	out << (strong);

	m_pXFoil->CreateXBL(x, nside1, nside2);
	//write top first
	m_pXFoil->FillHk(Hk, nside1, nside2);
	for (ibl=2; ibl<= nside1;ibl++)
	{
		uei = m_pXFoil->uedg[ibl][1];
		UeVinf[ibl][1] = uei * (1.0-m_pXFoil->tklam)
						/ (1.0-m_pXFoil->tklam*(uei/m_pXFoil->qinf)*(uei/m_pXFoil->qinf));
	}
	for (ibl=2; ibl<= nside2;ibl++)
	{
		uei = m_pXFoil->uedg[ibl][2];
		UeVinf[ibl][2] = uei * (1.0-m_pXFoil->tklam)
						/ (1.0-m_pXFoil->tklam*(uei/m_pXFoil->qinf)*(uei/m_pXFoil->qinf));
	}
	//---- fill compressible ue arrays
	for (ibl=2; ibl<= nside1;ibl++)	Cf[ibl][1] = m_pXFoil->tau[ibl][1] / que;
	for (ibl=2; ibl<= nside2;ibl++)	Cf[ibl][2] = m_pXFoil->tau[ibl][2] / que;

	//---- fill compressible ue arrays
	for (ibl=2; ibl<= nside1;ibl++)	Cd[ibl][1] = m_pXFoil->dis[ibl][1] / qrf/ qrf/ qrf;
	for (ibl=2; ibl<= nside2;ibl++)	Cd[ibl][2] = m_pXFoil->dis[ibl][2] / qrf/ qrf/ qrf;
	//NPlot
	for (ibl=2; ibl< nside1;ibl++)	AA0[ibl][1] = m_pXFoil->ctau[ibl][1];
	for (ibl=2; ibl< nside2;ibl++)	AA0[ibl][2] = m_pXFoil->ctau[ibl][2];

	m_pXFoil->FillRTheta(RTheta, nside1, nside2);
	for (ibl=2; ibl<= nside1; ibl++)
	{
		DStar[ibl][1] = m_pXFoil->dstr[ibl][1];
		Theta[ibl][1] = m_pXFoil->thet[ibl][1];
	}
	for (ibl=2; ibl<= nside2; ibl++)
	{
		DStar[ibl][2] = m_pXFoil->dstr[ibl][2];
		Theta[ibl][2] = m_pXFoil->thet[ibl][2];
	}

	out << tr("\nTop Side\n");
	if(type==1) OutString = QString(tr("    x         Hk     Ue/Vinf      Cf        Cd     A/A0       D*       Theta      CTq\n"));
	else        OutString = QString(tr("x,Hk,Ue/Vinf,Cf,Cd,A/A0,D*,Theta,CTq\n"));
	out << (OutString);
	for (ibl=2; ibl<nside1; ibl++)
	{
		if(type==1)
			OutString = QString("%1  %2  %3  %4 %5 %6  %7  %8  %9\n")
							.arg(x[ibl][1])
							.arg(Hk[ibl][1],8,'f',5)
							.arg(UeVinf[ibl][1],8,'f',5)
							.arg(Cf[ibl][1],8,'f',5)
							.arg(Cd[ibl][1],8,'f',5)
							.arg(AA0[ibl][1],8,'f',5)
							.arg(DStar[ibl][1],8,'f',5)
							.arg(Theta[ibl][1],8,'f',5)
							.arg(m_pXFoil->ctq[ibl][1],8,'f',5);
		else
			OutString = QString("%1, %2, %3, %4, %5, %6, %7, %8, %9\n")
							.arg(x[ibl][1])
							.arg(Hk[ibl][1],8,'f',5)
							.arg(UeVinf[ibl][1],8,'f',5)
							.arg(Cf[ibl][1],8,'f',5)
							.arg(Cd[ibl][1],8,'f',5)
							.arg(AA0[ibl][1],8,'f',5)
							.arg(DStar[ibl][1],8,'f',5)
							.arg(Theta[ibl][1],8,'f',5)
							.arg(m_pXFoil->ctq[ibl][1],8,'f',5);
		out << (OutString);
	}
	out << tr("\n\nBottom Side\n");
	if(type==1) OutString = QString(tr("    x         Hk     Ue/Vinf      Cf        Cd     A/A0       D*       Theta      CTq\n"));
	else        OutString = QString(tr("x,Hk,Ue/Vinf,Cf,Cd,A/A0,D*,Theta,CTq\n"));
	out << (OutString);
	for (ibl=2; ibl<nside2; ibl++)
	{
		if(type==1)
			OutString = QString("%1  %2  %3  %4 %5 %6  %7  %8  %9\n")
							.arg(x[ibl][2])
							.arg(Hk[ibl][2],8,'f',5)
							.arg(UeVinf[ibl][2],8,'f',5)
							.arg(Cf[ibl][2],8,'f',5)
							.arg(Cd[ibl][2],8,'f',5)
							.arg(AA0[ibl][2],8,'f',5)
							.arg(DStar[ibl][2],8,'f',5)
							.arg(Theta[ibl][2],8,'f',5)
							.arg(m_pXFoil->ctq[ibl][2],8,'f',5);
		else
			OutString = QString("%1, %2, %3, %4, %5, %6, %7, %8, %9\n")
							.arg(x[ibl][2])
							.arg(Hk[ibl][2],8,'f',5)
							.arg(UeVinf[ibl][2],8,'f',5)
							.arg(Cf[ibl][2],8,'f',5)
							.arg(Cd[ibl][2],8,'f',5)
							.arg(AA0[ibl][2],8,'f',5)
							.arg(DStar[ibl][2],8,'f',5)
							.arg(Theta[ibl][2],8,'f',5)
							.arg(m_pXFoil->ctq[ibl][2],8,'f',5);
		out << (OutString);
	}

	DestFile.close();
}


/**
 * The user has requested the export of all polars to text files
 */
void QXDirect::onExportAllPolars()
{
	QString FileName, DirName;
	QFile XFile;
	QTextStream out(&XFile);

	//select the directory for output
	DirName = QFileDialog::getExistingDirectory(this,  tr("Export Directory"), Settings::s_LastDirName);

	Polar *pPolar;
	for(int l=0; l<m_poaPolar->size(); l++)
	{
		pPolar = (Polar*)m_poaPolar->at(l);
		FileName = DirName + "/" + pPolar->foilName() + "_" + pPolar->m_PlrName;
		if(Settings::s_ExportFileType==XFLR5::TXT) FileName += ".txt";
		else                                       FileName += ".csv";

		XFile.setFileName(FileName);
		if (XFile.open(QIODevice::WriteOnly | QIODevice::Text))
		{
			pPolar->exportPolar(out, Settings::s_ExportFileType);
			XFile.close();
		}
	}
}


/**
 * The user has requested the export of the current foil to a text file
 */
void QXDirect::onExportCurFoil()
{
	if(!Foil::curFoil())	return;

	QString FileName;

	FileName = Foil::curFoil()->foilName();
	FileName.replace("/", " ");

	FileName = QFileDialog::getSaveFileName(this, tr("Export Foil"),
											Settings::s_LastDirName+"/"+FileName+".dat",
											tr("Foil File (*.dat)"));

	if(!FileName.length()) return;
	int pos = FileName.lastIndexOf("/");
	if(pos>0) Settings::s_LastDirName = FileName.left(pos);

	QFile XFile(FileName);

	if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

	QTextStream out(&XFile);

	Foil::curFoil()->exportFoil(out);
	XFile.close();
}


/**
 * The user has requested the export of the current OpPoint to a text file
 */
void QXDirect::onExportCurOpp()
{
	if(!Foil::curFoil() || !Polar::curPolar() || !OpPoint::curOpp())	return;

	QString FileName;

	QString filter;
	if(Settings::s_ExportFileType==XFLR5::TXT) filter = "Text File (*.txt)";
	else                                       filter = "Comma Separated Values (*.csv)";

	FileName = QFileDialog::getSaveFileName(this, tr("Export OpPoint"),
											Settings::s_LastDirName ,
											tr("Text File (*.txt);;Comma Separated Values (*.csv)"),
											&filter);
	if(!FileName.length()) return;

	int pos = FileName.lastIndexOf("/");
	if(pos>0) Settings::s_LastDirName = FileName.left(pos);
	pos = FileName.lastIndexOf(".csv");
	if (pos>0) Settings::s_ExportFileType = XFLR5::CSV;
	else       Settings::s_ExportFileType = XFLR5::TXT;

	QFile XFile(FileName);

	if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

	QTextStream out(&XFile);

	OpPoint::curOpp()->exportOpp(out, VERSIONNAME, Settings::s_ExportFileType);
	XFile.close();
}


/**
 * The user has requested the export of the OpPoints associated to the current Polar to a text file
 */
void QXDirect::onExportPolarOpps()
{
	MainFrame * pMainFrame = (MainFrame*)s_pMainFrame;
	if(!m_poaPolar->size())
	{
		QMessageBox::warning(pMainFrame, tr("Warning"), "No Operating Points to export to file");
		return;
	}

	int i,j;
	QString FileName;

	QString filter;
	if(Settings::s_ExportFileType==XFLR5::TXT) filter = "Text File (*.txt)";
	else                                       filter = "Comma Separated Values (*.csv)";

	FileName = QFileDialog::getSaveFileName(this, tr("Export OpPoint"),
											Settings::s_LastDirName ,
											tr("Text File (*.txt);;Comma Separated Values (*.csv)"),
											&filter);

	if(!FileName.length()) return;
	if(!FileName.length()) return;

	int pos = FileName.lastIndexOf("/");
	if(pos>0) Settings::s_LastDirName = FileName.left(pos);
	pos = FileName.lastIndexOf(".csv");
	if (pos>0) Settings::s_ExportFileType = XFLR5::CSV;
	else       Settings::s_ExportFileType = XFLR5::TXT;

	QFile XFile(FileName);

	if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

	QTextStream out(&XFile);


	QString Header, strong;
	out<<VERSIONNAME;
	out<<"\n\n";
	strong = Foil::curFoil()->foilName() + "\n";
	out << strong;

	OpPoint *pOpPoint;

	for (i=0; i<m_poaOpp->size(); i++)
	{
		pOpPoint = (OpPoint*)m_poaOpp->at(i);
		if(pOpPoint->m_strFoilName == Polar::curPolar()->foilName() && pOpPoint->m_strPlrName == Polar::curPolar()->polarName() )
		{
			if(Settings::s_ExportFileType==XFLR5::TXT)
				strong = QString("Reynolds = %1   Mach = %2  NCrit = %3\n")
									.arg(pOpPoint->Reynolds, 7, 'f', 0)
									.arg(pOpPoint->Mach, 4,'f',0)
									.arg(pOpPoint->ACrit, 3, 'f',1);
			else
				strong = QString("Reynolds =, %1,Mach =, %2,NCrit =, %3\n")
						.arg(pOpPoint->Reynolds, 7, 'f', 0)
						.arg(pOpPoint->Mach, 4,'f',0)
						.arg(pOpPoint->ACrit, 3, 'f',1);

			out<<strong;
			if(Settings::s_ExportFileType==1) Header = QString("  Alpha        Cd        Cl        Cm        XTr1      XTr2   TEHMom    Cpmn\n");
			else        Header = QString("Alpha,Cd,Cl,Cm,XTr1,XTr2,TEHMom,Cpmn\n");
			out<<Header;

			if(Settings::s_ExportFileType==XFLR5::TXT)
				strong = QString("%1   %2   %3   %4   %5   %6   %7  %8\n")
					.arg(pOpPoint->Alpha,7,'f',3)
					.arg(pOpPoint->Cd,9,'f',3)
					.arg(pOpPoint->Cl,7,'f',3)
					.arg(pOpPoint->Cm,7,'f',3)
					.arg(pOpPoint->Xtr1,7,'f',3)
					.arg(pOpPoint->Xtr2,7,'f',3)
					.arg(pOpPoint->m_TEHMom,7,'f',4)
					.arg(pOpPoint->Cpmn,7,'f',4);
			else
				strong = QString("%1,%2,%3,%4,%5,%6,%7,%8\n")
				.arg(pOpPoint->Alpha,7,'f',3)
				.arg(pOpPoint->Cd,9,'f',3)
				.arg(pOpPoint->Cl,7,'f',3)
				.arg(pOpPoint->Cm,7,'f',3)
				.arg(pOpPoint->Xtr1,7,'f',3)
				.arg(pOpPoint->Xtr2,7,'f',3)
				.arg(pOpPoint->m_TEHMom,7,'f',4)
				.arg(pOpPoint->Cpmn,7,'f',4);

			out<<strong;
			if(Settings::s_ExportFileType==XFLR5::TXT) out<< " Cpi          Cpv\n-----------------\n";
			else                                       out << "Cpi,Cpv\n";

			for (j=0; j<pOpPoint->n; j++)
			{
				if(pOpPoint->m_bViscResults)
				{
					if(Settings::s_ExportFileType==XFLR5::TXT) strong = QString("%1   %2\n").arg(pOpPoint->Cpi[j], 7,'f',4).arg(pOpPoint->Cpv[j], 7, 'f',4);
					else                                       strong = QString("%1,%2\n").arg(pOpPoint->Cpi[j], 7,'f',4).arg(pOpPoint->Cpv[j], 7, 'f',4);
				}
				else
				{
					strong=QString("%1\n").arg(pOpPoint->Cpi[j],7,'f',4);
				}

				out << strong;
			}
			out << "\n\n";
		}
	}
	XFile.close();


}


/**
 * The user has requested the export of the current Polar to a text file
 */
void QXDirect::onExportCurPolar()
{
	if(!Foil::curFoil() || !Polar::curPolar())	return;

	QString FileName, filter;

	if(Settings::s_ExportFileType==XFLR5::TXT) filter = "Text File (*.txt)";
	else                                       filter = "Comma Separated Values (*.csv)";

	FileName = Polar::curPolar()->polarName();
	FileName.replace("/", " ");
	FileName = QFileDialog::getSaveFileName(this, tr("Export Polar"),
											Settings::s_LastDirName + "/"+FileName,
											tr("Text File (*.txt);;Comma Separated Values (*.csv)"),
											&filter);
	if(!FileName.length()) return;

	int pos = FileName.lastIndexOf("/");
	if(pos>0) Settings::s_LastDirName = FileName.left(pos);
	pos = FileName.lastIndexOf(".csv");
	if (pos>0) Settings::s_ExportFileType = XFLR5::CSV;
	else       Settings::s_ExportFileType = XFLR5::TXT;

	QFile XFile(FileName);

	if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

	QTextStream out(&XFile);

	Polar::curPolar()->exportPolar(out, Settings::s_ExportFileType);
	XFile.close();
}


/**
 * The user has requested an edition of the current foil coordinates
 */
void QXDirect::onFoilCoordinates()
{
	if(!Foil::curFoil())	return;
	MainFrame* pMainFrame = (MainFrame*)s_pMainFrame;
	stopAnimate();

	bool bState = m_bShowPanels;//save current view setting

	void* ptr = OpPoint::curOpp();
	OpPoint::setCurOpp(NULL);
	m_bResetCurves = true;

	m_bPolarView    = false;
	updateView();

	bool bFlap       = m_BufferFoil.m_bTEFlap;
	double FlapAngle = m_BufferFoil.m_TEFlapAngle;
	double Xh        = m_BufferFoil.m_TEXHinge;
	double Yh        = m_BufferFoil.m_TEXHinge;

	m_BufferFoil.m_bTEFlap = false;
//	SetFoilFlap(&m_BufferFoil);

	FoilCoordDlg fcoDlg(pMainFrame);
	fcoDlg.m_pMemFoil    = Foil::curFoil();
	fcoDlg.m_pBufferFoil = &m_BufferFoil;
	fcoDlg.InitDialog();

	m_bShowPanels = true;
	updateView();

	int res = fcoDlg.exec();

	if(QDialog::Accepted == res)
	{
		m_BufferFoil.m_bTEFlap = bFlap;
		m_BufferFoil.m_TEFlapAngle = FlapAngle;
		m_BufferFoil.m_TEXHinge = Xh;
		m_BufferFoil.m_TEYHinge = Yh;

		Foil *pNewFoil = new Foil();
		pNewFoil->copyFoil(&m_BufferFoil);
		pNewFoil->m_FoilColor  = MainFrame::getColor(0);
		pNewFoil->m_FoilStyle = 0;
		pNewFoil->m_FoilWidth = 1;
		pNewFoil->m_bPoints = false;
		OpPoint::setCurOpp((OpPoint*)ptr);

		if(!addNewFoil(pNewFoil)) setBufferFoil();
		else                      setFoil(pNewFoil);

		pMainFrame->updateFoilListBox();
		emit projectModified();
	}
	else
	{
		//reset everything
		OpPoint::setCurOpp((OpPoint*)ptr);
		m_BufferFoil.m_bTEFlap = bFlap;
		m_BufferFoil.m_TEFlapAngle = FlapAngle;
		m_BufferFoil.m_TEXHinge = Xh;
		m_BufferFoil.m_TEYHinge = Yh;
//		SetFoilFlap(&m_BufferFoil);
		setBufferFoil();
		m_pXFoil->InitXFoilGeometry(Foil::curFoil());
	}

	m_BufferFoil.m_iHighLight = -1;
	m_bShowPanels = bState;//restore as it was
	updateView();
}


/**
 * The user has requested to perform an edition of the current foil's thickness and camber properties.
 */
void QXDirect::onFoilGeom()
{
	MainFrame* pMainFrame = (MainFrame*)s_pMainFrame;

	if(!Foil::curFoil())	return;

	stopAnimate();
	m_bPolarView   = false;

	void* ptr = OpPoint::curOpp();
	OpPoint::setCurOpp(NULL);
	m_bResetCurves = true;
	updateView();

	FoilGeomDlg fgeDlg(pMainFrame);
	fgeDlg.m_pMemFoil = Foil::curFoil();
	fgeDlg.m_pBufferFoil = &m_BufferFoil;
	fgeDlg.InitDialog();

	if(fgeDlg.exec() == QDialog::Accepted)
	{
		Foil *pNewFoil = new Foil();
		pNewFoil->copyFoil(&m_BufferFoil);
		pNewFoil->m_FoilColor  = MainFrame::getColor(0);
		pNewFoil->m_FoilStyle = 0;
		pNewFoil->m_FoilWidth = 1;
		pNewFoil->m_bPoints = false;
		OpPoint::setCurOpp((OpPoint*)ptr);

		if(!addNewFoil(pNewFoil)) setBufferFoil();
		else                      setFoil(pNewFoil);

		pMainFrame->updateFoilListBox();
		emit projectModified();
	}
	else
	{
		OpPoint::setCurOpp((OpPoint*)ptr);
		setBufferFoil();
		m_pXFoil->InitXFoilGeometry(Foil::curFoil());
	}

	updateView();
}


/**
 * The user has requested the highlighting of the current OpPoint on the polar curves
 */
void QXDirect::onHighlightOpp()
{
	if(!m_bPolarView) return;
	MainFrame *pMainFrame = (MainFrame*)s_pMainFrame;
	m_bHighlightOpp = !m_bHighlightOpp;

	pMainFrame->highlightOppAct->setChecked(m_bHighlightOpp);
	for(int ig=0; ig<m_PlrGraph.count(); ig++)
		m_PlrGraph[ig]->m_bHighlightPoint = m_bHighlightOpp;

	m_bResetCurves = true;
	updateView();
}


/**
 * The user has requested to hide all OpPoints
 */
void QXDirect::onHideAllOpps()
{
	OpPoint *pOpp;
	for (int i=0; i<m_poaOpp->size(); i++)
	{
		pOpp = (OpPoint*)m_poaOpp->at(i);
		pOpp->lineStyle().m_bIsVisible = false;
	}
	emit projectModified();
	m_bResetCurves = true;
	setAnalysisParams();
	updateView();
}


/**
 * The user has requested to hide all polar curves
 */
void QXDirect::onHideAllPolars()
{
	Polar *pPolar;
	for (int i=0; i<m_poaPolar->size(); i++)
	{
		pPolar = (Polar*)m_poaPolar->at(i);
		pPolar->lineStyle().m_bIsVisible = false;
	}
	emit projectModified();
	m_bResetCurves = true;
	setCurveParams();
	updateView();
}


/**
 * The user has requested to hide all polar curves associated to the current Foil
 */
void QXDirect::onHideFoilPolars()
{
	if(!Foil::curFoil()) return;
	Polar *pPolar;
	for (int i=0; i<m_poaPolar->size(); i++)
	{
		pPolar = (Polar*)m_poaPolar->at(i);
		if(pPolar->foilName() == Foil::curFoil()->foilName())
		{
			pPolar->lineStyle().m_bIsVisible = false;
		}
	}
	emit projectModified();
	m_bResetCurves = true;
	setCurveParams();
	updateView();
}


/**
 * The user has requested to hide all OpPoint curves associated to the current Foil
 */
void QXDirect::onHideFoilOpps()
{
	if(!Foil::curFoil() || !Polar::curPolar()) return;

	OpPoint *pOpp;

	for(int i=0; i<m_poaOpp->size(); i++)
	{
		pOpp = (OpPoint*)m_poaOpp->at(i);
		if(pOpp->m_strFoilName==Foil::curFoil()->foilName())
			pOpp->lineStyle().m_bIsVisible = false;
	}
	emit projectModified();
	m_bResetCurves = true;
	setCurveParams();
	updateView();
}


/**
 * The user has requested to hide all OpPoint curves associated to the current Polar
 */
void QXDirect::onHidePolarOpps()
{
	if(!Foil::curFoil() || !Polar::curPolar()) return;

	OpPoint *pOpp;

	for(int i=0; i<m_poaOpp->size(); i++)
	{
		pOpp = (OpPoint*)m_poaOpp->at(i);
		if(pOpp->m_strFoilName==Foil::curFoil()->foilName() && pOpp->m_strPlrName==Polar::curPolar()->polarName())
			pOpp->lineStyle().m_bIsVisible = false;
	}
	emit projectModified();
	m_bResetCurves = true;
	setCurveParams();
	updateView();
}


/**
 * The user has requested to import a polar from a text file.
 * The Polar will be added to the array only if a Foil with the parent name exists.
 */
void QXDirect::onImportXFoilPolar()
{
	MainFrame *pMainFrame = (MainFrame*)s_pMainFrame;
	Polar *pPolar = new Polar;
	double Re, alpha, CL, CD, CDp, CM, Xt, Xb,Cpmn, HMom;
	QString FoilName, PathName, strong, str;

	PathName = QFileDialog::getOpenFileName(pMainFrame, tr("Open File"),
											Settings::s_LastDirName,
											tr("XFoil Polar Format (*.*)"));
	if(!PathName.length())		return ;
	int pos = PathName.lastIndexOf("/");
	if(pos>0) Settings::s_LastDirName = PathName.left(pos);

	QFile XFile(PathName);
	if (!XFile.open(QIODevice::ReadOnly))
	{
		QString strange = tr("Could not read the file\n")+PathName;
		QMessageBox::warning(pMainFrame, tr("Warning"), strange);
		return;
	}

	QTextStream in(&XFile);
	int Line;
	bool bOK, bOK2, bRead;
	Line = 0;

	bRead  = ReadAVLString(in, Line, strong);// XFoil or XFLR5 version
	bRead  = ReadAVLString(in, Line, strong);// Foil Name

	FoilName = strong.right(strong.length()-22);
	FoilName = FoilName.trimmed();

	if(!Foil::foil(FoilName))
	{
		str = tr("No Foil with the name ")+FoilName;
		str+= tr("\ncould be found. The polar(s) will not be stored");
		delete pPolar;
		QMessageBox::warning(pMainFrame, tr("Warning"), str);
		return;
	}
	pPolar->m_FoilName = FoilName;

	bRead  = ReadAVLString(in, Line, strong);// analysis type

	pPolar->m_ReType = strong.mid(0,2).toInt(&bOK);
	pPolar->m_MaType= strong.mid(2,2).toInt(&bOK2);
	if(!bOK || !bOK2)
	{
		str = QString("Error reading line %1: Unrecognized Mach and Reynolds type.\nThe polar(s) will not be stored").arg(Line);
		delete pPolar;
		QMessageBox::warning(pMainFrame, tr("Warning"), str);
		return;
	}
	if     (pPolar->m_ReType ==1 && pPolar->m_MaType ==1) pPolar->m_PolarType = XFLR5::FIXEDSPEEDPOLAR;
	else if(pPolar->m_ReType ==2 && pPolar->m_MaType ==2) pPolar->m_PolarType = XFLR5::FIXEDLIFTPOLAR;
	else if(pPolar->m_ReType ==3 && pPolar->m_MaType ==1) pPolar->m_PolarType = XFLR5::RUBBERCHORDPOLAR;
	else                                                  pPolar->m_PolarType = XFLR5::FIXEDSPEEDPOLAR;


	bRead  = ReadAVLString(in, Line, strong);
	if(strong.length() < 34)
	{
		str = QString("Error reading line %1. The polar(s) will not be stored").arg(Line);
		delete pPolar;
		QMessageBox::warning(pMainFrame, tr("Warning"), str);
		return;
	}

	pPolar->m_XTop = strong.mid(9,6).toDouble(&bOK);
	if(!bOK)
	{
		str = QString("Error reading Bottom Transition value at line %1. The polar(s) will not be stored").arg(Line);
		delete pPolar;
		QMessageBox::warning(pMainFrame, tr("Warning"), str);
		return;

	}

	pPolar->m_XTop = strong.mid(28,6).toDouble(&bOK);
	if(!bOK)
	{
		str = QString("Error reading Top Transition value at line %1. The polar(s) will not be stored").arg(Line);
		delete pPolar;
		QMessageBox::warning(pMainFrame, tr("Warning"), str);
		return;
	}

	// Mach     Re     NCrit
	bRead  = ReadAVLString(in, Line, strong);// blank line
	if(strong.length() < 50)
	{
		str = QString("Error reading line %1. The polar(s) will not be stored").arg(Line);
		delete pPolar;
		QMessageBox::warning(pMainFrame, tr("Warning"), str);
		return;
	}

	pPolar->m_Mach = strong.mid(8,6).toDouble(&bOK);
	if(!bOK)
	{
		str = QString("Error reading Mach Number at line %1. The polar(s) will not be stored").arg(Line);
		delete pPolar;
		QMessageBox::warning(pMainFrame, tr("Warning"), str);
		return;
	}

	Re = strong.mid(24,10).toDouble(&bOK);
	if(!bOK)
	{
		str = QString("Error reading Reynolds Number at line %1. The polar(s) will not be stored").arg(Line);
		delete pPolar;
		QMessageBox::warning(pMainFrame, tr("Warning"), str);
		return;
	}
	Re *=1000000.0;

	pPolar->m_ACrit = strong.mid(52,8).toDouble(&bOK);
	if(!bOK)
	{
		str = QString("Error reading NCrit at line %1. The polar(s) will not be stored").arg(Line);
		delete pPolar;
		QMessageBox::warning(pMainFrame, tr("Warning"), str);
		return;
	}
	pPolar->m_Reynolds = Re;

	bRead  = ReadAVLString(in, Line, strong);// column titles
	bRead  = ReadAVLString(in, Line, strong);// underscores

	while( bRead)
	{
		bRead  = ReadAVLString(in, Line, strong);// polar data
		if(bRead)
		{
			if(strong.length())
			{
//				textline = strong.toLatin1();
//				text = textline.constData();
//				res = sscanf(text, "%lf%lf%lf%lf%lf%lf%lf%lf%lf", &alpha, &CL, &CD, &CDp, &CM, &Xt, &Xb, &Cpmn, &HMom);

				//Do this the C++ way
				QStringList values = strong.split(" ", QString::SkipEmptyParts);

				if(values.length()>=7)
				{
					alpha  = values.at(0).toDouble();
					CL     = values.at(1).toDouble();
					CD     = values.at(2).toDouble();
					CDp    = values.at(3).toDouble();
					CM     = values.at(4).toDouble();
					Xt     = values.at(5).toDouble();
					Xb     = values.at(6).toDouble();

					if(values.length() >= 9)
					{
						Cpmn    = values.at(7).toDouble();
						HMom    = values.at(8).toDouble();
						pPolar->addPoint(alpha, CD, CDp, CL, CM, Xt, Xb, Cpmn, HMom,Re,0.0);
					}
					else
					{
						pPolar->addPoint(alpha, CD, CDp, CL, CM, Xt, Xb, 0.0, 0.0,Re,0.0);

					}
				}
			}
		}
	}

	Re = pPolar->m_Reynolds/1000000.0;
	pPolar->m_PlrName = QString("T%1_Re%2_M%3")
						.arg(pPolar->m_PolarType)
						.arg(Re,0,'f',2)
						.arg(pPolar->m_Mach,0,'f',2);
	str = QString("_N%1").arg(pPolar->m_ACrit,0,'f',1);
	pPolar->m_PlrName += str + "_Imported";

	pPolar->lineStyle().m_Color = MainFrame::getColor(1);

	Polar::addPolar(pPolar);

	OpPoint::setCurOpp(NULL);
	setPolar(pPolar);
	pMainFrame->updatePolarListBox();
	updateView();
	emit projectModified();
}


/**
 * The user has requested to import a polar from a text file in JavaFoil format
 * The Polar will be added to the array only if a Foil with the parent name exists.
 *  @todo Note: this option has not been tested in years... the JavaFoil format may have changed since
 */
void QXDirect::onImportJavaFoilPolar()
{
	MainFrame *pMainFrame = (MainFrame*)s_pMainFrame;
	QString FoilName;
	QString strong, str;

	QString PathName;
	bool bOK;
	QByteArray textline;
	const char *text;

	PathName = QFileDialog::getOpenFileName(pMainFrame, tr("Open File"),
											Settings::s_LastDirName,
											tr("JavaFoil Polar Format (*.*)"));
	if(!PathName.length())		return ;
	int pos = PathName.lastIndexOf("/");
	if(pos>0) Settings::s_LastDirName = PathName.left(pos);

	QFile XFile(PathName);
	if (!XFile.open(QIODevice::ReadOnly))
	{
		QString strange = tr("Could not read the file\n")+PathName;
		QMessageBox::warning(pMainFrame, tr("Warning"), strange);
		return;
	}

	QTextStream in(&XFile);

	bool bIsReading = true;
	int res, Line;
	int NPolars = 0;
	double Re;

	double alpha, CL, CD, CM, Xt,  Xb;

	Line = 0;
	if(!ReadAVLString(in, Line, FoilName)) return;


	FoilName = FoilName.trimmed();

	if(!Foil::foil(FoilName))
	{
		str = tr("No Foil with the name ")+FoilName;
		str+= tr("\ncould be found. The polar(s) will not be stored");
		QMessageBox::warning(pMainFrame, tr("Warning"), str);
		return;
	}
	if(!ReadAVLString(in, Line, strong)) return; //blank line

	while(bIsReading)
	{
		if(!ReadAVLString(in, Line, strong)) break; //Re number

		strong = strong.right(strong.length()-4);
		Re = strong.toDouble(&bOK);
		if(!bOK)
		{
			bIsReading = false;
		}
		else
		{
			Polar *pPolar = new Polar();
			pPolar->m_FoilName = FoilName;
			pPolar->m_Reynolds = Re;;
			pPolar->m_PlrName = QString("T%1_Re2_M3_JavaFoil")
								.arg(pPolar->m_PolarType)
								.arg(pPolar->m_Reynolds/1000000.0,0,'f',2)
								.arg(pPolar->m_Mach,0,'f',2);

			pPolar->lineStyle().m_Color = MainFrame::getColor(1);
			Polar::addPolar(pPolar);
			Polar::setCurPolar(pPolar);
			NPolars++;

			if(!ReadAVLString(in, Line, strong)) break;//?	Cl	Cd	Cm 0.25	TU	TL	SU	SL	L/D
			if(!ReadAVLString(in, Line, strong)) break;//[?]	[-]	[-]	[-]	[-]	[-]	[-]	[-]	[-]

			res = 6;
			while(res==6)
			{
				bIsReading  = ReadAVLString(in, Line, strong);//values
				if(!bIsReading) break;
				strong = strong.trimmed();
				if(strong.length())
				{
					strong.replace(',', '.');

					textline = strong.toLatin1();
					text = textline.constData();

					res = sscanf(text, "%lf%lf%lf%lf%lf%lf",&alpha, &CL, &CD, &CM, &Xt, &Xb);
					if (res == 6) 	pPolar->addPoint(alpha, CD, 0.0, CL, CM, Xt, Xb, 0.0, 0.0, Re,0.0);
				}
				else
				{
					res = 0;
				}
			}
		}
		OpPoint::setCurOpp(NULL);
		setPolar();
		pMainFrame->updatePolarListBox();
		updateView();
		emit projectModified();
	}
}



/**
 * The user has requested the launch of the interface to create a foil from the interpolation of two existing Foil objects.
 */
void QXDirect::onInterpolateFoils()
{
	MainFrame* pMainFrame = (MainFrame*)s_pMainFrame;
	if(m_poaFoil->size()<2)
	{
		QMessageBox::warning(pMainFrame, tr("Warning"), tr("At least two foils are required"));
		return;
	}

	stopAnimate();

	onOpPointView();

	InterpolateFoilsDlg ifDlg(pMainFrame);
	ifDlg.m_poaFoil     = m_poaFoil;
	ifDlg.m_pBufferFoil = &m_BufferFoil;// work on the buffer foil
	ifDlg.InitDialog();

	if(ifDlg.exec() == QDialog::Accepted)
	{
		Foil *pNewFoil = new Foil();
		pNewFoil->copyFoil(&m_BufferFoil);
		pNewFoil->m_FoilColor  = MainFrame::getColor(0);
		pNewFoil->m_FoilStyle = 0;
		pNewFoil->m_FoilWidth = 1;
		pNewFoil->m_bPoints = false;
		pNewFoil->m_FoilName = ifDlg.m_NewFoilName;

		if(!addNewFoil(pNewFoil)) setBufferFoil();
		else                      setFoil(pNewFoil);

		pMainFrame->updateFoilListBox();
		emit projectModified();
	}
	else
	{
		setBufferFoil();// restore buffer foil.. from current foil
		m_pXFoil->InitXFoilGeometry(Foil::curFoil());
	}

	updateView();
}



/**
 * The user has requested the launch of the interface used to create a NACA type foil.
 */
void QXDirect::onNacaFoils()
{
	MainFrame *pMainFrame = (MainFrame*)s_pMainFrame;
	stopAnimate();
	m_bPolarView   = false;

	void* ptr0 = Foil::curFoil();
	void* ptr  = OpPoint::curOpp();
	Foil::setCurFoil(NULL);
	OpPoint::setCurOpp(NULL);

	m_bResetCurves = true;
	updateView();

	NacaFoilDlg nacaDlg(pMainFrame);
	nacaDlg.m_pBufferFoil = &m_BufferFoil;

	if (nacaDlg.exec() == QDialog::Accepted)
	{
		QString str;
		if(nacaDlg.s_Digits>0 && log10((double)nacaDlg.s_Digits)<4)
			str = QString("%1").arg(nacaDlg.s_Digits,4,10,QChar('0'));
		else
			str = QString("%1").arg(nacaDlg.s_Digits);
		str = "NACA "+ str;

		Foil *pNewFoil = new Foil;
		pNewFoil->copyFoil(&m_BufferFoil);
		pNewFoil->m_FoilColor  = MainFrame::getColor(0);
		pNewFoil->m_FoilStyle = 0;
		pNewFoil->m_FoilWidth = 1;
		pNewFoil->m_bPoints = false;
		pNewFoil->m_FoilName = str;

		OpPoint::setCurOpp((OpPoint*)ptr);

		if(!addNewFoil(pNewFoil))	setBufferFoil();

		setFoil(pNewFoil);
		pMainFrame->updateFoilListBox();
		emit projectModified();
	}
	else
	{
		Foil::setCurFoil((Foil*)ptr0);
		OpPoint::setCurOpp((OpPoint*)ptr);
		setBufferFoil();
		m_pXFoil->InitXFoilGeometry(Foil::curFoil());
	}
	setControls();
	updateView();
}


/**
 * The user has requested that the length of the current foil be normalized to 1.
 */
void QXDirect::onNormalizeFoil()
{
	MainFrame* pMainFrame = (MainFrame*)s_pMainFrame;
	if(!Foil::curFoil()) return;
	QString str;
	stopAnimate();


	double length = Foil::curFoil()->normalizeGeometry();
	m_pXFoil->InitXFoilGeometry(Foil::curFoil());
	setBufferFoil();
	str = QString(tr("The foil has been normalized from %1  to 1.000")).arg(length,7,'f',3);
	emit projectModified();
	pMainFrame->statusBar()->showMessage(str);

	updateView();
}


/**
 * The user has requested a  plot of the A/A0 variable using current XFoil results.
 */
void QXDirect::onNPlot()
{
	if(!m_pXFoil->lvconv) return;
	int i;
	int nside1, nside2, ibl;

	m_CpGraph.setYVariable(2);
	m_XFoilVar=6;
	m_CpGraph.deleteCurves();
	m_CpGraph.resetLimits();
	m_CpGraph.setAuto(true);
	m_CpGraph.setInverted(false);
	m_CpGraph.setYTitle("A/A0");
	Curve * pTopCurve = m_CpGraph.addCurve();
	Curve * pBotCurve = m_CpGraph.addCurve();
	pTopCurve->setCurveName(tr("Top"));
	pBotCurve->setCurveName(tr("Bot"));

	double x[IVX][3],y[IVX][3];

	m_pXFoil->CreateXBL(x, nside1, nside2);

	for (ibl=2; ibl< nside1;ibl++)
	{
		y[ibl][1] = m_pXFoil->ctau[ibl][1];
	}
	for ( ibl=2; ibl< nside2;ibl++)
	{
		y[ibl][2] = m_pXFoil->ctau[ibl][2];
	}

	for (i=2; i<=m_pXFoil->itran[1]-2; i++)
	{
		pTopCurve->appendPoint(x[i][1], y[i][1]);
	}
	for (i=2; i<=m_pXFoil->itran[2]-2; i++)
	{
		pBotCurve->appendPoint(x[i][2], y[i][2]);
	}
	m_CpGraph.SetXScale();
	setFoilScale();
	setControls();
	updateView();
}


/**
 * The user has requested to switch to the OpPoint view
 */
void QXDirect::onOpPointView()
{
	if(!m_bPolarView) return;

	m_bPolarView = false;
	m_bResetCurves = true;
	setFoilScale();
	setCurveParams();
	setAnalysisParams();

	setGraphTiles();
	setControls();

	updateView();
}


/**
 * The user has requested to switch to the Polar view
 */
void QXDirect::onPolarView()
{
	if(m_bPolarView) return;
	m_bPolarView = true;
	m_bResetCurves = true;
	setCurveParams();
	setAnalysisParams();

	setGraphTiles();
	setControls();

	updateView();
}

/**
 * The user has requested the launch of the interface used to filter the type of polars to be displayed.
 */
void QXDirect::onPolarFilter()
{
	PolarFilterDlg pfDlg((MainFrame*)s_pMainFrame);
	pfDlg.m_bMiarex = false;
	pfDlg.m_bType1 = m_bType1;
	pfDlg.m_bType2 = m_bType2;
	pfDlg.m_bType3 = m_bType3;
	pfDlg.m_bType4 = m_bType4;
	pfDlg.InitDialog();

	if(pfDlg.exec()==QDialog::Accepted)
	{
		m_bType1 = pfDlg.m_bType1;
		m_bType2 = pfDlg.m_bType2;
		m_bType3 = pfDlg.m_bType3;
		m_bType4 = pfDlg.m_bType4;
		if(m_bPolarView)
		{
			m_bResetCurves = true;
			updateView();
		}
	}
}


/**
 * The user has requested the launch of the interface to refine globally the Foil
*/
void QXDirect::onRefinePanelsGlobally()
{
	MainFrame* pMainFrame = (MainFrame*)s_pMainFrame;
	if(!Foil::curFoil())	return;
	stopAnimate();

	bool bState = m_bShowPanels;//save current view setting

	void* ptr = OpPoint::curOpp();
	OpPoint::setCurOpp(NULL);
	m_bResetCurves = true;

	onOpPointView();

	TwoDPanelDlg tdpDlg(pMainFrame);
	tdpDlg.m_pBufferFoil = &m_BufferFoil;
	tdpDlg.m_pMemFoil    = Foil::curFoil();

	m_bShowPanels = true;
	updateView();

	tdpDlg.InitDialog();

	if(QDialog::Accepted == tdpDlg.exec())
	{
		Foil *pNewFoil = new Foil();
		pNewFoil->copyFoil(&m_BufferFoil);
		pNewFoil->m_FoilColor  = MainFrame::getColor(0);
		pNewFoil->m_FoilStyle = 0;
		pNewFoil->m_FoilWidth = 1;
		pNewFoil->m_bPoints = true;
		OpPoint::setCurOpp((OpPoint*)ptr);
		if(!addNewFoil(pNewFoil))	setBufferFoil();
		setFoil(pNewFoil);
		pMainFrame->updateFoilListBox();
		emit projectModified();
	}
	else
	{
		//reset everything
		OpPoint::setCurOpp((OpPoint*)ptr);
		setBufferFoil();
		m_pXFoil->InitXFoilGeometry(Foil::curFoil());
	}

	m_bShowPanels = bState;//restore as it was
	updateView();
}


/**
 * The user has requested the display of the velocity in the Cp graph.
 */
void QXDirect::onQGraph()
{
	m_bPolarView = false;
	if(m_CpGraph.yVariable()!=1)
	{
		m_CpGraph.resetLimits();
		m_CpGraph.setAuto(true);
		m_CpGraph.setYVariable(1);
	}
	m_CpGraph.setInverted(false);
	m_bResetCurves = true;
	m_CpGraph.setYTitle(tr("Q"));

	setControls();

	m_CpGraph.SetXScale();
	setFoilScale();
	updateView();
}



/**
 * The user has requested to rename the Polar
 */
void QXDirect::onRenamePolar()
{
	if(!Polar::curPolar()) return;
	if(!Foil::curFoil()) return;
	MainFrame *pMainFrame = (MainFrame*)s_pMainFrame;

	int resp, k,l;
	Polar* pPolar = NULL;
	OpPoint * pOpp;
	QString OldName = Polar::curPolar()->polarName();

	QStringList NameList;
	for(k=0; k<m_poaPolar->size(); k++)
	{
		pPolar = (Polar*)m_poaPolar->at(k);
		if(pPolar->m_FoilName == Foil::curFoil()->foilName())
			NameList.append(pPolar->m_PlrName);
	}

	RenameDlg renDlg(pMainFrame);
	renDlg.InitDialog(&NameList, Polar::curPolar()->polarName(), tr("Enter the new name for the foil polar :"));

	bool bExists = true;

	while (bExists)
	{
		resp = renDlg.exec();
		if(resp==QDialog::Accepted)
		{
			if (OldName == renDlg.newName()) return;
			//Is the new name already used ?
			bExists = false;
			for (k=0; k<m_poaPolar->size(); k++)
			{
				pPolar = (Polar*)m_poaPolar->at(k);
				if ((pPolar->m_FoilName==Foil::curFoil()->foilName()) && (pPolar->m_PlrName == renDlg.newName()))
				{
					bExists = true;
					break;
				}
			}
			if(!bExists)
			{
				for (l=(int)m_poaOpp->size()-1;l>=0; l--)
				{
					pOpp = (OpPoint*)m_poaOpp->at(l);
					if (pOpp->m_strPlrName == OldName &&
						pOpp->m_strFoilName == Foil::curFoil()->foilName())
					{
						pOpp->m_strPlrName = renDlg.newName();
					}
				}
				Polar::curPolar()->setPolarName(renDlg.newName());
			}
			emit projectModified();
		}
		else if(resp ==10)
		{//user wants to overwrite
			if (OldName == renDlg.newName()) return;
			for (k=0; k<m_poaPolar->size(); k++)
			{
				pPolar = (Polar*)m_poaPolar->at(k);
				if (pPolar->m_PlrName == renDlg.newName())
				{
					bExists = true;
					break;
				}
			}
			for (l=m_poaOpp->size()-1;l>=0; l--)
			{
				pOpp = (OpPoint*)m_poaOpp->at(l);
				if (pOpp->m_strPlrName == pPolar->m_PlrName)
				{
					m_poaOpp->removeAt(l);
					if(pOpp==OpPoint::curOpp()) OpPoint::setCurOpp(NULL);
					delete pOpp;
				}
			}
			m_poaPolar->removeAt(k);
			if(pPolar==Polar::curPolar()) Polar::setCurPolar(NULL);
			delete pPolar;

			//and rename everything
			Polar::curPolar()->polarName() = renDlg.newName();

			for (l=m_poaOpp->size()-1;l>=0; l--)
			{
				pOpp = (OpPoint*)m_poaOpp->at(l);
				if (pOpp->m_strPlrName == OldName &&
					pOpp->m_strFoilName == Foil::curFoil()->foilName())
				{
					pOpp->m_strPlrName = renDlg.newName();
				}
			}

			bExists = false;
			emit projectModified();
		}
		else
		{
			return ;//cancelled
		}
	}
//	Polar::setCurPolar(NULL);
//	OpPoint::setCurOpp(NULL);
//	SetPolar();
	pMainFrame->updatePolarListBox();
	updateView();
}


/**
 *The user has requested the display of the detailed properties of the active OpPoint object.
 */
void QXDirect::onOpPointProps()
{
	if(!OpPoint::curOpp()) return;
	ObjectPropsDlg opDlg((MainFrame*)s_pMainFrame);
	QString strangeProps;
	OpPoint::curOpp()->getOppProperties(strangeProps);
	opDlg.InitDialog(tr("Operating point properties"), strangeProps);
	opDlg.exec();
}




/**
 *The user has requested the display of the detailed properties of the active Polar object.
 */
void QXDirect::onPolarProps()
{
	if(!Polar::curPolar()) return;
	ObjectPropsDlg opDlg((MainFrame*)s_pMainFrame);
	QString strangeProps;
	Polar::curPolar()->getPolarProperties(strangeProps);
	opDlg.InitDialog(tr("Polar properties"), strangeProps);
	opDlg.exec();
}




/**
 * The user has requested to reset all polar graph scales to their automatic default value
 */
void QXDirect::onResetAllPolarGraphsScales()
{
	for(int ig=0; ig<m_PlrGraph.count(); ig++)
	{
		m_PlrGraph[ig]->setAuto(true);
		m_PlrGraph[ig]->resetXLimits();
		m_PlrGraph[ig]->resetYLimits();
	}
	updateView();
}


/**
 * The user has requested the deletion of the dataof the current Polar.
 * The associated OpPoint objects will be deleted too.
 */
void QXDirect::onResetCurPolar()
{
	if(!Polar::curPolar()) return;
	MainFrame* pMainFrame = (MainFrame*)s_pMainFrame;
	Polar::curPolar()->resetPolar();

	OpPoint*pOpp;
	for(int i=m_poaOpp->size()-1;i>=0;i--)
	{
		pOpp = (OpPoint*)m_poaOpp->at(i);
		if(pOpp->m_strFoilName==Foil::curFoil()->foilName() && pOpp->m_strPlrName==Polar::curPolar()->polarName())
		{
			m_poaOpp->removeAt(i);
			delete pOpp;
		}
	}
	OpPoint::setCurOpp(NULL);

	pMainFrame->updateOppListBox();

	m_bResetCurves = true;
	updateView();

	emit projectModified();
}


/**
 * The user has requested the creation of a .plr file with the Polars of the active Foil object.
 */
void QXDirect::onSavePolars()
{
	if(!Foil::curFoil() || !m_poaPolar->size()) return;

	MainFrame* pMainFrame = (MainFrame*)s_pMainFrame;

	QString FileName;
	FileName = Foil::curFoil()->foilName() + ".plr";
	FileName.replace("/", " ");

	FileName = QFileDialog::getSaveFileName(this, tr("Polar File"), Settings::s_LastDirName+"/"+FileName, tr("Polar File (*.plr)"));
	if(!FileName.length()) return;

	QString strong = FileName.right(4);
	if(strong !=".plr" && strong !=".PLR") FileName += ".plr";

	QFile XFile(FileName);
	if (!XFile.open(QIODevice::WriteOnly)) return;

	int pos = FileName.lastIndexOf("/");
	if(pos>0) Settings::s_LastDirName = FileName.left(pos);

	QDataStream ar(&XFile);
#if QT_VERSION >= 0x040500
	ar.setVersion(QDataStream::Qt_4_5);
#endif
	ar.setByteOrder(QDataStream::LittleEndian);

	pMainFrame->writePolars(ar, Foil::curFoil());

	XFile.close();
}


/**
 * The user has toggled the switch for a sequential analysis.
 */
void QXDirect::onSequence()
{
	m_bSequence = m_pctrlSequence->isChecked();
	setOpPointSequence();
}


/**
 * The user has requested the launch of the interface to define a L.E. or T.E. flap.
 */
void QXDirect::onSetFlap()
{
	if(!Foil::curFoil()) return;
	stopAnimate();

	void* ptr = OpPoint::curOpp();
	OpPoint::setCurOpp(NULL);
	m_bResetCurves = true;
	MainFrame* pMainFrame = (MainFrame*)s_pMainFrame;

	onOpPointView();

	FlapDlg flpDlg(pMainFrame);
	flpDlg.m_pBufferFoil  = &m_BufferFoil;
	flpDlg.m_pMemFoil     = Foil::curFoil();
	flpDlg.m_pXFoil       = m_pXFoil;
	flpDlg.InitDialog();

	if(QDialog::Accepted == flpDlg.exec())
	{
		Foil *pNewFoil = new Foil();
		pNewFoil->copyFoil(&m_BufferFoil);
		pNewFoil->m_FoilColor  = MainFrame::getColor(0);
		pNewFoil->m_FoilStyle = 0;
		pNewFoil->m_FoilWidth = 1;
		pNewFoil->m_bPoints = false;

		OpPoint::setCurOpp((OpPoint*)ptr);

		if(!addNewFoil(pNewFoil)) setBufferFoil();
		else                      setFoil(pNewFoil);

		pMainFrame->updateFoilListBox();
		emit projectModified();
	}
	else
	{
		//reset everything
		OpPoint::setCurOpp((OpPoint*)ptr);
		setBufferFoil();
		m_pXFoil->InitXFoilGeometry(Foil::curFoil());
	}

	updateView();
}


/**
 * The user has requested the launch of the interface to modify the radius of the Foil's leading edge.
 */
void QXDirect::onSetLERadius()
{
	if(!Foil::curFoil())	return;
	MainFrame* pMainFrame = (MainFrame*)s_pMainFrame;
	stopAnimate();

	void* ptr = OpPoint::curOpp();
	OpPoint::setCurOpp(NULL);
	m_bResetCurves = true;
	onOpPointView();

	LEDlg lDlg(pMainFrame);
	lDlg.m_pBufferFoil = &m_BufferFoil;
	lDlg.m_pMemFoil    = Foil::curFoil();
	lDlg.InitDialog();

	if(QDialog::Accepted == lDlg.exec())
	{
		Foil *pNewFoil = new Foil();
		pNewFoil->copyFoil(&m_BufferFoil);
		pNewFoil->m_FoilColor  = MainFrame::getColor(0);
		pNewFoil->m_FoilStyle = 0;
		pNewFoil->m_FoilWidth = 1;
		pNewFoil->m_bPoints = false;
		OpPoint::setCurOpp((OpPoint*)ptr);

		if(!addNewFoil(pNewFoil)) setBufferFoil();
		else                      setFoil(pNewFoil);

		pMainFrame->updateFoilListBox();
		emit projectModified();
	}
	else
	{
		//reset everything
		OpPoint::setCurOpp((OpPoint*)ptr);
		setBufferFoil();
		m_pXFoil->InitXFoilGeometry(Foil::curFoil());
	}

	updateView();
}


/**
 * The user has requested the launch of the interface to modify the gap at the Foil's trailing edge.
 */
void QXDirect::onSetTEGap()
{
	MainFrame* pMainFrame = (MainFrame*)s_pMainFrame;
	if(!Foil::curFoil())	return;
	stopAnimate();

	void* ptr = OpPoint::curOpp();
	OpPoint::setCurOpp(NULL);
	m_bResetCurves = true;

	onOpPointView();

	TEGapDlg tegDlg(pMainFrame);
	tegDlg.m_pBufferFoil = &m_BufferFoil;
	tegDlg.m_pMemFoil    = Foil::curFoil();
	tegDlg.m_Gap         = Foil::curFoil()->m_Gap;
	tegDlg.InitDialog();

	if(QDialog::Accepted == tegDlg.exec())
	{
		Foil *pNewFoil = new Foil();
		pNewFoil->copyFoil(&m_BufferFoil);
		pNewFoil->m_FoilColor  = MainFrame::getColor(0);
		pNewFoil->m_FoilStyle = 0;
		pNewFoil->m_FoilWidth = 1;
		pNewFoil->m_bPoints = false;
		OpPoint::setCurOpp((OpPoint*)ptr);

		if(!addNewFoil(pNewFoil)) setBufferFoil();
		else                      setFoil(pNewFoil);

		pMainFrame->updateFoilListBox();
		emit projectModified();
	}
	else
	{
		//reset everything
		OpPoint::setCurOpp((OpPoint*)ptr);
		setBufferFoil();
		m_pXFoil->InitXFoilGeometry(Foil::curFoil());
	}

	updateView();

}


/**
 * The user has requested the display of all OpPoint curves.
 */
void QXDirect::onShowAllOpps()
{
	OpPoint *pOpp;

	m_bCurOppOnly = false;
	((MainFrame*)s_pMainFrame)->showCurOppOnly->setChecked(m_bCurOppOnly);

	for (int i=0; i<m_poaOpp->size(); i++)
	{
		pOpp = (OpPoint*)m_poaOpp->at(i);
		pOpp->lineStyle().m_bIsVisible = true;
	}
    emit projectModified();
	m_bResetCurves = true;
	setCurveParams();
	updateView();
}


/**
 * The user has requested the display of all Polar curves.
 */
void QXDirect::onShowAllPolars()
{
	Polar *pPolar;
	for (int i=0; i<m_poaPolar->size(); i++)
	{
		pPolar = (Polar*)m_poaPolar->at(i);
		pPolar->lineStyle().m_bIsVisible = true;
	}
	emit projectModified();
	m_bResetCurves = true;
	setCurveParams();
	updateView();
}




/**
 * The user has toggled the display of the curve of the active object
 */
void QXDirect::onShowCurve()
{
	//user has toggled visible switch

	if(m_bPolarView)
	{
		if (Polar::curPolar())
		{
			Polar::curPolar()->lineStyle().m_bIsVisible = m_pctrlShowCurve->isChecked();

		}
		m_bResetCurves = true;
	}
	else if (OpPoint::curOpp())
	{
		OpPoint::curOpp()->lineStyle().m_bIsVisible = m_pctrlShowCurve->isChecked();
		m_bResetCurves = true;
	}
	emit projectModified();
	updateView();
}


/**
 * The user has requested the display of only the Polar curves associated to the active Foil
 */
void QXDirect::onShowFoilPolarsOnly()
{
	if(!Foil::curFoil()) return;

	Polar *pPolar;
	for (int i=0; i<m_poaPolar->size(); i++)
	{
		pPolar = (Polar*)m_poaPolar->at(i);
		pPolar->lineStyle().m_bIsVisible = (pPolar->foilName() == Foil::curFoil()->foilName());
	}
	emit projectModified();
	m_bResetCurves = true;
	setCurveParams();
	updateView();
}


/**
 * The user has requested the display of the Polar curves associated to the active Foil
 */
void QXDirect::onShowFoilPolars()
{
	if(!Foil::curFoil()) return;
	Polar *pPolar;
	for (int i=0; i<m_poaPolar->size(); i++)
	{
		pPolar = (Polar*)m_poaPolar->at(i);
		if(pPolar->m_FoilName == Foil::curFoil()->foilName())
		{
			pPolar->lineStyle().m_bIsVisible = true;
		}
	}
	emit projectModified();
	m_bResetCurves = true;
	setCurveParams();
	updateView();
}


/**
 * The user has requested the display of the OpPoint curves associated to the active Foil
 */
void QXDirect::onShowFoilOpps()
{
	if(!Foil::curFoil() || !Polar::curPolar()) return;

	OpPoint *pOpp;

	m_bCurOppOnly = false;
	((MainFrame*)s_pMainFrame)->showCurOppOnly->setChecked(m_bCurOppOnly);

	for(int i=0; i<m_poaOpp->size(); i++)
	{
		pOpp = (OpPoint*)m_poaOpp->at(i);
		if(pOpp->m_strFoilName==Foil::curFoil()->foilName())
			pOpp->lineStyle().m_bIsVisible = true;
	}
	emit projectModified();
	if(!m_bPolarView) m_bResetCurves = true;
	setCurveParams();

	updateView();
}



/**
 * The user has requested the display of the curves of all OpPoint objects associated to the active Polar.
 */
void QXDirect::onShowPolarOpps()
{
	if(!Foil::curFoil() || !Polar::curPolar()) return;

	OpPoint *pOpp;

	m_bCurOppOnly = false;
	((MainFrame*)s_pMainFrame)->showCurOppOnly->setChecked(m_bCurOppOnly);

	for(int i=0; i<m_poaOpp->size(); i++)
	{
		pOpp = (OpPoint*)m_poaOpp->at(i);
		if(pOpp->m_strFoilName==Foil::curFoil()->foilName() && pOpp->m_strPlrName==Polar::curPolar()->polarName())
			pOpp->lineStyle().m_bIsVisible = true;
	}
	emit projectModified();
	if(!m_bPolarView) m_bResetCurves = true;
	setCurveParams();
	updateView();
}


/**
 * The user has toggled the switch used to define the type of input parameter bewteen aoa, Cl, and Re
 */
void QXDirect::onSpec()
{
	if      (m_pctrlSpec1->isChecked()) s_bAlpha = true;
	else if (m_pctrlSpec2->isChecked()) s_bAlpha = false;
	else if (m_pctrlSpec3->isChecked()) s_bAlpha = false;
}


/**
 * The user has toggled the switch which defines if OpPoints should be stored at the end of the analysis
 */
void QXDirect::onStoreOpp()
{
	OpPoint::s_bStoreOpp = m_pctrlStoreOpp->isChecked();
}


/**
 * The user has toggled the switch which defines if the analysis will be viscous or inviscid
 */
void QXDirect::onViscous()
{
	s_bViscous = m_pctrlViscous->isChecked();
}




/**
 * The user has requested the launch of the interface used to define advanced settings for the XFoil analysis
 */
void QXDirect::onXFoilAdvanced()
{
	XFoilAdvancedDlg xfaDlg((MainFrame*)s_pMainFrame);
	xfaDlg.m_IterLimit   = XFoilTask::s_IterLim;
	xfaDlg.m_bAutoInitBL     = XFoilTask::s_bAutoInitBL;
	xfaDlg.m_VAccel      = XFoil::vaccel;
	xfaDlg.m_bFullReport = XFoil::s_bFullReport;
	xfaDlg.InitDialog();

	if (QDialog::Accepted == xfaDlg.exec())
	{
		XFoil::vaccel             = xfaDlg.m_VAccel;
		XFoil::s_bFullReport      = xfaDlg.m_bFullReport;
		XFoilTask::s_bAutoInitBL  = xfaDlg.m_bAutoInitBL;
		XFoilTask::s_IterLim      = xfaDlg.m_IterLimit;
	}
}



/**
 * Reads the analysis parameters from the widgets.
 */
void QXDirect::readParams()
{
	if(!Polar::curPolar()) return;

	if      (m_pctrlSpec1->isChecked()) s_bAlpha = true;
	else if (m_pctrlSpec2->isChecked()) s_bAlpha = false;
	else if (m_pctrlSpec3->isChecked()) s_bAlpha = false;


	if(Polar::curPolar()->polarType()!=XFLR5::FIXEDAOAPOLAR)
	{
		if(s_bAlpha)
		{
			m_Alpha      = m_pctrlAlphaMin->value();
			m_AlphaMax   = m_pctrlAlphaMax->value();
			m_AlphaDelta = m_pctrlAlphaDelta->value();
		}
		else
		{
			m_Cl      = m_pctrlAlphaMin->value();
			m_ClMax   = m_pctrlAlphaMax->value();
			m_ClDelta = m_pctrlAlphaDelta->value();
		}
	}
	else
	{
		m_Reynolds      = m_pctrlAlphaMin->value();
		m_ReynoldsMax   = m_pctrlAlphaMax->value();
		m_ReynoldsDelta = m_pctrlAlphaDelta->value();
	}
	m_bSequence = m_pctrlSequence->isChecked();
	s_bInitBL   = m_pctrlInitBL->isChecked();
	s_bViscous  = m_pctrlViscous->isChecked();
	OpPoint::s_bStoreOpp = m_pctrlStoreOpp->isChecked();
}


/**
 * Saves the user-defined settings
 * @param pSettings a pointer to the QSetting object.
 */
void QXDirect::saveSettings(QSettings *pSettings)
{
	QString str1, str2, str3;
	pSettings->beginGroup("XDirect");
	{
		pSettings->setValue("AlphaSpec", s_bAlpha);
		pSettings->setValue("StoreOpp", OpPoint::s_bStoreOpp);
		pSettings->setValue("ViscousAnalysis", s_bViscous);
		pSettings->setValue("InitBL", s_bInitBL);
		pSettings->setValue("PolarView", m_bPolarView);
		pSettings->setValue("UserGraph", m_bShowUserGraph);
		pSettings->setValue("ShowPanels", m_bShowPanels);
		pSettings->setValue("Type1", m_bType1);
		pSettings->setValue("Type2", m_bType2);
		pSettings->setValue("Type3", m_bType3);
		pSettings->setValue("Type4", m_bType4);
		pSettings->setValue("FromList", m_bFromList);
		pSettings->setValue("FromZero", s_bFromZero);
		pSettings->setValue("TextOutput", m_bShowTextOutput);
		pSettings->setValue("NeutralLine", m_bNeutralLine);
		pSettings->setValue("CurOppOnly", m_bCurOppOnly);
		pSettings->setValue("ShowInviscid", m_bShowInviscid);
		pSettings->setValue("ShowCpGraph", m_bCpGraph);
		pSettings->setValue("Sequence", m_bSequence);
		pSettings->setValue("HighlightOpp",m_bHighlightOpp);
		pSettings->setValue("XFoilVar", m_XFoilVar);
		pSettings->setValue("TimeUpdateInterval", s_TimeUpdateInterval);
		pSettings->setValue("BatchUpdatePolarView", BatchThreadDlg::s_bUpdatePolarView);
		pSettings->setValue("PlrGraph", m_iPlrGraph);

		switch(m_iPlrView)
		{
			case XFLR5::ONEGRAPH:
				pSettings->setValue("PlrView", 1);
				break;
			case XFLR5::TWOGRAPHS:
				pSettings->setValue("PlrView", 2);
				break;
			case XFLR5::FOURGRAPHS:
				pSettings->setValue("PlrView", 4);
				break;
			default:
				pSettings->setValue("PlrView", 0);
				break;
		}

		pSettings->setValue("AlphaMin", m_Alpha);
		pSettings->setValue("AlphaMax", m_AlphaMax);
		pSettings->setValue("AlphaDelta", m_AlphaDelta);
		pSettings->setValue("ClMin", m_Cl);
		pSettings->setValue("ClMax", m_ClMax);
		pSettings->setValue("ClDelta", m_ClDelta);
		pSettings->setValue("ReynoldsMin", m_Reynolds);
		pSettings->setValue("ReynoldsMax", m_ReynoldsMax);
		pSettings->setValue("ReynolsDelta", m_ReynoldsDelta);

		pSettings->setValue("AutoInitBL", XFoilTask::s_bAutoInitBL);
		pSettings->setValue("IterLim", XFoilTask::s_IterLim);
		pSettings->setValue("FullReport", XFoil::s_bFullReport);


		pSettings->setValue("VAccel", m_pXFoil->vaccel);
		pSettings->setValue("KeepOpenErrors", s_bKeepOpenErrors);
		pSettings->setValue("NCrit", s_refPolar.m_ACrit);
		pSettings->setValue("XTopTr", s_refPolar.m_XTop);
		pSettings->setValue("XBotTr", s_refPolar.m_XBot);
		pSettings->setValue("Mach", s_refPolar.m_Mach);
		pSettings->setValue("ASpec", s_refPolar.m_ASpec);

		if(s_refPolar.polarType()==XFLR5::FIXEDSPEEDPOLAR)      pSettings->setValue("Type", 1);
		else if(s_refPolar.polarType()==XFLR5::FIXEDSPEEDPOLAR) pSettings->setValue("Type", 2);
		else if(s_refPolar.polarType()==XFLR5::FIXEDAOAPOLAR)   pSettings->setValue("Type", 4);
		else if(s_refPolar.polarType()==XFLR5::STABILITYPOLAR)  pSettings->setValue("Type", 7);

		pSettings->setValue("NReynolds", s_ReList.count());
		for (int i=0; i<s_ReList.count(); i++)
		{
			str1 = QString("ReList%1").arg(i);
			str2 = QString("MaList%1").arg(i);
			str3 = QString("NcList%1").arg(i);
			pSettings->setValue(str1, s_ReList[i]);
			pSettings->setValue(str2, s_MachList[i]);
			pSettings->setValue(str3, s_NCritList[i]);
		}
	}
	pSettings->endGroup();

	for(int ig=0; ig<m_PlrGraph.count(); ig++)
		m_PlrGraph[ig]->saveSettings(pSettings);

	m_CpGraph.saveSettings(pSettings);
}



/**
 * Initializes the widget values, depending on the type of Polar
 */
void QXDirect::setAnalysisParams()
{
	m_pctrlViscous->setChecked(s_bViscous);
	m_pctrlInitBL->setChecked(s_bInitBL);
	m_pctrlStoreOpp->setChecked(OpPoint::s_bStoreOpp);
//	m_pctrlShowPressure->setChecked(m_bPressure);
//	m_pctrlShowBL->setChecked(m_bBL);

	if(Polar::curPolar())
	{
		if(Polar::curPolar()->polarType()!=XFLR5::FIXEDAOAPOLAR)
		{
			m_pctrlAlphaMin->setPrecision(2);
			m_pctrlAlphaMax->setPrecision(2);
			m_pctrlAlphaDelta->setPrecision(2);
			if(s_bAlpha) m_pctrlSpec1->setChecked(true);
			else         m_pctrlSpec2->setChecked(true);
			m_pctrlSpec3->setEnabled(false);
		}
		else
		{
			m_pctrlSpec3->setChecked(true);
			m_pctrlSpec3->setEnabled(true);
			m_pctrlAlphaMin->setPrecision(0);
			m_pctrlAlphaMax->setPrecision(0);
			m_pctrlAlphaDelta->setPrecision(0);
		}
	}
	else
	{
		if(s_bAlpha) m_pctrlSpec1->setChecked(true);
		else         m_pctrlSpec2->setChecked(true);
		m_pctrlSpec3->setEnabled(false);
	}
	setOpPointSequence();

	setCurveParams();
}


/**
 * Sets the buffer Foil as a copy of the active Foil.
 * All geometric modifications are made on the buffer foil.
 * The buffer foil is the one displayed in the OpPoint view.
 */
void QXDirect::setBufferFoil()
{
	if(!Foil::curFoil() || !Foil::curFoil()->foilName().length()) return;

	m_BufferFoil.copyFoil(Foil::curFoil());

	m_BufferFoil.m_FoilName   = Foil::curFoil()->foilName();
	m_BufferFoil.m_FoilColor  = Foil::curFoil()->m_FoilColor;
	m_BufferFoil.m_FoilStyle = Foil::curFoil()->m_FoilStyle;
	m_BufferFoil.m_FoilWidth = Foil::curFoil()->m_FoilWidth;
}


/**
 * Updates the combobox widgets with the curve data from the active OpPoint or Polar, depending on the active view.
 */
void QXDirect::setCurveParams()
{
	if(m_bPolarView)
	{
		if(Polar::curPolar())
		{
			if(Polar::curPolar()->lineStyle().m_bIsVisible)  m_pctrlShowCurve->setChecked(true);  else  m_pctrlShowCurve->setChecked(false);

			m_LineStyle.m_Color = Polar::curPolar()->lineStyle().m_Color;
			m_LineStyle.m_Style = Polar::curPolar()->lineStyle().m_Style;
			m_LineStyle.m_Width = Polar::curPolar()->lineStyle().m_Width;
			m_LineStyle.m_PointStyle = Polar::curPolar()->lineStyle().m_PointStyle;
			fillComboBoxes();
		}
		else
		{
			fillComboBoxes(false);
		}
	}
	else
	{
		//set Opoint params
		if(OpPoint::curOpp())
		{
			if(OpPoint::curOpp()->lineStyle().m_bIsVisible)  m_pctrlShowCurve->setChecked(true);  else  m_pctrlShowCurve->setChecked(false);

			m_LineStyle.m_Color  = OpPoint::curOpp()->lineStyle().m_Color;
			m_LineStyle.m_Style  = OpPoint::curOpp()->lineStyle().m_Style;
			m_LineStyle.m_Width  = OpPoint::curOpp()->lineStyle().m_Width;
			m_LineStyle.m_PointStyle = OpPoint::curOpp()->lineStyle().m_PointStyle;
			fillComboBoxes();
		}
		else
		{
			fillComboBoxes(false);
		}
	}
	if(Polar::curPolar())
	{
		if(Polar::curPolar()->polarType()!=XFLR5::FIXEDAOAPOLAR)
		{
			m_pctrlUnit1->setText(QString::fromUtf8("°"));
			m_pctrlUnit2->setText(QString::fromUtf8("°"));
			m_pctrlUnit3->setText(QString::fromUtf8("°"));
		}
		else
		{
			m_pctrlUnit1->setText("");
			m_pctrlUnit2->setText("");
			m_pctrlUnit3->setText("");
		}
	}
}


/**
 * Initializes QXDirect with the data of the input Foil object.
 * If no Foil is proposed in input,sets the first stock Foil in alphabetical order.
 * Sets the first Polar object belonging to this Foil, if any.
 * Sets the first OpPoint object belonging to this Polar, if any.
 * @param pFoil a pointer to the active Foil object, or NULL if a stock Foil should be used.
 * @return a pointer to the Foil object which has been set.
 */
Foil* QXDirect::setFoil(Foil* pFoil)
{
	stopAnimate();

	Foil::setCurFoil(pFoil);

	if(!Foil::curFoil())
	{
		//take the first in the array, if any
		if(m_poaFoil->size())
		{
			Foil::setCurFoil((Foil*)m_poaFoil->at(0));
		}
	}


	if(Foil::curFoil() && !m_pXFoil->InitXFoilGeometry(Foil::curFoil()))
	{
		Foil::setCurFoil(NULL);
	}
	else
	{
		if(!Foil::curFoil())
		{
			Polar::setCurPolar(NULL);
			OpPoint::setCurOpp(NULL);
		}
		else if (Polar::curPolar() && Polar::curPolar()->foilName() !=Foil::curFoil()->foilName())
		{
//			Polar::setCurPolar(NULL);
//			OpPoint::setCurOpp(NULL);
		}
		else if (OpPoint::curOpp() && OpPoint::curOpp()->m_strFoilName  !=Foil::curFoil()->foilName())
		{
//			OpPoint::setCurOpp(NULL);
		}
	}

	setBufferFoil();

	setPolar();

	return Foil::curFoil();
}



/**
 * Initializes QXDirect with the specified Polar object.
 * If the specified polar is not valid, a stock polar associated to the current foil will be set.
 * Sets the first OpPoint object belonging to this Polar, if any.
 * Initializes the XFoil object with the Polar's data.
 * @param pPolar a pointer to the Polar object to set. If NULL, a stock polar associated to the current foil will be set.
 * @return a pointer to the Polar object which has been set.
 */
Polar * QXDirect::setPolar(Polar *pPolar)
{
	stopAnimate();

	if(!Foil::curFoil()|| !Foil::curFoil()->foilName().length())
	{
		Polar::setCurPolar(NULL);
		OpPoint::setCurOpp(NULL);
		setAnalysisParams();
		return NULL;
	}

	if(pPolar) Polar::setCurPolar(pPolar);

	if(!Polar::curPolar())
	{
		//try to get one from the object array
		for(int i=0; i<m_poaPolar->size(); i++)
		{
			pPolar = (Polar*)m_poaPolar->at(i);
			if(pPolar && pPolar->foilName()==Foil::curFoil()->foilName())
			{
				//set this one
				Polar::setCurPolar(pPolar);
				break;
			}
		}
	}

	if(Polar::curPolar())
	{
		if(Polar::curPolar()->foilName() != Foil::curFoil()->foilName())
		{
			Polar *pOldPolar;
			bool bFound = false;
			for (int i=0; i<m_poaPolar->size(); i++)
			{
				pOldPolar = (Polar*)m_poaPolar->at(i);
				if ((pOldPolar->foilName() == Foil::curFoil()->foilName()) &&
					(pOldPolar->m_PlrName == Polar::curPolar()->polarName()))
				{
					Polar::setCurPolar(pOldPolar);
					bFound = true;
					break;
				}
			}
			if(!bFound)
			{
				Polar::setCurPolar(NULL);
				OpPoint::setCurOpp(NULL);
			}
		}
		s_bInitBL = true;
		m_pctrlInitBL->setChecked(s_bInitBL);
	}

//	m_pXFoil->InitXFoilAnalysis(Polar::curPolar(), s_bViscous); //useless, will be done in XFoilTask
	m_bResetCurves = true;
	setAnalysisParams();
	setOpp();

	return Polar::curPolar();
}


/**
 * Initializes QXDirect with the OpPoint with the specified aoa.
 * If the OpPoint cannot be found for the active Foil and Polar, a stock OpPoint associated to the current foil and polar will be set.
 * @param Alpha the aoa of the OpPoint to ser
 * @return a pointer to the OpPoint object which has been set.
 */
OpPoint * QXDirect::setOpp(double Alpha)
{
	OpPoint * pOpp = NULL;
	MainFrame* pMainFrame = (MainFrame*)s_pMainFrame;

	if(!Foil::curFoil() || !Polar::curPolar())
	{
		OpPoint::setCurOpp(NULL);
		return NULL;
	}

	if(Alpha < -1234567.0) //the default
	{
		if(OpPoint::curOpp() && OpPoint::curOpp()->m_strFoilName == Foil::curFoil()->foilName())
			pOpp = OpPoint::curOpp();
		else
		{
			//try to get one from the array
			for(int iOpp=0; iOpp<OpPoint::s_oaOpp.count(); iOpp++)
			{
				OpPoint *pOldOpp = (OpPoint*)OpPoint::s_oaOpp.at(iOpp);
				if(pOldOpp->foilName()==Foil::curFoil()->foilName() && pOldOpp->polarName()==Polar::curPolar()->polarName())
				{
					break;
				}
			}
		}
	}
	else
	{
		pOpp = OpPoint::getOpp(Alpha);
	}


	if(pOpp) 
	{
		pMainFrame->selectOpPoint(pOpp);
	}
	OpPoint::setCurOpp(pOpp);

	m_bResetCurves = true;

	setControls();
	setCurveParams();

	return OpPoint::curOpp();
}


/**
 * Initializes the widgets with the sequence parameters for the type of the active Polar object.
 */
void QXDirect::setOpPointSequence()
{
	m_pctrlSequence->setEnabled(Polar::curPolar());
	m_pctrlAlphaMin->setEnabled(Polar::curPolar());
	m_pctrlAnalyze->setEnabled(Polar::curPolar());
	m_pctrlViscous->setEnabled(Polar::curPolar());
	m_pctrlInitBL->setEnabled(Polar::curPolar());
	m_pctrlStoreOpp->setEnabled(Polar::curPolar());

	if(m_bSequence && Polar::curPolar())
	{
		m_pctrlSequence->setCheckState(Qt::Checked);
		m_pctrlAlphaMax->setEnabled(true);
		m_pctrlAlphaDelta->setEnabled(true);
	}
	else if (Polar::curPolar())
	{
		m_pctrlSequence->setCheckState(Qt::Unchecked);
		m_pctrlAlphaMax->setEnabled(false);
		m_pctrlAlphaDelta->setEnabled(false);
	}
	else
	{
		m_pctrlAlphaMax->setEnabled(false);
		m_pctrlAlphaDelta->setEnabled(false);
	}


	if(Polar::curPolar() && Polar::curPolar()->polarType()!=XFLR5::FIXEDAOAPOLAR)
	{
		if(m_pctrlSpec3->isChecked())
		{
			m_pctrlSpec1->setChecked(true);
			s_bAlpha = true;
		}

		if(s_bAlpha)
		{
			m_pctrlAlphaMin->setValue(m_Alpha);
			m_pctrlAlphaMax->setValue(m_AlphaMax);
			m_pctrlAlphaDelta->setValue(m_AlphaDelta);
		}
		else
		{
			m_pctrlAlphaMin->setValue(m_Cl);
			m_pctrlAlphaMax->setValue(m_ClMax);
			m_pctrlAlphaDelta->setValue(m_ClDelta);
		}
		m_pctrlSpec1->setEnabled(true);
		m_pctrlSpec2->setEnabled(true);
		m_pctrlSpec3->setEnabled(false);
	}
	else if(Polar::curPolar() && Polar::curPolar()->polarType()==XFLR5::FIXEDAOAPOLAR)
	{
		m_pctrlSpec3->setChecked(true);
		s_bAlpha = true;		// no choice with type 4 polars
		m_pctrlAlphaMin->setValue(m_Reynolds);
		m_pctrlAlphaMax->setValue(m_ReynoldsMax);
		m_pctrlAlphaDelta->setValue(m_ReynoldsDelta);
		m_pctrlSpec1->setEnabled(false);
		m_pctrlSpec2->setEnabled(false);
		m_pctrlSpec3->setEnabled(true);
	}
	else
	{
		m_pctrlSpec1->setEnabled(false);
		m_pctrlSpec2->setEnabled(false);
		m_pctrlSpec3->setEnabled(false);
	}
}


/**
 * Sets the axis titles for the specified graph
 * @param pGraph a pointer to the Graph object for which the titles will be set
 */
void QXDirect::setGraphTitles(Graph* pGraph)
{
	if(!pGraph) return;

	QString Title;
	Title = Polar::variableName(pGraph->xVariable());
	pGraph->setXTitle(Title);

	Title = Polar::variableName(pGraph->yVariable());
	pGraph->setYTitle(Title);

}




/**
 * Creates the GUI associated to the toolbar.
 */
void QXDirect::setupLayout()
{
	QSizePolicy szPolicyExpanding;
	szPolicyExpanding.setHorizontalPolicy(QSizePolicy::Expanding);
	szPolicyExpanding.setVerticalPolicy(QSizePolicy::Expanding);

	QSizePolicy szPolicyMinimum;
	szPolicyMinimum.setHorizontalPolicy(QSizePolicy::Minimum);
	szPolicyMinimum.setVerticalPolicy(QSizePolicy::Minimum);

	QSizePolicy szPolicyMaximum;
	szPolicyMaximum.setHorizontalPolicy(QSizePolicy::Maximum);
	szPolicyMaximum.setVerticalPolicy(QSizePolicy::Maximum);


	QGroupBox *AnalysisBox = new QGroupBox(tr("Analysis settings"));
	{
		QVBoxLayout *AnalysisGroup = new QVBoxLayout;
		{
			m_pctrlSequence = new QCheckBox(tr("Sequence"));
			m_pctrlStoreOpp = new QCheckBox(tr("Store Opp"));
			m_pctrlAnalyze  = new QPushButton(tr("Analyze"));

			QHBoxLayout *SpecVarsLayout = new QHBoxLayout;
			{
				m_pctrlSpec1 = new QRadioButton("a");
				m_pctrlSpec2 = new QRadioButton(tr("Cl"));
				m_pctrlSpec3 = new QRadioButton(tr("Re"));
				m_pctrlSpec1->setFont(QFont("Symbol"));
				SpecVarsLayout->addWidget(m_pctrlSpec1);
				SpecVarsLayout->addWidget(m_pctrlSpec2);
				SpecVarsLayout->addWidget(m_pctrlSpec3);
			}

			QGridLayout *SequenceGroupLayout = new QGridLayout;
			{
				QLabel *AlphaMinLab   = new QLabel(tr("Start="));
				QLabel *AlphaMaxLab   = new QLabel(tr("End="));
				QLabel *DeltaAlphaLab = new QLabel(tr("D="));
				DeltaAlphaLab->setFont(QFont("Symbol"));
				DeltaAlphaLab->setAlignment(Qt::AlignRight);
				AlphaMinLab->setAlignment(Qt::AlignRight);
				AlphaMaxLab->setAlignment(Qt::AlignRight);

				m_pctrlUnit1 = new QLabel(QString::fromUtf8("°"));
				m_pctrlUnit2 = new QLabel(QString::fromUtf8("°"));
				m_pctrlUnit3 = new QLabel(QString::fromUtf8("°"));

				m_pctrlAlphaMin   = new DoubleEdit();
				m_pctrlAlphaMax   = new DoubleEdit();
				m_pctrlAlphaDelta = new DoubleEdit();
				m_pctrlAlphaMin->setMinimumHeight(20);
				m_pctrlAlphaMax->setMinimumHeight(20);
				m_pctrlAlphaDelta->setMinimumHeight(20);
				m_pctrlAlphaMin->setAlignment(Qt::AlignRight);
				m_pctrlAlphaMax->setAlignment(Qt::AlignRight);
				m_pctrlAlphaDelta->setAlignment(Qt::AlignRight);
				SequenceGroupLayout->addWidget(AlphaMinLab,1,1);
				SequenceGroupLayout->addWidget(AlphaMaxLab,2,1);
				SequenceGroupLayout->addWidget(DeltaAlphaLab,3,1);
				SequenceGroupLayout->addWidget(m_pctrlAlphaMin,1,2);
				SequenceGroupLayout->addWidget(m_pctrlAlphaMax,2,2);
				SequenceGroupLayout->addWidget(m_pctrlAlphaDelta,3,2);
				SequenceGroupLayout->addWidget(m_pctrlUnit1,1,3);
				SequenceGroupLayout->addWidget(m_pctrlUnit2,2,3);
				SequenceGroupLayout->addWidget(m_pctrlUnit3,3,3);
			}

			QHBoxLayout *AnalysisSettings = new QHBoxLayout;
			{
				m_pctrlViscous  = new QCheckBox(tr("Viscous"));
				m_pctrlInitBL   = new QCheckBox(tr("Init BL"));
				AnalysisSettings->addWidget(m_pctrlViscous);
				AnalysisSettings->addWidget(m_pctrlInitBL);
			}

			AnalysisGroup->addLayout(SpecVarsLayout);
			AnalysisGroup->addStretch(1);
			AnalysisGroup->addWidget(m_pctrlSequence);
			AnalysisGroup->addLayout(SequenceGroupLayout);
			AnalysisGroup->addStretch(1);
			AnalysisGroup->addLayout(AnalysisSettings);
			AnalysisGroup->addWidget(m_pctrlStoreOpp);
			AnalysisGroup->addWidget(m_pctrlAnalyze);
		}
		AnalysisBox->setLayout(AnalysisGroup);

	}

	QGroupBox *DisplayBox = new QGroupBox(tr("Display"));
	{
		QVBoxLayout *DisplayGroup = new QVBoxLayout;
		{
			m_pctrlShowBL        = new QCheckBox(tr("Show BL"));
			m_pctrlShowPressure  = new QCheckBox(tr("Show Pressure"));
			m_pctrlAnimate       = new QCheckBox(tr("Animate"));
			m_pctrlAnimateSpeed  = new QSlider(Qt::Horizontal);
			m_pctrlAnimateSpeed->setMinimum(0);
			m_pctrlAnimateSpeed->setMaximum(1000);
			m_pctrlAnimateSpeed->setSliderPosition(500);
			m_pctrlAnimateSpeed->setTickInterval(50);
			m_pctrlAnimateSpeed->setTickPosition(QSlider::TicksBelow);
			DisplayGroup->addWidget(m_pctrlShowBL);
			DisplayGroup->addWidget(m_pctrlShowPressure);
			DisplayGroup->addWidget(m_pctrlAnimate);
			DisplayGroup->addWidget(m_pctrlAnimateSpeed);
			DisplayGroup->addStretch(1);
		//	DisplayGroup->addWidget(m_pctrlHighlightOpp);
		}
		DisplayBox->setLayout(DisplayGroup);
		DisplayBox->setSizePolicy(szPolicyExpanding);
	}

	QGroupBox *pPolarPropsBox = new QGroupBox(tr("Polar properties"));
	{
		m_pctrlPolarProps = new QLabel;
		m_pctrlPolarProps->setSizePolicy(szPolicyExpanding);
		QFont fnt("Courier");
		m_pctrlPolarProps->setFont(fnt);


	//	m_pctrlPolarProps->setReadOnly(true);
	//	m_pctrlPolarProps->setWordWrapMode(QTextOption::NoWrap);
		QHBoxLayout *pPolarPropsLayout = new QHBoxLayout;
		{
			pPolarPropsLayout->addWidget(m_pctrlPolarProps);
		}
		pPolarPropsBox->setLayout(pPolarPropsLayout);
	}

	QGroupBox *pCurveBox = new QGroupBox(tr("Graph Curve Settings"));
	{
		QVBoxLayout *pCurveGroup = new QVBoxLayout;
		{
			QHBoxLayout *pCurveDisplay = new QHBoxLayout;
			{
				m_pctrlShowCurve  = new QCheckBox(tr("Curve"));
				pCurveDisplay->addWidget(m_pctrlShowCurve);
			}

			m_pctrlCurveStyle = new LineCbBox(this);
			m_pctrlCurveWidth = new LineCbBox(this);
			m_pctrlPointStyle = new LineCbBox(this);
			m_pctrlPointStyle->showPoints(true);
			m_pctrlCurveColor = new LineBtn(this);
			m_pctrlCurveColor->setMinimumHeight(m_pctrlCurveStyle->minimumSizeHint().height());

			for (int i=0; i<5; i++)
			{
				m_pctrlCurveStyle->addItem("item");
				m_pctrlCurveWidth->addItem("item");
				m_pctrlPointStyle->addItem("item");
			}

			m_pStyleDelegate = new LineDelegate(m_pctrlCurveStyle);
			m_pWidthDelegate = new LineDelegate(m_pctrlCurveWidth);
			m_pPointDelegate = new LineDelegate(m_pctrlPointStyle);
			m_pctrlCurveStyle->setItemDelegate(m_pStyleDelegate);
			m_pctrlCurveWidth->setItemDelegate(m_pWidthDelegate);
			m_pctrlPointStyle->setItemDelegate(m_pPointDelegate);

			QGridLayout *CurveStyleLayout = new QGridLayout;
			QLabel *lab200 = new QLabel(tr("Style"));
			QLabel *lab201 = new QLabel(tr("Width"));
			QLabel *lab202 = new QLabel(tr("Color"));
			QLabel *lab203 = new QLabel(tr("Points"));
			lab200->setAlignment(Qt::AlignRight |Qt::AlignVCenter);
			lab201->setAlignment(Qt::AlignRight |Qt::AlignVCenter);
			lab202->setAlignment(Qt::AlignRight |Qt::AlignVCenter);
			CurveStyleLayout->addWidget(lab203,1,1);
			CurveStyleLayout->addWidget(lab200,2,1);
			CurveStyleLayout->addWidget(lab201,3,1);
			CurveStyleLayout->addWidget(lab202,4,1);
			CurveStyleLayout->addWidget(m_pctrlPointStyle,1,2);
			CurveStyleLayout->addWidget(m_pctrlCurveStyle,2,2);
			CurveStyleLayout->addWidget(m_pctrlCurveWidth,3,2);
			CurveStyleLayout->addWidget(m_pctrlCurveColor,4,2);
			CurveStyleLayout->setColumnStretch(2,5);

			pCurveGroup->addLayout(pCurveDisplay);
			pCurveGroup->addLayout(CurveStyleLayout);
			pCurveGroup->addStretch(1);
		}
		pCurveBox->setLayout(pCurveGroup);
	}


	QVBoxLayout *mainLayout = new QVBoxLayout;
	{
		m_pctrlMiddleControls = new QStackedWidget;
		m_pctrlMiddleControls->addWidget(DisplayBox);
		m_pctrlMiddleControls->addWidget(pPolarPropsBox);

		mainLayout->addWidget(AnalysisBox);
		mainLayout->addStretch(1);
		mainLayout->addWidget(m_pctrlMiddleControls);
		mainLayout->addStretch(1);
		mainLayout->addWidget(pCurveBox);
		mainLayout->addStretch(1);
	}

	setLayout(mainLayout);

	setAttribute(Qt::WA_AlwaysShowToolTips);

	setSizePolicy(szPolicyExpanding);
}


/**
 * Interrupts the OpPoint animation
 */
void QXDirect::stopAnimate()
{
	if(m_bAnimate)
	{
		m_pAnimateTimer->stop();
		m_bAnimate = false;
		m_pctrlAnimate->setChecked(false);
		setOpp();
	}
}


/**
 * Updates the curve's style based on the selection in the comboboxes.
 */
void QXDirect::updateCurveStyle()
{
	if(m_bPolarView && Polar::curPolar())
	{
		Polar::curPolar()->lineStyle().m_Color = m_LineStyle.m_Color;
		Polar::curPolar()->lineStyle().m_Style = m_LineStyle.m_Style;
		Polar::curPolar()->lineStyle().m_Width = m_LineStyle.m_Width;
		Polar::curPolar()->lineStyle().m_PointStyle = m_LineStyle.m_PointStyle;
		m_bResetCurves = true;
	}
	else if (!m_bPolarView && OpPoint::curOpp())
	{
		OpPoint::curOpp()->lineStyle().m_Color = m_LineStyle.m_Color;
		OpPoint::curOpp()->lineStyle().m_Style = m_LineStyle.m_Style;
		OpPoint::curOpp()->lineStyle().m_Width = m_LineStyle.m_Width;
		OpPoint::curOpp()->lineStyle().m_PointStyle = m_LineStyle.m_PointStyle;
		m_bResetCurves = true;
	}

	updateView();
	emit projectModified();
}


/**
 * Refreshes the 2d central display.
 */
void QXDirect::updateView()
{
	if (!m_bPolarView)
	{
		if(m_bResetCurves) createOppCurves();
	}
	else
	{
		if(m_bResetCurves) createPolarCurves();

	}
	MainFrame *pMainFrame = (MainFrame*)s_pMainFrame;
	pMainFrame->m_pXDirectTileWidget->update();
}




/**
 * The user has requested the duplication of the current Foil.
 */
void QXDirect::onDuplicateFoil()
{
	if(!Foil::curFoil()) return;

	MainFrame *pMainFrame = (MainFrame*)s_pMainFrame;

	Foil *pNewFoil = new Foil;
	pNewFoil->copyFoil(Foil::curFoil());

	if(addNewFoil(pNewFoil))
	{
		pMainFrame->updateFoilListBox();
		setFoil(pNewFoil);
		emit projectModified();
	}
}


/**
 * The user has requested to rename the current Foil.
 */
void QXDirect::onRenameCurFoil()
{
	MainFrame* pMainFrame = (MainFrame*)s_pMainFrame;


	renameFoil(Foil::curFoil());
	pMainFrame->updateFoilListBox();
	setFoil(Foil::curFoil());
	emit projectModified();
}


/**
 * Adds a new Foil to the array.
 * Requests a name, and overwrites any former Foil with that name.
 * @param pFoil a pointer to the Foil to be added to the array
 * @return a pointer to the input Foil, or NULL if the operation was user-cancelled. @todo what's the point ?
 */
Foil* QXDirect::addNewFoil(Foil *pFoil)
{
	if(!pFoil) return NULL;
	QStringList NameList;
	for(int k=0; k<Foil::s_oaFoil.size(); k++)
	{
		Foil*pOldFoil = (Foil*)Foil::s_oaFoil.at(k);
		NameList.append(pOldFoil->m_FoilName);
	}

	RenameDlg renDlg((MainFrame*)s_pMainFrame);
	renDlg.InitDialog(&NameList, pFoil->foilName(), tr("Enter the foil's new name"));

	if(renDlg.exec() != QDialog::Rejected)
	{
		pFoil->setFoilName(renDlg.newName());
		pFoil->insertThisFoil();
		return pFoil;
	}
	return NULL;
}



/**
 * Renames the current Foil.
 * Requests a name, and overwrites any former Foil with that name.
 * @param pFoil a pointer to the Foil to be renamed.
 */
void QXDirect::renameFoil(Foil *pFoil)
{
	if(!pFoil) return;
	QStringList NameList;
	for(int k=0; k<Foil::s_oaFoil.size(); k++)
	{
		Foil*pOldFoil = (Foil*)Foil::s_oaFoil.at(k);
		NameList.append(pOldFoil->m_FoilName);
	}

	RenameDlg renDlg((MainFrame*)s_pMainFrame);
	renDlg.InitDialog(&NameList, pFoil->foilName(), tr("Enter the foil's new name"));

	if(renDlg.exec() != QDialog::Rejected)
	{
		pFoil->renameThisFoil(renDlg.newName());
	}
}



void QXDirect::setView(XFLR5::enumGraphView eView)
{
	if (m_bPolarView)
	{
		m_iPlrView = eView;
	}
}



void QXDirect::setGraphTiles()
{
	MainFrame *pMainFrame = (MainFrame*)s_pMainFrame;

	if(m_bPolarView)
	{
		switch(m_iPlrView)
		{
			case XFLR5::ONEGRAPH:
				pMainFrame->m_pXDirectTileWidget->setGraphList(m_PlrGraph, 1, 0);
				break;
			case XFLR5::TWOGRAPHS:
				pMainFrame->m_pXDirectTileWidget->setGraphList(m_PlrGraph, 2, 0);
				break;
			case XFLR5::FOURGRAPHS:
				pMainFrame->m_pXDirectTileWidget->setGraphList(m_PlrGraph, 4, 0);
				break;
			case XFLR5::ALLGRAPHS:
				pMainFrame->m_pXDirectTileWidget->setGraphList(m_PlrGraph, m_PlrGraph.count(), 0);
				break;
		}
	}
	else
	{
		QList<QGraph*> pGraphList;
		pGraphList.append(&m_CpGraph);
		pMainFrame->m_pXDirectTileWidget->setGraphList(pGraphList, 1, 0, Qt::Vertical);
	}
}



void QXDirect::setFoilScale()
{
	/** @todo set foil scale */
}

