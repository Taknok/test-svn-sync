/****************************************************************************

    Optim2d Class
    Copyright (C) 2021 Andre Deperrois

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

#include <QThread>
#include <QFutureSynchronizer>
#include <QtConcurrent/QtConcurrent>

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QRandomGenerator>
#include <QMessageBox>
#include <QDebug>

#include "optim2d.h"
#include <globals/globals.h>
#include <graph/curve.h>
#include <misc/text/doubleedit.h>
#include <misc/text/intedit.h>
#include <viewwidgets/graphwidgets/graphwt.h>
#include <viewwidgets/twodwidgets/foilwt.h>
#include <xdirect/analysis/xfoiltask.h>
#include <xdirect/objects2d.h>
#include <xfoil.h>

#define PI 3.141592654
#define LARGEVALUE 1.0e10

QByteArray Optim2d::s_HSplitterSizes;
QByteArray Optim2d::s_VSplitterSizes;
QByteArray Optim2d::s_Geometry;

double Optim2d::s_Alpha           = 0.0;
double Optim2d::s_Re              = 1.0e6;
double Optim2d::s_NCrit           = 9.0;
double Optim2d::s_XtrTop          = 1.0;
double Optim2d::s_XtrBot          = 1.0;
double Optim2d::s_Cl              = 0.0;
double Optim2d::s_MaxError        = 1.e-4;

bool   Optim2d::s_bPSO            = true;
bool   Optim2d::s_bMultiThreaded  = true;
int    Optim2d::s_Dt              = 100;
double Optim2d::s_InertiaWeight   = 0.5;
double Optim2d::s_CognitiveWeight = 0.5;
double Optim2d::s_SocialWeight    = 0.5;
int    Optim2d::s_PopSize         = 17;
int    Optim2d::s_MaxIter         = 100;
int    Optim2d::s_HHn             = 6;   //GA seems to converge a little better if even number so that there is no bump function centered on the LE
double Optim2d::s_HHt2            = 1.0;
double Optim2d::s_HHmax           = 0.025;
double Optim2d::s_ProbXOver       = 0.5;
double Optim2d::s_ProbMutation    = 0.15;
double Optim2d::s_SigmaMutation   = 0.2;

Optim2d::Optim2d(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle("2d Optimization");

    m_pFoil=nullptr;

    m_pBestFoil = new Foil;

    m_bModified = false;
    m_bSaved = true;

    //PSO
    m_Error = LARGEVALUE;
    m_Iter=0;
    m_iBest=0;

    m_iLE = -1;

    setupLayout();
    connectSignals();
}


Optim2d::~Optim2d()
{
    delete m_pBestFoil;
    for(int i=0; i<m_TempFoils.size(); i++) delete m_TempFoils[i];
}


void Optim2d::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        m_pHSplitter = new QSplitter(::Qt::Horizontal);
        {
            m_pHSplitter->setChildrenCollapsible(false);
            QFrame *pLeftFrame = new QFrame();
            {
                QVBoxLayout *pLeftLayout = new QVBoxLayout;
                {
                    m_ptwMain = new QTabWidget;
                    {
                        QFrame *pTargetPage = new QFrame;
                        {
                            QGridLayout *pOptimLayout = new QGridLayout;
                            {
                                QLabel *plabCl = new QLabel("Target Cl:");
                                m_pdeCl = new DoubleEdit(s_Cl);
                                QLabel *plabMaxError = new QLabel("Max. error:");
                                m_pdeMaxError = new DoubleEdit(s_MaxError);

                                m_pchMultithread = new QCheckBox("Multithreaded");
                                m_pchMultithread->setChecked(s_bMultiThreaded);
                                pOptimLayout->addWidget(plabCl,           1, 1);
                                pOptimLayout->addWidget(m_pdeCl,          1, 2);
                                pOptimLayout->addWidget(plabMaxError,     2, 1);
                                pOptimLayout->addWidget(m_pdeMaxError,    2, 2);
                                pOptimLayout->setRowStretch(3,1);
                                pOptimLayout->addWidget(m_pchMultithread, 4,1,1,2);
                            }
                            pTargetPage->setLayout(pOptimLayout);
                        }

                        QFrame *pAlgoPage= new QFrame;
                        {
                            QVBoxLayout *pSwarmLayout = new QVBoxLayout;
                            {
                                QFrame *pAlgoFrame = new QFrame;
                                {
                                    QHBoxLayout *pAlgoLayout = new QHBoxLayout;
                                    {
                                        QLabel *pLabAlgo = new QLabel("Algorithm:");
                                        m_prbPSO = new QRadioButton("PSO");
                                        m_prbPSO->setToolTip("Particle Swarm Optimization");
                                        m_prbGA  = new QRadioButton("GA");
                                        m_prbGA->setToolTip("Genetic Algorithm");
                                        m_prbPSO->setChecked(s_bPSO);
                                        m_prbGA->setChecked(!s_bPSO);

                                        pAlgoLayout->addStretch();
                                        pAlgoLayout->addWidget(pLabAlgo);
                                        pAlgoLayout->addWidget(m_prbPSO);
                                        pAlgoLayout->addWidget(m_prbGA);
                                        pAlgoLayout->addStretch();
                                    }
                                    pAlgoFrame->setLayout(pAlgoLayout);
                                }

                                QGroupBox *pCommonBox = new QGroupBox("Common");
                                {
                                    QGridLayout *pCommonLayout = new QGridLayout;
                                    {
                                        QLabel *plabPopSize = new QLabel("Swarm size:");
                                        m_piePopSize = new IntEdit(s_PopSize);

                                        QLabel *plabMaxIter = new QLabel("Max. iterations:");
                                        m_pieMaxIter = new IntEdit(s_MaxIter);

                                        QLabel *pLabUpdate = new QLabel("Update interval:");
                                        m_pieUpdateDt = new IntEdit(s_Dt);
                                        QLabel *pLabMilliSecs = new QLabel("ms");

                                        pCommonLayout->addWidget(plabPopSize,          1, 1);
                                        pCommonLayout->addWidget(m_piePopSize,         1, 2);

                                        pCommonLayout->addWidget(plabMaxIter,          5, 1);
                                        pCommonLayout->addWidget(m_pieMaxIter,         5, 2);

                                        pCommonLayout->addWidget(pLabUpdate,           6, 1);
                                        pCommonLayout->addWidget(m_pieUpdateDt,        6, 2);
                                        pCommonLayout->addWidget(pLabMilliSecs,        6, 3);
                                    }
                                    pCommonBox->setLayout(pCommonLayout);
                                }
                                m_pswAlgo = new QStackedWidget;
                                {
                                    QGroupBox *pPSOBox = new QGroupBox("PSO");
                                    {
                                        QGridLayout *pPSOLayout = new QGridLayout;
                                        {
                                            QLabel *plabInertia = new QLabel("Inertia weight:");
                                            m_pdeInertiaWeight = new DoubleEdit(s_InertiaWeight);
                                            m_pdeInertiaWeight->setToolTip("The inertia weight determines the influence of the particle's\n"
                                                                           "current velocity on its updated velocity.");

                                            QLabel *plabCognitive = new QLabel("Cognitive weight:");
                                            m_pdeCognitiveWeight = new DoubleEdit(s_CognitiveWeight);
                                            m_pdeCognitiveWeight->setToolTip("The cognitive weights determines the influence of the particle's best position");

                                            QLabel *plabSocial = new QLabel("Social weight:");
                                            m_pdeSocialWeight = new DoubleEdit(s_SocialWeight);
                                            m_pdeSocialWeight->setToolTip("The social weight determines the influence of the global best-known position");


                                            pPSOLayout->addWidget(plabInertia,          1, 1);
                                            pPSOLayout->addWidget(m_pdeInertiaWeight,   1, 2);

                                            pPSOLayout->addWidget(plabCognitive,        2, 1);
                                            pPSOLayout->addWidget(m_pdeCognitiveWeight, 2, 2);

                                            pPSOLayout->addWidget(plabSocial,           3, 1);
                                            pPSOLayout->addWidget(m_pdeSocialWeight,    3, 2);

                                            pPSOLayout->setColumnStretch(3,1);
                                        }
                                        pPSOBox->setLayout(pPSOLayout);
                                    }

                                    QGroupBox *pGABox = new QGroupBox("GA");
                                {
                                    QGridLayout *pGALayout = new QGridLayout;
                                    {
                                        QLabel *pLabXOver = new QLabel("Cross-over probability:");
                                        m_pdeProbXOver = new DoubleEdit(s_ProbXOver);
//                                        m_pdeProbXOver->setRange(0.0, 1.0);

                                        QLabel *pLabProbMute = new QLabel("Mutation probability:");
                                        m_pdeProbMutation = new DoubleEdit(s_ProbMutation);
//                                        m_pdeProbMutation->setRange(0.0, 1.0);

                                        QLabel *pLabSigMute = new QLabel("Mutation standard deviation:");
                                        m_pdeSigmaMutation = new DoubleEdit(s_SigmaMutation);
//                                        m_pdeSigmaMutation->setRange(0.0, 1.0); // absolute value

                                        pGALayout->addWidget(pLabXOver,            1, 1);
                                        pGALayout->addWidget(m_pdeProbXOver,       1, 2);

                                        pGALayout->addWidget(pLabProbMute,         2, 1);
                                        pGALayout->addWidget(m_pdeProbMutation,    2, 2);

                                        pGALayout->addWidget(pLabSigMute,          3, 1);
                                        pGALayout->addWidget(m_pdeSigmaMutation,   3, 2);

                                        pGALayout->setColumnStretch(3,1);
                                    }
                                    pGABox->setLayout(pGALayout);
                                }

                                    m_pswAlgo->addWidget(pPSOBox);
                                    m_pswAlgo->addWidget(pGABox);
                                    m_pswAlgo->setCurrentIndex(s_bPSO ? 0 : 1);
                                }
                                pSwarmLayout->addWidget(pAlgoFrame);
                                pSwarmLayout->addWidget(pCommonBox);
                                pSwarmLayout->addWidget(m_pswAlgo);

                                pSwarmLayout->addStretch();
                            }
                            pAlgoPage->setLayout(pSwarmLayout);
                        }

                        QFrame *pXFoilPage = new QFrame;
                        {
                            QGridLayout *pXFoilLayout = new QGridLayout;
                            {
                                QLabel *plabAlpha = new QLabel("Alpha:");
                                m_pdeAlpha = new DoubleEdit(s_Alpha);
                                QLabel *plabDeg = new QLabel(QChar(0260));

                                QLabel *plabRe = new QLabel("Reynolds:");
                                m_pdeRe   = new DoubleEdit(s_Re);

                                QLabel *plabNCrit = new QLabel("NCrit:");
                                m_pdeNCrit = new DoubleEdit(s_NCrit);

                                QLabel *plabXtrTop = new QLabel("Top transition:");
                                m_pdeXtrTop = new DoubleEdit(s_XtrTop);

                                QLabel *plabXtrBot = new QLabel("Bot. transition:");
                                m_pdeXtrBot = new DoubleEdit(s_XtrBot);

                                m_ppbAnalyze = new QPushButton("Analyze");

                                pXFoilLayout->addWidget(plabAlpha,    1, 1);
                                pXFoilLayout->addWidget(m_pdeAlpha,   1, 2);
                                pXFoilLayout->addWidget(plabDeg,      1, 3);
                                pXFoilLayout->addWidget(plabRe,       2, 1);
                                pXFoilLayout->addWidget(m_pdeRe,      2, 2);
                                pXFoilLayout->addWidget(plabNCrit,    3, 1);
                                pXFoilLayout->addWidget(m_pdeNCrit,   3, 2);
                                pXFoilLayout->addWidget(plabXtrTop,   4, 1);
                                pXFoilLayout->addWidget(m_pdeXtrTop,  4, 2);
                                pXFoilLayout->addWidget(plabXtrBot,   5, 1);
                                pXFoilLayout->addWidget(m_pdeXtrBot,  5, 2);
                                pXFoilLayout->addWidget(m_ppbAnalyze, 7,1,1,3);

                                pXFoilLayout->setRowStretch(6,1);
                                pXFoilLayout->setColumnStretch(1,1);
                                pXFoilLayout->setColumnStretch(2,1);
                            }
                            pXFoilPage->setLayout(pXFoilLayout);
                        }

                        QFrame *pHHPage = new QFrame;
                        {
                            QGridLayout *pHHLayout = new QGridLayout;
                            {
                                QLabel *plabNHH = new QLabel("Nb. of functions:");
                                m_pieNHH = new IntEdit(s_HHn);
                                QLabel *plabt2 = new QLabel("t2:");
                                m_pdeHHt2 = new DoubleEdit(s_HHt2);
                                QLabel *plabLax = new QLabel("Max. HH amplitude:");
                                m_pdeHHmax = new DoubleEdit(s_HHmax*100);
                                QLabel *plabPercent = new QLabel("% Ch.");

                                pHHLayout->addWidget(plabNHH,     1, 1);
                                pHHLayout->addWidget(m_pieNHH,    1, 2);
                                pHHLayout->addWidget(plabLax,     2, 1);
                                pHHLayout->addWidget(m_pdeHHmax,  2, 2);
                                pHHLayout->addWidget(plabPercent, 2, 3);
                                pHHLayout->addWidget(plabt2,      3, 1);
                                pHHLayout->addWidget(m_pdeHHt2,   3, 2);

                                pHHLayout->setRowStretch(4,1);
                                pHHLayout->setColumnStretch(1, 1);
                                pHHLayout->setColumnStretch(2, 1);
                            }

                            pHHPage->setLayout(pHHLayout);
                        }

                        m_ptwMain->addTab(pTargetPage, "Target");
                        m_ptwMain->addTab(pAlgoPage,   "Algorithms");
                        m_ptwMain->addTab(pXFoilPage,  "XFoil");
                        m_ptwMain->addTab(pHHPage,     "Hicks-Henne");
                    }

                    m_ppbMakeSwarm = new QPushButton("Make random population");
                    m_ppbSwarm = new QPushButton("Start optimization");

                    m_ppt = new QPlainTextEdit;
                    QFont fixedfnt(QFontDatabase::systemFont(QFontDatabase::FixedFont));
                    m_ppt->setFont(fixedfnt);

                    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close);
                    {
                        m_ppbStoreBestFoil = new QPushButton("Store best foil");
                        m_pButtonBox->addButton(m_ppbStoreBestFoil, QDialogButtonBox::ActionRole);

                        QPushButton *ppbClear = new QPushButton("Clear output");
                        connect(ppbClear, SIGNAL(clicked()), m_ppt, SLOT(clear()));
                        m_pButtonBox->addButton(ppbClear, QDialogButtonBox::ActionRole);

                        connect(m_ppbStoreBestFoil, SIGNAL(clicked()), SLOT(onStoreBestFoil()));
                        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
                    }

                    pLeftLayout->addWidget(m_ptwMain);
                    pLeftLayout->addWidget(m_ppbMakeSwarm);
                    pLeftLayout->addWidget(m_ppbSwarm);
                    pLeftLayout->addWidget(m_ppt);
                    pLeftLayout->addWidget(m_pButtonBox);
                }
                pLeftFrame->setLayout(pLeftLayout);
            }

            m_pVSplitter = new QSplitter(Qt::Vertical);
            {
                m_pVSplitter->setChildrenCollapsible(false);
                m_pFoilWt = new FoilWt;
//                m_pFoilWt->showLegend(true);
                m_pGraphWt = new GraphWt;
                m_pGraphWt->setGraph(&m_Graph);
                m_Graph.setMargin(71);
                m_pGraphWt->showLegend(true);

                m_pVSplitter->addWidget(m_pGraphWt);
                m_pVSplitter->addWidget(m_pFoilWt);
                m_pVSplitter->setStretchFactor(0,1);
            }

            m_pHSplitter->addWidget(pLeftFrame);
            m_pHSplitter->addWidget(m_pVSplitter);
            m_pHSplitter->setStretchFactor(1,1);
        }

        pMainLayout->addWidget(m_pHSplitter);

    }
    setLayout(pMainLayout);
}


void Optim2d::connectSignals()
{
    connect(m_prbPSO,       SIGNAL(clicked()), SLOT(onAlgorithm()));
    connect(m_prbGA,        SIGNAL(clicked()), SLOT(onAlgorithm()));
    connect(m_ppbMakeSwarm, SIGNAL(clicked()), SLOT(onMakeSwarm()));
    connect(m_ppbSwarm,     SIGNAL(clicked()), SLOT(onOptimize()));
    connect(&m_Timer,       SIGNAL(timeout()), SLOT(onIteration()));
    connect(m_ppbAnalyze,   SIGNAL(clicked()), SLOT(onAnalyze()));
}


void Optim2d::onButton(QAbstractButton *pButton)
{
    if (m_pButtonBox->button(QDialogButtonBox::Close) == pButton) onClose();
}


void Optim2d::onAlgorithm()
{
    s_bPSO = m_prbPSO->isChecked();

    m_pswAlgo->setCurrentIndex(s_bPSO ? 0 : 1);
    update();
}


void Optim2d::reject()
{
    onClose();
}


void Optim2d::onClose()
{
    QApplication::restoreOverrideCursor();// you never know
    m_Timer.stop();

    if(!m_bSaved)
    {
        int resp = QMessageBox::question(this, tr("Question"), tr("Exit without saving?"),  QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if(resp != QMessageBox::Yes) return;
    }
    accept();
}


void Optim2d::outputText(QString const &msg) const
{
    m_ppt->moveCursor(QTextCursor::End);
    m_ppt->insertPlainText(msg);
    m_ppt->moveCursor(QTextCursor::End);
    m_ppt->ensureCursorVisible();
}


void Optim2d::showEvent(QShowEvent *)
{
    restoreGeometry(s_Geometry);
    if(s_HSplitterSizes.length()>0) m_pHSplitter->restoreState(s_HSplitterSizes);
    if(s_VSplitterSizes.length()>0) m_pVSplitter->restoreState(s_VSplitterSizes);
#ifdef QT_DEBUG
    onAnalyze(); // just to display something
#endif
    m_pGraphWt->update();
}


void Optim2d::hideEvent(QHideEvent *)
{
    s_Alpha =m_pdeAlpha->value();

    s_Geometry = saveGeometry();
    s_HSplitterSizes  = m_pHSplitter->saveState();
    s_VSplitterSizes  = m_pVSplitter->saveState();
}


void Optim2d::loadSettings(QSettings &settings)
{
    settings.beginGroup("Optim2d");
    {
        s_Alpha           = settings.value("Alpha",           s_Alpha).toDouble();
        s_Re              = settings.value("Re",              s_Re).toDouble();
        s_NCrit           = settings.value("NCrit",           s_NCrit).toDouble();
        s_XtrTop          = settings.value("XtrTop",          s_XtrTop).toDouble();
        s_XtrBot          = settings.value("XtrBot",          s_XtrBot).toDouble();
        s_Cl              = settings.value("Cl",              s_Cl).toDouble();
        s_MaxError        = settings.value("MaxError",        s_MaxError).toDouble();

        s_bMultiThreaded  = settings.value("bMultithread",    s_bMultiThreaded).toBool();
        s_bPSO            = settings.value("bPSO",            s_bPSO).toBool();
        s_Dt              = settings.value("Dt",              s_Dt).toInt();
        s_InertiaWeight   = settings.value("InertiaWeight",   s_InertiaWeight).toDouble();
        s_CognitiveWeight = settings.value("CognitiveWeight", s_CognitiveWeight).toDouble();
        s_SocialWeight    = settings.value("SocialWeight",    s_SocialWeight).toDouble();
        s_ProbXOver       = settings.value("CrossOver",       s_ProbXOver).toDouble();
        s_ProbMutation    = settings.value("ProbMutation",    s_ProbMutation).toDouble();
        s_SigmaMutation   = settings.value("SigMutation",     s_SigmaMutation).toDouble();
        s_PopSize         = settings.value("PopSize",         s_PopSize).toInt();
        s_MaxIter         = settings.value("MaxIter",         s_MaxIter).toInt();
        s_HHn             = settings.value("HHn",             s_HHn).toInt();
        s_HHt2            = settings.value("HHt2",            s_HHt2).toDouble();
        s_HHmax           = settings.value("HHmax",           s_HHmax).toDouble();

        s_Geometry        = settings.value("WindowGeom",     QByteArray()).toByteArray();
        s_HSplitterSizes  = settings.value("HSplitterSizes", QByteArray()).toByteArray();
        s_VSplitterSizes  = settings.value("VSplitterSizes", QByteArray()).toByteArray();
    }
    settings.endGroup();
}


void Optim2d::saveSettings(QSettings &settings)
{
    settings.beginGroup("Optim2d");
    {
        settings.setValue("Alpha",             s_Alpha);
        settings.setValue("Re",                s_Re);
        settings.setValue("NCrit",             s_NCrit);
        settings.setValue("XtrTop",            s_XtrTop);
        settings.setValue("XtrBot",            s_XtrBot);
        settings.setValue("Cl",                s_Cl);
        settings.setValue("MaxError",          s_MaxError);

        settings.setValue("bMultithread",      s_bMultiThreaded);
        settings.setValue("bPSO",              s_bPSO);
        settings.setValue("Dt",                s_Dt);
        settings.setValue("InertiaWeight",     s_InertiaWeight);
        settings.setValue("CognitiveWeight",   s_CognitiveWeight);
        settings.setValue("SocialWeight",      s_SocialWeight);
        settings.setValue("CrossOver",         s_ProbXOver);
        settings.setValue("ProbMutation",      s_ProbMutation);
        settings.setValue("SigMutation",       s_SigmaMutation);
        settings.setValue("PopSize",           s_PopSize);
        settings.setValue("MaxIter",           s_MaxIter);
        settings.setValue("HHn",               s_HHn);
        settings.setValue("HHt2",              s_HHt2);
        settings.setValue("HHmax",             s_HHmax);

        settings.setValue("WindowGeom",     s_Geometry);
        settings.setValue("HSplitterSizes", s_HSplitterSizes);
        settings.setValue("VSplitterSizes", s_VSplitterSizes);
    }
    settings.endGroup();
}


void Optim2d::onStoreBestFoil()
{
    Foil *pNewFoil = new Foil(*m_pBestFoil);
    pNewFoil->initFoil();
    QString basename = "Optimized";

    int iter = 1;
    QString name = basename + QString::asprintf("_%d", iter);
    while(Objects2d::foilExists(name))
    {
        name = basename + QString::asprintf("_%d", iter);
        iter++;
    }
    pNewFoil->setFoilName(name);
    pNewFoil->setColor(randomColor(true));
    Objects2d::insertThisFoil(pNewFoil); // overwrites existing
    outputText("Saved the foil with name: "+name+"\n");
    m_bSaved = true;

    m_bModified = true;
}


void Optim2d::readData()
{
    s_bPSO            = m_prbPSO->isChecked();
    s_Alpha           = m_pdeAlpha->value();
    s_Re              = m_pdeRe->value();
    s_NCrit           = m_pdeNCrit->value();
    s_XtrTop          = m_pdeXtrTop->value();
    s_XtrBot          = m_pdeXtrBot->value();
    s_Cl              = m_pdeCl->value();
    s_MaxError        = m_pdeMaxError->value();
    s_HHn             = m_pieNHH->value();
    s_HHt2            = m_pdeHHt2->value();
    s_HHmax           = m_pdeHHmax->value()/100.0;
    s_MaxIter         = m_pieMaxIter->value();
    s_PopSize         = m_piePopSize->value();
    s_InertiaWeight   = m_pdeInertiaWeight->value();
    s_CognitiveWeight = m_pdeCognitiveWeight->value();
    s_SocialWeight    = m_pdeSocialWeight->value();
    s_PopSize         = m_piePopSize->value();
    s_ProbMutation    = m_pdeProbMutation->value();
    s_SigmaMutation   = m_pdeSigmaMutation->value();
    s_ProbXOver       = m_pdeProbXOver->value();
    s_Dt              = m_pieUpdateDt->value();
    s_bMultiThreaded  = m_pchMultithread->isChecked();

    m_BestPosition.resize(s_HHn);
}


void Optim2d::onMakeSwarm(bool bShow)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    m_Graph.deleteCurves();

    m_pFoilWt->clearFoils();
    m_pFoilWt->addFoil(m_pFoil);
    m_pFoilWt->addFoil(m_pBestFoil);
    for(int i=0; i<m_TempFoils.size(); i++) delete m_TempFoils[i];
    m_TempFoils.clear();
    update();

    readData();
    bool bWasActive = false;
    if(m_Timer.isActive())
    {
        bWasActive=true;
        m_Timer.stop();
    }

    outputText("Creating particles...\n");
    QApplication::processEvents();
    m_Swarm.resize(s_PopSize);
    m_BestPosition.resize(s_HHn);

    if(s_bMultiThreaded)
    {
        QFutureSynchronizer<void> futureSync;
        for (int isw=0; isw<m_Swarm.size(); isw++)
        {
            Particle &particle = m_Swarm[isw];
            futureSync.addFuture(QtConcurrent::run(this, &Optim2d::makeRandomParticle, &particle));
        }
        futureSync.waitForFinished();
        outputText("   ...done\n");
    }
    else
    {
        for (int isw=0; isw<m_Swarm.size(); isw++)
        {
            Particle &particle = m_Swarm[isw];
            makeRandomParticle(&particle);
            outputText(QString::asprintf("   created particle %d\n", isw));
        }
    }

    // debug helper
    if(bShow)
    {
        QColor clr = randomColor(true);
        for (int isw=0; isw<m_Swarm.size(); isw++)
        {
            Particle const&particle = m_Swarm.at(isw);
            Foil *pFoil = new Foil;
            pFoil->setFoilName(QString::asprintf("Particle_%d", isw));
            pFoil->setColor(clr);
            clr = clr.darker(119);
            makeFoil(&particle, pFoil);
            m_pFoilWt->addFoil(pFoil);
            m_TempFoils.append(pFoil);
        }
    }

    update();

    outputText(QString::asprintf("Made %d random particles\n", s_PopSize));

    if(bWasActive) m_Timer.start(s_Dt);
    QApplication::restoreOverrideCursor();
}


// note: QFutureSync requires that the parameters be passed by pointer and not by reference
void Optim2d::makeRandomParticle(Particle *pParticle) const
{
    pParticle->setError(LARGEVALUE);
    pParticle->setBestError(LARGEVALUE);
    pParticle->setDim(s_HHn);

    while(pParticle->error()>LARGEVALUE/10)
    {
        for(int i=0; i<pParticle->dim(); i++)
        {
            pParticle->setPos(i, QRandomGenerator::global()->bounded(2.0*s_HHmax)-s_HHmax);
        }

        double const velamp = s_HHmax;
        for(int i=0; i<pParticle->dim(); i++)
        {
            pParticle->setVel(i, QRandomGenerator::global()->bounded(velamp)-velamp/2.0);
        }
        double Cl = foilFunc(pParticle);
        pParticle->setFitness(Cl);
        pParticle->setError(Cl_error(Cl));
        pParticle->setBestError(pParticle->error());
    }
}


void Optim2d::onOptimize()
{
    readData();

    if(m_Timer.isActive())
    {
        m_Timer.stop();
        m_ppbSwarm->setText("Swarm");
        outputText("Optimization interrupted\n\n");
        QApplication::restoreOverrideCursor();
        return;
    }
    else
        m_ppbSwarm->setText("Stop");

    outputText(QString::asprintf("Optimizing for Cl=%.3g @ aoa=%.3g", s_Cl, s_Alpha)+QChar(0260)+"\n");

    m_bSaved = false;

    m_pFoilWt->clearFoils();
    m_pFoilWt->addFoil(m_pFoil);
    m_pFoilWt->addFoil(m_pBestFoil);

    QApplication::setOverrideCursor(Qt::BusyCursor);

    if(m_Swarm.size()==0 || m_Swarm.size()!=s_PopSize) onMakeSwarm(false);

    m_Iter = 0;
    m_iBest = -1;
    m_Error = LARGEVALUE;

    Curve *pCurve = m_Graph.curve(0);
    if(!pCurve) pCurve = m_Graph.addCurve();
    pCurve->setName("Error");
    pCurve->clear();

    m_Timer.start(s_Dt);
    update();
}


void Optim2d::onIteration()
{
    if(m_Swarm.size()==0)
    {
        outputText("Particles have not been created\n");
        m_Timer.stop();
        m_ppbSwarm->setText("Swarm");
        QApplication::restoreOverrideCursor();
        return;
    }

    if(s_bPSO)
    {
        if(s_bMultiThreaded)
        {
            QFutureSynchronizer<void> futureSync;
            for (int isw=0; isw<m_Swarm.size(); isw++)
            {
                Particle &particle = m_Swarm[isw];
                futureSync.addFuture(QtConcurrent::run(this, &Optim2d::PSO_moveParticle, &particle));
            }
            futureSync.waitForFinished();
        }
        else
        {
            for (int isw=0; isw<m_Swarm.size(); isw++)
            {
                Particle &particle = m_Swarm[isw];
                PSO_moveParticle(&particle);
            }
        }
    }
    else
    {
        GA_makeNewGen();
    }

    // make global best
    for (int isw=0; isw<m_Swarm.size(); isw++)
    {
        Particle &particle = m_Swarm[isw];
        if(particle.error()<m_Error)
        {
            m_BestPosition = particle.position();
            m_Error = particle.error();
            m_iBest = isw;
        }
    }

    // update the best foil
    if(m_iBest>=0 && m_iBest<m_Swarm.size())
    {
        Particle const &mf = m_Swarm.at(m_iBest);
        makeFoil(&mf, m_pBestFoil);
    }

    m_Iter++;
    outputText(QString::asprintf("It(%d): err=%7.3g\n", m_Iter, m_Error));
    Curve *pCurve = m_Graph.curve(0);
    if(pCurve) pCurve->appendPoint(m_Iter, m_Error);
    m_Graph.resetYLimits();

    update();

    if(m_Iter>=s_MaxIter || m_Error<s_MaxError)
    {
        m_Timer.stop();
        m_ppbSwarm->setText("Swarm");
        outputText(QString::asprintf("The winner is particle %d\n", m_iBest));
        outputText(QString::asprintf("Residual error = %7.3g\n\n", m_Error));
        QApplication::restoreOverrideCursor();
    }
}


void Optim2d::PSO_moveParticle(Particle *pParticle) const
{
    // note: QFutureSync requires that the parameters be passed by pointer and not by reference
    double const probregen = 0.01;
    double Cl=0;
    double r1=0, r2=0, newerror=0;
    QVector<double> newVelocity(s_HHn), newpos(s_HHn);
    for(int j=0; j<pParticle->dim(); j++)
    {
        r1 = QRandomGenerator::global()->bounded(1.0);
        r2 = QRandomGenerator::global()->bounded(1.0);

        newVelocity[j] = (s_InertiaWeight * pParticle->vel(j)) +
                         (s_CognitiveWeight * r1 * (pParticle->m_BestPosition.at(j) - pParticle->pos(j))) +
                         (s_SocialWeight    * r2 * (m_BestPosition.at(j)            - pParticle->pos(j)));
    }
    pParticle->setVelocity(newVelocity.constData());

    // new position
    for (int j=0; j<pParticle->dim(); j++)
        newpos[j] = pParticle->pos(j) + newVelocity.at(j);

    checkBounds(pParticle);

    pParticle->setPosition(newpos.constData());

    Cl = foilFunc(pParticle);
    newerror = Cl_error(Cl);
    pParticle->setError(newerror);


    // always regenerate if XFoil failed to converge on this particle: geom is likely flawed
    // othewise regenerate particles with random probability
    double regen = QRandomGenerator::global()->bounded(1.0);
    if (newerror>LARGEVALUE/10.0 || regen<probregen)
    {
        if (newerror>LARGEVALUE/10.0) qDebug("Regenerating particle due to XFoil failed convergence");
        makeRandomParticle(pParticle);
    }

    if (newerror<pParticle->bestError())
    {
        pParticle->setBestPosition(newpos.constData());
        pParticle->setBestError(newerror);
    }
}


void Optim2d::setFoil(Foil const *pFoil)
{
    m_pFoil = pFoil;
    m_pBestFoil->copyFoil(m_pFoil, false);
    m_pBestFoil->setColor(QColor(151, 107, 73));
    m_pBestFoil->setFoilName("Best foil");

    m_pFoilWt->addFoil(m_pFoil);
    m_pFoilWt->addFoil(m_pBestFoil);
}


void Optim2d::onAnalyze()
{
    m_Timer.stop();

    s_Alpha = m_pdeAlpha->value();

    Polar polar;
    polar.setReType(1);
    polar.setMaType(1);
    polar.setReynolds(s_Re);
    polar.setNCrit(s_NCrit);
    polar.setXtrTop(s_XtrTop);
    polar.setXtrBot(s_XtrBot);
    bool bViscous  = true;
    bool bInitBL = true;

    QString strange;

    XFoilTask *task = new XFoilTask; // watch the stack
    XFoil const &xfoil = task->m_XFoilInstance;
    task->m_OutStream.setString(&strange);
    task->setSequence(true, s_Alpha, s_Alpha, 0.0);
    task->initializeXFoilTask(m_pFoil, &polar, bViscous, bInitBL, false);
    task->run();
    outputText(strange+"\n");

/*    Curve *pCurve = m_Graph.curve(0);
    if(!pCurve) pCurve = m_Graph.addCurve("Cp");
    pCurve->setColor(Qt::darkCyan);

    pCurve->resizePoints(xfoil.n);
    for (int k=0; k<xfoil.n; k++) pCurve->setPoint(k, xfoil.x[k+1], xfoil.cpv[k+1]);*/

    m_iLE = -1;
    for(int i=0; i<xfoil.n-1; i++)
    {
        if(xfoil.x[i+1]<xfoil.x[i+2])
        {
            m_iLE = i;
            break;
        }
    }
    outputText(QString::asprintf("LE is at index %d\n", m_iLE));

    update();
    delete task;
}


double Optim2d::Cl_error(double Cl) const
{
    return fabs(Cl-s_Cl);
}


double Optim2d::foilFunc(Particle const*pParticle) const
{
    Foil tempfoil;
    makeFoil(pParticle, &tempfoil);

    Polar polar;
    polar.setReType(1);
    polar.setMaType(1);
    polar.setReynolds(s_Re);
    polar.setNCrit(s_NCrit);
    polar.setXtrTop(s_XtrTop);
    polar.setXtrBot(s_XtrBot);
    bool bViscous  = true;
    bool bInitBL = true;

    QString strange;

    XFoilTask *task = new XFoilTask; // watch the stack
    XFoil const &xfoil = task->m_XFoilInstance;
    task->m_OutStream.setString(&strange);
    task->setSequence(true, s_Alpha, s_Alpha, 0.0);
    task->initializeXFoilTask(&tempfoil, &polar, bViscous, bInitBL, false);
    task->run();

    double Cl = LARGEVALUE;
    if(xfoil.lvconv) Cl = xfoil.cl;

    delete task;

    return Cl;
}


void Optim2d::makeFoil(Particle const*pParticle, Foil *pFoil) const
{
    pFoil->copyFoil(m_pFoil, false);

    double t1=0, hh=0, x=0;

    for(int i=0; i<pFoil->n; i++)
    {
        x = pFoil->x[i];
        for(int j=0; j<pParticle->dim(); j++)
        {
            t1 = double(j+1)/double(pParticle->dim()+1); // HH undefined for t1=0
            hh = HH(x, t1, s_HHt2) * pParticle->pos(j);
            pFoil->xb[i] += pFoil->nx[i] *hh;
            pFoil->yb[i] += pFoil->ny[i] *hh;
        }
    }

    memcpy(pFoil->x, pFoil->xb, IBX*sizeof(double));
    memcpy(pFoil->y, pFoil->yb, IBX*sizeof(double));
    pFoil->normalizeGeometry();
}


/** Hicks-Henne bump function
 * parameter t1 controls the bump's position and t2 its width
 */
double Optim2d::HH(double x, double t1, double t2) const
{
    if(x<=0.0 || x>=1.0) return 0.0;
    return pow(sin(PI*pow(x, log(0.5)/log(t1))), t2);
}


void Optim2d::checkBounds(Particle *pParticle) const
{
    for(int i=0; i<pParticle->dim(); i++)
    {
        pParticle->setPos(i, std::max(-s_HHmax,  pParticle->pos(i)));
        pParticle->setPos(i, std::min( s_HHmax,  pParticle->pos(i)));
    }
}


void Optim2d::listPopulation() const
{
    for(int i=0; i<m_Swarm.size(); i++)
    {
        Particle const &ind = m_Swarm.at(i);
        qDebug(" p[%d]: Cl=%7.3g  Err=%7.3g", i, ind.fitness(), ind.error());
    }
    qDebug();
}


void Optim2d::GA_makeNewGen()
{
//    QElapsedTimer t;    t.start();

    GA_makeSelection();
//    qDebug("selection in %d ms", int(t.elapsed()));

    GA_crossOver();
//    qDebug("crossover in %d ms", int(t.elapsed()));

    GA_mutateGaussian();
//    qDebug("mutation  in %d ms", int(t.elapsed()));

    GA_evaluatePopulation();
//    qDebug("evaluation in %d ms\n", int(t.elapsed()));
}


void Optim2d::GA_makeSelection()
{
    QVector<double>error(m_Swarm.size()), cumul(m_Swarm.size());

    // find the error of the worst particle
    double worsterror = 0.0;
    for(int i=0; i<m_Swarm.size(); i++)
    {
        if(m_Swarm.at(i).error()>worsterror) worsterror = m_Swarm.at(i).error();
    }

    // define probability as: worsterror - particle.error
    // this ensures that the worst has probability 0 and the best has max probability
    for(int i=0; i<m_Swarm.size(); i++)
        error[i] = worsterror - m_Swarm.at(i).error();

    cumul.first() = error.first();
    for(int i=1; i<m_Swarm.size(); i++) cumul[i] = cumul.at(i-1) + error.at(i);

    QVector<Particle> newpop(m_Swarm);
    for(int i=0; i<m_Swarm.size(); i++)
    {
        double p = QRandomGenerator::global()->bounded(cumul.last());
        bool bFound = false;
        for(int j=1; j<m_Swarm.size(); j++)
        {
            if(p<=cumul.at(j))
            {
                newpop[i] = m_Swarm.at(j);
                bFound = true;
                break;
            }
        }
        Q_ASSERT(bFound);
    }

    m_Swarm = newpop;
}


/**
 * @todo: in the standard GA, the best does not carry over to the next generation
 * so that there may be a regression
 */
void Optim2d::GA_crossOver()
{
    double const alpha = 0.5;
    double frac=0;
    QVector<Particle> oldpop(m_Swarm);
    m_Swarm.clear();
    Particle parent[2], children[2];

    while (oldpop.size()>=2)
    {
        // extract two parents
        int ifirst = QRandomGenerator::global()->bounded(oldpop.size());
        parent[0] = oldpop.takeAt(ifirst);

        int isecond = QRandomGenerator::global()->bounded(oldpop.size());
        parent[1] = oldpop.takeAt(isecond);

        double prob = QRandomGenerator::global()->bounded(1.0);
        if(prob<s_ProbXOver)
        {
            // create two random children
            for(int iChild=0; iChild<2; iChild++)
            {
                Particle &child = children[iChild];
                child.setDim(parent[0].dim());
                for(int i=0; i<child.dim(); i++)
                {
                    frac = -alpha + QRandomGenerator::global()->bounded(1.0+alpha);
                    child.setPos(i, frac*parent[0].pos(i)+(1.0-frac)*parent[1].pos(i));
                }

                checkBounds(&child);
            }
            if(children[0].error()>LARGEVALUE/10.0 || children[1].error()>LARGEVALUE/10.0)
            {
                // XFoil has failed to converge: restore the parents
                children[0] = parent[0];
                children[1] = parent[1];
            }
        }
        else
        {
            children[0] = parent[0];
            children[1] = parent[1];
        }
        m_Swarm.append(children[0]);
        m_Swarm.append(children[1]);
    }

    m_Swarm.append(oldpop); // add the remaining single parent if odd population
}


/**
 * @todo: in the standard GA, the best can mutate, so that there may be a regression
 */
void Optim2d::GA_mutateGaussian()
{
    std::default_random_engine generator;
    std::normal_distribution<double> distribution(0.0, s_SigmaMutation*s_HHmax);
    double prob=0, randomvariation=0, newgene=0;
    for(int i=0; i<m_Swarm.size(); i++)
    {
        Particle &particle = m_Swarm[i];
        for(int j=0; j<particle.dim(); j++) // and y components
        {
            prob = QRandomGenerator::global()->bounded(1.0);
            if(prob<s_ProbMutation)
            {
                randomvariation = distribution(generator);
                newgene = particle.pos(j) + randomvariation;
                particle.setPos(j, newgene);
            }
        }

        checkBounds(&particle);
    }
}


void Optim2d::GA_evaluatePopulation()
{
    m_iBest = -1;
    m_Error=LARGEVALUE;

    if(s_bMultiThreaded)
    {
        QFutureSynchronizer<void> futureSync;
        for (int isw=0; isw<m_Swarm.size(); isw++)
        {
            Particle &particle = m_Swarm[isw];
            futureSync.addFuture(QtConcurrent::run(this, &Optim2d::GA_evaluateParticle, &particle));
        }
        futureSync.waitForFinished();
    }
    else
    {
        for(int i=0; i<m_Swarm.size(); i++)
        {
            Particle &particle = m_Swarm[i];
            GA_evaluateParticle(&particle);
        }
    }

    for(int i=0; i<m_Swarm.size(); i++)
    {
        Particle &particle = m_Swarm[i];
        if(particle.error()<m_Error)
        {
            m_Error = particle.error();
            m_BestPosition = particle.position();
            m_iBest = i;
        }
    }
}

// The lengthiest task
// Note: PSO is parallelized at swarm level and GA at particle level
// Note: QFutureSync requires that the parameters be passed by pointer and not by reference
void Optim2d::GA_evaluateParticle(Particle *pParticle) const
{
    double Cl = foilFunc(pParticle);

    while(Cl>LARGEVALUE/10.0)
    {
        // XFoil has failed to converge: make a new random particle
        makeRandomParticle(pParticle);
        Cl = foilFunc(pParticle);
    }

    pParticle->setFitness(Cl);
    pParticle->setError(Cl_error(Cl));
}


