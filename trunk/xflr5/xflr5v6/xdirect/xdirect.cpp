/****************************************************************************

    XDirect Class
    Copyright (C) 2008-2019 Andre Deperrois

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


#include <QColorDialog>
#include <QFileDialog>
#include <QGroupBox>
#include <QMenu>
#include <QMessageBox>
#include <QStatusBar>
#include <QThread>
#include <QTimer>

#include <globals/mainframe.h>
#include <graph/curve.h>
#include <graph/graphdlg.h>
#include <misc/editplrdlg.h>
#include <misc/line/linebtn.h>
#include <misc/line/linecbbox.h>
#include <misc/line/linedelegate.h>
#include <misc/objectpropsdlg.h>
#include <misc/options/settings.h>
#include <misc/polarfilterdlg.h>
#include <misc/renamedlg.h>
#include <misc/text/doubleedit.h>
#include <misc/text/mintextedit.h>
#include <viewwidgets/graphwidgets/xdirecttilewt.h>
#include <xdirect/analysis/batchctrldlg.h>
#include <xdirect/analysis/batchgraphdlg.h>
#include <xdirect/analysis/batchthreaddlg.h>
#include <xdirect/analysis/foilpolardlg.h>
#include <xdirect/analysis/xfoiladvanceddlg.h>
#include <xdirect/geometry/cadddlg.h>
#include <xdirect/geometry/flapdlg.h>
#include <xdirect/geometry/foilcoorddlg.h>
#include <xdirect/geometry/foilgeomdlg.h>
#include <xdirect/geometry/interpolatefoilsdlg.h>
#include <xdirect/geometry/ledlg.h>
#include <xdirect/geometry/nacafoildlg.h>
#include <xdirect/geometry/tegapdlg.h>
#include <xdirect/geometry/twodpaneldlg.h>
#include <xdirect/objects2d.h>
#include <xdirect/optim2d/optim2d.h>
#include <xdirect/xdirect.h>
#include <xdirect/xdirectstyledlg.h>
#include <xdirect/xml/xmlpolarreader.h>
#include <xdirect/xml/xmlpolarwriter.h>
#include <xflcore/xflcore.h>
#include <xflobjects/objects_global.h>
#include <xinverse/foilselectiondlg.h>


QVector<double> XDirect::s_ReList;
QVector<double> XDirect::s_MachList;
QVector<double> XDirect::s_NCritList;

bool XDirect::s_bViscous = true;
bool XDirect::s_bAlpha = true;
bool XDirect::s_bInitBL = true;
bool XDirect::s_bKeepOpenErrors = true;

double XDirect::s_Re      =  100000.0;
double XDirect::s_ReMax   = 1000000.0;
double XDirect::s_ReDelta =  100000.0;

int XDirect::s_TimeUpdateInterval = 100;

MainFrame *XDirect::s_pMainFrame;
Foil *    XDirect::m_pCurFoil = nullptr;
Polar*    XDirect::m_pCurPolar = nullptr;
OpPoint * XDirect::m_pCurOpp = nullptr;


/**
*The public constructor.
*/
XDirect::XDirect(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    m_pXFADlg = new XFoilAnalysisDlg(this);

    m_pOpPointWidget = nullptr;

    m_LineStyle = {true, Line::SOLID, 1, QColor(0,0,0), Line::NOSYMBOL};

    setupLayout();

    m_pAnimateTimer = new QTimer(this);
    m_posAnimate = 0; // no animation to start with
    connectSignals();

    fillComboBoxes(false);

    m_bAnimate        = false;
    m_bAnimatePlus    = false;
    m_bCpGraph        = true;

    m_bShowUserGraph  = true;
    m_bSequence       = false;

    m_bXPressed = m_bYPressed = false;

    m_bResetCurves    = true;

    m_bTrans          = false;
    m_bType1          = true;
    m_bType2          = true;
    m_bType3          = true;
    m_bType4          = true;
    m_bShowTextOutput = true;
    m_bNeutralLine    = true;
    m_bShowInviscid   = false;
    m_bCurOppOnly     = true;

    m_bPolarView      = true;
    m_iPlrGraph = 0;
    m_iPlrView  = Xfl::ALLGRAPHS;
    m_FoilYPos  = 150;

    m_posAnimate = 0;

    setCurPolar(nullptr);
    setCurOpp(nullptr);


    for(int ig=0; ig<MAXPOLARGRAPHS; ig++)
    {
        m_PlrGraph.append(new Graph);
        m_PlrGraph.at(ig)->setGraphName(QString("Polar_Graph_%1").arg(ig));
        m_PlrGraph.at(ig)->setGraphType(GRAPH::POLARGRAPH);
        m_PlrGraph[ig]->setXMin(0.0);
        m_PlrGraph[ig]->setXMax(0.1);
        m_PlrGraph[ig]->setYMin(-0.1);
        m_PlrGraph[ig]->setYMax(0.1);
        m_PlrGraph[ig]->setScaleType(2);
        m_PlrGraph[ig]->setBorderStyle({true, Line::SOLID, 3, QColor(200,200,200), Line::NOSYMBOL});
        m_PlrGraph[ig]->setMargin(50);
        if(ig==0) m_PlrGraph[ig]->setVariables(2,1);
        if(ig==1) m_PlrGraph[ig]->setVariables(0,1);
        if(ig==2) m_PlrGraph[ig]->setVariables(0,5);
        if(ig==3) m_PlrGraph[ig]->setVariables(6,1);
        if(ig==4) m_PlrGraph[ig]->setVariables(0,10);
        if(ig==5) m_PlrGraph[ig]->setVariables(0,11);
    }
    for(int ig=0; ig<MAXPOLARGRAPHS; ig++) setGraphTitles(m_PlrGraph[ig]);

    m_CpGraph.setGraphType(GRAPH::OPPGRAPH);
    m_CpGraph.setScaleType(1);
    m_CpGraph.setXTitle(tr("X"));
    m_CpGraph.setYTitle(tr("Cp"));
    m_CpGraph.setInverted(true);
    m_CpGraph.setXMin(0.0);
    m_CpGraph.setXMax(1.0);
    m_CpGraph.setYMin(-0.1);
    m_CpGraph.setYMax(0.1);
    m_CpGraph.setMargin(50);
    m_CpGraph.setBorderStyle({true, Line::SOLID, 3, QColor(200,200,200), Line::NOSYMBOL});

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
XDirect::~XDirect()
{
    for(int ig=m_PlrGraph.count()-1; ig>=0; ig--)
    {
        delete m_PlrGraph.at(ig);
        m_PlrGraph.removeAt(ig);
    }
    delete m_pXFADlg;
}


/** Sets the state of the window's widgets i.a.w. the state of the active ojbects and views. */
void XDirect::setControls()
{
    if(m_bPolarView) m_pswMiddleControls->setCurrentIndex(1);
    else             m_pswMiddleControls->setCurrentIndex(0);

    if(m_pCurPolar)
    {
        QString polarProps;
        m_pCurPolar->getPolarProperties(polarProps);
        m_pctrlPolarProps->setPlainText(polarProps);
    }
    else m_pctrlPolarProps->clear();

    s_pMainFrame->m_pOpPointsAct->setChecked(!m_bPolarView);
    s_pMainFrame->m_pPolarsAct->setChecked(m_bPolarView);

    //    s_pMainFrame->m_pShowPanels->setChecked(m_bShowPanels);
    s_pMainFrame->m_pShowNeutralLine->setChecked(m_bNeutralLine);
    s_pMainFrame->m_pShowInviscidCurve->setChecked(m_bShowInviscid);
    s_pMainFrame->m_pShowCurOppOnly->setChecked(m_bCurOppOnly);

    s_pMainFrame->m_psetCpVarGraph->setChecked(m_CpGraph.yVariable()==0);
    s_pMainFrame->m_psetQVarGraph->setChecked(m_CpGraph.yVariable()==1);

    s_pMainFrame->m_pExportBLData->setEnabled(m_pCurOpp);

    m_pchShowPressure->setEnabled(!m_bPolarView && m_pCurOpp);
    m_pchShowBL->setEnabled(!m_bPolarView && m_pCurOpp);
    m_pchAnimate->setEnabled(!m_bPolarView && m_pCurOpp);
    m_pslAnimateSpeed->setEnabled(!m_bPolarView && m_pCurOpp && m_pchAnimate->isChecked());
    //    m_pctrlHighlightOpp->setEnabled(m_bPolar);

    s_pMainFrame->m_pCurrentFoilMenu->setEnabled(m_pCurFoil);
    s_pMainFrame->m_pCurrentFoilMenu_OperFoilCtxMenu->setEnabled(m_pCurFoil);
    s_pMainFrame->m_pCurrentFoilMenu_OperPolarCtxMenu->setEnabled(m_pCurFoil);
    s_pMainFrame->m_pCurrentPolarMenu->setEnabled(m_pCurPolar);
    s_pMainFrame->m_pCurrentPolarMenu_OperFoilCtxMenu->setEnabled(m_pCurPolar);
    s_pMainFrame->m_pCurrentPolarMenu_OperPolarCtxMenu->setEnabled(m_pCurPolar);

    s_pMainFrame->m_pRenameCurFoil->setEnabled(m_pCurFoil);
    s_pMainFrame->m_pDirectDuplicateCurFoil->setEnabled(m_pCurFoil);
    s_pMainFrame->m_pDeleteCurFoil->setEnabled(m_pCurFoil);
    s_pMainFrame->m_pExportCurFoil->setEnabled(m_pCurFoil);
    s_pMainFrame->m_pRenameCurFoil->setEnabled(m_pCurFoil);
    s_pMainFrame->m_pSetCurFoilStyle->setEnabled(m_pCurFoil);
    s_pMainFrame->m_pDefinePolarAct->setEnabled(m_pCurFoil);
    s_pMainFrame->m_pBatchAnalysisAct->setEnabled(m_pCurFoil);
    s_pMainFrame->m_pDeleteFoilOpps->setEnabled(m_pCurFoil);
    s_pMainFrame->m_pDeleteFoilPolars->setEnabled(m_pCurFoil);

    s_pMainFrame->m_pEditCurPolar->setEnabled(m_pCurPolar);
    s_pMainFrame->m_pDeletePolar->setEnabled(m_pCurPolar);
    s_pMainFrame->m_pExportCurPolar->setEnabled(m_pCurPolar);
    s_pMainFrame->m_pHidePolarOpps->setEnabled(m_pCurPolar);
    s_pMainFrame->m_pShowPolarOpps->setEnabled(m_pCurPolar);
    s_pMainFrame->m_pDeletePolarOpps->setEnabled(m_pCurPolar);

    s_pMainFrame->m_pDerotateFoil->setEnabled(m_pCurFoil);
    s_pMainFrame->m_pNormalizeFoil->setEnabled(m_pCurFoil);
    s_pMainFrame->m_pRefineLocalFoil->setEnabled(m_pCurFoil);
    s_pMainFrame->m_pRefineGlobalFoil->setEnabled(m_pCurFoil);
    s_pMainFrame->m_pEditCoordsFoil->setEnabled(m_pCurFoil);
    s_pMainFrame->m_pScaleFoil->setEnabled(m_pCurFoil);
    s_pMainFrame->m_pSetLERadius->setEnabled(m_pCurFoil);
    s_pMainFrame->m_pSetTEGap->setEnabled(m_pCurFoil);
    s_pMainFrame->m_pSetFlap->setEnabled(m_pCurFoil);
    s_pMainFrame->m_pInterpolateFoils->setEnabled(m_pCurFoil);

    s_pMainFrame->m_pCurrentOppMenu->setEnabled(m_pCurOpp);
    s_pMainFrame->m_pDeleteCurOpp->setEnabled(m_pCurOpp);
    s_pMainFrame->m_pExportCurOpp->setEnabled(m_pCurOpp);

    s_pMainFrame->checkGraphActions();

    m_pchAlignChildren->setChecked(Settings::isAlignedChildrenStyle());
}


/**
* Connects signals and slots
*/
void XDirect::connectSignals()
{
    connect(this, SIGNAL(projectModified()), s_pMainFrame, SLOT(onProjectModified()));

    connect(m_prbSpec1,      SIGNAL(clicked()),          SLOT(onSpec()));
    connect(m_prbSpec2,      SIGNAL(clicked()),          SLOT(onSpec()));
    connect(m_prbSpec3,      SIGNAL(clicked()),          SLOT(onSpec()));
    connect(m_ppbAnalyze,    SIGNAL(clicked()),          SLOT(onAnalyze()));
    connect(m_pdeAlphaMin,   SIGNAL(editingFinished()),  SLOT(onInputChanged()));
    connect(m_pdeAlphaMax,   SIGNAL(editingFinished()),  SLOT(onInputChanged()));
    connect(m_pdeAlphaDelta, SIGNAL(editingFinished()),  SLOT(onInputChanged()));
    connect(m_plcbCurveStyle, SIGNAL(activated(int)),     SLOT(onCurveStyle(int)));
    connect(m_plcbCurveWidth, SIGNAL(activated(int)),     SLOT(onCurveWidth(int)));
    connect(m_plcbPointStyle, SIGNAL(activated(int)),     SLOT(onCurvePoints(int)));
    connect(m_plbCurveColor, SIGNAL(clickedLB()),        SLOT(onCurveColor()));
    connect(m_pchSequence,   SIGNAL(clicked()),          SLOT(onSequence()));
    connect(m_pchViscous,    SIGNAL(clicked()),          SLOT(onViscous()));
    connect(m_pchStoreOpp,   SIGNAL(clicked()),          SLOT(onStoreOpp()));

    connect(m_pchShowCurve,     SIGNAL(clicked()),  SLOT(onShowCurve()));
    connect(m_pchAlignChildren, SIGNAL(clicked(bool)),  s_pMainFrame, SLOT(onAlignChildrenStyle(bool)));

    connect(m_pchAnimate,      SIGNAL(clicked(bool)),    SLOT(onAnimate(bool)));
    connect(m_pslAnimateSpeed, SIGNAL(sliderMoved(int)), SLOT(onAnimateSpeed(int)));
    connect(m_pAnimateTimer,     SIGNAL(timeout()),        SLOT(onAnimateSingle()));


    connect(m_pchShowBL,       SIGNAL(clicked(bool)), s_pMainFrame->m_pXDirectTileWidget->opPointWidget(), SLOT(onShowBL(bool)));
    connect(m_pchShowPressure, SIGNAL(clicked(bool)), s_pMainFrame->m_pXDirectTileWidget->opPointWidget(), SLOT(onShowPressure(bool)));
}


/**
* Creates a curve of the Cp graph for a specified OpPoint instance, or for all the instances of OpPoint.
* @param pOpp a pointer to the instance of the operating point, the data of which is used to build the CCurve objects
*/
void XDirect::createOppCurves(OpPoint *pOpp)
{
    OpPoint *pOpPoint = nullptr;
    if(pOpp) pOpPoint = pOpp; else pOpPoint = m_pCurOpp;

    Curve *pCurve1;
    QString str;

    m_CpGraph.deleteCurves();

    /*    if(pOpPoint)
    {
        if(!pOpPoint || !pOpPoint->isVisible()) return;
        pCurve1    = m_CpGraph.addCurve();
        int r,g,b,a;
        pOpPoint->getColor(r,g,b,a);

        pCurve1->setLineStyle(pOpPoint->oppStyle(), pOpPoint->oppWidth(), colour(pOpPoint), pOpPoint->pointStyle(), pOpPoint->isVisible());
        pCurve1->setCurveName(pOpPoint->opPointName());

        fillOppCurve(pOpPoint, &m_CpGraph, pCurve1);

        if(m_bShowInviscid && pOpPoint && m_CpGraph.yVariable()<2)
        {
            Curve *pCpi = m_CpGraph.addCurve();
            pCpi->setPoints(pOpPoint->pointStyle());
            pCpi->setStyle(1);
            pCpi->setColor(colour(pOpPoint).darker(150));
            pCpi->setWidth(pOpPoint->oppWidth());
            str= QString("-Re=%1-Alpha=%2_Inviscid").arg(pOpPoint->Reynolds(),8,'f',0).arg(pOpPoint->aoa(),5,'f',2);
            str = pOpPoint->foilName()+str;
            pCpi->setCurveName(str);
            fillOppCurve(pOpPoint, &m_CpGraph, pCpi, true);
        }
    }*/


    for (int k=0; k<m_poaOpp->size(); k++)
    {
        pOpp = m_poaOpp->at(k);
        bool bShow = pOpp->isVisible();
        if(m_bCurOppOnly && pOpp!=curOpp()) bShow = false;
        if (pOpp && bShow)
        {
            pCurve1    = m_CpGraph.addCurve();

            //                pCurve1->setPoints(pOpp->pointStyle());
            LS2 ls(pOpp->theStyle());
            pCurve1->setLineStyle(ls);
            pCurve1->setName(pOpp->opPointName());

            fillOppCurve(pOpp, &m_CpGraph, pCurve1);

            if(m_bShowInviscid && pOpPoint && m_CpGraph.yVariable()<2 && pOpp==curOpp())
            {
                Curve *pCpi = m_CpGraph.addCurve();
                pCpi->setPointStyle(pOpPoint->pointStyle());
                pCpi->setStipple(1);
                pCpi->setColor(colour(pOpPoint).darker(150));
                pCpi->setWidth(pOpPoint->lineWidth());
                str= QString("-Re=%1-Alpha=%2_Inviscid").arg(pOpPoint->Reynolds(),8,'f',0).arg(pOpPoint->aoa(),5,'f',2);
                str = pOpPoint->foilName()+str;
                pCpi->setName(str);
                fillOppCurve(pOpPoint, &m_CpGraph, pCpi, true);
            }
        }
    }
}


/**
*Creates the curves of the graphs for all the visible polars.
*/
void XDirect::createPolarCurves()
{
    // curves must be entirely reconstructed each time from the
    // operating points database, since user may have added
    // or deleted points & polars

    Polar *pPolar = nullptr;

    for(int ig=0; ig<MAXPOLARGRAPHS; ig++) m_PlrGraph[ig]->deleteCurves();

    for (int k=0; k<m_poaPolar->size(); k++)
    {
        pPolar = m_poaPolar->at(k);

        if (pPolar->isVisible() && pPolar->m_Alpha.size()>0)
        {
            if ((pPolar->polarType()==Xfl::FIXEDSPEEDPOLAR  && m_bType1) ||
                    (pPolar->polarType()==Xfl::FIXEDLIFTPOLAR   && m_bType2) ||
                    (pPolar->polarType()==Xfl::RUBBERCHORDPOLAR && m_bType3) ||
                    (pPolar->polarType()==Xfl::FIXEDAOAPOLAR    && m_bType4))
            {

                Curve* pCurve[MAXPOLARGRAPHS];
                for(int ic=0; ic<MAXPOLARGRAPHS; ic++) pCurve[ic]=nullptr;
                //                Curve* pTr2Curve = nullptr;
                for(int ig=0; ig<MAXPOLARGRAPHS; ig++)
                {
                    pCurve[ig] = m_PlrGraph[ig]->addCurve();
                    pCurve[ig]->setLineStyle(pPolar->polarStyle(), pPolar->lineWidth(), colour(pPolar), pPolar->pointStyle(), pPolar->isVisible());

                    fillPolarCurve(pCurve[ig], pPolar, m_PlrGraph[ig]->xVariable(), m_PlrGraph[ig]->yVariable());
                    pCurve[ig]->setName(pPolar->polarName());

                    /*                    if(m_PlrGraph[ig]->yVariable() == 6)    pTr2Curve = m_PlrGraph[ig]->addCurve();
                    else                                    pTr2Curve = nullptr;
                    if(pTr2Curve)
                    {
                        pTr2Curve->setLineStyle(pPolar->polarStyle(), pPolar->polarWidth(), colour(pPolar), pPolar->pointStyle(), pPolar->isVisible());
                        fillPolarCurve(pTr2Curve, pPolar, m_PlrGraph[ig]->xVariable(), 7);

                        str = pPolar->polarName() + " / Xtr1";
                        pCurve[ig]->setCurveName(str);
                        str = pPolar->polarName() + " / Xtr2";
                        pTr2Curve->setCurveName(str);
                    }*/
                }
            }
        }
    }
}


/**
* Initializes the comboboxes with the active OpPoint or Polar line style
* @param bEnable true if the comboboxes should be enable as a result
*/
void XDirect::fillComboBoxes(bool bEnable)
{
    m_plbCurveColor->setEnabled(bEnable);
    m_plcbCurveStyle->setEnabled(bEnable);
    m_plcbCurveWidth->setEnabled(bEnable);
    m_plcbPointStyle->setEnabled(bEnable);

    m_pchShowCurve->setEnabled(bEnable);
    m_pchAlignChildren->setEnabled(bEnable);

    int LineWidth[5];
    int LineStyle[5];
    int PointStyle[5];

    for (int i=0; i<5;i++)
    {
        LineStyle[i] = m_LineStyle.m_Stipple;
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
    for (int i=0; i<5;i++) PointStyle[i]=i;
    m_pPointDelegate->setPointStyle(PointStyle);
    m_pPointDelegate->setLineColor(m_LineStyle.m_Color);

    if(bEnable)
    {
        m_plcbCurveStyle->setLine( m_LineStyle.m_Stipple, m_LineStyle.m_Width, m_LineStyle.m_Color, m_LineStyle.m_PointStyle);
        m_plcbCurveWidth->setLine( m_LineStyle.m_Stipple, m_LineStyle.m_Width, m_LineStyle.m_Color, m_LineStyle.m_PointStyle);
        m_plcbPointStyle->setLine(m_LineStyle.m_Stipple, m_LineStyle.m_Width, m_LineStyle.m_Color, m_LineStyle.m_PointStyle);
        m_plbCurveColor->setColor(m_LineStyle.m_Color);
        m_plbCurveColor->setStipple(m_LineStyle.m_Stipple);
        m_plbCurveColor->setWidth(m_LineStyle.m_Width);
        m_plbCurveColor->setPointStyle(m_LineStyle.m_PointStyle);
    }
    else
    {
        m_plcbCurveStyle->setLine( 0, 1, QColor(100,100,100), 0);
        m_plcbCurveWidth->setLine( 0, 1, QColor(100,100,100), 0);
        m_plcbPointStyle->setLine(0, 1, QColor(100,100,100), 0);
        m_plbCurveColor->setColor(QColor(100,100,100));
        m_plbCurveColor->setStipple(0);
        m_plbCurveColor->setWidth(1);
        m_plbCurveColor->setPointStyle(0);
    }



    m_plcbCurveStyle->update();
    m_plcbCurveWidth->update();
    m_plcbPointStyle->update();
    m_plbCurveColor->update();

    m_plcbCurveStyle->setCurrentIndex(m_LineStyle.m_Stipple);
    m_plcbCurveWidth->setCurrentIndex(m_LineStyle.m_Width-1);
    m_plcbPointStyle->setCurrentIndex(m_LineStyle.m_PointStyle);
}


/**
 * Fills the Cp graph curve with the data from the OpPoint.
 * @param pOpp a pointer to the OpPoint for which the curve is drawn
 * @param pGraph a pointer to the Graph to which the curve belongs
 * @param pCurve a pointer to the CCurve which will be filled with the data from the OpPoint
 * @param bInviscid true if the inviscid resutls should be displayed, false if the viscous results should be displayed
 */
void XDirect::fillOppCurve(OpPoint *pOpp, Graph *pGraph, Curve *pCurve, bool bInviscid)
{
    Foil *pOpFoil = Objects2d::foil(pOpp->foilName());

    m_CpGraph.resetLimits();
    m_CpGraph.setAuto(true);
    m_CpGraph.setInverted(false);
    switch(m_CpGraph.yVariable())
    {
        case 0:
        {
            m_CpGraph.setInverted(true);
            for (int j=0; j<pOpp->n; j++)
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
            for (int j=0; j<pOpp->n; j++)
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
        case 2:  //shear coeff
        {
            pGraph->setYTitle("sqrt(Max Shear)");
            Curve * pCurve0 = pGraph->addCurve();
            Curve * pCurve1 = pGraph->addCurve();
            Curve * pCurve2 = pGraph->addCurve();
            Curve * pCurve3 = pGraph->addCurve();

            pCurve0->setName("sqrt(Ctau_top)");
            pCurve1->setName("sqrt(CtauEq_top)");
            pCurve2->setName("sqrt(Ctau_bot)");
            pCurve3->setName("sqrt(CtauEq_bot)");

            int it1 = pOpp->blx.itran[1];
            int it2 = pOpp->blx.itran[2];

            for (int i=it1; i<=pOpp->blx.nside1-1; i++) pCurve0->appendPoint(pOpp->blx.xbl[i][1], pOpp->blx.ctau[i][1]);
            for (int i=2;   i<=pOpp->blx.nside1-1; i++) pCurve1->appendPoint(pOpp->blx.xbl[i][1], pOpp->blx.ctq[i][1]);

            for (int i=it2; i<=pOpp->blx.nside2-1; i++) pCurve2->appendPoint(pOpp->blx.xbl[i][2], pOpp->blx.ctau[i][2]);
            for (int i=2;   i<=pOpp->blx.nside2-1; i++) pCurve3->appendPoint(pOpp->blx.xbl[i][2], pOpp->blx.ctq[i][2]);
            break;
        }
        case 3:  //Dstar & theta TOP
        {
            pGraph->setYTitle("D* & Theta Top");

            Curve * pCurve0 = pGraph->addCurve();
            Curve * pCurve1 = pGraph->addCurve();

            pCurve0->setWidth(2);
            pCurve1->setWidth(2);
            pCurve0->setColor(QColor(55,155,75));
            pCurve1->setColor(QColor(55,75,155));

            pCurve0->setName("D*");
            pCurve1->setName("Theta");

            for (int i=2; i<pOpp->blx.nside1; i++)
            {
                pCurve0->appendPoint(pOpp->blx.xbl[i][1], pOpp->blx.dstr[i][1]);
                pCurve1->appendPoint(pOpp->blx.xbl[i][1], pOpp->blx.thet[i][1]);
            }
            break;
        }
        case 4:  //DStar & theta BOT
        {
            qDebug("fillin bottom");
            pGraph->setYTitle("D* & Theta Bot");
            Curve * pCurve0 = pGraph->addCurve();
            Curve * pCurve1 = pGraph->addCurve();

            pCurve0->setName("D*");
            pCurve1->setName("Theta");

            pCurve0->setWidth(2);
            pCurve1->setWidth(2);
            pCurve0->setColor(QColor(55,155,75));
            pCurve1->setColor(QColor(55,75,155));

            for (int i=2; i<pOpp->blx.nside2; i++)
            {
                pCurve0->appendPoint(pOpp->blx.xbl[i][2], pOpp->blx.dstr[i][2]);
                pCurve1->appendPoint(pOpp->blx.xbl[i][2], pOpp->blx.thet[i][2]);
            }
            break;
        }
        case 5:
        {
            pGraph->setYTitle("Log(Re_Theta)");
            Curve * pTopCurve = pGraph->addCurve();
            Curve * pBotCurve = pGraph->addCurve();
            pTopCurve->setName("Top");
            pBotCurve->setName("Bot");

            pTopCurve->setWidth(2);
            pBotCurve->setWidth(2);
            pTopCurve->setColor(QColor(55,155,75));
            pBotCurve->setColor(QColor(55,75,155));

            double y[IVX][3];
            memset(y, 0, 3*IVX*sizeof(double));
            for (int i=2; i<=pOpp->blx.nside1-1; i++){
                if (pOpp->blx.RTheta[i][1]>0.0) y[i][1] = log10( pOpp->blx.RTheta[i][1] );
                else                             y[i][1] = 0.0;
                pTopCurve->appendPoint(pOpp->blx.xbl[i][1], y[i][1]);
            }
            for (int i=2; i<=pOpp->blx.nside2-1; i++){
                if (pOpp->blx.RTheta[i][2]>0.0) y[i][2] = log10( pOpp->blx.RTheta[i][2] );
                else                             y[i][2] = 0.0;
                pBotCurve->appendPoint(pOpp->blx.xbl[i][2], y[i][2]);
            }
            break;
        }
        case 6:
        {
            pGraph->setYTitle(tr("Re_Theta"));
            Curve * pTopCurve = pGraph->addCurve();
            Curve * pBotCurve = pGraph->addCurve();
            pTopCurve->setName("ReTheta_Top");
            pBotCurve->setName("ReTheta_Bot");

            pTopCurve->setWidth(2);
            pBotCurve->setWidth(2);
            pTopCurve->setColor(QColor(55,155,75));
            pBotCurve->setColor(QColor(55,75,155));

            for (int i=2; i<=pOpp->blx.nside1-1; i++) pTopCurve->appendPoint(pOpp->blx.xbl[i][1], pOpp->blx.RTheta[i][1]);
            for (int i=2; i<=pOpp->blx.nside2-1; i++) pBotCurve->appendPoint(pOpp->blx.xbl[i][2], pOpp->blx.RTheta[i][2]);
            break;
        }
        case 7:  //Amplification factor
        {
            pGraph->setYTitle("N/N0");
            Curve * pTopCurve = pGraph->addCurve();
            Curve * pBotCurve = pGraph->addCurve();

            pTopCurve->setName("Top");
            pBotCurve->setName("Bot");

            pTopCurve->setWidth(2);
            pBotCurve->setWidth(2);
            pTopCurve->setColor(QColor(55,155,75));
            pBotCurve->setColor(QColor(55,75,155));

            double y[IVX][3];
            memset(y, 0, 3*IVX*sizeof(double));
            for (int ibl=2; ibl<pOpp->blx.nside1; ibl++)
            {
                y[ibl][1] = pOpp->blx.ctau[ibl][1];
            }
            for (int ibl=2; ibl<pOpp->blx.nside2; ibl++)
            {
                y[ibl][2] = pOpp->blx.ctau[ibl][2];
            }

            for (int i=2; i<=pOpp->blx.itran[1]-2; i++)
            {
                pTopCurve->appendPoint(pOpp->blx.xbl[i][1], y[i][1]);
            }
            for (int i=2; i<=pOpp->blx.itran[2]-2; i++)
            {
                pBotCurve->appendPoint(pOpp->blx.xbl[i][2], y[i][2]);
            }
            break;
        }
        case 8:
        {
            pGraph->setYTitle("Dissipation Coef.");
            double y[IVX][3];
            memset(y, 0, 3*IVX*sizeof(double));
            Curve * pTopCurve = pGraph->addCurve();
            Curve * pBotCurve = pGraph->addCurve();
            pTopCurve->setName("Dissipation-Top");
            pBotCurve->setName("Dissipation-Bot");

            pTopCurve->setWidth(2);
            pBotCurve->setWidth(2);
            pTopCurve->setColor(QColor(55,155,75));
            pBotCurve->setColor(QColor(55,75,155));

            double qrf = pOpp->blx.qinf;

            //---- fill compressible ue arrays
            for (int ibl=2; ibl<= pOpp->blx.nside1;ibl++)
            {
                y[ibl][1] = pOpp->blx.dis[ibl][1] / qrf/ qrf/ qrf;
            }
            for (int ibl=2; ibl<= pOpp->blx.nside2;ibl++)
            {
                y[ibl][2] = pOpp->blx.dis[ibl][2] / qrf/ qrf/ qrf;
            }

            for (int i=2; i<=pOpp->blx.nside1-1; i++)
            {
                pTopCurve->appendPoint(pOpp->blx.xbl[i][1], y[i][1]);
            }
            for (int i=2; i<=pOpp->blx.nside2-1; i++)
            {
                pBotCurve->appendPoint(pOpp->blx.xbl[i][2], y[i][2]);
            }
            break;
        }
        case 9:  //friction coefficient
        {
            pGraph->setYTitle("tau");
            Curve * pTopCurve = pGraph->addCurve();
            Curve * pBotCurve = pGraph->addCurve();
            pTopCurve->setName("Wall_shear_Top");
            pBotCurve->setName("Wall_shear_Bot");

            pTopCurve->setWidth(2);
            pBotCurve->setWidth(2);
            pTopCurve->setColor(QColor(55,155,75));
            pBotCurve->setColor(QColor(55,75,155));

            double que = 0.5*pOpp->blx.qinf*pOpp->blx.qinf;

            double y[IVX][ISX];
            memset(y, 0, IVX*ISX*sizeof(double));
            //---- fill compressible ue arrays
            for (int ibl=2; ibl<= pOpp->blx.nside1;ibl++)
            {
                y[ibl][1] = pOpp->blx.tau[ibl][1] / que;
            }
            for (int ibl=2; ibl<= pOpp->blx.nside2;ibl++)
            {
                y[ibl][2] = pOpp->blx.tau[ibl][2] / que;
            }

            for (int i=2; i<=pOpp->blx.nside1-1; i++)
            {
                pTopCurve->appendPoint(pOpp->blx.xbl[i][1], y[i][1]);
            }
            for (int i=2; i<=pOpp->blx.nside2-1; i++)
            {
                pBotCurve->appendPoint(pOpp->blx.xbl[i][2], y[i][2]);
            }
            break;
        }
        case 10:
        {
            pGraph->setYTitle("Ue");
            Curve * pTopCurve = pGraph->addCurve();
            Curve * pBotCurve = pGraph->addCurve();
            pTopCurve->setName("Top");
            pBotCurve->setName("Bot");

            pTopCurve->setWidth(2);
            pBotCurve->setWidth(2);
            pTopCurve->setColor(QColor(55,155,75));
            pBotCurve->setColor(QColor(55,75,155));

            double y[IVX][3];
            memset(y, 0, 3*IVX*sizeof(double));
            double uei=0;

            //---- fill compressible ue arrays
            for (int ibl=2; ibl<= pOpp->blx.nside1;ibl++)
            {
                uei = pOpp->blx.uedg[ibl][1];
                y[ibl][1] = uei * (1.0-pOpp->blx.tklam)
                        / (1.0-pOpp->blx.tklam*(uei/pOpp->blx.qinf)*(uei/pOpp->blx.qinf));
            }
            for (int ibl=2; ibl<= pOpp->blx.nside2;ibl++)
            {
                uei = pOpp->blx.uedg[ibl][2];
                y[ibl][2] = uei * (1.0-pOpp->blx.tklam)
                        / (1.0-pOpp->blx.tklam*(uei/pOpp->blx.qinf)*(uei/pOpp->blx.qinf));
            }

            for (int i=2; i<=pOpp->blx.nside1-1; i++)
            {
                pTopCurve->appendPoint(pOpp->blx.xbl[i][1], y[i][1]);
            }
            for (int i=2; i<=pOpp->blx.nside2-1; i++)
            {
                pBotCurve->appendPoint(pOpp->blx.xbl[i][2], y[i][2]);
            }
            break;
        }
        case 11: //Hk
        {
            pGraph->setYTitle("Hk");
            Curve * pTopCurve = pGraph->addCurve();
            Curve * pBotCurve = pGraph->addCurve();
            pTopCurve->setName("Top");
            pBotCurve->setName("Bot");

            pTopCurve->setWidth(2);
            pBotCurve->setWidth(2);
            pTopCurve->setColor(QColor(55,155,75));
            pBotCurve->setColor(QColor(55,75,155));

            for (int i=2; i<=pOpp->blx.nside1-1; i++)
            {
                pTopCurve->appendPoint(pOpp->blx.xbl[i][1], pOpp->blx.Hk[i][1]);
            }
            for (int i=2; i<=pOpp->blx.nside2-1; i++)
            {
                pBotCurve->appendPoint(pOpp->blx.xbl[i][2], pOpp->blx.Hk[i][2]);
            }

            break;
        }
        default:
        {
            for (int j=0; j<pOpp->n; j++)
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
void XDirect::fillPolarCurve(Curve *pCurve, Polar *pPolar, int XVar, int YVar)
{
    QVector<double> const *pX = getVariable(pPolar, XVar);
    QVector<double> const *pY = getVariable(pPolar, YVar);
    double fx = 1.0;
    double fy = 1.0;

    pCurve->setSelected(-1);

    if(XVar == 3) fx = 10000.0;
    if(YVar == 3) fy = 10000.0;

    for (int i=0; i<pPolar->m_Alpha.size(); i++)
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

        if(m_pCurOpp && Graph::isHighLighting()
                && m_pCurOpp->polarName()==m_pCurPolar->polarName() && m_pCurOpp->foilName()==m_pCurFoil->name())
        {
            if(qAbs(pPolar->m_Alpha[i]-m_pCurOpp->m_Alpha)<0.0001)
            {
                if(pPolar->polarName()==m_pCurOpp->polarName()  && m_pCurFoil->name()==pPolar->foilName())
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
QVector<double> *XDirect::getVariable(Polar *pPolar, int iVar)
{
    QVector<double> * pVar=nullptr;
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
void XDirect::keyPressEvent(QKeyEvent *pEvent)
{
    bool bShift = false;
    if(pEvent->modifiers() & Qt::ShiftModifier)   bShift =true;
//    bool bCtrl = false;
//    if(pEvent->modifiers() & Qt::ControlModifier)   bCtrl =true;

    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
            if (pEvent->modifiers().testFlag(Qt::AltModifier) & pEvent->modifiers().testFlag(Qt::ShiftModifier))
            {
                onOpPointProps();
                break;
            }
            else if (pEvent->modifiers().testFlag(Qt::AltModifier))
            {
                onPolarProps();
                break;
            }

            readParams();
            if(m_ppbAnalyze->hasFocus())  onAnalyze();
            else
            {
                activateWindow();
                m_ppbAnalyze->setFocus();
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
            if(m_bPolarView && pEvent->modifiers().testFlag(Qt::ControlModifier))
            {
                s_pMainFrame->onHighlightOperatingPoint();
            }
            break;
        }
        case Qt::Key_L:
            s_pMainFrame->onLogFile();
            break;
        case Qt::Key_X:
            m_bXPressed = true;
            break;
        case Qt::Key_Y:
            m_bYPressed = true;
            break;
            /*        case Qt::Key_1:
            if(bCtrl)
            {
                s_pMainFrame->onAFoil();
                pEvent->accept();
                return;
            }
            break;
        case Qt::Key_2:
            if(bCtrl)
            {
                s_pMainFrame->onAFoil();
                pEvent->accept();
                return;
            }
            break;
        case Qt::Key_3:
            if(bCtrl)
            {
                s_pMainFrame->onXInverse();
                pEvent->accept();
                return;
            }
            break;
        case Qt::Key_4:
            if(bCtrl)
            {
                s_pMainFrame->onXInverseMixed();
                pEvent->accept();
                return;
            }
        case Qt::Key_5:
            break;
        case Qt::Key_6:
            if(bCtrl)
            {
                s_pMainFrame->onMiarex();
                pEvent->accept();
                return;
            }
            break;*/

        case Qt::Key_F2:
        {
            if(bShift) onRenameCurPolar();
            else       onRenameCurFoil();
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
/*        case Qt::Key_F6:
        {
            if (pEvent->modifiers().testFlag(Qt::ShiftModifier))        onBatchAnalysis();
            else if (pEvent->modifiers().testFlag(Qt::ControlModifier)) onMultiThreadedBatchAnalysis();
            else                                                        onDefinePolar();
            break;
        }*/
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
        case Qt::Key_F12:
        {
            onOptim2d();
            break;
        }
        default:
            QWidget::keyPressEvent(pEvent);
    }

    pEvent->accept();
}



/**
 * Overrides the QWidget's keyReleaseEvent method.
 * Dispatches the key release event
 * @param event the QKeyEvent
 */
void XDirect::keyReleaseEvent(QKeyEvent *event)
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
void XDirect::loadSettings(QSettings &settings)
{
    QString str1, str2, str3;

    int oppVar = 0;
    settings.beginGroup("XDirect");
    {
        OpPoint::setStoreOpp(settings.value("StoreOpp").toBool());
        s_bAlpha          = settings.value("AlphaSpec").toBool();
        s_bViscous        = settings.value("ViscousAnalysis").toBool();
        s_bInitBL         = settings.value("InitBL").toBool();
        m_bPolarView      = settings.value("PolarView").toBool();
        m_bShowUserGraph  = settings.value("UserGraph").toBool();

        m_bType1          = settings.value("Type1").toBool();
        m_bType2          = settings.value("Type2").toBool();
        m_bType3          = settings.value("Type3").toBool();
        m_bType4          = settings.value("Type4").toBool();
        m_bShowTextOutput = settings.value("TextOutput").toBool();
        m_bNeutralLine    = settings.value("NeutralLine").toBool();
        m_bCurOppOnly     = settings.value("CurOppOnly").toBool();
        m_bShowInviscid   = settings.value("ShowInviscid", false).toBool();
        m_bCpGraph        = settings.value("ShowCpGraph", true).toBool();
        m_bSequence       = settings.value("Sequence", false).toBool();

        s_Re              = settings.value("Reynolds",      s_Re).toDouble();
        s_ReMax           = settings.value("ReynoldsMax",   s_ReMax).toDouble();
        s_ReDelta         = settings.value("ReynoldsDelta", s_ReDelta).toDouble();

        oppVar = settings.value("OppVar",0).toInt();
        s_TimeUpdateInterval = settings.value("TimeUpdateInterval",100).toInt();

        m_iPlrGraph      = settings.value("PlrGraph").toInt();

        switch(settings.value("PlrView").toInt())
        {
            case 1:
                m_iPlrView = Xfl::ONEGRAPH;
                break;
            case 2:
                m_iPlrView = Xfl::TWOGRAPHS;
                break;
            case 4:
                m_iPlrView = Xfl::FOURGRAPHS;
                break;
            default:
                m_iPlrView = Xfl::ALLGRAPHS;
                break;
        }

        m_XFoil.setVAccel(settings.value("VAccel").toDouble());
        s_bKeepOpenErrors = settings.value("KeepOpenErrors").toBool();

        XFoilTask::s_bAutoInitBL    = settings.value("AutoInitBL").toBool();
        XFoilTask::s_IterLim        = settings.value("IterLim", 100).toInt();

        XFoil::setFullReport(settings.value("FullReport").toBool());

        BatchThreadDlg::s_bUpdatePolarView = settings.value("BatchUpdatePolarView", false).toBool();
        BatchThreadDlg::s_nThreads = settings.value("MaxThreads", 12).toInt();

        int NRe = settings.value("NReynolds").toInt();
        s_ReList.clear();
        s_MachList.clear();
        s_NCritList.clear();
        for (int i=0; i<NRe; i++)
        {
            str1 = QString("ReList%1").arg(i);
            str2 = QString("MaList%1").arg(i);
            str3 = QString("NcList%1").arg(i);
            s_ReList.append(settings.value(str1).toDouble());
            s_MachList.append(settings.value(str2).toDouble());
            s_NCritList.append(settings.value(str3).toDouble());
        }
    }
    settings.endGroup();

    for(int ig=0; ig<m_PlrGraph.count(); ig++) m_PlrGraph[ig]->loadSettings(settings);

    m_CpGraph.loadSettings(settings);

    if(oppVar>=2) oppVar=0;
    m_CpGraph.setYVariable(oppVar);

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

    for(int ig=0; ig<MAXPOLARGRAPHS; ig++)
    {
        //        Graph *pGraph = m_PlrGraph[ig];
        setGraphTitles(m_PlrGraph[ig]);
    }
    m_pOpPointWidget->loadSettings(settings);

    XFoilAnalysisDlg::loadSettings(settings);
    FoilPolarDlg::loadSettings(settings);
    Optim2d::loadSettings(settings);
    BatchAbstractDlg::loadSettings(settings);
    BatchCtrlDlg::loadSettings(settings);
}


/**
 * The user has changed one of the analysis parameters. Reads all the data and maps it.
 */
void XDirect::onInputChanged()
{
    readParams();
}


/**
 * The user has clicked the animate checkcbox
 * @param bChecked the new state of the checkbox
 */
void XDirect::onAnimate(bool bChecked)
{
    m_pslAnimateSpeed->setEnabled(bChecked);
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
            pOpPoint = m_poaOpp->at(l);

            if (pOpPoint &&
                    pOpPoint->polarName()  == m_pCurPolar->polarName() &&
                    pOpPoint->foilName() == m_pCurFoil->name())
            {
                if(qAbs(m_pCurOpp->m_Alpha - pOpPoint->aoa())<0.0001)
                    m_posAnimate = l-1;
            }
        }
        m_bAnimate  = true;
        int speed = m_pslAnimateSpeed->value();
        m_pAnimateTimer->setInterval(800-speed);
        m_pAnimateTimer->start();
    }
    else
    {
        m_pAnimateTimer->stop();
        m_bAnimate = false;
        if(m_posAnimate<0 || m_posAnimate>=m_poaOpp->size()) return;
        OpPoint* pOpPoint = m_poaOpp->at(m_posAnimate);
        if(pOpPoint) setOpp(pOpPoint->aoa());
        //        UpdateView();
        return;
    }
}


/**
 * Called by the animation timer.
 * Updates the display with the data of the next OpPoint.
 */
void XDirect::onAnimateSingle()
{
    static int indexCbBox;
    QString str;
    bool bIsValid = false;

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

        pOpPoint = m_poaOpp->at(m_posAnimate);

        if (pOpPoint &&
                pOpPoint->polarName()  == m_pCurPolar->polarName() &&
                pOpPoint->foilName() == m_pCurFoil->name() &&
                pOpPoint != m_pCurOpp)
        {
            bIsValid = true;
            createOppCurves(pOpPoint);
            setCurOpp(pOpPoint);

            //select current OpPoint in Combobox
            if(m_pCurPolar->polarType()!=Xfl::FIXEDAOAPOLAR) str = QString("%1").arg(m_pCurOpp->m_Alpha,8,'f',2);
            else                                                     str = QString("%1").arg(m_pCurOpp->Reynolds(),8,'f',2);
            indexCbBox = s_pMainFrame->m_pcbOpPoint->findText(str);
            if(indexCbBox>=0) s_pMainFrame->m_pcbOpPoint->setCurrentIndex(indexCbBox);

            updateView();
        }
    }
}


/**
 * the user has moved the slider which defines the animation speed
 * @param val the slider's new position
 */
void XDirect::onAnimateSpeed(int val)
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
void XDirect::onAnalyze()
{
    if(!m_pCurFoil || !m_pCurPolar) return;
    //qDebug("cpgraphtype %d", m_CpGraph.scaleType());

    readParams();

    m_ppbAnalyze->setEnabled(false);

    bool bHigh = Graph::isHighLighting();
    Graph::setOppHighlighting(false);

    m_pXFADlg->m_pRmsGraph->copySettings(&Settings::s_RefGraph);

    m_pXFADlg->m_bAlpha = s_bAlpha;

    m_pXFADlg->initDialog();
    m_pXFADlg->show();
    m_pXFADlg->analyze();
    if(!s_bKeepOpenErrors || !m_pXFADlg->m_bErrors) m_pXFADlg->hide();


    // and update window
    emit projectModified();

    m_ppbAnalyze->setEnabled(true);

    s_bInitBL = !m_XFoil.isBLInitialized();
    m_pchInitBL->setChecked(s_bInitBL);;

    s_pMainFrame->updateOppListBox();

    if(s_bAlpha) setOpp(XFoilAnalysisDlg::s_Alpha);
    else         setOpp();

    Graph::setOppHighlighting(bHigh);

    m_bResetCurves = true;

    emit projectModified();

    setControls();
    updateView();
}


/**
 * Launches a single-threaded batch analysis
 */
void XDirect::onBatchAnalysis()
{
    if(!m_pCurFoil) return;

    onPolarView();
    updateView();

    m_ppbAnalyze->setEnabled(false);

    BatchGraphDlg *pBatchDlg = new BatchGraphDlg;
    pBatchDlg->m_pCurFoil     = m_pCurFoil;
    pBatchDlg->m_pRmsGraph->copySettings(&Settings::s_RefGraph);

    pBatchDlg->initDialog();

    if(pBatchDlg->exec()==QDialog::Accepted) emit projectModified();

    delete pBatchDlg;

    setPolar();
    s_pMainFrame->updatePolarListBox();

    m_ppbAnalyze->setEnabled(true);

    setOpp();

    emit projectModified();

    setControls();
    updateView();
}


/**
 * Launches a multi-threaded batch analysis
 */
void XDirect::onMultiThreadedBatchAnalysis()
{
    if(!m_pCurFoil)         return;

    if(QThread::idealThreadCount()<2)
    {
        QString strange = tr("Not enough threads available for multithreading");
        QMessageBox::warning(s_pMainFrame, tr("Warning"), strange);
        return;
    }

    onPolarView();
    updateView();

    m_ppbAnalyze->setEnabled(false);

    BatchThreadDlg *pBatchThreadDlg   = new BatchThreadDlg;
    pBatchThreadDlg->m_pCurFoil  = m_pCurFoil;
    pBatchThreadDlg->initDialog();

    pBatchThreadDlg->exec();

    delete pBatchThreadDlg;

    setPolar();
    s_pMainFrame->updatePolarListBox();

    m_ppbAnalyze->setEnabled(true);


    s_pMainFrame->updateOppListBox();

    setOpp();
    setControls();
    updateView();

    emit projectModified();
}


/**
 * Launches a batch analysis for control polars
 */
void XDirect::onBatchCtrlAnalysis()
{
    if(!m_pCurFoil)         return;

    onPolarView();
    updateView();

    m_ppbAnalyze->setEnabled(false);

    BatchCtrlDlg *pBatchCtrlDlg  = new BatchCtrlDlg;

    pBatchCtrlDlg->m_pCurFoil  = m_pCurFoil;
    pBatchCtrlDlg->initDialog();
    pBatchCtrlDlg->exec();


    delete pBatchCtrlDlg;

    setPolar();
    s_pMainFrame->updatePolarListBox();
    m_ppbAnalyze->setEnabled(true);

    s_pMainFrame->updateOppListBox();

    setOpp();
    setControls();
    updateView();

    emit projectModified();
}


/**
 * The user has requested to switch to the Cp graph view
 */
void XDirect::onCpGraph()
{
    onOpPointView();
    if(m_CpGraph.yVariable()!=0)
    {
        //        m_pCpGraph->ResetLimits();
        m_CpGraph.setAuto(true);
        m_CpGraph.setYVariable(0);
    }

    m_CpGraph.setInverted(true);
    m_bResetCurves = true;
    m_CpGraph.setYTitle(tr("Cp"));

    setControls();
    m_CpGraph.setXScale();
    setFoilScale();
    updateView();
}


/**
 * The user has toggled the request for the display of the inviscid Cp curve
 */
void XDirect::onCpi()
{
    m_bShowInviscid = !m_bShowInviscid;

    m_bResetCurves = true;
    setControls();
    updateView();
}


/**
 * The user has toggled the switch for the display of the current OpPoint only
 */
void XDirect::onCurOppOnly()
{
    m_bCurOppOnly = !m_bCurOppOnly;
    s_pMainFrame->m_pShowCurOppOnly->setChecked(m_bCurOppOnly);

    if(m_pCurOpp) m_pCurOpp->setVisible(true);
    m_bResetCurves = true;
    setAnalysisParams();
    updateView();
}


/**
 * The user has changed the color of the current curve
 */
void XDirect::onCurveColor()
{
    QColor Color = QColorDialog::getColor(m_LineStyle.m_Color);
    if(Color.isValid()) m_LineStyle.m_Color = Color;

    fillComboBoxes();
    updateCurveStyle();
}


/**
 * The user has changed the style of the current curve
 */
void XDirect::onCurveStyle(int index)
{
    m_LineStyle.setStipple(index);
    fillComboBoxes();
    updateCurveStyle();
}


/**
 * The user has changed the width of the current curve
 */
void XDirect::onCurveWidth(int index)
{
    m_LineStyle.m_Width = index+1;
    fillComboBoxes();
    updateCurveStyle();
}


void XDirect::onCurvePoints(int index)
{
    m_LineStyle.setPointStyle(index);
    fillComboBoxes();
    updateCurveStyle();
}


/**
 * The user has requested to define a new polar
 */
void XDirect::onDefinePolar()
{
    if(!m_pCurFoil) return;

    FoilPolarDlg fpDlg(s_pMainFrame);

    fpDlg.initDialog();

    int res = fpDlg.exec();
    if (res == QDialog::Accepted)
    {
        setCurPolar(new Polar());

        if(Settings::isAlignedChildrenStyle())
        {
            m_pCurPolar->setTheStyle(m_pCurFoil->theStyle());
        }
        else
        {
            QColor clr = randomColor(!Settings::isLightTheme());
            m_pCurPolar->setColor(clr.red(), clr.green(), clr.blue(), clr.alpha());
        }

        m_pCurPolar->setFoilName(m_pCurFoil->name());
        m_pCurPolar->setPolarName(fpDlg.m_PlrName);
        m_pCurPolar->setVisible(true);
        m_pCurPolar->copySpecification(&FoilPolarDlg::s_RefPolar);

        Objects2d::addPolar(m_pCurPolar);
        setPolar(m_pCurPolar);

        s_pMainFrame->updatePolarListBox();
        updateView();
        emit projectModified();
    }
    setControls();
}


/**
 * The user has requested the deletion of the current Foil.
 * Deletes the Foil, and selects the next one in the array, if any.
 */
void XDirect::onDeleteCurFoil()
{
    QString strong;
    strong = tr("Are you sure you want to delete")  +"\n"+ m_pCurFoil->name() +"\n";
    strong+= tr("and all associated OpPoints and Polars ?");

    int resp = QMessageBox::question(s_pMainFrame, tr("Question"), strong,  QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    if(resp != QMessageBox::Yes) return;


    Foil*pNextFoil = Objects2d::deleteFoil(m_pCurFoil);
    //    setCurFoil(pNextFoil);
    //    setCurOpp(nullptr);
    //    setCurPolar(nullptr);
    setFoil(pNextFoil);

    s_pMainFrame->updateFoilListBox();

    m_bResetCurves = true;

    emit projectModified();

    setControls();
    updateView();
}



/**
 * The user has requested the deletion of the current OpPoint.
 */
void XDirect::onDelCurOpp()
{
    OpPoint* pOpPoint = m_pCurOpp;
    stopAnimate();

    if (!pOpPoint) return;
    QString strong,str;
    strong = tr("Are you sure you want to delete the Operating Point\n");
    if(m_pCurPolar->polarType()!=Xfl::FIXEDAOAPOLAR) str = QString("Alpha = %1").arg(pOpPoint->aoa(),0,'f',2);
    else                                                     str = QString("Reynolds = %1").arg(pOpPoint->Reynolds(),0,'f',0);
    strong += str;
    strong += "  ?";

    if (QMessageBox::Yes == QMessageBox::question(s_pMainFrame, tr("Question"), strong,
                                                  QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel))
    {
        Objects2d::deleteOpp(m_pCurOpp);
        s_pMainFrame->updateOppListBox();
        setOpp();
        updateView();
    }
    setControls();
}


/**
 * The user has requested the deletion of the current Polar.
 */
void XDirect::onDeleteCurPolar()
{
    if(!m_pCurPolar) return;
    OpPoint *pOpPoint;
    int l;
    QString str;

    str = tr("Are you sure you want to delete the polar :\n  ") + m_pCurPolar->polarName();
    str += tr("\n and all the associated OpPoints ?");

    if (QMessageBox::Yes == QMessageBox::question(s_pMainFrame, tr("Question"), str,
                                                  QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel))
    {
        // start by removing all OpPoints
        for (l=m_poaOpp->size()-1; l>=0; l--)
        {
            pOpPoint = m_poaOpp->at(l);
            if (pOpPoint->polarName()  == m_pCurPolar->polarName() &&
                    pOpPoint->foilName() == m_pCurFoil->name())
            {
                m_poaOpp->removeAt(l);
                delete pOpPoint;
            }
        }
        // then remove the CPolar and update views
        for (l=m_poaPolar->size()-1; l>=0; l--)
        {
            if(m_pCurPolar == m_poaPolar->at(l))
            {
                m_poaPolar->removeAt(l);
                delete m_pCurPolar;
                break;
            }
        }
        setCurOpp(nullptr);
        setCurPolar(nullptr);
    }

    s_pMainFrame->updatePolarListBox();
    setPolar();

    emit projectModified();
    updateView();
}


/**
 * The user has requested the deletion of the OpPoints associated to the current Polar.
 */
void XDirect::onDeletePolarOpps()
{
    if(!m_pCurFoil || !m_pCurPolar) return;

    OpPoint *pOpp;

    for(int i=m_poaOpp->size()-1; i>=0; i--)
    {
        pOpp = m_poaOpp->at(i);
        if(pOpp->foilName()==m_pCurFoil->name() && pOpp->polarName()==m_pCurPolar->polarName())
        {
            m_poaOpp->removeAt(i);
            delete pOpp;
        }
    }

    setCurOpp(nullptr);
    emit projectModified();

    s_pMainFrame->updateOppListBox();
    m_bResetCurves = true;
    setCurveParams();
    setControls();
    updateView();
}


/**
 * The user has requested the deletion of the OpPoints associated to the current Foil.
 */
void XDirect::onDeleteFoilOpps()
{
    if(!m_pCurFoil || !m_pCurPolar) return;

    OpPoint *pOpp;

    for(int i=m_poaOpp->size()-1; i>=0; i--)
    {
        pOpp = m_poaOpp->at(i);
        if(pOpp->foilName()==m_pCurFoil->name())
        {
            m_poaOpp->removeAt(i);
            delete pOpp;
        }
    }
    setCurOpp(nullptr);

    s_pMainFrame->updateOppListBox();
    m_bResetCurves = true;
    setCurveParams();
    setControls();
    updateView();

    emit projectModified();
}


/**
 * The user has requested the deletion of the Polars associated to the current Foil.
 */
void XDirect::onDeleteFoilPolars()
{
    if(!m_pCurFoil) return;
    int l;
    OpPoint *pOpPoint;
    stopAnimate();

    QString strong;

    strong = tr("Are you sure you want to delete polars and OpPoints\n");
    strong +=tr("associated to ")+m_pCurFoil->name()  + " ?";
    if (QMessageBox::Yes == QMessageBox::question(s_pMainFrame, tr("Question"), strong,
                                                  QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel))

    {
        // start by removing all OpPoints
        for (l=m_poaOpp->size()-1; l>=0; l--)
        {
            pOpPoint = m_poaOpp->at(l);
            if (pOpPoint->foilName() == m_pCurFoil->name())
            {
                m_poaOpp->removeAt(l);
                delete pOpPoint;
            }
        }
        // then remove CPolar and update views
        Polar* pPolar;
        for (l=m_poaPolar->size()-1; l>=0; l--)
        {
            pPolar = m_poaPolar->at(l);
            if (pPolar->foilName() == m_pCurFoil->name())
            {
                m_poaPolar->removeAt(l);
                delete pPolar;
            }
        }
        setCurOpp(nullptr);

    }
    setCurPolar(nullptr);
    setPolar();

    m_bResetCurves = true;

    s_pMainFrame->updatePolarListBox();

    emit projectModified();

    setControls();
    updateView();
}


/**
 * The user has requested a local refinement of the panels of the current foil
 */
void XDirect::onCadd()
{
    stopAnimate();
    if(!m_pCurFoil)    return;
    onOpPointView();

    Foil *pCurFoil = curFoil(); //keep a reference to restore eventually
    Foil *pNewFoil = new Foil();
    pNewFoil->copyFoil(curFoil());

    OpPoint* pOpPoint = m_pCurOpp;
    setCurOpp(nullptr);
    m_bResetCurves = true;

    setFoil(pNewFoil);

    CAddDlg caDlg(s_pMainFrame);
    caDlg.m_pBufferFoil = pNewFoil;
    caDlg.m_pMemFoil    = pCurFoil;
    caDlg.initDialog();
    int psState = pNewFoil->pointStyle();
    if(psState==0) pNewFoil->setPointStyle(Line::LITTLECIRCLE);
    updateView();

    if(QDialog::Accepted == caDlg.exec())
    {
        pNewFoil->setPointStyle(psState);
        setRandomFoilColor(pNewFoil, !Settings::isLightTheme());
        setCurOpp(pOpPoint);

        if(addNewFoil(pNewFoil))
        {
            setFoil(pNewFoil);
            s_pMainFrame->updateFoilListBox();
            emit projectModified();
            updateView();
            return;
        }
    }

    //reset everything
    setFoil(pCurFoil);
    setCurOpp(pOpPoint);

    if(m_pCurFoil)
        m_XFoil.initXFoilGeometry(m_pCurFoil->n, m_pCurFoil->x, m_pCurFoil->y, m_pCurFoil->nx, m_pCurFoil->ny);
    delete pNewFoil;

    updateView();
}


/**
 * The user has requested that the foil be derotated
 */
void XDirect::onDerotateFoil()
{
    if(!m_pCurFoil) return;
    QString str;
    stopAnimate();
    Foil *pCurFoil = curFoil();
    Foil *pNewFoil = new Foil;
    pNewFoil->copyFoil(m_pCurFoil);
    setRandomFoilColor(pNewFoil, !Settings::isLightTheme());
    setCurFoil(pNewFoil);
    updateView();

    double angle = pNewFoil->deRotate();
    str = QString(tr("The foil has been de-rotated by %1 degrees")).arg(angle,6,'f',3);
    s_pMainFrame->statusBar()->showMessage(str);

    if(addNewFoil(pNewFoil))
    {
        setFoil(pNewFoil);
        s_pMainFrame->updateFoilListBox();
        emit projectModified();
        updateView();
        return;
    }
    //restore things
    delete pNewFoil;
    setFoil(pCurFoil);
    updateView();
}



/**
 * The user has requested that the length of the current foil be normalized to 1.
 */
void XDirect::onNormalizeFoil()
{
    if(!m_pCurFoil) return;
    QString str;
    stopAnimate();
    Foil *pCurFoil = curFoil();
    Foil *pNewFoil = new Foil;
    pNewFoil->copyFoil(m_pCurFoil);
    setRandomFoilColor(pNewFoil, !Settings::isLightTheme());
    setCurFoil(pNewFoil);
    updateView();

    double length = pNewFoil->normalizeGeometry();
    str = QString(tr("The foil has been normalized from %1  to 1.000")).arg(length,7,'f',3);
    s_pMainFrame->statusBar()->showMessage(str);
    if(addNewFoil(pNewFoil))
    {
        setFoil(pNewFoil);
        s_pMainFrame->updateFoilListBox();
        emit projectModified();
        updateView();
        return;
    }
    //restore things
    delete pNewFoil;
    setCurFoil(pCurFoil);
    if(m_pCurFoil) m_XFoil.initXFoilGeometry(m_pCurFoil->n, m_pCurFoil->x, m_pCurFoil->y, m_pCurFoil->nx, m_pCurFoil->ny);
    updateView();
}



/**
 * The user has requested to modify the parameters of the active polar
 */
void XDirect::onEditCurPolar()
{
    if (!m_pCurPolar) return;

    Polar *pMemPolar = new Polar;
    pMemPolar->copyPolar(m_pCurPolar);

    EditPlrDlg epDlg(s_pMainFrame);
    epDlg.initDialog(this, m_pCurPolar, nullptr, nullptr);

    LS2 style(m_pCurPolar->theStyle());

    m_pCurPolar->setPointStyle(Line::LITTLECIRCLE);

    m_bResetCurves = true;
    updateView();

    if(epDlg.exec() == QDialog::Accepted)
    {
        emit projectModified();
    }
    else
    {
        m_pCurPolar->copyPolar(pMemPolar);
    }
    m_pCurPolar->setStipple(style.m_Stipple);
    m_pCurPolar->setWidth(style.m_Width);
    m_pCurPolar->setColor(style.m_Color.red(), style.m_Color.green(), style.m_Color.blue());
    m_pCurPolar->setPointStyle(style.m_PointStyle);
    m_pCurPolar->setVisible(style.m_bIsVisible);
    m_bResetCurves = true;
    updateView();

    delete pMemPolar;
}


/**
 * The user has requested the export of the current results stored in the XFoil object to a text file
 */
void XDirect::onExportBLData()
{
    if(!m_pCurOpp || m_pCurOpp->blx.nside1==0) return;
    if(!m_pCurFoil) return;

    QString fileName,  OutString, strong;

    double xBL[IVX][ISX], UeVinf[IVX][ISX], Cf[IVX][ISX], Cd[IVX][ISX], AA0[IVX][ISX];
    double DStar[IVX][ISX], Theta[IVX][ISX];
    double uei;
    double que = 0.5*m_XFoil.QInf()*m_XFoil.QInf();
    double qrf = m_XFoil.QInf();
    int nside1, nside2, ibl;
    Xfl::enumTextFileType type = Xfl::TXT;

    fileName = m_pCurFoil->name();
    fileName.replace("/", " ");

    fileName = QFileDialog::getSaveFileName(this, tr("Export Current XFoil Results"),
                                            Settings::s_LastDirName,
                                            tr("Text File (*.txt);;Comma Separated Values (*.csv)"));

    if(!fileName.length()) return;
    int pos = fileName.lastIndexOf("/");
    if(pos>0) Settings::s_LastDirName = fileName.left(pos);

    pos  = fileName.lastIndexOf(".csv");
    if(pos>0) type = Xfl::CSV;

    QFile destFile(fileName);

    if (!destFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

    QTextStream out(&destFile);

    out << VERSIONNAME;
    out << ("\n");
    strong = m_pCurFoil->name()+ "\n";
    out << (strong);

    if(type==Xfl::TXT)
        strong = QString("Alpha = %1,  Re = %2,  Ma= %3,  ACrit=%4\n\n")
                .arg(m_pCurOpp->aoa(), 5, 'f',1)
                .arg(m_pCurOpp->Reynolds(), 8, 'f',0)
                .arg(m_pCurOpp->Mach(), 6, 'f',4)
                .arg(m_pCurOpp->ACrit, 4, 'f',1);
    else
        strong = QString("Alpha =, %1,Re =, %3,Ma=, %3,ACrit =,%4\n\n")
                .arg(m_pCurOpp->aoa(), 5, 'f',1)
                .arg(m_pCurOpp->Reynolds(), 8, 'f',0)
                .arg(m_pCurOpp->Mach(), 6, 'f',4)
                .arg(m_pCurOpp->ACrit, 4, 'f',1);
    out << (strong);


    nside1 = m_pCurOpp->blx.nside1;
    nside2 = m_pCurOpp->blx.nside2;

    for (ibl=2; ibl<= nside1;ibl++)    xBL[ibl][1] = m_pCurOpp->blx.xbl[ibl][1];
    for (ibl=2; ibl<= nside2;ibl++)    xBL[ibl][2] = m_pCurOpp->blx.xbl[ibl][2];

    //write top first
    for (ibl=2; ibl<= nside1;ibl++)
    {
        uei = m_pCurOpp->blx.uedg[ibl][1];
        UeVinf[ibl][1] = uei * (1.0-m_pCurOpp->blx.tklam)
                / (1.0-m_pCurOpp->blx.tklam*(uei/m_pCurOpp->blx.qinf)*(uei/m_pCurOpp->blx.qinf));
    }
    for (ibl=2; ibl<= nside2;ibl++)
    {
        uei = m_pCurOpp->blx.uedg[ibl][2];
        UeVinf[ibl][2] = uei * (1.0-m_pCurOpp->blx.tklam)
                / (1.0-m_pCurOpp->blx.tklam*(uei/m_pCurOpp->blx.qinf)*(uei/m_pCurOpp->blx.qinf));
    }
    //---- fill compressible ue arrays
    for (ibl=2; ibl<= nside1;ibl++)    Cf[ibl][1] = m_pCurOpp->blx.tau[ibl][1] / que;
    for (ibl=2; ibl<= nside2;ibl++)    Cf[ibl][2] = m_pCurOpp->blx.tau[ibl][2] / que;

    //---- fill compressible ue arrays
    for (ibl=2; ibl<= nside1;ibl++)    Cd[ibl][1] = m_pCurOpp->blx.dis[ibl][1] / qrf/ qrf/ qrf;
    for (ibl=2; ibl<= nside2;ibl++)    Cd[ibl][2] = m_pCurOpp->blx.dis[ibl][2] / qrf/ qrf/ qrf;
    //NPlot
    for (ibl=2; ibl< nside1;ibl++)    AA0[ibl][1] = m_pCurOpp->blx.ctau[ibl][1];
    for (ibl=2; ibl< nside2;ibl++)    AA0[ibl][2] = m_pCurOpp->blx.ctau[ibl][2];

    for (ibl=2; ibl<= nside1; ibl++)
    {
        DStar[ibl][1] = m_pCurOpp->blx.dstr[ibl][1];
        Theta[ibl][1] = m_pCurOpp->blx.thet[ibl][1];
    }
    for (ibl=2; ibl<= nside2; ibl++)
    {
        DStar[ibl][2] = m_pCurOpp->blx.dstr[ibl][2];
        Theta[ibl][2] = m_pCurOpp->blx.thet[ibl][2];
    }

    out << tr("\nTop Side\n");
    if(type==Xfl::TXT) OutString = QString(tr("    x         Hk     Ue/Vinf      Cf        Cd     A/A0       D*       Theta      CTq\n"));
    else                 OutString = QString(tr("x,Hk,Ue/Vinf,Cf,Cd,A/A0,D*,Theta,CTq\n"));
    out << (OutString);
    for (ibl=2; ibl<nside1; ibl++)
    {
        if(type==Xfl::TXT)
            OutString = QString("%1  %2  %3  %4 %5 %6  %7  %8  %9\n")
                    .arg(xBL[ibl][1],8,'f',5)
                    .arg(m_pCurOpp->blx.Hk[ibl][1],8,'f',5)
                    .arg(UeVinf[ibl][1],8,'f',5)
                    .arg(Cf[ibl][1],8,'f',5)
                    .arg(Cd[ibl][1],8,'f',5)
                    .arg(AA0[ibl][1],8,'f',5)
                    .arg(DStar[ibl][1],8,'f',5)
                    .arg(Theta[ibl][1],8,'f',5)
                    .arg(m_pCurOpp->blx.ctq[ibl][1],8,'f',5);
        else
            OutString = QString("%1, %2, %3, %4, %5, %6, %7, %8, %9\n")
                    .arg(xBL[ibl][1],8,'f',5)
                    .arg(m_pCurOpp->blx.Hk[ibl][1],8,'f',5)
                    .arg(UeVinf[ibl][1],8,'f',5)
                    .arg(Cf[ibl][1],8,'f',5)
                    .arg(Cd[ibl][1],8,'f',5)
                    .arg(AA0[ibl][1],8,'f',5)
                    .arg(DStar[ibl][1],8,'f',5)
                    .arg(Theta[ibl][1],8,'f',5)
                    .arg(m_pCurOpp->blx.ctq[ibl][1],8,'f',5);
        out << (OutString);
    }
    out << tr("\n\nBottom Side\n");
    if(type==Xfl::TXT) OutString = QString(tr("    x         Hk     Ue/Vinf      Cf        Cd     A/A0       D*       Theta      CTq\n"));
    else        OutString = QString(tr("x,Hk,Ue/Vinf,Cf,Cd,A/A0,D*,Theta,CTq\n"));
    out << (OutString);
    for (ibl=2; ibl<nside2; ibl++)
    {
        if(type==Xfl::TXT)
            OutString = QString("%1  %2  %3  %4 %5 %6  %7  %8  %9\n")
                    .arg(xBL[ibl][2],8,'f',5)
                    .arg(m_pCurOpp->blx.Hk[ibl][2],8,'f',5)
                    .arg(UeVinf[ibl][2],8,'f',5)
                    .arg(Cf[ibl][2],8,'f',5)
                    .arg(Cd[ibl][2],8,'f',5)
                    .arg(AA0[ibl][2],8,'f',5)
                    .arg(DStar[ibl][2],8,'f',5)
                    .arg(Theta[ibl][2],8,'f',5)
                    .arg(m_pCurOpp->blx.ctq[ibl][2],8,'f',5);
        else
            OutString = QString("%1, %2, %3, %4, %5, %6, %7, %8, %9\n")
                    .arg(xBL[ibl][2],8,'f',5)
                    .arg(m_pCurOpp->blx.Hk[ibl][2],8,'f',5)
                    .arg(UeVinf[ibl][2],8,'f',5)
                    .arg(Cf[ibl][2],8,'f',5)
                    .arg(Cd[ibl][2],8,'f',5)
                    .arg(AA0[ibl][2],8,'f',5)
                    .arg(DStar[ibl][2],8,'f',5)
                    .arg(Theta[ibl][2],8,'f',5)
                    .arg(m_pCurOpp->blx.ctq[ibl][2],8,'f',5);
        out << (OutString);
    }

    destFile.close();
}


/**
 * The user has requested the export of all polars to text files
 */
void XDirect::onExportAllPolarsTxt()
{
    QString DirName;
    //select the directory for output
    DirName = QFileDialog::getExistingDirectory(this,  tr("Export Directory"), Settings::s_LastDirName);
    onExportAllPolarsTxt(DirName, Settings::s_ExportFileType);
}


void XDirect::onExportAllPolarsTxt(QString DirName, Xfl::enumTextFileType exporttype)
{
    QString FileName;
    QFile XFile;
    QTextStream out(&XFile);

    for(int l=0; l<m_poaPolar->size(); l++)
    {
        Polar *pPolar = m_poaPolar->at(l);
        FileName = DirName + "/" + pPolar->foilName() + "_" + pPolar->polarName();
        if(Settings::s_ExportFileType==Xfl::TXT) FileName += ".txt";
        else                                       FileName += ".csv";

        XFile.setFileName(FileName);
        if (XFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            pPolar->exportPolar(out, VERSIONNAME, exporttype);
            XFile.close();
        }
        else
        {
            QString strange = tr("Could not write to the directory:") + DirName;
            QMessageBox::warning(s_pMainFrame, tr("Warning"), strange);
            return;
        }
    }
}


/**
 * The user has requested the export of all polars to .plr files
 */
void XDirect::onExportAllFoilPolars()
{
    if(!m_poaFoil->size() || !m_poaPolar->size()) return;

    QString FileName;
    FileName = ".plr";
    FileName.replace("/", " ");

    FileName = QFileDialog::getSaveFileName(this, tr("Polar File"), Settings::plrDirName()+"/"+FileName, tr("Polar File (*.plr)"));
    if(!FileName.length()) return;

    QString strong = FileName.right(4);
    if(strong !=".plr" && strong !=".PLR") FileName += ".plr";

    QFile XFile(FileName);
    if (!XFile.open(QIODevice::WriteOnly)) return;

    int pos = FileName.lastIndexOf("/");
    if(pos>0) Settings::setPlrDirName(FileName.left(pos));

    QDataStream ar(&XFile);
#if QT_VERSION >= 0x040500
    ar.setVersion(QDataStream::Qt_4_5);
#endif
    ar.setByteOrder(QDataStream::LittleEndian);

    FoilSelectionDlg dlg(s_pMainFrame);
    dlg.initDialog(Objects2d::pOAFoil(), QStringList());

    if(m_pCurFoil)
        dlg.setFoilName(m_pCurFoil->name());

    if(dlg.exec()==QDialog::Accepted)
        s_pMainFrame->saveFoilPolars(ar, dlg.foilList());

    XFile.close();
}


/**
 * The user has requested the creation of a .plr file with the Polars of the active Foil object.
 */
void XDirect::onSaveFoilPolars()
{
    if(!m_pCurFoil || !m_poaPolar->size()) return;

    QString FileName;
    FileName = m_pCurFoil->name() + ".plr";
    FileName.replace("/", " ");

    FileName = QFileDialog::getSaveFileName(this, tr("Polar File"), Settings::plrDirName()+"/"+FileName, tr("Polar File (*.plr)"));
    if(!FileName.length()) return;

    QString strong = FileName.right(4);
    if(strong !=".plr" && strong !=".PLR") FileName += ".plr";

    QFile XFile(FileName);
    if (!XFile.open(QIODevice::WriteOnly)) return;

    int pos = FileName.lastIndexOf("/");
    if(pos>0) Settings::setPlrDirName(FileName.left(pos));

    QDataStream ar(&XFile);
#if QT_VERSION >= 0x040500
    ar.setVersion(QDataStream::Qt_4_5);
#endif
    ar.setByteOrder(QDataStream::LittleEndian);

    QVector<Foil*> foilList = {m_pCurFoil};
    s_pMainFrame->saveFoilPolars(ar, foilList);

    XFile.close();
}


/**
 * The user has requested the export of the current foil to a text file
 */
void XDirect::onExportCurFoil()
{
    if(!m_pCurFoil)    return;

    QString FileName;

    FileName = m_pCurFoil->name();
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

    m_pCurFoil->exportFoil(out);
    XFile.close();
}


/**
 * The user has requested the export of the current OpPoint to a text file
 */
void XDirect::onExportCurOpp()
{
    if(!m_pCurFoil || !m_pCurPolar || !m_pCurOpp)    return;

    QString FileName;

    QString filter;
    if(Settings::s_ExportFileType==Xfl::TXT) filter = "Text File (*.txt)";
    else                                       filter = "Comma Separated Values (*.csv)";

    FileName = QFileDialog::getSaveFileName(this, tr("Export OpPoint"),
                                            Settings::s_LastDirName ,
                                            tr("Text File (*.txt);;Comma Separated Values (*.csv)"),
                                            &filter);
    if(!FileName.length()) return;

    int pos = FileName.lastIndexOf("/");
    if(pos>0) Settings::s_LastDirName = FileName.left(pos);
    pos = FileName.lastIndexOf(".csv");
    if (pos>0) Settings::s_ExportFileType = Xfl::CSV;
    else       Settings::s_ExportFileType = Xfl::TXT;

    QFile XFile(FileName);

    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

    QTextStream out(&XFile);

    m_pCurOpp->exportOpp(out, VERSIONNAME, Settings::s_ExportFileType, m_pCurFoil);
    XFile.close();
}


/**
 * The user has requested the export of the OpPoints associated to the current Polar to a text file
 */
void XDirect::onExportPolarOpps()
{
    if(!m_poaPolar->size())
    {
        QMessageBox::warning(s_pMainFrame, tr("Warning"), "No Operating Points to export to file");
        return;
    }

    int i,j;
    QString FileName;

    QString filter;
    if(Settings::s_ExportFileType==Xfl::TXT) filter = "Text File (*.txt)";
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
    if (pos>0) Settings::s_ExportFileType = Xfl::CSV;
    else       Settings::s_ExportFileType = Xfl::TXT;

    QFile XFile(FileName);

    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

    QTextStream out(&XFile);


    QString Header, strong;
    out<<VERSIONNAME;
    out<<"\n\n";
    strong = m_pCurFoil->name() + "\n";
    out << strong;

    OpPoint *pOpPoint;

    for (i=0; i<m_poaOpp->size(); i++)
    {
        pOpPoint = m_poaOpp->at(i);
        if(pOpPoint->foilName() == m_pCurPolar->foilName() && pOpPoint->polarName() == m_pCurPolar->polarName() )
        {
            if(Settings::s_ExportFileType==Xfl::TXT)
                strong = QString("Reynolds = %1   Mach = %2  NCrit = %3\n")
                        .arg(pOpPoint->Reynolds(), 7, 'f', 0)
                        .arg(pOpPoint->m_Mach, 4,'f',0)
                        .arg(pOpPoint->ACrit, 3, 'f',1);
            else
                strong = QString("Reynolds =, %1,Mach =, %2,NCrit =, %3\n")
                        .arg(pOpPoint->Reynolds(), 7, 'f', 0)
                        .arg(pOpPoint->m_Mach, 4,'f',0)
                        .arg(pOpPoint->ACrit, 3, 'f',1);

            out<<strong;
            if(Settings::s_ExportFileType==1) Header = QString("  Alpha        Cd        Cl        Cm        XTr1      XTr2   TEHMom    Cpmn\n");
            else        Header = QString("Alpha,Cd,Cl,Cm,XTr1,XTr2,TEHMom,Cpmn\n");
            out<<Header;

            if(Settings::s_ExportFileType==Xfl::TXT)
                strong = QString("%1   %2   %3   %4   %5   %6   %7  %8\n")
                        .arg(pOpPoint->aoa(),7,'f',3)
                        .arg(pOpPoint->Cd,9,'f',3)
                        .arg(pOpPoint->Cl,7,'f',3)
                        .arg(pOpPoint->Cm,7,'f',3)
                        .arg(pOpPoint->Xtr1,7,'f',3)
                        .arg(pOpPoint->Xtr2,7,'f',3)
                        .arg(pOpPoint->m_TEHMom,7,'f',4)
                        .arg(pOpPoint->Cpmn,7,'f',4);
            else
                strong = QString("%1,%2,%3,%4,%5,%6,%7,%8\n")
                        .arg(pOpPoint->aoa(),7,'f',3)
                        .arg(pOpPoint->Cd,9,'f',3)
                        .arg(pOpPoint->Cl,7,'f',3)
                        .arg(pOpPoint->Cm,7,'f',3)
                        .arg(pOpPoint->Xtr1,7,'f',3)
                        .arg(pOpPoint->Xtr2,7,'f',3)
                        .arg(pOpPoint->m_TEHMom,7,'f',4)
                        .arg(pOpPoint->Cpmn,7,'f',4);

            out<<strong;
            if(Settings::s_ExportFileType==Xfl::TXT) out<< " Cpi          Cpv\n-----------------\n";
            else                                       out << "Cpi,Cpv\n";

            for (j=0; j<pOpPoint->n; j++)
            {
                if(pOpPoint->m_bViscResults)
                {
                    if(Settings::s_ExportFileType==Xfl::TXT) strong = QString("%1   %2\n").arg(pOpPoint->Cpi[j], 7,'f',4).arg(pOpPoint->Cpv[j], 7, 'f',4);
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
void XDirect::onExportCurPolar()
{
    if(!m_pCurFoil || !m_pCurPolar)    return;

    QString FileName, filter;

    if(Settings::s_ExportFileType==Xfl::TXT) filter = "Text File (*.txt)";
    else                                       filter = "Comma Separated Values (*.csv)";

    FileName = m_pCurPolar->polarName();
    FileName.replace("/", " ");
    FileName = QFileDialog::getSaveFileName(this, tr("Export Polar"),
                                            Settings::s_LastDirName + "/"+FileName,
                                            tr("Text File (*.txt);;Comma Separated Values (*.csv)"),
                                            &filter);
    if(!FileName.length()) return;

    int pos = FileName.lastIndexOf("/");
    if(pos>0) Settings::s_LastDirName = FileName.left(pos);
    pos = FileName.lastIndexOf(".csv");
    if (pos>0) Settings::s_ExportFileType = Xfl::CSV;
    else       Settings::s_ExportFileType = Xfl::TXT;

    QFile XFile(FileName);

    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

    QTextStream out(&XFile);

    m_pCurPolar->exportPolar(out, VERSIONNAME, Settings::s_ExportFileType);
    XFile.close();
}


/**
 * The user has requested an edition of the current foil coordinates
 */
void XDirect::onFoilCoordinates()
{
    if(!m_pCurFoil)    return;
    stopAnimate();
    onOpPointView();

    Foil *pCurFoil = curFoil();
    Foil *pNewFoil = new Foil;
    pNewFoil->copyFoil(pCurFoil);
    pNewFoil->setPointStyle(1);
    OpPoint* pOpPoint = m_pCurOpp;
    setCurOpp(nullptr);
    m_bResetCurves = true;

    updateView();

    bool bFlap       = pCurFoil->m_bTEFlap;
    double FlapAngle = pCurFoil->m_TEFlapAngle;
    double Xh        = pCurFoil->m_TEXHinge;
    double Yh        = pCurFoil->m_TEXHinge;

    pNewFoil->m_bTEFlap = false;

    setCurFoil(pNewFoil);
    updateView();

    FoilCoordDlg fcoDlg(s_pMainFrame);
    fcoDlg.m_pMemFoil    = pCurFoil;
    fcoDlg.m_pBufferFoil = pNewFoil;
    fcoDlg.initDialog();

    if(QDialog::Accepted == fcoDlg.exec())
    {
        pNewFoil->m_bTEFlap = bFlap;
        pNewFoil->m_TEFlapAngle = FlapAngle;
        pNewFoil->m_TEXHinge = Xh;
        pNewFoil->m_TEYHinge = Yh;

        setRandomFoilColor(pNewFoil, !Settings::isLightTheme());

        setCurOpp(pOpPoint);

        if(addNewFoil(pNewFoil)) setFoil(pNewFoil);
        else
        {
            //reset everything
            setFoil(pCurFoil);
            setCurOpp(pOpPoint);
            m_XFoil.initXFoilGeometry(pCurFoil->n, pCurFoil->x, pCurFoil->y, pCurFoil->nx, pCurFoil->ny);
            delete pNewFoil;
        }

        s_pMainFrame->updateFoilListBox();
        emit projectModified();
    }
    else
    {
        //reset everything
        setCurFoil(pCurFoil);
        setCurOpp(pOpPoint);
        if(m_pCurFoil)
            m_XFoil.initXFoilGeometry(m_pCurFoil->n, m_pCurFoil->x, m_pCurFoil->y, m_pCurFoil->nx, m_pCurFoil->ny);
        delete pNewFoil;
    }

    curFoil()->setHighLight(-1);

    updateView();
}


/**
 * The user has requested to perform an edition of the current foil's thickness and camber properties.
 */
void XDirect::onFoilGeom()
{
    if(!m_pCurFoil)    return;

    stopAnimate();
    onOpPointView();

    Foil *pCurFoil = curFoil();
    Foil *pNewFoil = new Foil();
    pNewFoil->copyFoil(pCurFoil);
    setCurFoil(pNewFoil);

    OpPoint* pOpPoint = m_pCurOpp;
    setCurOpp(nullptr);
    m_bResetCurves = true;
    updateView();

    FoilGeomDlg fgeDlg(s_pMainFrame);
    fgeDlg.m_pMemFoil = pCurFoil;
    fgeDlg.m_pBufferFoil = pNewFoil;
    fgeDlg.initDialog();

    if(fgeDlg.exec() == QDialog::Accepted)
    {
        setRandomFoilColor(pNewFoil, !Settings::isLightTheme());
        setCurOpp(pOpPoint);

        if(addNewFoil(pNewFoil))
        {
            setFoil(pNewFoil);
            s_pMainFrame->updateFoilListBox();
            emit projectModified();
            updateView();
            return;
        }
    }

    delete pNewFoil;
    setCurFoil(pCurFoil);
    setCurOpp(pOpPoint);
    if(m_pCurFoil) m_XFoil.initXFoilGeometry(m_pCurFoil->n, m_pCurFoil->x, m_pCurFoil->y, m_pCurFoil->nx, m_pCurFoil->ny);

    updateView();
}



/**
 * The user has requested to hide all OpPoints
 */
void XDirect::onHideAllOpps()
{
    OpPoint *pOpp;
    for (int i=0; i<m_poaOpp->size(); i++)
    {
        pOpp = m_poaOpp->at(i);
        pOpp->setVisible(false);
    }
    emit projectModified();
    m_bResetCurves = true;
    setAnalysisParams();
    setCurveParams();
    updateView();
}


/**
 * The user has requested to hide all polar curves
 */
void XDirect::onHideAllPolars()
{
    for (int i=0; i<m_poaPolar->size(); i++)
    {
        Polar *pPolar = m_poaPolar->at(i);
        pPolar->setVisible(false);
    }
    emit projectModified();
    m_bResetCurves = true;
    setCurveParams();
    updateView();
}


/**
 * The user has requested to hide all polar curves associated to the current Foil
 */
void XDirect::onHideFoilPolars()
{
    if(!m_pCurFoil) return;
    for (int i=0; i<m_poaPolar->size(); i++)
    {
        Polar *pPolar = m_poaPolar->at(i);
        if(pPolar->foilName() == m_pCurFoil->name())
        {
            pPolar->setVisible(false);
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
void XDirect::onHideFoilOpps()
{
    if(!m_pCurFoil || !m_pCurPolar) return;

    for(int i=0; i<m_poaOpp->size(); i++)
    {
        OpPoint *pOpp = m_poaOpp->at(i);
        if(pOpp->foilName()==m_pCurFoil->name())
            pOpp->setVisible(false);
    }
    emit projectModified();
    m_bResetCurves = true;
    setCurveParams();
    updateView();
}


/**
 * The user has requested to hide all OpPoint curves associated to the current Polar
 */
void XDirect::onHidePolarOpps()
{
    if(!m_pCurFoil || !m_pCurPolar) return;

    OpPoint *pOpp;

    for(int i=0; i<m_poaOpp->size(); i++)
    {
        pOpp = m_poaOpp->at(i);
        if(pOpp->foilName()==m_pCurFoil->name() && pOpp->polarName()==m_pCurPolar->polarName())
            pOpp->setVisible(false);
    }
    emit projectModified();
    m_bResetCurves = true;
    setCurveParams();
    updateView();
}




/**
 * Imports the analysis definition from an XML file
 */
void XDirect::onImportXFoilPolars()
{
    QStringList pathNames;
    pathNames = QFileDialog::getOpenFileNames(this, tr("Open File"),
                                              Settings::s_LastDirName,
                                              tr("XFoil Polar Format (*.*)"));

    if(!pathNames.size()) return ;
    int pos = pathNames.at(0).lastIndexOf("/");
    if(pos>0) Settings::s_xmlDirName = pathNames.at(0).left(pos);

    Polar *pPolar = nullptr;
    for(int iFile=0; iFile<pathNames.size(); iFile++)
    {
        QFile XFile(pathNames.at(iFile));
        pPolar = importXFoilPolar(XFile);
    }

    setCurOpp(nullptr);
    setPolar(pPolar);
    s_pMainFrame->updatePolarListBox();
    updateView();
    emit projectModified();
}


/**
 * The user has requested to import a polar from a text file.
 * The Polar will be added to the array only if a Foil with the parent name exists.
 */
Polar * XDirect::importXFoilPolar(QFile & txtFile)
{
    Polar *pPolar = new Polar;
    double Re, alpha, CL, CD, CDp, CM, Xt, Xb,Cpmn, HMom;
    QString FoilName, strong, str;
    // JX-mod
    Foil *pFoil;


    if (!txtFile.open(QIODevice::ReadOnly))
    {
        QString strange = tr("Could not read the file\n")+txtFile.fileName();
        QMessageBox::warning(s_pMainFrame, tr("Warning"), strange);
        return nullptr;
    }

    QTextStream in(&txtFile);
    int Line;
    bool bOK, bOK2, bRead;
    Line = 0;

    bRead  = ReadAVLString(in, Line, strong);// XFoil or XFLR5 version
    bRead  = ReadAVLString(in, Line, strong);// Foil Name

    FoilName = strong.right(strong.length()-22);
    FoilName = FoilName.trimmed();

    if(!Objects2d::foil(FoilName))
    {
        str = tr("No Foil with the name ")+FoilName;
        str+= tr("\ncould be found. The polar(s) will not be stored");
        delete pPolar;
        QMessageBox::warning(s_pMainFrame, tr("Warning"), str);
        return nullptr;
    }
    pPolar->setFoilName(FoilName);

    bRead  = ReadAVLString(in, Line, strong);// analysis type

    pPolar->setReType(strong.midRef(0,2).toInt(&bOK));
    pPolar->setMaType(strong.midRef(2,2).toInt(&bOK2));
    if(!bOK || !bOK2)
    {
        str = QString("Error reading line %1: Unrecognized Mach and Reynolds type.\nThe polar(s) will not be stored").arg(Line);
        delete pPolar;
        QMessageBox::warning(s_pMainFrame, tr("Warning"), str);
        return nullptr;
    }
    if     (pPolar->ReType() ==1 && pPolar->MaType() ==1) pPolar->setPolarType(Xfl::FIXEDSPEEDPOLAR);
    else if(pPolar->ReType() ==2 && pPolar->MaType() ==2) pPolar->setPolarType(Xfl::FIXEDLIFTPOLAR);
    else if(pPolar->ReType() ==3 && pPolar->MaType() ==1) pPolar->setPolarType(Xfl::RUBBERCHORDPOLAR);
    else                                                  pPolar->setPolarType(Xfl::FIXEDSPEEDPOLAR);


    ReadAVLString(in, Line, strong);
    if(strong.length() < 34)
    {
        str = QString("Error reading line %1. The polar(s) will not be stored").arg(Line);
        delete pPolar;
        QMessageBox::warning(s_pMainFrame, tr("Warning"), str);
        return nullptr;
    }

    pPolar->setXtrBot(strong.midRef(9,6).toDouble(&bOK));
    if(!bOK)
    {
        str = QString("Error reading Bottom Transition value at line %1. The polar(s) will not be stored").arg(Line);
        delete pPolar;
        QMessageBox::warning(s_pMainFrame, tr("Warning"), str);
        return nullptr;
    }

    pPolar->setXtrTop(strong.midRef(28,6).toDouble(&bOK));
    if(!bOK)
    {
        str = QString("Error reading Top Transition value at line %1. The polar(s) will not be stored").arg(Line);
        delete pPolar;
        QMessageBox::warning(s_pMainFrame, tr("Warning"), str);
        return nullptr;
    }

    // Mach     Re     NCrit
    ReadAVLString(in, Line, strong);// blank line
    if(strong.length() < 50)
    {
        str = QString("Error reading line %1. The polar(s) will not be stored").arg(Line);
        delete pPolar;
        QMessageBox::warning(s_pMainFrame, tr("Warning"), str);
        return nullptr;
    }

    pPolar->setMach(strong.midRef(8,6).toDouble(&bOK));
    if(!bOK)
    {
        str = QString("Error reading Mach Number at line %1. The polar(s) will not be stored").arg(Line);
        delete pPolar;
        QMessageBox::warning(s_pMainFrame, tr("Warning"), str);
        return nullptr;
    }

    Re = strong.midRef(24,10).toDouble(&bOK);
    if(!bOK)
    {
        str = QString("Error reading Reynolds Number at line %1. The polar(s) will not be stored").arg(Line);
        delete pPolar;
        QMessageBox::warning(s_pMainFrame, tr("Warning"), str);
        return nullptr;
    }
    Re *=1000000.0;

    pPolar->setNCrit(strong.mid(52,8).toDouble(&bOK));
    if(!bOK)
    {
        str = QString("Error reading NCrit at line %1. The polar(s) will not be stored").arg(Line);
        delete pPolar;
        QMessageBox::warning(s_pMainFrame, tr("Warning"), str);
        return nullptr;
    }
    pPolar->setReynolds(Re);

    bRead  = ReadAVLString(in, Line, strong);// column titles
    bRead  = ReadAVLString(in, Line, strong);// underscores

    while(bRead)
    {
        bRead  = ReadAVLString(in, Line, strong);// polar data
        if(bRead)
        {
            if(strong.length())
            {
                //                textline = strong.toLatin1();
                //                text = textline.constData();
                //                res = sscanf(text, "%lf%lf%lf%lf%lf%lf%lf%lf%lf", &alpha, &CL, &CD, &CDp, &CM, &Xt, &Xb, &Cpmn, &HMom);

                //Do this the C++ way
                QStringList values;
#if QT_VERSION >= 0x050F00
            values = values = strong.split(" ", Qt::SkipEmptyParts);
#else
    values = values = strong.split(" ", QString::SkipEmptyParts);
#endif

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
    txtFile.close();

    // jx-mod Use xflr5 standards for naming and coloring of a new polar

    /*
        Re = pPolar->Reynolds()/1000000.0;
        QString strange = QString("T%1_Re%2_M%3")
                .arg(pPolar->polarType())
                .arg(Re,0,'f',3)                        // jx-mod Re has 3 decimals
                .arg(pPolar->Mach(),0,'f',2);
        str = QString("_N%1").arg(pPolar->NCrit(),0,'f',1);
        strange += str + "_Imported";

        pPolar->setPolarName(strange);
        QColor clr = MainFrame::getColor(1);
        pPolar->setColor(clr.red(), clr.green(), clr.blue());
    */

    pPolar->setAutoPolarName();
    pPolar->setPolarName(pPolar->polarName() + "_Imported");

    if(Settings::isAlignedChildrenStyle())
    {
        pFoil = Objects2d::foil(FoilName);
        pPolar->setTheStyle(pFoil->theStyle());
    }
    else
    {
        QColor clr = randomColor(!Settings::isLightTheme());
        pPolar->setColor(clr.red(), clr.green(), clr.blue(), clr.alpha());
    }

    Objects2d::addPolar(pPolar);
    return pPolar;
}


/**
 * The user has requested to import a polar from a text file in JavaFoil format
 * The Polar will be added to the array only if a Foil with the parent name exists.
 *  @todo Note: this option has not been tested in years... the JavaFoil format may have changed since
 */
void XDirect::onImportJavaFoilPolar()
{
    QString FoilName;
    QString strong, str;

    QString PathName;
    bool bOK;
    QByteArray textline;
    const char *text;

    PathName = QFileDialog::getOpenFileName(s_pMainFrame, tr("Open File"),
                                            Settings::s_LastDirName,
                                            tr("JavaFoil Polar Format (*.*)"));
    if(!PathName.length())        return ;
    int pos = PathName.lastIndexOf("/");
    if(pos>0) Settings::s_LastDirName = PathName.left(pos);

    QFile XFile(PathName);
    if (!XFile.open(QIODevice::ReadOnly))
    {
        QString strange = tr("Could not read the file\n")+PathName;
        QMessageBox::warning(s_pMainFrame, tr("Warning"), strange);
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

    if(!Objects2d::foil(FoilName))
    {
        str = tr("No Foil with the name ")+FoilName;
        str+= tr("\ncould be found. The polar(s) will not be stored");
        QMessageBox::warning(s_pMainFrame, tr("Warning"), str);
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
            pPolar->setFoilName(FoilName);
            pPolar->setReynolds(Re);
            pPolar->setPolarName(QString("T%1_Re2_M3_JavaFoil")
                                 .arg(pPolar->polarType())
                                 .arg(pPolar->Reynolds()/1000000.0,0,'f',2)
                                 .arg(pPolar->Mach(),0,'f',2));

            QColor clr = MainFrame::getColor(1);
            pPolar->setColor(clr.red(), clr.green(), clr.blue());
            Objects2d::addPolar(pPolar);
            setCurPolar(pPolar);
            NPolars++;

            if(!ReadAVLString(in, Line, strong)) break;//?    Cl    Cd    Cm 0.25    TU    TL    SU    SL    L/D
            if(!ReadAVLString(in, Line, strong)) break;//[?]    [-]    [-]    [-]    [-]    [-]    [-]    [-]    [-]

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
                    if (res == 6)     pPolar->addPoint(alpha, CD, 0.0, CL, CM, Xt, Xb, 0.0, 0.0, Re,0.0);
                }
                else
                {
                    res = 0;
                }
            }
        }
        setCurOpp(nullptr);
        setPolar();
        s_pMainFrame->updatePolarListBox();
        updateView();
        emit projectModified();
    }
}



/**
 * The user has requested the launch of the interface to create a foil from the interpolation of two existing Foil objects.
 */
void XDirect::onInterpolateFoils()
{
    if(m_poaFoil->size()<2)
    {
        QMessageBox::warning(s_pMainFrame, tr("Warning"), tr("At least two foils are required"));
        return;
    }

    stopAnimate();

    onOpPointView();

    Foil *pCurFoil = curFoil();
    Foil *pNewFoil = new Foil();
    pNewFoil->copyFoil(curFoil());
    setCurFoil(pNewFoil);

    InterpolateFoilsDlg ifDlg(s_pMainFrame);
    ifDlg.m_pBufferFoil = pNewFoil;
    ifDlg.initDialog();

    updateView();

    if(ifDlg.exec() == QDialog::Accepted)
    {
        setRandomFoilColor(pNewFoil, !Settings::isLightTheme());
        pNewFoil->setName(ifDlg.m_NewFoilName);

        if(addNewFoil(pNewFoil))
        {
            setFoil(pNewFoil);
            s_pMainFrame->updateFoilListBox();
            emit projectModified();
            updateView();
            return;
        }
    }

    //restore things
    setCurFoil(pCurFoil);
    if(m_pCurFoil)
        m_XFoil.initXFoilGeometry(m_pCurFoil->n, m_pCurFoil->x, m_pCurFoil->y, m_pCurFoil->nx, m_pCurFoil->ny);
    delete pNewFoil;
    updateView();
}



/**
 * The user has requested the launch of the interface used to create a NACA type foil.
 */
void XDirect::onNacaFoils()
{
    stopAnimate();
    onOpPointView();

    Foil* pCurFoil = m_pCurFoil;
    OpPoint* pCurOpp  = m_pCurOpp;
    setCurFoil(nullptr);
    setCurOpp(nullptr);

    m_bResetCurves = true;

    updateView();

    Foil *pNacaFoil = new Foil;
    pNacaFoil->setName("Naca0009");
    m_XFoil.naca4(9, 50);
    for (int j=0; j< m_XFoil.nb; j++)
    {
        pNacaFoil->xb[j] = m_XFoil.xb[j+1];
        pNacaFoil->yb[j] = m_XFoil.yb[j+1];
        pNacaFoil->x[j]  = m_XFoil.xb[j+1];
        pNacaFoil->y[j]  = m_XFoil.yb[j+1];
    }
    pNacaFoil->nb = m_XFoil.nb;
    pNacaFoil->n = m_XFoil.nb;
    pNacaFoil->initFoil();

    setFoil(pNacaFoil);
    updateView();

    NacaFoilDlg nacaDlg(s_pMainFrame);
    nacaDlg.m_pBufferFoil = pNacaFoil;

    if (nacaDlg.exec() == QDialog::Accepted)
    {
        QString str;
        if(nacaDlg.s_Digits>0 && log10(double(nacaDlg.s_Digits))<4)
            str = QString("%1").arg(nacaDlg.s_Digits,4,10,QChar('0'));
        else
            str = QString("%1").arg(nacaDlg.s_Digits);
        str = "NACA "+ str;

        setRandomFoilColor(pNacaFoil, !Settings::isLightTheme());
        pNacaFoil->setName(str);

        setCurOpp(pCurOpp);

        if(addNewFoil(pNacaFoil))
        {
            setFoil(pNacaFoil);
            s_pMainFrame->updateFoilListBox();
            emit projectModified();
            updateView();
            return;
        }
    }
    //reset everything
    setCurFoil(pCurFoil);
    setCurOpp(pCurOpp);
    if(m_pCurFoil) m_XFoil.initXFoilGeometry(m_pCurFoil->n, m_pCurFoil->x, m_pCurFoil->y, m_pCurFoil->nx, m_pCurFoil->ny);
    delete pNacaFoil;
    updateView();
}





/**
 * The user has requested to switch to the OpPoint view
 */
void XDirect::onOpPointView()
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
void XDirect::onPolarView()
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
void XDirect::onPolarFilter()
{
    PolarFilterDlg pfDlg(s_pMainFrame);
    pfDlg.m_bMiarex = false;
    pfDlg.m_bType1 = m_bType1;
    pfDlg.m_bType2 = m_bType2;
    pfDlg.m_bType3 = m_bType3;
    pfDlg.m_bType4 = m_bType4;
    pfDlg.initDialog();

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
void XDirect::onRefinePanelsGlobally()
{
    if(!m_pCurFoil)    return;
    stopAnimate();

    onOpPointView();

    Foil*pCurFoil = curFoil();

    Foil *pNewFoil = new Foil;
    pNewFoil->copyFoil(pCurFoil);
    setFoil(pNewFoil);

    OpPoint* pOpPoint = m_pCurOpp;
    setCurOpp(nullptr);
    m_bResetCurves = true;

    TwoDPanelDlg tdpDlg(s_pMainFrame);
    tdpDlg.m_pBufferFoil = pNewFoil;
    tdpDlg.m_pMemFoil    = pCurFoil;
    int psState = pNewFoil->pointStyle();
    if(psState==0)    pNewFoil->setPointStyle(1);

    updateView();

    tdpDlg.initDialog();

    if(QDialog::Accepted == tdpDlg.exec())
    {
        pNewFoil->setPointStyle(psState);
        setRandomFoilColor(pNewFoil, !Settings::isLightTheme());
        setCurOpp(pOpPoint);
        if(addNewFoil(pNewFoil))
        {
            setFoil(pNewFoil);
            s_pMainFrame->updateFoilListBox();
            emit projectModified();
            updateView();
            return;
        }
    }
    //reset everything
    setFoil(pCurFoil);
    setCurOpp(pOpPoint);
    m_XFoil.initXFoilGeometry(pCurFoil->n, pCurFoil->x, pCurFoil->y, pCurFoil->nx, pCurFoil->ny);
    delete pNewFoil;
    updateView();
}


/**
 * The user has requested the display of the velocity in the Cp graph.
 */
void XDirect::onQGraph()
{
    onOpPointView();
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

    m_CpGraph.setXScale();
    setFoilScale();
    updateView();
}



/**
 * The user has requested to rename the Polar
 */
void XDirect::onRenameCurPolar()
{
    if(!m_pCurPolar) return;
    if(!m_pCurFoil) return;

    int resp, k,l;
    Polar* pPolar = nullptr;
    OpPoint * pOpp;
    QString OldName = m_pCurPolar->polarName();

    QStringList NameList;
    for(k=0; k<m_poaPolar->size(); k++)
    {
        pPolar = m_poaPolar->at(k);
        if(pPolar->foilName() == m_pCurFoil->name())
            NameList.append(pPolar->polarName());
    }

    RenameDlg renDlg(s_pMainFrame);
    renDlg.initDialog(&NameList, m_pCurPolar->polarName(), tr("Enter the new name for the foil polar :"));

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
                pPolar = m_poaPolar->at(k);
                if ((pPolar->foilName()==m_pCurFoil->name()) && (pPolar->polarName() == renDlg.newName()))
                {
                    bExists = true;
                    break;
                }
            }
            if(!bExists)
            {
                for (l=m_poaOpp->size()-1;l>=0; l--)
                {
                    pOpp = m_poaOpp->at(l);
                    if (pOpp->polarName() == OldName &&
                            pOpp->foilName() == m_pCurFoil->name())
                    {
                        pOpp->setPolarName(renDlg.newName());
                    }
                }
                m_pCurPolar->setPolarName(renDlg.newName());
            }
            emit projectModified();
        }
        else if(resp ==10)
        {//user wants to overwrite
            if (OldName == renDlg.newName()) return;
            for (k=0; k<m_poaPolar->size(); k++)
            {
                pPolar = m_poaPolar->at(k);
                if (pPolar->polarName() == renDlg.newName())
                {
                    bExists = true;
                    break;
                }
            }
            for (l=m_poaOpp->size()-1;l>=0; l--)
            {
                pOpp = m_poaOpp->at(l);
                if (pOpp->polarName() == m_pCurPolar->polarName())
                {
                    m_poaOpp->removeAt(l);

                    if(pOpp==m_pCurOpp)
                        setCurOpp(nullptr);

                    delete pOpp;
                }
            }
            m_poaPolar->removeAt(k);
            if(pPolar==m_pCurPolar) setCurPolar(nullptr);
            delete pPolar;

            //and rename everything
            if(m_pCurPolar)
                m_pCurPolar->setPolarName(renDlg.newName());

            for (l=m_poaOpp->size()-1;l>=0; l--)
            {
                pOpp = m_poaOpp->at(l);
                if (pOpp->polarName() == OldName &&
                        pOpp->foilName() == m_pCurFoil->name())
                {
                    pOpp->setPolarName(renDlg.newName());
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
    //    setCurPolar(nullptr);
    //    setCurOpp(nullptr);
    //    SetPolar();
    s_pMainFrame->updatePolarListBox();
    updateView();
}


/**
 *The user has requested the display of the detailed properties of the active OpPoint object.
 */
void XDirect::onOpPointProps()
{
    if(!m_pCurOpp) return;
    ObjectPropsDlg opDlg(s_pMainFrame);
    QString strangeProps;
    m_pCurOpp->getOppProperties(strangeProps, m_pCurFoil);
    opDlg.initDialog(tr("Operating point properties"), strangeProps);
    opDlg.exec();
}


/**
 *The user has requested the display of the detailed properties of the active Polar object.
 */
void XDirect::onPolarProps()
{
    if(!m_pCurPolar) return;
    ObjectPropsDlg opDlg(s_pMainFrame);
    QString strangeProps;
    m_pCurPolar->getPolarProperties(strangeProps);
    opDlg.initDialog(tr("Polar properties"), strangeProps);
    opDlg.exec();
}


/**
 * The user has requested to reset all polar graph scales to their automatic default value
 */
void XDirect::onResetAllPolarGraphsScales()
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
void XDirect::onResetCurPolar()
{
    if(!m_pCurPolar) return;
    m_pCurPolar->resetPolar();

    OpPoint*pOpp;
    for(int i=m_poaOpp->size()-1;i>=0;i--)
    {
        pOpp = m_poaOpp->at(i);
        if(pOpp->foilName()==m_pCurFoil->name() && pOpp->polarName()==m_pCurPolar->polarName())
        {
            m_poaOpp->removeAt(i);
            delete pOpp;
        }
    }
    setCurOpp(nullptr);

    s_pMainFrame->updateOppListBox();

    m_bResetCurves = true;
    updateView();

    emit projectModified();
}


/**
 * The user has toggled the switch for a sequential analysis.
 */
void XDirect::onSequence()
{
    m_bSequence = m_pchSequence->isChecked();
    setOpPointSequence();
}


/**
 * The user has requested the launch of the interface to define a L.E. or T.E. flap.
 */
void XDirect::onSetFlap()
{
    if(!m_pCurFoil) return;
    stopAnimate();
    onOpPointView();

    Foil *pCurFoil =curFoil();
    Foil *pNewFoil = new Foil();
    pNewFoil->copyFoil(curFoil());
    setCurFoil(pNewFoil);

    OpPoint *pOpPoint = m_pCurOpp;
    setCurOpp(nullptr);
    m_bResetCurves = true;

    FlapDlg flpDlg(s_pMainFrame);
    flpDlg.m_pBufferFoil  = pNewFoil;
    flpDlg.m_pMemFoil     = pCurFoil;
    flpDlg.initDialog();

    if(QDialog::Accepted == flpDlg.exec())
    {
        //        pNewFoil->copyFoil(curFoil());
        setRandomFoilColor(pNewFoil, !Settings::isLightTheme());

        setCurOpp(pOpPoint);

        if(addNewFoil(pNewFoil))
        {
            setFoil(pNewFoil);
            s_pMainFrame->updateFoilListBox();
            emit projectModified();
            updateView();
            return;
        }
    }

    //reset everything
    setCurFoil(pCurFoil);
    setCurOpp(pOpPoint);

    if(m_pCurFoil)  m_XFoil.initXFoilGeometry(m_pCurFoil->n, m_pCurFoil->x, m_pCurFoil->y, m_pCurFoil->nx, m_pCurFoil->ny);

    delete pNewFoil;
    updateView();
}


/**
 * The user has requested the launch of the interface to modify the radius of the Foil's leading edge.
 */
void XDirect::onSetLERadius()
{
    if(!m_pCurFoil)    return;
    stopAnimate();
    onOpPointView();

    Foil *pCurFoil = curFoil();
    Foil *pNewFoil = new Foil();
    pNewFoil->copyFoil(curFoil());
    setCurFoil(pNewFoil);

    OpPoint *pOpPoint = m_pCurOpp;
    setCurOpp(nullptr);
    m_bResetCurves = true;

    LEDlg lDlg(s_pMainFrame);
    lDlg.m_pBufferFoil = pNewFoil;
    lDlg.m_pMemFoil    = pCurFoil;
    lDlg.initDialog();

    if(QDialog::Accepted == lDlg.exec())
    {
        setRandomFoilColor(pNewFoil, !Settings::isLightTheme());
        setCurOpp(pOpPoint);

        if(addNewFoil(pNewFoil))
        {
            setFoil(pNewFoil);
            s_pMainFrame->updateFoilListBox();
            emit projectModified();
            updateView();
            return;
        }
    }
    //reset everything
    setCurFoil(pCurFoil);
    setCurOpp(pOpPoint);

    if(m_pCurFoil)
        m_XFoil.initXFoilGeometry(m_pCurFoil->n, m_pCurFoil->x, m_pCurFoil->y, m_pCurFoil->nx, m_pCurFoil->ny);
    delete pNewFoil;
    updateView();
}


/**
 * The user has requested the launch of the interface to modify the gap at the Foil's trailing edge.
 */
void XDirect::onSetTEGap()
{
    if(!m_pCurFoil)    return;
    stopAnimate();
    onOpPointView();

    Foil *pNewFoil = new Foil();
    Foil *pCurFoil = curFoil();
    pNewFoil->copyFoil(pCurFoil);
    OpPoint *pOpPoint = m_pCurOpp;
    setCurOpp(nullptr);
    m_bResetCurves = true;

    setCurFoil(pNewFoil);

    TEGapDlg tegDlg(s_pMainFrame);
    tegDlg.m_pBufferFoil = pNewFoil;
    tegDlg.m_pMemFoil    = pCurFoil;
    tegDlg.m_Gap         = m_pCurFoil->TEGap();
    tegDlg.initDialog();

    if(QDialog::Accepted == tegDlg.exec())
    {
        setRandomFoilColor(pNewFoil, !Settings::isLightTheme());

        if(addNewFoil(pNewFoil))
        {
            setFoil(pNewFoil);
            s_pMainFrame->updateFoilListBox();
            emit projectModified();
            updateView();
            return;
        }
    }
    //reset everything
    setCurFoil(pCurFoil);
    setCurOpp(pOpPoint);
    if(m_pCurFoil)
        m_XFoil.initXFoilGeometry(m_pCurFoil->n, m_pCurFoil->x, m_pCurFoil->y, m_pCurFoil->nx, m_pCurFoil->ny);
    delete pNewFoil;
    updateView();
}


/**
 * The user has requested the display of all OpPoint curves.
 */
void XDirect::onShowAllOpps()
{
    OpPoint *pOpp;

    m_bCurOppOnly = false;
    (s_pMainFrame)->m_pShowCurOppOnly->setChecked(m_bCurOppOnly);

    for (int i=0; i<m_poaOpp->size(); i++)
    {
        pOpp = m_poaOpp->at(i);
        pOpp->setVisible(true);
    }
    emit projectModified();
    m_bResetCurves = true;
    setCurveParams();
    updateView();
}


/**
 * The user has requested the display of all Polar curves.
 */
void XDirect::onShowAllPolars()
{
    Polar *pPolar;
    for (int i=0; i<m_poaPolar->size(); i++)
    {
        pPolar = m_poaPolar->at(i);
        pPolar->setVisible(true);
    }
    emit projectModified();
    m_bResetCurves = true;
    setCurveParams();
    updateView();
}


/**
 * The user has toggled the display of the curve of the active object
 */
void XDirect::onShowCurve()
{
    //user has toggled visible switch

    if(m_bPolarView)
    {
        if (m_pCurPolar)
        {
            m_pCurPolar->setVisible(m_pchShowCurve->isChecked());

        }
        m_bResetCurves = true;
    }
    else if (m_pCurOpp)
    {
        m_pCurOpp->setVisible(m_pchShowCurve->isChecked());
        m_bResetCurves = true;
    }
    emit projectModified();
    updateView();
}


/**
 * The user has requested the display of only the Polar curves associated to the active Foil
 */
void XDirect::onShowFoilPolarsOnly()
{
    if(!m_pCurFoil) return;

    Polar *pPolar;
    for (int i=0; i<m_poaPolar->size(); i++)
    {
        pPolar = m_poaPolar->at(i);
        pPolar->setVisible((pPolar->foilName() == m_pCurFoil->name()));
    }
    emit projectModified();
    m_bResetCurves = true;
    setCurveParams();
    updateView();
}


/**
 * The user has requested the display of the Polar curves associated to the active Foil
 */
void XDirect::onShowFoilPolars()
{
    if(!m_pCurFoil) return;
    Polar *pPolar;
    for (int i=0; i<m_poaPolar->size(); i++)
    {
        pPolar = m_poaPolar->at(i);
        if(pPolar->foilName() == m_pCurFoil->name())
        {
            pPolar->setVisible(true);
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
void XDirect::onShowFoilOpps()
{
    if(!m_pCurFoil || !m_pCurPolar) return;

    m_bCurOppOnly = false;
    (s_pMainFrame)->m_pShowCurOppOnly->setChecked(m_bCurOppOnly);

    for(int i=0; i<m_poaOpp->size(); i++)
    {
        OpPoint *pOpp = m_poaOpp->at(i);
        if(pOpp->foilName()==m_pCurFoil->name())
            pOpp->setVisible(true);
    }
    emit projectModified();
    if(!m_bPolarView) m_bResetCurves = true;
    setCurveParams();

    updateView();
}


/**
 * The user has requested the display of the curves of all OpPoint objects associated to the active Polar.
 */
void XDirect::onShowPolarOpps()
{
    if(!m_pCurFoil || !m_pCurPolar) return;

    m_bCurOppOnly = false;
    (s_pMainFrame)->m_pShowCurOppOnly->setChecked(m_bCurOppOnly);

    for(int i=0; i<m_poaOpp->size(); i++)
    {
        OpPoint *pOpp = m_poaOpp->at(i);
        if(pOpp->foilName()==m_pCurFoil->name() && pOpp->polarName()==m_pCurPolar->polarName())
            pOpp->setVisible(true);
    }
    emit projectModified();
    if(!m_bPolarView) m_bResetCurves = true;
    setCurveParams();
    updateView();
}


/**
 * The user has toggled the switch used to define the type of input parameter bewteen aoa, Cl, and Re
 */
void XDirect::onSpec()
{
    if      (m_prbSpec1->isChecked()) s_bAlpha = true;
    else if (m_prbSpec2->isChecked()) s_bAlpha = false;
    else if (m_prbSpec3->isChecked()) s_bAlpha = false;
}


/**
 * The user has toggled the switch which defines if OpPoints should be stored at the end of the analysis
 */
void XDirect::onStoreOpp()
{
    OpPoint::setStoreOpp(m_pchStoreOpp->isChecked());
}


/**
 * The user has toggled the switch which defines if the analysis will be viscous or inviscid
 */
void XDirect::onViscous()
{
    s_bViscous = m_pchViscous->isChecked();
}


/**
 * The user has requested the launch of the interface used to define advanced settings for the XFoil analysis
 */
void XDirect::onXFoilAdvanced()
{
    XFoilAdvancedDlg xfaDlg(s_pMainFrame);
    xfaDlg.m_IterLimit   = XFoilTask::s_IterLim;
    xfaDlg.m_bAutoInitBL = XFoilTask::s_bAutoInitBL;
    xfaDlg.m_VAccel      = XFoil::VAccel();
    xfaDlg.m_bFullReport = XFoil::fullReport();
    xfaDlg.initDialog();

    if (QDialog::Accepted == xfaDlg.exec())
    {
        XFoil::setVAccel(xfaDlg.m_VAccel);
        XFoil::setFullReport(xfaDlg.m_bFullReport);
        XFoilTask::s_bAutoInitBL  = xfaDlg.m_bAutoInitBL;
        XFoilTask::s_IterLim      = xfaDlg.m_IterLimit;
    }
}


/**
 * Reads the analysis parameters from the widgets.
 */
void XDirect::readParams()
{
    if(!m_pCurPolar) return;

    if      (m_prbSpec1->isChecked()) s_bAlpha = true;
    else if (m_prbSpec2->isChecked()) s_bAlpha = false;
    else if (m_prbSpec3->isChecked()) s_bAlpha = false;

    m_bSequence = m_pchSequence->isChecked();

    double Alpha=0, AlphaMax=0, AlphaDelta=0;
    double Cl=0, ClMax=0, ClDelta=0;
    double Reynolds=0, ReynoldsMax=0, ReynoldsDelta=0;
    if(m_pCurPolar->polarType()!=Xfl::FIXEDAOAPOLAR)
    {
        if(s_bAlpha)
        {
            Alpha      = m_pdeAlphaMin->value();
            AlphaMax   = m_pdeAlphaMax->value();
            AlphaDelta = m_pdeAlphaDelta->value();
        }
        else
        {
            Cl      = m_pdeAlphaMin->value();
            ClMax   = m_pdeAlphaMax->value();
            ClDelta = m_pdeAlphaDelta->value();
        }
    }
    else
    {
        Reynolds      = m_pdeAlphaMin->value();
        ReynoldsMax   = m_pdeAlphaMax->value();
        ReynoldsDelta = m_pdeAlphaDelta->value();
    }

    m_pXFADlg->setAlpha(Alpha, AlphaMax, AlphaDelta);
    m_pXFADlg->setCl(Cl, ClMax, ClDelta);
    m_pXFADlg->setRe(Reynolds, ReynoldsMax, ReynoldsDelta);

    s_bInitBL   = m_pchInitBL->isChecked();
    s_bViscous  = m_pchViscous->isChecked();
    OpPoint::setStoreOpp(m_pchStoreOpp->isChecked());
}


/**
 * Saves the user-defined settings
 * @param pSettings a pointer to the QSetting object.
 */
void XDirect::saveSettings(QSettings &settings)
{
    QString str1, str2, str3;
    settings.beginGroup("XDirect");
    {
        settings.setValue("AlphaSpec", s_bAlpha);
        settings.setValue("StoreOpp", OpPoint::bStoreOpp());
        settings.setValue("ViscousAnalysis", s_bViscous);
        settings.setValue("InitBL", s_bInitBL);
        settings.setValue("PolarView", m_bPolarView);
        settings.setValue("UserGraph", m_bShowUserGraph);
        settings.setValue("Type1", m_bType1);
        settings.setValue("Type2", m_bType2);
        settings.setValue("Type3", m_bType3);
        settings.setValue("Type4", m_bType4);
        settings.setValue("TextOutput", m_bShowTextOutput);
        settings.setValue("CurOppOnly", m_bCurOppOnly);
        settings.setValue("ShowInviscid", m_bShowInviscid);
        settings.setValue("ShowCpGraph", m_bCpGraph);
        settings.setValue("Sequence", m_bSequence);
        settings.setValue("OppVar", m_CpGraph.yVariable());
        settings.setValue("TimeUpdateInterval", s_TimeUpdateInterval);
        settings.setValue("PlrGraph", m_iPlrGraph);
        settings.setValue("NeutralLine", m_bNeutralLine);

        switch(m_iPlrView)
        {
            case Xfl::ONEGRAPH:
                settings.setValue("PlrView", 1);
                break;
            case Xfl::TWOGRAPHS:
                settings.setValue("PlrView", 2);
                break;
            case Xfl::FOURGRAPHS:
                settings.setValue("PlrView", 4);
                break;
            default:
                settings.setValue("PlrView", 0);
                break;
        }


        settings.setValue("AutoInitBL", XFoilTask::s_bAutoInitBL);
        settings.setValue("IterLim", XFoilTask::s_IterLim);
        settings.setValue("FullReport", XFoil::fullReport());

        settings.setValue("BatchUpdatePolarView", BatchThreadDlg::s_bUpdatePolarView);
        settings.setValue("MaxThreads", BatchThreadDlg::s_nThreads);

        settings.setValue("Reynolds",      s_Re);
        settings.setValue("ReynoldsMax",   s_ReMax);
        settings.setValue("ReynoldsDelta", s_ReDelta);

        settings.setValue("VAccel", m_XFoil.VAccel());
        settings.setValue("KeepOpenErrors", s_bKeepOpenErrors);

        settings.setValue("NReynolds", s_ReList.count());
        for (int i=0; i<s_ReList.count(); i++)
        {
            str1 = QString("ReList%1").arg(i);
            str2 = QString("MaList%1").arg(i);
            str3 = QString("NcList%1").arg(i);
            settings.setValue(str1, s_ReList[i]);
            settings.setValue(str2, s_MachList[i]);
            settings.setValue(str3, s_NCritList[i]);
        }
    }
    settings.endGroup();

    for(int ig=0; ig<m_PlrGraph.count(); ig++)
        m_PlrGraph[ig]->saveSettings(settings);

    m_CpGraph.saveSettings(settings);
    m_pOpPointWidget->saveSettings(settings);

    XFoilAnalysisDlg::saveSettings(settings);
    FoilPolarDlg::saveSettings(settings);
    Optim2d::saveSettings(settings);
    BatchAbstractDlg::saveSettings(settings);
    BatchCtrlDlg::saveSettings(settings);
}


/**
 * Initializes the widget values, depending on the type of Polar
 */
void XDirect::setAnalysisParams()
{
    m_pchViscous->setChecked(s_bViscous);
    m_pchInitBL->setChecked(s_bInitBL);
    m_pchStoreOpp->setChecked(OpPoint::bStoreOpp());

    if(m_pCurPolar)
    {
        if(m_pCurPolar->polarType()!=Xfl::FIXEDAOAPOLAR)
        {
            m_pdeAlphaMin->setDigits(3);
            m_pdeAlphaMax->setDigits(3);
            m_pdeAlphaDelta->setDigits(3);
            if(s_bAlpha) m_prbSpec1->setChecked(true);
            else         m_prbSpec2->setChecked(true);
            m_prbSpec3->setEnabled(false);
            m_plabUnit1->setText(QChar(0260));
            m_plabUnit2->setText(QChar(0260));
            m_plabUnit3->setText(QChar(0260));
        }
        else
        {
            m_prbSpec3->setChecked(true);
            m_prbSpec3->setEnabled(true);
            m_pdeAlphaMin->setDigits(0);
            m_pdeAlphaMax->setDigits(0);
            m_pdeAlphaDelta->setDigits(0);
            m_plabUnit1->setText(" ");
            m_plabUnit2->setText(" ");
            m_plabUnit3->setText(" ");
        }
    }
    else
    {
        if(s_bAlpha) m_prbSpec1->setChecked(true);
        else         m_prbSpec2->setChecked(true);
        m_prbSpec3->setEnabled(false);
    }
    setOpPointSequence();
    if(m_pCurPolar)
    {
        if(m_pCurPolar->polarType()!=Xfl::FIXEDAOAPOLAR)
        {

        }
        else
        {

        }
    }

}



/**
 * Updates the combobox widgets with the curve data from the active OpPoint or Polar, depending on the active view.
 */
void XDirect::setCurveParams()
{
    if(m_bPolarView)
    {
        if(m_pCurPolar)
        {
            if(m_pCurPolar->isVisible())  m_pchShowCurve->setChecked(true);  else  m_pchShowCurve->setChecked(false);

            m_LineStyle.m_Color = colour(m_pCurPolar);
            m_LineStyle.m_Stipple = m_pCurPolar->polarStyle();
            m_LineStyle.m_Width = m_pCurPolar->lineWidth();
            m_LineStyle.m_PointStyle = m_pCurPolar->pointStyle();
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
        if(m_pCurOpp)
        {
            if(m_pCurOpp->isVisible())  m_pchShowCurve->setChecked(true);  else  m_pchShowCurve->setChecked(false);

            m_LineStyle = m_pCurOpp->theStyle();
            fillComboBoxes();
        }
        else
        {
            fillComboBoxes(false);
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
Foil* XDirect::setFoil(Foil* pFoil)
{
    stopAnimate();

    setCurFoil(pFoil);

    if(!m_pCurFoil)
    {
        //take the first in the array, if any
        if(m_poaFoil->size())
        {
            setCurFoil(m_poaFoil->at(0));
        }
    }

    Foil *pCurFoil = m_pCurFoil;
    bool bRes = false;
    if(pCurFoil)
    {
        if(pCurFoil->n>=IQX-2)
        {
            QString strange;
            strange = QString::asprintf("Cannot initialize the foil %s:\nNumber of panels=%d, Max. size=%d,\nRecommended size=100-150 panels",
                            pCurFoil->name().toStdString().c_str(), pCurFoil->n, IQX-2);
            QMessageBox::warning(s_pMainFrame, tr("Warning"), strange);
        }
        else
        {
            bRes = m_XFoil.initXFoilGeometry(pCurFoil->n, pCurFoil->x,pCurFoil->y, pCurFoil->nx, pCurFoil->ny);
            if(!bRes)
            {
                QString strange;
                strange = QString::asprintf("Error while initializing the foil %s", pCurFoil->name().toStdString().c_str());
                QMessageBox::warning(s_pMainFrame, tr("Warning"), strange);
            }
        }
    }

    if(pCurFoil && !bRes)
    {
        setCurFoil(nullptr);
    }
    else
    {
        if(!m_pCurFoil)
        {
            setCurPolar(nullptr);
            setCurOpp(nullptr);
        }
        else if (m_pCurPolar && m_pCurPolar->foilName() !=m_pCurFoil->name())
        {
            //            setCurPolar(nullptr);
            //            setCurOpp(nullptr);
        }
        else if (m_pCurOpp && m_pCurOpp->foilName()  !=m_pCurFoil->name())
        {
            //            setCurOpp(nullptr);
        }
    }

    setPolar();

    return m_pCurFoil;
}



/**
 * Initializes XDirect with the specified Polar object.
 * If the specified polar is not valid, a stock polar associated to the current foil will be set.
 * Sets the first OpPoint object belonging to this Polar, if any.
 * Initializes the XFoil object with the Polar's data.
 * @param pPolar a pointer to the Polar object to set. If NULL, a stock polar associated to the current foil will be set.
 * @return a pointer to the Polar object which has been set.
 */
Polar * XDirect::setPolar(Polar *pPolar)
{
    stopAnimate();

    if(!m_pCurFoil|| !m_pCurFoil->name().length())
    {
        setCurPolar(nullptr);
        setCurOpp(nullptr);
        setAnalysisParams();
        return nullptr;
    }

    if(pPolar) setCurPolar(pPolar);

    if(!m_pCurPolar)
    {
        //try to get one from the object array
        for(int i=0; i<m_poaPolar->size(); i++)
        {
            pPolar = m_poaPolar->at(i);
            if(pPolar && pPolar->foilName()==m_pCurFoil->name())
            {
                //set this one
                setCurPolar(pPolar);
                break;
            }
        }
    }

    if(m_pCurPolar)
    {
        if(m_pCurPolar->foilName() != m_pCurFoil->name())
        {
            Polar *pOldPolar;
            bool bFound = false;
            for (int i=0; i<m_poaPolar->size(); i++)
            {
                pOldPolar = m_poaPolar->at(i);
                if ((pOldPolar->foilName() == m_pCurFoil->name()) &&
                        (pOldPolar->polarName() == m_pCurPolar->polarName()))
                {
                    setCurPolar(pOldPolar);
                    bFound = true;
                    break;
                }
            }
            if(!bFound)
            {
                setCurPolar(nullptr);
                setCurOpp(nullptr);
            }
        }
        s_bInitBL = true;
        m_pchInitBL->setChecked(s_bInitBL);
    }

    //    m_XFoil.InitXFoilAnalysis(m_pCurPolar, s_bViscous); //useless, will be done in XFoilTask
    m_bResetCurves = true;
    setAnalysisParams();
    setOpp();
    setCurveParams();
    return m_pCurPolar;
}


/**
 * Initializes QXDirect with the OpPoint with the specified aoa.
 * If the OpPoint cannot be found for the active Foil and Polar, a stock OpPoint associated to the current foil and polar will be set.
 * @param Alpha the aoa of the OpPoint to ser
 * @return a pointer to the OpPoint object which has been set.
 */
OpPoint * XDirect::setOpp(double Alpha)
{
    OpPoint * pOpp = nullptr;

    if(!m_pCurFoil || !m_pCurPolar)
    {
        setCurOpp(nullptr);
        return nullptr;
    }

    if(Alpha < -1234567.0) //the default
    {
        if(m_pCurOpp && m_pCurOpp->foilName() == m_pCurFoil->name() &&
                m_pCurOpp->polarName()==m_pCurPolar->polarName())
            pOpp = m_pCurOpp;
        else if(m_pCurOpp)
        {
            //try to use the same alpha
            double aoa = m_pCurOpp->aoa();
            pOpp = Objects2d::getOpp(m_pCurFoil, m_pCurPolar, aoa);
        }
    }
    else
    {
        pOpp = Objects2d::getOpp(m_pCurFoil, m_pCurPolar, Alpha);
    }

    if(!pOpp)
    {
        //if unsuccessful so far,
        //try to get the first one from the array
        for(int iOpp=0; iOpp<Objects2d::oppCount(); iOpp++)
        {
            OpPoint *pOldOpp = Objects2d::oppAt(iOpp);
            if(pOldOpp->foilName()==m_pCurFoil->name() && pOldOpp->polarName()==m_pCurPolar->polarName())
            {
                pOpp = pOldOpp;
                break;
            }
        }
    }

    if(pOpp)
    {
        s_pMainFrame->selectOpPoint(pOpp);
    }
    setCurOpp(pOpp);
    m_bResetCurves = true;

    setControls();
    setCurveParams();

    return m_pCurOpp;
}


/**
 * Initializes the widgets with the sequence parameters for the type of the active Polar object.
 */
void XDirect::setOpPointSequence()
{
    m_pchSequence->setEnabled(m_pCurPolar);
    m_pdeAlphaMin->setEnabled(m_pCurPolar);
    m_ppbAnalyze->setEnabled(m_pCurPolar);
    m_pchViscous->setEnabled(m_pCurPolar);
    m_pchInitBL->setEnabled(m_pCurPolar);
    m_pchStoreOpp->setEnabled(m_pCurPolar);

    if(m_bSequence && m_pCurPolar)
    {
        m_pchSequence->setCheckState(Qt::Checked);
        m_pdeAlphaMax->setEnabled(true);
        m_pdeAlphaDelta->setEnabled(true);
    }
    else if (m_pCurPolar)
    {
        m_pchSequence->setCheckState(Qt::Unchecked);
        m_pdeAlphaMax->setEnabled(false);
        m_pdeAlphaDelta->setEnabled(false);
    }
    else
    {
        m_pdeAlphaMax->setEnabled(false);
        m_pdeAlphaDelta->setEnabled(false);
    }


    if(m_pCurPolar && m_pCurPolar->polarType()!=Xfl::FIXEDAOAPOLAR)
    {
        if(m_prbSpec3->isChecked())
        {
            m_prbSpec1->setChecked(true);
            s_bAlpha = true;
        }

        if(s_bAlpha)
        {
            m_pdeAlphaMin->setValue(XFoilAnalysisDlg::s_Alpha);
            m_pdeAlphaMax->setValue(XFoilAnalysisDlg::s_AlphaMax);
            m_pdeAlphaDelta->setValue(XFoilAnalysisDlg::s_AlphaDelta);
        }
        else
        {
            m_pdeAlphaMin->setValue(XFoilAnalysisDlg::s_Cl);
            m_pdeAlphaMax->setValue(XFoilAnalysisDlg::s_ClMax);
            m_pdeAlphaDelta->setValue(XFoilAnalysisDlg::s_ClDelta);
        }
        m_prbSpec1->setEnabled(true);
        m_prbSpec2->setEnabled(true);
        m_prbSpec3->setEnabled(false);
    }
    else if(m_pCurPolar && m_pCurPolar->polarType()==Xfl::FIXEDAOAPOLAR)
    {
        m_prbSpec3->setChecked(true);
        s_bAlpha = true;        // no choice with type 4 polars
        m_pdeAlphaMin->setValue(s_Re);
        m_pdeAlphaMax->setValue(s_ReMax);
        m_pdeAlphaDelta->setValue(s_ReDelta);
        m_prbSpec1->setEnabled(false);
        m_prbSpec2->setEnabled(false);
        m_prbSpec3->setEnabled(true);
    }
    else
    {
        m_prbSpec1->setEnabled(false);
        m_prbSpec2->setEnabled(false);
        m_prbSpec3->setEnabled(false);
    }
}


/**
 * Sets the axis titles for the specified graph
 * @param pGraph a pointer to the Graph object for which the titles will be set
 */
void XDirect::setGraphTitles(Graph* pGraph)
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
void XDirect::setupLayout()
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


    QGroupBox *pAnalysisBox = new QGroupBox(tr("Analysis settings"));
    {
        pAnalysisBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
        QVBoxLayout *pAnalysisGroup = new QVBoxLayout;
        {
            m_pchSequence = new QCheckBox(tr("Sequence"));
            m_pchStoreOpp = new QCheckBox(tr("Store Opp"));
            m_ppbAnalyze  = new QPushButton(tr("Analyze"));

            QHBoxLayout *pSpecVarsLayout = new QHBoxLayout;
            {
                m_prbSpec1 = new QRadioButton("a");
                m_prbSpec2 = new QRadioButton(tr("Cl"));
                m_prbSpec3 = new QRadioButton(tr("Re"));
                m_prbSpec1->setFont(QFont("Symbol"));
                pSpecVarsLayout->addWidget(m_prbSpec1);
                pSpecVarsLayout->addWidget(m_prbSpec2);
                pSpecVarsLayout->addWidget(m_prbSpec3);
            }

            QGridLayout *pSequenceGroupLayout = new QGridLayout;
            {
                QLabel *pAlphaMinLab   = new QLabel(tr("Start="));
                QLabel *pAlphaMaxLab   = new QLabel(tr("End="));
                QLabel *pDeltaAlphaLab = new QLabel(tr("D="));
                pDeltaAlphaLab->setFont(QFont("Symbol"));
                pDeltaAlphaLab->setAlignment(Qt::AlignRight);
                pAlphaMinLab->setAlignment(Qt::AlignRight);
                pAlphaMaxLab->setAlignment(Qt::AlignRight);

                m_plabUnit1 = new QLabel(QChar(0260));
                m_plabUnit2 = new QLabel(QChar(0260));
                m_plabUnit3 = new QLabel(QChar(0260));

                m_pdeAlphaMin   = new DoubleEdit(0,3);
                m_pdeAlphaMax   = new DoubleEdit(0,3);
                m_pdeAlphaDelta = new DoubleEdit(0,3);
                m_pdeAlphaMin->setMinimumHeight(20);
                m_pdeAlphaMax->setMinimumHeight(20);
                m_pdeAlphaDelta->setMinimumHeight(20);
                m_pdeAlphaMin->setAlignment(Qt::AlignRight);
                m_pdeAlphaMax->setAlignment(Qt::AlignRight);
                m_pdeAlphaDelta->setAlignment(Qt::AlignRight);
                pSequenceGroupLayout->addWidget(pAlphaMinLab,1,1);
                pSequenceGroupLayout->addWidget(pAlphaMaxLab,2,1);
                pSequenceGroupLayout->addWidget(pDeltaAlphaLab,3,1);
                pSequenceGroupLayout->addWidget(m_pdeAlphaMin,1,2);
                pSequenceGroupLayout->addWidget(m_pdeAlphaMax,2,2);
                pSequenceGroupLayout->addWidget(m_pdeAlphaDelta,3,2);
                pSequenceGroupLayout->addWidget(m_plabUnit1,1,3);
                pSequenceGroupLayout->addWidget(m_plabUnit2,2,3);
                pSequenceGroupLayout->addWidget(m_plabUnit3,3,3);
            }

            QHBoxLayout *pAnalysisSettings = new QHBoxLayout;
            {
                m_pchViscous  = new QCheckBox(tr("Viscous"));
                m_pchInitBL   = new QCheckBox(tr("Init BL"));
                pAnalysisSettings->addWidget(m_pchViscous);
                pAnalysisSettings->addWidget(m_pchInitBL);
            }

            pAnalysisGroup->addLayout(pSpecVarsLayout);
            pAnalysisGroup->addStretch(1);
            pAnalysisGroup->addWidget(m_pchSequence);
            pAnalysisGroup->addLayout(pSequenceGroupLayout);
            pAnalysisGroup->addStretch(1);
            pAnalysisGroup->addLayout(pAnalysisSettings);
            pAnalysisGroup->addWidget(m_pchStoreOpp);
            pAnalysisGroup->addWidget(m_ppbAnalyze);
        }
        pAnalysisBox->setLayout(pAnalysisGroup);

    }

    QGroupBox *pDisplayBox = new QGroupBox(tr("Display"));
    {
        QVBoxLayout *pDisplayGroup = new QVBoxLayout;
        {
            m_pchShowBL        = new QCheckBox(tr("Displacement thickness"));
            m_pchShowPressure  = new QCheckBox(tr("Pressure"));
            m_pchAnimate       = new QCheckBox(tr("Animate"));
            m_pslAnimateSpeed  = new QSlider(Qt::Horizontal);
            m_pslAnimateSpeed->setMinimum(0);
            m_pslAnimateSpeed->setMaximum(1000);
            m_pslAnimateSpeed->setSliderPosition(500);
            m_pslAnimateSpeed->setTickInterval(50);
            m_pslAnimateSpeed->setTickPosition(QSlider::TicksBelow);
            pDisplayGroup->addWidget(m_pchShowBL);
            pDisplayGroup->addWidget(m_pchShowPressure);
            pDisplayGroup->addWidget(m_pchAnimate);
            pDisplayGroup->addWidget(m_pslAnimateSpeed);
            pDisplayGroup->addStretch(1);
        }
        pDisplayBox->setLayout(pDisplayGroup);
        pDisplayBox->setSizePolicy(szPolicyExpanding);
    }


    QFrame *pPolarPropsFrame = new QFrame;
    {
        QVBoxLayout *pPolarPropsLayout = new QVBoxLayout;
        {
            QLabel *pPolarProps = new QLabel(tr("Polar properties"));
            m_pctrlPolarProps = new MinTextEdit;
            QFont fnt("Courier");
            m_pctrlPolarProps->setFont(fnt);
            m_pctrlPolarProps->setReadOnly(true);

            pPolarPropsLayout->addWidget(pPolarProps);
            pPolarPropsLayout->addWidget(m_pctrlPolarProps);
        }
        pPolarPropsFrame->setLayout(pPolarPropsLayout);
    }


    QGroupBox *pCurveBox = new QGroupBox(tr("Graph Curve Settings"));
    {
        QVBoxLayout *pCurveGroup = new QVBoxLayout;
        {
            QHBoxLayout *pCurveDisplay = new QHBoxLayout;
            {
                m_pchShowCurve     = new QCheckBox(tr("Curve"));
                m_pchAlignChildren = new QCheckBox(tr("Flow down style"));
                QString tip = tr("If activated:\n"
                                 "all changes made to the style of the polar objects will flow down to the operating points\n"
                                 "all changes made to the style of the foil objects will flow down to the polars and to the operating points");
                m_pchAlignChildren->setToolTip(tip);
                pCurveDisplay->addWidget(m_pchShowCurve);
                pCurveDisplay->addWidget(m_pchAlignChildren);
            }

            m_plcbCurveStyle = new LineCbBox(this);
            m_plcbCurveWidth = new LineCbBox(this);
            m_plcbPointStyle = new LineCbBox(this);
            m_plcbPointStyle->showPoints(true);
            m_plbCurveColor = new LineBtn(this);
            m_plbCurveColor->setMinimumHeight(m_plcbCurveStyle->minimumSizeHint().height());

            for (int i=0; i<5; i++)
            {
                m_plcbCurveStyle->addItem("item");
                m_plcbCurveWidth->addItem("item");
                m_plcbPointStyle->addItem("item");
            }

            m_pStyleDelegate = new LineDelegate(m_plcbCurveStyle);
            m_pWidthDelegate = new LineDelegate(m_plcbCurveWidth);
            m_pPointDelegate = new LineDelegate(m_plcbPointStyle);
            m_plcbCurveStyle->setItemDelegate(m_pStyleDelegate);
            m_plcbCurveWidth->setItemDelegate(m_pWidthDelegate);
            m_plcbPointStyle->setItemDelegate(m_pPointDelegate);

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
            CurveStyleLayout->addWidget(m_plcbPointStyle,1,2);
            CurveStyleLayout->addWidget(m_plcbCurveStyle,2,2);
            CurveStyleLayout->addWidget(m_plcbCurveWidth,3,2);
            CurveStyleLayout->addWidget(m_plbCurveColor,4,2);
            CurveStyleLayout->setColumnStretch(2,5);

            pCurveGroup->addLayout(pCurveDisplay);
            pCurveGroup->addLayout(CurveStyleLayout);
            //            pCurveGroup->addStretch(1);
        }
        pCurveBox->setLayout(pCurveGroup);
    }


    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        m_pswMiddleControls = new QStackedWidget;
        m_pswMiddleControls->addWidget(pDisplayBox);
        m_pswMiddleControls->addWidget(pPolarPropsFrame);

        pMainLayout->addWidget(pAnalysisBox);
        //        pMainLayout->addStretch(1);
        pMainLayout->addWidget(m_pswMiddleControls);
        //        pMainLayout->addStretch(1);
        pMainLayout->addWidget(pCurveBox);
        //        pMainLayout->addStretch(1);
    }

    setLayout(pMainLayout);

    setAttribute(Qt::WA_AlwaysShowToolTips);

    setSizePolicy(szPolicyExpanding);
}


/**
 * Interrupts the OpPoint animation
 */
void XDirect::stopAnimate()
{
    if(m_bAnimate)
    {
        m_pAnimateTimer->stop();
        m_bAnimate = false;
        m_pchAnimate->setChecked(false);
        setOpp();
    }
}


/**
 * Updates the curve's style based on the selection in the comboboxes.
 */
void XDirect::updateCurveStyle()
{
    if(m_bPolarView && m_pCurPolar)
    {
        m_pCurPolar->setColor(m_LineStyle.m_Color.red(), m_LineStyle.m_Color.green(), m_LineStyle.m_Color.blue());
        m_pCurPolar->setStipple(m_LineStyle.m_Stipple);
        m_pCurPolar->setWidth(m_LineStyle.m_Width);
        m_pCurPolar->setPointStyle(m_LineStyle.m_PointStyle);

        if(Settings::isAlignedChildrenStyle())
        {
            Objects2d::setPolarChildrenStyle(m_pCurPolar);
        }

        m_bResetCurves = true;
    }
    else if (!m_bPolarView && m_pCurOpp)
    {
        m_pCurOpp->setTheStyle(m_LineStyle);
        m_bResetCurves = true;
    }

    updateView();
    emit projectModified();
}


/**
 * Refreshes the 2d central display.
 */
void XDirect::updateView()
{
    if (!m_bPolarView)
    {
        if(m_bResetCurves) createOppCurves();
    }
    else
    {
        if(m_bResetCurves) createPolarCurves();
    }
    s_pMainFrame->m_pXDirectTileWidget->update();
    update();
}


/**
 * The user has requested the duplication of the current Foil.
 */
void XDirect::onDuplicateFoil()
{
    if(!m_pCurFoil) return;

    Foil *pNewFoil = new Foil;
    pNewFoil->copyFoil(m_pCurFoil);

    if(addNewFoil(pNewFoil))
    {
        s_pMainFrame->updateFoilListBox();
        setFoil(pNewFoil);
        emit projectModified();
    }
}


/**
 * The user has requested to rename the current Foil.
 */
void XDirect::onRenameCurFoil()
{
    renameFoil(m_pCurFoil);
    s_pMainFrame->updateFoilListBox();
    setFoil(m_pCurFoil);
    emit projectModified();
}


/**
 * Adds a new Foil to the array.
 * Requests a name, and overwrites any former Foil with that name.
 * @param pFoil a pointer to the Foil to be added to the array
 * @return a pointer to the input Foil, or NULL if the operation was user-cancelled. @todo what's the point ?
 */
Foil* XDirect::addNewFoil(Foil *pFoil)
{
    if(!pFoil) return nullptr;
    QStringList NameList;
    for(int k=0; k<Objects2d::foilCount(); k++)
    {
        Foil*pOldFoil = Objects2d::foilAt(k);
        NameList.append(pOldFoil->name());
    }

    RenameDlg renDlg(s_pMainFrame);
    renDlg.initDialog(&NameList, pFoil->name(), tr("Enter the foil's new name"));

    if(renDlg.exec() != QDialog::Rejected)
    {
        pFoil->setName(renDlg.newName());
        Objects2d::insertThisFoil(pFoil);

        return pFoil;
    }
    return nullptr;
}



/**
 * Renames the current Foil.
 * Requests a name, and overwrites any former Foil with that name.
 * @param pFoil a pointer to the Foil to be renamed.
 */
void XDirect::renameFoil(Foil *pFoil)
{
    if(!pFoil) return;
    QStringList NameList;
    for(int k=0; k<Objects2d::foilCount(); k++)
    {
        Foil*pOldFoil = Objects2d::foilAt(k);
        NameList.append(pOldFoil->name());
    }

    RenameDlg renDlg(s_pMainFrame);
    renDlg.initDialog(&NameList, pFoil->name(), tr("Enter the foil's new name"));

    if(renDlg.exec() != QDialog::Rejected)
    {
        Objects2d::renameThisFoil(pFoil, renDlg.newName());
    }
}


void XDirect::setView(Xfl::enumGraphView eView)
{
    if (m_bPolarView)
    {
        m_iPlrView = eView;
    }
}


void XDirect::setGraphTiles()
{
    if(m_bPolarView)
    {
        switch(m_iPlrView)
        {
            case Xfl::ONEGRAPH:
                s_pMainFrame->m_pXDirectTileWidget->setGraphList(m_PlrGraph, 1, 0);
                break;
            case Xfl::TWOGRAPHS:
                s_pMainFrame->m_pXDirectTileWidget->setGraphList(m_PlrGraph, 2, 0);
                break;
            case Xfl::FOURGRAPHS:
                s_pMainFrame->m_pXDirectTileWidget->setGraphList(m_PlrGraph, 4, 0);
                break;
            case Xfl::ALLGRAPHS:
                s_pMainFrame->m_pXDirectTileWidget->setGraphList(m_PlrGraph, m_PlrGraph.count(), 0);
                break;
        }
    }
    else
    {
        QVector<Graph*> pGraphList;
        pGraphList.append(&m_CpGraph);
        s_pMainFrame->m_pXDirectTileWidget->setGraphList(pGraphList, 1, 0, Qt::Vertical);
    }
}


/**
 * Sets the Foil scale in the OpPoint view.
 */
void XDirect::setFoilScale()
{
    s_pMainFrame->m_pXDirectTileWidget->opPointWidget()->setFoilScale();
}


/**
 * Imports the analysis definition from an XML file
 */
void XDirect::onImportXMLAnalysis()
{
    QString PathName;
    PathName = QFileDialog::getOpenFileName(s_pMainFrame, tr("Open XML File"),
                                            Settings::s_LastDirName,
                                            tr("Analysis XML file")+"(*.xml)");
    if(!PathName.length())        return ;
    int pos = PathName.lastIndexOf("/");
    if(pos>0) Settings::s_LastDirName = PathName.left(pos);

    QFile XFile(PathName);
    if (!XFile.open(QIODevice::ReadOnly))
    {
        QString strange = tr("Could not read the file\n")+PathName;
        QMessageBox::warning(s_pMainFrame, tr("Warning"), strange);
        return;
    }

    XFile.close();
    importAnalysisFromXML(XFile);
}


/**
 * Imports the Analysis definition from an XML file
 */
void XDirect::importAnalysisFromXML(QFile &xmlFile)
{
    if (!xmlFile.open(QIODevice::ReadOnly))
    {
        QString strange = tr("Could not read the file\n")+xmlFile.fileName();
        QMessageBox::warning(s_pMainFrame, tr("Warning"), strange);
        return;
    }

    Polar *pPolar = new Polar;
    XmlPolarReader polarReader(xmlFile, pPolar);
    polarReader.readXMLPolarFile();

    if(polarReader.hasError())
    {
        QString errorMsg = polarReader.errorString() + QString("\nline %1 column %2").arg(polarReader.lineNumber()).arg(polarReader.columnNumber());
        QMessageBox::warning(s_pMainFrame, "XML read", errorMsg, QMessageBox::Ok);
    }
    else
    {
        Foil *pFoil = Objects2d::foil(pPolar->foilName());
        if(!pFoil && m_pCurFoil)
        {
            s_pMainFrame->statusBar()->showMessage(tr("Attaching the analysis to the active foil"));
            pPolar->setFoilName(m_pCurFoil->name());
            pFoil = m_pCurFoil;
        }
        else if(!pFoil)
        {
            s_pMainFrame->statusBar()->showMessage(tr("No foil to attach the polar to"));
            delete pPolar;
            return;
        }

        Objects2d::addPolar(pPolar);
        setCurOpp(nullptr);
        setCurPolar(pPolar);

        s_pMainFrame->updatePolarListBox();
        emit projectModified();
        setControls();
        setAnalysisParams();
    }
    updateView();
}


/**
 * Exports the analysis data to an XML file
 */
void XDirect::onExportXMLAnalysis()
{
    if(!m_pCurPolar) return ;// is there anything to export ?

    Polar *pCurPolar = m_pCurPolar;
    QString filter = "XML file (*.xml)";
    QString FileName, strong;

    strong = pCurPolar->polarName();
    strong.replace("/", "_");
    strong.replace(".", "_");

    FileName = QFileDialog::getSaveFileName(s_pMainFrame, tr("Export analysis definition to xml file"),
                                            Settings::s_LastDirName +'/'+strong,
                                            filter,
                                            &filter);

    if(!FileName.length()) return;
    int pos = FileName.lastIndexOf("/");
    if(pos>0) Settings::s_LastDirName = FileName.left(pos);

    if(FileName.indexOf(".xml", Qt::CaseInsensitive)<0) FileName += ".xml";


    QFile XFile(FileName);
    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;


    XmlPolarWriter polarWriter(XFile);
    polarWriter.writeXMLPolar(pCurPolar);

    XFile.close();
}


void XDirect::onOptim2d()
{
    Optim2d o2d(s_pMainFrame);
    o2d.setFoil(curFoil());   
    o2d.exec();
    if(o2d.isModified())
    {
        s_pMainFrame->updateFoilListBox();
        emit projectModified();
    }
    updateView();
}

