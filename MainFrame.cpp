/****************************************************************************

    MainFrame  Class
    Copyright (C) 2008 Andre Deperrois xflr5@yahoo.com

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
#include "MainFrame.h"
#include "Globals.h"
#include "Design/AFoil.h"
#include "XDirect/XDirect.h"
#include "Miarex/Miarex.h"
#include "Misc/AboutQ5.h"
#include "Misc/DisplaySettingsDlg.h"
#include "Misc/RenameDlg.h"
#include "Misc/LinePickerDlg.h"
//#include "Objects/POpp.h"
//#include "Objects/Wing.h"
//#include "Objects/WPolar.h"


MainFrame::MainFrame(QWidget *parent, Qt::WFlags flags)
    : QMainWindow(parent, flags)
{
	setWindowTitle("Q5");
	m_VersionName = "QFLR5 v5.01";

	m_bMaximized = true;
	m_LengthUnit  = 3;
	m_AreaUnit    = 3;
	m_WeightUnit  = 1;
	m_SpeedUnit   = 5;
	m_ForceUnit   = 2;
	m_MomentUnit  = 4;

	CreateDockWindows();

	m_BorderClr       = QColor(200,200,200);
	m_BackgroundColor = QColor(0, 20, 40);
	m_GraphBackColor  = QColor(0, 30, 50);
	m_TextColor       = QColor(220,220,220);
	m_TextFont.setFamily("Courier");
	m_ImageFormat = 2;

	LoadSettings();
	qApp->setStyle(m_StyleName);

	QXDirect *pXDirect = (QXDirect*)m_pXDirect;
	pXDirect->SetAnalysisParams();

	CreateActions();
	CreateMenus();
	CreateToolBars();
	CreateStatusBar();

	m_mtoUnit   = 1.0;
	m_m2toUnit  = 1.0;
	m_kgtoUnit  = 1.0;
	m_mstoUnit  = 1.0;
	m_NtoUnit   = 1.0;
	m_NmtoUnit  = 1.0;

// second line
	m_crColors[0] = QColor(255,0,0),
	m_crColors[1] = QColor(0,255,0),
	m_crColors[2] = QColor(0,0,255),
	m_crColors[3] = QColor(255,255,0),
	m_crColors[4] = QColor(255,0,255),
	m_crColors[5] = QColor(0,255,255),

// third line
	m_crColors[6]  = QColor(200,60,60),
	m_crColors[7]  = QColor(0,160,0),
	m_crColors[8]  = QColor(100,100,240),
	m_crColors[9]  = QColor(170,170,0),
	m_crColors[10] = QColor(130,0,130),
	m_crColors[11] = QColor(0,130,130),

// fourth line
	m_crColors[12] = QColor(255,128,128),
	m_crColors[13] = QColor(0,255,128),
	m_crColors[14] = QColor(64,200,255),
	m_crColors[15] = QColor(170,170,100),
	m_crColors[16] = QColor(190,100,190),
	m_crColors[17] = QColor(100,170,170),

// fifth line
	m_crColors[18] = QColor(228,150,70),
	m_crColors[19] = QColor(170,255,170),
	m_crColors[20] = QColor(120,120,255),
	m_crColors[21] = QColor(228,228,128),
	m_crColors[22] = QColor(255,170,255),
	m_crColors[23] = QColor(170,255,255),

// first line
	m_crColors[24] = QColor(50,50,50),
	m_crColors[25] = QColor(90,90,90),
	m_crColors[26] = QColor(130,130,130),
	m_crColors[27] = QColor(170,170,170),
	m_crColors[28] = QColor(210,210,210),

	m_bSaved     = true;
	m_bSaveOpps  = false;
	m_bSaveWOpps = false;
	m_iApp = XFOILANALYSIS;


	CWing::s_pFrame = this;
}

MainFrame::~MainFrame()
{
	delete m_pglWidget;
	delete m_p2DWidget;
}


void MainFrame::AboutQFLR5()
{
	AboutQ5 dlg;
	dlg.exec();
}


void MainFrame::AddFoil(CFoil *pFoil)
{

	CFoil *pOldFoil = GetFoil(pFoil->m_FoilName);
	if(pOldFoil)
	{
		DeleteFoil(pOldFoil, false);
	}

	bool IsInserted = false;

	for (int i=0; i<m_oaFoil.size(); i++)
	{
		pOldFoil = (CFoil*)m_oaFoil.at(i);
		if (pFoil->m_FoilName < pOldFoil->m_FoilName)
		{
			m_oaFoil.insert(i, pFoil);
			IsInserted = true;
			break;
		}
	}
	if (!IsInserted) m_oaFoil.append(pFoil);

	pFoil->InitFoil();
	m_pCurFoil = pFoil;

	SetSaveState(false);
}


CPolar* MainFrame::AddPolar(CPolar *pPolar)
{
    if(!pPolar) return NULL;
    bool bExists   = false;
    bool bInserted = false;
    bool bFound;
    CPolar *pOldPlr;
    CPolar *pOld2Plr;
    QString strong;
    int i,j,k,l,p;

    for (i=0; i<m_oaPolar.size(); i++)
    {
	pOldPlr = (CPolar*)m_oaPolar.at(i);
	if (pOldPlr->m_PlrName  == pPolar->m_PlrName &&
		pOldPlr->m_FoilName == pPolar->m_FoilName) bExists = true;
    }
    while(!bInserted)
    {
		if(!bExists)
		{
			for (j=0; j<m_oaPolar.size(); j++)
			{
				pOldPlr = (CPolar*)m_oaPolar.at(j);

				if (pPolar->m_FoilName.compare(pOldPlr->m_FoilName, Qt::CaseInsensitive)<0)
				{
					m_oaPolar.insert(j, pPolar);
					bInserted = true;
					break;
				}

				else if (pPolar->m_FoilName == pOldPlr->m_FoilName)
				{
					if(pPolar->m_Type < pOldPlr->m_Type)
					{
						m_oaPolar.insert(j, pPolar);
						bInserted = true;
						break;
					}
					else if(pPolar->m_Type == pOldPlr->m_Type)
					{
						if (pPolar->m_Type !=4)
						{
							//sort by re Nbr
							if(pPolar->m_Reynolds < pOldPlr->m_Reynolds)
							{
								m_oaPolar.insert(j, pPolar);
								bInserted = true;
								break;
							}
						}
						else
						{
							//Type 4, sort by Alphas
							if(pPolar->m_ASpec < pOldPlr->m_ASpec)
							{
								m_oaPolar.insert(j, pPolar);
								bInserted = true;
								break;
							}
						}
					}
				}
			}
			if(!bInserted)
			{
				m_oaPolar.append(pPolar);
				bInserted = true;
			}
			return pPolar;
		}
		else
		{
			for (l=0; l<m_oaPolar.size(); l++)
			{
				pOldPlr = (CPolar*)m_oaPolar.at(l);
				if (pOldPlr->m_FoilName == pPolar->m_FoilName &&
					pOldPlr->m_PlrName  == pPolar->m_PlrName)
				{
					p = 2;
					bFound = true;
					while(bFound)
					{
						strong = QString(" (%1)").arg( p);
						strong = pPolar->m_PlrName + strong;
						for (k=0; k<m_oaPolar.size(); k++)
						{
							bFound = false;
							pOld2Plr = (CPolar*)m_oaPolar.at(k);
							if (pOld2Plr->m_FoilName == pPolar->m_FoilName &&
								pOld2Plr->m_PlrName  == strong)
							{
								p++;
								bFound = true;
								break;
							}
						}
					}
					pPolar->m_PlrName = strong;
				}
			}
			bExists = false;
		}
    }
    return NULL;
}



void MainFrame::AddRecentFile(const QString &PathName)
{
	m_RecentFiles.removeAll(PathName);
	m_RecentFiles.prepend(PathName);
	while (m_RecentFiles.size() > MAXRECENTFILES)
		m_RecentFiles.removeLast();

	updateRecentFileActions();
}


void MainFrame::ClientToGL(QPoint const &point, CVector &real)
{
	if(!m_pglWidget) return;
	double h2 = (double)m_pglWidget->m_rCltRect.height() /2.0;
	double w2 = (double)m_pglWidget->m_rCltRect.width()  /2.0;

	if(w2>h2)
	{
		real.x =  ((double)point.x() - w2) / w2;
		real.y = -((double)point.y() - h2) / w2;
	}
	else
	{
		real.x =  ((double)point.x() - w2) / h2;
		real.y = -((double)point.y() - h2) / h2;
	}
}



void MainFrame::closeEvent (QCloseEvent * event)
{
	if(!m_bSaved)
	{
		int resp = QMessageBox::question(window(), "QFLR5", tr("Save project before exit ?"),
										 QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
		if(resp == QMessageBox::Yes)
		{
			if(!SaveProject(m_FileName))
			{
				QMessageBox::information(window(), tr("QFLR5"), tr("Could not save the project"));
				event->ignore();
				return;
			}
		}
		else if (resp==QMessageBox::Cancel)
		{
			event->ignore();
			return;
		}
	}

	DeleteProject();
	SaveSettings();
	event->accept();//continue closing
}





void MainFrame::contextMenuEvent (QContextMenuEvent * event)
{
	QPoint CltPt = event->pos();
	QPoint ScreenPt = event->pos();
	ScreenPt.rx() += geometry().x();
	ScreenPt.ry() += geometry().y();
	switch(m_iApp)
	{
		case XFOILANALYSIS:
			QXDirect *pXDirect = (QXDirect*)m_pXDirect;
			pXDirect->m_pCurGraph = pXDirect->GetGraph(CltPt);
			if(pXDirect->m_bPolar) 	OperPolarCtxMenu->exec(ScreenPt);
			else                    OperFoilCtxMenu->exec(ScreenPt);
			break;
	}
}


void MainFrame::CreateActions()
{
	newProjectAct = new QAction(QIcon(":/images/new.png"), tr("New Project"), this);
	newProjectAct->setShortcut(tr("Ctrl+N"));
	newProjectAct->setStatusTip(tr("Save and close the current project, create a new project"));
	connect(newProjectAct, SIGNAL(triggered()), this, SLOT(OnNewProject()));

	openAct = new QAction(QIcon(":/images/open.png"), tr("&Open..."), this);
	openAct->setShortcut(tr("Ctrl+O"));
	openAct->setStatusTip(tr("Open an existing file"));
	connect(openAct, SIGNAL(triggered()), this, SLOT(OnLoadFile()));

	saveAct = new QAction(QIcon(":/images/save.png"), tr("Save"), this);
 	saveAct->setShortcut(tr("Ctrl+S"));
	saveAct->setStatusTip(tr("Save the project to disk"));
	connect(saveAct, SIGNAL(triggered()), this, SLOT(OnSaveProject()));

	saveProjectAsAct = new QAction(tr("Save Project As..."), this);
	saveProjectAsAct->setStatusTip(tr("Save the current project under a new name"));
	connect(saveProjectAsAct, SIGNAL(triggered()), this, SLOT(OnSaveProjectAs()));

	restoreToolbarsAct	 = new QAction("Restore toolbars", this);
	restoreToolbarsAct->setStatusTip(tr("Restores the toolbars to their original state"));
	connect(restoreToolbarsAct, SIGNAL(triggered()), this, SLOT(OnRestoreToolbars()));

	saveViewToImageFileAct = new QAction("Save to Image File", this);
	saveViewToImageFileAct->setShortcut(tr("Ctrl+I"));
	saveViewToImageFileAct->setStatusTip(tr("SAves the current view to a file on disk"));
	connect(saveViewToImageFileAct, SIGNAL(triggered()), this, SLOT(OnSaveViewToImageFile()));


	for (int i = 0; i < MAXRECENTFILES; ++i)
	{
		recentFileActs[i] = new QAction(this);
		recentFileActs[i]->setVisible(false);
		connect(recentFileActs[i], SIGNAL(triggered()),
				this, SLOT(openRecentFile()));
	}

	styleAct = new QAction("Set Style", this);
	connect(styleAct, SIGNAL(triggered()), this, SLOT(OnStyle()));

	exportCurGraphAct = new QAction(tr("Export Graph"), this);
	exportCurGraphAct->setStatusTip(tr("Export the current graph data to a text file"));
	connect(exportCurGraphAct, SIGNAL(triggered()), this, SLOT(OnExportCurGraph()));

	resetCurGraphScales = new QAction(tr("Reset Graph Scales"), this);
	resetCurGraphScales->setStatusTip(tr("Restores the graph's x and y scales"));
	connect(resetCurGraphScales, SIGNAL(triggered()), this, SLOT(OnResetCurGraphScales()));

 	exitAct = new QAction(tr("E&xit"), this);
	exitAct->setShortcut(tr("Ctrl+Q"));
	exitAct->setStatusTip(tr("Exit the application"));
	connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

	aboutAct = new QAction(tr("&About"), this);
	aboutAct->setStatusTip(tr("QFLR5"));
	connect(aboutAct, SIGNAL(triggered()), this, SLOT(AboutQFLR5()));

	aboutQtAct = new QAction(tr("About &Qt"), this);
	aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
	connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

	CreateXDirectActions();
}


void MainFrame::CreateDockWindows()
{
	QDockWidget *dock = new QDockWidget(tr("QFLR5"), this);
	dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

	m_p2DWidget = new TwoDWidget(dock);
	setCentralWidget(m_p2DWidget);

	m_pXDirect = new QXDirect(dock);
	QXDirect *pXDirect = (QXDirect*)m_pXDirect;
	dock->setWidget(pXDirect);
	addDockWidget(Qt::RightDockWidgetArea, dock);
	pXDirect->setAttribute(Qt::WA_DeleteOnClose, false);

	m_pMiarex = new QMiarex(dock);
	QMiarex * pMiarex = (QMiarex*)m_pMiarex;
	m_pAFoil  = new AFoil(dock);
	AFoil *pAFoil = (AFoil*)m_pAFoil;
/*	m_pglWidget = new GLWidget(dock);
	m_pglWidget->pXDirect = pXDirect;
	m_pglWidget->m_pGLViewRect  = &m_GLViewRect;*/
	m_pglWidget = NULL;

	m_p2DWidget->m_pXDirect = pXDirect;
	m_p2DWidget->m_pMainFrame = this;

	QSizePolicy sizepol;
	sizepol.setHorizontalPolicy(QSizePolicy::Expanding);
	sizepol.setVerticalPolicy(QSizePolicy::Expanding);
	m_p2DWidget->setSizePolicy(sizepol);

	pMiarex->m_pMainFrame              = this;
	pMiarex->m_poaBody   = &m_oaBody;
	pMiarex->m_poaPlane  = &m_oaPlane;
	pMiarex->m_poaWing   = &m_oaWing;
	pMiarex->m_poaWPolar = &m_oaWPolar;
	pMiarex->m_poaWOpp   = &m_oaWOpp;
	pMiarex->m_poaPOpp   = &m_oaPOpp;
	pMiarex->m_poaFoil   = &m_oaFoil;
	pMiarex->m_poaPolar  = &m_oaPolar;

	pAFoil->m_pMainFrame               = this;
	pAFoil->m_poaFoil    = &m_oaFoil;

	pXDirect->m_pMainFrame             = this;
	pXDirect->m_rCltRect               = geometry();
	pXDirect->m_p2DWidget              = m_p2DWidget;
	pXDirect->m_pCpGraph->m_pParent    = m_p2DWidget;
	pXDirect->m_pPolarGraph->m_pParent = m_p2DWidget;
	pXDirect->m_pTrGraph->m_pParent    = m_p2DWidget;
	pXDirect->m_pCzGraph->m_pParent    = m_p2DWidget;
	pXDirect->m_pCmGraph->m_pParent    = m_p2DWidget;
	pXDirect->m_pUserGraph->m_pParent  = m_p2DWidget;
	pXDirect->m_poaFoil  = &m_oaFoil;
	pXDirect->m_poaPolar = &m_oaPolar;
	pXDirect->m_poaOpp   = &m_oaOpp;
}
void MainFrame::CreateMenus()
{
	CreateXDirectMenus();
}
void MainFrame::CreateXDirectMenus()
{
	//XDirect foil Context Menu
	OperFoilCtxMenu = new QMenu("Context Menu",this);
	CurFoilCtxMenu = OperFoilCtxMenu->addMenu("Current Foil");
	CurFoilCtxMenu->addAction(setCurFoilStyle);
	CurFoilCtxMenu->addSeparator();
	CurFoilCtxMenu->addAction(renameCurFoil);
	CurFoilCtxMenu->addAction(deleteCurFoil);
	CurFoilCtxMenu->addAction(exportCurFoil);
	CurFoilCtxMenu->addSeparator();//_______________
	CurFoilCtxMenu->addAction(deleteFoilPolars);
	CurFoilCtxMenu->addAction(showFoilPolars);
	CurFoilCtxMenu->addAction(hideFoilPolars);
	CurFoilCtxMenu->addAction(saveFoilPolars);
	CurFoilCtxMenu->addSeparator();
	CurFoilDesignMenu = CurFoilCtxMenu->addMenu("Design Operations");
	CurFoilDesignMenu->addAction(NormalizeFoil);
	CurFoilDesignMenu->addAction(DerotateFoil);
	CurFoilDesignMenu->addAction(RefineLocalFoil);
	CurFoilDesignMenu->addAction(RefineGlobalFoil);
	CurFoilDesignMenu->addAction(EditCoordsFoil);
	CurFoilDesignMenu->addAction(ScaleFoil);
	CurFoilDesignMenu->addAction(SetTEGap);
	CurFoilDesignMenu->addAction(SetLERadius);
	CurFoilDesignMenu->addAction(SetFlap);
	CurFoilDesignMenu->addSeparator();
	CurFoilDesignMenu->addAction(InterpolateFoils);
	CurFoilDesignMenu->addAction(NacaFoils);

	OperFoilCtxMenu->addSeparator();//_______________
	CurPolarCtxMenu = OperFoilCtxMenu->addMenu("Current Polar");
	CurPolarCtxMenu->addAction(deletePolar);
	CurPolarCtxMenu->addAction(exportCurPolar);
	OperFoilCtxMenu->addSeparator();//_______________
	OperFoilCtxMenu->addAction(showInviscidCurve);
	OperFoilCtxMenu->addSeparator();//_______________
	CurGraphCtxMenu = OperFoilCtxMenu->addMenu("Cp graph");
	CurGraphCtxMenu->addAction(resetCpGraphScales);
	CurGraphCtxMenu->addAction(defineCpGraphSettings);
	CurGraphCtxMenu->addAction(exportCpGraphAct);
	OperFoilCtxMenu->addSeparator();//_______________
	OperFoilCtxMenu->addAction(definePolar);
	OperFoilCtxMenu->addAction(defineBatch);
	OperFoilCtxMenu->addSeparator();//_______________
	OperFoilCtxMenu->addAction(showAllPolars);
	OperFoilCtxMenu->addAction(hideAllPolars);
	OperFoilCtxMenu->addSeparator();//_______________
	OperFoilCtxMenu->addAction(showCurOppOnly);
	OperFoilCtxMenu->addAction(showAllOpPoints);
	OperFoilCtxMenu->addAction(hideAllOpPoints);
	OperFoilCtxMenu->addSeparator();//_______________
	OperFoilCtxMenu->addAction(resetFoilScale);
	OperFoilCtxMenu->addAction(showPanels);
	OperFoilCtxMenu->addAction(showNeutralLine);
	//End XDirect foil Context Menu

	//XDirect polar Context Menu
	OperPolarCtxMenu = new QMenu("Context Menu",this);
	CurFoilCtxMenu = OperPolarCtxMenu->addMenu("Current Foil");
	CurFoilCtxMenu->addAction(renameCurFoil);
	CurFoilCtxMenu->addAction(deleteCurFoil);
	CurFoilCtxMenu->addAction(exportCurFoil);
	CurFoilCtxMenu->addSeparator();//_______________
	CurFoilCtxMenu->addAction(deleteFoilPolars);
	CurFoilCtxMenu->addAction(showFoilPolars);
	CurFoilCtxMenu->addAction(hideFoilPolars);
	CurFoilCtxMenu->addAction(saveFoilPolars);
	CurFoilCtxMenu->addSeparator();
	OperPolarCtxMenu->addSeparator();//_______________
	CurPolarCtxMenu = OperPolarCtxMenu->addMenu("Current Polar");
	CurPolarCtxMenu->addAction(deletePolar);
	CurPolarCtxMenu->addAction(exportCurPolar);
	OperPolarCtxMenu->addSeparator();//_______________
	OperPolarCtxMenu->addAction(allPolarGraphsSettingsAct);
	OperPolarCtxMenu->addAction( allPolarGraphsScales);
	CurGraphCtxMenu = OperPolarCtxMenu->addMenu("Current Graph");
	CurGraphCtxMenu->addAction(resetCurGraphScales);
	CurGraphCtxMenu->addAction(curPolarGraphVariableAct);
	CurGraphCtxMenu->addAction(GraphDlgAction);
	CurGraphCtxMenu->addAction(exportCurGraphAct);
	OperPolarCtxMenu->addSeparator();//_______________
	OperPolarCtxMenu->addAction(definePolar);
	OperPolarCtxMenu->addAction(defineBatch);
	OperPolarCtxMenu->addSeparator();//_______________
	OperPolarCtxMenu->addAction(showAllPolars);
	OperPolarCtxMenu->addAction(hideAllPolars);
	OperPolarCtxMenu->addAction(showAllOpPoints);
	OperPolarCtxMenu->addAction(hideAllOpPoints);
	//End XDirect polar Context Menu

	fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(newProjectAct);
	fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
	fileMenu->addAction(saveProjectAsAct);
	separatorAct = fileMenu->addSeparator();
	for (int i = 0; i < MAXRECENTFILES; ++i)
		fileMenu->addAction(recentFileActs[i]);
	fileMenu->addSeparator();
	fileMenu->addAction(exitAct);
	updateRecentFileActions();

    viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(OpPointsAct);
    viewMenu->addAction(PolarsAct);
	viewMenu->addSeparator();
	viewMenu->addAction(restoreToolbarsAct);
	viewMenu->addAction(styleAct);
	viewMenu->addAction(saveViewToImageFileAct);

    foilMenu = menuBar()->addMenu(tr("&Foil"));
	currentFoilMenu = foilMenu->addMenu("Current Foil");
	currentFoilMenu->addAction(setCurFoilStyle);
	currentFoilMenu->addSeparator();
	currentFoilMenu->addAction(exportCurFoil);
	currentFoilMenu->addAction(renameCurFoil);
	currentFoilMenu->addAction(deleteCurFoil);
	currentFoilMenu->addSeparator();
	currentFoilMenu->addAction(deleteFoilPolars);
	currentFoilMenu->addAction(showFoilPolars);
	currentFoilMenu->addAction(hideFoilPolars);
	currentFoilMenu->addAction(saveFoilPolars);
	foilMenu->addSeparator();
	foilMenu->addAction(resetFoilScale);
	foilMenu->addAction(showPanels);
	foilMenu->addAction(showNeutralLine);

    designMenu = menuBar()->addMenu(tr("&Design"));
	designMenu->addAction(NormalizeFoil);
	designMenu->addAction(DerotateFoil);
	designMenu->addAction(RefineLocalFoil);
	designMenu->addAction(RefineGlobalFoil);
	designMenu->addAction(EditCoordsFoil);
	designMenu->addAction(ScaleFoil);
	designMenu->addAction(SetTEGap);
	designMenu->addAction(SetLERadius);
	designMenu->addAction(SetFlap);
	designMenu->addSeparator();
	designMenu->addAction(InterpolateFoils);
	designMenu->addAction(NacaFoils);

	PolarMenu = menuBar()->addMenu(tr("&Polars"));
	currentPolarMenu = PolarMenu->addMenu("Current Polar");
	currentPolarMenu->addAction(exportCurPolar);
	currentPolarMenu->addAction(deletePolar);
	PolarMenu->addSeparator();
	PolarMenu->addAction(definePolar);
	PolarMenu->addAction(defineBatch);
	PolarMenu->addSeparator();
	PolarMenu->addAction(showAllPolars);
	PolarMenu->addAction(hideAllPolars);
	PolarMenu->addSeparator();
	GraphPolarMenu = PolarMenu->addMenu("Polar Graphs");
	CurPolarGraphMenu = GraphPolarMenu->addMenu("Polar Graphs");
	CurPolarGraphMenu->addAction(curPolarGraphVariableAct);
	CurPolarGraphMenu->addAction(GraphDlgAction);
	CurPolarGraphMenu->addAction(resetCurGraphScales);
	CurPolarGraphMenu->addAction(exportCurGraphAct);
	GraphPolarMenu->addSeparator();
	GraphPolarMenu->addAction(allPolarGraphsSettingsAct);
	GraphPolarMenu->addAction(allPolarGraphsScales);
	GraphPolarMenu->addSeparator();
	GraphPolarMenu->addAction(AllPolarGraphsAct);
	GraphPolarMenu->addAction(TwoPolarGraphsAct);
	GraphPolarMenu->addSeparator();
	for(int i=0; i<5; i++)
		GraphPolarMenu->addAction(PolarGraphAct[i]);
	PolarMenu->addSeparator();

	OpPointMenu = menuBar()->addMenu(tr("Operating Points"));
	OpPointMenu->addSeparator();
	OpPointMenu->addAction(setCpVarGraph);
	OpPointMenu->addAction(setQVarGraph);
	OpPointMenu->addSeparator();
	OpPointMenu->addAction(showInviscidCurve);
	OpPointMenu->addSeparator();
	CurXFoilResults = OpPointMenu->addMenu("Current XFoil Results");
	CurXFoilResults->addAction(CurXFoilResExport);
	CurXFoilResults->addAction(CurXFoilCtPlot);
	CurXFoilResults->addAction(CurXFoilDbPlot);
	CurXFoilResults->addAction(CurXFoilDtPlot);
	CurXFoilResults->addAction(CurXFoilRtLPlot);
	CurXFoilResults->addAction(CurXFoilRtPlot);
	CurXFoilResults->addAction(CurXFoilNPlot);
	CurXFoilResults->addAction(CurXFoilCdPlot);
	CurXFoilResults->addAction(CurXFoilCfPlot);
	CurXFoilResults->addAction(CurXFoilUePlot);
	CurXFoilResults->addAction(CurXFoilHPlot);
	OpPointMenu->addSeparator();
	OpPointMenu->addAction(resetCpGraphScales);
	OpPointMenu->addAction(defineCpGraphSettings);
	OpPointMenu->addAction(exportCpGraphAct);
	OpPointMenu->addSeparator();
	OpPointMenu->addAction(hideAllOpPoints);
	OpPointMenu->addAction(showAllOpPoints);
	OpPointMenu->addSeparator();
	OpPointMenu->addAction(showCurOppOnly);
	OpPointMenu->addAction(exportCurOpp);
	OpPointMenu->addAction(deleteCurOpp);
	OpPointMenu->addSeparator();
	OpPointMenu->addAction(resetXFoil);
	OpPointMenu->addAction(viewXFoilAdvanced);
	OpPointMenu->addAction(viewLogFile);

//    menuBar()->addSeparator();

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);
}


void MainFrame::CreateXDirectActions()
{
	QXDirect *pXDirect = (QXDirect*)m_pXDirect;

	OpPointsAct = new QAction(tr("OpPoint view"), this);
	OpPointsAct->setStatusTip(tr("Show Operating point view"));
	connect(OpPointsAct, SIGNAL(triggered()), pXDirect, SLOT(OnOpPoints()));

	PolarsAct = new QAction(tr("Polar view"), this);
	PolarsAct->setStatusTip(tr("Show Polar view"));
	connect(PolarsAct, SIGNAL(triggered()), pXDirect, SLOT(OnPolars()));

	defineCpGraphSettings = new QAction(tr("Define Cp Graph Settings"), this);
	connect(defineCpGraphSettings, SIGNAL(triggered()), pXDirect, SLOT(OnCpGraphSettings()));

	resetCpGraphScales = new QAction(tr("Reset Cp Graph Scales"), this);
	connect(resetCpGraphScales, SIGNAL(triggered()), pXDirect, SLOT(OnResetCpGraphScales()));

	exportCpGraphAct = new QAction(tr("Export Cp Graph Data"), this);
	connect(exportCpGraphAct, SIGNAL(triggered()), pXDirect, SLOT(OnExportCpGraph()));

	GraphDlgAction = new QAction(tr("Define Graph Settings"), this);
	connect(GraphDlgAction, SIGNAL(triggered()), pXDirect, SLOT(OnGraphSettings()));

	AllPolarGraphsAct = new QAction(tr("All Polar Graphs"), this);
	connect(AllPolarGraphsAct, SIGNAL(triggered()), pXDirect, SLOT(OnAllPolarGraphs()));

	allPolarGraphsSettingsAct = new QAction(tr("All Polar Graph Settings"), this);
	allPolarGraphsSettingsAct->setStatusTip("Modifies the setting for all polar graphs simultaneously");
	connect(allPolarGraphsSettingsAct, SIGNAL(triggered()), pXDirect, SLOT(OnAllPolarGraphsSetting()));

	allPolarGraphsScales = new QAction(tr("Reset All Polar Graph Scales"), this);
	connect(allPolarGraphsScales, SIGNAL(triggered()), pXDirect, SLOT(OnResetAllPolarGraphsScales()));

	curPolarGraphVariableAct = new QAction(tr("Polar Graph Variable"), this);
	curPolarGraphVariableAct->setStatusTip("Defines the X and Y variables for this graph");
	connect(curPolarGraphVariableAct, SIGNAL(triggered()), pXDirect, SLOT(OnPolarGraphVariable()));

	TwoPolarGraphsAct = new QAction(tr("Two Polar Graphs"), this);
	connect(TwoPolarGraphsAct, SIGNAL(triggered()), pXDirect, SLOT(OnCouplePolarGraphs()));

	for (int i = 0; i < 5; ++i)
	{
		PolarGraphAct[i] = new QAction(this);
		PolarGraphAct[i]->setData(i);
		connect(PolarGraphAct[i], SIGNAL(triggered()), pXDirect, SLOT(OnSinglePolarGraph()));
	}
	PolarGraphAct[0]->setText("Cl vs.Cd");
	PolarGraphAct[1]->setText("Cl vs.Alpha");
	PolarGraphAct[2]->setText("Cl vs. Xtr.");
	PolarGraphAct[3]->setText("Cm vs.Alpha");
	PolarGraphAct[4]->setText("Glide ratio vs. alpha");


	deleteCurFoil = new QAction(tr("Delete..."), this);
	connect(deleteCurFoil, SIGNAL(triggered()), pXDirect, SLOT(OnDeleteCurFoil()));

	renameCurFoil = new QAction(tr("Rename..."), this);
	connect(renameCurFoil, SIGNAL(triggered()), this, SLOT(OnRenameCurFoil()));

	setCurFoilStyle = new QAction(tr("Set Style..."), this);
	connect(setCurFoilStyle, SIGNAL(triggered()), this, SLOT(OnCurFoilStyle()));

	exportCurFoil = new QAction(tr("Export..."), this);
	connect(exportCurFoil, SIGNAL(triggered()), pXDirect, SLOT(OnExportCurFoil()));

	deleteFoilPolars = new QAction(tr("Delete polars"), this);
	deleteFoilPolars->setStatusTip(tr("Delete all the polars associated to this foil"));
	connect(deleteFoilPolars, SIGNAL(triggered()), pXDirect, SLOT(OnDeleteFoilPolars()));

	showFoilPolars = new QAction(tr("Show associated polars"), this);
	connect(showFoilPolars, SIGNAL(triggered()), pXDirect, SLOT(OnShowFoilPolars()));

	hideFoilPolars = new QAction(tr("Hide associated polars"), this);
	connect(hideFoilPolars, SIGNAL(triggered()), pXDirect, SLOT(OnHideFoilPolars()));

	definePolar = new QAction(tr("Define an Analysis"), this);
	definePolar->setShortcut(tr("F6"));
	definePolar->setStatusTip(tr("Defines a single analysis/polar"));
	connect(definePolar, SIGNAL(triggered()), pXDirect, SLOT(OnSingleAnalysis()));

	defineBatch = new QAction(tr("Batch Analysis"), this);
	defineBatch->setShortcut(tr("Shift+F6"));
	defineBatch->setStatusTip(tr("Launches a batch of analysis calculation for a specified range or list of Reynolds numbers"));
	connect(defineBatch, SIGNAL(triggered()), pXDirect, SLOT(OnBatchAnalysis()));

	deletePolar = new QAction(tr("Delete the polar"), this);
	deletePolar->setStatusTip(tr("Deletes the currently selected polar"));
	connect(deletePolar, SIGNAL(triggered()), this, SLOT(OnDeleteCurPolar()));

	exportCurPolar = new QAction(tr("Export the Polar"), this);
	connect(exportCurPolar, SIGNAL(triggered()), pXDirect, SLOT(OnExportCurPolar()));

	showPanels = new QAction(tr("Show Panels"), this);
	showPanels->setStatusTip(tr("Show the foil's panels"));
	connect(showPanels, SIGNAL(triggered()), pXDirect, SLOT(OnShowPanels()));

	resetFoilScale = new QAction(tr("Reset Foil Scale"), this);
	resetFoilScale->setStatusTip(tr("Resets the foil's scale to original size"));
	connect(resetFoilScale, SIGNAL(triggered()), pXDirect, SLOT(OnResetFoilScale()));

	showInviscidCurve = new QAction(tr("Show Inviscid"), this);
	showInviscidCurve->setStatusTip(tr("Display the Opp's inviscid curve"));
	connect(showInviscidCurve, SIGNAL(triggered()), pXDirect, SLOT(OnCpi()));


	showNeutralLine = new QAction(tr("Neutral Line"), this);
	connect(showNeutralLine, SIGNAL(triggered()), pXDirect, SLOT(OnShowNeutralLine()));

	showAllPolars = new QAction(tr("Show All Polars"), this);
	connect(showAllPolars, SIGNAL(triggered()), pXDirect, SLOT(OnShowAllPolars()));

	hideAllPolars = new QAction(tr("Hide All Polars"), this);
	connect(hideAllPolars, SIGNAL(triggered()), pXDirect, SLOT(OnHideAllPolars()));

	saveFoilPolars = new QAction(tr("Save the Associated Polars"), this);
	connect(saveFoilPolars, SIGNAL(triggered()), pXDirect, SLOT(OnSavePolars()));

	showCurOppOnly = new QAction(tr("Show Current Opp Only"), this);
	connect(showCurOppOnly, SIGNAL(triggered()), pXDirect, SLOT(OnCurOppOnly()));

	showAllOpPoints = new QAction(tr("Show All Opps"), this);
	connect(showAllOpPoints, SIGNAL(triggered()), pXDirect, SLOT(OnShowAllOpps()));

	hideAllOpPoints = new QAction(tr("Hide All Opps"), this);
	connect(hideAllOpPoints, SIGNAL(triggered()), pXDirect, SLOT(OnHideAllOpps()));

	exportCurOpp = new QAction(tr("Export Current Opp"), this);
	connect(exportCurOpp, SIGNAL(triggered()), pXDirect, SLOT(OnExportCurOpp()));


	deleteCurOpp = new QAction(tr("Delete Current Opp"), this);
	connect(deleteCurOpp, SIGNAL(triggered()), pXDirect, SLOT(OnDelCurOpp()));

	resetXFoil = new QAction(tr("Reset XFoil"), this);
	connect(resetXFoil, SIGNAL(triggered()), pXDirect, SLOT(OnResetXFoil()));

	viewXFoilAdvanced = new QAction(tr("XFoil Advanced Settings"), this);
	defineBatch->setStatusTip(tr("Tip : you don't want to use that option..."));
	connect(viewXFoilAdvanced, SIGNAL(triggered()), pXDirect, SLOT(OnXFoilAdvanced()));

	viewLogFile = new QAction(tr("View Log File"), this);
	connect(viewLogFile, SIGNAL(triggered()), this, SLOT(OnLogFile()));

	DerotateFoil = new QAction(tr("De-rotate the Foil"), this);
	connect(DerotateFoil, SIGNAL(triggered()), pXDirect, SLOT(OnDerotateFoil()));

	NormalizeFoil = new QAction(tr("Normalize the Foil"), this);
	connect(NormalizeFoil, SIGNAL(triggered()), pXDirect, SLOT(OnNormalizeFoil()));

	RefineLocalFoil = new QAction(tr("Refine Locally"), this);
	connect(RefineLocalFoil, SIGNAL(triggered()), pXDirect, SLOT(OnCadd()));

	RefineGlobalFoil = new QAction(tr("Refine Globally"), this);
	connect(RefineGlobalFoil, SIGNAL(triggered()), pXDirect, SLOT(OnPanels()));

	EditCoordsFoil = new QAction(tr("Edit Foil Coordinates"), this);
	connect(EditCoordsFoil, SIGNAL(triggered()), pXDirect, SLOT(OnFoilCoordinates()));

	ScaleFoil = new QAction(tr("Scale camber and thickness"), this);
	connect(ScaleFoil, SIGNAL(triggered()), pXDirect, SLOT(OnFoilGeom()));

	SetTEGap = new QAction(tr("Set T.E. Gap"), this);
	connect(SetTEGap, SIGNAL(triggered()), pXDirect, SLOT(OnSetTEGap()));

	SetLERadius = new QAction(tr("Set L.E. Radius"), this);
	connect(SetLERadius, SIGNAL(triggered()), pXDirect, SLOT(OnSetLERadius()));

	SetFlap = new QAction(tr("Set Flap"), this);
	connect(SetFlap, SIGNAL(triggered()), pXDirect, SLOT(OnSetFlap()));

	InterpolateFoils = new QAction(tr("Interpolate Foils"), this);
	connect(InterpolateFoils, SIGNAL(triggered()), pXDirect, SLOT(OnInterpolateFoils()));

	NacaFoils = new QAction(tr("Naca Foils"), this);
	connect(NacaFoils, SIGNAL(triggered()), pXDirect, SLOT(OnNacaFoils()));

	setCpVarGraph = new QAction(tr("Cp Variable"), this);
	setCpVarGraph->setStatusTip(tr("Sets Cp vs. chord graph"));
	connect(setCpVarGraph, SIGNAL(triggered()), pXDirect, SLOT(OnCpGraph()));

	setQVarGraph = new QAction(tr("Q Variable"), this);
	setQVarGraph->setStatusTip(tr("Sets Speed vs. chord graph"));
	connect(setQVarGraph, SIGNAL(triggered()), pXDirect, SLOT(OnQGraph()));

	CurXFoilResExport = new QAction(tr("Export Cur. XFoil Results"), this);
	CurXFoilResExport->setStatusTip(tr("Sets Speed vs. chord graph"));
	connect(CurXFoilResExport, SIGNAL(triggered()), pXDirect, SLOT(OnExportCurXFoilResults()));

	CurXFoilCtPlot = new QAction(tr("Max. Shear Coefficient"), this);
	connect(CurXFoilCtPlot, SIGNAL(triggered()), pXDirect, SLOT(OnCtPlot()));

	CurXFoilDbPlot = new QAction(tr("Bottom Side D* and Theta"), this);
	connect(CurXFoilDbPlot, SIGNAL(triggered()), pXDirect, SLOT(OnDbPlot()));

	CurXFoilDtPlot = new QAction(tr("Top Side D* and Theta"), this);
	connect(CurXFoilDtPlot, SIGNAL(triggered()), pXDirect, SLOT(OnDtPlot()));

	CurXFoilRtLPlot = new QAction(tr("Log(Re_Theta)"), this);
	connect(CurXFoilRtLPlot, SIGNAL(triggered()), pXDirect, SLOT(OnRtLPlot()));

	CurXFoilRtPlot = new QAction(tr("Re_Theta"), this);
	connect(CurXFoilRtPlot, SIGNAL(triggered()), pXDirect, SLOT(OnRtPlot()));

	CurXFoilNPlot = new QAction(tr("Amplification Ratio"), this);
	connect(CurXFoilNPlot, SIGNAL(triggered()), pXDirect, SLOT(OnNPlot()));

	CurXFoilCdPlot = new QAction(tr("Dissipation Coefficient"), this);
	connect(CurXFoilCdPlot, SIGNAL(triggered()), pXDirect, SLOT(OnCdPlot()));

	CurXFoilCfPlot = new QAction(tr("Skin Friction Coefficient"), this);
	connect(CurXFoilCfPlot, SIGNAL(triggered()), pXDirect, SLOT(OnCfPlot()));

	CurXFoilUePlot = new QAction(tr("Edge Velocity"), this);
	connect(CurXFoilUePlot, SIGNAL(triggered()), pXDirect, SLOT(OnUePlot()));

	CurXFoilHPlot = new QAction(tr("Kinematic Shape Parameter"), this);
	connect(CurXFoilHPlot, SIGNAL(triggered()), pXDirect, SLOT(OnHPlot()));
}


void MainFrame::CreateStatusBar()
{
	statusBar()->showMessage(tr("Ready"));
	m_pctrlProjectName = new QLabel(" ");
	m_pctrlProjectName->setMinimumWidth(200);
	statusBar()->addPermanentWidget(m_pctrlProjectName);
}

void MainFrame::CreateToolBars()
{
//create XDirect Tool bar;
    m_pctrlFoil = new QComboBox();	
    m_pctrlPolar = new QComboBox;
    m_pctrlOpPoint = new QComboBox;

    m_pctrlFoil->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    m_pctrlPolar->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    m_pctrlOpPoint->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    m_pctrlFoil->setMinimumWidth(200);
    m_pctrlPolar->setMinimumWidth(200);
    m_pctrlOpPoint->setMinimumWidth(30);

    XDirectToolBar = addToolBar(tr("File"));
	XDirectToolBar->addAction(newProjectAct);
	XDirectToolBar->addAction(openAct);
	XDirectToolBar->addAction(saveAct);
    XDirectToolBar->addSeparator();
    XDirectToolBar->addWidget(m_pctrlFoil);
    XDirectToolBar->addWidget(m_pctrlPolar);
    XDirectToolBar->addWidget(m_pctrlOpPoint);

    connect(m_pctrlFoil,    SIGNAL(activated(int)), this, SLOT(OnSelChangeFoil(int)));
    connect(m_pctrlPolar,   SIGNAL(activated(int)), this, SLOT(OnSelChangePolar(int)));
    connect(m_pctrlOpPoint, SIGNAL(activated(int)), this, SLOT(OnSelChangeOpp(int)));

}

void MainFrame::OnDeleteCurPolar()
{
	QXDirect *pXDirect = (QXDirect*)m_pXDirect;
	if(!pXDirect->m_pCurPolar) return;
	OpPoint *pOpPoint;
	int l;
	QString str;

	str = "Are you sure you want to delete the polar :\n  "+pXDirect->m_pCurPolar->m_PlrName;
	str += "\n and all the associated OpPoints ?";

	if (QMessageBox::Yes == QMessageBox::question(window(), "QFLR5", str,
		QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel))
	{
		// start by removing all OpPoints
		for (l=m_oaOpp.size()-1; l>=0; l--)
		{
			pOpPoint = (OpPoint*)m_oaOpp.at(l);
			if (pOpPoint->m_strPlrName == pXDirect->m_pCurPolar->m_PlrName &&
				pOpPoint->m_strFoilName == pXDirect->m_pCurFoil->m_FoilName)
			{
				m_oaOpp.removeAt(l);
				delete pOpPoint;
			}
		}
		// then remove CPolar and update views
		for (l=m_oaPolar.size()-1; l>=0; l--)
		{
			if(pXDirect->m_pCurPolar == m_oaPolar.at(l))
			{
				m_oaPolar.removeAt(l);
				delete pXDirect->m_pCurPolar;
			}
		}
		pXDirect->m_pCurOpp   = NULL;
		pXDirect->m_pCurPolar = NULL;
	}

	UpdatePolars();
	pXDirect->SetPolar();

	SetSaveState(false);
	UpdateView();
}

bool MainFrame::DeleteFoil(CFoil *pFoil, bool bAsk)
{
	QXDirect *pXDirect = (QXDirect*)m_pXDirect;
	if(!pFoil || !pFoil->m_FoilName.length()) return false;
    QString strong;
    CFoil *pOldFoil;
    OpPoint * pOpPoint;
    CPolar* pPolar;
    int j;
    bool bDelete = true;

    if(bAsk)
    {
		strong = "Are you sure you want to delete \n" + pFoil->m_FoilName ;
		strong+= "\nand all associated OpPoints and Polars ?";

		int resp=QMessageBox::question(0,"QFLR5", strong,  QMessageBox::Yes | QMessageBox::Default, QMessageBox::No);
		if(resp==QMessageBox::Yes)			bDelete = true;
		else return false;
    }

    if (bDelete)
    {
		for (j= m_oaOpp.size()-1; j>=0; j--)
		{
			pOpPoint = (OpPoint*)m_oaOpp[j];
			if(pOpPoint->m_strFoilName == pFoil->m_FoilName)
			{
				m_oaOpp.removeAt(j);
				delete pOpPoint;
			}
		}
		for (j= (int)m_oaPolar.size()-1;j>=0; j--)
		{
			pPolar = (CPolar*)m_oaPolar.at(j);
			if(pPolar->m_FoilName == pFoil->m_FoilName)
			{
				m_oaPolar.removeAt(j);
				delete pPolar;
			}
		}
		for (j=0; j< m_oaFoil.size(); j++)
		{
			pOldFoil = (CFoil*)m_oaFoil.at(j);
			if (pOldFoil == pFoil)
			{
				m_oaFoil.removeAt(j);
				delete pOldFoil;
				if(m_pCurFoil == pOldFoil)           m_pCurFoil = NULL;
				if(pXDirect->m_pCurFoil == pOldFoil) pXDirect->m_pCurFoil = NULL;
				break;
			}
		}
		pXDirect->m_pCurOpp = NULL;
		pXDirect->m_pCurPolar = NULL;
		pXDirect->m_pCurFoil = NULL;
		SetSaveState(false);
    }
    return true;

}

void MainFrame::DeletePlane(CPlane *pPlane, bool bResultsOnly)
{
	if(!pPlane || !pPlane->m_PlaneName.length()) return ;
	QMiarex *pMiarex = (QMiarex*)m_pMiarex;
	CWPolar* pWPolar;
	CPOpp * pPOpp;
	CPlane *pOldPlane;
	SetSaveState(false);
	int i;
	//first remove all POpps associated to the plane
	for (i=(int)m_oaPOpp.size()-1; i>=0; i--)
	{
		pPOpp = (CPOpp*)m_oaPOpp.at(i);
		if(pPOpp->m_PlaneName == pPlane->m_PlaneName)
		{
			m_oaPOpp.removeAt(i);
			delete pPOpp;
		}
	}

	pMiarex->m_pCurPOpp = NULL;
	pMiarex->m_pCurWOpp = NULL;
	//next remove all PPolars associated to the plane
	for (i=(int)m_oaWPolar.size()-1; i>=0; i--)
	{
		pWPolar = (CWPolar*)m_oaWPolar.at(i);
		if (pWPolar->m_UFOName == pPlane->m_PlaneName)
		{
			if(!bResultsOnly)
			{
				m_oaWPolar.removeAt(i);
				if(pWPolar == pMiarex->m_pCurWPolar)
				{
					pMiarex->m_pCurWPolar = NULL;
	//				m_WOperDlgBar.EnableAnalysis(false);
				}
				delete pWPolar;
			}
			else
			{
				pWPolar->ResetWPlr();
				pWPolar->m_WArea  = pMiarex->m_pCurWing->m_Area;
				pWPolar->m_WMAChord  = pMiarex->m_pCurWing->m_MAChord;
				pWPolar->m_WSpan     = pMiarex->m_pCurWing->m_Span;
			}
		}
	}

	if(bResultsOnly) return;


	for (i=(int)m_oaPlane.size()-1; i>=0; i--)
	{
		pOldPlane = (CPlane*)m_oaPlane.at(i);
		if (pOldPlane == pPlane)
		{
			m_oaPlane.removeAt(i);
			delete pPlane;
			if(pPlane == pMiarex->m_pCurPlane)
			{
				pMiarex->m_pCurPlane = NULL;
				pMiarex->m_pCurWing  = NULL;
				pMiarex->m_pCurStab  = NULL;
				pMiarex->m_pCurFin   = NULL;
				pMiarex->m_pCurBody  = NULL;
			}
			break;
		}
	}
}


void MainFrame::DeleteProject()
{
	// clear everything
	int i;
	void *pObj;
/*	for (i=m_oaPlane.size()-1; i>=0; i--)
	{
		pObj = m_oaPlane.at(i);
		m_oaPlane.removeAt(i);
		delete (CPlane*)pObj;
	}*/
	for (i=m_oaPOpp.size()-1; i>=0; i--)
	{
		pObj = m_oaPOpp.at(i);
		m_oaPOpp.removeAt(i);
		delete (CPOpp*)pObj;
	}
	for (i=m_oaWing.size()-1; i>=0; i--)
	{
		pObj = m_oaWing.at(i);
		m_oaWing.removeAt(i);
		delete (CWing*)pObj;
	}
	for (i=m_oaWPolar.size()-1; i>=0; i--)
	{
		pObj = m_oaWPolar.at(i);
		m_oaWPolar.removeAt(i);
		delete (CWPolar*)pObj;
	}
	for (i=m_oaWOpp.size()-1; i>=0; i--)
	{
		pObj = m_oaWOpp.at(i);
		m_oaWOpp.removeAt(i);
		delete (CWOpp*)pObj;
	}
	for (i=m_oaFoil.size()-1; i>=0; i--)
	{
		pObj = m_oaFoil.at(i);
		m_oaFoil.removeAt(i);
		delete (CFoil*)pObj;
	}
	for (i=m_oaPolar.size()-1; i>=0; i--)
	{
		pObj = m_oaPolar.at(i);
		m_oaPolar.removeAt(i);
		delete (CPolar*)pObj;
	}
	for (i=m_oaOpp.size()-1; i>=0; i--)
	{
		pObj = m_oaOpp.at(i);
		m_oaOpp.removeAt(i);
		delete (OpPoint*)pObj;
	}
	for (i=m_oaBody.size()-1; i>=0; i--)
	{
		pObj = m_oaBody.at(i);
		m_oaBody.removeAt(i);
		delete (CBody*)pObj;
	}

/*	pMiarex->m_pCurPlane  = NULL;
	pMiarex->m_pCurPOpp   = NULL;
	pMiarex->m_pCurWing   = NULL;
	pMiarex->m_pCurWPolar = NULL;
	pMiarex->m_pCurWOpp   = NULL;
	pMiarex->m_pCurBody   = NULL;
	pMiarex->m_pCurFrame  = NULL;
	pMiarex->CreateWPolarCurves();
	pMiarex->CreateWOppCurves();
	pMiarex->CreateCpCurves();*/

	QXDirect *pXDirect = (QXDirect*)m_pXDirect;
	pXDirect->m_pXFoil->m_FoilName = "";
	pXDirect->m_pCurFoil  = NULL;
	pXDirect->m_pCurPolar = NULL;
	pXDirect->m_pCurOpp   = NULL;
	pXDirect->SetFoil();

/*	if(m_iApp == MIAREX)
	{
		UpdateUFOs();
		pMiarex->SetUFO();
		pMiarex->SetBody();
	}
	else if (m_iApp==XFOILANALYSIS) */
		UpdateFoils();
		if(pXDirect->m_bPolar) pXDirect->CreatePolarCurves();
		else                   pXDirect->CreateOppCurves();

/*	m_AFoilCtrlBar.m_pRefFoil = NULL;
	if(m_iApp==DIRECTDESIGN)
	{
		m_AFoilCtrlBar.FillFoilList();
		m_AFoilCtrlBar.SelectFoil();
	}

	XInverse.Clear();*/

	SetProjectName("");
	SetSaveState(true);
}

QColor MainFrame::GetColor(int type)
{
	//type
	// 0=Foil
	// 1=Polar
	// 2=Opp
	// 3=Wing (unused)
	// 4=WPolar
	// 5=WOpp
	// 6=POpp
	QColor color = QColor(255,0,0);
	int i,j;
	bool bFound = false;
	switch (type)
	{
		case 0:
		{
			CFoil *pFoil;
			for (j=0; j<24; j++)
			{
				for (i=0; i<m_oaFoil.size(); i++)
				{
					pFoil = (CFoil*)m_oaFoil.at(i);
					bFound = false;
					if(pFoil->m_FoilColor == m_crColors[j])
					{
						bFound = true;
						break;
					}
				}
				if(!bFound) return m_crColors[j];
			}
			return m_crColors[m_oaFoil.size()%24];
			break;
		}
		case 1:
		{
			CPolar *pPolar;
			for (j=0; j<24; j++)
			{
				for (i=0; i<m_oaPolar.size(); i++)
				{
					pPolar = (CPolar*)m_oaPolar.at(i);
					bFound = false;
					if(pPolar->m_Color == m_crColors[j])
					{
						bFound = true;
						break;
					}
				}
				if(!bFound) return m_crColors[j];
			}
			return m_crColors[m_oaPolar.size()%24];
			break;
		}
		case 2:
		{
			OpPoint *pOpPoint;
			for (j=0; j<24; j++){
				for (i=0; i<m_oaOpp.size(); i++)
				{
					pOpPoint = (OpPoint*)m_oaOpp.at(i);
					bFound = false;
					if(pOpPoint->m_Color == m_crColors[j])
					{
						bFound = true;
						break;
					}
				}
				if(!bFound) return m_crColors[j];
			}
			return m_crColors[m_oaOpp.size()%24];
			break;
		}
		case 4:
/*			{
			CWPolar *pWPolar;
			for (j=0; j<24; j++)
			{
				for (i=0; i<m_oaWPolar.size(); i++)
				{
					pWPolar = (CWPolar*)m_oaWPolar.at(i);
					bFound = false;
					if(pWPolar->m_Color == m_crColors[j])
					{
						bFound = true;
						break;
					}
				}
				if(!bFound)
					return m_crColors[j];

			}
			return m_crColors[m_oaWPolar.size()%24];
			break;
		}*/
		case 5:
/*		{
			CWOpp *pWOpp;
			for (j=0; j<24; j++){
				for (i=0; i<m_oaWOpp.size(); i++)
				{
					pWOpp = (CWOpp*)m_oaWOpp.at(i);
					bFound = false;
					if(pWOpp->m_Color == m_crColors[j])
					{
						bFound = true;
						break;
					}
				}
				if(!bFound) return m_crColors[j];
			}
			return m_crColors[m_oaWOpp.size()%24];
			break;
		}*/
		case 6:
/*		{
			CPOpp *pPOpp;
			for (j=0; j<24; j++){
				for (i=0; i<m_oaPOpp.size(); i++)
				{
					pPOpp = (CPOpp*)m_oaPOpp.at(i);
					bFound = false;
					if(pPOpp->m_Color == m_crColors[j])
					{
						bFound = true;
						break;
					}
				}
				if(!bFound)
					return m_crColors[j];
			}
			return m_crColors[m_oaPOpp.size()%24];
			break;
		}*/

		default:
		{
			return QColor(255,0,0);
		}
	}
	return color;
}


CFoil* MainFrame::GetFoil(QString strFoilName)
{
	//returns a pointer to the foil with the corresponding name
	// returns NULL if not found
	if(!strFoilName.length()) return NULL;
	CFoil* pFoil;
	for (int i=0; i<m_oaFoil.size(); i++)
	{
		pFoil = (CFoil*)m_oaFoil.at(i);
		if (pFoil->m_FoilName == strFoilName)
		{
			return pFoil;
		}
	}

	return NULL;
}

CPolar *MainFrame::GetPolar(QString strPolarName)
{
	if(!m_pCurFoil) return NULL;

	if (!strPolarName.length())
	{
		//try to get the first from the Combobox
		if(!m_pctrlPolar->count()) return NULL;        //nothing more to try
		strPolarName = m_pctrlPolar->itemText(0); //... and carry on
	}

    CPolar *pPolar;
    for (int i=0; i<m_oaPolar.size(); i++)
    {
		pPolar = (CPolar*) m_oaPolar.at(i);
		if (pPolar->m_FoilName == m_pCurFoil->m_FoilName &&  pPolar->m_PlrName == strPolarName)
		{
			return pPolar;
		}
	}
	return NULL;
}

OpPoint *MainFrame::GetOpp(double Alpha)
{
    OpPoint* pOpPoint;
	QXDirect *pXDirect = (QXDirect*)m_pXDirect;
	CPolar *pCurPolar = pXDirect->m_pCurPolar;
    if(!pCurPolar) return NULL;

    for (int i=0; i<m_oaOpp.size(); i++)
    {
		if(!pCurPolar) return NULL;
		pOpPoint = (OpPoint*)m_oaOpp.at(i);
		//since alphas are calculated at 1/100th
		if (pOpPoint->m_strFoilName == m_pCurFoil->m_FoilName)
		{
			if (pOpPoint->m_strPlrName == pCurPolar->m_PlrName)
			{
				if(pCurPolar->m_Type !=4)
				{
					if(fabs(pOpPoint->Alpha - Alpha) <0.01)
					{
						return pOpPoint;
					}
				}
				else{
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


void MainFrame::GLToClient(CVector const &real, QPoint &point)
{
	if(!m_pglWidget) return;
	double h2 = m_pglWidget->m_pGLViewRect->height() /2.0;
	double w2 = m_pglWidget->m_pGLViewRect->width()  /2.0;

	double dx, dy;
	dx =  real.x + w2;
	dy = -real.y + h2;

	point.setX((int)(dx * m_pglWidget->m_rCltRect.width()));
	point.setY((int)(dy * m_pglWidget->m_rCltRect.height()));
}

void MainFrame::keyPressEvent(QKeyEvent *event)
{
qDebug() << "MainFrame:: Switching key event to XDirect";
	QXDirect *pXDirect = (QXDirect*)m_pXDirect;
	pXDirect->keyPressEvent(event);
}


bool MainFrame::LoadPolarFileV3(QDataStream &ar, bool bIsStoring, int ArchiveFormat)
{
	CFoil *pFoil;
	CPolar *pPolar = NULL;
	CPolar *pOldPlr;
	OpPoint *pOpp, *pOldOpp;
	QString strong;
	QXDirect *pXDirect =(QXDirect*)m_pXDirect;


	//first read all available foils
	int i,l,n;
	ar>>n;
	for (i=0;i<n; i++)
	{
		pFoil = new CFoil();

		pFoil->Serialize(ar, bIsStoring);
		if (!pFoil->m_FoilName.length())
		{
			delete pFoil;
			return false;
		}

		AddFoil(pFoil);
	}
	//next read all available polars

	ar>>n;
	for (i=0;i<n; i++)
	{
		pPolar = new CPolar();

		pPolar->m_Color = GetColor(1);
		pPolar->Serialize(ar, bIsStoring);
		if (!pPolar->m_FoilName.length())
		{
			delete pPolar;
			return false;
		}
		for (l=0; l<m_oaPolar.size(); l++)
		{
			pOldPlr = (CPolar*)m_oaPolar[l];
			if (pOldPlr->m_FoilName == pPolar->m_FoilName &&
				pOldPlr->m_PlrName  == pPolar->m_PlrName)
			{
				//just overwrite...
				m_oaPolar.removeAt(l);
				delete pOldPlr;
				//... and continue to add
			}
		}
		pPolar = AddPolar(pPolar);
	}

	//Last read all available operating points

	ar>>n;
	for (i=0;i<n; i++)
	{
		pOpp = new OpPoint();
		if(!pOpp)
		{
			delete pOpp;
			return false;
		}
//		pOpp->m_pXDirect = &XDirect;
		pOpp->m_Color = m_crColors[m_oaOpp.size()%24];
		if(ArchiveFormat>=100002)
		{
			if (!pOpp->Serialize(ar, bIsStoring, 100002))
			{
				delete pOpp;
				return false;
			}
			else
			{
				pFoil = GetFoil(pOpp->m_strFoilName);
				if(pFoil)
				{
					memcpy(pOpp->x, pFoil->x, sizeof(pOpp->x));
					memcpy(pOpp->y, pFoil->y, sizeof(pOpp->y));
				}
				else
				{
					delete pOpp;
				}
			}
		}
		else
		{
			if (!pOpp->Serialize(ar, bIsStoring))
			{
				delete pOpp;
				return false;
			}
			else
			{
				pFoil = GetFoil(pOpp->m_strFoilName);
				if(pFoil)
				{
					memcpy(pOpp->x, pFoil->x, sizeof(pOpp->x));
					memcpy(pOpp->y, pFoil->y, sizeof(pOpp->y));
				}
				else
				{
					delete pOpp;
				}
			}
		}
		if(pOpp)
		{
			for (int l=0; l<m_oaOpp.size(); l++)
			{
				pOldOpp = (OpPoint*)m_oaOpp.at(l);
				if (pOldOpp->m_strFoilName == pOpp->m_strFoilName &&
					pOldOpp->m_strPlrName  == pOpp->m_strPlrName &&
					abs(pOldOpp->Alpha-pOpp->Alpha)<0.001)
				{
					//just overwrite...
					m_oaOpp.removeAt(l);
					delete pOldOpp;
					//... and continue to add
				}
			}
		}

		pOpp = pXDirect->AddOpPoint(pOpp);
	}

	return true;
}


void MainFrame::LoadSettings()
{
	QXDirect *pXDirect = (QXDirect*)m_pXDirect;
	QString FileName = QDir::tempPath() + "/QFLR5.set";
	QFile *pXFile = new QFile(FileName);
	if (!pXFile->open(QIODevice::ReadOnly))
	{
		return;
	}

	int k;
	int a,b,c,d;
	QString strange;

	QDataStream ar(pXFile);
	ar >> k;//format
	if(k !=100507)
	{
		pXFile->close();
		return;
	}
	ar >> a >> b >> c >> d;
	QPoint pt(a,b);
	QSize sz(c,d);
//	resize(sz);
//	move(pt);
	ar >> m_bMaximized;
	ar >>m_StyleName;
	ar >> m_LastDirName;
	ar >> m_LengthUnit >> m_AreaUnit >> m_WeightUnit >> m_SpeedUnit >> m_ForceUnit >> m_MomentUnit;
	ar >> m_BackgroundColor >>  m_GraphBackColor >> m_TextColor;
	ar >> m_TextFont;
	ar >> m_ImageFormat;

	ar >> k;
	if(k<0 || k> MAXRECENTFILES)
	{
		pXFile->close();
		return;
	}
	m_RecentFiles.clear();
	for(int i=0; i<k; i++)
	{
		ar >> strange;
		m_RecentFiles.append(strange);
	}
	pXDirect->LoadSettings(ar);

	pXFile->close();
}



int MainFrame::LoadXFLR5File(QString PathName)
{
	QFile XFile(PathName);
	if (!XFile.open(QIODevice::ReadOnly))
	{
		QString strange = "Could not read the file\n"+PathName;
		QMessageBox::information(window(), "QFLR5", strange);
		return 0;
	}

	QXDirect * pXDirect = (QXDirect*)m_pXDirect;
	QString end;
	end = PathName.right(4);
	end = end.toLower();

	if(end==".plr")
	{
		QDataStream ar(&XFile);
		ar.setByteOrder(QDataStream::LittleEndian);
		CFoil *pFoil = ReadPolarFile(ar);
		pXDirect->m_bPolar = true;
		pXDirect->m_pCurPolar = NULL;
		pXDirect->m_pCurOpp   = NULL;
		m_pCurFoil = pXDirect->SetFoil(pFoil);
		pXDirect->SetPolar();

		XFile.close();

		AddRecentFile(PathName);
		SetSaveState(false);
		return XFOILANALYSIS;
	}
	else
	{
		if(end==".dat")
		{
			QTextStream ar(&XFile);
			CFoil *pFoil = ReadFoilFile(ar);
			XFile.close();

			if(pFoil)
			{
				AddFoil(pFoil);
				pXDirect->m_pCurFoil  = pFoil;
				pXDirect->m_pCurPolar = NULL;
				pXDirect->m_pCurOpp   = NULL;
				m_pCurFoil = pXDirect->SetFoil(pFoil);
				pXDirect->SetPolar();
//				AFoil.SetFoils(m_pCurFoil);
				XFile.close();
				SetSaveState(false);
				AddRecentFile(PathName);
				return XFOILANALYSIS;
			}
		}
		else
		{
			if(end==".wpa")
			{
				if(!m_bSaved)
				{
					QString strong = "Save the current project ?";
					int resp =  QMessageBox::question(0,"QFLR5", strong,  QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
					if(resp==QMessageBox::Cancel)
					{
						XFile.close();
						return 0;
					}
					else if (resp==QMessageBox::Yes)
					{
						if(!SaveProject(m_FileName))
						{
							XFile.close();
							return 0;
						}
					}
				}

				DeleteProject();

				QDataStream ar(&XFile);
				ar.setByteOrder(QDataStream::LittleEndian);

				if(SerializeProject(ar, false))
				{
					m_pCurFoil = pXDirect->SetFoil();
					UpdateFoils();
					UpdateView();
				}

				AddRecentFile(PathName);
				SetSaveState(true);
				SetProjectName(PathName);

				XFile.close();
				return XFOILANALYSIS;
			}
		}
	}
	XFile.close();
	return 0;
}


void MainFrame::OnNewProject()
{
	if(!m_bSaved)
	{
		int resp = QMessageBox::question(window(), "QFLR5", "Save the current project ?",
										  QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);

		if (QMessageBox::Cancel == resp)
		{
			return;
		}
		else if (QMessageBox::Yes == resp)
		{
			if(SaveProject(m_FileName))
			{
				SetSaveState(true);
				statusBar()->showMessage("The project " + m_ProjectName + " has been saved");
			}
			else return; //save failed, don't close
		}
		else if (QMessageBox::No == resp)
		{
			DeleteProject();
		}
	}
	else
	{
		DeleteProject();
	}

	UpdateView();
}


void MainFrame::OnLoadFile()
{
	QString PathName;
	QXDirect *pXDirect = (QXDirect*)m_pXDirect;

	PathName = QFileDialog::getOpenFileName(this, "Open File",
											m_LastDirName, 
											"XFLR5 file (*.dat *.plr *.wpa)");
	int pos = PathName.lastIndexOf("/");
	if(pos>0) m_LastDirName = PathName.left(pos);
	if(!PathName.length())		return;

	m_iApp = LoadXFLR5File(PathName);

	if(m_iApp==0)
	{
//		if (app == XFOILANALYSIS) OnXDirect();
//		else if(app==MIAREX)      OnMiarex();
	}
	else if(m_iApp==XFOILANALYSIS)
	{
		if(m_oaPolar.size())
		{
			if(pXDirect->m_bPolar) pXDirect->CreatePolarCurves();
			else                   pXDirect->CreateOppCurves();
		}
		UpdateFoils();
		UpdateView();
	}
	else if(m_iApp==MIAREX)
	{
//		UpdateUFOs();
//		Miarex.SetUFO();
//		Miarex.SetBody();
//		UpdateView();
	}
	else if(m_iApp==DIRECTDESIGN)
	{
//		AFoil.SetFoils();
	}
}




void MainFrame::OnLogFile()
{
	QString FileName = QDir::tempPath() + "/QFLR5.log";
	QDesktopServices::openUrl(FileName);
}



void MainFrame::OnCurFoilStyle()
{
	if(!m_pCurFoil) return;

	LinePickerDlg dlg;
	dlg.InitDialog(m_pCurFoil->m_nFoilStyle, m_pCurFoil->m_nFoilWidth, m_pCurFoil->m_FoilColor);

	if(QDialog::Accepted==dlg.exec())
	{
		m_pCurFoil->m_FoilColor  = dlg.GetColor();
		m_pCurFoil->m_nFoilStyle = dlg.GetStyle();
		m_pCurFoil->m_nFoilWidth = dlg.GetWidth();
		SetSaveState(false);
	}
	UpdateView();
}

void MainFrame::OnExportCurGraph()
{
        QGraph *pGraph = NULL;
	switch(m_iApp)
	{
		case XFOILANALYSIS:
			QXDirect *pXDirect = (QXDirect*)m_pXDirect;
			pGraph = pXDirect->m_pCurGraph;
			break;
	}

	if(!pGraph) return;

	QFile DestFile;
	QString FileName;
	QString SelectedFilter;
	QFileDialog::Options options;
//	options |= QFileDialog::DontUseNativeDialog;

	pGraph->GetGraphName(FileName);
	FileName.replace("/", " ");
	FileName = QFileDialog::getSaveFileName(this, "Export Graph", m_LastDirName,
											"Text File (*.txt);;Comma Separated Values (*.csv)",
											&SelectedFilter, options);


	int pos, type;
	pos = FileName.lastIndexOf("/");
	if(pos>0) m_LastDirName = FileName.left(pos);

	QFile XFile(FileName);

	if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;
	pos = FileName.indexOf(".csv");
	if(pos>0) type=2; else type=1;

	pGraph->ExportToFile(XFile, type);

}

void MainFrame::OnResetCurGraphScales()
{
	QGraph *pGraph = NULL;
	switch(m_iApp)
	{
		case XFOILANALYSIS:
			QXDirect *pXDirect = (QXDirect*)m_pXDirect;
			pGraph = pXDirect->m_pCurGraph;
			break;
	}

	if(!pGraph) return;


	pGraph->SetAuto(true);
	pGraph->ResetXLimits();
	pGraph->ResetYLimits();
	UpdateView();
}

void MainFrame::OnRenameCurFoil()
{
	if(!m_pCurFoil) return;
	QXDirect *pXDirect = (QXDirect*)m_pXDirect;

	QString strong;
	bool bNotFound = true;
	int i,k,l;
	CFoil*pOldFoil;
	CPolar * pPolar;
	OpPoint * pOpPoint;

	QString OldName = m_pCurFoil->m_FoilName;


	while(bNotFound)
	{
		QStringList NameList;
		for(k=0; k<m_oaFoil.size(); k++)
		{
			pOldFoil = (CFoil*)m_oaFoil.at(k);
			NameList.append(pOldFoil->m_FoilName);
		}
		RenameDlg dlg(this);
		dlg.m_pstrArray = & NameList;
		dlg.m_strQuestion = "Enter the foil's new name";
		dlg.m_strName = OldName;
		bool bExists = false;
		dlg.InitDialog();
		int resp = dlg.exec();
		strong = dlg.m_strName;

		if(QDialog::Accepted == resp)
		{
			if (strong==OldName)
			{
				// do nothing
				bNotFound = false;
			}
			else
			{
				for (l=0; l<m_oaFoil.size(); l++)
				{
					pOldFoil = (CFoil*)m_oaFoil.at(l);
					if(pOldFoil->m_FoilName == strong)
					{
						bExists = true;
						break;
					}
				}
				if (!bExists)
				{
					bNotFound = false;// at last (users !...)
					// so rename the foil and associated polars and opps
					m_pCurFoil->m_FoilName = strong;
					bool bInserted = false;
					for(i=0;i<m_oaFoil.size();i++)
					{
						pOldFoil = (CFoil*)m_oaFoil.at(i);
						if(pOldFoil == m_pCurFoil)
						{
							m_oaFoil.removeAt(i);
							//and re-insert
							for(l=0;l<m_oaFoil.size();l++)
							{
								pOldFoil = (CFoil*)m_oaFoil.at(l);
								if(m_pCurFoil->m_FoilName.compare(pOldFoil->m_FoilName, Qt::CaseInsensitive)<0)
								{
									//then insert before
									m_oaFoil.insert(l, m_pCurFoil);
									bInserted = true;
									break;
								}
							}
							if(!bInserted)	m_oaFoil.append(m_pCurFoil);
							break;
						}
					}
					for (i=0; i<m_oaPolar.size(); i++)
					{
						pPolar = (CPolar*)m_oaPolar.at(i);
						if(pPolar->m_FoilName == OldName)
						{
							pPolar->m_FoilName = strong;
						}
					}
					for (i=0; i<m_oaOpp.size(); i++)
					{
						pOpPoint = (OpPoint*)m_oaOpp.at(i);
						if(pOpPoint->m_strFoilName == OldName)
						{
							pOpPoint->m_strFoilName = strong;
						}
					}
					SetSaveState(false);
				}
			}
		}
		else if(resp==10)
		{
			// user wants to overwrite an existing airfoil
			if (strong==OldName)
			{
				// do nothing
				bNotFound = false;
			}
			else
			{
				//So delete any foil with that name
				for (l=(int)m_oaFoil.size()-1;l>=0; l--)
				{
					pOldFoil = (CFoil*)m_oaFoil.at(l);
					if(pOldFoil->m_FoilName == strong)
					{
						if(m_pCurFoil == pOldFoil)           m_pCurFoil = NULL;
						if(pXDirect->m_pCurFoil == pOldFoil) pXDirect->m_pCurFoil = NULL;
						m_oaFoil.removeAt(l);
						delete pOldFoil;
					}
				}
				// delete all associated OpPoints
				OpPoint * pOpPoint;
				for (l=m_oaOpp.size()-1;l>=0;l--)
				{
					pOpPoint = (OpPoint*)m_oaOpp[l];
					if (pOpPoint->m_strFoilName == strong)
					{
						if(pOpPoint == pXDirect->m_pCurOpp) pXDirect->m_pCurOpp = NULL;
						m_oaOpp.removeAt(l);
						delete pOpPoint;
					}
				}
				// delete all Polar results for that airfoil
				CPolar * pPolar;
				for (l=m_oaPolar.size()-1;l>=0;l--)
				{
					pPolar = (CPolar*) m_oaPolar.at(l);
					if (pPolar->m_FoilName == strong)
					{
						if(pXDirect->m_pCurPolar == pPolar) pXDirect->m_pCurPolar = NULL;
						m_oaPolar.removeAt(l);
						delete pPolar;
					}
				}
				// finally add to array
				m_pCurFoil->m_FoilName = strong;
				for (i=0; i<m_oaPolar.size(); i++)
				{
					pPolar = (CPolar*)m_oaPolar.at(i);
					if(pPolar->m_FoilName == OldName)
					{
						pPolar->m_FoilName = strong;
					}
				}
				for (i=0; i<m_oaOpp.size(); i++)
				{
					pOpPoint = (OpPoint*)m_oaOpp.at(i);
					if(pOpPoint->m_strFoilName == OldName)
					{
						pOpPoint->m_strFoilName = strong;
					}
				}
	//			UpdateFoils();
				SetSaveState(false);
				bNotFound = false;//exit loop
			}
		}
		else
		{
			// Cancel so exit
			bNotFound = false;//exit loop
		}
	}

	pXDirect->SetFoil();
	UpdateFoils();
	UpdateView();
}

void MainFrame::OnRestoreToolbars()
{
	if(m_iApp==XFOILANALYSIS)
	{
		QXDirect *pXDirect = (QXDirect*)m_pXDirect;
		pXDirect->setVisible(!pXDirect->isVisible());
	}
}

void MainFrame::OnSaveProject()
{
	if (!m_ProjectName.length() || m_ProjectName=="*")
	{
		OnSaveProjectAs();
		return;
	}
	if(SaveProject(m_FileName))
	{
		SetSaveState(true);
		statusBar()->showMessage("The project " + m_ProjectName + " has been saved");
	}
}



bool MainFrame::OnSaveProjectAs()
{
	if(SaveProject())
	{
		SetProjectName(m_FileName);
		AddRecentFile(m_FileName);
		statusBar()->showMessage("The project " + m_ProjectName + " has been saved");
		SetSaveState(true);
	}
	return true;
}


void MainFrame::OnSaveViewToImageFile()
{
	QSize sz(m_p2DWidget->geometry().width(), m_p2DWidget->geometry().height());
	QImage img(sz, QImage::Format_RGB32);
	QPainter painter(&img);

	switch(m_iApp)
	{
		case XFOILANALYSIS:
			QXDirect *pXDirect = (QXDirect*)m_pXDirect;
			pXDirect->PaintView(painter);
			break;
	}

	QString FileName;
	QString Filter;
	switch(m_ImageFormat)
	{
		case 0 :
		{
			Filter = "Windows Bitmap (*.bmp)";
			break;
		}
		case 1 :
		{
			Filter = "JPEG (*.jpg)";
			break;
		}
		case 2 :
		{
			Filter = "Portable Network Graphics (*.png)";
			break;
		}

	}
	FileName = QFileDialog::getSaveFileName(this, "Save Image",
											m_LastDirName,
											"Windows Bitmap (*.bmp);;JPEG (*.jpg);;Portable Network Graphics (*.png)",
											&Filter);
	img.save(FileName);
	if(Filter == "Windows Bitmap (*.bmp)")                 m_ImageFormat = 0;
	else if(Filter == "JPEG (*.jpg)")                      m_ImageFormat = 1;
	else if(Filter == "Portable Network Graphics (*.png)") m_ImageFormat = 2;
/*
BMP	Windows Bitmap	Read/write
GIF	Graphic Interchange Format (optional)	Read
JPG	Joint Photographic Experts Group	Read/write
JPEG	Joint Photographic Experts Group	Read/write
PNG	Portable Network Graphics	Read/write
PBM	Portable Bitmap	Read
PGM	Portable Graymap	Read
PPM	Portable Pixmap	Read/write
TIFF	Tagged Image File Format	Read/write
XBM	X11 Bitmap	Read/write
XPM	X11 Pixmap	Read/write*/


}



void MainFrame::OnSelChangeFoil(int i)
{
	QXDirect *pXDirect = (QXDirect*)m_pXDirect;
	i=0;
	// Gets the new selected foil name and notifies XDirect
	// sets a polar, if any
	// then updates the polar combobox
//
	pXDirect->m_bAnimate = false;
//	m_OperDlgBar.m_ctrlAnimate.SetCheck(FALSE);
    QString strong;
    int sel = m_pctrlFoil->currentIndex();
    if (sel >=0) strong = m_pctrlFoil->itemText(sel);

    m_pCurFoil = GetFoil(strong);
	pXDirect->SetFoil(m_pCurFoil);
	pXDirect->SetPolar();
    m_iApp = XFOILANALYSIS;
    UpdatePolars();

	UpdateView();
}


void MainFrame::OnSelChangePolar(int i)
{
	i=0;
	QXDirect *pXDirect = (QXDirect*)m_pXDirect;
	pXDirect->m_bAnimate = false;
//	m_OperDlgBar.m_ctrlAnimate.SetCheck(FALSE);
    // Gets the new selected polar name and notifies XDirect
    QString strong;
	int sel = m_pctrlPolar->currentIndex();
    if (sel >= 0) strong = m_pctrlPolar->itemText(sel);
    m_iApp = XFOILANALYSIS;
	pXDirect->SetPolar(strong);
	UpdateOpps();
	UpdateView();
}

void MainFrame::OnSelChangeOpp(int i)
{
	QXDirect *pXDirect = (QXDirect*)m_pXDirect;
	pXDirect->m_bAnimate = false;
//	m_OperDlgBar.m_ctrlAnimate.SetCheck(FALSE);

    // Gets the new selected Opp name and notifies XDirect
    QString strong;

	if (i>=0) strong = m_pctrlOpPoint->itemText(i);
    m_iApp = XFOILANALYSIS;

    double Alpha;
    bool bOK;
    Alpha = strong.toFloat(&bOK);
    if(bOK)
    {
//		pXDirect->OnOper();//TODO
		pXDirect->SetOpp(Alpha);
		pXDirect->CreateOppCurves();
    }
    else
    {
	    QMessageBox::information(window(), tr("QFLR5"), tr("Unknown Operating point"));
		pXDirect->m_pCurOpp = NULL;
		pXDirect->SetOpp();
    }
	UpdateView();
}



void MainFrame::OnStyle()
{
	QXDirect *pXDirect = (QXDirect*)m_pXDirect;

	QGraph m_RefGraph;//which setttings ?

	DisplaySettingsDlg dlg(this);
	dlg.m_BackgroundColor = m_BackgroundColor;
	dlg.m_GraphBackColor  = m_GraphBackColor;
	dlg.m_TextColor       = m_TextColor;
	dlg.m_TextFont        = m_TextFont;
	dlg.m_pRefGraph       = &m_RefGraph;

	dlg.InitDialog();

	if(dlg.exec() ==QDialog::Accepted)
	{
		m_BackgroundColor = dlg.m_BackgroundColor;
		m_GraphBackColor  = dlg.m_GraphBackColor;
		m_TextColor       = dlg.m_TextColor;
		m_TextFont        = dlg.m_TextFont;
		pXDirect->m_pPolarGraph->SetBkColor(m_GraphBackColor);
		pXDirect->m_pCpGraph->SetBkColor(m_GraphBackColor);
		pXDirect->m_pCmGraph->SetBkColor(m_GraphBackColor);
		pXDirect->m_pCzGraph->SetBkColor(m_GraphBackColor);
		pXDirect->m_pTrGraph->SetBkColor(m_GraphBackColor);
		pXDirect->m_pUserGraph->SetBkColor(m_GraphBackColor);

		if(dlg.m_bIsGraphModified)
		{
			pXDirect->m_pPolarGraph->CopySettings(&m_RefGraph, false);
			pXDirect->m_pCpGraph->CopySettings(&m_RefGraph, false);
			pXDirect->m_pCmGraph->CopySettings(&m_RefGraph, false);
			pXDirect->m_pCzGraph->CopySettings(&m_RefGraph, false);
			pXDirect->m_pTrGraph->CopySettings(&m_RefGraph, false);
			pXDirect->m_pUserGraph->CopySettings(&m_RefGraph, false);
		}
	}
}


void MainFrame::openRecentFile()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action) return;

	QXDirect *pXDirect = (QXDirect*) m_pXDirect;

	m_iApp = LoadXFLR5File(action->data().toString());

	if(m_iApp==0)
	{
//remove filename from list
		QString FileName = action->data().toString();
		m_RecentFiles.removeAll(FileName);
		updateRecentFileActions();
	}
	else if(m_iApp==XFOILANALYSIS)
	{
		SetSaveState(false);

		if(m_oaPolar.size())
		{
			if(pXDirect->m_bPolar) pXDirect->CreatePolarCurves();
			else                   pXDirect->CreateOppCurves();
		}
		UpdateFoils();
		UpdateView();
	}
	else if(m_iApp==MIAREX)
	{
//		UpdateUFOs();
//		Miarex.SetUFO();
//		Miarex.SetBody();
//		Miarex.UpdateView();
	}
	else if(m_iApp==DIRECTDESIGN)
	{
//		AFoil.SetFoils();
	}
}


CFoil* MainFrame::ReadFoilFile(QTextStream &in)
{
	QString Strong, StrTemp;
	QString FoilName;
	CFoil* pFoil = NULL;
	int pos, res, line, i, ip;
	pos = line = 0;
	double x,y, area;

	bool bRead;

	QByteArray textline;
	const char *text;

	pFoil = new CFoil();
	if(!pFoil)	return NULL;

	while(pos>=0 && !in.atEnd())
	{
		Strong = in.readLine();
		pos = Strong.indexOf("#",0);
	}

	if(!in.atEnd())
	{
		textline = Strong.toAscii();
		text = textline.constData();
		res = sscanf(text, "%lf%lf", &x,&y);
		if(res==2)
		{
			//there isn't a name on the first line
			FoilName = "New Foil";
			{
				pFoil->xb[0] = x;
				pFoil->yb[0] = y;
				pFoil->nb=1;
			}
		}
		else FoilName = Strong;
		// remove fore and aft spaces
		FoilName = FoilName.trimmed();
	}

	bRead = true;

	while (bRead)
	{
		Strong = in.readLine();
		pos = Strong.indexOf("#",0);

		if (bRead && pos<0)
		{

			textline = Strong.toAscii();
			text = textline.constData();
			res = sscanf(text, "%lf%lf", &x,&y);
			if(res==2)
			{
				pFoil->xb[pFoil->nb] = x;
				pFoil->yb[pFoil->nb] = y;
				pFoil->nb++;
				if(pFoil->nb>IQX)
				{
					delete pFoil;
					return NULL;
				}
			}
			else bRead = false;
		}
	}

	pFoil->m_FoilName = FoilName;

// Check if the foil was written clockwise or counter-clockwise

	area = 0.0;
	for (i=0; i<pFoil->nb; i++)
	{
		if(i==pFoil->nb-1)	ip = 0;
		else				ip = i+1;
		area +=  0.5*(pFoil->yb[i]+pFoil->yb[ip])*(pFoil->xb[i]-pFoil->xb[ip]);
	}

	if(area < 0.0)
	{
		//reverse the points order
		double xtmp, ytmp;
		for (int i=0; i<pFoil->nb/2; i++)
		{
			xtmp         = pFoil->xb[i];
			ytmp         = pFoil->yb[i];
			pFoil->xb[i] = pFoil->xb[pFoil->nb-i-1];
			pFoil->yb[i] = pFoil->yb[pFoil->nb-i-1];
			pFoil->xb[pFoil->nb-i-1] = xtmp;
			pFoil->yb[pFoil->nb-i-1] = ytmp;
		}
	}

	memcpy(pFoil->x, pFoil->xb, sizeof(pFoil->xb));
	memcpy(pFoil->y, pFoil->yb, sizeof(pFoil->yb));
	pFoil->n=pFoil->nb;

	pFoil->m_FoilColor = QColor(255,0,120);
//   delete pFoil;
//	m_pglWidget->m_pFoil = pFoil;
	pFoil->InitFoil();

	return pFoil;
}


CFoil * MainFrame::ReadPolarFile(QDataStream &ar)
{
//	QXDirect *pXDirect = (QXDirect*)m_pXDirect;
	QString FoilName;
	CFoil* pFoil = NULL;
	CPolar *pPolar = NULL;
	CPolar * pOldPolar;
	int i, n, l;

	ar >> n;

	if(n<100000)
	{
		//old format
		QMessageBox::information(window(), "QFLR5", "Obsolete format, cannot read");
		return NULL;
	}
	else if (n >=100000)
	{
		//new format XFLR5 v1.99+
		//first read all available foils
		ar>>n;
		for (i=0;i<n; i++)
		{
			pFoil = new CFoil();
			pFoil->Serialize(ar, false);
			if (!pFoil->m_FoilName.length())
			{
				delete pFoil;
				return NULL;
			}
			AddFoil(pFoil);
		}

		//next read all available polars

		ar>>n;
		for (i=0;i<n; i++)
		{
			pPolar = new CPolar();

			pPolar->m_Color = QColor(56,128,188,127);
			pPolar->Serialize(ar, false);
			if (!pPolar->m_FoilName.length())
			{
				delete pPolar;
				return pFoil;
			}
			for (l=0; l<m_oaPolar.size(); l++)
			{
				pOldPolar = (CPolar*)m_oaPolar[l];
				if (pOldPolar->m_FoilName == pPolar->m_FoilName &&
					pOldPolar->m_PlrName  == pPolar->m_PlrName)
				{
					//just overwrite...
					m_oaPolar.removeAt(l);
					delete pOldPolar;
					//... and continue to add
				}
			}
			pPolar = AddPolar(pPolar);

		}
	}
	return pFoil;
}



void MainFrame::RemoveOpPoint(bool bCurrent)
{
	int i;
	QXDirect *pXDirect =(QXDirect*)m_pXDirect;

	if(bCurrent)
	{
		// we remove only the current OpPoint
		OpPoint* pOpPoint = pXDirect->m_pCurOpp;
		OpPoint* opt;
		for (i=0; i<m_oaOpp.size(); i++)
		{
			opt =(OpPoint*)m_oaOpp.at(i);
			if (pOpPoint == opt)
			{
				m_oaOpp.removeAt(i);
				delete pOpPoint;
				break;
			}
		}
	}
	else
	{
		// kill'em all
		OpPoint* pOpPoint;
		for (i=m_oaOpp.size()-1; i>=0;i--)
		{
			pOpPoint =(OpPoint*)m_oaOpp.at(i);
			if (pOpPoint->m_strFoilName == pXDirect->m_pCurFoil->m_FoilName &&
				pOpPoint->m_strPlrName == pXDirect->m_pCurPolar->m_PlrName)
			{
				m_oaOpp.removeAt(i);
				delete pOpPoint;
			}
		}
	}

	pXDirect->m_pCurOpp = NULL;
}


bool MainFrame::SaveProject(QString PathName)
{
	if(!PathName.length())
	{
		QString Filter = ".wpa";
		QString FileName = m_ProjectName;
		if(FileName.right(1)=="*") 	FileName = FileName.left(FileName.length()-1);
		FileName.replace("/", " ");
/*		QFileDialog dlg;
		dlg.setFilter(".wpa");
		dlg.setDirectory(m_LastDirName);
		dlg.selectFile(FileName);
		PathName = dlg.getSaveFileName(this);*/
		PathName = QFileDialog::getSaveFileName(this, "Save the Project File",
												m_LastDirName+"/"+FileName,
												"XFLR5 Project File (*.wpa)", &Filter);
		int pos = PathName.indexOf(".wpa", Qt::CaseInsensitive);
		if(pos<0) PathName += ".wpa";
		pos = PathName.lastIndexOf("/");
		if(pos>0) m_LastDirName = PathName.left(pos);
	}
	if(!PathName.length()) return false;//nothing more to do

	QString strong;
	QFile fp(PathName);

	if (!fp.open(QIODevice::WriteOnly))
	{
		QMessageBox::information(window(), tr("QFLR5"), tr("Could not open the file for writing"));
		return false;
	}

	QDataStream ar(&fp);
	ar.setByteOrder(QDataStream::LittleEndian);
	SerializeProject(ar,true);
	m_FileName = PathName;
	fp.close();
	return true;
}


void MainFrame::SaveSettings()
{
	QXDirect *pXDirect = (QXDirect*)m_pXDirect;
	QString FileName = QDir::tempPath() + "/QFLR5.set";
	QFile *pXFile = new QFile(FileName);

	if (!pXFile->open(QIODevice::WriteOnly))
	{
		QMessageBox::information(window(), tr("QFLR5"), tr("could not write saved settings"));
		return;
	}

	QDataStream ar(pXFile);

	ar << 100507;
	ar << frameGeometry().x();
	ar << frameGeometry().y();
	ar << frameGeometry().width();
	ar << frameGeometry().height();
	ar << isMaximized();
	ar << m_StyleName;

	ar << m_LastDirName;

	ar << m_LengthUnit << m_AreaUnit << m_WeightUnit << m_SpeedUnit << m_ForceUnit << m_MomentUnit;
	ar << m_BackgroundColor <<  m_GraphBackColor << m_TextColor;
	ar << m_TextFont ;
	ar << m_ImageFormat;

	ar << m_RecentFiles.size();
	for(int i=0; i<m_RecentFiles.size(); i++)
		ar << m_RecentFiles.at(i);

	pXDirect->SaveSettings(ar);

	pXFile->close();
}


bool MainFrame::SelectFoil(CFoil *pFoil)
{
    //Selects pFoil in the combobox and returns true
    //On error, selects the first and returns false
    return true;
    if(!pFoil) return false;
    int pos = m_pctrlFoil->findText(pFoil->m_FoilName);
    if(pos>=0)
    {
	    m_pctrlFoil->setCurrentIndex(pos);
	    return true;
    }
    return false;
}

bool MainFrame::SelectPolar(CPolar *pPolar)
{
    //Selects pPolar in the combobox and returns true
    //On error, selects the first and returns false
    if(!pPolar) return false;

    CPolar *pOldPolar;
    for(int i=0; i<m_pctrlPolar->count(); i++)
    {
	pOldPolar = GetPolar(m_pctrlPolar->itemText(i));
	if(pOldPolar && pPolar==pOldPolar)
	{
	    //TODO : check if this activates the Selchange signal
	    m_pctrlPolar->setCurrentIndex(i);
	    return true;
	}
    }
    return false;
}

bool MainFrame::SelectOpPoint(OpPoint *pOpp)
{
    //Selects pOpp in the combobox and returns true
    //On error, selects the first and returns false
	QXDirect *pXDirect = (QXDirect*)m_pXDirect;
	CPolar *pCurPlr    = pXDirect->m_pCurPolar;
    if(!pOpp || !pCurPlr) return false;
    QString strong;
    double alpha, Re;

    for(int i=0; i<m_pctrlOpPoint->count(); i++)
    {
	    if(pCurPlr->m_Type !=4)
	    {
		    alpha = m_pctrlOpPoint->itemText(i).toDouble();
		    if(fabs(alpha-pOpp->Alpha)<0.01)
		    {
			    m_pctrlOpPoint->setCurrentIndex(i);
			    return true;
		    }
	    }
	    else
	    {
		    Re = m_pctrlOpPoint->itemText(i).toDouble();
		    if(fabs(Re-pOpp->Reynolds)<1.0)
		    {
			    m_pctrlOpPoint->setCurrentIndex(i);
			    return true;
		    }
	    }
    }
    return false;
}


bool MainFrame::SerializeProject(QDataStream &ar, bool bIsStoring)
{
//	CWaitCursor Wait;
	QMiarex *pMiarex = (QMiarex*)m_pMiarex;
	AFoil *pAFoil = (AFoil*)m_pAFoil;

	CWing *pWing     = NULL;
	CWPolar *pWPolar = NULL;
	CWOpp *pWOpp     = NULL;
	CPOpp *pPOpp     = NULL;
	CPlane *pPlane   = NULL;
	CBody *pBody     = NULL;
	CPolar *pPolar   = NULL;
	CFoil *pFoil     = NULL;


	QString str, strong;
	int i, n, j, k;
	float f;

	if (bIsStoring)
	{
		// storing code
		ar << 100012;
		// 100012 : Added sideslip
		// 100011 : Added Body serialization
		// 100010 : Converted to I.S. units
		// 100009 : added serialization of opps in numbered format
		// 100008 : Added m_WngAnalysis.m_bTiltedGeom, m_WngAnalysis.m_bWakeRollUp
		// 100006 : version 2.99Beta001 format
		// 100005 : version 2.00 format
		//Archive format introduced in Beta V22
		// Write units //added in Beta v16
		// Save analysis settings // added in Beta v22
		ar << m_LengthUnit;
		ar << m_AreaUnit;
		ar << m_WeightUnit;
		ar << m_SpeedUnit;
		ar << m_ForceUnit;
		ar << m_MomentUnit;

		ar << pMiarex->m_WngAnalysis.m_Type;
		ar << (float)pMiarex->m_WngAnalysis.m_Weight;
		ar << (float)pMiarex->m_WngAnalysis.m_QInf;
		ar << (float)pMiarex->m_WngAnalysis.m_XCmRef;
		ar << (float)pMiarex->m_WngAnalysis.m_Density;
		ar << (float)pMiarex->m_WngAnalysis.m_Viscosity;
		ar << (float)pMiarex->m_WngAnalysis.m_Alpha;
		ar << (float)pMiarex->m_WngAnalysis.m_Beta;
		ar << pMiarex->m_WngAnalysis.m_AnalysisType;

		if (pMiarex->m_WngAnalysis.m_bVLM1)   ar << 1;
		else								ar << 0;
//		if (pMiarex->m_WngAnalysis.m_bMiddle) ar << 1; else ar << 0;
		ar <<1;
		if (pMiarex->m_WngAnalysis.m_bTiltedGeom) ar << 1;
		else									ar << 0;
		if (pMiarex->m_WngAnalysis.m_bWakeRollUp) ar << 1;
		else									ar << 0;

		ar << (int)m_oaWing.size() ;//number of wings
		// Store the wings
		for (i=0; i<m_oaWing.size();i++)
		{
			pWing = (CWing*)m_oaWing.at(i);
			pWing->SerializeWing(ar, true);
		}

		// now store all the WPolars
		ar << (int)m_oaWPolar.size();
		for (i=0; i<m_oaWPolar.size();i++)
		{
			pWPolar = (CWPolar*)m_oaWPolar.at(i);
			pWPolar->m_pParent = this;
			pWPolar->SerializeWPlr(ar, bIsStoring);
		}

		// next store all the WOpps
		if(m_bSaveWOpps)
		{
			ar << (int)m_oaWOpp.size();
			for (i=0; i<m_oaWOpp.size();i++)
			{
				pWOpp = (CWOpp*)m_oaWOpp.at(i);
				pWOpp->SerializeWOpp(ar, bIsStoring);
			}
		}
		else ar << 0;

		// then the foils,  polars and Opps
                WritePolars(ar);

		// next the bodies
		ar << (int)m_oaBody.size();
		for (i=0; i<m_oaBody.size();i++)
		{
			pBody = (CBody*)m_oaBody.at(i);
			pBody->SerializeBody(ar, bIsStoring);
		}

		// last write the planes...
		ar << (int)m_oaPlane.size();
		for (i=0; i<m_oaPlane.size();i++)
		{
			pPlane = (CPlane*)m_oaPlane.at(i);
			pPlane->SerializePlane(ar, bIsStoring);
		}

		if(m_bSaveWOpps)
		{
			// not forgetting their POpps
			ar << (int)m_oaPOpp.size();
			for (i=0; i<m_oaPOpp.size();i++)
			{
				pPOpp = (CPOpp*)m_oaPOpp.at(i);
				pPOpp->SerializePOpp(ar, bIsStoring);
			}
		}
		else ar << 0;

		pAFoil->m_pSF->Serialize(ar, bIsStoring);
		pAFoil->m_pPF->Serialize(ar, bIsStoring);

		return true;
	}
	else
	{
		// LOADING CODE

		int ArchiveFormat;
		ar >> n;

		if(n<100000)
		{
		// then n is the number of wings to load
		// up to v1.99beta15
			ArchiveFormat = 100000;
		}
		else
		{
		// then n is the ArchiveFormat number
			ArchiveFormat = n;
			ar >> m_LengthUnit;
			ar >> m_AreaUnit;
			ar >> m_WeightUnit;
			ar >> m_SpeedUnit;
			ar >> m_ForceUnit;
			if(ArchiveFormat>=100005)
			{
				ar >> m_MomentUnit;
			}

			SetUnits(m_LengthUnit, m_AreaUnit, m_SpeedUnit, m_WeightUnit, m_ForceUnit, m_MomentUnit,
					 m_mtoUnit, m_m2toUnit, m_mstoUnit, m_kgtoUnit, m_NtoUnit, m_NmtoUnit);

			if(ArchiveFormat>=100004)
			{
				ar >> pMiarex->m_WngAnalysis.m_Type;
				ar >> f; pMiarex->m_WngAnalysis.m_Weight=f;
				ar >> f; pMiarex->m_WngAnalysis.m_QInf=f;
				ar >> f; pMiarex->m_WngAnalysis.m_XCmRef=f;
				if(ArchiveFormat<100010) pMiarex->m_WngAnalysis.m_XCmRef=f/1000.0;
				ar >> f; pMiarex->m_WngAnalysis.m_Density=f;
				ar >> f; pMiarex->m_WngAnalysis.m_Viscosity=f;
				ar >> f; pMiarex->m_WngAnalysis.m_Alpha=f;
				if(ArchiveFormat>=100012)
				{
					ar >>f; pMiarex->m_WngAnalysis.m_Beta=f;
				}

				ar >> pMiarex->m_WngAnalysis.m_AnalysisType;

			}
			if(ArchiveFormat>=100006)
			{
				ar >> k;
				if (k) pMiarex->m_WngAnalysis.m_bVLM1 = true;
				else   pMiarex->m_WngAnalysis.m_bVLM1 = false;

				ar >> k;
//					if (k) pMiarex->m_WngAnalysis.m_bMiddle = true;
//					else   pMiarex->m_WngAnalysis.m_bMiddle = false;
			}

			if(ArchiveFormat>=100008){
				ar >> k;
				if (k) pMiarex->m_WngAnalysis.m_bTiltedGeom = true;
				else   pMiarex->m_WngAnalysis.m_bTiltedGeom = false;

				ar >> k;
				if (k) pMiarex->m_WngAnalysis.m_bWakeRollUp = true;
				else   pMiarex->m_WngAnalysis.m_bWakeRollUp = false;
			}
			// and read n again
			ar >> n;
		}

		// WINGS FIRST
		for (i=0;i<n; i++)
		{
			pWing = new CWing;


			if (!pWing->SerializeWing(ar, bIsStoring))
			{
					if(pWing) delete pWing;
					return false;
			}
			if(pWing)
			{
//					pWing->ComputeGeometry();
				pWing = pMiarex->AddWing(pWing);
			}
			else
			{
				if(pWing) delete pWing;
			}
		}


		//THEN WPOLARS
		ar >> n;// number of WPolars to load
		for (i=0;i<n; i++)
		{
			pWPolar = new CWPolar;

			if (!pWPolar->SerializeWPlr(ar, bIsStoring))
			{
				if(pWPolar) delete pWPolar;
				return false;
			}
			if(!pWPolar->m_AnalysisType==1 && ArchiveFormat <100003)
				pWPolar->ResetWPlr();//former VLM version was flawed
			pWPolar = pMiarex->AddWPolar(pWPolar);
		}

		//THEN WOPPS
		ar >> n;// number of WOpps to load
		for (i=0;i<n; i++)
		{
			pWOpp = new CWOpp();
			bool bWOppOK;
			if (ArchiveFormat<=100001)
			{
				return false;
			}
			else
			{
				bWOppOK = pWOpp->SerializeWOpp(ar, bIsStoring);
				if(pWOpp && bWOppOK)
				{
					pWing = pMiarex->GetWing(pWOpp->m_WingName);
					if(pWing) pWOpp->m_MAChord = pWing->m_MAChord;
					else      bWOppOK = false;
				}
			}
			if (!bWOppOK)
			{
				if(pWOpp) delete pWOpp;
				return false;
			}

			if(pWOpp->m_AnalysisType==1 || ArchiveFormat >=100006)//former VLM version was flawed
						pMiarex->InsertWOpp(pWOpp);

			else delete pWOpp;
		}

		ar >> n;
		//=100000 ... unused

		//THEN FOILS, POLARS and OPPS
		if(ArchiveFormat>=100009)
		{
			if(!LoadPolarFileV3(ar, bIsStoring,100002))
			{
				return false;
			}
		}
		else
		{
			if(ArchiveFormat>=100006)
			{
				if(!LoadPolarFileV3(ar, bIsStoring))
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}

		if(n==100000)
		{
			for (j=0; j<m_oaPolar.size(); j++)
			{
				pPolar = (CPolar*)m_oaPolar.at(j);
				for (k=0; k<m_oaFoil.size(); k++)
				{
					pFoil = (CFoil*)m_oaFoil.at(k);
					if(pFoil->m_FoilName==pPolar->m_FoilName)
					{
						pPolar->m_ACrit    = 9.0;
						pPolar->m_XTop     = 1.0;
						pPolar->m_XBot     = 1.0;
						str = QString("_N%1").arg(9.0,4,'f',1);
						pPolar->m_PlrName += str;
						break;
					}
				}
			}
		}

		if(ArchiveFormat>=100011)
		{
			ar >> n;// number of Bodies to load
			for (i=0;i<n; i++)
			{
				pBody = new CBody();

				if (!pBody->SerializeBody(ar, bIsStoring))
				{
					if(pPOpp) delete pPOpp;
				}
				pMiarex->AddBody(pBody);
			}
		}

		if(ArchiveFormat>=100006)
		{ //read the planes
			ar >> n;
			// last read the planes
			for (i=0; i<n;i++)
			{
				pPlane = new CPlane();
				if(pPlane)
				{
					if(pPlane->SerializePlane(ar, bIsStoring)) pMiarex->AddPlane(pPlane);
					else
					{
						if(pPlane) delete pPlane;
						return false;
					}
				}
			}
			//and their pPolars

			if(ArchiveFormat <100007)
			{
				ar >> n;// number of PPolars to load
				for (i=0;i<n; i++)
				{
					pWPolar = new CWPolar();

					if (!pWPolar->SerializeWPlr(ar, bIsStoring))
					{

						if(pWPolar) delete pWPolar;
						return false;
					}
					if(!pWPolar->m_AnalysisType==1 && ArchiveFormat <100003)
						pWPolar->ResetWPlr();
					pMiarex->AddWPolar(pWPolar);
				}
			}

			ar >> n;// number of POpps to load
			for (i=0;i<n; i++)
			{
				pPOpp = new CPOpp();

				if (!pPOpp->SerializePOpp(ar, bIsStoring))
				{
					if(pPOpp) delete pPOpp;
					return false;
				}
				pMiarex->AddPOpp(false, NULL, NULL, NULL, pPOpp);

			}
			//lock all bodies with results
			for(i=0;i<m_oaWPolar.size(); i++)
			{
				pWPolar = (CWPolar*)m_oaWPolar.at(i);
				pPlane  = pMiarex->GetPlane(pWPolar->m_UFOName);
				if(pPlane && pWPolar->m_Alpha.size())
				{
					if(pPlane->m_bBody && pPlane->m_pBody)
						pPlane->m_pBody->m_bLocked = true;
				}
			}
		}

		pMiarex->m_pCurPOpp = NULL;

		pAFoil->m_pSF->Serialize(ar, bIsStoring);

		pAFoil->m_pPF->Serialize(ar, bIsStoring);

		for (i=0; i<m_oaWing.size();i++)
		{
			pWing = (CWing*)m_oaWing[i];
			pWing->ComputeGeometry();
		}

		return true;
	}
}


CFoil* MainFrame::SetModFoil(CFoil* pNewFoil, bool bKeepExistingFoil)
{
	// Adds the buffer foil to the ObArray,
	// gives it a proper name, FoilName or another,
	// selects it ,
	// and initializes XFoil, comboboxes and everything.

	QXDirect *pXDirect = (QXDirect*)m_pXDirect;
	int j,k,l;
	bool bExists = false;
	bool bNotFound = true;
	pNewFoil->m_bSaved = false;
	CFoil*pOldFoil;

	if(!pNewFoil->m_FoilName.length())
	{
		bExists = true;
	}
	else
	{
		for (j=0; j<m_oaFoil.size(); j++)
		{
			pOldFoil = (CFoil*)m_oaFoil.at(j);
			if (pOldFoil->m_FoilName == pNewFoil->m_FoilName)
			{
				bExists = true;
				if(bKeepExistingFoil)
				{
						delete pNewFoil;
						pNewFoil = NULL;
						m_pCurFoil = NULL;
						return pOldFoil;
				}
				break;
			}
		}
	}
	if(bExists)
	{
		while(bNotFound)
		{
			CFoil *pFoil;
			QStringList NameList;
			for(k=0; k<m_oaFoil.size(); k++)
			{
				pFoil = (CFoil*)m_oaFoil.at(k);
				NameList.append(pFoil->m_FoilName);
			}
			RenameDlg dlg(this);
			dlg.m_pstrArray = & NameList;
			dlg.m_strQuestion = "A foil of that name already exists\r\nPlease enter a new name";
			dlg.m_strName = pNewFoil->m_FoilName;
			dlg.InitDialog();

			bool exists = false;
			QString strong;
			int resp = dlg.exec();
			strong = dlg.m_strName;

			if(QDialog::Accepted == resp)
			{
				strong = dlg.m_strName;
				for (l=0; l<m_oaFoil.size(); l++)
				{
					pOldFoil = (CFoil*)m_oaFoil.at(l);
					if(pOldFoil->m_FoilName == strong) exists = true;
				}
				if (!exists)
				{
					bNotFound = false;// at last (users !...)
					// so create and add foil to array
					pNewFoil->m_FoilName = strong;

					AddFoil(pNewFoil);
				}
			}
			else if(resp==10)
			{
				// user wants to overwrite existing airfoil
				//So delete any foil with that name
				for (l=m_oaFoil.size()-1;l>=0; l--)
				{
					pOldFoil = (CFoil*)m_oaFoil.at(l);
					if(pOldFoil->m_FoilName == strong)
					{
						m_oaFoil.removeAt(l);
						delete pOldFoil;
						if(m_pCurFoil == pOldFoil)           m_pCurFoil = NULL;
						if(pXDirect->m_pCurFoil == pOldFoil) pXDirect->m_pCurFoil = NULL;
					}
				}
				// delete all associated OpPoints
				OpPoint * pOpPoint;
				for (l=m_oaOpp.size()-1;l>=0;l--)
				{
					pOpPoint = (OpPoint*)m_oaOpp[l];
					if (pOpPoint->m_strFoilName == strong)
					{
						m_oaOpp.removeAt(l);
						if(pOpPoint == pXDirect->m_pCurOpp) pXDirect->m_pCurOpp = NULL;
						delete pOpPoint;
					}
				}
				// delete all Polar results for that airfoil, but keep polar for analysis
				CPolar * pPolar;
				for (l=0; l <m_oaPolar.size();l++)
				{
					pPolar = (CPolar*) m_oaPolar.at(l);
					if (pPolar->m_FoilName == strong)
					{
						pPolar->ResetPolar();
					}
				}
				// finally add to array
				pNewFoil->m_FoilName = strong;
				AddFoil(pNewFoil);
				bNotFound = false;//exit loop
			}
			else
			{
				// Cancel so exit
				delete pNewFoil;
				pNewFoil = NULL;
				m_pCurFoil = NULL;
				return NULL;// foil not added
			}
		}
	}
	else{
		AddFoil(pNewFoil);
	}
	if(m_iApp == XFOILANALYSIS) pXDirect->SetFoil(pNewFoil);
	m_pCurFoil = pNewFoil;
	return pNewFoil;// foil added
}


void MainFrame::SetProjectName(QString PathName)
{
	m_FileName = PathName;
	int pos = PathName.lastIndexOf("/");
	if (pos>0) m_ProjectName = PathName.right(PathName.length()-pos-1);
	else m_ProjectName = PathName;
	if(m_ProjectName.length()>4)
	{
		m_ProjectName = m_ProjectName.left(m_ProjectName.length()-4);
		m_pctrlProjectName->setText(m_ProjectName);
	}
}


void MainFrame::SetSaveState(bool bSave)
{
	m_bSaved = bSave;

	int len = m_ProjectName.length();
	if(m_ProjectName.right(1)=="*") m_ProjectName = m_ProjectName.left(len-1);
	if (!bSave)
	{
		m_ProjectName += "*";
		m_pctrlProjectName->setText(m_ProjectName);
	}
}


QString MainFrame::ShortenFileName(QString &PathName)
{
	QString strong, strange;
	if(PathName.length()>60)
	{
		int pos = PathName.lastIndexOf('/');
		if (pos>0)
		{
			strong = '/'+PathName.right(PathName.length()-pos-1);
			strange = PathName.left(60-strong.length()-6);
			pos = strange.lastIndexOf('/');
			if(pos>0) strange = strange.left(pos)+"/...  ";
			strong = strange+strong;
		}
		else
		{
			strong = PathName.left(40);
		}

	}
	else strong = PathName;
	return strong;
}


void MainFrame::UpdateFoils()
{
	// fills combobox with foil names
	// then selects XDirect current foil if any, else selects the first, if any
	// else disables the combobox
	QXDirect *pXDirect = (QXDirect*)m_pXDirect;
	int i, pos;
	QString strong;
	m_pctrlFoil->clear();

	CFoil *pFoil;
	m_pCurFoil = pXDirect->m_pCurFoil;

	for (i=0; i<m_oaFoil.size(); i++)
	{
		pFoil = (CFoil*)m_oaFoil[i];
		m_pctrlFoil->addItem(pFoil->m_FoilName);
	}

	if (m_pctrlFoil->count())
	{
		m_pctrlFoil->setEnabled(true);
		//select the current foil, if any...
		if (m_pCurFoil)
		{
			pos = m_pctrlFoil->findText(m_pCurFoil->m_FoilName);
			if (pos>=0) m_pctrlFoil->setCurrentIndex(pos);
			else
			{
				// if error, select the first...
				m_pctrlFoil->setCurrentIndex(0);
				strong = m_pctrlFoil->itemText(0);
				//...and set it
//				pXDirect->m_pCurFoil = GetFoil(strong);
//				pXDirect->SetFoil(strong);
			}
		}
		//... else select the first
		else
		{
			// if error, select the first...
			m_pctrlFoil->setCurrentIndex(0);
			strong = m_pctrlFoil->itemText(0);
			//...and set it

			pXDirect->SetFoil(GetFoil(strong));
		}
	}
	else
	{
		m_pctrlFoil->clear();
		m_pctrlFoil->setEnabled(false);
		pXDirect->SetFoil();
	}
	UpdatePolars();
}

void MainFrame::UpdatePolars()
{
    // fills combobox with polar names associated to XDirect' current foil
    // then selects XDirect current polar if any, else selects the first, if any
    // else disables the combobox
    // sets the polar in XDirect in all cases
	QXDirect *pXDirect = (QXDirect*)m_pXDirect;
	int i, size, pos;
    CPolar *pPolar;
    QString strong;
    m_pctrlPolar->clear();

	m_pCurFoil = pXDirect->m_pCurFoil;

    if(!m_pCurFoil || !m_pCurFoil->m_FoilName.length())
    {
		m_pctrlPolar->setEnabled(false);
		m_pctrlOpPoint->clear();
		m_pctrlOpPoint->setEnabled(false);
		return;
	}

    size = 0;
    //count the number of polars associated to the current foil
    for (i=0; i<m_oaPolar.size(); i++)
    {
	pPolar = (CPolar*)m_oaPolar[i];
	if(pPolar->m_FoilName == m_pCurFoil->m_FoilName)
	{
	    size++;
	}
    }
    if (size)
    {

		// if any
		m_pctrlPolar->setEnabled(true);

		for (i=0; i<m_oaPolar.size(); i++)
		{
			pPolar = (CPolar*)m_oaPolar[i];
			if(pPolar->m_FoilName == m_pCurFoil->m_FoilName)
			{
				m_pctrlPolar->addItem(pPolar->m_PlrName);
			}
		}
		if(pXDirect->m_pCurPolar)
		{
			pos = m_pctrlPolar->findText(pXDirect->m_pCurPolar->m_PlrName);
			if(pos>=0) m_pctrlPolar->setCurrentIndex(pos);
			else
			{
				m_pctrlPolar->setCurrentIndex(0);
				strong = m_pctrlPolar->itemText(0);
		//		pXDirect->SetPolar(strong);
			}
		}
		else
		{
			m_pctrlPolar->setCurrentIndex(0);
			strong = m_pctrlPolar->itemText(0);
	//			pXDirect->SetPolar(strong);
		}
    }
    else
    {
		// otherwise disable control
		m_pctrlPolar->setEnabled(false);
	//		pXDirect->SetPolar();
    }

    UpdateOpps();
}

void MainFrame::UpdateOpps()
{
    // fills the combobox with the Opp names associated to XDirect's current foil
    // then selects XDirect current opp if any, else selects the first, if any
    // else disables the combobox

	QXDirect *pXDirect = (QXDirect*)m_pXDirect;
	int i, pos;
    OpPoint *pOpp;
    QString strong, str;
    m_pctrlOpPoint->clear();

//	m_pCurFoil = pXDirect->m_pCurFoil;
	CPolar *pCurPlr    = pXDirect->m_pCurPolar;

    if (!m_pCurFoil || !m_pCurFoil->m_FoilName.length() || !pCurPlr  || !pCurPlr->m_PlrName.length())
    {
		m_pctrlOpPoint->clear();
		m_pctrlOpPoint->setEnabled(false);
		return;
    }

    int size = 0;
    //count the number of Opps associated to the current foil & polar
    for (i=0; i<m_oaOpp.size(); i++)
    {
		pOpp = (OpPoint*)m_oaOpp[i];
		if (pOpp->m_strFoilName == m_pCurFoil->m_FoilName && pOpp->m_strPlrName  == pCurPlr->m_PlrName)
		{
			size++;
		}
    }

    if (size)
    {
		// if any
		m_pctrlOpPoint->setEnabled(true);
		for (i=0; i<m_oaOpp.size(); i++)
		{
			pOpp = (OpPoint*)m_oaOpp[i];
			if (pOpp->m_strFoilName == m_pCurFoil->m_FoilName && pOpp->m_strPlrName  == pCurPlr->m_PlrName)
			{
				if (pCurPlr->m_Type !=4)
				{
					if(fabs(pOpp->Alpha)<0.0001) pOpp->Alpha = 0.0001;
					str = QString("%1").arg(pOpp->Alpha,8,'f',2);
					m_pctrlOpPoint->addItem(str);
				}
				else
				{
					str = QString("%1").arg(pOpp->Reynolds,8,'f',0);
					m_pctrlOpPoint->addItem(str);
				}
			}
		}
		if (pXDirect->m_pCurOpp && pXDirect->m_pCurOpp->m_strFoilName==pXDirect->m_pCurFoil->m_FoilName)
		{
			//select it
			if (pCurPlr->m_Type !=4)
			{
				str = QString("%8.2f").arg(pXDirect->m_pCurOpp->Alpha);
			}
			else
			{
				str = QString("%8.0f").arg(pXDirect->m_pCurOpp->Reynolds);
			}
			pos = m_pctrlOpPoint->findText(str);

			if(pos>=0) m_pctrlOpPoint->setCurrentIndex(pos);
		}
		else
		{
			//select the first
			m_pctrlOpPoint->setCurrentIndex(0);
			str = m_pctrlOpPoint->itemText(0);
			QByteArray textline;
			const char *text;
			double x;
			textline = str.toAscii();
			text = textline.constData();
			int res = sscanf(text, "%lf", &x);

			if(res==1) pXDirect->SetOpp(x);
			else       pXDirect->SetOpp();

			if(!pXDirect->m_pCurOpp)
			{
				m_pctrlOpPoint->setEnabled(false);
				pXDirect->m_pCurOpp = NULL;
			}
		}
		if (m_pctrlOpPoint->count()) m_pctrlOpPoint->setEnabled(true);
		else                         m_pctrlOpPoint->setEnabled(false);
    }
    else
    {
		// otherwise disable control
		m_pctrlOpPoint->setEnabled(false);
		pXDirect->m_pCurOpp = NULL;
		m_pctrlOpPoint->clear();
    }
}



void MainFrame::updateRecentFileActions()
{
	int numRecentFiles = qMin(m_RecentFiles.size(), MAXRECENTFILES);

	QString text;
	for (int i = 0; i < numRecentFiles; ++i)
	{
		text = tr("&%1 %2").arg(i + 1).arg(ShortenFileName(m_RecentFiles[i]));
		recentFileActs[i]->setText(text);
		recentFileActs[i]->setData(m_RecentFiles[i]);
		recentFileActs[i]->setVisible(true);
	}
	for (int j = numRecentFiles; j < MAXRECENTFILES; ++j)
		recentFileActs[j]->setVisible(false);

	separatorAct->setVisible(numRecentFiles > 0);
}



void MainFrame::UpdateView()
{
	switch(m_iApp)
	{
		case XFOILANALYSIS:
			QXDirect *pXDirect = (QXDirect*)m_pXDirect;
			pXDirect->UpdateView();
			break;
	}
}


void MainFrame::WritePolars(QDataStream &ar, CFoil *pFoil)
{
	int i;
	if(!pFoil)
	{
		ar << 100002;
		//100001 : transferred NCrit, XTopTr, XBotTr to polar file
		//100002 : means we are serializings opps in the new numbered format
		//first write foils
		ar << (int)m_oaFoil.size();

		for (i=0; i<m_oaFoil.size(); i++)
		{
			pFoil = (CFoil*)m_oaFoil.at(i);
			pFoil->Serialize(ar, true);
		}

		//then write polars
		ar << (int)m_oaPolar.size();
		CPolar * pPolar ;
		for (i=0; i<m_oaPolar.size();i++)
		{
			pPolar = (CPolar*)m_oaPolar.at(i);
//			pPolar->m_pMainFrame = this;
			pPolar->Serialize(ar, true);
		}
		if(m_bSaveOpps)
		{
			ar << (int)m_oaOpp.size();
			OpPoint * pOpp ;
			for (i=0; i<m_oaOpp.size();i++)
			{
				pOpp = (OpPoint*)m_oaOpp.at(i);
//					pOpp->m_pParent = this;
				pOpp->Serialize(ar,true,100002);
			}
		}
		else ar << 0;
	}
	else
	{
		ar << 100002;
		//100001 : transferred NCrit, XTopTr, XBotTr to polar file
		//100002 : means we are serializings opps in the new numbered format
		//first write foil
		ar << 1;//only one this time
		pFoil->Serialize(ar,true);
		//count polars associated to the foil
		CPolar * pPolar ;
		int n=0;
		for (i=0; i<m_oaPolar.size();i++)
		{
			pPolar = (CPolar*)m_oaPolar.at(i);
			if (pPolar->m_FoilName == pFoil->m_FoilName) n++;
		}
		//then write polars
		ar << n;
		for (i=0; i<m_oaPolar.size();i++)
		{
			pPolar = (CPolar*)m_oaPolar.at(i);
//			pPolar->m_pFrame = this;
			if (pPolar->m_FoilName == pFoil->m_FoilName) pPolar->Serialize(ar, true);
		}
	}
}







