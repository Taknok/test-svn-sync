/****************************************************************************

	QXDirect Class
	Copyright (C) 2008-2009 Andre Deperrois XFLR5@yahoo.com

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

#include <QtGui>
#include <QCheckBox>
#include <math.h>
 
#include "../Globals.h"
#include "../MainFrame.h"
#include "../Graph/GraphVariableDlg.h"
#include "../Graph/GraphDlg.h"
#include "../Misc/EditPlrDlg.h"
#include "XDirect.h"
#include "XFoilAnalysisDlg.h"
#include "FoilPolarDlg.h"
#include "TwoDPanelDlg.h"
#include "InterpolateFoilsDlg.h"
#include "NacaFoilDlg.h"
#include "BatchDlg.h"
#include "FoilCoordDlg.h"
#include "FoilGeomDlg.h"
#include "TEGapDlg.h"
#include "LEDlg.h"
#include "FlapDlg.h"
#include "CAddDlg.h"
#include "XFoilAdvancedDlg.h"
#include "XDirectStyleDlg.h"

QXDirect::QXDirect(QWidget *parent)
    : QWidget(parent)
{
	SetupLayout();

	m_pXFoil = NULL;
	m_pXFoil = new XFoil();

	m_CurveStyle = 0;
	m_CurveWidth = 1;
	m_CurveColor = QColor(0,0,0);

	m_pAnimateTimer = new QTimer(this);
	m_posAnimate = 0; // no animation to start with
	Connect();

	FillComboBoxes(false);

	m_hcArrow.setShape(Qt::ArrowCursor);
	m_hcCross.setShape(Qt::CrossCursor);
	m_hcMove.setShape(Qt::ClosedHandCursor);

	pi = 3.141592654;

	m_bAnimate        = false;
	m_bAnimatePlus    = false;
	m_bAutoInitBL     = true;
	m_bCpGraph        = true;
	m_bTransGraph     = false;
	m_bShowPanels     = true;
	m_bShowUserGraph  = true;
	m_bSequence       = false;

	m_Type            = 1;

	m_bTrans          = false;
	m_bType1          = true;
	m_bType2          = true;
	m_bType3          = true;
	m_bType4          = true;
	m_bPressure       = true;
	m_bBL             = false;
	m_bFromList       = false;
	m_bFromZero       = false;
	m_bShowTextOutput = true;
	m_bNeutralLine    = true;
	m_bShowInviscid   = false;
	m_bAlpha          = true;
	m_bInitBL         = true;
	m_bStoreOpp       = true;
	m_bViscous        = true;
	m_bCurOppOnly     = true;

	m_iNeutralStyle = 2;
	m_iNeutralWidth = 1;
	m_crNeutralColor = QColor(200,200,255);

	m_crBLColor = QColor(200,70,70);
	m_iBLStyle = 1;
	m_iBLWidth = 1;
	m_crPressureColor= QColor(0,255,0);
	m_iPressureStyle = 0;
	m_iPressureWidth = 1;

	m_IterLim   = 100;

	m_bPolar          = true;
	m_iPlrGraph = 1;
	m_iPlrView  = 0;
	m_OppVar    = 0;
	m_XFoilVar  = 0;
	m_FoilYPos  = 150;

	m_PointDown.setX(0);
	m_PointDown.setY(0);

	m_iFoilStyle = 0;
	m_iFoilWidth = 0;

	m_XPolar     = 2;
	m_YPolar     = 1;
	m_XCz        = 0;
	m_YCz        = 1;
	m_XCm        = 0;
	m_YCm        = 5;
	m_XTr        = 6;
	m_YTr        = 1;
	m_XUser      =  0;
	m_YUser      = 10;

	m_posAnimate = 0;

	m_pCmGraph     = NULL;
	m_pCpGraph     = NULL;
	m_pCurGraph    = NULL;
	m_pCzGraph     = NULL;
	m_pPolarGraph  = NULL;
	m_pTrGraph     = NULL;
	m_pUserGraph   = NULL;
	m_pCurFoil     = NULL;
	m_pCurPolar    = NULL;
	m_pCurOpp      = NULL;


	m_Reynolds      = 100000.0;
	m_ReynoldsMax   = 150000.0;
	m_ReynoldsDelta =  10000.0;

	m_NCrit = 9.0;
	m_XTopTr = 1.0;
	m_XBotTr = 1.0;
	m_Mach = 0.0;

	m_Alpha      = 0.0;
	m_AlphaMax   = 1.0;
	m_AlphaDelta = 0.5;
	m_Cl         = 0.0;
	m_ClMax      = 1.0;
	m_ClDelta    = 0.1;
	m_pCpGraph    = new QGraph();
	m_pPolarGraph = new QGraph();
	m_pCmGraph    = new QGraph();
	m_pCzGraph    = new QGraph();
	m_pTrGraph    = new QGraph();
	m_pUserGraph  = new QGraph();


	m_pCpGraph->SetXTitle("X");
	m_pCpGraph->SetYTitle("Cp");
	m_pCpGraph->SetInverted(true);
	m_pCpGraph->SetXMin(0.0);
	m_pCpGraph->SetXMax(1.0);
	m_pCpGraph->SetYMin(-0.1);
	m_pCpGraph->SetYMax(0.1);
	m_pCpGraph->SetMargin(50);
	m_pCpGraph->SetBkColor(QColor(0,30,50));
	m_pCpGraph->SetBorderColor(QColor(200,200,200));
	m_pCpGraph->SetBorder(true);
	m_pCpGraph->SetBorderStyle(0);

	m_pPolarGraph->SetXMin(0.0);
	m_pPolarGraph->SetXMax(0.1);
	m_pPolarGraph->SetYMin(-0.1);
	m_pPolarGraph->SetYMax(0.1);
	m_pPolarGraph->SetType(2);
	m_pPolarGraph->SetBkColor(QColor(0,30,50));
	m_pPolarGraph->SetBorderColor(QColor(200,200,200));
	m_pPolarGraph->SetBorder(true);
	m_pPolarGraph->SetBorderStyle(0);
	m_pPolarGraph->SetBorderWidth(3);
	m_pPolarGraph->SetMargin(50);

	m_pCmGraph->SetXMin(0.0);
	m_pCmGraph->SetXMax(0.1);
	m_pCmGraph->SetYMin(-0.1);
	m_pCmGraph->SetYMax(0.1);
	m_pCmGraph->SetType(2);
	m_pCmGraph->SetBkColor(QColor(0,30,50));
	m_pCmGraph->SetBorderColor(QColor(200,200,200));
	m_pCmGraph->SetBorder(true);
	m_pCmGraph->SetBorderStyle(0);
	m_pCmGraph->SetBorderWidth(3);
	m_pCmGraph->SetMargin(50);

	m_pCzGraph->SetXMin(0.0);
	m_pCzGraph->SetXMax(0.1);
	m_pCzGraph->SetYMin(-0.1);
	m_pCzGraph->SetYMax(0.1);
	m_pCzGraph->SetType(2);
	m_pCzGraph->SetBkColor(QColor(0,30,50));
	m_pCzGraph->SetBorderColor(QColor(200,200,200));
	m_pCzGraph->SetBorder(true);
	m_pCzGraph->SetBorderStyle(0);
	m_pCzGraph->SetBorderWidth(3);
	m_pCzGraph->SetMargin(50);

	m_pTrGraph->SetXMin(0.0);
	m_pTrGraph->SetXMax(0.1);
	m_pTrGraph->SetYMin(-0.1);
	m_pTrGraph->SetYMax(0.1);
	m_pTrGraph->SetType(2);
	m_pTrGraph->SetBkColor(QColor(0,30,50));
	m_pTrGraph->SetBorderColor(QColor(200,200,200));
	m_pTrGraph->SetBorder(true);
	m_pTrGraph->SetBorderStyle(0);
	m_pTrGraph->SetBorderWidth(3);
	m_pTrGraph->SetMargin(50);

	m_pUserGraph->SetXMin(0.0);
	m_pUserGraph->SetXMax(0.1);
	m_pUserGraph->SetYMin(-0.1);
	m_pUserGraph->SetYMax(0.1);
	m_pUserGraph->SetType(2);
	m_pUserGraph->SetBkColor(QColor(0,30,50));
	m_pUserGraph->SetBorderColor(QColor(200,200,200));
	m_pUserGraph->SetBorder(true);
	m_pUserGraph->SetBorderStyle(0);
	m_pUserGraph->SetBorderWidth(3);
	m_pUserGraph->SetMargin(50);


	SetGraphTitles(m_pPolarGraph, m_XPolar,m_YPolar);
	SetGraphTitles(m_pCzGraph, m_XCz, m_YCz);
	SetGraphTitles(m_pCmGraph, m_XCm, m_YCm);
	SetGraphTitles(m_pTrGraph, m_XTr, m_YTr);
	SetGraphTitles(m_pUserGraph, m_XUser, m_YUser);

	memset(m_ReList, 0, sizeof(m_ReList));
	memset(m_NCritList, 0, sizeof(m_NCritList));
	memset(m_MachList, 0, sizeof(m_MachList));
	m_ReList[0]  =   40000.0;
	m_ReList[1]  =   60000.0;
	m_ReList[2]  =   80000.0;
	m_ReList[3]  =  100000.0;
	m_ReList[4]  =  130000.0;
	m_ReList[5]  =  160000.0;
	m_ReList[6]  =  200000.0;
	m_ReList[7]  =  250000.0;
	m_ReList[8]  =  300000.0;
	m_ReList[9]  =  500000.0;
	m_ReList[10] = 1000000.0;
	m_ReList[11] = 3000000.0;

	m_NCritList[0]  = 9.00;
	m_NCritList[1]  = 9.00;
	m_NCritList[2]  = 9.00;
	m_NCritList[3]  = 9.00;
	m_NCritList[4]  = 9.00;
	m_NCritList[5]  = 9.00;
	m_NCritList[6]  = 9.00;
	m_NCritList[7]  = 9.00;
	m_NCritList[8]  = 9.00;
	m_NCritList[9]  = 9.00;
	m_NCritList[10] = 9.00;
	m_NCritList[11] = 9.00;
	m_NRe = 12;

	m_FoilOffset.setX(0);
	m_FoilOffset.setY(400);
}


QXDirect::~QXDirect()
{
	delete m_pCpGraph;
	delete m_pCmGraph;
	delete m_pCzGraph;
	delete m_pPolarGraph;
	delete m_pTrGraph;
	delete m_pUserGraph;
}


void QXDirect::AddOpData(OpPoint *pOpPoint)
{
	// Adds result of the XFoil Calculation to the OpPoint object
	int i, j, ibl, is, k;

	pOpPoint->m_strFoilName = m_pCurFoil->m_FoilName;
	pOpPoint->m_strPlrName  = m_pCurPolar->m_PlrName;

	pOpPoint->n        = m_pXFoil->n;
	pOpPoint->Cd       = m_pXFoil->cd;
	pOpPoint->Cdp      = m_pXFoil->cdp;
	pOpPoint->Cl       = m_pXFoil->cl;
	pOpPoint->m_XCP    = m_pXFoil->xcp;
	pOpPoint->Cm       = m_pXFoil->cm;
	pOpPoint->Reynolds = m_pXFoil->reinf;
	pOpPoint->Mach     = m_pXFoil->minf;
	pOpPoint->ACrit    = m_pXFoil->acrit;
	pOpPoint->m_bTEFlap    = m_pCurFoil->m_bTEFlap;
	pOpPoint->m_bLEFlap    = m_pCurFoil->m_bLEFlap;

	pOpPoint->Cpmn   = m_pXFoil->cpmn;

	for (k=0; k<m_pXFoil->n; k++)
	{
		pOpPoint->x[k]   = m_pXFoil->x[k+1];
		pOpPoint->y[k]   = m_pXFoil->y[k+1];
//		pOpPoint->s[k]   = m_pXFoil->s[k+1];
		pOpPoint->Cpi[k] = m_pXFoil->cpi[k+1];
		pOpPoint->Qi[k]  = m_pXFoil->qgamm[k+1];
	}

	if(m_pXFoil->lvisc && m_pXFoil->lvconv)
	{
		pOpPoint->Xtr1 =m_pXFoil->xoctr[1];
		pOpPoint->Xtr2 =m_pXFoil->xoctr[2];
		pOpPoint->m_bVisc = true; 
		pOpPoint->m_bDispSurf = true;
		for (k=0; k<m_pXFoil->n; k++) 
		{
			pOpPoint->Cpv[k] = m_pXFoil->cpv[k+1];
			pOpPoint->Qv[k] = m_pXFoil->qvis[k+1];
		}
	}
	else
	{
		pOpPoint->m_bVisc = false;
	}

	if(pOpPoint->m_bTEFlap || pOpPoint->m_bLEFlap)
	{
//		SetHingeMoments(pOpPoint); //TODO
//		pOpPoint->HMom = m_pXFoil->hmom;
//		pOpPoint->XForce = m_pXFoil->hfx;
//		pOpPoint->YForce = m_pXFoil->hfy;
	}

	if(!m_pXFoil->lvisc || !m_pXFoil->lvconv)	return;

//---- add boundary layer on both sides of airfoil 
	int nd1=0;
	int nd2=0;
	int nd3=0;
	for (is=1; is<=2; is++)
	{
		for ( ibl=2; ibl<=m_pXFoil->iblte[is];ibl++)
		{
			i = m_pXFoil->ipan[ibl][is];
			pOpPoint->xd1[i] = m_pXFoil->x[i] + m_pXFoil->nx[i]*m_pXFoil->dstr[ibl][is];
			pOpPoint->yd1[i] = m_pXFoil->y[i] + m_pXFoil->ny[i]*m_pXFoil->dstr[ibl][is];
			nd1++;
		}
	}

//---- set upper and lower wake dstar fractions based on first wake point
	is=2;
	double dstrte = m_pXFoil->dstr[m_pXFoil->iblte[is]+1][is];
	double dsf1, dsf2;
	if(dstrte!=0.0) 
	{
		dsf1 = (m_pXFoil->dstr[m_pXFoil->iblte[1]][1] + 0.5*m_pXFoil->ante) / dstrte;
		dsf2 = (m_pXFoil->dstr[m_pXFoil->iblte[2]][2] + 0.5*m_pXFoil->ante) / dstrte;
	}
	else
	{
		dsf1 = 0.5;
		dsf2 = 0.5;
	}

//---- plot upper wake displacement surface
	ibl = m_pXFoil->iblte[1];
	i = m_pXFoil->ipan[ibl][1];
	pOpPoint->xd2[0] = m_pXFoil->x[i] + m_pXFoil->nx[i]*m_pXFoil->dstr[ibl][1];
	pOpPoint->yd2[0] = m_pXFoil->y[i] + m_pXFoil->ny[i]*m_pXFoil->dstr[ibl][1];
	nd2++;

	j= m_pXFoil->ipan[m_pXFoil->iblte[is]+1][is]  -1;
	for (ibl=m_pXFoil->iblte[is]+1; ibl<=m_pXFoil->nbl[is]; ibl++)
	{
		i = m_pXFoil->ipan[ibl][is];
		pOpPoint->xd2[i-j] = m_pXFoil->x[i] - m_pXFoil->nx[i]*m_pXFoil->dstr[ibl][is]*dsf1;
		pOpPoint->yd2[i-j] = m_pXFoil->y[i] - m_pXFoil->ny[i]*m_pXFoil->dstr[ibl][is]*dsf1;
		nd2++;
	}
	
//---- plot lower wake displacement surface
	ibl = m_pXFoil->iblte[2];
	i = m_pXFoil->ipan[ibl][2];
	pOpPoint->xd3[0] = m_pXFoil->x[i] + m_pXFoil->nx[i]*m_pXFoil->dstr[ibl][2];
	pOpPoint->yd3[0] = m_pXFoil->y[i] + m_pXFoil->ny[i]*m_pXFoil->dstr[ibl][2];
	nd3++;

	j = m_pXFoil->ipan[m_pXFoil->iblte[is]+1][is]  -1;
	for (ibl=m_pXFoil->iblte[is]+1; ibl<=m_pXFoil->nbl[is]; ibl++)
	{
		i = m_pXFoil->ipan[ibl][is];
		pOpPoint->xd3[i-j] = m_pXFoil->x[i] + m_pXFoil->nx[i]*m_pXFoil->dstr[ibl][is]*dsf2;
		pOpPoint->yd3[i-j] = m_pXFoil->y[i] + m_pXFoil->ny[i]*m_pXFoil->dstr[ibl][is]*dsf2;
		nd3++;
	}
	pOpPoint->nd1 = nd1;
	pOpPoint->nd2 = nd2;
	pOpPoint->nd3 = nd3;
}



OpPoint* QXDirect::AddOpPoint()
{
	// adds an Operating Point to the array from XFoil results

	MainFrame*pMainFrame = (MainFrame*)m_pMainFrame;
	int i;

	OpPoint *pNewPoint = new OpPoint();
	if(pNewPoint ==NULL)
	{
//			AfxMessageBox("Not enough memory to store the OpPoint\nOnly the polar point will be saved", MB_OK);
		return NULL;
	}
	else
	{
		if(!m_pXFoil->lvconv)
		{
			delete pNewPoint;
			return NULL;
		}
		pNewPoint->m_pOperWidget = this;
		pNewPoint->Alpha = m_pXFoil->alfa * 180/pi;
		pNewPoint->m_Color = pMainFrame->GetColor(2);
		AddOpData(pNewPoint);
		pMainFrame->SetSaveState(false);
	}

	if(m_bStoreOpp)
	{
		//insert the OpPoint in the Operating points array
		InsertOpPoint(pNewPoint);
	}

	// Now insert OpPoint in the current CPolar object

	if(m_pXFoil->lvconv && m_pCurPolar) 
	{
		if(m_pCurPolar->m_Type ==2 || m_pCurPolar->m_Type ==3)
		{
			if(pNewPoint && pNewPoint->Reynolds<1.00e7)
			{
				m_pCurPolar->AddData(pNewPoint);
			}
		}
		else
		{
			m_pCurPolar->AddData(pNewPoint);
		}
	}

	if(!m_bStoreOpp)
	{
		delete pNewPoint;
		pNewPoint = NULL;
	}
	m_pCurOpp = pNewPoint;

	if(m_bPolar)
	{
		CreatePolarCurves();
//		UpdateView();
	}
	else
	{
		SetOpp(-1.e10);
	}
	pMainFrame->SetSaveState(false);
	return pNewPoint;
}


void QXDirect::CheckButtons()
{
	MainFrame* pMainFrame = (MainFrame*)m_pMainFrame;

	pMainFrame->m_pctrlOppView->setChecked(!m_bPolar);
	pMainFrame->m_pctrlPolarView->setChecked(m_bPolar);

	pMainFrame->OpPointsAct->setChecked(!m_bPolar);
	pMainFrame->PolarsAct->setChecked(m_bPolar);

	pMainFrame->showPanels->setChecked(m_bShowPanels);
	pMainFrame->showNeutralLine->setChecked(m_bNeutralLine);
	pMainFrame->showInviscidCurve->setChecked(m_bShowInviscid);
	pMainFrame->showCurOppOnly->setChecked(m_bCurOppOnly);

	pMainFrame->setCpVarGraph->setChecked(m_OppVar==0);
	pMainFrame->setQVarGraph->setChecked(m_OppVar==1);

	pMainFrame->PolarGraphAct[0]->setChecked(m_iPlrView==1 && m_iPlrGraph == 1);
	pMainFrame->PolarGraphAct[1]->setChecked(m_iPlrView==1 && m_iPlrGraph == 2);
	pMainFrame->PolarGraphAct[2]->setChecked(m_iPlrView==1 && m_iPlrGraph == 3);
	pMainFrame->PolarGraphAct[3]->setChecked(m_iPlrView==1 && m_iPlrGraph == 4);
	pMainFrame->PolarGraphAct[4]->setChecked(m_iPlrView==1 && m_iPlrGraph == 5);

	pMainFrame->TwoPolarGraphsAct->setChecked(m_iPlrView==2);
	pMainFrame->AllPolarGraphsAct->setChecked(m_iPlrView==0);

	pMainFrame->CurXFoilCtPlot->setChecked(!m_bPolar  && m_OppVar==2 && m_XFoilVar ==1);
	pMainFrame->CurXFoilDbPlot->setChecked(!m_bPolar  && m_OppVar==2 && m_XFoilVar ==2);
	pMainFrame->CurXFoilDtPlot->setChecked(!m_bPolar  && m_OppVar==2 && m_XFoilVar ==3);
	pMainFrame->CurXFoilRtLPlot->setChecked(!m_bPolar && m_OppVar==2 && m_XFoilVar ==4);
	pMainFrame->CurXFoilRtPlot->setChecked(!m_bPolar  && m_OppVar==2 && m_XFoilVar ==5);
	pMainFrame->CurXFoilNPlot->setChecked(!m_bPolar   && m_OppVar==2 && m_XFoilVar ==6);
	pMainFrame->CurXFoilCdPlot->setChecked(!m_bPolar  && m_OppVar==2 && m_XFoilVar ==7);
	pMainFrame->CurXFoilCfPlot->setChecked(!m_bPolar  && m_OppVar==2 && m_XFoilVar ==8);
	pMainFrame->CurXFoilUePlot->setChecked(!m_bPolar  && m_OppVar==2 && m_XFoilVar ==9);
	pMainFrame->CurXFoilHPlot->setChecked(!m_bPolar   && m_OppVar==2 && m_XFoilVar ==10);
}



void QXDirect::Connect()
{
	connect(m_pctrlSpec1, SIGNAL(clicked()), this, SLOT(OnSpec()));
	connect(m_pctrlSpec2, SIGNAL(clicked()), this, SLOT(OnSpec()));
	connect(m_pctrlSpec3, SIGNAL(clicked()), this, SLOT(OnSpec()));
	connect(m_pctrlAnalyze, SIGNAL(clicked()), this, SLOT(OnAnalyze()));
	connect(m_pctrlAlphaMin, SIGNAL(editingFinished()), this, SLOT(OnAlphaMinChanged()));
	connect(m_pctrlAlphaMax, SIGNAL(editingFinished()), this, SLOT(OnAlphaMaxChanged()));
	connect(m_pctrlAlphaDelta, SIGNAL(editingFinished()), this, SLOT(OnDeltaAlphaChanged()));
	connect(m_pctrlCurveStyle, SIGNAL(activated(int)), this, SLOT(OnCurveStyle(int)));
	connect(m_pctrlCurveWidth, SIGNAL(activated(int)), this, SLOT(OnCurveWidth(int)));
	connect(m_pctrlCurveColor, SIGNAL(clicked()), this, SLOT(OnCurveColor()));
	connect(m_pctrlSequence, SIGNAL(clicked()), this, SLOT(OnSequence()));
	connect(m_pctrlInitBL, SIGNAL(clicked()), this, SLOT(OnInitBL()));
	connect(m_pctrlShowBL, SIGNAL(clicked()), this, SLOT(OnShowBL()));
	connect(m_pctrlShowPressure, SIGNAL(clicked()), this, SLOT(OnShowPressure()));
	connect(m_pctrlStoreOpp, SIGNAL(clicked()), this, SLOT(OnStoreOpp()));
	connect(m_pctrlShowPoints, SIGNAL(clicked()), this, SLOT(OnShowPoints()));
	connect(m_pctrlShowCurve, SIGNAL(clicked()), this, SLOT(OnShowCurve()));

	connect(m_pctrlAnimate, SIGNAL(clicked(bool)), this, SLOT(OnAnimate(bool)));
	connect(m_pctrlAnimateSpeed, SIGNAL(sliderMoved(int)), this, SLOT(OnAnimateSpeed(int)));
	connect(m_pAnimateTimer, SIGNAL(timeout()), this, SLOT(OnAnimateSingle()));
}



void QXDirect::CreateOppCurves(OpPoint *pOpp)
{
	OpPoint *pOpPoint = NULL;
	if(pOpp) pOpPoint = pOpp; else pOpPoint = m_pCurOpp;

	CCurve *pCurve1;
	QString str;
	int k;

	m_pCpGraph->DeleteCurves();

	if(m_bCurOppOnly && pOpPoint)
	{
		if(!pOpPoint || !pOpPoint->m_bIsVisible) return;
		pCurve1    = m_pCpGraph->AddCurve();
		if(pOpPoint->m_bShowPoints) pCurve1->ShowPoints(true);
		
		pCurve1->SetStyle(pOpPoint->m_Style);
		pCurve1->SetColor(pOpPoint->m_Color);
		pCurve1->SetWidth(pOpPoint->m_Width);

		str = QString("-Re=%1-Alpha=%2").arg(pOpPoint->Reynolds,8,'f',0).arg(pOpPoint->Alpha,5,'f',2);
		str = pOpPoint->m_strFoilName+str;
		pCurve1->SetTitle(str);

		FillOppCurve(pOpPoint, m_pCpGraph, pCurve1);

		if(m_bShowInviscid && pOpPoint)
		{
			CCurve *pCpi = m_pCpGraph->AddCurve();
			if(pOpPoint->m_bShowPoints) pCpi->ShowPoints(true);
			pCpi->SetStyle(1);
			pCpi->SetColor(pOpPoint->m_Color.darker(150));
			pCpi->SetWidth(pOpPoint->m_Width);
			str= QString("-Re=%1-Alpha=%2_Inviscid").arg(pOpPoint->Reynolds,8,'f',0).arg(pOpPoint->Alpha,5,'f',2);
			str = pOpPoint->m_strFoilName+str;
			pCpi->SetTitle(str);
			FillOppCurve(pOpPoint, m_pCpGraph, pCpi, true);
		}
	}
	else if(!m_bCurOppOnly)
	{
		for (k=0; k<m_poaOpp->size(); k++)
		{
			pOpp = (OpPoint*)m_poaOpp->at(k);
			if (pOpp && pOpp->m_bIsVisible)
			{
				pCurve1    = m_pCpGraph->AddCurve();

				if(pOpp->m_bShowPoints) pCurve1->ShowPoints(true);

				pCurve1->SetStyle(pOpp->m_Style);
				pCurve1->SetColor(pOpp->m_Color);
				pCurve1->SetWidth(pOpp->m_Width);

				str= QString("-Re=%1-Alpha=%2").arg(pOpp->Reynolds,8,'f',0).arg(pOpp->Alpha,5,'f',2);
				str = pOpp->m_strFoilName+str;
				pCurve1->SetTitle(str);

				FillOppCurve(pOpp, m_pCpGraph, pCurve1);
			}
		}
	}

}


void QXDirect::CreatePolarCurves()
{
	// curves must be entirely reconstructed each time from the
	// operating points database, since user may have added
	// or deleted points & polars
	int k;
	CPolar *pPolar;
	QString str;

	m_pPolarGraph->DeleteCurves();
	m_pCmGraph->DeleteCurves();
	m_pCzGraph->DeleteCurves();
	m_pTrGraph->DeleteCurves();
	m_pUserGraph->DeleteCurves();

	for (k=0; k<m_poaPolar->size(); k++)
	{
		pPolar = (CPolar*)m_poaPolar->at(k);

		if (pPolar->m_bIsVisible && pPolar->m_Alpha.size()>0)
		{
                        if (	(pPolar->m_Type == 1 && m_bType1) ||
                                (pPolar->m_Type == 2 && m_bType2) ||
                                (pPolar->m_Type == 3 && m_bType3) ||
                                (pPolar->m_Type == 4 && m_bType4))
			{
				CCurve* pPolarCurve = m_pPolarGraph->AddCurve();
				CCurve* pCmCurve    = m_pCmGraph->AddCurve();
				CCurve* pCzCurve    = m_pCzGraph->AddCurve();
				CCurve* pTr1Curve   = m_pTrGraph->AddCurve();
				CCurve* pTr2Curve = NULL;
				if(m_XTr == 6)	pTr2Curve   = m_pTrGraph->AddCurve();

				CCurve* pUserCurve  = m_pUserGraph->AddCurve();

				if(pPolar->m_bShowPoints)
				{
					pPolarCurve->ShowPoints(true);
					pCmCurve->ShowPoints(true);
					pCzCurve->ShowPoints(true);
					pTr1Curve->ShowPoints(true);
					if(pTr2Curve) pTr2Curve->ShowPoints(true);
					pUserCurve->ShowPoints(true);
				}

				pPolarCurve->SetStyle(pPolar->m_Style);
				pCmCurve->SetStyle(pPolar->m_Style);
				pCzCurve->SetStyle(pPolar->m_Style);
				pTr1Curve->SetStyle(pPolar->m_Style);
				if(pTr2Curve) pTr2Curve->SetStyle(pPolar->m_Style);
				pUserCurve->SetStyle(pPolar->m_Style);

				pPolarCurve->SetColor(pPolar->m_Color);
				pCmCurve->SetColor(pPolar->m_Color);
				pCzCurve->SetColor(pPolar->m_Color);
				pTr1Curve->SetColor(pPolar->m_Color);
				if(pTr2Curve) pTr2Curve->SetColor(pPolar->m_Color);
				pUserCurve->SetColor(pPolar->m_Color);

				pPolarCurve->SetWidth(pPolar->m_Width);
				pCmCurve->SetWidth(pPolar->m_Width);
				pCzCurve->SetWidth(pPolar->m_Width);
				pTr1Curve->SetWidth(pPolar->m_Width);
				if(pTr2Curve) pTr2Curve->SetWidth(pPolar->m_Width);
				pUserCurve->SetWidth(pPolar->m_Width);

				FillPolarCurve(pPolarCurve, pPolar, m_XPolar, m_YPolar);
				FillPolarCurve(pCmCurve, pPolar, m_XCm, m_YCm);
				FillPolarCurve(pCzCurve, pPolar, m_XCz, m_YCz);
				FillPolarCurve(pTr1Curve, pPolar, m_XTr, m_YTr);
				if(pTr2Curve) FillPolarCurve(pTr2Curve, pPolar, 7, 1);
				FillPolarCurve(pUserCurve, pPolar, m_XUser, m_YUser);

				pPolarCurve->SetTitle(pPolar->m_PlrName);
				pCmCurve->SetTitle(pPolar->m_PlrName);
				pUserCurve->SetTitle(pPolar->m_PlrName);

				if(pTr2Curve)
				{
					str = pPolar->m_PlrName + " / Xtr1";
					pTr1Curve->SetTitle(str);
					str = pPolar->m_PlrName + " / Xtr2";
					pTr2Curve->SetTitle(str);
				}
				else
				{
					pTr1Curve->SetTitle(pPolar->m_PlrName);
				}
			}
		}
	}
}

void QXDirect::DeleteFoil(bool bAsk)
{
	if(!m_pCurFoil || !m_pCurFoil->m_FoilName.length()) return;

	MainFrame* pFrame = (MainFrame*)m_pMainFrame;

	if(pFrame->DeleteFoil(m_pCurFoil, bAsk))
	{
		m_pCurOpp = NULL;
		m_pCurPolar = NULL;
		m_pCurFoil = NULL;
		if(m_bPolar) CreatePolarCurves();
		else         CreateOppCurves();
		UpdateView();
	}
}


void QXDirect::DeleteOpPoint(bool bCurrent)
{
	int i;
	if(bCurrent)
	{
		// we remove only the current OpPoint
		
		OpPoint* opt;
		for (i=0; i<m_poaOpp->size(); i++)
		{
			opt =(OpPoint*)m_poaOpp->at(i);
			if (opt == m_pCurOpp)
			{
				m_poaOpp->removeAt(i);
				delete m_pCurOpp;
				m_pCurOpp = NULL;
				break;
			}
		}
	}
	else
	{
		// kill'em all
		OpPoint* pOpPoint;
		for (i=m_poaOpp->size()-1; i>=0;i--)
		{
			pOpPoint =(OpPoint*)m_poaOpp->at(i);
			if (pOpPoint->m_strFoilName == m_pCurFoil->m_FoilName &&
				pOpPoint->m_strPlrName == m_pCurPolar->m_PlrName)
			{
				m_poaOpp->removeAt(i);
				delete pOpPoint;
			}
		}
	}
	m_pCurOpp = NULL;
}


void QXDirect::FillComboBoxes(bool bEnable)
{
	if(!bEnable)
	{
		m_pctrlCurveColor->setEnabled(false);
		m_pctrlCurveStyle->setEnabled(false);
		m_pctrlCurveWidth->setEnabled(false);
		m_pctrlShowCurve->setEnabled(false);
		m_pctrlShowPoints->setEnabled(false);
	}
	else
	{
		m_pctrlCurveColor->setEnabled(true);
		m_pctrlCurveStyle->setEnabled(true);
		m_pctrlCurveWidth->setEnabled(true);
		m_pctrlShowCurve->setEnabled(true);
		m_pctrlShowPoints->setEnabled(true);
	}
	int LineWidth[5];
	for (int i=0; i<5;i++) LineWidth[i] = m_CurveWidth;
	m_pStyleDelegate->SetLineWidth(LineWidth); // the same selected width for all styles
	m_pStyleDelegate->SetLineColor(m_CurveColor);

	int LineStyle[5];
	for (int i=0; i<5;i++) LineStyle[i] = m_CurveStyle;
	m_pWidthDelegate->SetLineStyle(LineStyle); //the same selected style for all widths
	m_pWidthDelegate->SetLineColor(m_CurveColor);

	m_pctrlCurveStyle->SetLine(m_CurveStyle, m_CurveWidth, m_CurveColor);
	m_pctrlCurveWidth->SetLine(m_CurveStyle, m_CurveWidth, m_CurveColor);

	m_pctrlCurveColor->SetColor(m_CurveColor);
	m_pctrlCurveColor->SetStyle(m_CurveStyle);
	m_pctrlCurveColor->SetWidth(m_CurveWidth);

	m_pctrlCurveStyle->update();
	m_pctrlCurveWidth->update();
	m_pctrlCurveColor->update();

	m_pctrlCurveStyle->setCurrentIndex(m_CurveStyle);
	m_pctrlCurveWidth->setCurrentIndex(m_CurveWidth-1);

}


void QXDirect::FillOppCurve(OpPoint *pOpp, Graph *pGraph, CCurve *pCurve, bool bInviscid)
{
	int j;
	switch(m_OppVar)
	{
		case 0:
		{
			for (j=0; j<pOpp->n; j++)
			{
				if(!bInviscid)
				{
					if(pOpp->m_bVisc) pCurve->AddPoint(pOpp->x[j], pOpp->Cpv[j]);
				}
				else
				{
					pCurve->AddPoint(pOpp->x[j], pOpp->Cpi[j]);
				}
			}
			pGraph->SetYTitle("Cp");
			break;
		}
		case 1:
		{
			for (j=0; j<pOpp->n; j++)
			{
				if(!bInviscid)
				{
					if(pOpp->m_bVisc) pCurve->AddPoint(pOpp->x[j], pOpp->Qv[j]);
				}
				else
				{
					pCurve->AddPoint(pOpp->x[j], pOpp->Qi[j]);
				}
			}
			pGraph->SetYTitle("Q");
			break;
		}
		default:
		{
			for (j=0; j<pOpp->n; j++)
			{
				if(!bInviscid)
				{
					if(pOpp->m_bVisc) pCurve->AddPoint(pOpp->x[j], pOpp->Cpv[j]);
				}
				else{
					pCurve->AddPoint(pOpp->x[j], pOpp->Cpi[j]);
				}
			}
			pGraph->SetYTitle("Cp");
			break;
		}
	}
}



void QXDirect::FillPolarCurve(CCurve *pCurve, CPolar *pPolar, int XVar, int YVar)
{
	int i;
	QList <double> *pX;
	QList <double> *pY;

	pX = (QList <double> *) GetVariable(pPolar, XVar);
	pY = (QList <double> *) GetVariable(pPolar, YVar);
	double fx = 1.0;
	double fy = 1.0;

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
						pCurve->AddPoint(1.0/sqrt((*pX)[i]), 1.0/sqrt((*pY)[i]));
					}
				}
				else
				{
					pCurve->AddPoint(1.0/sqrt((*pX)[i]), (*pY)[i]*fy);
				}
			}
		}
		else{
			if (YVar==12)
			{
				if((*pY)[i]>0.0)
				{
					pCurve->AddPoint((*pX)[i]*fx, 1.0/sqrt((*pY)[i]));
				}
			}
			else
			{
				pCurve->AddPoint((*pX)[i]*fx, (*pY)[i]*fy);
			}
		}
	}
}


OpPoint* QXDirect::GetOpPoint(double Alpha)
{
	OpPoint* pOpPoint;
	for (int i=0; i<m_poaOpp->size(); i++)
	{
		if(!m_pCurPolar) return NULL;
		pOpPoint = (OpPoint*)m_poaOpp->at(i);

		//since alphas are calculated at 1/100th
		if (pOpPoint->m_strFoilName == m_pCurFoil->m_FoilName)
		{
			if (pOpPoint->m_strPlrName == m_pCurPolar->m_PlrName)
			{
				if(m_pCurPolar->m_Type !=4)
				{
					if(fabs(pOpPoint->Alpha - Alpha) <0.01)
					{
						return pOpPoint;
					}
				}
				else
				{
					if(fabs(pOpPoint->Reynolds - Alpha) <0.1)
					{
						return pOpPoint;
					}
				}
			}
		}
	}
	return NULL;// shouldn't ever get here, fortunately
}


void * QXDirect::GetVariable(CPolar *pPolar, int iVar)
{
	// returns a pointer to the variable array defined by its index iVar
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



QGraph* QXDirect::GetGraph(QPoint &pt)
{
	//pt is in client coordinates
	if (m_bPolar)
	{
		if(m_iPlrView == 1) return m_pCurGraph;// no need for change
		if(m_iPlrView == 2)
		{
			if(m_pPolarGraph->IsInDrawRect(pt)){return m_pPolarGraph;}
			if(m_pUserGraph->IsInDrawRect(pt)){return m_pUserGraph;}
			return NULL;
		}
		QRect r;
		m_pCmGraph->GetClientRect(r);
		if(m_pPolarGraph->IsInDrawRect(pt)){return m_pPolarGraph;}
		if(m_pCzGraph->IsInDrawRect(pt)){return m_pCzGraph;}
		if(m_pCmGraph->IsInDrawRect(pt)){return m_pCmGraph;}
		if(m_pTrGraph->IsInDrawRect(pt)){return m_pTrGraph;}
		if(m_pUserGraph->IsInDrawRect(pt)){return m_pUserGraph;}
	}
	else
	{
		if(m_bCpGraph) return m_pCpGraph;
		else           return NULL;
	}
	return NULL;
}

bool QXDirect::InitXFoil()
{
	//loads pFoil in XFoil,
	//calculates normal vectors,
	//and sets results in current foil

	if(!m_pCurFoil) return false;

	CFoil *pFoil = m_pCurFoil;
	int i, k;

	m_pXFoil->m_FoilName = pFoil->m_FoilName;
	for (i =0; i<pFoil->n; i++)
	{
		m_pXFoil->xb[i+1] = pFoil->x[i];
		m_pXFoil->yb[i+1] = pFoil->y[i];
	}

	m_pXFoil->nb = pFoil->n;
//	if(pFoil->m_bTEFlap){
//		m_pXFoil->lflap  = true;
//		m_pXFoil->lbflap = true;
//	}
//	else {
		m_pXFoil->lflap  = false;
		m_pXFoil->lbflap = false;
//	}

	m_pXFoil->ddef = 0.0;
	m_pXFoil->xbf  = 1.0;
	m_pXFoil->ybf  = 0.0;

	m_pXFoil->lscini = false;
	m_pXFoil->lqspec = false;
	m_pXFoil->lvisc  = false;
//	m_pXFoil->acrit      = pFoil->m_NCrit;
//	m_pXFoil->xstrip[1]  = pFoil->m_XTopTr;
//	m_pXFoil->xstrip[2]  = pFoil->m_XBotTr;

	if(m_pCurPolar)
	{
		m_pXFoil->acrit      = m_pCurPolar->m_ACrit;
		m_pXFoil->xstrip[1]  = m_pCurPolar->m_XTop;
		m_pXFoil->xstrip[2]  = m_pCurPolar->m_XBot;
	}

	if(m_pXFoil->Preprocess())
	{
		m_pXFoil->CheckAngles();
		for (k=0; k<m_pXFoil->n;k++)
		{
			pFoil->nx[k] = m_pXFoil->nx[k+1];
			pFoil->ny[k] = m_pXFoil->ny[k+1];
		}
		pFoil->n = m_pXFoil->n;
		return true;
	}
	else
	{
		QMessageBox msgBox;
		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.setWindowTitle("QFLR5");
		msgBox.setText("Unrecognized foil format");
		msgBox.exec();
		return false;
	}
}

void QXDirect::InitXFoil2()
{
	//Sets Analysis parameters in XFoil
	if(!m_pCurPolar) return;

	m_pXFoil->lblini = false;
	m_pXFoil->lipan = false;
	m_bInitBL = !m_pXFoil->lblini;
	m_pctrlInitBL->setChecked(m_bInitBL);

	m_pXFoil->m_bTrace = false;

	m_pXFoil->reinf1 = m_pCurPolar->m_Reynolds;
	if (m_pCurPolar->m_Type == 4) m_pXFoil->alfa = m_pCurPolar->m_ASpec*pi/180.0;

	m_pXFoil->minf1  = m_pCurPolar->m_Mach;

	m_pXFoil->retyp  = m_pCurPolar->m_ReType;
	m_pXFoil->matyp  = m_pCurPolar->m_MaType;

	if(m_pCurPolar)
	{
		m_pXFoil->acrit      = m_pCurPolar->m_ACrit;
		m_pXFoil->xstrip[1]  = m_pCurPolar->m_XTop;
		m_pXFoil->xstrip[2]  = m_pCurPolar->m_XBot;
	}

	m_pXFoil->lalfa = true;
	m_pXFoil->qinf  = 1.0;

	if (m_pCurPolar->m_Mach > 0.000001)
	{
		if(!m_pXFoil->SetMach())
		{
			QString str;
			str = "... Invalid Analysis Settings\nCpCalc: local speed too large \n Compressibility corrections invalid ";
			QMessageBox msgBox;
			msgBox.setStandardButtons(QMessageBox::Ok);
			msgBox.setWindowTitle("QFLR5");
			msgBox.setText("str");
			msgBox.exec();
		}
	}
}

void QXDirect::InsertOpPoint(OpPoint *pNewPoint)
{
	if(!pNewPoint) return;
	MainFrame *pMainFrame = (MainFrame*)m_pMainFrame;
	bool bIsInserted = false;
	OpPoint* pOpPoint;
	CPolar *pPolar = pMainFrame->GetPolar(pNewPoint->m_strFoilName, pNewPoint->m_strPlrName);
//		m_pCurPolar = pPolar;
	if(!pPolar)
	{
		delete pNewPoint;
		return ;
	}

	// first add the OpPoint to the OpPoint Array for the current FoilName
	for (int i=0; i<m_poaOpp->size(); i++)
	{
		pOpPoint = (OpPoint*)m_poaOpp->at(i);
		if (pNewPoint->m_strFoilName.compare(pOpPoint->m_strFoilName)<0)
		{
			//insert point
			m_poaOpp->insert(i, pNewPoint);
			bIsInserted = true;
			i = m_poaOpp->size();// to break
		}
		else if (pNewPoint->m_strFoilName == pOpPoint->m_strFoilName)
		{
			if (pNewPoint->Reynolds < pOpPoint->Reynolds)
			{
				//insert point
				m_poaOpp->insert(i, pNewPoint);
				bIsInserted = true;
				i = m_poaOpp->size();// to break
			}
			else if (fabs(pNewPoint->Reynolds-pOpPoint->Reynolds)<1.0)
			{
				if(fabs(pNewPoint->Alpha - pOpPoint->Alpha)<0.005)
				{
					//replace existing point
					m_poaOpp->removeAt(i);
					delete pOpPoint;
					m_poaOpp->insert(i, pNewPoint);
					bIsInserted = true;
					i = m_poaOpp->size();// to break
				}
				else if (pNewPoint->Alpha < pOpPoint->Alpha)
				{
					//insert point
					m_poaOpp->insert(i, pNewPoint);
					bIsInserted = true;
					i = m_poaOpp->size();// to break
				}
			}
		}
	}
	if (!bIsInserted) 	m_poaOpp->append(pNewPoint);
}


void QXDirect::keyPressEvent(QKeyEvent *event)
{
	switch (event->key())
	{
		case Qt::Key_Return:
			ReadParams();
			if(m_pctrlAnalyze->hasFocus())  OnAnalyze();
			else                            m_pctrlAnalyze->setFocus();
			break;
		case Qt::Key_Tab:
			ReadParams();
			break;
		case Qt::Key_Escape:
			StopAnimate();
			UpdateView();
			break;
		case Qt::Key_1:
			m_iPlrView  = 1;
			m_iPlrGraph = 1;
			CheckButtons();
			UpdateView();
			break;
		case Qt::Key_2:
			m_iPlrView  = 1;
			m_iPlrGraph = 2;
			CheckButtons();
			UpdateView();
			break;
		case Qt::Key_3:
			m_iPlrView  = 1;
			m_iPlrGraph = 3;
			CheckButtons();
			UpdateView();
			break;
		case Qt::Key_4:
			m_iPlrView  = 1;
			m_iPlrGraph = 4;
			CheckButtons();
			UpdateView();
			break;
		case Qt::Key_5:
			m_iPlrView  = 1;
			m_iPlrGraph = 5;
			CheckButtons();
			UpdateView();
			break;
		case Qt::Key_A:
			m_iPlrView = 0;
			CheckButtons();
			UpdateView();
			break;
		case Qt::Key_T:
			m_iPlrView = 2;
			CheckButtons();
			UpdateView();
			break;
		case Qt::Key_V:
			if(m_bPolar && m_pCurGraph) 	OnPolarGraphVariable();
			break;
		case Qt::Key_F5:
			if(!m_bPolar) return;
			OnOpPoints();
			break;
		case Qt::Key_F6:
			if (event->modifiers().testFlag(Qt::ShiftModifier)) OnBatchAnalysis();
			else                                                OnSingleAnalysis();
			break;
		case Qt::Key_F8:
			if(m_bPolar) return;
			OnPolars();
			break;
		default:
			QWidget::keyPressEvent(event);
	}
}


void QXDirect::LoadSettings(QDataStream &ar)
{
	if(ar.status() != QDataStream::Ok)
	{
		QMessageBox msgBox;
		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.setWindowTitle("QFLR5");
		msgBox.setText("archive has an issue");
		msgBox.exec();
		return;
	}

	ar >> m_bAlpha >> m_bStoreOpp >> m_bViscous >> m_bInitBL >> m_bBL >> m_bPressure;
	ar >> m_bPolar >> m_bShowUserGraph >> m_bShowPanels >> m_bType1 >> m_bType2 >> m_bType3 >> m_bType4;
	ar >> m_bAutoInitBL >> m_bFromList >> m_bFromZero >> m_bShowTextOutput >> m_bNeutralLine >> m_bCurOppOnly;
	ar >> m_bShowInviscid >> m_bCpGraph >> m_bSequence;
	ar >> m_crBLColor       >> m_iBLStyle       >> m_iBLWidth;
	ar >> m_crPressureColor >> m_iPressureStyle >> m_iPressureWidth;
	ar >> m_crNeutralColor >> m_iNeutralStyle >> m_iNeutralWidth;

	ar >> m_OppVar >> m_XFoilVar >> m_IterLim ;
	ar >> m_XPolar >> m_YPolar >> m_XCz >> m_YCz >> m_XCm >> m_YCm >> m_XTr >> m_YTr >> m_XUser >> m_YUser;

	if(m_OppVar == 0 || m_OppVar>=2)
	{
		m_pCpGraph->SetYTitle("Cp");
		m_pCpGraph->SetInverted(true);
	}
	else
	{
		m_pCpGraph->SetYTitle("Q");
		m_pCpGraph->SetInverted(false);
	}

	SetGraphTitles(m_pPolarGraph, m_XPolar,m_YPolar);
	SetGraphTitles(m_pCzGraph, m_XCz, m_YCz);
	SetGraphTitles(m_pCmGraph, m_XCm, m_YCm);
	SetGraphTitles(m_pTrGraph, m_XTr, m_YTr);
	SetGraphTitles(m_pUserGraph, m_XUser, m_YUser);

	ar >> m_iPlrGraph >> m_iPlrView;
	ar >> m_Alpha >> m_AlphaMax >> m_AlphaDelta;
	ar >> m_Cl >> m_ClMax >> m_ClDelta;
	ar >> m_Reynolds >> m_ReynoldsMax >> m_ReynoldsDelta;
	ar >> m_NCrit >> m_XTopTr >> m_XBotTr;
	ar >> m_Mach >> m_MachDef;
	ar >> m_Type;
	ar >> m_pXFoil->vaccel >> m_IterLim >> m_bAutoInitBL ;
	ar >> m_NRe;
	for (int i=0; i<m_NRe; i++) ar >> m_ReList[i] >> m_MachList[i] >> m_NCritList[i];


	m_pPolarGraph->Serialize(ar, false);
	m_pCmGraph->Serialize(ar, false);
	m_pCzGraph->Serialize(ar, false);
	m_pTrGraph->Serialize(ar, false);
	m_pUserGraph->Serialize(ar, false);

	m_pCpGraph->Serialize(ar, false);

}



void QXDirect::mouseMoveEvent(QMouseEvent *event)
{
	QPoint pt(event->x(), event->y()); //client coordinates 
	m_pCurGraph = GetGraph(pt);
	if(!hasFocus()) setFocus();//to catch keyboard input;

	if ((event->buttons() & Qt::LeftButton) && m_bTrans)
	{
		if(m_pCurGraph && m_bTransGraph)
		{
			// we translate the curves inside the graph
			double xu, yu;
			m_pCurGraph->SetAuto(false);
			double x1 =  m_pCurGraph->ClientTox(m_PointDown.x()) ;
			double y1 =  m_pCurGraph->ClientToy(m_PointDown.y()) ;

			xu = m_pCurGraph->ClientTox(pt.x());
			yu = m_pCurGraph->ClientToy(pt.y());

			double xmin = m_pCurGraph->GetXMin() - xu+x1;
			double xmax = m_pCurGraph->GetXMax() - xu+x1;
			double ymin = m_pCurGraph->GetYMin() - yu+y1;
			double ymax = m_pCurGraph->GetYMax() - yu+y1;

			m_pCurGraph->SetWindow(xmin, xmax, ymin, ymax);

			m_PointDown.setX(pt.x());
			m_PointDown.setY(pt.y());
			if(!m_bAnimate) UpdateView();
		}
		else if (m_pCurFoil  && !m_bPolar)
		{
			// we translate the airfoil
			
			m_FoilOffset.rx() += pt.x()-m_PointDown.x();
			m_FoilOffset.ry() += pt.y()-m_PointDown.y();
			m_PointDown.setX(pt.x());
			m_PointDown.setY(pt.y());
			if(!m_bAnimate) UpdateView();
		}
	}

	else if (m_pCurFoil &&
			((event->buttons() & Qt::MidButton) || event->modifiers().testFlag(Qt::AltModifier)))
	{
		// we zoom the graph or the foil		
		if(m_pCurGraph && m_pCurGraph->IsInDrawRect(pt) && m_bCpGraph)
		{ 
			//zoom graph

/*			SHORT shX = GetKeyState('X');
			SHORT shY = GetKeyState('Y');

			if (shX & 0x8000)
			{
				//zoom x scale
				m_pCurGraph->SetAutoX(false);
				m_pCurGraph->SetAutoX(false);
				if(pt.y()-m_PointDown().y<0) m_pCurGraph->Scalex(1.04);
				else                         m_pCurGraph->Scalex(1.0/1.04);
			}
			else if(shY & 0x8000)
			{
				//zoom y scale
				m_pCurGraph->SetAutoY(false);
				m_pCurGraph->SetAutoY(false);
				if(pt.y()-m_PointDown().y<0) m_pCurGraph->Scaley(1.04);
				else                         m_pCurGraph->Scaley(1.0/1.04);
			}
			else*/
			{
				//zoom both
				m_pCurGraph->SetAuto(false);
				if(pt.y()-m_PointDown.y()<0) m_pCurGraph->Scale(1.06);
				else                         m_pCurGraph->Scale(1.0/1.06);
			}

			if(!m_bAnimate) UpdateView();
		}
		else if(m_pCurFoil && !m_bPolar)
		{
			//zoom the foil
			double scale = m_fFoilScale;

			if(pt.y()-m_PointDown.y()<0) m_fFoilScale *= 1.06;
			else                         m_fFoilScale /= 1.06;

			int a = (int)((m_rCltRect.right()+m_rCltRect.left())/2);

			m_FoilOffset.rx() = a + (int)((m_FoilOffset.x()-a)/scale*m_fFoilScale);

			if(!m_bAnimate) UpdateView();

		}
		m_PointDown = pt;
	}
}




void QXDirect::mousePressEvent(QMouseEvent *event)
{
	QPoint pt(event->x(), event->y()); //client coordinates

	m_pCurGraph = GetGraph(pt);

	TwoDWidget *p2DWidget = (TwoDWidget*)m_p2DWidget;

	if(event->buttons() & Qt::LeftButton)
	{
		if (m_pCurFoil || (m_pCurGraph && m_pCurGraph->IsInDrawRect(pt) && m_bCpGraph))
		{
			m_PointDown.setX(pt.x());
			m_PointDown.setY(pt.y());

			m_bTrans = true;
			p2DWidget->setCursor(m_hcMove);
			if (m_pCurGraph && m_pCurGraph->IsInDrawRect(pt)) m_bTransGraph = true;
			else                                              m_bTransGraph = false;
			if(!m_bAnimate) UpdateView();
		}
	}
}

void QXDirect::mouseReleaseEvent(QMouseEvent *event)
{
	TwoDWidget *p2DWidget = (TwoDWidget*)m_p2DWidget;
	QPoint pt(event->x(), event->y()); //client coordinates

	m_bTrans = false;
	p2DWidget->setCursor(m_hcArrow);
}




void QXDirect::OnAllPolarGraphs()
{
	m_iPlrView  = 0;
	m_bPolar = true;
	CheckButtons();
	UpdateView();
}

void QXDirect::OnAllPolarGraphsSetting()
{
	GraphDlg dlg;
	QGraph graph;
	graph.CopySettings(m_pPolarGraph);
	dlg.m_pMemGraph = m_pPolarGraph;
	dlg.m_pGraph = &graph;
	dlg.SetParams();

	if(dlg.exec() == QDialog::Accepted)
	{
		m_pPolarGraph->CopySettings(&graph);
		m_pCmGraph->CopySettings(&graph);
		m_pCzGraph->CopySettings(&graph);
		m_pTrGraph->CopySettings(&graph);
		m_pUserGraph->CopySettings(&graph);
	}
	UpdateView();
}

void QXDirect::OnAlphaMinChanged()
{
	QString str;
/*	QString str1;
	str = m_pctrlAlphaMin->text();
	str1.setNum(m_Alpha,'f',6);
	QMessageBox msgBox;
	msgBox.setStandardButtons(QMessageBox::Ok);
	msgBox.setWindowTitle("QFLR5");
	msgBox.setText("text="+str+"\nvalue="+str1);
	msgBox.exec();*/
	if(!m_pCurPolar) return;
	if(m_pCurPolar->m_Type !=4)
	{
		if(m_bAlpha)
		{
			m_Alpha      = m_pctrlAlphaMin->GetValue();
			m_pctrlAlphaMin->clear();
			m_pctrlAlphaMin->insert(str.setNum(m_Alpha,'f',2));
		}
		else
		{
			m_Cl      = m_pctrlAlphaMin->GetValue();
			m_pctrlAlphaMin->clear();
			m_pctrlAlphaMin->insert(str.setNum(m_Cl,'f',2));
		}
	}
	else
	{
			m_Reynolds      = m_pctrlAlphaMin->GetValue();
			m_pctrlAlphaMin->clear();
			m_pctrlAlphaMin->insert(str.setNum(m_Reynolds,'f',0));
	}
}


void QXDirect::OnAlphaMaxChanged()
{
	if(!m_pCurPolar) return;
	QString str;
	if(m_pCurPolar->m_Type !=4)
	{
		if(m_bAlpha)
		{
			m_AlphaMax      = m_pctrlAlphaMax->GetValue();
			m_pctrlAlphaMax->clear();
			m_pctrlAlphaMax->insert(str.setNum(m_AlphaMax,'f',2));
		}
		else
		{
			m_ClMax    = m_pctrlAlphaMax->GetValue();
			m_pctrlAlphaMax->clear();
			m_pctrlAlphaMax->insert(str.setNum(m_ClMax,'f',2));
		}
	}
	else
	{
			m_ReynoldsMax      = m_pctrlAlphaMax->GetValue();
			m_pctrlAlphaMax->clear();
			m_pctrlAlphaMax->insert(str.setNum(m_ReynoldsMax,'f',0));
	}
}


void QXDirect::OnAnimate(bool bChecked)
{
	if(!m_pCurFoil || !m_pCurPolar)
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
				pOpPoint->m_strPlrName  == m_pCurPolar->m_PlrName &&
				pOpPoint->m_strFoilName == m_pCurFoil->m_FoilName)
			{
					if(fabs(m_pCurOpp->Alpha - pOpPoint->Alpha)<0.0001)
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
		if(pOpPoint) SetOpp(pOpPoint->Alpha);
//		UpdateView();
		return;
	}
}



void QXDirect::OnAnimateSingle()
{
	//KickIdle
	QString strong;
	int size = (int)m_poaOpp->size();
	OpPoint* pOpPoint;

	if(size<=1) return;

	//find the current position to display

	if(m_bAnimatePlus)
	{
		m_posAnimate++;
		if (m_posAnimate >= size)
		{
			m_posAnimate = size-2;
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
	if(!pOpPoint) return;

	if (pOpPoint->m_strPlrName  == m_pCurPolar->m_PlrName &&
		pOpPoint->m_strFoilName == m_pCurFoil->m_FoilName)
	{
			CreateOppCurves(pOpPoint);
			m_pCurOpp = pOpPoint;
			UpdateView();
	}
}


void QXDirect::OnAnimateSpeed(int val)
{
	if(m_pAnimateTimer->isActive())
	{
		m_pAnimateTimer->setInterval(800-val);
	}
}


void QXDirect::OnAnalyze()
{
	if(!m_pCurFoil || !m_pCurPolar) return;
	if(m_bViscous) m_pXFoil->lvisc = true;

	ReadParams();

	XFoilAnalysisDlg m_XFDlg;

	m_XFDlg.SetAlpha(m_Alpha, m_AlphaMax, m_AlphaDelta);
	m_XFDlg.SetCl(m_Cl, m_ClMax, m_ClDelta);
	m_XFDlg.SetRe(m_Reynolds, m_ReynoldsMax, m_ReynoldsDelta);

	m_XFDlg.m_pMainFrame  = m_pMainFrame;
	m_XFDlg.m_pXDirect = this;
	m_XFDlg.m_bSequence = m_bSequence;
	m_XFDlg.m_bAlpha = m_bAlpha;
	m_XFDlg.m_bType4 = (m_pCurPolar->m_Type==4);

	m_XFDlg.m_FoilName = m_pCurFoil->m_FoilName;
	m_XFDlg.m_IterLim = m_IterLim;
	m_XFDlg.m_pXFoil = m_pXFoil;
	m_XFDlg.setMinimumSize(600,400);
	m_XFDlg.InitDialog();
	m_XFDlg.setModal(true);
	m_XFDlg.show();
	m_XFDlg.repaint();
	m_XFDlg.StartAnalysis();
// and update window

	m_bInitBL = !m_pXFoil->lblini;
	m_pctrlInitBL->setChecked(m_bInitBL);;

	m_pCurGraph = m_pCpGraph;

	MainFrame *pMainFrame = (MainFrame*)m_pMainFrame;
	pMainFrame->UpdateOpps();

	SetOpp();

	if(m_bPolar) CreatePolarCurves();
	UpdateView();
}


void QXDirect::OnBatchAnalysis()
{
	MainFrame* pMainFrame = (MainFrame*)m_pMainFrame;
	if(!m_pCurFoil) 		return;

	BatchDlg BDlg;
	BDlg.m_pXDirect = this;
	BDlg.m_pMainFrame    = m_pMainFrame;
	BDlg.m_pXFoil    = m_pXFoil;
	BDlg.m_pCurFoil  = m_pCurFoil;
	BDlg.m_Mach      = 0.0;
	BDlg.m_ReMin     = m_Reynolds;
	BDlg.m_ReMax     = m_ReynoldsMax;
	BDlg.m_ReInc     = m_ReynoldsDelta;
	BDlg.m_Type      = 1;
	BDlg.m_IterLim   = m_IterLim;
	BDlg.m_FoilName  = m_pCurFoil->m_FoilName;
	BDlg.m_bAlpha    = true;
	BDlg.m_SpMin     = m_Alpha;
	BDlg.m_SpMax     = m_AlphaMax;
	BDlg.m_SpInc     = m_AlphaDelta;
	BDlg.m_AlphaMin  = m_Alpha;
	BDlg.m_AlphaMax  = m_AlphaMax;
	BDlg.m_AlphaInc  = m_AlphaDelta;
	BDlg.m_ClMin     = m_Cl;
	BDlg.m_ClMax     = m_ClMax;
	BDlg.m_ClInc     = m_ClDelta;
	BDlg.m_NCrit     = m_NCrit;
	BDlg.m_XTopTr    = m_XTopTr;
	BDlg.m_XBotTr    = m_XBotTr;
	BDlg.m_ReList    = m_ReList;
	BDlg.m_MachList  = m_MachList;
	BDlg.m_NCritList = m_NCritList;
	BDlg.m_NRe       = m_NRe;
	BDlg.m_bFromList = m_bFromList;
	BDlg.m_bFromZero = m_bFromZero;
	BDlg.InitDialog();

	BDlg.exec();
	pMainFrame->SetSaveState(false);

	m_Reynolds         = BDlg.m_ReMin;
	m_ReynoldsMax      = BDlg.m_ReMax;
	m_ReynoldsDelta    = BDlg.m_ReInc;
	m_Alpha            = BDlg.m_AlphaMin;
	m_AlphaMax         = BDlg.m_AlphaMax;
	m_AlphaDelta       = BDlg.m_AlphaInc;
	m_Cl               = BDlg.m_ClMin;
	m_ClMax            = BDlg.m_ClMax;
	m_ClDelta          = BDlg.m_ClInc;
	m_MachDef          = BDlg.m_Mach;
	m_Type             = BDlg.m_Type;
	m_NCrit            = BDlg.m_NCrit;
	m_XTopTr           = BDlg.m_XTopTr;
	m_XBotTr           = BDlg.m_XBotTr;
	m_NRe              = BDlg.m_NRe;
	m_bAlpha           = BDlg.m_bAlpha;
	m_bFromList        = BDlg.m_bFromList;
	m_bFromZero        = BDlg.m_bFromZero;

	pMainFrame->UpdatePolars();

	m_pCurOpp = NULL;

	CreatePolarCurves();

	SetPolar();

	if(m_pCurPolar)
	{
/*		if(m_pCurPolar->m_Type ==4)
		{
			pFrame->m_OperDlgBar.CheckRadioButton(IDC_PARAM1,IDC_PARAM3,IDC_PARAM3);
		}
		else{
			if(m_bAlpha) pFrame->m_OperDlgBar.CheckRadioButton(IDC_PARAM1,IDC_PARAM3,IDC_PARAM1);
			else         pFrame->m_OperDlgBar.CheckRadioButton(IDC_PARAM1,IDC_PARAM3,IDC_PARAM2);
		}*/
	}
	UpdateView();
}

void QXDirect::OnCadd()
{
	StopAnimate();
	MainFrame* pMainFrame = (MainFrame*)m_pMainFrame;
	if(!m_pCurFoil)		return;

	void* ptr = m_pCurOpp;
	m_pCurOpp = NULL;
	CreateOppCurves();

	bool bPressure = m_bPressure;
	bool bBL       = m_bBL;
	bool bState = m_bShowPanels;
	m_bPressure = false;
	m_bBL       = false;
	OnOpPoints();

	CAddDlg Adlg;
	Adlg.m_pBufferFoil = &m_BufferFoil;
	Adlg.m_pMemFoil    = m_pCurFoil;
	Adlg.m_pXDirect    = this;
	Adlg.m_pAFoil      = NULL;
	Adlg.InitDialog();

	m_bShowPanels = true;
	UpdateView();

	if(QDialog::Accepted == Adlg.exec())
	{
		CFoil *pNewFoil = new CFoil();
		pNewFoil->CopyFoil(&m_BufferFoil);
		pNewFoil->m_FoilColor  = pMainFrame->GetColor(0);
		pNewFoil->m_nFoilStyle = 1;
		pNewFoil->m_nFoilWidth = 1;
		pNewFoil->m_bPoints = false;
		m_pCurOpp = (OpPoint*)ptr;
		pMainFrame->SetModFoil(pNewFoil);
		pMainFrame->UpdateFoils();
	}
	else
	{
		m_pCurOpp = (OpPoint*)ptr;
		SetBufferFoil();
		InitXFoil();
	}

	m_bShowPanels = bState;

	m_bPressure = bPressure;
	m_bBL       = bBL;
	UpdateView();
}

void QXDirect::OnCfPlot()
{
	if(!m_pXFoil->lvconv) return;
	int i;
	double x[IVX][3],y[IVX][3];
	int nside1, nside2, ibl;

	m_OppVar = 2;
	m_XFoilVar = 8;
	m_pCpGraph->DeleteCurves();
	m_pCpGraph->ResetLimits();
	m_pCpGraph->SetAuto(true);
	m_pCpGraph->SetInverted(false);
	m_pCpGraph->SetYTitle("Cf");
	CCurve * pTopCurve = m_pCpGraph->AddCurve();
	CCurve * pBotCurve = m_pCpGraph->AddCurve();
	pTopCurve->SetTitle("Top");
	pBotCurve->SetTitle("Bot");

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
		pTopCurve->AddPoint(x[i][1], y[i][1]);
	}
	for (i=2; i<=nside2-1; i++)
	{
		pBotCurve->AddPoint(x[i][2], y[i][2]);
	}
	m_pCpGraph->SetXScale();
	SetFoilScale();
	CheckButtons();
	UpdateView();
}


void QXDirect::OnCdPlot()
{
	if(!m_pXFoil->lvconv) return;
	double x[IVX][3],y[IVX][3];
	int nside1, nside2, ibl;
	int i;

	m_OppVar = 2;
	m_XFoilVar = 7;
	m_pCpGraph->DeleteCurves();
	m_pCpGraph->ResetLimits();
	m_pCpGraph->SetAuto(true);
	m_pCpGraph->SetInverted(false);
	m_pCpGraph->SetYTitle("Cd'");
	CCurve * pTopCurve = m_pCpGraph->AddCurve();
	CCurve * pBotCurve = m_pCpGraph->AddCurve();
	pTopCurve->SetTitle("Top");
	pBotCurve->SetTitle("Bot");

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
		pTopCurve->AddPoint(x[i][1], y[i][1]);
	}
	for (i=2; i<=nside2-1; i++)
	{
		pBotCurve->AddPoint(x[i][2], y[i][2]);
	}
	m_pCpGraph->SetXScale();
	SetFoilScale();
	CheckButtons();
	UpdateView();
}


void QXDirect::OnCouplePolarGraphs()
{
	m_iPlrView  = 2;
	m_bPolar = true;
	CheckButtons();
	UpdateView();
}



void QXDirect::OnCpGraph()
{
	m_bPolar = false;
	if(m_OppVar!=0)
	{
		m_pCpGraph->ResetLimits();
		m_pCpGraph->SetAuto(true);
	}
	m_OppVar = 0;
	m_pCpGraph->SetInverted(true);
	CreateOppCurves();
	m_pCpGraph->SetYTitle("Q");

	CheckButtons();
	m_pCpGraph->SetXScale();
	SetFoilScale();
	UpdateView();
}

void QXDirect::OnCpGraphSettings()
{
	GraphDlg dlg;
	QGraph graph;
	graph.CopySettings(m_pCpGraph);
	dlg.m_pMemGraph = m_pCurGraph;
	dlg.m_pGraph = &graph;
	dlg.SetParams();

	if(dlg.exec() == QDialog::Accepted)
	{
		m_pCpGraph->CopySettings(&graph);
	}
	UpdateView();
}


void QXDirect::OnCpi()
{
	m_bShowInviscid = !m_bShowInviscid;

	CreateOppCurves();
	CheckButtons();
	UpdateView();
}

void QXDirect::OnCtPlot()
{
	if(!m_pXFoil->lvconv) return;
	int i;

	m_OppVar = 2;
	m_XFoilVar=1;
	m_pCpGraph->DeleteCurves();
	m_pCpGraph->ResetLimits();
	m_pCpGraph->SetAuto(true);
	m_pCpGraph->SetInverted(false);
	m_pCpGraph->SetYTitle("Max Shear");
	CCurve * pCurve0 = m_pCpGraph->AddCurve();
	CCurve * pCurve1 = m_pCpGraph->AddCurve();
	CCurve * pCurve2 = m_pCpGraph->AddCurve();
	CCurve * pCurve3 = m_pCpGraph->AddCurve();
	pCurve0->SetTitle("Top Shear");
	pCurve1->SetTitle("Top Shear eq");
	pCurve2->SetTitle("Bot Shear");
	pCurve3->SetTitle("Bot Shear eq");

	double x[IVX][3];
	int nside1, nside2;

	m_pXFoil->CreateXBL(x, nside1, nside2);

	int it1 = m_pXFoil->itran[1];
	int it2 = m_pXFoil->itran[2];

	for (i=it1; i<=nside1-1; i++)	pCurve0->AddPoint(x[i][1], m_pXFoil->ctau[i][1]);
	for (i=2; i<=nside1-1; i++)		pCurve1->AddPoint(x[i][1], m_pXFoil->ctq[i][1]);

	for (i=it2; i<=nside2-1; i++)	pCurve2->AddPoint(x[i][2], m_pXFoil->ctau[i][2]);
	for (i=2; i<=nside2-1; i++)		pCurve3->AddPoint(x[i][2], m_pXFoil->ctq[i][2]);

	m_pCpGraph->SetXScale();
	SetFoilScale();
	CheckButtons();
	UpdateView();

}

void QXDirect::OnCurOppOnly()
{
	m_bCurOppOnly = !m_bCurOppOnly;
	if(m_pCurOpp) m_pCurOpp->m_bIsVisible = true;
	CreateOppCurves();
	SetAnalysisParams();
	UpdateView();
}


void QXDirect::OnCurveColor()
{
	bool bOK;
	QRgb rgb = m_CurveColor.rgba();
	rgb = QColorDialog::getRgba(rgb, &bOK);
	m_CurveColor = QColor::fromRgba(rgb);

	FillComboBoxes();
	
	UpdateCurve();
}

void QXDirect::OnCurveStyle(int index)
{
	m_CurveStyle = index;
	FillComboBoxes();
	UpdateCurve();
}

void QXDirect::OnCurveWidth(int index)
{
	m_CurveWidth = index+1;
	FillComboBoxes();
	UpdateCurve();
}

void QXDirect::OnDeleteCurFoil()
{
	MainFrame* pMainFrame = (MainFrame*)m_pMainFrame;
	DeleteFoil(true);
	pMainFrame->UpdateFoils();

	if(m_bPolar) CreatePolarCurves();
	else         CreateOppCurves();
	pMainFrame->SetSaveState(false);

	UpdateView();

}

void QXDirect::OnDelCurOpp()
{
	OpPoint* pOpPoint = m_pCurOpp;
	StopAnimate();

	if (!pOpPoint) return;
	QString strong,str;
	strong = "Are you sure you want to delete the Operating Point \n";
	if(m_pCurPolar->m_Type !=4) str = QString("Alpha = %1").arg(pOpPoint->Alpha,0,'f',2);
	else str = QString("Reynolds = %1").arg(pOpPoint->Reynolds,0,'f',0);
	strong += str;
	strong += "  ?";

	if (QMessageBox::Yes == QMessageBox::question(window(), "QFLR5", strong,
		QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel))
	{
		MainFrame* pMainFrame = (MainFrame*)m_pMainFrame;
		pMainFrame->RemoveOpPoint(true);
		pMainFrame->UpdateOpps();
		SetOpp();
		UpdateView();
	}
}


void QXDirect::OnDeletePolarOpps()
{
	if(!m_pCurFoil || !m_pCurPolar) return;

	MainFrame* pMainFrame = (MainFrame*)m_pMainFrame;
	OpPoint *pOpp;

	for(int i=m_poaOpp->size()-1; i>=0; i--)
	{
		pOpp = (OpPoint*)m_poaOpp->at(i);
		if(pOpp->m_strFoilName==m_pCurFoil->m_FoilName && pOpp->m_strPlrName==m_pCurPolar->m_PlrName)
			m_poaOpp->removeAt(i);
			if(m_pCurOpp == pOpp) m_pCurOpp=NULL;
			delete pOpp;
	}
	pMainFrame->SetSaveState(false);
	pMainFrame->UpdateOpps();
	if(!m_bPolar) CreateOppCurves();
	SetCurveParams();
	UpdateView();
}


void QXDirect::OnShowPolarOpps()
{
	if(!m_pCurFoil || !m_pCurPolar) return;

	MainFrame* pMainFrame = (MainFrame*)m_pMainFrame;
	OpPoint *pOpp;

	for(int i=0; i<m_poaOpp->size(); i++)
	{
		pOpp = (OpPoint*)m_poaOpp->at(i);
		if(pOpp->m_strFoilName==m_pCurFoil->m_FoilName && pOpp->m_strPlrName==m_pCurPolar->m_PlrName)
			pOpp->m_bIsVisible = true;
	}
	pMainFrame->SetSaveState(false);
	if(!m_bPolar) CreateOppCurves();
	SetCurveParams();
	UpdateView();
}


void QXDirect::OnHidePolarOpps()
{
	if(!m_pCurFoil || !m_pCurPolar) return;

	MainFrame* pMainFrame = (MainFrame*)m_pMainFrame;
	OpPoint *pOpp;

	for(int i=0; i<m_poaOpp->size(); i++)
	{
		pOpp = (OpPoint*)m_poaOpp->at(i);
		if(pOpp->m_strFoilName==m_pCurFoil->m_FoilName && pOpp->m_strPlrName==m_pCurPolar->m_PlrName)
			pOpp->m_bIsVisible = false;
	}
	pMainFrame->SetSaveState(false);
	if(!m_bPolar) CreateOppCurves();
	SetCurveParams();
	UpdateView();
}



void QXDirect::OnDeleteFoilOpps()
{
	if(!m_pCurFoil || !m_pCurPolar) return;

	MainFrame* pMainFrame = (MainFrame*)m_pMainFrame;
	OpPoint *pOpp;

	for(int i=m_poaOpp->size()-1; i>=0; i--)
	{
		pOpp = (OpPoint*)m_poaOpp->at(i);
		if(pOpp->m_strFoilName==m_pCurFoil->m_FoilName)
			m_poaOpp->removeAt(i);
			if(m_pCurOpp == pOpp) m_pCurOpp=NULL;
			delete pOpp;
	}
	pMainFrame->SetSaveState(false);
	pMainFrame->UpdateOpps();
	if(!m_bPolar) CreateOppCurves();
	SetCurveParams();
	UpdateView();
}


void QXDirect::OnShowFoilOpps()
{
	if(!m_pCurFoil || !m_pCurPolar) return;

	MainFrame* pMainFrame = (MainFrame*)m_pMainFrame;
	OpPoint *pOpp;

	for(int i=0; i<m_poaOpp->size(); i++)
	{
		pOpp = (OpPoint*)m_poaOpp->at(i);
		if(pOpp->m_strFoilName==m_pCurFoil->m_FoilName)
			pOpp->m_bIsVisible = true;
	}
	pMainFrame->SetSaveState(false);
	if(!m_bPolar) CreateOppCurves();
	SetCurveParams();
	UpdateView();
}


void QXDirect::OnHideFoilOpps()
{
	if(!m_pCurFoil || !m_pCurPolar) return;

	MainFrame* pMainFrame = (MainFrame*)m_pMainFrame;
	OpPoint *pOpp;

	for(int i=0; i<m_poaOpp->size(); i++)
	{
		pOpp = (OpPoint*)m_poaOpp->at(i);
		if(pOpp->m_strFoilName==m_pCurFoil->m_FoilName)
			pOpp->m_bIsVisible = false;
	}
	pMainFrame->SetSaveState(false);
	if(!m_bPolar) CreateOppCurves();
	SetCurveParams();
	UpdateView();
}


void QXDirect::OnDeleteFoilPolars()
{
	if(!m_pCurFoil) return;
	MainFrame * pMainFrame = (MainFrame*) m_pMainFrame;
	int l;
	OpPoint *pOpPoint;
	StopAnimate();

	QString strong;

	strong = "Are you sure you want to delete polars and OpPoints\n";
	strong +="associated to "+m_pCurFoil->m_FoilName  + " ?";
	if (QMessageBox::Yes == QMessageBox::question(window(), "QFLR5", strong,
		QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel))

	{
		// start by removing all OpPoints
		for (l=m_poaOpp->size()-1; l>=0; l--)
		{
			pOpPoint = (OpPoint*)m_poaOpp->at(l);
			if (pOpPoint->m_strFoilName == m_pCurFoil->m_FoilName)
			{
				m_poaOpp->removeAt(l);
				delete pOpPoint;
			}
		}
		// then remove CPolar and update views
		CPolar* pPolar;
		for (l=m_poaPolar->size()-1; l>=0; l--)
		{
			pPolar = (CPolar*)m_poaPolar->at(l);
			if (pPolar->m_FoilName == m_pCurFoil->m_FoilName)
			{
				m_poaPolar->removeAt(l);
				delete pPolar;
			}
		}
		m_pCurOpp = NULL;

	}
	m_pCurPolar = NULL;
	SetPolar();
	pMainFrame->UpdatePolars();
	pMainFrame->SetSaveState(false);

	SetAnalysisParams();
	UpdateView();
}

void QXDirect::OnDeltaAlphaChanged()
{
	if(!m_pCurPolar) return;
	QString str;
	if(m_pCurPolar->m_Type !=4)
	{
		if(m_bAlpha)
		{
			m_AlphaDelta      = m_pctrlAlphaDelta->GetValue();
		}
		else
		{
			m_ClDelta    = m_pctrlAlphaDelta->GetValue();
		}
	}
	else
	{
			m_ReynoldsDelta      = m_pctrlAlphaDelta->GetValue();
	}
}

void QXDirect::OnDerotateFoil()
{
	if(!m_pCurFoil) return;
	QString str;
	MainFrame* pMainFrame = (MainFrame*)m_pMainFrame;
	double angle = m_pCurFoil->DeRotate();
	SetBufferFoil();
	pMainFrame->SetSaveState(false);
	str = QString("The foil has been de-rotated by %1 degrees").arg(angle,6,'f',3);
	pMainFrame->statusBar()->showMessage(str);

	UpdateView();

}

void QXDirect::OnDtPlot()
{
	if(!m_pXFoil->lvconv) return;
	int i;

	m_OppVar = 2;
	m_XFoilVar=3;
	m_pCpGraph->DeleteCurves();
	m_pCpGraph->ResetLimits();
	m_pCpGraph->SetAuto(true);
	m_pCpGraph->SetInverted(false);
	m_pCpGraph->SetYTitle(" ");


	double x[IVX][3];
	int nside1, nside2;

	CCurve * pCurve1 = m_pCpGraph->AddCurve();
	CCurve * pCurve2 = m_pCpGraph->AddCurve();

	pCurve1->SetTitle("D*");
	pCurve2->SetTitle("Theta");
	m_pXFoil->CreateXBL(x, nside1, nside2);

	for (i=2; i<nside1; i++){
		pCurve1->AddPoint(x[i][1], m_pXFoil->dstr[i][1]);
		pCurve2->AddPoint(x[i][1], m_pXFoil->thet[i][1]);
	}

	m_pCpGraph->SetXScale();
	SetFoilScale();
	CheckButtons();
	UpdateView();
}

void QXDirect::OnDbPlot()
{
	if(!m_pXFoil->lvconv) return;
	int i;

	m_OppVar = 2;
	m_XFoilVar = 2;
	m_pCpGraph->DeleteCurves();
	m_pCpGraph->ResetLimits();
	m_pCpGraph->SetAuto(true);
	m_pCpGraph->SetInverted(false);
	m_pCpGraph->SetYTitle(" ");


	double x[IVX][3];
	int nside1, nside2;

	CCurve * pCurve1 = m_pCpGraph->AddCurve();
	CCurve * pCurve2 = m_pCpGraph->AddCurve();

	pCurve1->SetTitle("D*");
	pCurve2->SetTitle("Theta");
	m_pXFoil->CreateXBL(x, nside1, nside2);

	for (i=2; i<nside2; i++)
	{
		pCurve1->AddPoint(x[i][2], m_pXFoil->dstr[i][2]);
		pCurve2->AddPoint(x[i][2], m_pXFoil->thet[i][2]);
	}

	m_pCpGraph->SetXScale();
	SetFoilScale();
	CheckButtons();
	UpdateView();
}


void QXDirect::OnDuplicateFoil()
{
	if(!m_pCurFoil) return;
	MainFrame* pMainFrame = (MainFrame*)m_pMainFrame;
	CFoil *pNewFoil = new CFoil();
	pNewFoil->CopyFoil(m_pCurFoil);
	pNewFoil->InitFoil();

	m_BufferFoil.CopyFoil(pNewFoil);
	if(pMainFrame->SetModFoil(pNewFoil))
	{
		SetFoil();
		pMainFrame->UpdateFoils();
		UpdateView();
		pMainFrame->SetSaveState(false);
	}
	else delete pNewFoil;
}


void QXDirect::OnEditCurPolar()
{
	if (!m_pCurPolar) return;
	MainFrame* pMainFrame = (MainFrame*)m_pMainFrame;

	CPolar MemPolar;
	MemPolar.Copy(m_pCurPolar);

	EditPlrDlg dlg;
	dlg.m_pPolar = m_pCurPolar;
	dlg.m_pXDirect = this;
	dlg.InitDialog();

	bool bPoints = m_pCurPolar->m_bShowPoints;
	m_pCurPolar->m_bShowPoints = true;

	CreatePolarCurves();
	UpdateView();

	if(dlg.exec() == QDialog::Accepted)
	{
		pMainFrame->SetSaveState(false);
	}
	else
	{
		m_pCurPolar->Copy(&MemPolar);
	}
	m_pCurPolar->m_bShowPoints = bPoints;
	CreatePolarCurves();
	UpdateView();
}

void QXDirect::OnExportCurXFoilResults()
{
	if(!m_pXFoil->lvconv) return;
	if(!m_pCurFoil)		  return;

	MainFrame *pMainFrame = (MainFrame*)m_pMainFrame;
	QString FileName,  OutString, strong;

	double x[IVX][3],Hk[IVX][3],UeVinf[IVX][3], Cf[IVX][3], Cd[IVX][3], AA0[IVX][3];
	double RTheta[IVX][3], DStar[IVX][3], Theta[IVX][3];
	double uei;
	double que = 0.5*m_pXFoil->qinf*m_pXFoil->qinf;
	double qrf = m_pXFoil->qinf;
	int nside1, nside2, ibl;
	int type = 1;

	FileName = m_pCurFoil->m_FoilName;
	FileName.replace("/", " ");

	FileName = QFileDialog::getSaveFileName(this, "Export Current XFoil Results",
											pMainFrame->m_LastDirName,
											"Text File (*.dat *.csv)");

	int pos = FileName.lastIndexOf("/");
	if(pos>0) pMainFrame->m_LastDirName = FileName.left(pos);

	pos  = FileName.lastIndexOf(".csv");
	if(pos>0) type = 2;

	QFile DestFile(FileName);

	if (!DestFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

	QTextStream out(&DestFile);

	out << (pMainFrame->m_VersionName);
	out << ("\n");
	strong = m_pXFoil->m_FoilName+ "\n";
	out << (strong);

	if(type==1)
		strong = QString("Alpha = %1,  Re = %2,  Ma= %3,  ACrit=%4 \n\n")
						 .arg(m_pXFoil->alfa*180./pi, 5, 'f',1)
						 .arg(m_pXFoil->reinf1, 8, 'f',0)
						 .arg(m_pXFoil->minf1, 6, 'f',4)
						 .arg(m_pXFoil->acrit, 4, 'f',1);
	else
		strong = QString("Alpha =, %1,Re =, %3,Ma=, %3,ACrit =,%4 \n\n")
						 .arg(m_pXFoil->alfa*180./pi, 5, 'f',1)
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

	out << ("\nTop Side\n");
	if(type==1) OutString = QString("    x         Hk     Ue/Vinf      Cf        Cd     A/A0       D*       Theta      CTq\n");
	else        OutString = QString("x,Hk,Ue/Vinf,Cf,Cd,A/A0,D*,Theta,CTq\n");
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
	out << ("\n\nBottom Side\n");
	if(type==1) OutString = QString("    x         Hk     Ue/Vinf      Cf        Cd     A/A0       D*       Theta      CTq\n");
	else        OutString = QString("x,Hk,Ue/Vinf,Cf,Cd,A/A0,D*,Theta,CTq\n");
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



void QXDirect::OnExportCurFoil()
{
	if(!m_pCurFoil)	return;

	MainFrame *pMainFrame = (MainFrame*)m_pMainFrame;
	QString FileName, DestFileName, OutString;
	QFile DestFile;

	FileName = m_pCurFoil->m_FoilName;
	FileName.replace("/", " ");

	FileName = QFileDialog::getSaveFileName(this, "Export Foil",
											pMainFrame->m_LastDirName,
											"Foil File (*.dat)");

	int pos = FileName.lastIndexOf("/");
	if(pos>0) pMainFrame->m_LastDirName = FileName.left(pos);

	QFile XFile(FileName);

	if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

	QTextStream out(&XFile);

	m_pCurFoil->ExportFoil(out);
	XFile.close();
}

void QXDirect::OnExportCurOpp()
{
	if(!m_pCurFoil || !m_pCurPolar || !m_pCurOpp)	return;

	QString filter =".csv";

	MainFrame *pMainFrame = (MainFrame*)m_pMainFrame;
	QString FileName, DestFileName, OutString;
	QFile DestFile;
	int type = 1;

	FileName = QFileDialog::getSaveFileName(this, "Export Polar",
											pMainFrame->m_LastDirName ,
											"Text File (*.txt; *.csv)",
											&filter);

	int pos = FileName.lastIndexOf("/");
	if(pos>0) pMainFrame->m_LastDirName = FileName.left(pos);
	pos = FileName.lastIndexOf(".csv");
	if (pos>0) type = 2;

	QFile XFile(FileName);

	if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

	QTextStream out(&XFile);

	m_pCurOpp->ExportOpp(out, "QFLR5 v0.01", type);
	XFile.close();
}


void QXDirect::OnExportCurPolar()
{
	if(!m_pCurFoil || !m_pCurPolar)	return;

	QString filter =".csv";

	MainFrame *pMainFrame = (MainFrame*)m_pMainFrame;
	QString FileName, DestFileName, OutString;
	QFile DestFile;
	int type = 1;

	FileName = m_pCurPolar->m_PlrName;
	FileName.replace("/", " ");
	FileName = QFileDialog::getSaveFileName(this, "Export Polar",
											pMainFrame->m_LastDirName + "/"+FileName,
											"Text File (*.txt; *.csv)",
											&filter);

	int pos = FileName.lastIndexOf("/");
	if(pos>0) pMainFrame->m_LastDirName = FileName.left(pos);
	pos = FileName.lastIndexOf(".csv");
	if (pos>0) type = 2;

	QFile XFile(FileName);

	if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

	QTextStream out(&XFile);

	m_pCurPolar->ExportPolar(out, type);
	XFile.close();
}

void QXDirect::OnFoilCoordinates()
{
	if(!m_pCurFoil)	return;
	MainFrame* pMainFrame = (MainFrame*)m_pMainFrame;

	bool bState = m_bShowPanels;//save current view setting

	void* ptr = m_pCurOpp;
	m_pCurOpp = NULL;
	CreateOppCurves();

	bool bPressure = m_bPressure;
	bool bBL       = m_bBL;
	m_bPressure = false;
	m_bBL       = false;
	m_bPolar    = false;
	UpdateView();

	bool bFlap       = m_BufferFoil.m_bTEFlap;
	double FlapAngle = m_BufferFoil.m_TEFlapAngle;
	double Xh        = m_BufferFoil.m_TEXHinge;
	double Yh        = m_BufferFoil.m_TEXHinge;

	m_BufferFoil.m_bTEFlap = false;
//	SetFoilFlap(&m_BufferFoil);

	FoilCoordDlg dlg;
	dlg.m_pXDirect = this;
	dlg.m_pAFoil = NULL;
	dlg.m_pMemFoil    = m_pCurFoil;
	dlg.m_pBufferFoil = &m_BufferFoil;
	dlg.InitDialog();

	m_bShowPanels = true;
	UpdateView();

	QString OldFoilName = m_pCurFoil->m_FoilName;

	if(QDialog::Accepted == dlg.exec())
	{
		m_BufferFoil.m_bTEFlap = bFlap;
		m_BufferFoil.m_TEFlapAngle = FlapAngle;
		m_BufferFoil.m_TEXHinge = Xh;
		m_BufferFoil.m_TEYHinge = Yh;

		CFoil *pNewFoil = new CFoil();
		pNewFoil->CopyFoil(&m_BufferFoil);
		pNewFoil->m_FoilColor  = pMainFrame->GetColor(0);
		pNewFoil->m_nFoilStyle = 0;
		pNewFoil->m_nFoilWidth = 1;
		pNewFoil->m_bPoints = false;
		m_pCurOpp = (OpPoint*)ptr;
		pMainFrame->SetModFoil(pNewFoil);
		pMainFrame->UpdateFoils();
	}
	else
	{
		//reset everything
		m_pCurOpp = (OpPoint*)ptr;
		m_BufferFoil.m_bTEFlap = bFlap;
		m_BufferFoil.m_TEFlapAngle = FlapAngle;
		m_BufferFoil.m_TEXHinge = Xh;
		m_BufferFoil.m_TEYHinge = Yh;
//		SetFoilFlap(&m_BufferFoil);
		SetBufferFoil();
		InitXFoil();
	}

	m_BufferFoil.m_iHighLight = -1;
	m_bShowPanels = bState;//restore as it was
	m_bPressure = bPressure;
	m_bBL       = bBL;
	UpdateView();
}

void QXDirect::OnFoilGeom()
{
	MainFrame* pMainFrame = (MainFrame*)m_pMainFrame;

	if(!m_pCurFoil)	return;

	StopAnimate();
	m_bPolar   = false;

	void* ptr = m_pCurOpp;
	m_pCurOpp = NULL;
	CreateOppCurves();
	UpdateView();

	FoilGeomDlg dlg;
	dlg.m_pMemFoil = m_pCurFoil;
	dlg.m_pBufferFoil = &m_BufferFoil;
	dlg.m_pXDirect = this;
	dlg.m_pAFoil   = NULL;
	dlg.InitDialog();

	if(dlg.exec() == QDialog::Accepted)
	{
		CFoil *pNewFoil = new CFoil();
		pNewFoil->CopyFoil(&m_BufferFoil);
		pNewFoil->m_FoilColor  = pMainFrame->GetColor(0);
		pNewFoil->m_nFoilStyle = 0;
		pNewFoil->m_nFoilWidth = 1;
		pNewFoil->m_bPoints = false;
		m_pCurOpp = (OpPoint*)ptr;
		pMainFrame->SetModFoil(pNewFoil);
		pMainFrame->UpdateFoils();
	}
	else
	{
		m_pCurOpp = (OpPoint*)ptr;
		SetBufferFoil();
		InitXFoil();
	}
	UpdateView();
}




void QXDirect::OnHPlot()
{
	if(!m_pXFoil->lvconv) return;
	int i;

	m_OppVar = 2;
	m_XFoilVar = 10;
	m_pCpGraph->DeleteCurves();
	m_pCpGraph->ResetLimits();
	m_pCpGraph->SetAuto(true);
	m_pCpGraph->SetInverted(false);
	m_pCpGraph->SetYTitle("Hk");
	CCurve * pTopCurve = m_pCpGraph->AddCurve();
	CCurve * pBotCurve = m_pCpGraph->AddCurve();
	pTopCurve->SetTitle("Top");
	pBotCurve->SetTitle("Bot");

	double x[IVX][3],y[IVX][3];
	int nside1, nside2;

	m_pXFoil->CreateXBL(x, nside1, nside2);
	m_pXFoil->FillHk(y, nside1, nside2);

	for (i=2; i<=nside1-1; i++)
	{
		pTopCurve->AddPoint(x[i][1], y[i][1]);
	}
	for (i=2; i<=nside2-1; i++)
	{
		pBotCurve->AddPoint(x[i][2], y[i][2]);
	}

	m_pCpGraph->SetXScale();
	SetFoilScale();
	CheckButtons();
	UpdateView();
}


void QXDirect::OnHideAllOpps()
{
	MainFrame * pMainFrame = (MainFrame*) m_pMainFrame;
	OpPoint *pOpp;
	for (int i=0; i<m_poaOpp->size(); i++)
	{
		pOpp = (OpPoint*)m_poaOpp->at(i);
		pOpp->m_bIsVisible = false;
	}
	pMainFrame->SetSaveState(false);
	if(!m_bPolar) CreateOppCurves();
	SetAnalysisParams();
	UpdateView();
}


void QXDirect::OnHideAllPolars()
{
	MainFrame * pMainFrame = (MainFrame*) m_pMainFrame;
	CPolar *pPolar;
	for (int i=0; i<m_poaPolar->size(); i++)
	{
		pPolar = (CPolar*)m_poaPolar->at(i);
		pPolar->m_bIsVisible = false;
	}
	pMainFrame->SetSaveState(false);
	CreatePolarCurves();
	SetAnalysisParams();
	UpdateView();
}


void QXDirect::OnHideFoilPolars()
{
	if(!m_pCurFoil) return;
	MainFrame * pMainFrame = (MainFrame*) m_pMainFrame;
	CPolar *pPolar;
	for (int i=0; i<m_poaPolar->size(); i++)
	{
		pPolar = (CPolar*)m_poaPolar->at(i);
		if(pPolar->m_FoilName == m_pCurFoil->m_FoilName)
		{
			pPolar->m_bIsVisible = false;
		}
	}
	pMainFrame->SetSaveState(false);
	CreatePolarCurves();
	SetAnalysisParams();
	UpdateView();
}


void QXDirect::OnInitBL()
{
	if(!m_pXFoil) return;
	if (m_pctrlInitBL->isChecked())
	{
		m_pXFoil->lblini = false;
		m_pXFoil->lipan  = false;
	}
	else
	{
		m_pXFoil->lblini = true;
	}
}


void QXDirect::OnInterpolateFoils()
{
	if(m_poaFoil->size()<2)
	{
		QMessageBox msgBox;
		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.setWindowTitle("QFLR5");
		msgBox.setText("At least two foils are required");
		msgBox.exec();
		return;
	}
	MainFrame* pMainFrame = (MainFrame*)m_pMainFrame;

	StopAnimate();

	bool bPressure = m_bPressure;
	bool bBL       = m_bBL;
	m_bPressure = false;
	m_bBL       = false;
	OnOpPoints();

	InterpolateFoilsDlg dlg;
	dlg.m_pMainFrame = m_pMainFrame;
	dlg.m_poaFoil     = m_poaFoil;
	dlg.m_pBufferFoil = &m_BufferFoil;// work on the buffer foil
	dlg.m_pXDirect = this;
	dlg.m_pAFoil   = NULL;
	dlg.InitDialog();

	if(dlg.exec() == QDialog::Accepted)
	{
		CFoil *pNewFoil = new CFoil();
		pNewFoil->CopyFoil(&m_BufferFoil);
		pNewFoil->m_FoilColor  = pMainFrame->GetColor(0);
                pNewFoil->m_nFoilStyle = 0;
		pNewFoil->m_nFoilWidth = 1;
		pNewFoil->m_bPoints = false;
		pNewFoil->m_FoilName = dlg.m_NewFoilName;
		pMainFrame->SetModFoil(pNewFoil);
		pMainFrame->UpdateFoils();
	}
	else
	{
		SetBufferFoil();// restore buffer foil.. from current foil
		InitXFoil();
	}
	m_bPressure = bPressure;
	m_bBL       = bBL;
	UpdateView();

}



void QXDirect::OnNacaFoils()
{
	MainFrame *pMainFrame = (MainFrame*)m_pMainFrame;

	m_bPolar   = false;

	void* ptr0 = m_pCurFoil;
	void* ptr  = m_pCurOpp;
	m_pCurFoil = NULL;
	m_pCurOpp = NULL;

	CreateOppCurves();
	UpdateView();


	NacaFoilDlg dlg;
 	dlg.m_pBufferFoil = &m_BufferFoil;
	dlg.m_pXDirect    = this;
	dlg.m_pAFoil      = NULL;

	if (dlg.exec() == QDialog::Accepted)
	{
		QString str;
		if(dlg.m_Digits>0 && log10((double)dlg.m_Digits)<4)
			str = QString("%1").arg(dlg.m_Digits,4,10,QChar('0'));
		else
			str = QString("%1").arg(dlg.m_Digits);
		str = "NACA "+ str;

		CFoil *pNewFoil = new CFoil;
		pNewFoil->CopyFoil(&m_BufferFoil);
		pNewFoil->m_FoilColor  = pMainFrame->GetColor(0);
		pNewFoil->m_nFoilStyle = 0;
		pNewFoil->m_nFoilWidth = 1;
		pNewFoil->m_bPoints = false;
		pNewFoil->m_FoilName = str;
		m_pCurOpp = (OpPoint*)ptr;
		CFoil *pOldFoil = pMainFrame->GetFoil(str);
		if(pOldFoil) pMainFrame->SetModFoil(pNewFoil);
		else         pMainFrame->AddFoil(pNewFoil);
		pMainFrame->UpdateFoils();

	}
	else
	{
		m_pCurFoil = (CFoil*)ptr0;
		m_pCurOpp  = (OpPoint*)ptr;
		SetBufferFoil();
		InitXFoil();
	}
	UpdateView();
}

void QXDirect::OnNormalizeFoil()
{
	MainFrame* pMainFrame = (MainFrame*)m_pMainFrame;
	if(!m_pCurFoil) return;
	QString str, str1;

	double length = m_pCurFoil->NormalizeGeometry();
	InitXFoil();
	SetBufferFoil();
	str = QString("The foil has been normalized from %1  to 1.000").arg(length,7,'f',3);
	pMainFrame->SetSaveState(false);
	pMainFrame->statusBar()->showMessage(str);

	UpdateView();
}



void QXDirect::OnNPlot()
{
	if(!m_pXFoil->lvconv) return;
	int i;
	int nside1, nside2, ibl;

	m_OppVar = 2;
	m_XFoilVar=6;
	m_pCpGraph->DeleteCurves();
	m_pCpGraph->ResetLimits();
	m_pCpGraph->SetAuto(true);
	m_pCpGraph->SetInverted(false);
	m_pCpGraph->SetYTitle("A/A0");
	CCurve * pTopCurve = m_pCpGraph->AddCurve();
	CCurve * pBotCurve = m_pCpGraph->AddCurve();
	pTopCurve->SetTitle("Top");
	pBotCurve->SetTitle("Bot");

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
		pTopCurve->AddPoint(x[i][1], y[i][1]);
	}
	for (i=2; i<=m_pXFoil->itran[2]-2; i++)
	{
		pBotCurve->AddPoint(x[i][2], y[i][2]);
	}
	m_pCpGraph->SetXScale();
	SetFoilScale();
	CheckButtons();
	UpdateView();
}



void QXDirect::OnOpPoints()
{
	if(!m_bPolar) return;
//	MainFrame* pMainFrame = (MainFrame*)m_pMainFrame;

	m_bPolar = false;
	CreateOppCurves();
	SetFoilScale();
	SetCurveParams();
	CheckButtons();
	UpdateView();
}

void QXDirect::OnPolars()
{
	if(m_bPolar) return;
//	MainFrame* pMainFrame = (MainFrame*)m_pMainFrame;
	m_bPolar = true;
	CreatePolarCurves();
	SetCurveParams();
	CheckButtons();
	UpdateView();
}


void QXDirect::OnPanels()
{
	MainFrame* pMainFrame = (MainFrame*)m_pMainFrame;
	if(!m_pCurFoil)	return;
	StopAnimate();

	bool bState = m_bShowPanels;//save current view setting

	void* ptr = m_pCurOpp;
	m_pCurOpp = NULL;
	CreateOppCurves();

	bool bPressure = m_bPressure;
	bool bBL       = m_bBL;
	m_bPressure = false;
	m_bBL       = false;
	OnOpPoints();

	TwoDPanelDlg dlg;
	dlg.m_pXDirect = this;
	dlg.m_pAFoil   = NULL;
	dlg.m_pBufferFoil = &m_BufferFoil;
	dlg.m_pMemFoil    = m_pCurFoil;

	m_bShowPanels = true;
	UpdateView();

	dlg.InitDialog();

	if(QDialog::Accepted == dlg.exec())
	{
		CFoil *pNewFoil = new CFoil();
		pNewFoil->CopyFoil(&m_BufferFoil);
		pNewFoil->m_FoilColor  = pMainFrame->GetColor(0);
		pNewFoil->m_nFoilStyle = 0;
		pNewFoil->m_nFoilWidth = 1;
		pNewFoil->m_bPoints = true;
		m_pCurOpp = (OpPoint*)ptr;
		pMainFrame->SetModFoil(pNewFoil);
		pMainFrame->UpdateFoils();
	}
	else
	{
		//reset everything
		m_pCurOpp = (OpPoint*)ptr;
		SetBufferFoil();
		InitXFoil();
	}

	m_bShowPanels = bState;//restore as it was
	m_bPressure   = bPressure;
	m_bBL         = bBL;
	UpdateView();
}

void QXDirect::OnPolarGraphVariable()
{
	if(!m_pCurGraph) return;
	if(!m_bPolar)    return;
	GraphVariableDlg dlg;
	dlg.InitDialog(1);

	if(m_pCurGraph == m_pPolarGraph)
	{
		dlg.SetSelection(m_XPolar, m_YPolar);
	}
	else if(m_pCurGraph == m_pCmGraph)
	{
		dlg.SetSelection(m_XCm, m_YCm);
	}
	else if(m_pCurGraph == m_pCzGraph)
	{
		dlg.SetSelection(m_XCz, m_YCz);
	}
	else if(m_pCurGraph == m_pTrGraph)
	{
		dlg.SetSelection(m_XTr, m_YTr);
	}
	else if(m_pCurGraph == m_pUserGraph)
	{
		dlg.SetSelection(m_XUser, m_YUser);
	}
	else return;

	if(QDialog::Accepted == dlg.exec())
	{
		int iX, iY;
		dlg.GetSelection(iX, iY);

		SetGraphTitles(m_pCurGraph, iX, iY);

		if(m_pCurGraph == m_pPolarGraph)
		{
			m_XPolar = iX;
			m_YPolar = iY;
		}
		if(m_pCurGraph == m_pCmGraph)
		{
			m_XCm = iX;
			m_YCm = iY;
		}
		if(m_pCurGraph == m_pCzGraph)
		{
			m_XCz = iX;
			m_YCz = iY;
		}
		if(m_pCurGraph == m_pTrGraph)
		{
			m_XTr = iX;
			m_YTr = iY;
		}
		if(m_pCurGraph == m_pUserGraph)
		{
			m_XUser = iX;
			m_YUser = iY;
		}

		if(iY == 4) m_pCurGraph->SetInverted(true); else m_pCurGraph->SetInverted(false);
		m_pCurGraph->ResetLimits();
		CreatePolarCurves();
		UpdateView();
	}
}

void QXDirect::OnQGraph()
{
	m_bPolar = false;
	if(m_OppVar!=1)
	{
		m_pCpGraph->ResetLimits();
		m_pCpGraph->SetAuto(true);
	}
	m_OppVar = 1;
	m_pCpGraph->SetInverted(false);
	CreateOppCurves();
	m_pCpGraph->SetYTitle("Q");

	CheckButtons();

	m_pCpGraph->SetXScale();
	SetFoilScale();
	UpdateView();
}

void QXDirect::OnResetAllPolarGraphsScales()
{
	m_pPolarGraph->SetAuto(true);
	m_pPolarGraph->ResetXLimits();
	m_pPolarGraph->ResetYLimits();

	m_pCmGraph->SetAuto(true);
	m_pCmGraph->ResetXLimits();
	m_pCmGraph->ResetYLimits();

	m_pCzGraph->SetAuto(true);
	m_pCzGraph->ResetXLimits();
	m_pCzGraph->ResetYLimits();

	m_pTrGraph->SetAuto(true);
	m_pTrGraph->ResetXLimits();
	m_pTrGraph->ResetYLimits();

	m_pUserGraph->SetAuto(true);
	m_pUserGraph->ResetXLimits();
	m_pUserGraph->ResetYLimits();

	UpdateView();
}

void QXDirect::OnResetCpGraphScales()
{
	m_pCurGraph = m_pCpGraph;
	MainFrame *pMainFrame = (MainFrame*)m_pMainFrame;
	pMainFrame->OnResetCurGraphScales();
}

void QXDirect::OnResetFoilScale()
{
	SetFoilScale();
	if(!m_bAnimate) UpdateView();

}


void QXDirect::OnResetXFoil()
{
	m_pXFoil->Initialize();
	m_pXFoil->lblini = false;
	m_pXFoil->lipan  = false;
	InitXFoil();
	InitXFoil2();
}




void QXDirect::OnRtPlot()
{
	if(!m_pXFoil->lvconv) return;
	int i;

	m_OppVar = 2;
	m_XFoilVar=5;
	m_pCpGraph->DeleteCurves();
	m_pCpGraph->ResetLimits();
	m_pCpGraph->SetAuto(true);
	m_pCpGraph->SetInverted(false);
	m_pCpGraph->SetYTitle("Re_Theta");
	CCurve * pTopCurve = m_pCpGraph->AddCurve();
	CCurve * pBotCurve = m_pCpGraph->AddCurve();
	pTopCurve->SetTitle("Top");
	pBotCurve->SetTitle("Bot");

	double x[IVX][3],y[IVX][3];
	int nside1, nside2;

	m_pXFoil->CreateXBL(x, nside1, nside2);
	m_pXFoil->FillRTheta(y, nside1, nside2);

	for (i=2; i<=nside1-1; i++)	pTopCurve->AddPoint(x[i][1], y[i][1]);
	for (i=2; i<=nside2-1; i++) pBotCurve->AddPoint(x[i][2], y[i][2]);

	m_pCpGraph->SetXScale();
	SetFoilScale();
	CheckButtons();
	UpdateView();
}


void QXDirect::OnRtLPlot()
{
	if(!m_pXFoil->lvconv) return;
	int i;

	m_OppVar = 2;
	m_XFoilVar=4;
	m_pCpGraph->DeleteCurves();
	m_pCpGraph->ResetLimits();
	m_pCpGraph->SetAuto(true);
	m_pCpGraph->SetInverted(false);
	m_pCpGraph->SetYTitle("Re_Theta");
	CCurve * pTopCurve = m_pCpGraph->AddCurve();
	CCurve * pBotCurve = m_pCpGraph->AddCurve();
	pTopCurve->SetTitle("Top");
	pBotCurve->SetTitle("Bot");

	double x[IVX][3],y[IVX][3];
	int nside1, nside2;

	m_pXFoil->CreateXBL(x, nside1, nside2);
	m_pXFoil->FillRTheta(y, nside1, nside2);

	for (i=2; i<=nside1-1; i++){
		if (y[i][1]>0.0) y[i][1] = log10( y[i][1] );
		else             y[i][1] = 0.0;
		pTopCurve->AddPoint(x[i][1], y[i][1]);
	}
	for (i=2; i<=nside2-1; i++){
		if (y[i][2]>0.0) y[i][2] = log10( y[i][2] );
		else             y[i][2] = 0.0;
		pBotCurve->AddPoint(x[i][2], y[i][2]);
	}
	m_pCpGraph->SetXScale();
	SetFoilScale();
	CheckButtons();
	UpdateView();
}


void QXDirect::OnSavePolars()
{
	if(!m_pCurFoil || !m_poaPolar->size()) return;

	MainFrame* pMainFrame = (MainFrame*)m_pMainFrame;

	QString FileName, FoilName;
	FileName = m_pCurFoil->m_FoilName + ".plr";
	FileName.replace("/", " ");

	FileName = QFileDialog::getSaveFileName(this, tr("Polar File"), pMainFrame->m_LastDirName+"/"+FileName, tr("Polar File (*.plr)"));

	QString strong = FileName.right(4);
	if(strong !=".plr" || strong !=".PLR") FileName += ".plr";

	QFile XFile(FileName);
	if (!XFile.open(QIODevice::WriteOnly)) return;

	int pos = FileName.lastIndexOf("/");
	if(pos>0) pMainFrame->m_LastDirName = FileName.left(pos);

	QDataStream ar(&XFile);
	ar.setByteOrder(QDataStream::LittleEndian);

	pMainFrame->WritePolars(ar, m_pCurFoil);

	XFile.close();
}


void QXDirect::OnSequence()
{
	m_bSequence = m_pctrlSequence->isChecked();
	SetOpPointSequence();
}


void QXDirect::OnSetFlap()
{
	if(!m_pCurFoil) return;
	StopAnimate();

	void* ptr = m_pCurOpp;
	m_pCurOpp = NULL;
	CreateOppCurves();
	MainFrame* pMainFrame = (MainFrame*)m_pMainFrame;

	bool bPressure = m_bPressure;
	bool bBL       = m_bBL;
	m_bPressure = false;
	m_bBL       = false;
	OnOpPoints();

	FlapDlg dlg;
	dlg.m_pBufferFoil  = &m_BufferFoil;
	dlg.m_pMemFoil     = m_pCurFoil;
	dlg.m_pXFoil       = m_pXFoil;
	dlg.m_pXDirect     = this;
	dlg.m_pAFoil       = NULL;
	dlg.InitDialog();

	if(QDialog::Accepted == dlg.exec())
	{
		CFoil *pNewFoil = new CFoil();
		pNewFoil->CopyFoil(&m_BufferFoil);
		pNewFoil->m_FoilColor  = pMainFrame->GetColor(0);
		pNewFoil->m_nFoilStyle = 0;
		pNewFoil->m_nFoilWidth = 1;
		pNewFoil->m_bPoints = false;
		m_pCurOpp = (OpPoint*)ptr;
		pMainFrame->SetModFoil(pNewFoil);
		pMainFrame->UpdateFoils();
	}
	else
	{
		//reset everything
		m_pCurOpp = (OpPoint*)ptr;
		SetBufferFoil();
		InitXFoil();
	}

	m_bPressure = bPressure;
	m_bBL       = bBL;
	UpdateView();
}


void QXDirect::OnSetLERadius()
{
	if(!m_pCurFoil)	return;
	MainFrame* pMainFrame = (MainFrame*)m_pMainFrame;
	StopAnimate();

	void* ptr = m_pCurOpp;
	m_pCurOpp = NULL;
	CreateOppCurves();
	bool bPressure = m_bPressure;
	bool bBL       = m_bBL;
	m_bPressure = false;
	m_bBL       = false;
	OnOpPoints();

	LEDlg Ldlg;
	Ldlg.m_pBufferFoil = &m_BufferFoil;
	Ldlg.m_pMemFoil    = m_pCurFoil;
	Ldlg.m_pXDirect   = this;
	Ldlg.m_pAFoil     = NULL;
	Ldlg.InitDialog();

	if(QDialog::Accepted == Ldlg.exec())
	{
		CFoil *pNewFoil = new CFoil();
		pNewFoil->CopyFoil(&m_BufferFoil);
		pNewFoil->m_FoilColor  = pMainFrame->GetColor(0);
		pNewFoil->m_nFoilStyle = 0;
		pNewFoil->m_nFoilWidth = 1;
		pNewFoil->m_bPoints = false;
		m_pCurOpp = (OpPoint*)ptr;
		pMainFrame->SetModFoil(pNewFoil);
		pMainFrame->UpdateFoils();
	}

	else
	{
		//reset everything
		m_pCurOpp = (OpPoint*)ptr;
		SetBufferFoil();
		InitXFoil();
	}

	m_bPressure = bPressure;
	m_bBL       = bBL;
	UpdateView();
}



void QXDirect::OnSetTEGap()
{
	MainFrame* pMainFrame = (MainFrame*)m_pMainFrame;
	if(!m_pCurFoil)	return;
	StopAnimate();

	void* ptr = m_pCurOpp;
	m_pCurOpp = NULL;
	CreateOppCurves();

	bool bPressure = m_bPressure;
	bool bBL       = m_bBL;
	m_bPressure = false;
	m_bBL       = false;
	OnOpPoints();

	TEGapDlg Gdlg;
	Gdlg.m_pXDirect   = this;
	Gdlg.m_pAFoil     = NULL;
	Gdlg.m_pBufferFoil = &m_BufferFoil;
	Gdlg.m_pMemFoil    = m_pCurFoil;
	Gdlg.m_Gap         = m_pCurFoil->m_Gap;
	Gdlg.InitDialog();

	if(QDialog::Accepted == Gdlg.exec())
	{
		CFoil *pNewFoil = new CFoil();
		pNewFoil->CopyFoil(&m_BufferFoil);
		pNewFoil->m_FoilColor  = pMainFrame->GetColor(0);
		pNewFoil->m_nFoilStyle = 0;
		pNewFoil->m_nFoilWidth = 1;
		pNewFoil->m_bPoints = false;
		m_pCurOpp = (OpPoint*)ptr;
		pMainFrame->SetModFoil(pNewFoil);
		pMainFrame->UpdateFoils();
	}

	else
	{
		//reset everything
		m_pCurOpp = (OpPoint*)ptr;
		SetBufferFoil();
		InitXFoil();
	}

	m_bPressure = bPressure;
	m_bBL       = bBL;
	UpdateView();

}




void QXDirect::OnShowAllOpps()
{
	MainFrame * pMainFrame = (MainFrame*) m_pMainFrame;
	OpPoint *pOpp;
	for (int i=0; i<m_poaOpp->size(); i++)
	{
		pOpp = (OpPoint*)m_poaOpp->at(i);
		pOpp->m_bIsVisible = true;
	}
	pMainFrame->SetSaveState(false);
	if(!m_bPolar) CreateOppCurves();
	SetAnalysisParams();
	UpdateView();
}

void QXDirect::OnShowAllPolars()
{
	MainFrame * pMainFrame = (MainFrame*) m_pMainFrame;
	CPolar *pPolar;
	for (int i=0; i<m_poaPolar->size(); i++)
	{
		pPolar = (CPolar*)m_poaPolar->at(i);
		pPolar->m_bIsVisible = true;
	}
	pMainFrame->SetSaveState(false);
	CreatePolarCurves();
	SetAnalysisParams();
	UpdateView();
}



void QXDirect::OnShowBL()
{
	if(m_pctrlShowBL->isChecked())
	{
		if(m_bPolar) OnOpPoints();
		m_pctrlShowPressure->setChecked(false);
		m_bBL = true;
		m_bPressure = false;
	}
	else
	{
		m_bBL = false;
	}
	if(!m_bAnimate)	UpdateView();
}


void QXDirect::OnShowFoilPolars()
{
	if(!m_pCurFoil) return;
	MainFrame * pMainFrame = (MainFrame*) m_pMainFrame;
	CPolar *pPolar;
	for (int i=0; i<m_poaPolar->size(); i++)
	{
		pPolar = (CPolar*)m_poaPolar->at(i);
		if(pPolar->m_FoilName == m_pCurFoil->m_FoilName)
		{
			pPolar->m_bIsVisible = true;
		}
	}
	pMainFrame->SetSaveState(false);
	CreatePolarCurves();
	SetAnalysisParams();
	UpdateView();
}


void QXDirect::OnShowCurve()
{
	//user has toggled visible switch
	MainFrame *pMainFrame = (MainFrame*)m_pMainFrame;
	
	if(m_bPolar)
	{
		if (m_pCurPolar)
		{ 
			m_pCurPolar->m_bIsVisible = m_pctrlShowCurve->isChecked();

		}
		CreatePolarCurves(); 
	}
	else if (m_pCurOpp)
	{
		m_pCurOpp->m_bIsVisible = m_pctrlShowCurve->isChecked();
		CreateOppCurves(); 
	}
	pMainFrame->SetSaveState(false);
	UpdateView();
}

void QXDirect::OnShowNeutralLine()
{
	m_bNeutralLine = !m_bNeutralLine;
	if(!m_bPolar) UpdateView();
	MainFrame *pMainFrame = (MainFrame*)m_pMainFrame;
	pMainFrame->showNeutralLine->setChecked(m_bNeutralLine);
}


void QXDirect::OnShowPanels()
{
	m_bShowPanels = !m_bShowPanels;
	if(!m_bPolar)	UpdateView();
	MainFrame *pMainFrame = (MainFrame*)m_pMainFrame;
	pMainFrame->showPanels->setChecked(m_bShowPanels);
}


void QXDirect::OnShowPoints()
{
	MainFrame *pMainFrame = (MainFrame*)m_pMainFrame;

	if(m_bPolar)
	{
		if (m_pCurPolar)
		{ 
			m_pCurPolar->m_bShowPoints = m_pctrlShowPoints->isChecked();
		}
		CreatePolarCurves(); 
	}
	else if (m_pCurOpp)
	{
		m_pCurOpp->m_bShowPoints = m_pctrlShowPoints->isChecked();
		CreateOppCurves(); 
	}
	
	pMainFrame->SetSaveState(false);
	UpdateView();	
}



void QXDirect::OnShowPressure()
{
	if(m_pctrlShowPressure->isChecked())
	{
		if(m_bPolar) OnOpPoints();
		m_pctrlShowBL->setChecked(false);
		m_bBL = false;
		m_bPressure = true;
	}
	else
	{
		m_bPressure = false;
	}
	if(!m_bAnimate)		UpdateView();
}



void QXDirect::OnSinglePolarGraph()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action) return;

	m_iPlrView  = 1;
	m_iPlrGraph = action->data().toInt()+1;
	m_bPolar = true;
	CheckButtons();
	UpdateView();
}



void QXDirect::OnSingleAnalysis()
{
    MainFrame* pMainFrame = (MainFrame*)m_pMainFrame;
    int i,j;
    CPolar *pPolar;
    bool bFound;
    if(!m_pCurFoil) return;

	FoilPolarDlg Adlg(this);
	Adlg.m_MachDef  = m_MachDef;
	Adlg.m_NCrit    = m_NCrit;
	Adlg.m_XBotTr   = m_XBotTr;
	Adlg.m_XTopTr   = m_XTopTr;
	Adlg.m_Mach     = m_Mach;
	Adlg.m_Reynolds = m_Reynolds;
	Adlg.m_Type     = m_Type;

	Adlg.InitDialog();

    int res = Adlg.exec();
    if (res == QDialog::Accepted)
    {
		m_pCurPolar = new CPolar();

		m_pCurPolar->m_FoilName = m_pCurFoil->m_FoilName;
		m_pCurPolar->m_PlrName = Adlg.m_PlrName;
		m_pCurPolar->m_bIsVisible = true;
		m_pCurPolar->m_Type = Adlg.m_Type;

		switch (m_pCurPolar->m_Type)
		{
			case 1:
				m_pCurPolar->m_MaType = 1;
				m_pCurPolar->m_ReType = 1;
				break;
			case 2:
				m_pCurPolar->m_MaType = 2;
				m_pCurPolar->m_ReType = 2;
				break;
			case 3:
				m_pCurPolar->m_MaType = 1;
				m_pCurPolar->m_ReType = 3;
				break;
			case 4:
				m_pCurPolar->m_MaType = 1;
				m_pCurPolar->m_ReType = 1;
				break;
			default:
				m_pCurPolar->m_ReType = 1;
				m_pCurPolar->m_MaType = 1;
				break;
		}

		m_Type     = Adlg.m_Type;
		m_MachDef  = Adlg.m_MachDef;
		m_NCrit    = Adlg.m_NCrit;
		m_XBotTr   = Adlg.m_XBotTr;
		m_XTopTr   = Adlg.m_XTopTr;
		m_Mach     = Adlg.m_Mach;
		m_Reynolds = Adlg.m_Reynolds;


		m_pCurPolar->m_Reynolds = Adlg.m_Reynolds;
		m_pCurPolar->m_Mach     = Adlg.m_Mach;
		m_pCurPolar->m_ASpec    = Adlg.m_ASpec;
		m_pCurPolar->m_ACrit    = Adlg.m_NCrit;
		m_pCurPolar->m_XTop     = Adlg.m_XTopTr;
		m_pCurPolar->m_XBot     = Adlg.m_XBotTr;
		m_pCurPolar->m_Color = pMainFrame->GetColor(1);

		m_pCurPolar = pMainFrame->AddPolar(m_pCurPolar);

		for(i=0; i<12;i++)
		{
			bFound = false;
			for (j=0; j<m_poaPolar->size();j++)
			{
			pPolar = (CPolar*)m_poaPolar->at(j);
			if(pPolar->m_Color == pMainFrame->m_crColors[i]) bFound = true;
			}
			if(!bFound)
			{
			m_pCurPolar->m_Color = pMainFrame->m_crColors[i];
			break;
			}
		}

		SetPolar();
		SetBufferFoil();
		UpdateView();
		pMainFrame->UpdatePolars();
		pMainFrame->SelectPolar(m_pCurPolar);
	    pMainFrame->SetSaveState(false);
    }
}


void QXDirect::OnSpec()
{
	if      (m_pctrlSpec1->isChecked()) m_bAlpha = true;
	else if (m_pctrlSpec2->isChecked()) m_bAlpha = false;
	else if (m_pctrlSpec3->isChecked()) m_bAlpha = false;
}

void QXDirect::OnStoreOpp()
{
	m_bStoreOpp = m_pctrlStoreOpp->isChecked();
}



void QXDirect::OnUePlot()
{
	if(!m_pXFoil->lvconv) return;
	int i;
	double x[IVX][3],y[IVX][3];
	double uei;
	int nside1, nside2, ibl;

	m_OppVar = 2;
	m_XFoilVar = 9;
	m_pCpGraph->DeleteCurves();
	m_pCpGraph->ResetLimits();
	m_pCpGraph->SetAuto(true);
	m_pCpGraph->SetInverted(false);
	m_pCpGraph->SetYTitle("Ue/Vinf");
	CCurve * pTopCurve = m_pCpGraph->AddCurve();
	CCurve * pBotCurve = m_pCpGraph->AddCurve();
	pTopCurve->SetTitle("Top");
	pBotCurve->SetTitle("Bot");

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
		pTopCurve->AddPoint(x[i][1], y[i][1]);
	}
	for (i=2; i<=nside2-1; i++)
	{
		pBotCurve->AddPoint(x[i][2], y[i][2]);
	}
	m_pCpGraph->SetXScale();
	SetFoilScale();
	CheckButtons();
	UpdateView();
}


void QXDirect::OnViscous()
{
	m_bViscous = m_pctrlViscous->isChecked();
}


void QXDirect::OnXDirectStyle()
{
	XDirectStyleDlg dlg;
	dlg.m_pXDirect = this;
	dlg.m_iBLStyle         = m_iBLStyle;
	dlg.m_iBLWidth         = m_iBLWidth;
	dlg.m_crBLColor        = m_crBLColor;
	dlg.m_iPressureStyle   = m_iPressureStyle;
	dlg.m_iPressureWidth   = m_iPressureWidth;
	dlg.m_crPressureColor  = m_crPressureColor;
	dlg.m_iNeutralStyle    = m_iNeutralStyle;
	dlg.m_iNeutralWidth    = m_iNeutralWidth;
	dlg.m_crNeutralColor   = m_crNeutralColor;
	dlg.InitDialog();
	if(dlg.exec() == QDialog::Accepted)
	{
		m_iBLStyle         = dlg.m_iBLStyle;
		m_iBLWidth         = dlg.m_iBLWidth;
		m_crBLColor        = dlg.m_crBLColor;
		m_iPressureStyle   = dlg.m_iPressureStyle;
		m_iPressureWidth   = dlg.m_iPressureWidth;
		m_crPressureColor  = dlg.m_crPressureColor;
		m_iNeutralStyle    = dlg.m_iNeutralStyle;
		m_iNeutralWidth    = dlg.m_iNeutralWidth;
		m_crNeutralColor   = dlg.m_crNeutralColor;
	}
	UpdateView();
}

void QXDirect::OnXFoilAdvanced()
{
	XFoilAdvancedDlg dlg;
	dlg.m_IterLimit = m_IterLim;
	dlg.m_VAccel  = m_pXFoil->vaccel;
	dlg.m_bInitBL = m_bAutoInitBL;
	dlg.InitDialog();

	if (QDialog::Accepted == dlg.exec())
	{
		m_pXFoil->vaccel = dlg.m_VAccel;
		m_IterLim        = dlg.m_IterLimit;
		m_bAutoInitBL    = dlg.m_bInitBL;
	}
}


void QXDirect::PaintBL(QPainter &painter, OpPoint* pOpPoint, double scale)
{
	QPoint offset, From, To;
	double x,y;
	int i;
	double alpha = -pOpPoint->Alpha*pi/180.0;
	double cosa = cos(alpha);
	double sina = sin(alpha);

	if(!pOpPoint->m_bVisc || !pOpPoint->m_bDispSurf) return;

	offset = m_FoilOffset;
	scale  = scale;
	scale  = scale;

	QPen WakePen(m_crBLColor);
	WakePen.setStyle(GetStyle(m_iBLStyle));
	WakePen.setWidth(m_iBLWidth);

	painter.setPen(WakePen);

	x = (pOpPoint->xd1[1]-0.5)*cosa - pOpPoint->yd1[1]*sina + 0.5;
	y = (pOpPoint->xd1[1]-0.5)*sina + pOpPoint->yd1[1]*cosa;
	From.rx() = (int)( x*scale) + offset.x();
	From.ry() = (int)(-y*scale) + offset.y();
	for (i=2; i<=pOpPoint->nd1; i++)
	{
		x = (pOpPoint->xd1[i]-0.5)*cosa - pOpPoint->yd1[i]*sina + 0.5;
		y = (pOpPoint->xd1[i]-0.5)*sina + pOpPoint->yd1[i]*cosa;
		To.rx() = (int)( x*scale) + offset.x();
		To.ry() = (int)(-y*scale) + offset.y();
		painter.drawLine(From, To);
		From = To;
	}

	x = (pOpPoint->xd2[0]-0.5)*cosa - pOpPoint->yd2[0]*sina + 0.5;
	y = (pOpPoint->xd2[0]-0.5)*sina + pOpPoint->yd2[0]*cosa;
	From.rx() = (int)( x*scale) + offset.x();
	From.ry() = (int)(-y*scale) + offset.y();
	for (i=1; i<pOpPoint->nd2; i++)
	{
		x = (pOpPoint->xd2[i]-0.5)*cosa - pOpPoint->yd2[i]*sina + 0.5;
		y = (pOpPoint->xd2[i]-0.5)*sina + pOpPoint->yd2[i]*cosa;
		To.rx() = (int)( x*scale) + offset.x();
		To.ry() = (int)(-y*scale) + offset.y();
		painter.drawLine(From, To);
		From = To;
	}

	x = (pOpPoint->xd3[0]-0.5)*cosa - pOpPoint->yd3[0]*sina + 0.5;
	y = (pOpPoint->xd3[0]-0.5)*sina + pOpPoint->yd3[0]*cosa;
	From.rx() = (int)( x*scale) + offset.x();
	From.ry() = (int)(-y*scale) + offset.y();
	for (i=1; i<pOpPoint->nd3; i++)
	{
		x = (pOpPoint->xd3[i]-0.5)*cosa - pOpPoint->yd3[i]*sina + 0.5;
		y = (pOpPoint->xd3[i]-0.5)*sina + pOpPoint->yd3[i]*cosa;
		To.rx() = (int)( x*scale) + offset.x();
		To.ry() = (int)(-y*scale) + offset.y();
		painter.drawLine(From, To);
		From = To;
	}
}


void QXDirect::PaintPressure(QPainter &painter, OpPoint* pOpPoint, double scale)
{
	if(!pOpPoint->m_bVisc ) return;

	int i;
	double alpha = -pOpPoint->Alpha*pi/180.0;
	double cosa = cos(alpha);
	double sina = sin(alpha);
	double x, y ,xs, ys, xe, ye, dx, dy, x1, x2, y1, y2, r2;
	double cp;
	QPoint From, To;
	QPoint offset = m_FoilOffset;

	QPen CpvPen(m_crPressureColor);
	CpvPen.setStyle(GetStyle(m_iPressureStyle));
	CpvPen.setWidth(m_iPressureWidth);
	painter.setPen(CpvPen);


	for(i=0; i<m_BufferFoil.n; i++)
	{
		if(pOpPoint->m_bVisc)	cp = pOpPoint->Cpv[i];
		else					cp = pOpPoint->Cpi[i];
		x = m_BufferFoil.x[i];
		y = m_BufferFoil.y[i];

		xs = (x-0.5)*cosa - y*sina + 0.5;
		ys = (x-0.5)*sina + y*cosa;

		if(cp>0)
		{
			x += m_BufferFoil.nx[i] * cp * 0.05;
			y += m_BufferFoil.ny[i] * cp * 0.05;

			xe = (x-0.5)*cosa - y*sina + 0.5;
			ye = (x-0.5)*sina + y*cosa;
			painter.drawLine((int)( xs*scale) + offset.x(), (int)(-ys*scale) + offset.y(),
							 (int)( xe*scale) + offset.x(), (int)(-ye*scale) + offset.y());

			dx = xe - xs;
			dy = ye - ys;
			r2 = sqrt(dx*dx + dy*dy);
			if(r2!=0.0) //you can never be sure...
			{
				dx = dx/r2;
				dy = dy/r2;
			}

			x1 = xs + 0.0085*dx + 0.005*dy;
			y1 = ys + 0.0085*dy - 0.005*dx;
			x2 = xs + 0.0085*dx - 0.005*dy;
			y2 = ys + 0.0085*dy + 0.005*dx;

			painter.drawLine((int)( xs*scale) + offset.x(), (int)(-ys*scale) + offset.y(),
							 (int)( x1*scale) + offset.x(), (int)(-y1*scale) + offset.y());

			painter.drawLine((int)( xs*scale) + offset.x(), (int)(-ys*scale) + offset.y(),
							 (int)( x2*scale) + offset.x(), (int)(-y2*scale) + offset.y());
		}
		else
		{

			x += -m_BufferFoil.nx[i] * cp *0.05;
			y += -m_BufferFoil.ny[i] * cp *0.05;

			xe = (x-0.5)*cosa - y*sina+ 0.5;
			ye = (x-0.5)*sina + y*cosa;
			painter.drawLine((int)( xs*scale) + offset.x(), (int)(-ys*scale) + offset.y(),
							 (int)( xe*scale) + offset.x(), (int)(-ye*scale) + offset.y());

			dx = xe - xs;
			dy = ye - ys;
			r2 = sqrt(dx*dx + dy*dy);
			if(r2!=0.0) //you can never be sure...
			{
				dx = -dx/r2;
				dy = -dy/r2;
			}

			x1 = xe + 0.0085*dx + 0.005*dy;
			y1 = ye + 0.0085*dy - 0.005*dx;
			x2 = xe + 0.0085*dx - 0.005*dy;
			y2 = ye + 0.0085*dy + 0.005*dx;

			painter.drawLine((int)( xe*scale) + offset.x(), (int)(-ye*scale) + offset.y(),
							 (int)( x1*scale) + offset.x(), (int)(-y1*scale) + offset.y());

			painter.drawLine((int)( xe*scale) + offset.x(), (int)(-ye*scale) + offset.y(),
							 (int)( x2*scale) + offset.x(), (int)(-y2*scale) + offset.y());
		}
	}
	//last draw lift at XCP position
	QPen LiftPen(m_crPressureColor);
	LiftPen.setStyle(GetStyle(m_iPressureStyle));
	LiftPen.setWidth(m_iPressureWidth+1);
	painter.setPen(LiftPen);

	xs =  (pOpPoint->m_XCP-0.5)*cosa  + 0.5;
	ys = -(pOpPoint->m_XCP-0.5)*sina ;

	xe = xs;
	ye = ys - pOpPoint->Cl/10.0;

	painter.drawLine((int)( xs*scale) + offset.x(), (int)(ys*scale) + offset.y(),
					 (int)( xs*scale) + offset.x(), (int)(ye*scale) + offset.y());

	dx = xe - xs;
	dy = ye - ys;
	r2 = sqrt(dx*dx + dy*dy);
	dx = -dx/r2;
	dy = -dy/r2;

	x1 = xe + 0.0085*dx + 0.005*dy;
	y1 = ye + 0.0085*dy - 0.005*dx;
	x2 = xe + 0.0085*dx - 0.005*dy;
	y2 = ye + 0.0085*dy + 0.005*dx;

	painter.drawLine((int)( xe*scale) + offset.x(), (int)(ye*scale) + offset.y(),
					 (int)( x1*scale) + offset.x(), (int)(y1*scale) + offset.y());

	painter.drawLine((int)( xe*scale) + offset.x(), (int)(ye*scale) + offset.y(),
					 (int)( x2*scale) + offset.x(), (int)(y2*scale) + offset.y());
}


void QXDirect::PaintCoupleGraphs(QPainter &painter)
{
	MainFrame *pMainFrame = (MainFrame*)m_pMainFrame;
	int margin = 10;

	int h  = m_rCltRect.height();
	int w  = m_rCltRect.width();
	int w2 = (int)(w/2);
	int h23 = (int)(2*h/3);

	QRect Rect1(0,0,w2,h23);
	QRect Rect2(w2,0,w2,h23);
	QRect Rect3(0, h23, w,h-h23);
	painter.fillRect(Rect3, pMainFrame->m_BackgroundColor);

	m_pPolarGraph->DrawGraph(Rect1, painter);
	m_pUserGraph->DrawGraph(Rect2, painter);

	QPoint Place(5, h23+margin);
	PaintPolarLegend(Place, h, painter);
}


void QXDirect::PaintOpPoint(QPainter &painter)
{
	QColor color;
	QString Result, str, str1;
	MainFrame *pMainFrame = (MainFrame*)m_pMainFrame;
	painter.fillRect(m_rCltRect, pMainFrame->m_BackgroundColor);

	if (m_rCltRect.width()<150 || m_rCltRect.height()<150) return;

	if(m_bNeutralLine)
	{
		QPen NeutralPen(m_crNeutralColor);
		NeutralPen.setStyle(GetStyle(m_iNeutralStyle));
		NeutralPen.setWidth(m_iNeutralWidth);
		painter.setPen(NeutralPen);
		painter.drawLine(m_rCltRect.left(),m_FoilOffset.y(),
						 m_rCltRect.right(),m_FoilOffset.y());
	}

	if (!m_pCurFoil || !m_pCurFoil->m_FoilName.length())
		return;

	if(m_pCurFoil)
	{
		double Alpha = 0.0;
		if(m_pCurOpp) Alpha = m_pCurOpp->Alpha;

		m_BufferFoil.m_bPoints = m_bShowPanels;
		m_BufferFoil.DrawFoil(painter, -Alpha, m_fFoilScale, m_fFoilScale, m_FoilOffset);

		if(m_bPressure && m_pCurOpp) PaintPressure(painter, m_pCurOpp, m_fFoilScale);
		if(m_bBL && m_pCurOpp)       PaintBL(painter, m_pCurOpp, m_fFoilScale);
	}

	if(m_bCpGraph)
	{
		//Draw Cp Graph
		m_pCpGraph->DrawGraph(painter);
		QPoint Place(m_rCltRect.right()-330, m_rCltRect.top() + 20);
		m_pCpGraph->DrawLegend(painter, Place);//Graph::DrawLegend uses graph's legend font and color
	}


	// Write Titles and results
	QString strong;

	painter.setFont(pMainFrame->m_TextFont);
	int D = 0;
	int ZPos = m_rCltRect.bottom();
	int XPos = m_rCltRect.right()-10;
	QPen WritePen(pMainFrame->m_TextColor);
	painter.setPen(WritePen);

	//write the foil's properties

	int Back = 5;

	if(m_BufferFoil.m_bTEFlap) Back +=3;

//	pDC->SetTextAlign(TA_LEFT);
	int LeftPos = m_rCltRect.left()+10;
	ZPos = m_rCltRect.bottom() - 10 - Back*12;

	D = 0;
	str = "%";
	str1 = QString("Thickness        = %1").arg(m_BufferFoil.m_fThickness*100.0, 6, 'f', 2);
	painter.drawText(LeftPos,ZPos+D, str1+str);
	D+=12;

	str1 = QString("Max.Thick.pos.   = %1").arg(m_BufferFoil.m_fXThickness*100.0, 6, 'f', 2);
	painter.drawText(LeftPos,ZPos+D, str1+str);
	D+=12;

	str1 = QString("Max. Camber      = %1").arg( m_BufferFoil.m_fCamber*100.0, 6, 'f', 2);
	painter.drawText(LeftPos,ZPos+D, str1+str);
	D+=12;

	str1 = QString("Max.Thick.pos.   = %1").arg(m_BufferFoil.m_fXCamber*100.0, 6, 'f', 2);
	painter.drawText(LeftPos,ZPos+D, str1+str);
	D+=12;

	str1 = QString("Number of Panels =  %1").arg( m_BufferFoil.n);
	painter.drawText(LeftPos,ZPos+D, str1);
	D+=12;

	if(m_BufferFoil.m_bTEFlap)
	{
		str1 = QString("Flap Angle = %1 deg").arg( m_BufferFoil.m_TEFlapAngle, 6, 'f', 2);
		painter.drawText(LeftPos,ZPos+D, str1);
		D+=12;

		str1 = QString("XHinge = %1").arg( m_BufferFoil.m_TEXHinge, 6, 'f', 1);
		strong+="%";
		painter.drawText(LeftPos,ZPos+D, str1+strong);
		D+=12;


		str1 = QString("YHinge = %1").arg( m_BufferFoil.m_TEYHinge, 6, 'f', 1);
		strong+="%";
		painter.drawText(LeftPos,ZPos+D, str1+strong);
		D+=12;
	}


//	pDC->SetTextAlign(TA_RIGHT);

//	ZPos = pDrawRect->top+10;
//	XPos = pDrawRect->right-20;
	D = 0;
	Back = 7;

	int dwidth,dheight;
	dwidth  = 220;
	dheight = 12;

	ZPos = m_rCltRect.bottom()-Back*12 - 10;
	XPos = m_rCltRect.right()-dwidth-20;
	D=0;

	if(m_pCurPolar)
	{
		str1 = QString("Polar Type =         %1").arg( m_pCurPolar->m_Type);
		painter.drawText(XPos,ZPos, dwidth, dheight, Qt::AlignRight | Qt::AlignTop, str1);
		D+=12;
		if(m_pCurPolar->m_Type ==1)
		{
			ReynoldsFormat(strong, m_pCurPolar->m_Reynolds );
			strong ="Reynolds = " + strong;
			painter.drawText(XPos,ZPos+D, dwidth, dheight, Qt::AlignRight | Qt::AlignTop, strong);
			D+=12;
			strong = QString("Mach = %1").arg( m_pCurPolar->m_Mach,9,'f',3);
			painter.drawText(XPos,ZPos+D, dwidth, dheight, Qt::AlignRight | Qt::AlignTop, strong);
			D+=12;
		}
		if(m_pCurPolar->m_Type ==2)
		{
			ReynoldsFormat(strong, m_pCurPolar->m_Reynolds );
			strong ="Re.sqrt(Cl) = " + strong;
			painter.drawText(XPos,ZPos+D, dwidth, dheight, Qt::AlignRight | Qt::AlignTop, strong);
			D+=12;
			strong = QString("M.sqrt(Cl) = %1").arg(m_pCurPolar->m_Mach,9,'f',3);
			painter.drawText(XPos,ZPos+D, dwidth, dheight, Qt::AlignRight | Qt::AlignTop, strong);
			D+=12;
		}
		if(m_pCurPolar->m_Type ==3)
		{
			ReynoldsFormat(strong, m_pCurPolar->m_Reynolds );
			strong ="Re.sqrt(Cl) = " + strong;
			painter.drawText(XPos,ZPos+D, dwidth, dheight, Qt::AlignRight | Qt::AlignTop, strong);
			D+=12;

			strong = QString("Mach = %1").arg(m_pCurPolar->m_Mach,9,'f',3);
			painter.drawText(XPos,ZPos+D, dwidth, dheight, Qt::AlignRight | Qt::AlignTop, strong);
			D+=12;
		}
		if(m_pCurPolar->m_Type ==4)
		{
			strong = QString("Alpha = %1 deg").arg(m_pCurPolar->m_ASpec,8,'f',2);
			painter.drawText(XPos,ZPos+D, dwidth, dheight, Qt::AlignRight | Qt::AlignTop, strong);
			D+=12;
			strong = QString("Mach = %1").arg(m_pCurPolar->m_Mach,9,'f',3);
			painter.drawText(XPos,ZPos+D, dwidth, dheight, Qt::AlignRight | Qt::AlignTop, strong);
			D+=12;
		}

		strong = QString("NCrit = %1").arg(m_pCurPolar->m_ACrit,9,'f',2);
		painter.drawText(XPos,ZPos+D, dwidth, dheight, Qt::AlignRight | Qt::AlignTop, strong);
		D+=12;

		strong = QString("Forced Upper Trans. = %1").arg(m_pCurPolar->m_XTop,9,'f',3);
		painter.drawText(XPos,ZPos+D, dwidth, dheight, Qt::AlignRight | Qt::AlignTop, strong);
		D+=12;
		strong = QString("Forced Lower Trans. = %1").arg(m_pCurPolar->m_XBot, 9, 'f', 3);
		painter.drawText(XPos,ZPos+D, dwidth, dheight, Qt::AlignRight | Qt::AlignTop, strong);
		D+=12;
	}

	if(m_pCurOpp)
	{
		Back =6;
		if(m_pCurOpp->m_bTEFlap) Back++;
		if(m_pCurOpp->m_bLEFlap) Back++;
		if(m_pCurOpp->m_bVisc && fabs(m_pCurOpp->Cd)>0.0) Back++;
		if(m_pCurPolar->m_Type==2 ) Back++;
		if(m_pCurPolar->m_Type!=1 && m_pCurPolar->m_Type!=4) Back++;

		ZPos = m_rCltRect.bottom()-Back*12 - 10;
		XPos = (int)((m_rCltRect.right()+m_rCltRect.left())/2.0)-50;
		D=0;

		if(m_pCurPolar->m_Type!=1)
		{
			ReynoldsFormat(Result, m_pCurOpp->Reynolds);
			Result = "Re = "+ Result;
			painter.drawText(XPos,ZPos+D, Result);
			D+=12;
		}
		if(m_pCurPolar->m_Type==2)
		{
			Result = QString("Ma = %1").arg(m_pCurOpp->Mach, 9, 'f', 4);
			painter.drawText(XPos,ZPos+D, Result);
			D+=12;
		}
		if(m_pCurPolar->m_Type!=4)
		{
			Result = QString("       Alpha = %1 deg").arg(m_pCurOpp->Alpha, 8, 'f', 2);
			painter.drawText(XPos,ZPos+D, Result);
			D+=12;
		}
		Result = QString("          Cl = %1").arg(m_pCurOpp->Cl, 9, 'f', 3);
		painter.drawText(XPos,ZPos+D, Result);
		D+=12;

		Result = QString("          Cm = %1").arg(m_pCurOpp->Cm, 9, 'f', 3);
		painter.drawText(XPos,ZPos+D, Result);
		D+=12;

		Result = QString("          Cd = %1").arg(m_pCurOpp->Cd, 10, 'f', 4);
		painter.drawText(XPos,ZPos+D, Result);
		D+=12;

		if(m_pCurOpp->m_bVisc && fabs(m_pCurOpp->Cd)>0.0)
		{
			Result = QString("         L/D = %1").arg(m_pCurOpp->Cl/m_pCurOpp->Cd, 8, 'f', 2);
			painter.drawText(XPos,ZPos+D, Result);
			D+=12;
		}

		Result = QString("Upper Trans. = %1").arg(m_pCurOpp->Xtr1, 9, 'f', 3);
		painter.drawText(XPos,ZPos+D, Result);
		D+=12;

		Result = QString("Lower Trans. = %1").arg(m_pCurOpp->Xtr2, 9, 'f', 3);
		painter.drawText(XPos,ZPos+D, Result);
		D+=12;

		if(m_pCurOpp->m_bTEFlap)
		{
			Result = QString("TE Hinge Moment/span = %1").arg(m_pCurOpp->m_TEHMom, 9, 'e', 3);
			painter.drawText(XPos,ZPos+D, Result);
			D+=12;
		}

		if(m_pCurOpp->m_bLEFlap)
		{
			Result = QString("LE Hinge Moment/span = %1").arg(m_pCurOpp->m_LEHMom, 9, 'e', 3);
			painter.drawText(XPos,ZPos+D, Result);
			D+=12;
		}
	}
}



void QXDirect::PaintPolarGraphs(QPainter &painter)
{
	MainFrame *pMainFrame = (MainFrame*)m_pMainFrame;
	int h  = m_rCltRect.height();
	int w  = m_rCltRect.width();
	int h2 = (int)(h/2);
	int w2 = (int)(w/2);
	int w4 = (int)(w/4);

	QRect Rect1(0,0,w2,h2);
	QRect Rect2(w2,0,w4,h2);
	QRect Rect3(w2, h2, w4, h2);
	QRect Rect4(3*w4,0,w4,h2);
	QRect Rect5(0,h2, w2,h-h2);
	QRect Rect6(3*w4,h2,w4,h2);

	painter.fillRect(Rect5, pMainFrame->m_BackgroundColor);

	m_pPolarGraph->DrawGraph(Rect1, painter);
	m_pCzGraph->DrawGraph(Rect2, painter);
	m_pCmGraph->DrawGraph(Rect3, painter);
	m_pTrGraph->DrawGraph(Rect4, painter);

	if(m_bShowUserGraph)	m_pUserGraph->DrawGraph(Rect6, painter);
	else				 	painter.fillRect(Rect6, pMainFrame->m_BackgroundColor);


	QPoint Place(10, h2+30);
	PaintPolarLegend(Place,  h, painter);
}


void QXDirect::PaintPolarLegend(QPoint place, int bottom, QPainter &painter)
{
	MainFrame *pMainFrame = (MainFrame*)m_pMainFrame;
	int LegendSize, LegendWidth, ypos, x1;
	QColor color;
	int i,j,k,l,nc,ny,nFoils;

	LegendSize = 30;
	LegendWidth = 210;
	QFontMetrics fm(pMainFrame->m_TextFont);
	ypos = fm.height();

	painter.setFont(pMainFrame->m_TextFont);

	QStringList str; // we need to make an inventory of foils
	CPolar * pPolar;
	CFoil *pFoil;
	for (j=0; j<m_poaFoil->size(); j++)
	{
		pFoil = (CFoil*)m_poaFoil->at(j);
		for (i=0; i<m_poaPolar->size(); i++)
		{
			pPolar = (CPolar*)m_poaPolar->at(i);
			if (pPolar->m_FoilName == pFoil->m_FoilName && pPolar->m_Alpha.size())
			{
				str.append(pFoil->m_FoilName);
				break;
			}
		}// finished inventory
	}

	nFoils= str.size();

//	painter.SetTextAlign(TA_LEFT);
	painter.setBackgroundMode(Qt::TransparentMode);
	painter.setFont(pMainFrame->m_TextFont);
	QPen TextPen(pMainFrame->m_TextColor);
	TextPen.setWidth(1);
	painter.setPen(TextPen);
	QBrush LegendBrush(pMainFrame->m_BackgroundColor);
	painter.setBrush(LegendBrush);

	QPen LegendPen;
	LegendPen.setWidth(1);

	ny =0;
	for (k = 0; k<nFoils; k++)
	{
		int FoilPlrs = 0;
		for (l=0; l < m_poaPolar->size(); l++)
		{
			pPolar = (CPolar*)m_poaPolar->at(l);
			if (pPolar->m_Alpha.size() &&
				pPolar->m_PlrName.length() &&
				pPolar->m_bIsVisible &&
				pPolar->m_FoilName == str.at(k) &&
                                ((pPolar->m_Type == 1 && m_bType1) ||
                                 (pPolar->m_Type == 2 && m_bType2) ||
                                 (pPolar->m_Type == 3 && m_bType3) ||
                                 (pPolar->m_Type == 4 && m_bType4)))
					FoilPlrs++;
		}
		if (FoilPlrs)
		{
			int YPos = place.y() + (ny+FoilPlrs+2) * ypos;// bottom line of this foil's legend
			if(abs(bottom) > abs(YPos))
			{
				ny++;
			}
			else if (k>0)
			{
			// move rigth if less than client bottom area
				place.rx() += LegendWidth;
				ny=1;
			}
			painter.setPen(TextPen);
			painter.drawText(place.x() + (int)(0.5*LegendSize), place.y() + ypos*ny-(int)(ypos/2),
							 str.at(k));
		}
		for (nc=0; nc < m_poaPolar->size(); nc++)
		{
			pPolar = (CPolar*)m_poaPolar->at(nc);
			if(str.at(k) == pPolar->m_FoilName)
			{
				if (pPolar->m_Alpha.size() && pPolar->m_PlrName.length() && pPolar->m_bIsVisible &&
                                        ((pPolar->m_Type == 1 && m_bType1) ||
                                         (pPolar->m_Type == 2 && m_bType2) ||
                                         (pPolar->m_Type == 3 && m_bType3) ||
                                         (pPolar->m_Type == 4 && m_bType4)))
				{
					//is there anything to draw ?
					LegendPen.setColor(pPolar->m_Color);
					LegendPen.setStyle(GetStyle(pPolar->m_Style));
					LegendPen.setWidth(pPolar->m_Width);
					painter.setPen(LegendPen);

					painter.drawLine(place.x() + (int)(1.0*LegendSize), place.y() + (int)(1.*ypos*ny)+2,
									 place.x() + (int)(2.0*LegendSize), place.y() + (int)(1.*ypos*ny)+2);
					if(pPolar->m_bShowPoints)
					{
						x1 = place.x() + (int)(1.5*LegendSize);
						painter.drawRect(x1-2, place.y() + (int)(1.*ypos*ny), 4, 4);
					}

					painter.setPen(TextPen);
					painter.drawText(place.x() + (int)(2.5*LegendSize), place.y() + (int)(1.*ypos*ny)+(int)(ypos/3),
									 pPolar->m_PlrName);
					ny++ ;
				}
			}
		}
		if (FoilPlrs) ny++;
	}
}



void QXDirect::PaintSingleGraph(QPainter &painter)
{
	MainFrame* pMainFrame = (MainFrame*)m_pMainFrame;

	int h   = m_rCltRect.height();
	int w   = m_rCltRect.width();
	int w3  = (int)(w/3);
	int w23 = 2*w3;

	QRect Rect1(0,0,2*w3,h);
	QRect Rect2(2*w3, 0, w-2*w3,h);
	painter.fillRect(Rect2, pMainFrame->m_BackgroundColor);
	QPoint Place(w23+10, 10);

	PaintPolarLegend(Place, h, painter);

	switch (m_iPlrGraph)
	{
		case 1:
		{
			m_pCurGraph = m_pPolarGraph;
			break;
		}
		case 2:
		{
			m_pCurGraph = m_pCzGraph;
			break;
		}
		case 3:
		{
			m_pCurGraph = m_pTrGraph;
			break;
		}
		case 4:
		{
			m_pCurGraph = m_pCmGraph;
			break;
		}
		case 5:
		{
			m_pCurGraph = m_pUserGraph;
			break;
		}
	}

	if (m_pCurGraph) m_pCurGraph->DrawGraph(Rect1, painter);


}



void QXDirect::PaintView(QPainter &painter)
{
	MainFrame* pMainFrame = (MainFrame*)m_pMainFrame;
	if(m_pCurFoil && !m_bPolar)
	{
		PaintOpPoint(painter);
	}
	else if (m_bPolar)
	{
		if(m_iPlrView==0)      PaintPolarGraphs(painter);
		else if(m_iPlrView==1) PaintSingleGraph(painter);
		else if(m_iPlrView==2) PaintCoupleGraphs(painter);
	}
	else
	{
		painter.fillRect(m_rCltRect, pMainFrame->m_BackgroundColor);
	}
}


void QXDirect::ReadParams()
{
	if(!m_pCurPolar) return;

	if      (m_pctrlSpec1->isChecked()) m_bAlpha = true;
	else if (m_pctrlSpec2->isChecked()) m_bAlpha = false;
	else if (m_pctrlSpec3->isChecked()) m_bAlpha = false;


	if(m_pCurPolar->m_Type !=4)
	{
		if(m_bAlpha)
		{
			m_Alpha      = m_pctrlAlphaMin->GetValue();
			m_AlphaMax   = m_pctrlAlphaMax->GetValue();
			m_AlphaDelta = m_pctrlAlphaDelta->GetValue();
		}
		else
		{
			m_Cl      = m_pctrlAlphaMin->GetValue();
			m_ClMax   = m_pctrlAlphaMax->GetValue();
			m_ClDelta = m_pctrlAlphaDelta->GetValue();
		}
	}
	else
	{
			m_Reynolds      = m_pctrlAlphaMin->GetValue();
			m_ReynoldsMax   = m_pctrlAlphaMax->GetValue();
			m_ReynoldsDelta = m_pctrlAlphaDelta->GetValue();
	}
	m_bSequence = m_pctrlSequence->isChecked();
	m_bInitBL   = m_pctrlInitBL->isChecked();
	m_bViscous  = m_pctrlViscous->isChecked();
	m_bStoreOpp = m_pctrlStoreOpp->isChecked();
}


void QXDirect::SaveSettings(QDataStream &ar)
{
	if(ar.status() != QDataStream::Ok)
	{
		QMessageBox msgBox;
		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.setWindowTitle("QFLR5");
		msgBox.setText("file is closed");
		msgBox.exec();
		return;
	}
	ar << m_bAlpha << m_bStoreOpp << m_bViscous << m_bInitBL << m_bBL << m_bPressure;
	ar << m_bPolar << m_bShowUserGraph << m_bShowPanels << m_bType1 << m_bType2 << m_bType3 << m_bType4;
	ar << m_bAutoInitBL << m_bFromList << m_bFromZero << m_bShowTextOutput << m_bNeutralLine << m_bCurOppOnly;
	ar << m_bShowInviscid << m_bCpGraph << m_bSequence;

	ar << m_crBLColor       << m_iBLStyle       << m_iBLWidth;
	ar << m_crPressureColor << m_iPressureStyle << m_iPressureWidth;
	ar << m_crNeutralColor << m_iNeutralStyle << m_iNeutralWidth;

	ar << m_OppVar << m_XFoilVar << m_IterLim << m_XPolar << m_YPolar << m_XCz << m_YCz;
	ar << m_XCm << m_YCm << m_XTr << m_YTr << m_XUser << m_YUser;
	ar << m_iPlrGraph << m_iPlrView;
	ar << m_Alpha << m_AlphaMax << m_AlphaDelta;
	ar << m_Cl << m_ClMax << m_ClDelta;
	ar << m_Reynolds << m_ReynoldsMax << m_ReynoldsDelta;
	ar << m_NCrit << m_XTopTr << m_XBotTr;
	ar << m_Mach << m_MachDef;
	ar << m_Type;
	ar << m_pXFoil->vaccel <<  m_IterLim << m_bAutoInitBL ;

	ar << m_NRe;
	for (int i=0; i<m_NRe; i++) ar << m_ReList[i] << m_MachList[i] << m_NCritList[i];

	m_pPolarGraph->Serialize(ar, true);
	m_pCmGraph->Serialize(ar, true);
	m_pCzGraph->Serialize(ar, true);
	m_pTrGraph->Serialize(ar, true);
	m_pUserGraph->Serialize(ar, true);

	m_pCpGraph->Serialize(ar, true);
}


void QXDirect::SetAnalysisParams()
{
	if(m_bViscous)	m_pctrlViscous->setChecked(true);      else m_pctrlViscous->setChecked(false);
	if(m_bInitBL)	m_pctrlInitBL->setChecked(true);       else m_pctrlInitBL->setChecked(false);
	if(m_bStoreOpp)	m_pctrlStoreOpp->setChecked(true);     else m_pctrlStoreOpp->setChecked(false);
	if(m_bPressure)	m_pctrlShowPressure->setChecked(true); else m_pctrlShowPressure->setChecked(false);
	if(m_bBL)	    m_pctrlShowBL->setChecked(true);       else m_pctrlShowBL->setChecked(false);

	if(m_pCurPolar)
	{
		if(m_pCurPolar->m_Type != 4)
		{
			m_pctrlAlphaMin->SetPrecision(2);
			m_pctrlAlphaMax->SetPrecision(2);
			m_pctrlAlphaDelta->SetPrecision(2);
			if(m_bAlpha) m_pctrlSpec1->setChecked(true);
			else         m_pctrlSpec2->setChecked(true);
			m_pctrlSpec3->setEnabled(false);
		}
		else
		{
			m_pctrlSpec3->setChecked(true);
			m_pctrlSpec3->setEnabled(true);
			m_pctrlAlphaMin->SetPrecision(0);
			m_pctrlAlphaMax->SetPrecision(0);
			m_pctrlAlphaDelta->SetPrecision(0);
		}
	}
	else
	{
		if(m_bAlpha) m_pctrlSpec1->setChecked(true);
		else         m_pctrlSpec2->setChecked(true);
		m_pctrlSpec3->setEnabled(false);
	}
	SetOpPointSequence();

	SetCurveParams();
}


void QXDirect::SetBufferFoil()
{
	if(!m_pCurFoil || !m_pCurFoil->m_FoilName.length()) return;

	m_BufferFoil.CopyFoil(m_pCurFoil);

	m_BufferFoil.m_FoilName   = m_pCurFoil->m_FoilName;
	m_BufferFoil.m_FoilColor  = m_pCurFoil->m_FoilColor;
	m_BufferFoil.m_nFoilStyle = m_pCurFoil->m_nFoilStyle;
	m_BufferFoil.m_nFoilWidth = m_pCurFoil->m_nFoilWidth;
}

void QXDirect::SetCurveParams()
{
	if(m_bPolar)
	{
		if(m_pCurPolar)
		{
			if(m_pCurPolar->m_bIsVisible)  m_pctrlShowCurve->setChecked(true);  else  m_pctrlShowCurve->setChecked(false);
			if(m_pCurPolar->m_bShowPoints) m_pctrlShowPoints->setChecked(true); else  m_pctrlShowPoints->setChecked(false);

			m_CurveColor = m_pCurPolar->m_Color;
			m_CurveStyle = m_pCurPolar->m_Style;
			m_CurveWidth = m_pCurPolar->m_Width;
			FillComboBoxes();
		}
		else
		{
			FillComboBoxes(false);
		}
	}
	else
	{
		//set Opoint params
		if(m_pCurOpp)
		{
			if(m_pCurOpp->m_bIsVisible)  m_pctrlShowCurve->setChecked(true);  else  m_pctrlShowCurve->setChecked(false);
			if(m_pCurOpp->m_bShowPoints) m_pctrlShowPoints->setChecked(true); else  m_pctrlShowPoints->setChecked(false);

			m_CurveColor = m_pCurOpp->m_Color;
			m_CurveStyle = m_pCurOpp->m_Style;
			m_CurveWidth = m_pCurOpp->m_Width;
			FillComboBoxes();
		}
		else
		{
			FillComboBoxes(false);
		}
	}
}



CFoil* QXDirect::SetFoil(CFoil* pFoil)
{
	MainFrame* pMainFrame = (MainFrame*)m_pMainFrame;
	StopAnimate();

	m_pCurFoil = pFoil;

	if(!m_pCurFoil)
	{
		//take the first in the array, if any
		if(m_poaFoil->size())
		{
			m_pCurFoil = (CFoil*)m_poaFoil->at(0);
		}
	}


	if(m_pCurFoil && !InitXFoil())
	{
		DeleteFoil(false);
		pMainFrame->m_pCurFoil = NULL;
	}
	else
	{
		if(!m_pCurFoil)
		{
			m_pCurPolar = NULL;
			m_pCurOpp = NULL;
		}
		else if (m_pCurPolar && m_pCurPolar->m_FoilName !=m_pCurFoil->m_FoilName)
		{
//			m_pCurPolar = NULL;
//			m_pCurOpp = NULL;
		}
		else if (m_pCurOpp && m_pCurOpp->m_strFoilName  !=m_pCurFoil->m_FoilName)
		{
//			m_pCurOpp = NULL;
		}
	}
	pMainFrame->m_pCurFoil = m_pCurFoil;

	SetBufferFoil();

	SetPolar();


//		if(m_bPolar) CreatePolarCurves();
//		else         CreateOppCurves();
	return m_pCurFoil;
}


CFoil* QXDirect::SetFoil(QString FoilName)
{
	MainFrame* pMainFrame = (MainFrame*)m_pMainFrame;
	int j;
	StopAnimate();

	if(FoilName.length())
	{
		CFoil *pFoil;
		m_pCurFoil = NULL;
		for (j=0; j< m_poaFoil->size(); j++)
		{
			pFoil = (CFoil*)m_poaFoil->at(j);
			if(pFoil->m_FoilName == FoilName)
			{
				m_pCurFoil = pFoil;				
				break;
			}
		}
	}
	else m_pCurFoil = NULL;

	if(m_pCurFoil)
	{
		if(!InitXFoil())
		{
			pMainFrame->m_pCurFoil=NULL;
			DeleteFoil(false);
			SetPolar();
			return NULL;
		}
	}
	if(!m_pCurFoil)
	{
		m_pCurPolar = NULL;
		m_pCurOpp = NULL;
	}
	else if(m_pCurPolar && m_pCurPolar->m_FoilName !=m_pCurFoil->m_FoilName)
	{
//		m_pCurPolar = NULL;
//		m_pCurOpp = NULL;
	}
	else if(m_pCurOpp && m_pCurOpp->m_strFoilName  !=m_pCurFoil->m_FoilName)
	{
//		m_pCurOpp = NULL;
	}

	//are there 2 or more Opps associated to this foil
	int count = 0;

	OpPoint *pOpp;
	for (int i=0; i< m_poaOpp->size(); i++)
	{
		pOpp = (OpPoint*)m_poaOpp->at(i);
		if(pOpp->m_strFoilName == FoilName) count++;
		if(count>=2) break;
	}

	pMainFrame->m_pCurFoil=m_pCurFoil;

	SetBufferFoil();
	SetPolar();

	if(m_bPolar) CreatePolarCurves();
	else         CreateOppCurves();
	return m_pCurFoil;
}




void QXDirect::SetFoilScale(QRect CltRect)
{
	m_rCltRect = CltRect;

	SetFoilScale();
}


void QXDirect::SetFoilScale()
{
	QRect rect(10, 10, + m_rCltRect.width()-20, m_rCltRect.height()-2*m_FoilYPos);
	m_pCpGraph->SetDrawRect(rect);

	m_FoilOffset.rx() = m_rCltRect.left() +(int)(1.0*m_pCpGraph->GetMargin());
	m_FoilOffset.ry() = m_rCltRect.bottom()-m_FoilYPos;


	m_fFoilScale = (m_rCltRect.width()-2.0*m_pCpGraph->GetMargin());
	if(m_OppVar>=2)
	{
		double p0  = m_pCpGraph->xToClient(0.0);
		double p1  = m_pCpGraph->xToClient(1.0);
		m_fFoilScale =  (p1-p0);
	}
}


CPolar * QXDirect::SetPolar(CPolar *pPolar)
{
	// Finds the plr
	// sets it as the active polar
	// and initializes XFoil
	QString PlrName, strong;
	StopAnimate();

	if(!m_pCurFoil|| !m_pCurFoil->m_FoilName.length())
	{
		m_pCurPolar = NULL;
		SetAnalysisParams();
		return NULL;
	}

	if(pPolar) m_pCurPolar = pPolar;

	if(!m_pCurPolar)
	{
		//try to get one from the object array
		for(int i=0; i<m_poaPolar->size(); i++)
		{
			pPolar = (CPolar*)m_poaPolar->at(i);
			if(pPolar && pPolar->m_FoilName==m_pCurFoil->m_FoilName)
			{
				//set this one
				m_pCurPolar = pPolar;
				break;
			}
		}
	}

	if(m_pCurPolar)
	{
		if(m_pCurPolar->m_FoilName == m_pCurFoil->m_FoilName)
		{
			InitXFoil2();
		}
		else
		{
			CPolar *pOldPolar;
			bool bFound = false;
			for (int i=0; i<m_poaPolar->size(); i++)
			{
				pOldPolar = (CPolar*)m_poaPolar->at(i);
				if ((pOldPolar->m_FoilName == m_pCurFoil->m_FoilName) &&
					(pOldPolar->m_PlrName  == m_pCurPolar->m_PlrName))
				{
					m_pCurPolar = pOldPolar;
					InitXFoil2();
					bFound = true;
					break;
				}
			}
			if(!bFound){
				m_pCurPolar = NULL;
				m_pCurOpp   = NULL;
			}
		}
	}

	if (m_bPolar) CreatePolarCurves();
	SetAnalysisParams();
	SetOpp();

	return m_pCurPolar;
}


CPolar * QXDirect::SetPolar(QString PlrName)
{
	// Finds the plr with name PlrName
	// sets it as the active polar
	// and initializes XFoil
	MainFrame* pMainFrame = (MainFrame*)m_pMainFrame;

	if(!m_pCurFoil) return NULL;

	if(!PlrName.length())
	{// try to read it
		if (!pMainFrame->m_pctrlPolar->count())
		{
			m_pCurPolar = NULL;
			m_pCurOpp   = NULL;
			return NULL;//give up
		}
		//else
		PlrName = pMainFrame->m_pctrlPolar->currentText();
	}
	m_pCurPolar = pMainFrame->GetPolar(m_pCurFoil->m_FoilName, PlrName);

	if (!m_pCurPolar || !m_pCurFoil ||
		!m_pCurPolar->m_FoilName.length() ||
		!m_pCurFoil->m_FoilName.length())
	{
		m_pCurPolar = NULL;
		m_pCurOpp   = NULL;
		return NULL;
	}

	if(m_pCurPolar && (m_pCurPolar->m_FoilName == m_pCurFoil->m_FoilName))
	{
		InitXFoil2();
	}
	else
	{
		m_pCurPolar = NULL;
		m_pCurOpp = NULL;
	}

	if(m_pCurOpp)
	{
		if (m_pCurOpp->m_strFoilName != m_pCurFoil->m_FoilName ||
			m_pCurOpp->m_strPlrName  != PlrName)
		{
//			does the Opp exist for these Foil/plrs ?
			OpPoint *pOpp;
			bool bFound = false;
			for (int l =0; l<m_poaOpp->size(); l++)
			{
				pOpp = (OpPoint*)m_poaOpp->at(l);
				if (pOpp->m_strFoilName == m_pCurFoil->m_FoilName &&
					pOpp->m_strPlrName  == PlrName)
				{
					if(fabs(pOpp->Alpha-m_pCurOpp->Alpha)<0.0001)
					{
						m_pCurOpp = pOpp;
						bFound = true;
						break;
					}
				}
			}

			if(!bFound)
				m_pCurOpp = NULL;//give up
		}
	}

	SetAnalysisParams();

	SetOpp();

	return m_pCurPolar;
}



OpPoint * QXDirect::SetOpp(double Alpha)
{
	// set the opp, if valid
	// else set the current Opp, if any
	// else set the comboBox's first, if any
	// else set it to NULL
	OpPoint * pOpp = NULL;
	MainFrame* pFrame = (MainFrame*)m_pMainFrame;
	QString strong;

	if(!m_pCurFoil)  return NULL;
	if(!m_pCurPolar) return NULL;

	if(Alpha < -1234567.0) //the default
	{
		if(m_pCurOpp && m_pCurOpp->m_strFoilName == m_pCurFoil->m_FoilName)	pOpp = m_pCurOpp;
		else
		{
			//try to get one from the combobox
			if(pFrame->m_pctrlOpPoint->count())
			{
				QString strong;
				bool bOK;
				int sel = pFrame->m_pctrlOpPoint->currentIndex();
				if (sel>=0) strong = pFrame->m_pctrlOpPoint->itemText(sel);
				Alpha = strong.toFloat(&bOK);
				if(!bOK) pOpp = NULL;
				else     pOpp = pFrame->GetOpp(Alpha);
			}
			else pOpp = NULL;
		}
	}
	else
	{
		pOpp = pFrame->GetOpp(Alpha);
	}


	if(pOpp) 
	{
		//set it
		pFrame->SelectOpPoint(pOpp);//to Combobox
	}
	m_pCurOpp = pOpp;

	CreateOppCurves();

//	m_posAnimate = pFrame->m_pctrlOpp.GetCurSel(); //TODO
	SetCurveParams();

	return m_pCurOpp;
}



void QXDirect::SetOpPointSequence()
{
	if(m_bSequence)
	{
		m_pctrlSequence->setCheckState(Qt::Checked);
		m_pctrlAlphaMax->setEnabled(true);
		m_pctrlAlphaDelta->setEnabled(true);
	}
	else
	{
		m_pctrlSequence->setCheckState(Qt::Unchecked);
		m_pctrlAlphaMax->setEnabled(false);
		m_pctrlAlphaDelta->setEnabled(false);
	}


	if(!m_pCurPolar || (m_pCurPolar && m_pCurPolar->m_Type != 4))
	{
		if(m_pctrlSpec3->isChecked())
		{
			m_pctrlSpec1->setChecked(true);
			m_bAlpha = true;
		}

		if(m_bAlpha)
		{
			m_pctrlAlphaMin->SetValue(m_Alpha);
			m_pctrlAlphaMax->SetValue(m_AlphaMax);
			m_pctrlAlphaDelta->SetValue(m_AlphaDelta);
		}
		else
		{
			m_pctrlAlphaMin->SetValue(m_Cl);
			m_pctrlAlphaMax->SetValue(m_ClMax);
			m_pctrlAlphaDelta->SetValue(m_ClDelta);
		}
	}
	else
	{
		m_pctrlSpec3->setChecked(true);
		m_bAlpha = true;		// no choice with type 4 polars
		m_pctrlAlphaMin->SetValue(m_Reynolds);
		m_pctrlAlphaMax->SetValue(m_ReynoldsMax);
		m_pctrlAlphaDelta->SetValue(m_ReynoldsDelta);
	}
}

void QXDirect::SetGraphTitles(Graph* pGraph, int iX, int iY)
{
	switch (iX)
	{
		case 0:
			pGraph->SetXTitle("Alpha");
			break;
		case 1:
			pGraph->SetXTitle("Cl");
			break;
		case 2:
			pGraph->SetXTitle("Cd");
			break;
		case 3:
			pGraph->SetXTitle("Cd x 10000");
			break;
		case 4:
			pGraph->SetXTitle("Cdp");
			break;
		case 5:
			pGraph->SetXTitle("Cm");
			break;
		case 6:
			pGraph->SetXTitle("Xtr1");
			break;
		case 7:
			pGraph->SetXTitle("Xtr2");
			break;
		case 8:
			pGraph->SetXTitle("HMom");
			break;
		case 9:
			pGraph->SetXTitle("Cpmin");
			break;
		case 10:
			pGraph->SetXTitle("Cl/Cd");
			break;
		case 11:
			pGraph->SetXTitle("|Cl|^(3/2)/Cd");
			break;
		case 12:
			pGraph->SetXTitle("1/Rt(Cl)");
			break;
		case 13:
			pGraph->SetXTitle("Re");
			break;
		case 14:
			pGraph->SetXTitle("XCp");
			break;
		default:
			pGraph->SetXTitle("Alpha");
			break;
	}
	switch (iY)
	{
		case 0:
			pGraph->SetYTitle("Alpha");
			break;
		case 1:
			pGraph->SetYTitle("Cl");
			break;
		case 2:
			pGraph->SetYTitle("Cd");
			break;
		case 3:
			pGraph->SetYTitle("Cd x 10000");
			break;
		case 4:
			pGraph->SetYTitle("Cdp");
			break;
		case 5:
			pGraph->SetYTitle("Cm");
			break;
		case 6:
			pGraph->SetYTitle("Xtr1");
			break;
		case 7:
			pGraph->SetYTitle("Xtr2");
			break;
		case 8:
			pGraph->SetYTitle("HMom");
			break;
		case 9:
			pGraph->SetYTitle("Cpmin");
			break;
		case 10:
			pGraph->SetYTitle("Cl/Cd");
			break;
		case 11:
			pGraph->SetYTitle("|Cl|^(3/2)/Cd");
			break;
		case 12:
			pGraph->SetYTitle("1/Rt(Cl)");
			break;
		case 13:
			pGraph->SetYTitle("Re");
			break;
		case 14:
			pGraph->SetYTitle("XCp");
			break;
		default:
			pGraph->SetYTitle("Alpha");
			break;
	}
}



void QXDirect::SetupLayout()
{
	QHBoxLayout *SpecVarsBox = new QHBoxLayout;

	m_pctrlSpec1 = new QRadioButton("a");
	m_pctrlSpec2 = new QRadioButton("Cl");
	m_pctrlSpec3 = new QRadioButton("Re");
	m_pctrlSpec1->setFont(QFont("Symbol"));
	SpecVarsBox->addWidget(m_pctrlSpec1);
	SpecVarsBox->addWidget(m_pctrlSpec2);
	SpecVarsBox->addWidget(m_pctrlSpec3);

	m_pctrlSequence = new QCheckBox("Sequence");

	QGridLayout *SequenceGroup = new QGridLayout;
	QLabel *AlphaMinLab   = new QLabel("Start=");
	QLabel *AlphaMaxLab   = new QLabel("End=");
	QLabel *DeltaAlphaLab = new QLabel("D=");
	DeltaAlphaLab->setFont(QFont("Symbol"));
	DeltaAlphaLab->setAlignment(Qt::AlignRight);
	AlphaMinLab->setAlignment(Qt::AlignRight);
	AlphaMaxLab->setAlignment(Qt::AlignRight);

	m_pctrlAlphaMin     = new FloatEdit();
	m_pctrlAlphaMax     = new FloatEdit();
	m_pctrlAlphaDelta   = new FloatEdit();
	m_pctrlAlphaMin->setMinimumHeight(20);
	m_pctrlAlphaMax->setMinimumHeight(20);
	m_pctrlAlphaDelta->setMinimumHeight(20);
	m_pctrlAlphaMin->setAlignment(Qt::AlignRight);
	m_pctrlAlphaMax->setAlignment(Qt::AlignRight);
	m_pctrlAlphaDelta->setAlignment(Qt::AlignRight);
	SequenceGroup->addWidget(AlphaMinLab,1,1);
	SequenceGroup->addWidget(AlphaMaxLab,2,1);
	SequenceGroup->addWidget(DeltaAlphaLab,3,1);
	SequenceGroup->addWidget(m_pctrlAlphaMin,1,2);
	SequenceGroup->addWidget(m_pctrlAlphaMax,2,2);
	SequenceGroup->addWidget(m_pctrlAlphaDelta,3,2);

	QHBoxLayout *AnalysisSettings = new QHBoxLayout;
	m_pctrlViscous  = new QCheckBox("Viscous");
	m_pctrlInitBL   = new QCheckBox("Init BL");
	AnalysisSettings->addWidget(m_pctrlViscous);
	AnalysisSettings->addWidget(m_pctrlInitBL);

	m_pctrlStoreOpp = new QCheckBox("Store Opp");
	m_pctrlAnalyze  = new QPushButton("Analyze");

	QVBoxLayout *AnalysisGroup = new QVBoxLayout;
	AnalysisGroup->addLayout(SpecVarsBox);
	AnalysisGroup->addStretch(1);
	AnalysisGroup->addWidget(m_pctrlSequence);
	AnalysisGroup->addLayout(SequenceGroup);
	AnalysisGroup->addStretch(1);
	AnalysisGroup->addLayout(AnalysisSettings);
	AnalysisGroup->addWidget(m_pctrlStoreOpp);
	AnalysisGroup->addWidget(m_pctrlAnalyze);

	QGroupBox *AnalysisBox = new QGroupBox("Analysis settings");
	AnalysisBox->setLayout(AnalysisGroup);


	QVBoxLayout *DisplayGroup = new QVBoxLayout;
	m_pctrlShowBL        = new QCheckBox("Show BL");
	m_pctrlShowPressure  = new QCheckBox("Show Pressure");
	m_pctrlAnimate       = new QCheckBox("Animate");
	m_pctrlAnimateSpeed  = new QSlider(Qt::Horizontal);
	m_pctrlAnimateSpeed->setMinimum(0);
	m_pctrlAnimateSpeed->setMaximum(500);
	m_pctrlAnimateSpeed->setSliderPosition(250);
	m_pctrlAnimateSpeed->setTickInterval(25);
	m_pctrlAnimateSpeed->setTickPosition(QSlider::TicksBelow);
	DisplayGroup->addWidget(m_pctrlShowBL);
	DisplayGroup->addWidget(m_pctrlShowPressure);
	DisplayGroup->addWidget(m_pctrlAnimate);
	DisplayGroup->addWidget(m_pctrlAnimateSpeed);
	QGroupBox *DisplayBox = new QGroupBox("Display");
	DisplayBox->setLayout(DisplayGroup);


	QHBoxLayout *CurveDisplay = new QHBoxLayout;
	m_pctrlShowCurve  = new QCheckBox("Curve");
	m_pctrlShowPoints = new QCheckBox("Points");
	CurveDisplay->addWidget(m_pctrlShowCurve);
	CurveDisplay->addWidget(m_pctrlShowPoints);
	QVBoxLayout *CurveGroup = new QVBoxLayout;
	m_pctrlCurveStyle = new LineCbBox();
	m_pctrlCurveWidth = new LineCbBox();
	m_pctrlCurveColor = new LineButton;
	for (int i=0; i<5; i++)
	{
		m_pctrlCurveStyle->addItem("item");
		m_pctrlCurveWidth->addItem("item");
	}
	m_pStyleDelegate = new LineDelegate;
	m_pWidthDelegate = new LineDelegate;
	m_pctrlCurveStyle->setItemDelegate(m_pStyleDelegate);
	m_pctrlCurveWidth->setItemDelegate(m_pWidthDelegate);

	CurveGroup->addLayout(CurveDisplay);
	CurveGroup->addWidget(m_pctrlCurveStyle);
	CurveGroup->addWidget(m_pctrlCurveWidth);
	CurveGroup->addWidget(m_pctrlCurveColor);
	QGroupBox *CurveBox = new QGroupBox("Curve settings");
	CurveBox->setLayout(CurveGroup);


	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addStretch(1);
	mainLayout->addWidget(AnalysisBox);
	mainLayout->addStretch(1);
	mainLayout->addWidget(DisplayBox);
	mainLayout->addStretch(1);
	mainLayout->addWidget(CurveBox);
	mainLayout->addStretch(1);

	setLayout(mainLayout);
}


void QXDirect::StopAnimate()
{
	if(m_bAnimate)
	{
		m_pAnimateTimer->stop();
		m_bAnimate = false;
		m_pctrlAnimate->setChecked(false);
		SetOpp();
	}
}


void QXDirect::UpdateCurve()
{
	MainFrame *pMainFrame = (MainFrame*)m_pMainFrame;

	if(m_bPolar && m_pCurPolar)
	{
		m_pCurPolar->m_Color = m_CurveColor;
		m_pCurPolar->m_Style = m_CurveStyle;
		m_pCurPolar->m_Width = (int)m_CurveWidth;
		CreatePolarCurves();
	}
	else if (!m_bPolar && m_pCurOpp)
	{
		m_pCurOpp->m_Color = m_CurveColor;
		m_pCurOpp->m_Style = m_CurveStyle;
		m_pCurOpp->m_Width = (int)m_CurveWidth;
		CreateOppCurves();
	}

	UpdateView();
	pMainFrame->SetSaveState(false);
}


void QXDirect::UpdateView()
{
	TwoDWidget *p2DWidget = (TwoDWidget*)m_p2DWidget;

	if(m_p2DWidget)
	{
		p2DWidget->update();
	}
}


void QXDirect::wheelEvent (QWheelEvent *event )
{
//	point is in client coordinates

	QPoint pt(event->x(), event->y()); //client coordinates
	m_pCurGraph = GetGraph(pt);

	if(m_pCurGraph && m_pCurGraph->IsInDrawRect(pt) && m_bCpGraph)
	{
/*		SHORT shX = GetKeyState('X');
		SHORT shY = GetKeyState('Y');

		if (shX & 0x8000)
		{
			//zoom x scale
			m_pCurGraph->SetAutoX(false);
			if(zDelta>0) m_pCurGraph->Scalex(1.06);
			else m_pCurGraph->Scalex(1.0/1.06);
		}
		else if(shY & 0x8000)
		{
			//zoom y scale
			m_pCurGraph->SetAutoY(false);
			if(zDelta>0) m_pCurGraph->Scaley(1.06);
			else m_pCurGraph->Scaley(1.0/1.06);
		}
		else*/
		{
			//zoom both
			m_pCurGraph->SetAuto(false);
			if(event->delta()>0) m_pCurGraph->Scale(1.06);
			else                 m_pCurGraph->Scale(1.0/1.06);
		}

		if(!m_bAnimate) UpdateView();
	}
	else if(m_pCurFoil && !m_bPolar)
	{
		double scale = m_fFoilScale;

		if(event->delta()>0) m_fFoilScale /= 1.06;
		else                 m_fFoilScale *= 1.06;

		int a = (int)((m_rCltRect.right()+m_rCltRect.left())/2);

		m_FoilOffset.rx() = a + (int)((m_FoilOffset.x()-a)/scale*m_fFoilScale);

		if(!m_bAnimate) UpdateView();
	}
}
