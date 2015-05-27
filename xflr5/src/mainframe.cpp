/****************************************************************************

	MainFrame  Class
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

#include "mainframe.h"
#include "globals.h"
#include "design/AFoil.h"
#include "miarex/Miarex.h"
#include "miarex/design/InertiaDlg.h"
#include "miarex/design/EditObjectDelegate.h"
#include "miarex/Objects3D.h"
#include "miarex/analysis/WPolarDlg.h"
#include "miarex/analysis/StabPolarDlg.h"
#include "miarex/analysis/PanelAnalysisDlg.h"
#include "miarex/design/BodyTransDlg.h"
#include "miarex/design/GL3dWingDlg.h"
#include "miarex/design/GL3dBodyDlg.h"
#include "miarex/view/GL3DScales.h"
#include "miarex/view/StabViewDlg.h"
#include "miarex/design/PlaneDlg.h"
#include "miarex/mgt/PlaneTableDelegate.h"
#include "misc/AboutQ5.h"
#include "misc/ObjectPropsDlg.h"
#include "misc/LinePickerDlg.h"
#include "misc/Settings.h"
#include "misc/SaveOptionsDlg.h"
#include "misc/TranslatorDlg.h"
#include "misc/RenameDlg.h"
#include "misc/Units.h"
#include "misc/W3dPrefsDlg.h"
#include "misc/EditPlrDlg.h"
#include "graph/GraphDlg.h"
#include "xdirect/XDirect.h"
#include "xdirect/analysis/BatchDlg.h"
#include "xdirect/analysis/BatchThreadDlg.h"
#include "xdirect/geometry/NacaFoilDlg.h"
#include "xdirect/geometry/InterpolateFoilsDlg.h"
#include "xdirect/geometry/CAddDlg.h"
#include "xdirect/geometry/TwoDPanelDlg.h"
#include "xdirect/geometry/FoilCoordDlg.h"
#include "xdirect/geometry/FoilGeomDlg.h"
#include "xdirect/geometry/TEGapDlg.h"
#include "xdirect/geometry/LEDlg.h"
#include "xdirect/ManageFoilsDlg.h"
#include "xdirect/analysis/XFoilAnalysisDlg.h"
#include "xdirect/analysis/FoilPolarDlg.h"
#include "xinverse/XInverse.h"
#include "objects/Polar.h"
#include "viewwidgets/foilwidget.h"

#include <QMessageBox>
#include <QtCore>
#include <QToolBar>
#include <QDockWidget>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QApplication>
#include <QDesktopServices>
#include <QFileDialog>
#include <QStyledItemDelegate>

#ifdef Q_OS_MAC
	#include <CoreFoundation/CoreFoundation.h>
#endif


QPointer<MainFrame> MainFrame::_self = 0L;

QString MainFrame::s_ProjectName = "";
QString MainFrame::s_LanguageFilePath = "";
QDir MainFrame::s_StylesheetDir;
QDir MainFrame::s_TranslationDir;


bool MainFrame::s_bTrace = false;
bool MainFrame::s_bSaved = true;
QFile *MainFrame::s_pTraceFile = NULL;

QLabel *MainFrame::m_pctrlProjectName = NULL;
QList <QColor> MainFrame::s_ColorList;



MainFrame::MainFrame(QWidget *parent, Qt::WindowFlags flags) : QMainWindow(parent, flags)
{
	setAttribute(Qt::WA_DeleteOnClose);

	if(s_bTrace)
	{
		QString FileName = QDir::homePath() + "/Trace.log";
		s_pTraceFile = new QFile(FileName);
		if (!s_pTraceFile->open(QIODevice::ReadWrite | QIODevice::Text)) s_bTrace = false;
	}

	setWindowTitle(VERSIONNAME);
	setWindowIcon(QIcon(":/images/xflr5_64.png"));

	Settings sets(this);//to initialize the static variables
	//"Qt does not support style hints on X11 since this information is not provided by the window system."
	Settings::s_TextFont.setStyleHint(QFont::TypeWriter, QFont::OpenGLCompatible);
	Settings::s_TextFont.setStyleStrategy(QFont::OpenGLCompatible);
	Settings::s_TextFont.setFamily(Settings::s_TextFont.defaultFamily());
	Settings::s_TextFont.setPointSize(9);

	Settings::s_TableFont.setStyleHint(QFont::TypeWriter);
	Settings::s_TableFont.setStyleStrategy(QFont::PreferDevice);
	Settings::s_TableFont.setFamily(Settings::s_TableFont.defaultFamily());
	Settings::s_TableFont.setPointSize(8);

	Settings::s_RefGraph.setTitleFont(Settings::s_TextFont);
	Settings::s_RefGraph.setLabelFont(Settings::s_TextFont);

//	Settings::s_StyleSheetName = "xflr5_style";
	Settings::s_StyleSheetName = "";

	if(!QGLFormat::hasOpenGL())
	{
		QMessageBox::warning(this, tr("Warning"), tr("Your system does not provide support for OpenGL.\nXFLR5 will not operate correctly."));
	}
	

	CreateDockWindows();

	m_ImageFormat = XFLR5::PNG;
	Settings::s_ExportFileType = XFLR5::TXT;

	m_bSaveOpps     = false;
	m_bSaveWOpps    = true;
	m_bSaveSettings = true;

	m_GraphExportFilter = "Comma Separated Values (*.csv)";


	QAFoil *pAFoil       = (QAFoil*)m_pAFoil;
	QXDirect *pXDirect   = (QXDirect*)m_pXDirect;
	QXInverse *pXInverse = (QXInverse*)m_pXInverse;
	QMiarex *pMiarex     = (QMiarex*)m_pMiarex;

#ifdef Q_OS_MAC
		QSettings settings(QSettings::NativeFormat,QSettings::UserScope,"sourceforge.net","xflr5");
#else
		QSettings settings(QSettings::IniFormat,QSettings::UserScope,"XFLR5");
#endif
	QString str;
	int kx=117, ky=57;
	bool bOK;
	settings.beginGroup("MainFrame");
	{
		str = settings.value("LanguageFilePath").toString();
		if(str.length()) s_LanguageFilePath = str;
		kx = settings.value("FrameGeometryx", 117).toInt(&bOK);
		ky = settings.value("FrameGeometryy",  57).toInt(&bOK);
	}
	settings.endGroup();

	XFoilAnalysisDlg::s_Position = QPoint(kx+31, ky+31);
	BatchThreadDlg::s_Position   = QPoint(kx+31, ky+31);
	BatchDlg::s_Position         = QPoint(kx+31, ky+31);
	LLTAnalysisDlg::s_Position   = QPoint(kx+31, ky+31);
	PanelAnalysisDlg::s_Position = QPoint(kx+31, ky+31);
	EditPlrDlg::s_Position       = QPoint(kx+31, ky+31);
	InertiaDlg::s_Position       = QPoint(kx+31, ky+31);


	if(s_LanguageFilePath.length())
	{
		qApp->removeTranslator(&m_Translator);
		if(m_Translator.load(s_LanguageFilePath))
		{
			qApp->installTranslator(&m_Translator);
		}
	}

	if(loadSettings())
	{
		Settings::loadSettings(&settings);

		pAFoil->loadSettings(&settings);
		pXDirect->loadSettings(&settings);
		pMiarex->loadSettings(&settings);
		pXInverse->loadSettings(&settings);

		GL3DScales::loadSettings(&settings);
		W3dPrefsDlg::loadSettings(&settings);
	}

	SetupDataDir();

	pXDirect->setAnalysisParams();
	CreateActions();
	CreateMenus();
	CreateToolbars();
	CreateStatusBar();

	m_pXDirectTileWidget->Connect();
	m_pMiarexTileWidget->Connect();

	s_ColorList.append(QColor(255,0,0));
	s_ColorList.append(QColor(0,255,0));
	s_ColorList.append(QColor(0,0,255));
	s_ColorList.append(QColor(255,255,0));
	s_ColorList.append(QColor(255,0,255));
	s_ColorList.append(QColor(0,255,255));

	s_bSaved     = true;
	m_bHighlightOpp = m_bHighlightWOpp = false;

	m_iApp = XFLR5::NOAPP;
	m_pctrlAFoilToolBar->hide();
	m_pctrlXDirectToolBar->hide();
	m_pctrlXInverseToolBar->hide();
	m_pctrlMiarexToolBar->hide();
	m_pctrlStabViewWidget->hide();

	setMenus();

	QString styleSheet;
	if(Settings::s_bStyleSheets)
	{
	   ReadStyleSheet(Settings::s_StyleSheetName, styleSheet);
	}

    Objects3D::setStaticPointers();
}


MainFrame::~MainFrame()
{
	Trace("Destroying mainframe");

	if(s_pTraceFile) s_pTraceFile->close();

	for(int ioa=Foil::s_oaFoil.size()-1; ioa>=0; ioa--)
	{
		delete (Foil*)Foil::s_oaFoil.at(ioa);
		Foil::s_oaFoil.removeAt(ioa);
	}

	for(int ioa=Polar::s_oaPolar.size()-1; ioa>=0; ioa--)
	{
		delete (Polar*)Polar::s_oaPolar.at(ioa);
		Polar::s_oaPolar.removeAt(ioa);
	}

	for(int ioa=OpPoint::s_oaOpp.size()-1; ioa>=0; ioa--)
	{
		delete (OpPoint*)OpPoint::s_oaOpp.at(ioa);
		OpPoint::s_oaOpp.removeAt(ioa);
	}
}


void MainFrame::AboutQt()
{
#ifndef QT_NO_MESSAGEBOX
    QMessageBox::aboutQt(
#ifdef Q_OS_MAC
    this
#else
//            activeWindow()
	this
#endif // Q_OS_MAC
            );
#endif // QT_NO_MESSAGEBOX
}



void MainFrame::aboutXFLR5()
{
	AboutQ5 dlg(this);
	dlg.exec();
}


void MainFrame::AddRecentFile(const QString &PathName)
{
	m_RecentFiles.removeAll(PathName);
	m_RecentFiles.prepend(PathName);
	while (m_RecentFiles.size() > MAXRECENTFILES)
		m_RecentFiles.removeLast();

	updateRecentFileActions();
}


void MainFrame::closeEvent (QCloseEvent * event)
{
	if(!s_bSaved)
	{
		int resp = QMessageBox::question(this, tr("Exit"), tr("Save the project before exit ?"),
										 QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel,
										 QMessageBox::Yes);
		if(resp == QMessageBox::Yes)
		{
			if(!SaveProject(m_FileName))
			{
				event->ignore();
				return;
			}
			AddRecentFile(m_FileName);
		}
		else if (resp==QMessageBox::Cancel)
		{
			event->ignore();
			return;
		}
	}
	deleteProject(true);

	SaveSettings();
	event->accept();//continue closing
}



void MainFrame::CreateActions()
{
	newProjectAct = new QAction(QIcon(":/images/new.png"), tr("New Project"), this);
	newProjectAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_N));
	newProjectAct->setStatusTip(tr("Save and close the current project, create a new project"));
	connect(newProjectAct, SIGNAL(triggered()), this, SLOT(OnNewProject()));

	closeProjectAct = new QAction(QIcon(":/images/new.png"), tr("Close the Project"), this);
	closeProjectAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_W));
	closeProjectAct->setStatusTip(tr("Save and close the current project"));
	connect(closeProjectAct, SIGNAL(triggered()), this, SLOT(OnNewProject()));

	openAct = new QAction(QIcon(":/images/open.png"), tr("&Open..."), this);
	openAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_O));
	openAct->setStatusTip(tr("Open an existing file"));
	connect(openAct, SIGNAL(triggered()), this, SLOT(OnLoadFile()));

	insertAct = new QAction(tr("&Insert Project..."), this);
	insertAct->setStatusTip(tr("Insert an existing project in the current project"));
	connect(insertAct, SIGNAL(triggered()), this, SLOT(OnInsertProject()));

	OnAFoilAct = new QAction(tr("&Direct Foil Design"), this);
	OnAFoilAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_1));
	OnAFoilAct->setStatusTip(tr("Open Foil Design application"));
	connect(OnAFoilAct, SIGNAL(triggered()), this, SLOT(OnAFoil()));

	OnXInverseAct = new QAction(tr("&XFoil Inverse Design"), this);
	OnXInverseAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_3));
	OnXInverseAct->setStatusTip(tr("Open XFoil inverse analysis application"));
	connect(OnXInverseAct, SIGNAL(triggered()), this, SLOT(OnXInverse()));

	OnMixedInverseAct = new QAction(tr("&XFoil Mixed Inverse Design"), this);
	OnMixedInverseAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_4));
	OnMixedInverseAct->setStatusTip(tr("Open XFoil Mixed Inverse analysis application"));
	connect(OnMixedInverseAct, SIGNAL(triggered()), this, SLOT(OnXInverseMixed()));

	OnXDirectAct = new QAction(tr("&XFoil Direct Analysis"), this);
	OnXDirectAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_5));
	OnXDirectAct->setStatusTip(tr("Open XFoil direct analysis application"));
	connect(OnXDirectAct, SIGNAL(triggered()), this, SLOT(onXDirect()));

	OnMiarexAct = new QAction(tr("&Wing and Plane Design"), this);
	OnMiarexAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_6));
	OnMiarexAct->setStatusTip(tr("Open Wing/plane design and analysis application"));
	connect(OnMiarexAct, SIGNAL(triggered()), this, SLOT(onMiarex()));

	saveAct = new QAction(QIcon(":/images/save.png"), tr("Save"), this);
	saveAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
	saveAct->setStatusTip(tr("Save the project to disk"));
	connect(saveAct, SIGNAL(triggered()), this, SLOT(onSaveProject()));

	saveProjectAsAct = new QAction(tr("Save Project As..."), this);
	saveProjectAsAct->setStatusTip(tr("Save the current project under a new name"));
	connect(saveProjectAsAct, SIGNAL(triggered()), this, SLOT(onSaveProjectAs()));

	saveOptionsAct = new QAction(tr("Save Options"), this);
	saveOptionsAct->setStatusTip(tr("Define the save options for operating points"));
	connect(saveOptionsAct, SIGNAL(triggered()), this, SLOT(onSaveOptions()));

	unitsAct = new QAction(tr("Units..."), this);
	unitsAct->setStatusTip(tr("Define the units for this project"));
	connect(unitsAct, SIGNAL(triggered()), this, SLOT(onUnits()));

	languageAct = new QAction(tr("Language..."), this);
	languageAct->setStatusTip(tr("Define the default language for the application"));
	connect(languageAct, SIGNAL(triggered()), this, SLOT(OnLanguage()));

	restoreToolbarsAct	 = new QAction(tr("Restore toolbars"), this);
	restoreToolbarsAct->setStatusTip(tr("Restores the toolbars to their original state"));
	connect(restoreToolbarsAct, SIGNAL(triggered()), this, SLOT(OnRestoreToolbars()));

	saveViewToImageFileAct = new QAction(tr("Save View to Image File"), this);
	saveViewToImageFileAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_I));
	saveViewToImageFileAct->setStatusTip(tr("Saves the current view to a file on disk"));
	connect(saveViewToImageFileAct, SIGNAL(triggered()), this, SLOT(onSaveViewToImageFile()));


	resetSettingsAct = new QAction(tr("Reset Default Settings"), this);
	resetSettingsAct->setStatusTip(tr("will revert to default settings at the next session"));
	connect(resetSettingsAct, SIGNAL(triggered()), this, SLOT(OnResetSettings()));


	for (int i = 0; i < MAXRECENTFILES; ++i)
	{
		recentFileActs[i] = new QAction(this);
		recentFileActs[i]->setVisible(false);
		connect(recentFileActs[i], SIGNAL(triggered()), this, SLOT(openRecentFile()));
	}

	m_pExportCurGraphAct = new QAction(tr("Export Graph"), this);
	m_pExportCurGraphAct->setStatusTip(tr("Export the current graph data to a text file"));
	connect(m_pExportCurGraphAct, SIGNAL(triggered()), this, SLOT(onExportCurGraph()));

	m_pResetCurGraphScales = new QAction(QIcon(":/images/OnResetGraphScale.png"), tr("Reset Graph Scales")+"\t(R)", this);
	m_pResetCurGraphScales->setStatusTip(tr("Restores the graph's x and y scales"));
	connect(m_pResetCurGraphScales, SIGNAL(triggered()), this, SLOT(onResetCurGraphScales()));

	m_pCurGraphDlgAct = new QAction(tr("Define Graph Settings")+"\t(G)", this);
	connect(m_pCurGraphDlgAct, SIGNAL(triggered()), this, SLOT(onCurGraphSettings()));

	styleAct = new QAction(tr("General Display Settings"), this);
	styleAct->setStatusTip(tr("Define the color and font options for all views and graphs"));
	connect(styleAct, SIGNAL(triggered()), this, SLOT(OnStyleSettings()));

	exitAct = new QAction(tr("E&xit"), this);
	exitAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));
	exitAct->setStatusTip(tr("Exit the application"));
	connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

	openGLAct = new QAction(tr("OpenGL info"), this);
	connect(openGLAct, SIGNAL(triggered()), this, SLOT(OnOpenGLInfo()));

	aboutAct = new QAction(tr("&About"), this);
	aboutAct->setStatusTip(tr("More information about XFLR5"));
	connect(aboutAct, SIGNAL(triggered()), this, SLOT(aboutXFLR5()));

	aboutQtAct = new QAction(tr("About Qt"), this);
	connect(aboutQtAct, SIGNAL(triggered()), this, SLOT(AboutQt()));

	createGraphActions();
	CreateAFoilActions();
	CreateXDirectActions();
	CreateXInverseActions();
	CreateMiarexActions();
}


void MainFrame::CreateAFoilActions()
{
	QAFoil *pAFoil = (QAFoil*)m_pAFoil;

	storeSplineAct= new QAction(QIcon(":/images/OnStoreFoil.png"), tr("Store Splines as Foil"), this);
	storeSplineAct->setStatusTip(tr("Store the current splines in the foil database"));
	connect(storeSplineAct, SIGNAL(triggered()), pAFoil, SLOT(OnStoreSplines()));

	splineControlsAct= new QAction(tr("Splines Params"), this);
	splineControlsAct->setStatusTip(tr("Define parameters for the splines : degree, number of out points"));
	connect(splineControlsAct, SIGNAL(triggered()), pAFoil, SLOT(OnSplineControls()));

	exportSplinesToFileAct= new QAction(tr("Export Splines To File"), this);
	exportSplinesToFileAct->setStatusTip(tr("Define parameters for the splines : degree, number of out points"));
	connect(exportSplinesToFileAct, SIGNAL(triggered()), pAFoil, SLOT(OnExportSplinesToFile()));

	newSplinesAct= new QAction(tr("New Splines"), this);
	newSplinesAct->setStatusTip(tr("Reset the splines"));
	connect(newSplinesAct, SIGNAL(triggered()), pAFoil, SLOT(OnNewSplines()));

	UndoAFoilAct= new QAction(QIcon(":/images/OnUndo.png"), tr("Undo"), this);
	UndoAFoilAct->setShortcut(Qt::CTRL + Qt::Key_Z);
	UndoAFoilAct->setStatusTip(tr("Cancels the last modification"));
	connect(UndoAFoilAct, SIGNAL(triggered()), pAFoil, SLOT(OnUndo()));

	RedoAFoilAct= new QAction(QIcon(":/images/OnRedo.png"), tr("Redo"), this);
	RedoAFoilAct->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_Z);
	RedoAFoilAct->setStatusTip(tr("Restores the last cancelled modification"));
	connect(RedoAFoilAct, SIGNAL(triggered()), pAFoil, SLOT(OnRedo()));

	ShowAllFoils= new QAction(tr("Show All Foils"), this);
	connect(ShowAllFoils, SIGNAL(triggered()), pAFoil, SLOT(OnShowAllFoils()));
	HideAllFoils= new QAction(tr("Hide All Foils"), this);
	connect(HideAllFoils, SIGNAL(triggered()), pAFoil, SLOT(OnHideAllFoils()));


	AFoilDelete = new QAction(tr("Delete..."), this);
	connect(AFoilDelete, SIGNAL(triggered()), pAFoil, SLOT(OnDeleteCurFoil()));

	AFoilRename = new QAction(tr("Rename..."), this);
	AFoilRename->setShortcut(Qt::Key_F2);
	connect(AFoilRename, SIGNAL(triggered()), pAFoil, SLOT(OnRenameFoil()));

	AFoilExport = new QAction(tr("Export..."), this);
	connect(AFoilExport, SIGNAL(triggered()), pAFoil, SLOT(OnExportCurFoil()));

	pAFoilDuplicateFoil = new QAction(tr("Duplicate..."), this);
	connect(pAFoilDuplicateFoil, SIGNAL(triggered()), pAFoil, SLOT(OnDuplicate()));

	ShowCurrentFoil= new QAction(tr("Show Current Foil"), this);
	connect(ShowCurrentFoil, SIGNAL(triggered()), pAFoil, SLOT(OnShowCurrentFoil()));

	HideCurrentFoil= new QAction(tr("Hide Current Foil"), this);
	connect(HideCurrentFoil, SIGNAL(triggered()), pAFoil, SLOT(OnHideCurrentFoil()));

	AFoilDerotateFoil = new QAction(tr("De-rotate the Foil"), this);
	connect(AFoilDerotateFoil, SIGNAL(triggered()), pAFoil, SLOT(OnAFoilDerotateFoil()));

	AFoilNormalizeFoil = new QAction(tr("Normalize the Foil"), this);
	connect(AFoilNormalizeFoil, SIGNAL(triggered()), pAFoil, SLOT(OnAFoilNormalizeFoil()));

	AFoilRefineGlobalFoil = new QAction(tr("Refine Globally")+"\t(F3)", this);
	connect(AFoilRefineGlobalFoil, SIGNAL(triggered()), pAFoil, SLOT(OnAFoilPanels()));

	AFoilRefineLocalFoil = new QAction(tr("Refine Locally")+"\t(Shift+F3)", this);
	connect(AFoilRefineLocalFoil, SIGNAL(triggered()), pAFoil, SLOT(OnAFoilCadd()));

	AFoilEditCoordsFoil = new QAction(tr("Edit Foil Coordinates"), this);
	connect(AFoilEditCoordsFoil, SIGNAL(triggered()), pAFoil, SLOT(OnAFoilFoilCoordinates()));

	AFoilScaleFoil = new QAction(tr("Scale camber and thickness")+"\t(F9)", this);
	connect(AFoilScaleFoil, SIGNAL(triggered()), pAFoil, SLOT(OnAFoilFoilGeom()));

	AFoilSetTEGap = new QAction(tr("Set T.E. Gap"), this);
	connect(AFoilSetTEGap, SIGNAL(triggered()), pAFoil, SLOT(OnAFoilSetTEGap()));

	AFoilSetLERadius = new QAction(tr("Set L.E. Radius"), this);
	connect(AFoilSetLERadius, SIGNAL(triggered()), pAFoil, SLOT(OnAFoilSetLERadius()));

	AFoilLECircle = new QAction(tr("Show LE Circle"), this);
	connect(AFoilLECircle, SIGNAL(triggered()), pAFoil, SLOT(OnAFoilLECircle()));

	m_pShowLegend = new QAction(tr("Show Legend"), this);
	m_pShowLegend->setCheckable(true);
	connect(m_pShowLegend, SIGNAL(triggered()), pAFoil, SLOT(OnShowLegend()));

	AFoilSetFlap = new QAction(tr("Set Flap")+"\t(F10)", this);
	connect(AFoilSetFlap, SIGNAL(triggered()), pAFoil, SLOT(OnAFoilSetFlap()));

	AFoilInterpolateFoils = new QAction(tr("Interpolate Foils")+"\t(F11)", this);
	connect(AFoilInterpolateFoils, SIGNAL(triggered()), pAFoil, SLOT(OnAFoilInterpolateFoils()));

	AFoilNacaFoils = new QAction(tr("Naca Foils"), this);
	connect(AFoilNacaFoils, SIGNAL(triggered()), pAFoil, SLOT(OnAFoilNacaFoils()));

	AFoilTableColumns = new QAction(tr("Set Table Columns"), this);
	connect(AFoilTableColumns, SIGNAL(triggered()), pAFoil, SLOT(OnAFoilTableColumns()));

	AFoilTableColumnWidths = new QAction(tr("Reset column widths"), this);
	connect(AFoilTableColumnWidths, SIGNAL(triggered()), pAFoil, SLOT(OnResetColumnWidths()));



	AFoilGridAct= new QAction(tr("Grid Settings"), this);
	AFoilGridAct->setStatusTip(tr("Define the grid settings for the view"));
	connect(AFoilGridAct, SIGNAL(triggered()), m_pDirect2dWidget, SLOT(onGridSettings()));

	InsertSplinePt = new QAction(tr("Insert Control Point")+"\tShift+Click", this);
	connect(InsertSplinePt, SIGNAL(triggered()), m_pDirect2dWidget, SLOT(onInsertPt()));

	RemoveSplinePt = new QAction(tr("Remove Control Point")+"\tCtrl+Click", this);
	connect(RemoveSplinePt, SIGNAL(triggered()), m_pDirect2dWidget, SLOT(onRemovePt()));

	ResetXScaleAct= new QAction(QIcon(":/images/OnResetXScale.png"), tr("Reset X Scale"), this);
	ResetXScaleAct->setStatusTip(tr("Resets the scale to fit the current screen width"));
	connect(ResetXScaleAct, SIGNAL(triggered()), m_pDirect2dWidget, SLOT(onResetXScale()));

	ResetXYScaleAct= new QAction(QIcon(":/images/OnResetFoilScale.png"), tr("Reset Scales")+"\t(R)", this);
	ResetXYScaleAct->setStatusTip(tr("Resets the x and y scales to screen size"));
	connect(ResetXYScaleAct, SIGNAL(triggered()), m_pDirect2dWidget, SLOT(onResetScales()));


	AFoilLoadImage = new QAction(tr("Load background image")   +"\tCtrl+Shift+I", this);
	connect(AFoilLoadImage, SIGNAL(triggered()), m_pDirect2dWidget, SLOT(onLoadBackImage()));
	AFoilClearImage = new QAction(tr("Clear background image") +"\tCtrl+Shift+I", this);

	connect(AFoilClearImage, SIGNAL(triggered()), m_pDirect2dWidget, SLOT(onClearBackImage()));


	ResetYScaleAct= new QAction(tr("Reset Y Scale"), this);
	connect(ResetYScaleAct, SIGNAL(triggered()), m_pDirect2dWidget, SLOT(onResetYScale()));

	zoomInAct= new QAction(QIcon(":/images/OnZoomIn.png"), tr("Zoom in"), this);
	zoomInAct->setStatusTip(tr("Zoom the view by drawing a rectangle in the client area"));
	connect(zoomInAct, SIGNAL(triggered()), m_pDirect2dWidget, SLOT(onZoomIn()));

	zoomLessAct= new QAction(QIcon(":/images/OnZoomLess.png"), tr("Zoom Less"), this);
	zoomLessAct->setStatusTip(tr("Zoom Less"));
	connect(zoomLessAct, SIGNAL(triggered()), m_pDirect2dWidget, SLOT(onZoomLess()));

	zoomYAct= new QAction(QIcon(":/images/OnZoomYScale.png"), tr("Zoom Y Scale Only"), this);
	zoomYAct->setStatusTip(tr("Zoom Y scale Only"));
	connect(zoomYAct, SIGNAL(triggered()), m_pDirect2dWidget, SLOT(onZoomYOnly()));
}

//
void MainFrame::CreateAFoilMenus()
{
	m_pAFoilViewMenu = menuBar()->addMenu(tr("&View"));
	{
		m_pAFoilViewMenu->addAction(ShowCurrentFoil);
		m_pAFoilViewMenu->addAction(HideCurrentFoil);
		m_pAFoilViewMenu->addAction(ShowAllFoils);
		m_pAFoilViewMenu->addAction(HideAllFoils);
		m_pAFoilViewMenu->addSeparator();
		m_pAFoilViewMenu->addAction(zoomInAct);
		m_pAFoilViewMenu->addAction(zoomLessAct);
		m_pAFoilViewMenu->addAction(ResetXScaleAct);
		m_pAFoilViewMenu->addAction(ResetYScaleAct);
		m_pAFoilViewMenu->addAction(ResetXYScaleAct);
		m_pAFoilViewMenu->addSeparator();
		m_pAFoilViewMenu->addAction(m_pShowLegend);
		m_pAFoilViewMenu->addAction(AFoilLECircle);
		m_pAFoilViewMenu->addAction(AFoilGridAct);
		m_pAFoilViewMenu->addSeparator();
		m_pAFoilViewMenu->addAction(AFoilLoadImage);
		m_pAFoilViewMenu->addAction(AFoilClearImage);
		m_pAFoilViewMenu->addSeparator();
		m_pAFoilViewMenu->addAction(saveViewToImageFileAct);
	}

	m_pAFoilDesignMenu = menuBar()->addMenu(tr("F&oil"));
	{
		m_pAFoilDesignMenu->addAction(AFoilRename);
		m_pAFoilDesignMenu->addAction(AFoilDelete);
		m_pAFoilDesignMenu->addAction(AFoilExport);
		m_pAFoilDesignMenu->addAction(pAFoilDuplicateFoil);
		m_pAFoilDesignMenu->addSeparator();
		m_pAFoilDesignMenu->addAction(HideAllFoils);
		m_pAFoilDesignMenu->addAction(ShowAllFoils);
		m_pAFoilDesignMenu->addSeparator();
		m_pAFoilDesignMenu->addAction(AFoilNormalizeFoil);
		m_pAFoilDesignMenu->addAction(AFoilDerotateFoil);
		m_pAFoilDesignMenu->addAction(AFoilRefineLocalFoil);
		m_pAFoilDesignMenu->addAction(AFoilRefineGlobalFoil);
		m_pAFoilDesignMenu->addAction(AFoilEditCoordsFoil);
		m_pAFoilDesignMenu->addAction(AFoilScaleFoil);
		m_pAFoilDesignMenu->addAction(AFoilSetTEGap);
		m_pAFoilDesignMenu->addAction(AFoilSetLERadius);
		m_pAFoilDesignMenu->addAction(AFoilSetFlap);
		m_pAFoilDesignMenu->addSeparator();
		m_pAFoilDesignMenu->addAction(AFoilInterpolateFoils);
		m_pAFoilDesignMenu->addAction(AFoilNacaFoils);
		m_pAFoilDesignMenu->addAction(ManageFoilsAct);
	}


	m_pAFoilSplineMenu = menuBar()->addMenu(tr("&Splines"));
	{
		m_pAFoilSplineMenu->addAction(InsertSplinePt);
		m_pAFoilSplineMenu->addAction(RemoveSplinePt);
		m_pAFoilSplineMenu->addSeparator();
		m_pAFoilSplineMenu->addAction(UndoAFoilAct);
		m_pAFoilSplineMenu->addAction(RedoAFoilAct);
		m_pAFoilSplineMenu->addSeparator();
		m_pAFoilSplineMenu->addAction(newSplinesAct);
		m_pAFoilSplineMenu->addAction(splineControlsAct);
		m_pAFoilSplineMenu->addAction(storeSplineAct);
		m_pAFoilSplineMenu->addAction(exportSplinesToFileAct);
	}

	//AFoil Context Menu
	m_pAFoilCtxMenu = new QMenu(tr("Context Menu"),this);
	{
		m_pAFoilCtxMenu->addMenu(m_pAFoilDesignMenu);
		m_pAFoilCtxMenu->addSeparator();
		m_pAFoilCtxMenu->addMenu(m_pAFoilSplineMenu);
		m_pAFoilCtxMenu->addSeparator();
		m_pAFoilCtxMenu->addAction(ShowAllFoils);
		m_pAFoilCtxMenu->addAction(HideAllFoils);
		m_pAFoilCtxMenu->addSeparator();
		m_pAFoilCtxMenu->addAction(ResetXScaleAct);
		m_pAFoilCtxMenu->addAction(ResetYScaleAct);
		m_pAFoilCtxMenu->addAction(ResetXYScaleAct);
		m_pAFoilCtxMenu->addSeparator();
		m_pAFoilCtxMenu->addAction(m_pShowLegend);
		m_pAFoilCtxMenu->addAction(AFoilLECircle);
		m_pAFoilCtxMenu->addAction(AFoilGridAct);
		m_pAFoilCtxMenu->addSeparator();
		m_pAFoilCtxMenu->addAction(AFoilLoadImage);
		m_pAFoilCtxMenu->addAction(AFoilClearImage);
		m_pAFoilCtxMenu->addSeparator();
		m_pAFoilCtxMenu->addAction(saveViewToImageFileAct);
		m_pAFoilCtxMenu->addSeparator();
		m_pAFoilCtxMenu->addAction(AFoilTableColumns);
		m_pAFoilCtxMenu->addAction(AFoilTableColumnWidths);
	}
	m_pDirect2dWidget->setContextMenu(m_pAFoilCtxMenu);

	//Context menu to be displayed when user right clicks on a foil in the table
	m_pAFoilTableCtxMenu = new QMenu(tr("Foil Actions"),this);
	{
		m_pAFoilTableCtxMenu->addAction(AFoilRename);
		m_pAFoilTableCtxMenu->addAction(AFoilDelete);
		m_pAFoilTableCtxMenu->addAction(AFoilExport);
		m_pAFoilTableCtxMenu->addAction(pAFoilDuplicateFoil);
		m_pAFoilTableCtxMenu->addSeparator();
		m_pAFoilTableCtxMenu->addAction(AFoilNormalizeFoil);
		m_pAFoilTableCtxMenu->addAction(AFoilDerotateFoil);
		m_pAFoilTableCtxMenu->addAction(AFoilRefineLocalFoil);
		m_pAFoilTableCtxMenu->addAction(AFoilRefineGlobalFoil);
		m_pAFoilTableCtxMenu->addAction(AFoilEditCoordsFoil);
		m_pAFoilTableCtxMenu->addAction(AFoilScaleFoil);
		m_pAFoilTableCtxMenu->addAction(AFoilSetTEGap);
		m_pAFoilTableCtxMenu->addAction(AFoilSetLERadius);
		m_pAFoilTableCtxMenu->addAction(AFoilSetFlap);
		m_pAFoilTableCtxMenu->addSeparator();
		m_pAFoilTableCtxMenu->addAction(AFoilTableColumns);
		m_pAFoilTableCtxMenu->addAction(AFoilTableColumnWidths);
	}
}


void MainFrame::CreateAFoilToolbar()
{
	m_pctrlAFoilToolBar = addToolBar(tr("Foil"));
	m_pctrlAFoilToolBar->addAction(newProjectAct);
	m_pctrlAFoilToolBar->addAction(openAct);
	m_pctrlAFoilToolBar->addAction(saveAct);	
	m_pctrlAFoilToolBar->addSeparator();
	m_pctrlAFoilToolBar->addAction(zoomInAct);
	m_pctrlAFoilToolBar->addAction(zoomLessAct);
	m_pctrlAFoilToolBar->addAction(ResetXYScaleAct);
	m_pctrlAFoilToolBar->addAction(ResetXScaleAct);
	m_pctrlAFoilToolBar->addAction(zoomYAct);
	m_pctrlAFoilToolBar->addSeparator();
	m_pctrlAFoilToolBar->addAction(UndoAFoilAct);
	m_pctrlAFoilToolBar->addAction(RedoAFoilAct);

	m_pctrlAFoilToolBar->addSeparator();
	m_pctrlAFoilToolBar->addAction(storeSplineAct);
}



void MainFrame::CreateDockWindows()
{
	QAFoil::s_pMainFrame           = this;
	QXDirect::s_pMainFrame         = this;
	QXInverse::s_pMainFrame        = this;
	QMiarex::s_pMainFrame          = this;
	ThreeDWidget::s_pMainFrame     = this;
	Body::s_pMainFrame             = this;
	Plane::s_pMainFrame            = this;
	Section2dWidget::s_pMainFrame  = this;
	GraphWidget::s_pMainFrame      = this;
	FoilWidget::s_pMainFrame       = this;
	WingWidget::s_pMainFrame       = this;
	QGraph::s_pMainFrame           = this;


	m_pctrlXDirectWidget = new QDockWidget(tr("Direct foil analysis"), this);
	m_pctrlXDirectWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::RightDockWidgetArea, m_pctrlXDirectWidget);

	m_pctrlXInverseWidget = new QDockWidget(tr("Inverse foil analysis"), this);
	m_pctrlXInverseWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::RightDockWidgetArea, m_pctrlXInverseWidget);

	m_pctrlMiarexWidget = new QDockWidget(tr("Plane analysis"), this);
	m_pctrlMiarexWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::RightDockWidgetArea, m_pctrlMiarexWidget);

	m_pctrlAFoilWidget = new QDockWidget(tr("Foil direct design"), this);
	m_pctrlAFoilWidget->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
	addDockWidget(Qt::BottomDockWidgetArea, m_pctrlAFoilWidget);

	m_p2dWidget = new TwoDWidget(this);
	m_p3dWidget = new ThreeDWidget(this);
	m_pDirect2dWidget = new Direct2dDesign(this);
	m_pXDirectTileWidget = new XDirectTileWidget(this);
	m_pMiarexTileWidget  = new MiarexTileWidget(this);

	m_pXDirect = new QXDirect(this);
	QXDirect *pXDirect = (QXDirect*)m_pXDirect;
	m_pctrlXDirectWidget->setWidget(pXDirect);
	m_pctrlXDirectWidget->setVisible(false);
	m_pctrlXDirectWidget->setFloating(true);
	m_pctrlXDirectWidget->move(960,60);

	m_pXInverse = new QXInverse(this);
	QXInverse *pXInverse = (QXInverse*)m_pXInverse;
	m_pctrlXInverseWidget->setWidget(pXInverse);
	m_pctrlXInverseWidget->setVisible(false);
	m_pctrlXInverseWidget->setFloating(true);
	m_pctrlXInverseWidget->move(960,60);

	m_pMiarex = new QMiarex;
	QMiarex * pMiarex = (QMiarex*)m_pMiarex;
	m_pctrlMiarexWidget->setWidget(pMiarex);
	m_pctrlMiarexWidget->setVisible(false);
	m_pctrlMiarexWidget->setFloating(true);
	m_pctrlMiarexWidget->move(960,60);


	m_pGL3DScales = new GL3DScales(this);
	GL3DScales * pGL3DScales = (GL3DScales*)m_pGL3DScales;
	GL3DScales::s_pMiarex      = m_pMiarex;
	m_pctrl3DScalesWidget = new QDockWidget(tr("3D Scales"), this);
	m_pctrl3DScalesWidget->setAllowedAreas(Qt::LeftDockWidgetArea);
	addDockWidget(Qt::LeftDockWidgetArea, m_pctrl3DScalesWidget);
	m_pctrl3DScalesWidget->setWidget(pGL3DScales);
	pGL3DScales->setParent(m_pctrl3DScalesWidget);
	m_pctrl3DScalesWidget->setVisible(false);
	m_pctrl3DScalesWidget->setFloating(true);
	m_pctrl3DScalesWidget->move(60,60);

	StabViewDlg::s_pMiarex = m_pMiarex;
	m_pStabView = new StabViewDlg(this);
	StabViewDlg * pStabView = (StabViewDlg*)m_pStabView;
	m_pctrlStabViewWidget = new QDockWidget(tr("Stability"), this);
	m_pctrlStabViewWidget->setAllowedAreas(Qt::LeftDockWidgetArea);
	addDockWidget(Qt::LeftDockWidgetArea, m_pctrlStabViewWidget);
	m_pctrlStabViewWidget->setWidget(pStabView);
	m_pctrlStabViewWidget->setVisible(false);
	m_pctrlStabViewWidget->setFloating(true);
	m_pctrlStabViewWidget->move(60,60);

	m_pctrlCentralWidget = new QStackedWidget;
	m_pctrlCentralWidget->addWidget(m_p2dWidget);
	m_pctrlCentralWidget->addWidget(m_p3dWidget);
	m_pctrlCentralWidget->addWidget(m_pDirect2dWidget);
	m_pctrlCentralWidget->addWidget(m_pXDirectTileWidget);
	m_pctrlCentralWidget->addWidget(m_pMiarexTileWidget);


	setCentralWidget(m_pctrlCentralWidget);

	m_pAFoil  = new QAFoil(this);
	QAFoil *pAFoil = (QAFoil*)m_pAFoil;
	pAFoil->m_p2DWidget = m_pDirect2dWidget;
	m_pctrlAFoilWidget->setWidget(pAFoil);
	m_pctrlAFoilWidget->setVisible(false);

	m_p2dWidget->m_pXInverse  = pXInverse;
	m_p2dWidget->m_pMainFrame = this;

	pMiarex->m_p3dWidget = m_p3dWidget;


	pXDirect->m_CpGraph.m_pParent    = m_p2dWidget;
	pXDirect->m_poaFoil  = &Foil::s_oaFoil;
	pXDirect->m_poaPolar = &Polar::s_oaPolar;
	pXDirect->m_poaOpp   = &OpPoint::s_oaOpp;

	pAFoil->initDialog(m_pDirect2dWidget, &Foil::s_oaFoil, pXDirect->m_pXFoil);

	QXInverse::s_p2DWidget        = m_p2dWidget;
	pXInverse->s_pMainFrame       = this;
	pXInverse->m_pXFoil           = pXDirect->m_pXFoil;
	pXInverse->m_poaFoil          = &Foil::s_oaFoil;

	GL3dWingDlg::s_poaFoil = &Foil::s_oaFoil;
	EditObjectDelegate::s_poaFoil = &Foil::s_oaFoil;

	LLTAnalysis::s_poaPolar = &Polar::s_oaPolar;

	ThreeDWidget::s_pMiarex       = m_pMiarex;
	WingWidget::s_pMiarex         = m_pMiarex;
	XFoilAnalysisDlg::s_pXDirect  = m_pXDirect;
	NacaFoilDlg::s_pXFoil         = pXDirect->m_pXFoil;
	InterpolateFoilsDlg::s_pXFoil = pXDirect->m_pXFoil;
	CAddDlg::s_pXFoil             = pXDirect->m_pXFoil;
	TwoDPanelDlg::s_pXFoil        = pXDirect->m_pXFoil;
	FoilGeomDlg::s_pXFoil         = pXDirect->m_pXFoil;
	TEGapDlg::s_pXFoil            = pXDirect->m_pXFoil;
	LEDlg::s_pXFoil               = pXDirect->m_pXFoil;
	BatchDlg::s_pXDirect          = m_pXDirect;
	BatchThreadDlg::s_pXDirect    = m_pXDirect;

	GraphTileWidget::s_pMainFrame = this;
	GraphTileWidget::s_pMiarex    = m_pMiarex;
	GraphTileWidget::s_pXDirect   = m_pXDirect;

	LegendWidget::s_pMainFrame = this;
	LegendWidget::s_pMiarex    = m_pMiarex;
	LegendWidget::s_pXDirect   = m_pXDirect;

	pMiarex->Connect();
}



void MainFrame::CreateMenus()
{
	m_pFileMenu = menuBar()->addMenu(tr("&File"));
	{
		m_pFileMenu->addAction(newProjectAct);
		m_pFileMenu->addAction(openAct);
		m_pFileMenu->addAction(insertAct);
		m_pFileMenu->addAction(closeProjectAct);
		m_pFileMenu->addSeparator();
		m_pFileMenu->addAction(saveAct);
		m_pFileMenu->addAction(saveProjectAsAct);
		m_pFileMenu->addSeparator();
		m_pFileMenu->addAction(OnAFoilAct);
		m_pFileMenu->addAction(OnXInverseAct);
		m_pFileMenu->addAction(OnXDirectAct);
		m_pFileMenu->addAction(OnMiarexAct);
		separatorAct = m_pFileMenu->addSeparator();
		for (int i = 0; i < MAXRECENTFILES; ++i)
			m_pFileMenu->addAction(recentFileActs[i]);
		m_pFileMenu->addSeparator();
		m_pFileMenu->addAction(exitAct);
		updateRecentFileActions();
	}

	m_pOptionsMenu = menuBar()->addMenu(tr("O&ptions"));
	{
		m_pOptionsMenu->addSeparator();
		m_pOptionsMenu->addAction(languageAct);
		m_pOptionsMenu->addSeparator();
		m_pOptionsMenu->addAction(unitsAct);
		m_pOptionsMenu->addSeparator();
		m_pOptionsMenu->addAction(styleAct);
		m_pOptionsMenu->addSeparator();
		m_pOptionsMenu->addAction(saveOptionsAct);
		m_pOptionsMenu->addSeparator();
		m_pOptionsMenu->addAction(restoreToolbarsAct);
		m_pOptionsMenu->addSeparator();
		m_pOptionsMenu->addAction(resetSettingsAct);
	}

	m_pGraphMenu = menuBar()->addMenu(tr("&Graphs"));
	{
		for(int ig=0; ig<MAXGRAPHS; ig++)
			m_pGraphMenu->addAction(m_pSingleGraph[ig]);
		m_pGraphMenu->addSeparator();
		m_pGraphMenu->addAction(m_pTwoGraphs);
		m_pGraphMenu->addAction(m_pFourGraphs);
		m_pGraphMenu->addAction(m_pAllGraphs);
		m_pGraphMenu->addSeparator();
		m_pGraphMenu->addAction(m_pAllGraphsSettings);
		m_pGraphMenu->addAction(m_pAllGraphsScalesAct);
		m_pGraphMenu->addSeparator();
		m_pGraphMenu->addAction(m_pHighlightWOppAct);
	}

	m_pHelpMenu = menuBar()->addMenu(tr("&?"));
	{
		m_pHelpMenu->addAction(openGLAct);
		m_pHelpMenu->addAction(aboutQtAct);
		m_pHelpMenu->addAction(aboutAct);
	}



	//Create Application-Specific Menus
	CreateXDirectMenus();
	CreateXInverseMenus();
	CreateMiarexMenus();
	CreateAFoilMenus();
}
 


void MainFrame::createGraphActions()
{
	QMiarex *pMiarex = (QMiarex*)m_pMiarex;

	for (int ig=0; ig<MAXGRAPHS; ++ig)
	{
		m_pSingleGraph[ig] = new QAction(tr("Graph")+QString(" %1\t(%2)").arg(ig+1).arg(ig+1), this);
		m_pSingleGraph[ig]->setData(ig);
		m_pSingleGraph[ig]->setCheckable(true);
		connect(m_pSingleGraph[ig], SIGNAL(triggered()), m_pXDirectTileWidget, SLOT(onSingleGraph()));
		connect(m_pSingleGraph[ig], SIGNAL(triggered()), m_pMiarexTileWidget, SLOT(onSingleGraph()));
	}
	m_pTwoGraphs = new QAction(tr("Two Graphs")+"\t(T)", this);
	m_pTwoGraphs->setStatusTip(tr("Display the first two graphs"));
	m_pTwoGraphs->setCheckable(true);
	connect(m_pTwoGraphs, SIGNAL(triggered()), m_pXDirectTileWidget, SLOT(onTwoGraphs()));
	connect(m_pTwoGraphs, SIGNAL(triggered()), m_pMiarexTileWidget, SLOT(onTwoGraphs()));

	m_pFourGraphs = new QAction(tr("Four Graphs")+"\t(F)", this);
	m_pFourGraphs->setStatusTip(tr("Display four graphs"));
	m_pFourGraphs->setCheckable(true);
	connect(m_pFourGraphs, SIGNAL(triggered()), m_pXDirectTileWidget, SLOT(onFourGraphs()));
	connect(m_pFourGraphs, SIGNAL(triggered()), m_pMiarexTileWidget, SLOT(onFourGraphs()));

	m_pAllGraphs = new QAction(tr("All Graphs")+"\t(A)", this);
	m_pAllGraphs->setStatusTip(tr("Display four graphs"));
	m_pAllGraphs->setCheckable(true);
	connect(m_pAllGraphs, SIGNAL(triggered()), m_pXDirectTileWidget, SLOT(onAllGraphs()));
	connect(m_pAllGraphs, SIGNAL(triggered()), m_pMiarexTileWidget, SLOT(onAllGraphs()));

	m_pGraphDlgAct = new QAction(tr("Define Graph Settings")+"\t(G)", this);
	m_pGraphDlgAct->setStatusTip(tr("Define the settings for the selected graph"));
	connect(m_pGraphDlgAct, SIGNAL(triggered()), m_pXDirectTileWidget, SLOT(onCurGraphSettings()));
	connect(m_pGraphDlgAct, SIGNAL(triggered()), m_pMiarexTileWidget, SLOT(onCurGraphSettings()));

	m_pAllGraphsScalesAct = new QAction(tr("Reset All Graph Scales"), this);
	m_pAllGraphsScalesAct->setStatusTip(tr("Reset the scales of all four polar graphs"));
	connect(m_pAllGraphsScalesAct, SIGNAL(triggered()), m_pXDirectTileWidget, SLOT(onAllGraphScales()));
	connect(m_pAllGraphsScalesAct, SIGNAL(triggered()), m_pMiarexTileWidget, SLOT(onAllGraphScales()));

	m_pAllGraphsSettings = new QAction(tr("All Graph Settings"), this);
	m_pAllGraphsSettings->setStatusTip(tr("Define the settings of all graphs"));
	connect(m_pAllGraphsSettings, SIGNAL(triggered()), m_pXDirectTileWidget, SLOT(onAllGraphSettings()));
	connect(m_pAllGraphsSettings, SIGNAL(triggered()), m_pMiarexTileWidget, SLOT(onAllGraphSettings()));

	m_pHighlightWOppAct	 = new QAction(tr("Highlight Current OpPoint")+"\t(Ctrl+H)", this);
	m_pHighlightWOppAct->setCheckable(true);
	m_pHighlightWOppAct->setStatusTip(tr("Highlights on the polar curve the currently selected operating point"));
	connect(m_pHighlightWOppAct, SIGNAL(triggered()), pMiarex, SLOT(OnHighlightWOpp()));
}


void MainFrame::CreateMiarexActions()
{
	QMiarex *pMiarex = (QMiarex*)m_pMiarex;

	WOppAct = new QAction(QIcon(":/images/OnWOppView.png"), tr("OpPoint View")+"\tF5", this);
	WOppAct->setCheckable(true);
	WOppAct->setStatusTip(tr("Switch to the Operating point view"));
//	WOppAct->setShortcut(Qt::Key_F5);
	connect(WOppAct, SIGNAL(triggered()), pMiarex, SLOT(onWOppView()));

	WPolarAct = new QAction(QIcon(":/images/OnPolarView.png"), tr("Polar View")+"\tF8", this);
	WPolarAct->setCheckable(true);
	WPolarAct->setStatusTip(tr("Switch to the Polar view"));
//	WPolarAct->setShortcut(Qt::Key_F8);
	connect(WPolarAct, SIGNAL(triggered()), pMiarex, SLOT(onWPolarView()));

	StabTimeAct = new QAction(QIcon(":/images/OnStabView.png"),tr("Time Response Vew")+"\tShift+F8", this);
	StabTimeAct->setCheckable(true);
	StabTimeAct->setStatusTip(tr("Switch to stability analysis post-processing"));
//	StabTimeAct->setShortcut(tr("Shift+F8"));
	connect(StabTimeAct, SIGNAL(triggered()), pMiarex, SLOT(onStabTimeView()));

	RootLocusAct = new QAction(QIcon(":/images/OnRootLocus.png"),tr("Root Locus View")+"\tCtrl+F8", this);
	RootLocusAct->setCheckable(true);
	RootLocusAct->setStatusTip(tr("Switch to root locus view"));
	connect(RootLocusAct, SIGNAL(triggered()), pMiarex, SLOT(onRootLocusView()));

	W3DAct = new QAction(QIcon(":/images/On3DView.png"), tr("3D View")+"\tF4", this);
	W3DAct->setCheckable(true);
	W3DAct->setStatusTip(tr("Switch to the 3D view"));
	connect(W3DAct, SIGNAL(triggered()), pMiarex, SLOT(on3DView()));

	CpViewAct = new QAction(QIcon(":/images/OnCpView.png"), tr("Cp View")+"\tF9", this);
	CpViewAct->setCheckable(true);
	CpViewAct->setStatusTip(tr("Switch to the Cp view"));
	connect(CpViewAct, SIGNAL(triggered()), pMiarex, SLOT(OnCpView()));

	W3DPrefsAct = new QAction(tr("3D Color Preferences"), this);
	W3DPrefsAct->setStatusTip(tr("Define the style and color preferences for the 3D view"));
	connect(W3DPrefsAct, SIGNAL(triggered()), pMiarex, SLOT(On3DPrefs()));

	MiarexPolarFilter = new QAction(tr("Polar Filter"), this);
	MiarexPolarFilter->setStatusTip(tr("Define which type of polars should be shown or hidden"));
	connect(MiarexPolarFilter, SIGNAL(triggered()), pMiarex, SLOT(OnPolarFilter()));

	W3DScalesAct = new QAction(tr("3D Scales"), this);
	W3DScalesAct->setStatusTip(tr("Define the scales for the 3D display of lift, moment, drag, and downwash"));
	W3DScalesAct->setCheckable(true);
	connect(W3DScalesAct, SIGNAL(triggered()), pMiarex, SLOT(OnGL3DScale()));

	W3DLightAct = new QAction(tr("3D Light Options"), this);
	W3DLightAct->setStatusTip(tr("Define the light options in 3D view"));
	connect(W3DLightAct, SIGNAL(triggered()), pMiarex, SLOT(OnSetupLight()));

	definePlaneAct = new QAction(tr("Define a New Plane")+"\tF3", this);
	definePlaneAct->setStatusTip(tr("Shows a dialogbox to create a new plane definition"));
	connect(definePlaneAct, SIGNAL(triggered()), pMiarex, SLOT(OnNewPlane()));

	definePlaneObjectAct = new QAction(tr("Define... (Advanced users)")+"\tF3", this);
	definePlaneObjectAct->setStatusTip(tr("Shows a dialogbox to create a new plane definition"));
	connect(definePlaneObjectAct, SIGNAL(triggered()), pMiarex, SLOT(OnNewPlaneObject()));

	editPlaneAct = new QAction(tr("Edit...")+"\tShift+F3", this);
	editPlaneAct->setStatusTip(tr("Shows a form to edit the currently selected plane"));
	connect(editPlaneAct, SIGNAL(triggered()), pMiarex, SLOT(OnEditCurPlane()));

	editObjectAct = new QAction(tr("Edit... (advanced users)")+"\tCtrl+F3", this);
	editObjectAct->setStatusTip(tr("Shows a form to edit the currently selected plane"));
	connect(editObjectAct, SIGNAL(triggered()), pMiarex, SLOT(OnEditCurObject()));

	exporttoXML = new QAction(tr("Export to xml file"), this);
	connect(exporttoXML, SIGNAL(triggered()), pMiarex, SLOT(onExportPlanetoXML()));

	importXMLPlaneAct = new QAction(tr("Import plane from xml file"), this);
	connect(importXMLPlaneAct, SIGNAL(triggered()), pMiarex, SLOT(onImportPlanefromXML()));

	editWingAct = new QAction(tr("Edit wing..."), this);
	editWingAct->setStatusTip(tr("Shows a form to edit the wing of the currently selected plane"));
	editWingAct->setShortcut(QKeySequence(Qt::Key_F10));
	connect(editWingAct, SIGNAL(triggered()), pMiarex, SLOT(OnEditCurWing()));

	m_pEditBodyAct = new QAction(tr("Edit body..."), this);
	m_pEditBodyAct->setStatusTip(tr("Shows a form to edit the body of the currently selected plane"));
	m_pEditBodyAct->setShortcut(QKeySequence(Qt::Key_F11));
	connect(m_pEditBodyAct, SIGNAL(triggered()), pMiarex, SLOT(onEditCurBody()));

	m_pEditBodyObjectAct= new QAction(tr("Edit body (advanced users"), this);
	m_pEditBodyObjectAct->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_F11));
	connect(m_pEditBodyObjectAct, SIGNAL(triggered()), pMiarex, SLOT(onEditCurBodyObject()));

	renameCurPlaneAct = new QAction(tr("Rename...")+"\tF2", this);
	renameCurPlaneAct->setStatusTip(tr("Rename the currently selected object"));
	connect(renameCurPlaneAct, SIGNAL(triggered()), pMiarex, SLOT(OnRenameCurPlane()));

	exporttoAVL = new QAction(tr("Export to AVL..."), this);
	exporttoAVL->setStatusTip(tr("Export the current plane or wing to a text file in the format required by AVL"));
	connect(exporttoAVL, SIGNAL(triggered()), pMiarex, SLOT(OnExporttoAVL()));

	exportCurWOpp = new QAction(tr("Export..."), this);
	exportCurWOpp->setStatusTip(tr("Export the current operating point to a text or csv file"));
	connect(exportCurWOpp, SIGNAL(triggered()), pMiarex, SLOT(OnExportCurWOpp()));


	scaleWingAct = new QAction(tr("Scale Wing"), this);
	scaleWingAct->setStatusTip(tr("Scale the dimensions of the currently selected wing"));
	connect(scaleWingAct, SIGNAL(triggered()), pMiarex, SLOT(OnScaleWing()));

	managePlanesAct = new QAction(tr("Manage objects"), this);
	managePlanesAct->setStatusTip(tr("Rename or delete the planes and wings stored in the database"));
	managePlanesAct->setShortcut(Qt::Key_F7);
	connect(managePlanesAct, SIGNAL(triggered()), pMiarex, SLOT(OnManagePlanes()));

	m_pImportWPolar = new QAction(tr("Import Polar"), this);
	m_pImportWPolar->setStatusTip(tr("Import a polar from a text file"));
	connect(m_pImportWPolar, SIGNAL(triggered()), pMiarex, SLOT(OnImportWPolar()));

	m_pPlaneInertia = new QAction(tr("Define Inertia")+"\tF12", this);
	m_pPlaneInertia->setStatusTip(tr("Define the inertia for the current plane or wing"));
	connect(m_pPlaneInertia, SIGNAL(triggered()), pMiarex, SLOT(OnPlaneInertia()));

	showCurWOppOnly = new QAction(tr("Show Current OpPoint Only"), this);
	showCurWOppOnly->setStatusTip(tr("Hide all the curves except for the one corresponding to the currently selected operating point"));
	showCurWOppOnly->setCheckable(true);
	connect(showCurWOppOnly, SIGNAL(triggered()), pMiarex, SLOT(OnCurWOppOnly()));

	showAllWOpps = new QAction(tr("Show All OpPoints"), this);
	showAllWOpps->setStatusTip(tr("Show the graph curves of all operating points"));
	connect(showAllWOpps, SIGNAL(triggered()), pMiarex, SLOT(OnShowAllWOpps()));

	hideAllWOpps = new QAction(tr("Hide All OpPoints"), this);
	hideAllWOpps->setStatusTip(tr("Hide the graph curves of all operating points"));
	connect(hideAllWOpps, SIGNAL(triggered()), pMiarex, SLOT(OnHideAllWOpps()));

	deleteAllWOpps = new QAction(tr("Delete All OpPoints"), this);
	deleteAllWOpps->setStatusTip(tr("Delete all the operating points of all planes and polars"));
	connect(deleteAllWOpps, SIGNAL(triggered()), pMiarex, SLOT(OnDeleteAllWOpps()));

	showAllWPlrOpps = new QAction(tr("Show Associated OpPoints"), this);
	showAllWPlrOpps->setStatusTip(tr("Show the curves of all the operating points of the currently selected polar"));
	connect(showAllWPlrOpps, SIGNAL(triggered()), pMiarex, SLOT(OnShowAllWPlrOpps()));
	
	hideAllWPlrOpps = new QAction(tr("Hide Associated OpPoints"), this);
	hideAllWPlrOpps->setStatusTip(tr("Hide the curves of all the operating points of the currently selected polar"));
	connect(hideAllWPlrOpps, SIGNAL(triggered()), pMiarex, SLOT(OnHideAllWPlrOpps()));
	
	deleteAllWPlrOpps = new QAction(tr("Delete Associated OpPoints"), this);
	deleteAllWPlrOpps->setStatusTip(tr("Delete all the operating points of the currently selected polar"));
	connect(deleteAllWPlrOpps, SIGNAL(triggered()), pMiarex, SLOT(OnDeleteAllWPlrOpps()));

	showEllipticCurve = new QAction(tr("Show Elliptic Curve"), this);
	showEllipticCurve->setStatusTip(tr("Show the theoretical optimal elliptic lift curve on all graphs for which the selected variable is the local lift"));
	showEllipticCurve->setCheckable(true);
	connect(showEllipticCurve, SIGNAL(triggered()), pMiarex, SLOT(onShowEllipticCurve()));

	showXCmRefLocation = new QAction(tr("Show XCG location"), this);
	showXCmRefLocation->setStatusTip(tr("Show the position of the center of gravity defined in the analysis"));
	showXCmRefLocation->setCheckable(true);
	connect(showXCmRefLocation, SIGNAL(triggered()), pMiarex, SLOT(onShowXCmRef()));

	showStabCurve = new QAction(tr("Show Elevator Curve"), this);
	showStabCurve->setStatusTip(tr("Show the graph curves for the elevator"));
	showStabCurve->setCheckable(true);
	connect(showStabCurve, SIGNAL(triggered()), pMiarex, SLOT(OnStabCurve()));

	showFinCurve = new QAction(tr("Show Fin Curve"), this);
	showFinCurve->setStatusTip(tr("Show the graph curves for the fin"));
	showFinCurve->setCheckable(true);
	connect(showFinCurve, SIGNAL(triggered()), pMiarex, SLOT(OnFinCurve()));

	showWing2Curve = new QAction(tr("Show Second Wing Curve"), this);
	showWing2Curve->setStatusTip(tr("Show the graph curves for the second wing"));
	showWing2Curve->setCheckable(true);
	connect(showWing2Curve, SIGNAL(triggered()), pMiarex, SLOT(OnWing2Curve()));

	defineWPolar = new QAction(tr("Define an Analysis")+" \t(F6)", this);
	defineWPolar->setStatusTip(tr("Define an analysis for the current wing or plane"));
	connect(defineWPolar, SIGNAL(triggered()), pMiarex, SLOT(onDefineWPolar()));


	defineWPolarObjectAct = new QAction(tr("Define a polar object (advanced users)"), this);
	defineWPolarObjectAct->setStatusTip(tr("Shows a form to edit a new polar object"));
	connect(defineWPolarObjectAct, SIGNAL(triggered()), pMiarex, SLOT(OnDefineWPolarObject()));

	editWPolarAct = new QAction(tr("Edit...")+" \t(Ctrl+F6)", this);
	editWPolarAct->setStatusTip(tr("Modify the analysis parameters of this polar"));
	connect(editWPolarAct, SIGNAL(triggered()), pMiarex, SLOT(onEditCurWPolar()));


	editWPolarObjectAct = new QAction(tr("Edit object (advanced users)"), this);
	editWPolarObjectAct->setStatusTip(tr("Shows a form to edit the currently selected polar"));
	connect(editWPolarObjectAct, SIGNAL(triggered()), pMiarex, SLOT(onEditCurWPolarObject()));

	editWPolarPts = new QAction(tr("Edit data points..."), this);
	editWPolarPts->setStatusTip(tr("Modify the data points of this polar"));
	connect(editWPolarPts, SIGNAL(triggered()), pMiarex, SLOT(OnEditCurWPolarPts()));

	defineStabPolar = new QAction(tr("Define a Stability Analysis")+"\t(Shift+F6)", this);
	defineStabPolar->setStatusTip(tr("Define a stability analysis for the current wing or plane"));
//	defineStabPolar->setShortcut(tr("Ctrl+F6"));
	connect(defineStabPolar, SIGNAL(triggered()), pMiarex, SLOT(onDefineStabPolar()));

	hidePlaneWPlrs = new QAction(tr("Hide Associated Polars"), this);
	hidePlaneWPlrs->setStatusTip(tr("Hide all the polar curves associated to the currently selected wing or plane"));
	connect(hidePlaneWPlrs, SIGNAL(triggered()), pMiarex, SLOT(OnHidePlaneWPolars()));
	
	showPlaneWPlrsOnly = new QAction(tr("Show Only Associated Polars"), this);
	connect(showPlaneWPlrsOnly, SIGNAL(triggered()), pMiarex, SLOT(OnShowPlaneWPolarsOnly()));

	showPlaneWPlrs = new QAction(tr("Show Associated Polars"), this);
	showPlaneWPlrs->setStatusTip(tr("Show all the polar curves associated to the currently selected wing or plane"));
	connect(showPlaneWPlrs, SIGNAL(triggered()), pMiarex, SLOT(OnShowPlaneWPolars()));

	deletePlaneWPlrs = new QAction(tr("Delete Associated Polars"), this);
	deletePlaneWPlrs->setStatusTip(tr("Delete all the polars associated to the currently selected wing or plane"));
	connect(deletePlaneWPlrs, SIGNAL(triggered()), pMiarex, SLOT(OnDeletePlaneWPolars()));

	hideAllWPlrs = new QAction(tr("Hide All Polars"), this);
	hideAllWPlrs->setStatusTip(tr("Hide all the polar curves of all wings and planes"));
	connect(hideAllWPlrs, SIGNAL(triggered()), pMiarex, SLOT(OnHideAllWPolars()));
	
	showAllWPlrs = new QAction(tr("Show All Polars"), this);
	showAllWPlrs->setStatusTip(tr("Show all the polar curves of all wings and planes"));
	connect(showAllWPlrs, SIGNAL(triggered()), pMiarex, SLOT(OnShowAllWPolars()));

	hidePlaneWOpps = new QAction(tr("Hide Associated OpPoints"), this);
	hidePlaneWOpps->setStatusTip(tr("Hide all the operating point curves of the currently selected wing or plane"));
	connect(hidePlaneWOpps, SIGNAL(triggered()), pMiarex, SLOT(OnHidePlaneOpps()));
	
	showPlaneWOpps = new QAction(tr("Show Associated OpPoints"), this);
	showPlaneWOpps->setStatusTip(tr("Show all the operating point curves of the currently selected wing or plane"));
	connect(showPlaneWOpps, SIGNAL(triggered()), pMiarex, SLOT(OnShowPlaneOpps()));
	
	deletePlaneWOpps = new QAction(tr("Delete Associated OpPoints"), this);
	deletePlaneWOpps->setStatusTip(tr("Delete all the operating points of the currently selected wing or plane"));
	connect(deletePlaneWOpps, SIGNAL(triggered()), pMiarex, SLOT(OnDeletePlaneOpps()));

	deleteCurPlane = new QAction(tr("Delete..."), this);
	deleteCurPlane->setStatusTip(tr("Delete the currently selected wing or plane"));
	connect(deleteCurPlane, SIGNAL(triggered()), pMiarex, SLOT(OnDeleteCurPlane()));

	duplicateCurPlane = new QAction(tr("Duplicate..."), this);
	duplicateCurPlane->setStatusTip(tr("Duplicate the currently selected wing or plane"));
	connect(duplicateCurPlane, SIGNAL(triggered()), pMiarex, SLOT(OnDuplicateCurPlane()));

	savePlaneAsProjectAct = new QAction(tr("Save as Project..."), this);
	savePlaneAsProjectAct->setStatusTip(tr("Save the currently selected wing or plane as a new separate project"));
	connect(savePlaneAsProjectAct, SIGNAL(triggered()), this, SLOT(OnSavePlaneAsProject()));

	renameCurWPolar = new QAction(tr("Rename..."), this);
	renameCurWPolar->setStatusTip(tr("Rename the currently selected polar"));
	connect(renameCurWPolar, SIGNAL(triggered()), pMiarex, SLOT(OnRenameCurWPolar()));

	exportCurWPolar = new QAction(tr("Export ..."), this);
	exportCurWPolar->setStatusTip(tr("Export the currently selected polar to a text or csv file"));
	connect(exportCurWPolar, SIGNAL(triggered()), pMiarex, SLOT(OnExportCurWPolar()));

	resetCurWPolar = new QAction(tr("Reset ..."), this);
	resetCurWPolar->setStatusTip(tr("Delete all the points of the currently selected polar, but keep the analysis settings"));
	connect(resetCurWPolar, SIGNAL(triggered()), pMiarex, SLOT(OnResetCurWPolar()));

	deleteCurWPolar = new QAction(tr("Delete ..."), this);
	deleteCurWPolar->setStatusTip(tr("Delete the currently selected polar"));
	connect(deleteCurWPolar, SIGNAL(triggered()), pMiarex, SLOT(OnDeleteCurWPolar()));

	deleteCurWOpp = new QAction(tr("Delete..."), this);
	deleteCurWOpp->setStatusTip(tr("Delete the currently selected operating point"));
	connect(deleteCurWOpp, SIGNAL(triggered()), pMiarex, SLOT(OnDeleteCurWOpp()));

	advancedSettings = new QAction(tr("Advanced Settings..."), this);
	advancedSettings->setStatusTip(tr("Define the settings for LLT, VLM and Panel analysis"));
	connect(advancedSettings, SIGNAL(triggered()), pMiarex, SLOT(OnAdvancedSettings()));

	ShowPolarProps = new QAction(tr("Properties"), this);
	ShowPolarProps->setStatusTip(tr("Show the properties of the currently selected polar"));
	ShowPolarProps->setShortcut(QKeySequence(Qt::ALT + Qt::Key_Return));
	connect(ShowPolarProps, SIGNAL(triggered()), pMiarex, SLOT(OnWPolarProperties()));

	ShowWOppProps = new QAction(tr("Properties"), this);
	ShowWOppProps->setStatusTip(tr("Show the properties of the currently selected operating point"));
	ShowWOppProps->setShortcut(QKeySequence(Qt::ALT + Qt::SHIFT + Qt::Key_Return));
	connect(ShowWOppProps, SIGNAL(triggered()), pMiarex, SLOT(OnPlaneOppProperties()));
}


void MainFrame::CreateMiarexMenus()
{
	//MainMenu for Miarex Application
	m_pMiarexViewMenu = menuBar()->addMenu(tr("&View"));
	{
		m_pMiarexViewMenu->addAction(WOppAct);
		m_pMiarexViewMenu->addAction(WPolarAct);
		m_pMiarexViewMenu->addAction(W3DAct);
		m_pMiarexViewMenu->addAction(CpViewAct);
		m_pMiarexViewMenu->addAction(StabTimeAct);
		m_pMiarexViewMenu->addAction(RootLocusAct);
		m_pMiarexViewMenu->addSeparator();
		m_pMiarexViewMenu->addAction(W3DPrefsAct);
		m_pMiarexViewMenu->addAction(W3DLightAct);
		m_pMiarexViewMenu->addAction(W3DScalesAct);
		m_pMiarexViewMenu->addSeparator();
		m_pMiarexViewMenu->addAction(saveViewToImageFileAct);
	}


	m_pPlaneMenu = menuBar()->addMenu(tr("&Plane"));
	{
		m_pPlaneMenu->addAction(definePlaneAct);
		m_pPlaneMenu->addAction(definePlaneObjectAct);
		m_pPlaneMenu->addAction(managePlanesAct);
		m_pCurrentPlaneMenu = m_pPlaneMenu->addMenu(tr("Current Plane"));
		{
			m_pCurrentPlaneMenu->addAction(editPlaneAct);
			m_pCurrentPlaneMenu->addAction(editObjectAct);
			m_pCurrentPlaneMenu->addAction(editWingAct);
			m_pCurrentPlaneMenu->addAction(m_pEditBodyAct);
			m_pCurrentPlaneMenu->addAction(m_pEditBodyObjectAct);
			m_pCurrentPlaneMenu->addSeparator();
			m_pCurrentPlaneMenu->addAction(renameCurPlaneAct);
			m_pCurrentPlaneMenu->addAction(duplicateCurPlane);
			m_pCurrentPlaneMenu->addAction(deleteCurPlane);
			m_pCurrentPlaneMenu->addAction(savePlaneAsProjectAct);
			m_pCurrentPlaneMenu->addSeparator();
			m_pCurrentPlaneMenu->addAction(scaleWingAct);
			m_pCurrentPlaneMenu->addSeparator();
			m_pCurrentPlaneMenu->addAction(m_pPlaneInertia);
			m_pCurrentPlaneMenu->addSeparator();
			m_pCurrentPlaneMenu->addAction(exporttoAVL);
			m_pCurrentPlaneMenu->addAction(exporttoXML);
			m_pCurrentPlaneMenu->addSeparator();
			m_pCurrentPlaneMenu->addAction(showPlaneWPlrsOnly);
			m_pCurrentPlaneMenu->addAction(showPlaneWPlrs);
			m_pCurrentPlaneMenu->addAction(hidePlaneWPlrs);
			m_pCurrentPlaneMenu->addAction(deletePlaneWPlrs);
			m_pCurrentPlaneMenu->addSeparator();
			m_pCurrentPlaneMenu->addAction(hidePlaneWOpps);
			m_pCurrentPlaneMenu->addAction(showPlaneWOpps);
			m_pCurrentPlaneMenu->addAction(deletePlaneWOpps);
		}
		m_pPlaneMenu->addAction(importXMLPlaneAct);
	}

	m_pMiarexWPlrMenu = menuBar()->addMenu(tr("&Polars"));
	{
		m_pCurWPlrMenu = m_pMiarexWPlrMenu->addMenu(tr("Current Polar"));
		{
			m_pCurWPlrMenu->addAction(ShowPolarProps);
			m_pCurWPlrMenu->addAction(editWPolarAct);
			m_pCurWPlrMenu->addAction(editWPolarObjectAct);
			m_pCurWPlrMenu->addAction(editWPolarPts);
			m_pCurWPlrMenu->addAction(renameCurWPolar);
			m_pCurWPlrMenu->addAction(deleteCurWPolar);
			m_pCurWPlrMenu->addAction(exportCurWPolar);
			m_pCurWPlrMenu->addAction(resetCurWPolar);
			m_pCurWPlrMenu->addSeparator();
			m_pCurWPlrMenu->addAction(showAllWPlrOpps);
			m_pCurWPlrMenu->addAction(hideAllWPlrOpps);
			m_pCurWPlrMenu->addAction(deleteAllWPlrOpps);
		}

		m_pMiarexWPlrMenu->addSeparator();
		m_pMiarexWPlrMenu->addAction(m_pImportWPolar);
		m_pMiarexWPlrMenu->addSeparator();
		m_pMiarexWPlrMenu->addAction(MiarexPolarFilter);
		m_pMiarexWPlrMenu->addSeparator();
		m_pMiarexWPlrMenu->addAction(hideAllWPlrs);
		m_pMiarexWPlrMenu->addAction(showAllWPlrs);
		m_pMiarexWPlrMenu->addSeparator();
	}

	m_pMiarexWOppMenu = menuBar()->addMenu(tr("&OpPoint"));
	{
		m_pCurWOppMenu = m_pMiarexWOppMenu->addMenu(tr("Current OpPoint"));
		{
			m_pCurWOppMenu->addAction(ShowWOppProps);
			m_pCurWOppMenu->addAction(exportCurWOpp);
			m_pCurWOppMenu->addAction(deleteCurWOpp);
		}
		m_pMiarexWOppMenu->addSeparator();
		m_pMiarexWOppMenu->addAction(showCurWOppOnly);
		m_pMiarexWOppMenu->addAction(showAllWOpps);
		m_pMiarexWOppMenu->addAction(hideAllWOpps);
		m_pMiarexWOppMenu->addAction(deleteAllWOpps);
		m_pMiarexWOppMenu->addSeparator();
		m_pMiarexWOppMenu->addAction(showEllipticCurve);
		m_pMiarexWOppMenu->addAction(showXCmRefLocation);
		m_pMiarexWOppMenu->addAction(showWing2Curve);
		m_pMiarexWOppMenu->addAction(showStabCurve);
		m_pMiarexWOppMenu->addAction(showFinCurve);
	}

	//Miarex Analysis Menu
	m_pMiarexAnalysisMenu  = menuBar()->addMenu(tr("&Analysis"));
	{
		m_pMiarexAnalysisMenu->addAction(defineWPolar);
		m_pMiarexAnalysisMenu->addAction(defineWPolarObjectAct);
		m_pMiarexAnalysisMenu->addAction(defineStabPolar);
		m_pMiarexAnalysisMenu->addSeparator();
		m_pMiarexAnalysisMenu->addAction(viewLogFile);
		m_pMiarexAnalysisMenu->addAction(advancedSettings);
	}


	//WOpp View Context Menu
	m_pWOppCtxMenu = new QMenu(tr("Context Menu"),this);
	{
		m_pWOppCtxMenu->addMenu(m_pCurrentPlaneMenu);
		m_pWOppCtxMenu->addSeparator();
		m_pWOppCtxMenu->addMenu(m_pCurWPlrMenu);
		m_pWOppCtxMenu->addSeparator();
		m_pWOppCtxMenu->addMenu(m_pCurWOppMenu);
		m_pWOppCtxMenu->addSeparator();
		m_pWOppCtxMenu->addAction(showCurWOppOnly);
		m_pWOppCtxMenu->addAction(showAllWOpps);
		m_pWOppCtxMenu->addAction(hideAllWOpps);
		m_pWOppCtxMenu->addAction(deleteAllWOpps);
		m_pWOppCtxMenu->addSeparator();
		m_pWOppCtxMenu->addAction(showEllipticCurve);
		m_pWOppCtxMenu->addAction(showXCmRefLocation);
		m_pWOppCtxMenu->addAction(showWing2Curve);
		m_pWOppCtxMenu->addAction(showStabCurve);
		m_pWOppCtxMenu->addAction(showFinCurve);
		m_pWOppCtxMenu->addSeparator();
		m_pWOppCtxMenu->addAction(viewLogFile);
		m_pWOppCtxMenu->addAction(saveViewToImageFileAct);
	}

	//WOpp View Context Menu
	m_pWCpCtxMenu = new QMenu(tr("Context Menu"),this);
	{
		m_pWCpCtxMenu->addMenu(m_pCurrentPlaneMenu);
		m_pWCpCtxMenu->addSeparator();
		m_pWCpCtxMenu->addMenu(m_pCurWPlrMenu);
		m_pWCpCtxMenu->addSeparator();
		m_pWCpCtxMenu->addMenu(m_pCurWOppMenu);
		m_pWCpCtxMenu->addSeparator();
		m_pWCpCtxMenu->addAction(showWing2Curve);
		m_pWCpCtxMenu->addAction(showStabCurve);
		m_pWCpCtxMenu->addAction(showFinCurve);
		m_pWCpCtxMenu->addSeparator();
		m_pWCpCtxMenu->addAction(viewLogFile);
		m_pWCpCtxMenu->addAction(saveViewToImageFileAct);
	}

	//WTime View Context Menu
	m_pWTimeCtxMenu = new QMenu(tr("Context Menu"),this);
	{
		m_pWTimeCtxMenu->addMenu(m_pCurrentPlaneMenu);
		m_pWTimeCtxMenu->addSeparator();
		m_pWTimeCtxMenu->addMenu(m_pCurWPlrMenu);
		m_pWTimeCtxMenu->addSeparator();
		m_pWTimeCtxMenu->addMenu(m_pCurWOppMenu);
		m_pWTimeCtxMenu->addSeparator();
		m_pWTimeCtxMenu->addAction(showCurWOppOnly);
		m_pWTimeCtxMenu->addAction(showAllWOpps);
		m_pWTimeCtxMenu->addAction(hideAllWOpps);
		m_pWTimeCtxMenu->addAction(deleteAllWOpps);
		m_pWTimeCtxMenu->addSeparator();
		m_pWTimeCtxMenu->addAction(viewLogFile);
		m_pWTimeCtxMenu->addAction(saveViewToImageFileAct);
	}

	//Polar View Context Menu
	m_pWPlrCtxMenu = new QMenu(tr("Context Menu"),this);
	{
		m_pWPlrCtxMenu->addMenu(m_pCurrentPlaneMenu);
		m_pWPlrCtxMenu->addSeparator();
		m_pWPlrCtxMenu->addMenu(m_pCurWPlrMenu);
		m_pWPlrCtxMenu->addSeparator();
		m_pWPlrCtxMenu->addAction(hideAllWPlrs);
		m_pWPlrCtxMenu->addAction(showAllWPlrs);
		m_pWPlrCtxMenu->addSeparator();
		m_pWPlrCtxMenu->addAction(viewLogFile);
		m_pWPlrCtxMenu->addAction(saveViewToImageFileAct);
	}

	//W3D View Context Menu
	m_pW3DCtxMenu = new QMenu(tr("Context Menu"),this);
	{
		m_pW3DCtxMenu->addMenu(m_pCurrentPlaneMenu);
		m_pW3DCtxMenu->addSeparator();
		m_pW3DCtxMenu->addMenu(m_pCurWPlrMenu);
		m_pW3DCtxMenu->addSeparator();
		m_pW3DCtxMenu->addMenu(m_pCurWOppMenu);
		m_pW3DCtxMenu->addSeparator();
		m_pW3DCtxMenu->addAction(deleteAllWOpps);
		m_pW3DCtxMenu->addSeparator();
		m_pW3DCtxMenu->addAction(W3DScalesAct);
		m_pW3DCtxMenu->addAction(W3DLightAct);
		m_pW3DCtxMenu->addSeparator();
		m_pW3DCtxMenu->addAction(viewLogFile);
		m_pW3DCtxMenu->addAction(saveViewToImageFileAct);
	}

	//W3D Stab View Context Menu
	m_pW3DStabCtxMenu = new QMenu(tr("Context Menu"),this);
	{
		m_pW3DStabCtxMenu->addMenu(m_pCurrentPlaneMenu);
		m_pW3DStabCtxMenu->addSeparator();
		m_pW3DStabCtxMenu->addMenu(m_pCurWPlrMenu);
		m_pW3DStabCtxMenu->addSeparator();
		m_pW3DStabCtxMenu->addMenu(m_pCurWOppMenu);
		m_pW3DStabCtxMenu->addSeparator();
		m_pW3DStabCtxMenu->addAction(W3DLightAct);
		m_pW3DStabCtxMenu->addSeparator();
		m_pW3DStabCtxMenu->addAction(viewLogFile);
		m_pW3DStabCtxMenu->addAction(saveViewToImageFileAct);
	}
}


void MainFrame::CreateMiarexToolbar()
{
	m_pctrlPlane  = new QComboBox();
	QStyledItemDelegate *pPlaneDelegate = new QStyledItemDelegate(this);
	m_pctrlPlane->setItemDelegate(pPlaneDelegate); //necessary to support stylesheets
	m_pctrlPlanePolar = new QComboBox;
	QStyledItemDelegate *pPPolarDelegate = new QStyledItemDelegate(this);
	m_pctrlPlanePolar->setItemDelegate(pPPolarDelegate); //necessary to support stylesheets
	m_pctrlPlaneOpp   = new QComboBox;
	QStyledItemDelegate *pPOppDelegate = new QStyledItemDelegate(this);
	m_pctrlPlaneOpp->setItemDelegate(pPOppDelegate); //necessary to support stylesheets

	m_pctrlPlane->setFocusPolicy(Qt::NoFocus);// to override keypressevent and key_F4
	m_pctrlPlanePolar->setFocusPolicy(Qt::NoFocus);
	m_pctrlPlaneOpp->setFocusPolicy(Qt::NoFocus);


	m_pctrlPlane->setMinimumWidth(150);
	m_pctrlPlanePolar->setMinimumWidth(150);
	m_pctrlPlaneOpp->setMinimumWidth(80);
	m_pctrlPlane->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	m_pctrlPlanePolar->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	m_pctrlPlaneOpp->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	m_pctrlPlane->setMaxVisibleItems(23);
	m_pctrlPlanePolar->setMaxVisibleItems(23);
	m_pctrlPlaneOpp->setMaxVisibleItems(23);

	m_pctrlMiarexToolBar = addToolBar(tr("Plane"));
	m_pctrlMiarexToolBar->addAction(newProjectAct);
	m_pctrlMiarexToolBar->addAction(openAct);
	m_pctrlMiarexToolBar->addAction(saveAct);
	m_pctrlMiarexToolBar->addSeparator();
	m_pctrlMiarexToolBar->addAction(WOppAct);
	m_pctrlMiarexToolBar->addAction(WPolarAct);
	m_pctrlMiarexToolBar->addAction(W3DAct);
	m_pctrlMiarexToolBar->addAction(CpViewAct);
	m_pctrlMiarexToolBar->addAction(RootLocusAct);
	m_pctrlMiarexToolBar->addAction(StabTimeAct);

	m_pctrlMiarexToolBar->addSeparator();
	m_pctrlMiarexToolBar->addWidget(m_pctrlPlane);
	m_pctrlMiarexToolBar->addWidget(m_pctrlPlanePolar);
	m_pctrlMiarexToolBar->addWidget(m_pctrlPlaneOpp);

	connect(m_pctrlPlane,      SIGNAL(activated(int)), this, SLOT(OnSelChangePlane(int)));
	connect(m_pctrlPlanePolar, SIGNAL(activated(int)), this, SLOT(OnSelChangeWPolar(int)));
	connect(m_pctrlPlaneOpp,   SIGNAL(activated(int)), this, SLOT(OnSelChangePlaneOpp(int)));
}


void MainFrame::CreateStatusBar()
{
	statusBar()->showMessage(tr("Ready"));
	m_pctrlProjectName = new QLabel(" ");
	m_pctrlProjectName->setMinimumWidth(200);
	statusBar()->addPermanentWidget(m_pctrlProjectName);
}


void MainFrame::CreateToolbars()
{
	CreateXDirectToolbar();
	CreateXInverseToolbar();
	CreateMiarexToolbar();
	CreateAFoilToolbar();
}


void MainFrame::CreateXDirectToolbar()
{
	m_pctrlFoil    = new QComboBox;
	m_pctrlPolar   = new QComboBox;
	m_pctrlOpPoint = new QComboBox;

	QStyledItemDelegate *pFoilDelegate = new QStyledItemDelegate(this);
	m_pctrlFoil->setItemDelegate(pFoilDelegate); //necessary to support stylesheets

	QStyledItemDelegate *pPolarDelegate = new QStyledItemDelegate(this);
	m_pctrlPolar->setItemDelegate(pPolarDelegate); //necessary to support stylesheets

	QStyledItemDelegate *pOppDelegate = new QStyledItemDelegate(this);
	m_pctrlOpPoint->setItemDelegate(pOppDelegate); //necessary to support stylesheets

	m_pctrlFoil->setMaxVisibleItems(23);
	m_pctrlPolar->setMaxVisibleItems(23);
	m_pctrlOpPoint->setMaxVisibleItems(23);
	m_pctrlFoil->setMinimumWidth(150);
	m_pctrlPolar->setMinimumWidth(150);
	m_pctrlOpPoint->setMinimumWidth(80);
	m_pctrlFoil->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	m_pctrlPolar->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	m_pctrlOpPoint->setSizeAdjustPolicy(QComboBox::AdjustToContents);

	m_pctrlXDirectToolBar = addToolBar(tr("Foil"));
	m_pctrlXDirectToolBar->addAction(newProjectAct);
	m_pctrlXDirectToolBar->addAction(openAct);
	m_pctrlXDirectToolBar->addAction(saveAct);
	m_pctrlXDirectToolBar->addSeparator();
	m_pctrlXDirectToolBar->addAction(OpPointsAct);
	m_pctrlXDirectToolBar->addAction(PolarsAct);
	m_pctrlXDirectToolBar->addSeparator();
	m_pctrlXDirectToolBar->addWidget(m_pctrlFoil);
	m_pctrlXDirectToolBar->addWidget(m_pctrlPolar);
	m_pctrlXDirectToolBar->addWidget(m_pctrlOpPoint);

	connect(m_pctrlFoil,    SIGNAL(activated(int)), this, SLOT(OnSelChangeFoil(int)));
	connect(m_pctrlPolar,   SIGNAL(activated(int)), this, SLOT(OnSelChangePolar(int)));
	connect(m_pctrlOpPoint, SIGNAL(activated(int)), this, SLOT(OnSelChangeOpp(int)));
}




void MainFrame::CreateXDirectActions()
{
	QXDirect *pXDirect = (QXDirect*)m_pXDirect;

	OpPointsAct = new QAction(QIcon(":/images/OnCpView.png"), tr("OpPoint view")+"\tF5", this);
	OpPointsAct->setCheckable(true);
	OpPointsAct->setStatusTip(tr("Show Operating point view"));
	connect(OpPointsAct, SIGNAL(triggered()), pXDirect, SLOT(onOpPointView()));

	PolarsAct = new QAction(QIcon(":/images/OnPolarView.png"), tr("Polar view")+"\tF8", this);
	PolarsAct->setCheckable(true);
	PolarsAct->setStatusTip(tr("Show Polar view"));
	connect(PolarsAct, SIGNAL(triggered()), pXDirect, SLOT(onPolarView()));

	XDirectPolarFilter = new QAction(tr("Polar Filter"), this);
	connect(XDirectPolarFilter, SIGNAL(triggered()), pXDirect, SLOT(OnPolarFilter()));

	highlightOppAct	 = new QAction(tr("Highlight Current OpPoint")+"\t(Ctrl+H)", this);
	highlightOppAct->setCheckable(true);
	highlightOppAct->setStatusTip(tr("Highlights on the polar curve the currently selected operating point"));
	connect(highlightOppAct, SIGNAL(triggered()), pXDirect, SLOT(OnHighlightOpp()));


	deleteCurFoil = new QAction(tr("Delete..."), this);
	connect(deleteCurFoil, SIGNAL(triggered()), pXDirect, SLOT(OnDeleteCurFoil()));

	renameCurFoil = new QAction(tr("Rename...")+"\tF2", this);
	connect(renameCurFoil, SIGNAL(triggered()), pXDirect, SLOT(OnRenameCurFoil()));

	exportCurFoil = new QAction(tr("Export..."), this);
	connect(exportCurFoil, SIGNAL(triggered()), pXDirect, SLOT(OnExportCurFoil()));

	pDirectDuplicateCurFoil = new QAction(tr("Duplicate..."), this);
	connect(pDirectDuplicateCurFoil, SIGNAL(triggered()), pXDirect, SLOT(OnDuplicateFoil()));

	setCurFoilStyle = new QAction(tr("Set Style..."), this);
	connect(setCurFoilStyle, SIGNAL(triggered()), this, SLOT(OnCurFoilStyle()));

	deleteFoilPolars = new QAction(tr("Delete associated polars"), this);
	deleteFoilPolars->setStatusTip(tr("Delete all the polars associated to this foil"));
	connect(deleteFoilPolars, SIGNAL(triggered()), pXDirect, SLOT(onDeleteFoilPolars()));

	showFoilPolarsOnly = new QAction(tr("Show only associated polars"), this);
	connect(showFoilPolarsOnly, SIGNAL(triggered()), pXDirect, SLOT(OnShowFoilPolarsOnly()));

	showFoilPolars = new QAction(tr("Show associated polars"), this);
	connect(showFoilPolars, SIGNAL(triggered()), pXDirect, SLOT(OnShowFoilPolars()));

	hideFoilPolars = new QAction(tr("Hide associated polars"), this);
	connect(hideFoilPolars, SIGNAL(triggered()), pXDirect, SLOT(OnHideFoilPolars()));

	saveFoilPolars = new QAction(tr("Save associated polars"), this);
	connect(saveFoilPolars, SIGNAL(triggered()), pXDirect, SLOT(OnSavePolars()));

	hidePolarOpps = new QAction(tr("Hide associated OpPoints"), this);
	connect(hidePolarOpps, SIGNAL(triggered()), pXDirect, SLOT(OnHidePolarOpps()));

	showPolarOpps = new QAction(tr("Show associated OpPoints"), this);
	connect(showPolarOpps, SIGNAL(triggered()), pXDirect, SLOT(OnShowPolarOpps()));

	deletePolarOpps = new QAction(tr("Delete associated OpPoints"), this);
	connect(deletePolarOpps, SIGNAL(triggered()), pXDirect, SLOT(onDeletePolarOpps()));

	exportPolarOpps = new QAction(tr("Export associated OpPoints"), this);
	connect(exportPolarOpps, SIGNAL(triggered()), pXDirect, SLOT(OnExportPolarOpps()));

	hideFoilOpps = new QAction(tr("Hide associated OpPoints"), this);
	connect(hideFoilOpps, SIGNAL(triggered()), pXDirect, SLOT(OnHideFoilOpps()));

	showFoilOpps = new QAction(tr("Show associated OpPoints"), this);
	connect(showFoilOpps, SIGNAL(triggered()), pXDirect, SLOT(OnShowFoilOpps()));

	deleteFoilOpps = new QAction(tr("Delete associated OpPoints"), this);
	connect(deleteFoilOpps, SIGNAL(triggered()), pXDirect, SLOT(onDeleteFoilOpps()));

	m_pDefinePolarAct = new QAction(tr("Define an Analysis")+"\tF6", this);
	m_pDefinePolarAct->setStatusTip(tr("Defines a single analysis/polar"));
	connect(m_pDefinePolarAct, SIGNAL(triggered()), pXDirect, SLOT(OnDefinePolar()));

	m_pBatchAnalysisAct = new QAction(tr("Batch Analysis")+"\tShift+F6", this);
	m_pBatchAnalysisAct->setStatusTip(tr("Launches a batch of analysis calculation for a specified range or list of Reynolds numbers"));
	connect(m_pBatchAnalysisAct, SIGNAL(triggered()), pXDirect, SLOT(OnBatchAnalysis()));

	MultiThreadedBatchAct = new QAction(tr("Multi-threaded Batch Analysis")+"\tCtrl+F6", this);
	MultiThreadedBatchAct->setStatusTip(tr("Launches a batch of analysis calculation using all available computer CPU cores"));
	connect(MultiThreadedBatchAct, SIGNAL(triggered()), pXDirect, SLOT(OnMultiThreadedBatchAnalysis()));

	deletePolar = new QAction(tr("Delete"), this);
	deletePolar->setStatusTip(tr("Deletes the currently selected polar"));
	connect(deletePolar, SIGNAL(triggered()), pXDirect, SLOT(onDeleteCurPolar()));

	resetCurPolar = new QAction(tr("Reset"), this);
	resetCurPolar->setStatusTip(tr("Deletes the contents of the currently selected polar"));
	connect(resetCurPolar, SIGNAL(triggered()), pXDirect, SLOT(OnResetCurPolar()));

	editCurPolar = new QAction(tr("Edit"), this);
	editCurPolar->setStatusTip(tr("Remove the unconverged or erroneaous points of the currently selected polar"));
	connect(editCurPolar, SIGNAL(triggered()), pXDirect, SLOT(OnEditCurPolar()));

	exportCurPolar = new QAction(tr("Export"), this);
	connect(exportCurPolar, SIGNAL(triggered()), pXDirect, SLOT(OnExportCurPolar()));

	exportAllPolars = new QAction(tr("Export all polars"), this);
	connect(exportAllPolars, SIGNAL(triggered()), pXDirect, SLOT(OnExportAllPolars()));

	m_pXDirectStyleAct = new QAction(tr("Define Styles"), this);
	m_pXDirectStyleAct->setStatusTip(tr("Define the style for the boundary layer and the pressure arrows"));

	m_pShowNeutralLine = new QAction(tr("Neutral Line"), this);
	m_pShowNeutralLine->setCheckable(true);

	m_pShowPanels = new QAction(tr("Show Panels"), this);
	m_pShowPanels->setCheckable(true);
	m_pShowPanels->setStatusTip(tr("Show the foil's panels"));

	m_pResetFoilScale = new QAction(tr("Reset Foil Scale"), this);
	m_pResetFoilScale->setStatusTip(tr("Resets the foil's scale to original size"));

	ManageFoilsAct = new QAction(tr("Manage Foils"), this);
	ManageFoilsAct->setShortcut(Qt::Key_F7);
	connect(ManageFoilsAct, SIGNAL(triggered()), this, SLOT(onManageFoils()));

	RenamePolarAct = new QAction(tr("Rename"), this);
	connect(RenamePolarAct, SIGNAL(triggered()), pXDirect, SLOT(OnRenamePolar()));


	showInviscidCurve = new QAction(tr("Show Inviscid Curve"), this);
	showInviscidCurve->setCheckable(true);
	showInviscidCurve->setStatusTip(tr("Display the Opp's inviscid curve"));
	connect(showInviscidCurve, SIGNAL(triggered()), pXDirect, SLOT(OnCpi()));


	showAllPolars = new QAction(tr("Show All Polars"), this);
	connect(showAllPolars, SIGNAL(triggered()), pXDirect, SLOT(OnShowAllPolars()));

	hideAllPolars = new QAction(tr("Hide All Polars"), this);
	connect(hideAllPolars, SIGNAL(triggered()), pXDirect, SLOT(OnHideAllPolars()));

	showCurOppOnly = new QAction(tr("Show Current Opp Only"), this);
	showCurOppOnly->setCheckable(true);
	connect(showCurOppOnly, SIGNAL(triggered()), pXDirect, SLOT(OnCurOppOnly()));

	showAllOpPoints = new QAction(tr("Show All Opps"), this);
	connect(showAllOpPoints, SIGNAL(triggered()), pXDirect, SLOT(OnShowAllOpps()));

	hideAllOpPoints = new QAction(tr("Hide All Opps"), this);
	connect(hideAllOpPoints, SIGNAL(triggered()), pXDirect, SLOT(OnHideAllOpps()));

	exportCurOpp = new QAction(tr("Export"), this);
	connect(exportCurOpp, SIGNAL(triggered()), pXDirect, SLOT(OnExportCurOpp()));

	deleteCurOpp = new QAction(tr("Delete"), this);
	connect(deleteCurOpp, SIGNAL(triggered()), pXDirect, SLOT(OnDelCurOpp()));

	getOppProps = new QAction(tr("Properties"), this);
	connect(getOppProps, SIGNAL(triggered()), pXDirect, SLOT(OnOpPointProps()));

	getPolarProps = new QAction(tr("Properties"), this);
	connect(getPolarProps, SIGNAL(triggered()), pXDirect, SLOT(OnPolarProps()));

	viewXFoilAdvanced = new QAction(tr("XFoil Advanced Settings"), this);
	m_pBatchAnalysisAct->setStatusTip(tr("Tip : you don't want to use that option..."));
	connect(viewXFoilAdvanced, SIGNAL(triggered()), pXDirect, SLOT(OnXFoilAdvanced()));
 
	viewLogFile = new QAction(tr("View Log File")+"\t(L)", this);
	connect(viewLogFile, SIGNAL(triggered()), this, SLOT(OnLogFile()));

	DerotateFoil = new QAction(tr("De-rotate the Foil"), this);
	connect(DerotateFoil, SIGNAL(triggered()), pXDirect, SLOT(OnDerotateFoil()));

	NormalizeFoil = new QAction(tr("Normalize the Foil"), this);
	connect(NormalizeFoil, SIGNAL(triggered()), pXDirect, SLOT(OnNormalizeFoil()));

	RefineLocalFoil = new QAction(tr("Refine Locally")+"\t(Shift+F3)", this);
	connect(RefineLocalFoil, SIGNAL(triggered()), pXDirect, SLOT(OnCadd()));

	RefineGlobalFoil = new QAction(tr("Refine Globally")+"\t(F3)", this);
	connect(RefineGlobalFoil, SIGNAL(triggered()), pXDirect, SLOT(OnRefinePanelsGlobally()));

	EditCoordsFoil = new QAction(tr("Edit Foil Coordinates"), this);
	connect(EditCoordsFoil, SIGNAL(triggered()), pXDirect, SLOT(OnFoilCoordinates()));

	ScaleFoil = new QAction(tr("Scale camber and thickness")+"\t(F9)", this);
	connect(ScaleFoil, SIGNAL(triggered()), pXDirect, SLOT(OnFoilGeom()));

	SetTEGap = new QAction(tr("Set T.E. Gap"), this);
	connect(SetTEGap, SIGNAL(triggered()), pXDirect, SLOT(OnSetTEGap()));

	SetLERadius = new QAction(tr("Set L.E. Radius"), this);
	connect(SetLERadius, SIGNAL(triggered()), pXDirect, SLOT(OnSetLERadius()));

	SetFlap = new QAction(tr("Set Flap")+"\t(F10)", this);
	connect(SetFlap, SIGNAL(triggered()), pXDirect, SLOT(OnSetFlap()));

	InterpolateFoils = new QAction(tr("Interpolate Foils")+"\t(F11)", this);
	connect(InterpolateFoils, SIGNAL(triggered()), pXDirect, SLOT(OnInterpolateFoils()));

	NacaFoils = new QAction(tr("Naca Foils"), this);
	connect(NacaFoils, SIGNAL(triggered()), pXDirect, SLOT(OnNacaFoils()));

	setCpVarGraph = new QAction(tr("Cp Variable"), this);
	setCpVarGraph->setCheckable(true);
	setCpVarGraph->setStatusTip(tr("Sets Cp vs. chord graph"));
	connect(setCpVarGraph, SIGNAL(triggered()), pXDirect, SLOT(OnCpGraph()));

	setQVarGraph = new QAction(tr("Q Variable"), this);
	setQVarGraph->setCheckable(true);
	setQVarGraph->setStatusTip(tr("Sets Speed vs. chord graph"));
	connect(setQVarGraph, SIGNAL(triggered()), pXDirect, SLOT(OnQGraph()));

	CurXFoilResExport = new QAction(tr("Export Cur. XFoil Results"), this);
	CurXFoilResExport->setStatusTip(tr("Sets Speed vs. chord graph"));
	connect(CurXFoilResExport, SIGNAL(triggered()), pXDirect, SLOT(OnExportCurXFoilResults()));

	CurXFoilCtPlot = new QAction(tr("Max. Shear Coefficient"), this);
	CurXFoilCtPlot->setCheckable(true);
	connect(CurXFoilCtPlot, SIGNAL(triggered()), pXDirect, SLOT(OnCtPlot()));

	CurXFoilDbPlot = new QAction(tr("Bottom Side D* and Theta"), this);
	CurXFoilDbPlot->setCheckable(true);
	connect(CurXFoilDbPlot, SIGNAL(triggered()), pXDirect, SLOT(OnDbPlot()));

	CurXFoilDtPlot = new QAction(tr("Top Side D* and Theta"), this);
	CurXFoilDtPlot->setCheckable(true);
	connect(CurXFoilDtPlot, SIGNAL(triggered()), pXDirect, SLOT(OnDtPlot()));

	CurXFoilRtLPlot = new QAction(tr("Log(Re_Theta)"), this);
	CurXFoilRtLPlot->setCheckable(true);
	connect(CurXFoilRtLPlot, SIGNAL(triggered()), pXDirect, SLOT(OnRtLPlot()));

	CurXFoilRtPlot = new QAction(tr("Re_Theta"), this);
	CurXFoilRtPlot->setCheckable(true);
	connect(CurXFoilRtPlot, SIGNAL(triggered()), pXDirect, SLOT(OnRtPlot()));

	CurXFoilNPlot = new QAction(tr("Amplification Ratio"), this);
	CurXFoilNPlot->setCheckable(true);
	connect(CurXFoilNPlot, SIGNAL(triggered()), pXDirect, SLOT(OnNPlot()));

	CurXFoilCdPlot = new QAction(tr("Dissipation Coefficient"), this);
	CurXFoilCdPlot->setCheckable(true);
	connect(CurXFoilCdPlot, SIGNAL(triggered()), pXDirect, SLOT(OnCdPlot()));

	CurXFoilCfPlot = new QAction(tr("Skin Friction Coefficient"), this);
	CurXFoilCfPlot->setCheckable(true);
	connect(CurXFoilCfPlot, SIGNAL(triggered()), pXDirect, SLOT(OnCfPlot()));

	CurXFoilUePlot = new QAction(tr("Edge Velocity"), this);
	CurXFoilUePlot->setCheckable(true);
	connect(CurXFoilUePlot, SIGNAL(triggered()), pXDirect, SLOT(OnUePlot()));

	CurXFoilHPlot = new QAction(tr("Kinematic Shape Parameter"), this);
	CurXFoilHPlot->setCheckable(true);
	connect(CurXFoilHPlot, SIGNAL(triggered()), pXDirect, SLOT(OnHPlot()));

//	m_pImportJavaFoilPolar = new QAction(tr("Import JavaFoil Polar"), this);
//	connect(m_pImportJavaFoilPolar, SIGNAL(triggered()), pXDirect, SLOT(OnImportJavaFoilPolar()));

	m_pImportXFoilPolar = new QAction(tr("Import XFoil Polar"), this);
	connect(m_pImportXFoilPolar, SIGNAL(triggered()), pXDirect, SLOT(OnImportXFoilPolar()));
}


void MainFrame::CreateXDirectMenus()
{
	//MainMenu for XDirect Application
	m_pXDirectViewMenu = menuBar()->addMenu(tr("&View"));
	{
		m_pXDirectViewMenu->addAction(OpPointsAct);
		m_pXDirectViewMenu->addAction(PolarsAct);
		m_pXDirectViewMenu->addSeparator();
		m_pXDirectViewMenu->addAction(saveViewToImageFileAct);
	}

	m_pXDirectFoilMenu = menuBar()->addMenu(tr("&Foil"));
	{
		m_pXDirectFoilMenu->addAction(ManageFoilsAct);
		m_pXDirectFoilMenu->addSeparator();
		m_pCurrentFoilMenu = m_pXDirectFoilMenu->addMenu(tr("Current Foil"));
		{
			m_pCurrentFoilMenu->addAction(setCurFoilStyle);
			m_pCurrentFoilMenu->addSeparator();
			m_pCurrentFoilMenu->addAction(exportCurFoil);
			m_pCurrentFoilMenu->addAction(renameCurFoil);
			m_pCurrentFoilMenu->addAction(deleteCurFoil);
			m_pCurrentFoilMenu->addAction(pDirectDuplicateCurFoil);
			m_pCurrentFoilMenu->addSeparator();
			m_pCurrentFoilMenu->addAction(showFoilPolarsOnly);
			m_pCurrentFoilMenu->addAction(showFoilPolars);
			m_pCurrentFoilMenu->addAction(hideFoilPolars);
			m_pCurrentFoilMenu->addAction(deleteFoilPolars);
			m_pCurrentFoilMenu->addAction(saveFoilPolars);
			m_pCurrentFoilMenu->addSeparator();
			m_pCurrentFoilMenu->addAction(showFoilOpps);
			m_pCurrentFoilMenu->addAction(hideFoilOpps);
			m_pCurrentFoilMenu->addAction(deleteFoilOpps);
		}
		m_pXDirectFoilMenu->addSeparator();
		m_pXDirectFoilMenu->addAction(m_pResetFoilScale);
		m_pXDirectFoilMenu->addAction(m_pShowPanels);
		m_pXDirectFoilMenu->addAction(m_pShowNeutralLine);
		m_pXDirectFoilMenu->addAction(m_pXDirectStyleAct);
	}

	m_pDesignMenu = menuBar()->addMenu(tr("&Design"));
	{
		m_pDesignMenu->addAction(NormalizeFoil);
		m_pDesignMenu->addAction(DerotateFoil);
		m_pDesignMenu->addAction(RefineGlobalFoil);
		m_pDesignMenu->addAction(RefineLocalFoil);
		m_pDesignMenu->addAction(EditCoordsFoil);
		m_pDesignMenu->addAction(ScaleFoil);
		m_pDesignMenu->addAction(SetTEGap);
		m_pDesignMenu->addAction(SetLERadius);
		m_pDesignMenu->addAction(SetFlap);
		m_pDesignMenu->addSeparator();
		m_pDesignMenu->addAction(InterpolateFoils);
		m_pDesignMenu->addAction(NacaFoils);
	}

	m_pXFoilAnalysisMenu = menuBar()->addMenu(tr("Analysis"));
	{
		m_pXFoilAnalysisMenu->addAction(m_pDefinePolarAct);
		m_pXFoilAnalysisMenu->addAction(m_pBatchAnalysisAct);
		m_pXFoilAnalysisMenu->addAction(MultiThreadedBatchAct);
		m_pXFoilAnalysisMenu->addSeparator();
//		XFoilAnalysisMenu->addAction(resetXFoil);
		m_pXFoilAnalysisMenu->addAction(viewXFoilAdvanced);
		m_pXFoilAnalysisMenu->addAction(viewLogFile);
	}

	m_pPolarMenu = menuBar()->addMenu(tr("&Polars"));
	{
		m_pCurrentPolarMenu = m_pPolarMenu->addMenu(tr("Current Polar"));
		{
			m_pCurrentPolarMenu->addAction(getPolarProps);
			m_pCurrentPolarMenu->addAction(editCurPolar);
			m_pCurrentPolarMenu->addAction(resetCurPolar);
			m_pCurrentPolarMenu->addAction(deletePolar);
			m_pCurrentPolarMenu->addAction(RenamePolarAct);
			m_pCurrentPolarMenu->addAction(exportCurPolar);
			m_pCurrentPolarMenu->addSeparator();
			m_pCurrentPolarMenu->addAction(showPolarOpps);
			m_pCurrentPolarMenu->addAction(hidePolarOpps);
			m_pCurrentPolarMenu->addAction(deletePolarOpps);
			m_pCurrentPolarMenu->addAction(exportPolarOpps);
		}
		m_pPolarMenu->addSeparator();
		m_pPolarMenu->addAction(m_pImportXFoilPolar);
	//	PolarMenu->addAction(m_pImportJavaFoilPolar);
		m_pPolarMenu->addSeparator();
		m_pPolarMenu->addAction(exportAllPolars);
		m_pPolarMenu->addSeparator();
		m_pPolarMenu->addAction(XDirectPolarFilter);
		m_pPolarMenu->addSeparator();
		m_pPolarMenu->addAction(showAllPolars);
		m_pPolarMenu->addAction(hideAllPolars);
		m_pPolarMenu->addSeparator();
/*		m_pGraphPolarMenu = m_pPolarMenu->addMenu(tr("Polar Graphs"));
		{
			m_pGraphPolarMenu->addAction(allPolarGraphsSettingsAct);
			m_pGraphPolarMenu->addAction(allPolarGraphsScales);
			m_pGraphPolarMenu->addAction(m_pResetGraphLegendAct);
			m_pGraphPolarMenu->addSeparator();
			m_pGraphPolarMenu->addAction(AllPolarGraphsAct);
			m_pGraphPolarMenu->addAction(TwoPolarGraphsAct);
			m_pGraphPolarMenu->addSeparator();
			for(int i=0; i<5; i++)
				m_pGraphPolarMenu->addAction(m_pPolarGraphAct[i]);
			m_pGraphPolarMenu->addSeparator();
			m_pGraphPolarMenu->addAction(highlightOppAct);
		}*/
	}

	m_pOpPointMenu = menuBar()->addMenu(tr("Operating Points"));
	{
		m_pCurrentOppMenu = m_pOpPointMenu->addMenu(tr("Current OpPoint"));
		{
			m_pCurrentOppMenu->addAction(exportCurOpp);
			m_pCurrentOppMenu->addAction(deleteCurOpp);
			m_pCurrentOppMenu->addAction(getOppProps);
		}
		m_pXDirectCpGraphMenu = m_pOpPointMenu->addMenu(tr("Cp Graph"));
		{
			m_pXDirectCpGraphMenu->addAction(setCpVarGraph);
			m_pXDirectCpGraphMenu->addAction(setQVarGraph);
			m_pXDirectCpGraphMenu->addSeparator();
			m_pXDirectCpGraphMenu->addAction(showInviscidCurve);
			m_pXDirectCpGraphMenu->addSeparator();
			m_pCurXFoilResults = m_pXDirectCpGraphMenu->addMenu(tr("Current XFoil Results"));
			{
				m_pCurXFoilResults->addAction(CurXFoilResExport);
				m_pCurXFoilResults->addSeparator();
				m_pCurXFoilResults->addAction(CurXFoilCtPlot);
				m_pCurXFoilResults->addAction(CurXFoilDbPlot);
				m_pCurXFoilResults->addAction(CurXFoilDtPlot);
				m_pCurXFoilResults->addAction(CurXFoilRtLPlot);
				m_pCurXFoilResults->addAction(CurXFoilRtPlot);
				m_pCurXFoilResults->addAction(CurXFoilNPlot);
				m_pCurXFoilResults->addAction(CurXFoilCdPlot);
				m_pCurXFoilResults->addAction(CurXFoilCfPlot);
				m_pCurXFoilResults->addAction(CurXFoilUePlot);
				m_pCurXFoilResults->addAction(CurXFoilHPlot);
			}
			m_pXDirectCpGraphMenu->addSeparator();
//			m_pXDirectCpGraphMenu->addAction(resetCurGraphScales);
//			m_pXDirectCpGraphMenu->addAction(exportCurGraphAct);
		}
		m_pOpPointMenu->addSeparator();
		m_pOpPointMenu->addAction(showCurOppOnly);
		m_pOpPointMenu->addAction(hideAllOpPoints);
		m_pOpPointMenu->addAction(showAllOpPoints);
	}

	//XDirect foil Context Menu
	m_pOperFoilCtxMenu = new QMenu(tr("Context Menu"),this);
	{
		m_pOperFoilCtxMenu->addMenu(m_pCurrentFoilMenu);
		m_pOperFoilCtxMenu->addSeparator();//_______________
		m_pOperFoilCtxMenu->addMenu(m_pCurrentPolarMenu);
		m_pOperFoilCtxMenu->addSeparator();//_______________
		m_pOperFoilCtxMenu->addMenu(m_pDesignMenu);
		m_pOperFoilCtxMenu->addSeparator();//_______________
		m_pCurOppCtxMenu = m_pOperFoilCtxMenu->addMenu(tr("Current OpPoint"));
		{
			m_pCurOppCtxMenu->addAction(exportCurOpp);
			m_pCurOppCtxMenu->addAction(deleteCurOpp);
			m_pCurOppCtxMenu->addAction(getOppProps);
		}
		m_pOperFoilCtxMenu->addSeparator();//_______________
	//	CurGraphCtxMenu = OperFoilCtxMenu->addMenu(tr("Cp graph"));
		m_pOperFoilCtxMenu->addMenu(m_pXDirectCpGraphMenu);

		m_pOperFoilCtxMenu->addSeparator();//_______________
		m_pOperFoilCtxMenu->addAction(m_pDefinePolarAct);
		m_pOperFoilCtxMenu->addAction(m_pBatchAnalysisAct);
		m_pOperFoilCtxMenu->addAction(MultiThreadedBatchAct);
		m_pOperFoilCtxMenu->addSeparator();//_______________
		m_pOperFoilCtxMenu->addAction(showAllPolars);
		m_pOperFoilCtxMenu->addAction(hideAllPolars);
		m_pOperFoilCtxMenu->addSeparator();//_______________
		m_pOperFoilCtxMenu->addAction(showCurOppOnly);
		m_pOperFoilCtxMenu->addAction(showAllOpPoints);
		m_pOperFoilCtxMenu->addAction(hideAllOpPoints);
		m_pOperFoilCtxMenu->addSeparator();//_______________
		m_pOperFoilCtxMenu->addAction(m_pResetFoilScale);
		m_pOperFoilCtxMenu->addAction(m_pShowPanels);
		m_pOperFoilCtxMenu->addAction(m_pShowNeutralLine);
		m_pOperFoilCtxMenu->addAction(m_pXDirectStyleAct);
		m_pOperFoilCtxMenu->addSeparator();//_______________
		m_pOperFoilCtxMenu->addAction(saveViewToImageFileAct);
	}
	//End XDirect foil Context Menu


	//XDirect polar Context Menu
	m_pOperPolarCtxMenu = new QMenu(tr("Context Menu"),this);
	{
		m_pOperPolarCtxMenu->addMenu(m_pCurrentFoilMenu);
		m_pOperPolarCtxMenu->addMenu(m_pCurrentPolarMenu);
		m_pOperPolarCtxMenu->addSeparator();//_______________
		m_pCurGraphCtxMenu = m_pOperPolarCtxMenu->addMenu(tr("Current Graph"));
		{
			m_pCurGraphCtxMenu->addAction(m_pResetCurGraphScales);
			m_pCurGraphCtxMenu->addAction(m_pCurGraphDlgAct);
			m_pCurGraphCtxMenu->addAction(m_pExportCurGraphAct);
		}
		m_pOperPolarCtxMenu->addAction(m_pAllGraphsSettings);
		m_pOperPolarCtxMenu->addAction(m_pAllGraphsScalesAct);
		m_pOperPolarCtxMenu->addSeparator();//_______________
		m_pOperPolarCtxMenu->addAction(m_pDefinePolarAct);
		m_pOperPolarCtxMenu->addAction(m_pBatchAnalysisAct);
		m_pOperPolarCtxMenu->addAction(MultiThreadedBatchAct);
		m_pOperPolarCtxMenu->addSeparator();//_______________
		m_pOperPolarCtxMenu->addAction(showAllPolars);
		m_pOperPolarCtxMenu->addAction(hideAllPolars);
		m_pOperPolarCtxMenu->addAction(showAllOpPoints);
		m_pOperPolarCtxMenu->addAction(hideAllOpPoints);
		m_pOperPolarCtxMenu->addSeparator();//_______________
		m_pOperPolarCtxMenu->addAction(saveViewToImageFileAct);
	}

	//End XDirect polar Context Menu
}


void MainFrame::CreateXInverseActions()
{
	QXInverse *pXInverse = (QXInverse*)m_pXInverse;

	StoreFoil = new QAction(QIcon(":/images/OnStoreFoil.png"), tr("Store Foil"), this);
	StoreFoil->setStatusTip(tr("Store Foil in database"));
	connect(StoreFoil, SIGNAL(triggered()), pXInverse, SLOT(OnStoreFoil()));

	ExtractFoil = new QAction(QIcon(":/images/OnExtractFoil.png"),tr("Extract Foil"), this);
	ExtractFoil->setStatusTip(tr("Extract a Foil from the database for modification"));
	connect(ExtractFoil, SIGNAL(triggered()), pXInverse, SLOT(OnExtractFoil()));

	m_pXInverseStyles = new QAction(tr("Define Styles"), this);
	m_pXInverseStyles->setStatusTip(tr("Define the styles for this view"));
	connect(m_pXInverseStyles, SIGNAL(triggered()), pXInverse, SLOT(OnInverseStyles()));

	m_pXInverseResetFoilScale = new QAction(QIcon(":/images/OnResetFoilScale.png"), tr("Reset foil scale")+"\t(R)", this);
	m_pXInverseResetFoilScale->setStatusTip(tr("Resets the scale to fit the screen size"));
	connect(m_pXInverseResetFoilScale, SIGNAL(triggered()), pXInverse, SLOT(OnResetFoilScale()));

	InverseInsertCtrlPt = new QAction(tr("Insert Control Point")+"\tShift+Click", this);
	connect(InverseInsertCtrlPt, SIGNAL(triggered()), pXInverse, SLOT(OnInsertCtrlPt()));

	InverseRemoveCtrlPt = new QAction(tr("Remove Control Point")+"\tCtrl+Click", this);
	connect(InverseRemoveCtrlPt, SIGNAL(triggered()), pXInverse, SLOT(OnRemoveCtrlPt()));

	InvQInitial = new QAction(tr("Show Q-Initial"), this);
	InvQInitial->setCheckable(true);
	connect(InvQInitial, SIGNAL(triggered()), pXInverse, SLOT(OnQInitial()));

	InvQSpec = new QAction(tr("Show Q-Spec"), this);
	InvQSpec->setCheckable(true);
	connect(InvQSpec, SIGNAL(triggered()), pXInverse, SLOT(OnQSpec()));

	InvQViscous = new QAction(tr("Show Q-Viscous"), this);
	InvQViscous->setCheckable(true);
	connect(InvQViscous, SIGNAL(triggered()), pXInverse, SLOT(OnQViscous()));

	InvQPoints = new QAction(tr("Show Points"), this);
	InvQPoints->setCheckable(true);
	connect(InvQPoints, SIGNAL(triggered()), pXInverse, SLOT(OnQPoints()));

	InvQReflected = new QAction(tr("Show Reflected"), this);
	InvQReflected->setCheckable(true);
	connect(InvQReflected, SIGNAL(triggered()), pXInverse, SLOT(OnQReflected()));

	InverseZoomIn = new QAction(QIcon(":/images/OnZoomIn.png"), tr("Zoom in"), this);
	InverseZoomIn->setStatusTip(tr("Zoom the view by drawing a rectangle in the client area"));
	connect(InverseZoomIn, SIGNAL(triggered()), pXInverse, SLOT(OnZoomIn()));

	InverseZoomX = new QAction(QIcon(":/images/OnZoomGraphX.png"), tr("Zoom X Scale"), this);
	InverseZoomX->setStatusTip(tr("Zoom X Scale Only"));
	connect(InverseZoomX, SIGNAL(triggered()), pXInverse, SLOT(OnZoomX()));

	InverseZoomY = new QAction(QIcon(":/images/OnZoomGraphY.png"), tr("Zoom Y Scale"), this);
	InverseZoomY->setStatusTip(tr("Zoom Y Scale Only"));
	connect(InverseZoomY, SIGNAL(triggered()), pXInverse, SLOT(OnZoomY()));
}


void MainFrame::CreateXInverseMenus()
{
	//MainMenu for XInverse Application
	m_pXInverseViewMenu = menuBar()->addMenu(tr("&View"));
	m_pXInverseViewMenu->addAction(m_pXInverseStyles);
	m_pXInverseViewMenu->addSeparator();
	m_pXInverseViewMenu->addAction(saveViewToImageFileAct);

	m_pXInverseGraphMenu = menuBar()->addMenu(tr("&Graph"));
	m_pXInverseGraphMenu->addAction(m_pCurGraphDlgAct);
	m_pXInverseGraphMenu->addAction(m_pResetCurGraphScales);
	m_pXInverseGraphMenu->addAction(m_pExportCurGraphAct);

	m_pXInverseFoilMenu = menuBar()->addMenu(tr("&Foil"));
	m_pXInverseFoilMenu->addAction(StoreFoil);
	m_pXInverseFoilMenu->addAction(ExtractFoil);
	m_pXInverseFoilMenu->addAction(m_pXInverseResetFoilScale);
	m_pXInverseFoilMenu->addSeparator();
	m_pXInverseFoilMenu->addAction(InvQInitial);
	m_pXInverseFoilMenu->addAction(InvQSpec);
	m_pXInverseFoilMenu->addAction(InvQViscous);
	m_pXInverseFoilMenu->addAction(InvQPoints);
	m_pXInverseFoilMenu->addAction(InvQReflected);

	//Context Menu for XInverse Application
	m_pInverseContextMenu = new QMenu(tr("Context Menu"),this);
	m_pInverseContextMenu->addAction(m_pXInverseStyles);
	m_pInverseContextMenu->addAction(m_pXInverseResetFoilScale);
	m_pInverseContextMenu->addSeparator();
	m_pInverseContextMenu->addAction(m_pCurGraphDlgAct);
	m_pInverseContextMenu->addAction(m_pResetCurGraphScales);
	m_pInverseContextMenu->addAction(m_pExportCurGraphAct);
	m_pInverseContextMenu->addSeparator();
	m_pInverseContextMenu->addAction(InverseInsertCtrlPt);
	m_pInverseContextMenu->addAction(InverseRemoveCtrlPt);
	m_pInverseContextMenu->addSeparator();
	m_pInverseContextMenu->addAction(InvQInitial);
	m_pInverseContextMenu->addAction(InvQSpec);
	m_pInverseContextMenu->addAction(InvQViscous);
	m_pInverseContextMenu->addAction(InvQPoints);
	m_pInverseContextMenu->addAction(InvQReflected);
}



void MainFrame::CreateXInverseToolbar()
{
	m_pctrlFullInverse  = new QRadioButton(tr("Full Inverse"));
	m_pctrlMixedInverse = new QRadioButton(tr("Mixed Inverse"));
	QXInverse *pXInverse = (QXInverse*)m_pXInverse;
	connect(m_pctrlFullInverse,  SIGNAL(clicked()), pXInverse, SLOT(OnInverseApp()));
	connect(m_pctrlMixedInverse, SIGNAL(clicked()), pXInverse, SLOT(OnInverseApp()));

	m_pctrlXInverseToolBar = addToolBar(tr("XInverse"));
	m_pctrlXInverseToolBar->addAction(newProjectAct);
	m_pctrlXInverseToolBar->addAction(openAct);
	m_pctrlXInverseToolBar->addAction(saveAct);
	m_pctrlXInverseToolBar->addSeparator();
	m_pctrlXInverseToolBar->addWidget(m_pctrlFullInverse);
	m_pctrlXInverseToolBar->addWidget(m_pctrlMixedInverse);
	m_pctrlXInverseToolBar->addSeparator();
	m_pctrlXInverseToolBar->addAction(ExtractFoil);
	m_pctrlXInverseToolBar->addAction(StoreFoil);
	m_pctrlXInverseToolBar->addSeparator();
	m_pctrlXInverseToolBar->addAction(InverseZoomIn);
	m_pctrlXInverseToolBar->addAction(InverseZoomX);
	m_pctrlXInverseToolBar->addAction(InverseZoomY);
	m_pctrlXInverseToolBar->addAction(m_pResetCurGraphScales);
	m_pctrlXInverseToolBar->addAction(m_pXInverseResetFoilScale);
}





void MainFrame::deleteProject(bool bClosing)
{
	// clear everything
	int i;
	void *pObj;

	Objects3D::deleteObjects();

	for (i=Foil::s_oaFoil.size()-1; i>=0; i--)
	{
		pObj = Foil::s_oaFoil.at(i);
		Foil::s_oaFoil.removeAt(i);
		delete (Foil*)pObj;
	}
	for (i=Polar::s_oaPolar.size()-1; i>=0; i--)
	{
		pObj = Polar::s_oaPolar.at(i);
		Polar::s_oaPolar.removeAt(i);
		delete (Polar*)pObj;
	}
	for (i=OpPoint::s_oaOpp.size()-1; i>=0; i--)
	{
		pObj = OpPoint::s_oaOpp.at(i);
		OpPoint::s_oaOpp.removeAt(i);
		delete (OpPoint*)pObj;
	}

	QMiarex * pMiarex = (QMiarex*)m_pMiarex;
	pMiarex->m_pCurPlane  = NULL;
	pMiarex->m_pCurPOpp   = NULL;
	pMiarex->m_pCurWPolar = NULL;
	pMiarex->m_bStream = false;

	if(!bClosing)
	{
		m_pctrlPlane->clear();
		m_pctrlPlanePolar->clear();
		m_pctrlPlaneOpp->clear();
		m_pctrlFoil->clear();
		m_pctrlPolar->clear();
		m_pctrlOpPoint->clear();

		m_pctrlPlane->setEnabled(false);
		m_pctrlPlanePolar->setEnabled(false);
		m_pctrlPlaneOpp->setEnabled(false);
		m_pctrlFoil->setEnabled(false);
		m_pctrlPolar->setEnabled(false);
		m_pctrlOpPoint->setEnabled(false);


		pMiarex->setPlane();
		if(pMiarex->m_iView==XFLR5::WPOLARVIEW)    pMiarex->createWPolarCurves(); /** @todo --> in miarex! */
		else if(pMiarex->m_iView==XFLR5::WOPPVIEW) pMiarex->CreateWOppCurves();
		else if(pMiarex->m_iView==XFLR5::WCPVIEW)  pMiarex->createCpCurves();
		if(m_iApp==XFLR5::MIAREX) pMiarex->setControls();

		QXDirect *pXDirect = (QXDirect*)m_pXDirect;
		pXDirect->m_pXFoil->m_FoilName = "";
		Foil::setCurFoil(NULL);
		Polar::setCurPolar(NULL);
		OpPoint::setCurOpp(NULL);
		pXDirect->setFoil();

		UpdateFoilListBox();
		if(pXDirect->m_bPolarView) pXDirect->createPolarCurves();
		else                       pXDirect->createOppCurves();

		QAFoil *pAFoil = (QAFoil*)m_pAFoil;
		pAFoil->FillFoilTable();
		pAFoil->SelectFoil();


		QXInverse *pXInverse =(QXInverse*)m_pXInverse;
		pXInverse->Clear();

		SetProjectName("");
		setSaveState(true);
	}
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
	int i,j;
	bool bFound = false;
	switch (type)
	{
		case 0:
		{
			Foil *pFoil;
			for (j=0; j<s_ColorList.size(); j++)
			{
				for (i=0; i<Foil::s_oaFoil.size(); i++)
				{
					pFoil = (Foil*)Foil::s_oaFoil.at(i);
					bFound = false;
					if(pFoil->m_FoilColor == s_ColorList.at(j))
					{
						bFound = true;
						break;
					}
				}
				if(!bFound) return s_ColorList.at(j);
			}
			break;
		}
		case 1:
		{
			Polar *pPolar;
			for (j=0; j<s_ColorList.size(); j++)
			{
				for (i=0; i<Polar::s_oaPolar.size(); i++)
				{
					pPolar = (Polar*)Polar::s_oaPolar.at(i);
					bFound = false;
					if(pPolar->m_Color == s_ColorList.at(j))
					{
						bFound = true;
						break;
					}
				}
				if(!bFound)
					return s_ColorList.at(j);
			}
			break;
		}
		case 2:
		{
			OpPoint *pOpPoint;
			for (j=0; j<s_ColorList.size(); j++){
				for (i=0; i<OpPoint::s_oaOpp.size(); i++)
				{
					pOpPoint = (OpPoint*)OpPoint::s_oaOpp.at(i);
					bFound = false;
					if(pOpPoint->m_Color == s_ColorList.at(j))
					{
						bFound = true;
						break;
					}
				}
				if(!bFound) return s_ColorList.at(j);
			}
			break;
		}

		default:
		{
		}
	}
	return randomColor();
}



void MainFrame::keyPressEvent(QKeyEvent *event)
{
	bool bCtrl = (event->modifiers() & Qt::ControlModifier);

	if(m_iApp == XFLR5::XFOILANALYSIS && m_pXDirect)
	{
		QXDirect* pXDirect = (QXDirect*)m_pXDirect;
		pXDirect->keyPressEvent(event);
	}
	else if(m_iApp == XFLR5::MIAREX && m_pMiarex)
	{
		QMiarex* pMiarex = (QMiarex*)m_pMiarex;
		pMiarex->keyPressEvent(event);
	}
	else if(m_iApp == XFLR5::DIRECTDESIGN && m_pAFoil)
	{
		QAFoil *pAFoil= (QAFoil*)m_pAFoil;
		pAFoil->keyPressEvent(event);
	}
	else if(m_iApp == XFLR5::INVERSEDESIGN && m_pXInverse)
	{
		QXInverse *pXInverse= (QXInverse*)m_pXInverse;
		pXInverse->keyPressEvent(event);
	}
    else
	{
		switch (event->key())
		{
		    case Qt::Key_1:
		    {
				if(bCtrl) OnAFoil();
				break;
			}
		    case Qt::Key_2:
		    {
				if(bCtrl) OnAFoil();
				break;
			}
		    case Qt::Key_3:
		    {
				if(bCtrl) OnXInverse();
				break;
			}
		    case Qt::Key_4:
		    {
				if(bCtrl) OnXInverseMixed();
				break;
			}
		    case Qt::Key_5:
		    {
				if(bCtrl) onXDirect();
				break;
		    }
		    case Qt::Key_6:
		    {
				if(bCtrl) onMiarex();
				break;
			}
			case Qt::Key_L:
			{
				OnLogFile();
				break;
			}
			case Qt::Key_Control:
			{
				QMiarex* pMiarex = (QMiarex*)m_pMiarex;
				pMiarex->m_bArcball = true;
				updateView();
				break;
			}	

			default:
				event->ignore();
		}
	}
	event->accept();
}


void MainFrame::keyReleaseEvent(QKeyEvent *event)
{
	if(m_iApp == XFLR5::XFOILANALYSIS && m_pXDirect)
	{
		QXDirect* pXDirect = (QXDirect*)m_pXDirect;
		pXDirect->keyReleaseEvent(event);
	}
	else if(m_iApp == XFLR5::MIAREX && m_pMiarex)
	{
		QMiarex* pMiarex = (QMiarex*)m_pMiarex;

		if (event->key()==Qt::Key_Control)
		{
			pMiarex->m_bArcball = false;
			updateView();
		}
		else pMiarex->keyReleaseEvent(event);
	}
	else if(m_iApp == XFLR5::DIRECTDESIGN && m_pAFoil)
	{
		QAFoil *pAFoil= (QAFoil*)m_pAFoil;
		pAFoil->keyReleaseEvent(event);
	}
	else if(m_iApp == XFLR5::INVERSEDESIGN && m_pXInverse)
	{
		QXInverse *pXInverse= (QXInverse*)m_pXInverse;
		pXInverse->keyReleaseEvent(event);
	}
	event->accept();
}


bool MainFrame::LoadPolarFileV3(QDataStream &ar, bool bIsStoring, int ArchiveFormat)
{
	Foil *pFoil;
	Polar *pPolar = NULL;
	Polar *pOldPlr;
	OpPoint *pOpp, *pOldOpp;

	//first read all available foils
	int i,l,n;
	ar >> n;
//	if(n<100003) return false;

	ar>>n;

	for (i=0;i<n; i++)
	{
		pFoil = new Foil();

		if (!pFoil->Serialize(ar, bIsStoring))
		{
			delete pFoil;
			return false;
		}

		pFoil->insertThisFoil();
	}

	//next read all available polars
	ar>>n;
	for (i=0; i<n; i++)
	{
		pPolar = new Polar();

		pPolar->m_Color = GetColor(1);

		if (!pPolar->Serialize(ar, bIsStoring))
		{
			delete pPolar;
			return false;
		}
		for (l=0; l<Polar::s_oaPolar.size(); l++)
		{
			pOldPlr = (Polar*)Polar::s_oaPolar[l];
			if (pOldPlr->m_FoilName == pPolar->m_FoilName &&
				pOldPlr->m_PlrName  == pPolar->m_PlrName)
			{
				//just overwrite...
				Polar::s_oaPolar.removeAt(l);
				delete pOldPlr;
				//... and continue to add
			}
		}
		Polar::addPolar(pPolar);
	}

	//Last read all available operating points
	ar>>n;
	for (i=0; i<n; i++)
	{
		pOpp = new OpPoint();
		if(!pOpp)
		{
			delete pOpp;
			return false;
		}

		pOpp->m_Color = s_ColorList[OpPoint::s_oaOpp.size()%24];
		if(ArchiveFormat>=100002)
		{
			if (!pOpp->SerializeOppWPA(ar, bIsStoring, 100002))
			{
				delete pOpp;
				return false;
			}
			else
			{
				pFoil = Foil::foil(pOpp->m_strFoilName);
				if(pFoil)
				{
//					memcpy(pOpp->x, pFoil->x, sizeof(pOpp->x));
//					memcpy(pOpp->y, pFoil->y, sizeof(pOpp->y));
				}
				else
				{
					delete pOpp;
				}
			}
		}
		else
		{
			if (!pOpp->SerializeOppWPA(ar, bIsStoring))
			{
				delete pOpp;
				return false;
			}
			else
			{
				pFoil = Foil::foil(pOpp->m_strFoilName);
				if(pFoil)
				{
//					memcpy(pOpp->x, pFoil->x, sizeof(pOpp->x));
//					memcpy(pOpp->y, pFoil->y, sizeof(pOpp->y));
				}
				else
				{
					delete pOpp;
				}
			}
		}
		if(pOpp)
		{
			for (int l=OpPoint::s_oaOpp.size()-1;l>=0; l--)
			{
				pOldOpp = (OpPoint*)OpPoint::s_oaOpp.at(l);
				if (pOldOpp->m_strFoilName == pOpp->m_strFoilName &&
					pOldOpp->m_strPlrName  == pOpp->m_strPlrName &&
					qAbs(pOldOpp->Alpha-pOpp->Alpha)<0.001)
				{
					//just overwrite...
					OpPoint::s_oaOpp.removeAt(l);
					delete pOldOpp;
					//... and continue to add
				}
			}
		}
		OpPoint::insertOpPoint(pOpp);
	}

	return true;
}


bool MainFrame::loadSettings()
{
	QPoint pt;
	bool bFloat;
	int SettingsFormat;
	QSize size;
	

#ifdef Q_OS_MAC
        QSettings settings(QSettings::NativeFormat,QSettings::UserScope,"sourceforge.net","xflr5");
#else
        QSettings settings(QSettings::IniFormat,QSettings::UserScope,"XFLR5");
#endif

	settings.beginGroup("MainFrame");
	{
		SettingsFormat = settings.value("SettingsFormat").toInt();
		if(SettingsFormat != SETTINGSFORMAT) return false;


		Settings::s_StyleName = settings.value("StyleName","").toString();
		int k = settings.value("ExportFileType", 0).toInt();
		if (k==0) Settings::s_ExportFileType = XFLR5::TXT;
		else      Settings::s_ExportFileType = XFLR5::CSV;

		s_LanguageFilePath = settings.value("LanguageFilePath").toString();

		m_GraphExportFilter = settings.value("GraphExportFilter",".csv").toString();

		bFloat  = settings.value("Miarex_Float").toBool();
		pt.rx() = settings.value("Miarex_x").toInt();
		pt.ry() = settings.value("Miarex_y").toInt();
		size    = settings.value("MiarexSize").toSize();
		m_pctrlMiarexWidget->setFloating(bFloat);
		if(bFloat) m_pctrlMiarexWidget->move(pt);
		m_pctrlMiarexWidget->resize(size);

		bFloat  = settings.value("XDirect_Float").toBool();
		pt.rx() = settings.value("XDirect_x").toInt();
		pt.ry() = settings.value("XDirect_y").toInt();
		size    = settings.value("XDirectSize").toSize();
		m_pctrlXDirectWidget->setFloating(bFloat);
		if(bFloat) m_pctrlXDirectWidget->move(pt);
		m_pctrlXDirectWidget->resize(size);

		bFloat  = settings.value("AFoil_Float").toBool();
		pt.rx() = settings.value("AFoil_x").toInt();
		pt.ry() = settings.value("AFoil_y").toInt();
		size    = settings.value("AFoilSize").toSize();
		m_pctrlAFoilWidget->setFloating(bFloat);
		if(bFloat) m_pctrlAFoilWidget->move(pt);
		m_pctrlAFoilWidget->resize(size);

		bFloat  = settings.value("XInverse_Float").toBool();
		pt.rx() = settings.value("XInverse_x").toInt();
		pt.ry() = settings.value("XInverse_y").toInt();
		size    = settings.value("XInverseSize").toSize();
		m_pctrlXInverseWidget->setFloating(bFloat);
		if(bFloat) m_pctrlXInverseWidget->move(pt);
		m_pctrlXInverseWidget->resize(size);

		bFloat  = settings.value("StabView_Float").toBool();
		pt.rx() = settings.value("StabView_x").toInt();
		pt.ry() = settings.value("StabView_y").toInt();
		size    = settings.value("StabSize").toSize();
		m_pctrlStabViewWidget->setFloating(bFloat);
		if(bFloat) m_pctrlStabViewWidget->move(pt);
		m_pctrlStabViewWidget->resize(size);

		m_ImageDirName = settings.value("ImageDirName").toString();
		m_ExportLastDirName = settings.value("ExportLastDirName").toString();

		Units::s_LengthUnit  = settings.value("LengthUnit").toInt();
		Units::s_AreaUnit    = settings.value("AreaUnit").toInt();
		Units::s_WeightUnit  = settings.value("WeightUnit").toInt();
		Units::s_SpeedUnit   = settings.value("SpeedUnit").toInt();
		Units::s_ForceUnit   = settings.value("ForceUnit").toInt();
		Units::s_MomentUnit  = settings.value("MomentUnit").toInt();
		Units::SetUnitConversionFactors();


		switch(settings.value("ImageFormat").toInt())
		{
			case 0:
				m_ImageFormat = XFLR5::PNG;
				break;
			case 1:
				m_ImageFormat = XFLR5::JPEG;
				break;
			case 2:
				m_ImageFormat = XFLR5::BMP;
				break;
			default:
				m_ImageFormat = XFLR5::PNG;
				break;
		}

		m_bSaveOpps   = settings.value("SaveOpps").toBool();
		m_bSaveWOpps  = settings.value("SaveWOpps").toBool();

//		a = settings.value("RecentFileSize").toInt();
		QString RecentF,strange;
		m_RecentFiles.clear();
		int n=0;
		do
		{
			RecentF = QString("RecentFile_%1").arg(n);
			strange = settings.value(RecentF).toString();
			if(strange.length())
			{
				m_RecentFiles.append(strange);
				n++;
			}
			else break;
		}while(n<MAXRECENTFILES);


		Settings::s_bStyleSheets  = settings.value("ShowStyleSheets", false).toBool();
		Settings::s_StyleSheetName = settings.value("StyleSheetName", "xflr5_style").toString();
	}

	return true;
}


MainFrame* MainFrame::self() {
    if (!_self) {
        _self = new MainFrame(0L, 0L);
    }
    return _self;
}


XFLR5::enumApp MainFrame::LoadXFLR5File(QString PathName)
{
	QFile XFile(PathName);
	if (!XFile.open(QIODevice::ReadOnly))
	{
		QString strange = tr("Could not open the file\n")+PathName;
		QMessageBox::information(window(), tr("Info"), strange);
		return XFLR5::NOAPP;
	}

	QAFoil *pAFoil= (QAFoil*)m_pAFoil;
	QXDirect * pXDirect = (QXDirect*)m_pXDirect;

	QString end;
	//QFileInfo fileinfo(XFile);
	end = PathName.right(4);
	end = end.toLower();
	//QString dir = fileinfo.canonicalPath();

	PathName.replace(QDir::separator(), "/"); // Qt sometimes uses the windows \ separator

	int pos = PathName.lastIndexOf("/");
	if(pos>0) Settings::s_LastDirName = PathName.left(pos);
	if(end==".plr")
	{
		QDataStream ar(&XFile);
		ar.setVersion(QDataStream::Qt_4_5);
		ar.setByteOrder(QDataStream::LittleEndian);

		ReadPolarFile(ar);

		pXDirect->m_bPolarView = true;
		Polar::setCurPolar(NULL);
		OpPoint::setCurOpp(NULL);

		pXDirect->setFoil();

		pXDirect->setPolar();

		XFile.close();

		AddRecentFile(PathName);
		setSaveState(false);
		pXDirect->setControls();
		return XFLR5::XFOILANALYSIS;
	}
	else if(end==".dat")
	{
		QTextStream ar(&XFile);
		Foil *pFoil = (Foil*)ReadFoilFile(ar);
		XFile.close();

		if(pFoil)
		{
			pFoil->insertThisFoil();
			Foil::setCurFoil(pFoil);
			Polar::setCurPolar(NULL);
			OpPoint::setCurOpp(NULL);

			if(m_iApp==XFLR5::XFOILANALYSIS)
			{
				pXDirect->setControls();
				pXDirect->setFoil(pFoil);
			}
			else if(m_iApp==XFLR5::DIRECTDESIGN)  pAFoil->SelectFoil(pFoil);

			XFile.close();

			setSaveState(false);
			AddRecentFile(PathName);

			return XFLR5::XFOILANALYSIS;
		}
	}
	else if(end==".wpa")
	{
		if(!s_bSaved)
		{
			QString strong = tr("Save the current project ?");
			int resp =  QMessageBox::question(this ,tr("Save"), strong,  QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
			if(resp==QMessageBox::Cancel)
			{
				XFile.close();
				return XFLR5::NOAPP;
			}
			else if (resp==QMessageBox::Yes)
			{
				if(!SaveProject(m_FileName))
				{
					XFile.close();
					return XFLR5::NOAPP;
				}
			}
		}

		deleteProject();

		QDataStream ar(&XFile);
		ar.setVersion(QDataStream::Qt_4_5);
		ar.setByteOrder(QDataStream::LittleEndian);
		if(!SerializeProjectWPA(ar, false))
		{
			QMessageBox::warning(this,tr("Warning"), tr("Error reading the file")+"\n"+tr("Saved the valid part"));
		}

		AddRecentFile(PathName);
		setSaveState(true);
		PathName.replace(".wpa", ".xfl", Qt::CaseInsensitive);
		SetProjectName(PathName);

		XFile.close();

		if(Objects3D::s_oaPlane.size()) return XFLR5::MIAREX;
		else                            return XFLR5::XFOILANALYSIS;
	}
	else if(end==".xfl")
	{
		if(!s_bSaved)
		{
			QString strong = tr("Save the current project ?");
			int resp =  QMessageBox::question(this ,tr("Save"), strong, QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
			if(resp==QMessageBox::Cancel)
			{
				XFile.close();
				return XFLR5::NOAPP;
			}
			else if (resp==QMessageBox::Yes)
			{
				if(!SaveProject(m_FileName))
				{
					XFile.close();
					return XFLR5::NOAPP;
				}
			}
		}

		deleteProject();

		QDataStream ar(&XFile);
		if(!SerializeProjectXFL(ar, false))
		{
			QMessageBox::warning(this,tr("Warning"), tr("Error reading the file")+"\n"+tr("Saved the valid part"));
		}

		AddRecentFile(PathName);
		setSaveState(true);
		SetProjectName(PathName);

		XFile.close();

		if(Objects3D::s_oaPlane.size()) return XFLR5::MIAREX;
		else                            return XFLR5::XFOILANALYSIS;
	}


	XFile.close();

	return XFLR5::NOAPP;
}



void MainFrame::OnAFoil()
{
	QMiarex *pMiarex = (QMiarex*)m_pMiarex;
	pMiarex->stopAnimate();

	QXDirect *pXDirect = (QXDirect*)m_pXDirect;
	pXDirect->stopAnimate();

	m_iApp = XFLR5::DIRECTDESIGN;
	m_pctrlMiarexToolBar->hide();
	m_pctrlStabViewWidget->hide();
	m_pctrlXDirectToolBar->hide();
	m_pctrlXInverseToolBar->hide();
	m_pctrlAFoilToolBar->show();

	m_pctrlMiarexWidget->hide();
	m_pctrlXDirectWidget->hide();
	m_pctrlXInverseWidget->hide();
	m_pctrlAFoilWidget->show();

	setMainFrameCentralWidget();
	setMenus();
	QAFoil *pAFoil = (QAFoil*)m_pAFoil;
	pAFoil->SetAFoilParams();
}


void MainFrame::OnCurFoilStyle()
{
	if(!Foil::curFoil()) return;

	LinePickerDlg dlg(this);
	dlg.InitDialog(Foil::curFoil()->m_FoilStyle, Foil::curFoil()->m_FoilWidth, Foil::curFoil()->m_FoilColor);

	if(QDialog::Accepted==dlg.exec())
	{
		Foil::curFoil()->m_FoilColor = dlg.GetColor();
		Foil::curFoil()->m_FoilStyle = dlg.GetStyle();
		Foil::curFoil()->m_FoilWidth = dlg.GetWidth();
		QXDirect *pXDirect = (QXDirect*)m_pXDirect;
		pXDirect->m_BufferFoil.m_FoilColor = Foil::curFoil()->m_FoilColor;
		pXDirect->m_BufferFoil.m_FoilStyle = Foil::curFoil()->m_FoilStyle;
		pXDirect->m_BufferFoil.m_FoilWidth = Foil::curFoil()->m_FoilWidth;
		setSaveState(false);
	}

	updateView();
}



void MainFrame::OnInsertProject()
{
	QString PathName;
	QAFoil *pAFoil = (QAFoil*)m_pAFoil;
	QXDirect *pXDirect = (QXDirect*)m_pXDirect;
	QMiarex *pMiarex = (QMiarex*)m_pMiarex;
	pMiarex->m_bArcball = false;

	PathName = QFileDialog::getOpenFileName(this, tr("Open File"),
											Settings::s_LastDirName,
											tr("Project file (*.xfl)"));
	if(!PathName.length())		return;
	int pos = PathName.lastIndexOf("/");
	if(pos>0) Settings::s_LastDirName = PathName.left(pos);


	QFile XFile(PathName);
	if (!XFile.open(QIODevice::ReadOnly))
	{
		QString strange = tr("Could not read the file\n") + PathName;
		QMessageBox::information(window(), tr("Warning"), strange);
		return;
	}

	QDataStream ar(&XFile);

	SerializeProjectXFL(ar, false);

	setSaveState(false);

	if(m_iApp == XFLR5::MIAREX)
	{
		updatePlaneListBox();
		pMiarex->setPlane();

		if(pMiarex->m_iView==XFLR5::WPOLARVIEW)    pMiarex->createWPolarCurves();
		else if(pMiarex->m_iView==XFLR5::WOPPVIEW) pMiarex->CreateWOppCurves();
		else if(pMiarex->m_iView==XFLR5::WCPVIEW)  pMiarex->createCpCurves();
	}
	else if(m_iApp == XFLR5::XFOILANALYSIS)
	{
		if(pXDirect->m_bPolarView) pXDirect->createPolarCurves();
		else                       pXDirect->createOppCurves();
		UpdateFoilListBox();
	}
	else if(m_iApp == XFLR5::DIRECTDESIGN)
	{
		pAFoil->FillFoilTable();
		pAFoil->SelectFoil();
	}
	updateView();
}



void MainFrame::OnLanguage()
{
    TranslatorDlg tDlg(this);
    tDlg.InitDialog();
    if(tDlg.exec()==QDialog::Accepted)
	{
	}
}




void MainFrame::OnLoadFile()
{
	QStringList PathNames;
	QString PathName;
	XFLR5::enumApp App  = XFLR5::NOAPP;
	bool warn_non_airfoil_multiload = false;
	QXDirect *pXDirect = (QXDirect*)m_pXDirect;
	QMiarex *pMiarex = (QMiarex*)m_pMiarex;
	pMiarex->m_bArcball = false;

	PathNames = QFileDialog::getOpenFileNames(this, tr("Open File"),
												Settings::s_LastDirName,
												"XFLR5 file (*.dat *.plr *.wpa *.xfl)");
	if(!PathNames.size()) return;
	if(PathNames.size() > 1)
	{
		for (int i=0; i<PathNames.size(); i++)
		{
			PathName = PathNames.at(i);
			if (PathName.endsWith(".dat"))
			{
				App = LoadXFLR5File(PathName);
			} else {
				warn_non_airfoil_multiload = true;
			}
		}
		if (warn_non_airfoil_multiload) {
			QMessageBox::warning(0, QObject::tr("Warning"), QObject::tr("Multiple file loading only available for airfoil files.\nNon *.dat files will be ignored."));
		}
    }
    else
    {
		PathName = PathNames.at(0);
		if(!PathName.length()) return;


        PathName.replace(QDir::separator(), "/"); // Qt sometimes uses the windows \ separator

		int pos = PathName.lastIndexOf("/");
		if(pos>0) Settings::s_LastDirName = PathName.left(pos);

		App = LoadXFLR5File(PathName);
	}

	if(m_iApp==XFLR5::NOAPP)
	{
		m_iApp = App;
//		QString strange = PathName.right(4);
//		strange = strange.toLower();
		if(m_iApp==XFLR5::MIAREX) onMiarex();
		else               onXDirect();
	}

	if(App==0)
	{
	}
	else if(m_iApp==XFLR5::XFOILANALYSIS)
	{
		if(Polar::s_oaPolar.size())
		{
			if(pXDirect->m_bPolarView) pXDirect->createPolarCurves();
			else                       pXDirect->createOppCurves();
		}
		UpdateFoilListBox();
		updateView();
	}
	else if(m_iApp==XFLR5::MIAREX)
	{
		updatePlaneListBox();
		pMiarex->setPlane();
		pMiarex->m_bArcball = false;
		pMiarex->m_bIs2DScaleSet = false;
		pMiarex->setControls();
		updateView();
	}
	else if(m_iApp==XFLR5::DIRECTDESIGN)
	{
		QAFoil *pAFoil = (QAFoil*)m_pAFoil;
		pAFoil->SetAFoilParams();
		pAFoil->SelectFoil(Foil::curFoil());
		updateView();
	}
	else if(m_iApp==XFLR5::INVERSEDESIGN)
	{
		OnXInverse();
		updateView();
	}
}


void MainFrame::OnLogFile()
{
	QString FileName = QDir::tempPath() + "/XFLR5.log";
	// 20090605 Francesco Meschia
	QDesktopServices::openUrl(QUrl::fromLocalFile(FileName));
}




void MainFrame::OnNewProject()
{
	QMiarex *pMiarex = (QMiarex*)m_pMiarex;
	pMiarex->m_bArcball = false;
	if(!s_bSaved)
	{
		int resp = QMessageBox::question(this, tr("Question"), tr("Save the current project ?"),
										  QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);

		if (QMessageBox::Cancel == resp)
		{
			return;
		}
		else if (QMessageBox::Yes == resp)
		{
			if(SaveProject(m_FileName))
			{
				deleteProject();
				statusBar()->showMessage(tr("The project ") + s_ProjectName + tr(" has been saved"));
			}
			else return; //save failed, don't close
		}
		else if (QMessageBox::No == resp)
		{
			deleteProject();
		}
	}
	else
	{
		deleteProject();
	}

	updateView();
}


void MainFrame::OnOpenGLInfo()
{
    QSurfaceFormat fmt = m_p3dWidget->format();
	QString strongProps;


	strongProps += QString("OpenGl::majorVersion      = %1\n").arg(fmt.majorVersion());
	strongProps += QString("OpenGl::minorVersion      = %1\n").arg(fmt.minorVersion());


	if(fmt.stereo())            strongProps += QString("OpenGl::stereo            = enabled\n");
	else                        strongProps += QString("OpenGl::stereo            = disabled\n");


	if(fmt.swapInterval()==-1)  strongProps += QString("OpenGl::swapInterval      = supported\n");
	else                        strongProps += QString("OpenGl::swapInterval      = not supported\n");

	strongProps += QString("OpenGl::depthBufferSize   = %1\n").arg(fmt.depthBufferSize());
	strongProps += QString("OpenGl::samplesPerPixel   = %1\n").arg(fmt.samples());


	if(fmt.profile() == QSurfaceFormat::NoProfile)                strongProps += QString("Opengl::CompatibilityProfile::NoProfile");
	else if(fmt.profile() == QSurfaceFormat::CoreProfile)         strongProps += QString("Opengl::CompatibilityProfile::CoreProfile");
	else if(fmt.profile()== QSurfaceFormat::CompatibilityProfile) strongProps += QString("Opengl::CompatibilityProfile::CompatibilityProfile");

	ObjectPropsDlg dlg(this);
	dlg.InitDialog(tr("Support for OpenGL provided by your system:"), strongProps);
	dlg.exec();
}



void MainFrame::OnResetSettings()
{
	int resp = QMessageBox::question(this, tr("Default Settings"), tr("Are you sure you want to reset the default settings ?"),
									 QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);
	if(resp == QMessageBox::Yes)
	{
		QMessageBox::warning(this,tr("Default Settings"), tr("The settings will be reset at the next session"));
#ifdef Q_OS_MAC
        QSettings settings(QSettings::NativeFormat,QSettings::UserScope,"sourceforge.net","xflr5");
#else
        QSettings settings(QSettings::IniFormat,QSettings::UserScope,"XFLR5");
#endif
		settings.clear();
		Settings::s_LastDirName = QDir::homePath();
		// do not save on exit
		m_bSaveSettings = false;
	}
}


void MainFrame::OnRestoreToolbars()
{
	if(m_iApp==XFLR5::XFOILANALYSIS)
	{
		m_pctrlXInverseToolBar->hide();
		m_pctrlAFoilToolBar->hide();
		m_pctrlMiarexToolBar->hide();
		m_pctrlStabViewWidget->hide();
		m_pctrl3DScalesWidget->hide();

		m_pctrlAFoilWidget->hide();
		m_pctrlXInverseWidget->hide();
		m_pctrlMiarexWidget->hide();

		m_pctrlXDirectToolBar->show();
		m_pctrlXDirectWidget->show();
	}
	else if(m_iApp==XFLR5::DIRECTDESIGN)
	{
		m_pctrlXInverseToolBar->hide();
		m_pctrlMiarexToolBar->hide();
		m_pctrlXDirectToolBar->hide();
		m_pctrl3DScalesWidget->hide();

		m_pctrlXDirectWidget->hide();
		m_pctrlXInverseWidget->hide();
		m_pctrlMiarexWidget->hide();
		m_pctrlStabViewWidget->hide();

		m_pctrlAFoilToolBar->show();
		m_pctrlAFoilWidget->show();
	}
	else if(m_iApp==XFLR5::INVERSEDESIGN)
	{
		m_pctrlAFoilToolBar->hide();
		m_pctrlMiarexToolBar->hide();
		m_pctrlXDirectToolBar->hide();
		m_pctrl3DScalesWidget->hide();
		m_pctrlStabViewWidget->hide();

		m_pctrlAFoilWidget->hide();
		m_pctrlXDirectWidget->hide();
		m_pctrlMiarexWidget->hide();

		m_pctrlXInverseToolBar->show();
		m_pctrlXInverseWidget->show();
	}
	else if(m_iApp==XFLR5::MIAREX)
	{
		m_pctrlXInverseToolBar->hide();
		m_pctrlAFoilToolBar->hide();
		m_pctrlXDirectToolBar->hide();
		m_pctrl3DScalesWidget->hide();

		m_pctrlAFoilWidget->hide();
		m_pctrlXDirectWidget->hide();
		m_pctrlXInverseWidget->hide();
		m_pctrlMiarexWidget->show();
		m_pctrlMiarexToolBar->show();
//		QMiarex *pMiarex = (QMiarex*)m_pMiarex;
//		if(pMiarex->m_iView==WSTABVIEW) m_pctrlStabViewWidget->show();
//		else                            m_pctrlStabViewWidget->hide();
	}
}


void MainFrame::onSaveOptions()
{
    SaveOptionsDlg soDlg(this);
    soDlg.InitDialog(m_bSaveOpps, m_bSaveWOpps);
    if(soDlg.exec()==QDialog::Accepted)
	{
        m_bSaveOpps  = soDlg.m_bOpps;
        m_bSaveWOpps = soDlg.m_bWOpps;
	}
}


void MainFrame::onSaveProject()
{
	if (!s_ProjectName.length() || s_ProjectName=="*")
	{
		onSaveProjectAs();
		return;
	}
	if(SaveProject(m_FileName))
	{
		AddRecentFile(m_FileName);
		statusBar()->showMessage(tr("The project ") + s_ProjectName + tr(" has been saved"));
	}
	QMiarex *pMiarex = (QMiarex*)m_pMiarex;
	pMiarex->m_bArcball = false;
	pMiarex->updateView();
}



bool MainFrame::onSaveProjectAs()
{
	if(SaveProject())
	{
		SetProjectName(m_FileName);
		AddRecentFile(m_FileName);
		statusBar()->showMessage(tr("The project ") + s_ProjectName + tr(" has been saved"));
	}
	QMiarex *pMiarex = (QMiarex*)m_pMiarex;
	pMiarex->m_bArcball = false;
	return true;
}




void MainFrame::onSaveViewToImageFile()
{
	QMiarex *pMiarex = (QMiarex*)m_pMiarex;
	pMiarex->m_bArcball = false;

	QSize sz(m_p2dWidget->geometry().width(), m_p2dWidget->geometry().height());
	QImage img(sz, QImage::Format_RGB32);
	QPainter painter(&img);
	QString FileName, Filter;

	switch(m_ImageFormat)
	{
		case XFLR5::PNG :
		{
			Filter = "Portable Network Graphics (*.png)";
			break;
		}
		case XFLR5::JPEG :
		{
			Filter = "JPEG (*.jpg)";
			break;
		}
		case XFLR5::BMP :
		{
			Filter = "Windows Bitmap (*.bmp)";
			break;
		}
	}

	FileName = QFileDialog::getSaveFileName(this, tr("Save Image"),
											m_ImageDirName,
											"Portable Network Graphics (*.png);;JPEG (*.jpg);;Windows Bitmap (*.bmp)",
											&Filter);

	if(!FileName.length()) return;

	int pos = FileName.lastIndexOf("/");
	if(pos>0) m_ImageDirName = FileName.left(pos);

	if(Filter == "Portable Network Graphics (*.png)")
	{
		if(FileName.right(4)!=".png") FileName+= ".png";
		m_ImageFormat = XFLR5::PNG;
	}
	else if(Filter == "JPEG (*.jpg)")
	{
		if(FileName.right(4)!=".jpg") FileName+= ".jpg";
		m_ImageFormat = XFLR5::JPEG;
	}
	else if(Filter == "Windows Bitmap (*.bmp)")
	{
		if(FileName.right(4)!=".bmp") FileName+= ".bmp";
		m_ImageFormat = XFLR5::BMP;
	}

	switch(m_iApp)
	{
		case XFLR5::XFOILANALYSIS:
		{
			QPixmap pix = m_pXDirectTileWidget->grab();
			pix.save(FileName, "PNG");
			return;
		}
		case XFLR5::DIRECTDESIGN:
		{
			QPixmap pix = m_pDirect2dWidget->grab();
			pix.save(FileName, "PNG");
			return;
		}
		case XFLR5::INVERSEDESIGN:
		{
			QPixmap pix = m_p2dWidget->grab();
			pix.save(FileName, "PNG");
			return;
		}
		case XFLR5::MIAREX:
		{
			QMiarex *pMiarex = (QMiarex*)m_pMiarex;
			pMiarex->m_bArcball = false;

			if(pMiarex->m_iView==XFLR5::W3DVIEW)
			{
//				m_p3dWidget->grabFrameBuffer().save(FileName);
				return;
			}
			else
			{
				QPixmap pix = m_pMiarexTileWidget->grab();
				pix.save(FileName, "PNG");
				return;
			}
		}
		default:
			break;
	}
	img.save(FileName);
}


void MainFrame::OnSelChangePlane(int sel)
{
	// Gets the new selected wing name and notifies Miarex
	// then updates WPolar combobox
	// and selects either the current WPolar
	// or the first one in the list, if any
	QMiarex *pMiarex = (QMiarex*)m_pMiarex;
	pMiarex->stopAnimate();
	QString strong;
//	int sel = m_pctrlPlane->currentIndex();
	if (sel >=0) strong = m_pctrlPlane->itemText(sel);
	pMiarex->setPlane(strong);
	pMiarex->updateView();
}


void MainFrame::OnSelChangeWPolar(int sel)
{
	QMiarex *pMiarex = (QMiarex*)m_pMiarex;
	pMiarex->stopAnimate();

	QString strong;
//	int sel = m_pctrlPlanePolar->currentIndex();
	if (sel>=0) strong = m_pctrlPlanePolar->itemText(sel);
	m_iApp = XFLR5::MIAREX;
	pMiarex->setWPolar(false, strong);
	pMiarex->setControls();
	pMiarex->updateView();
}


void MainFrame::OnSelChangePlaneOpp(int sel)
{
	QString strong;
	QMiarex *pMiarex = (QMiarex*)m_pMiarex;
	pMiarex->stopAnimate();

	// Gets the new selected WOpp name and notifies Miarex
	if(!m_pctrlPlaneOpp->count())
	{
		if (pMiarex->m_iView==XFLR5::WOPPVIEW)    pMiarex->CreateWOppCurves();
		else if(pMiarex->m_iView==XFLR5::WCPVIEW) pMiarex->createCpCurves();
		pMiarex->updateView();
		return;
	}

	if (sel>=0) strong = m_pctrlPlaneOpp->itemText(sel);
	else
	{
		m_pctrlPlaneOpp->setCurrentIndex(0);
		strong = m_pctrlPlaneOpp->itemText(0).trimmed();
	}
	if(strong.length())
	{
		bool bOK;
		double x = locale().toDouble(strong, &bOK);
		if(bOK)
		{
			m_iApp = XFLR5::MIAREX;
			pMiarex->setPlaneOpp(false, x);
			pMiarex->updateView();
		}
		else
		{
			QMessageBox::warning(window(), tr("Warning"), tr("Unidentified Operating Point"));
			pMiarex->setPlaneOpp(true);
		}
	}
}


void MainFrame::OnSelChangeFoil(int sel)
{
	// Gets the new selected foil name and notifies XDirect
	// sets a polar, if any
	// then updates the polar combobox

	QXDirect *pXDirect = (QXDirect*)m_pXDirect;
	pXDirect->m_bAnimate = false;

	QString strong;
//	int sel = m_pctrlFoil->currentIndex();
	if (sel >=0) strong = m_pctrlFoil->itemText(sel);

	Foil::setCurFoil(Foil::foil(strong));
	pXDirect->setFoil(Foil::curFoil());
	pXDirect->setPolar();
	m_iApp = XFLR5::XFOILANALYSIS;
	UpdatePolarListBox();
	pXDirect->setControls();
	updateView();
}


void MainFrame::OnSelChangePolar(int sel)
{
	QXDirect *pXDirect = (QXDirect*)m_pXDirect;
	pXDirect->m_bAnimate = false;

	// Gets the new selected polar name and notifies XDirect
	QString strong;
//	int selNew = m_pctrlPolar->currentIndex();
	if (sel>=0) strong = m_pctrlPolar->itemText(sel);
	m_iApp = XFLR5::XFOILANALYSIS;


	pXDirect->setPolar(Polar::getPolar(Foil::curFoil(), strong));
	UpdateOppListBox();
	pXDirect->setControls();
	updateView();
}


void MainFrame::OnSelChangeOpp(int sel)
{
	QXDirect *pXDirect = (QXDirect*)m_pXDirect;
	pXDirect->m_bAnimate = false;

	// Gets the new selected Opp name and notifies XDirect
	QString strong;

	if (sel>=0) strong = m_pctrlOpPoint->itemText(sel);
	m_iApp = XFLR5::XFOILANALYSIS;

	double Alpha;
	bool bOK;
	Alpha = locale().toDouble(strong, &bOK);

	if(bOK)
	{
		pXDirect->setOpp(Alpha);
	}
	else
	{
		QMessageBox::warning(window(), tr("Warning"), tr("Unidentified Operating Point"));
		OpPoint::setCurOpp(NULL);
		pXDirect->setOpp();
	}
	updateView();
}



void MainFrame::OnStyleSettings()
{
	QXDirect *pXDirect   = (QXDirect*)m_pXDirect;
	QMiarex *pMiarex     = (QMiarex*)m_pMiarex;

	Settings DSdlg(this);

	DSdlg.initDialog();

	DSdlg.exec();
	pMiarex->m_bResetglGeom = true;
	pMiarex->m_bResetglBody = true;
	pMiarex->m_bResetglLegend = true;

	if(DSdlg.m_bIsGraphModified)
	{
		setGraphSettings(&Settings::s_RefGraph);
	}
	QAFoil *pAFoil = (QAFoil*)m_pAFoil;
	pAFoil->SetTableFont();

	pXDirect->m_CpGraph.setInverted(true);
	pMiarex->m_CpGraph.setInverted(true);
	updateView();
}


void MainFrame::onUnits()
{
	Units uDlg(this);
	uDlg.initDialog();

	if(uDlg.exec()==QDialog::Accepted)
	{
		setSaveState(false);

		if(m_iApp==XFLR5::MIAREX)
		{
			QMiarex *pMiarex= (QMiarex*)m_pMiarex;
			pMiarex->updateUnits();
		}
	}
}


void MainFrame::onXDirect()
{
	QMiarex *pMiarex = (QMiarex*)m_pMiarex;
	pMiarex->stopAnimate();

	QXDirect *pXDirect = (QXDirect*)m_pXDirect;
	m_iApp = XFLR5::XFOILANALYSIS;

	m_pctrlMiarexToolBar->hide();
	m_pctrlStabViewWidget->hide();
	m_pctrlAFoilToolBar->hide();
	m_pctrlXInverseToolBar->hide();
	m_pctrlXDirectToolBar->show();

	m_pctrlAFoilWidget->hide();
	m_pctrlMiarexWidget->hide();
	m_pctrlXInverseWidget->hide();
	m_pctrlXDirectWidget->show();

	pXDirect->setFoil();
	UpdateFoilListBox();
	setMainFrameCentralWidget();
	setMenus();
	checkGraphActions();
	pXDirect->setControls();
	pXDirect->updateView();
}


void MainFrame::onMiarex()
{
	QXDirect *pXDirect = (QXDirect*)m_pXDirect;
	pXDirect->stopAnimate();
	QMiarex *pMiarex = (QMiarex*)m_pMiarex;	m_iApp = XFLR5::MIAREX;

	m_pctrlXDirectToolBar->hide();
	m_pctrlXInverseToolBar->hide();
	m_pctrlAFoilToolBar->hide();
	m_pctrlMiarexToolBar->show();

	m_pctrlXDirectWidget->hide();
	m_pctrlAFoilWidget->hide();
	m_pctrlXInverseWidget->hide();
	m_pctrlMiarexWidget->show();

	updatePlaneListBox();
	pMiarex->setPlane();
	pMiarex->m_bArcball = false;

	setMenus();
	setMainFrameCentralWidget();
	checkGraphActions();
	pMiarex->setControls();
	pMiarex->setCurveParams();
	updateView();
}

void MainFrame::OnXInverse()
{
	QXDirect *pXDirect = (QXDirect*)m_pXDirect;
	pXDirect->stopAnimate();
	QMiarex *pMiarex = (QMiarex*)m_pMiarex;
	pMiarex->stopAnimate();

	QXInverse *pXInverse = (QXInverse*)m_pXInverse;
//	pXInverse->SetScale();
	m_iApp = XFLR5::INVERSEDESIGN;

	m_pctrlMiarexToolBar->hide();
	m_pctrlStabViewWidget->hide();
	m_pctrlAFoilToolBar->hide();
	m_pctrlXDirectToolBar->hide();
	m_pctrlXInverseToolBar->show();

	m_pctrlAFoilWidget->hide();
	m_pctrlMiarexWidget->hide();
	m_pctrlXDirectWidget->hide();
	m_pctrlXInverseWidget->show();

	setMainFrameCentralWidget();
	setMenus();
	checkGraphActions();
	pXInverse->SetParams();
	pXInverse->updateView();
}


void MainFrame::OnXInverseMixed()
{
	QXDirect *pXDirect = (QXDirect*)m_pXDirect;
	pXDirect->stopAnimate();
	QMiarex *pMiarex = (QMiarex*)m_pMiarex;
	pMiarex->stopAnimate();

	QXInverse *pXInverse = (QXInverse*)m_pXInverse;
//	pXInverse->SetScale();
	m_iApp = XFLR5::INVERSEDESIGN;
	m_pctrlMiarexToolBar->hide();
	m_pctrlStabViewWidget->hide();
	m_pctrlAFoilToolBar->hide();
	m_pctrlXDirectToolBar->hide();
	m_pctrlXInverseToolBar->show();

	m_pctrlAFoilWidget->hide();
	m_pctrlMiarexWidget->hide();
	m_pctrlXDirectWidget->hide();
	m_pctrlXInverseWidget->show();
	pXInverse->m_bFullInverse = false;
	setMainFrameCentralWidget();
	setMenus();
	checkGraphActions();
	pXInverse->SetParams();
	pXInverse->updateView();
}


void MainFrame::openRecentFile()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action) return;

	QXDirect *pXDirect = (QXDirect*) m_pXDirect;
	
	XFLR5::enumApp App = LoadXFLR5File(action->data().toString());
	if(m_iApp==XFLR5::NOAPP) m_iApp = App;

	if(App==XFLR5::NOAPP)
	{
		m_iApp = App;
		QString FileName = action->data().toString();
		m_RecentFiles.removeAll(FileName);
		updateRecentFileActions();
	}

	else if(m_iApp==XFLR5::XFOILANALYSIS)
	{
		if(Polar::s_oaPolar.size())
		{
			if(pXDirect->m_bPolarView) pXDirect->createPolarCurves();
			else                       pXDirect->createOppCurves();
		}
		onXDirect();
		UpdateFoilListBox();
		updateView();
	}
	else if(m_iApp==XFLR5::MIAREX)
	{
		onMiarex();
	}
	else if(m_iApp==XFLR5::DIRECTDESIGN)
	{
		QAFoil *pAFoil = (QAFoil*)m_pAFoil;
		pAFoil->SetAFoilParams();
		updateView();
	}
	else if(m_iApp==XFLR5::INVERSEDESIGN)
	{
		OnXInverse();
		updateView();
	}
}


void *MainFrame::ReadFoilFile(QTextStream &in)
{
	QString Strong;
	QString tempStr;
	QString FoilName;

	Foil* pFoil = NULL;
	int pos, res, i, ip;
	pos = 0;
	double x,y, z,area;
	bool bRead;


	pFoil = new Foil();
	if(!pFoil)	return NULL;

	while(tempStr.length()==0 && !in.atEnd())
	{
		FoilName = Strong;
		Strong = in.readLine();
		pos = Strong.indexOf("#",0);
		// ignore everything after # (including #)
		if(pos>0)Strong.truncate(pos);
		tempStr = Strong;
		tempStr.remove(" ");
	}

	if(!in.atEnd())
	{
		// FoilName contains the last comment
		ReadValues(Strong, res,x,y,z);
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
	do
	{
		Strong = in.readLine();
		pos = Strong.indexOf("#",0);
		// ignore everything after # (including #)
		if(pos>0)Strong.truncate(pos);
		tempStr = Strong;
		tempStr.remove(" ");
		if (!Strong.isNull() && bRead && tempStr.length())
		{
			ReadValues(Strong, res, x,y,z);
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
	}while (bRead && !Strong.isNull());

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
	pFoil->n = pFoil->nb;

	pFoil->m_FoilColor = GetColor(0);
	pFoil->InitFoil();

	return pFoil;
}


/**
 *Reads a Foil and its related Polar objects from a binary stream associated to a .plr file.
 * @param ar the binary stream
 * @return the pointer to the Foil object which has been created, or NULL if failure.
 */
void MainFrame::ReadPolarFile(QDataStream &ar)
{
	Foil* pFoil = NULL;
	Polar *pPolar = NULL;
	Polar * pOldPolar;
	int i, n, l;

	ar >> n;

	if(n<100000)
	{
		//old format
		QMessageBox::warning(window(), tr("Warning"), tr("Obsolete format, cannot read"));
		return;
	}
	else if (n >=100000)
	{
		//new format XFLR5 v1.99+
		//first read all available foils
		ar>>n;
		for (i=0;i<n; i++)
		{
			pFoil = new Foil();
			if (!pFoil->Serialize(ar, false))
			{
				delete pFoil;
				return;
			}
			pFoil->insertThisFoil();
		}

		//next read all available polars

		ar>>n;
		for (i=0;i<n; i++)
		{
			pPolar = new Polar();

			if (!pPolar->Serialize(ar, false))
			{
				delete pPolar;
				return;
			}
			for (l=0; l<Polar::s_oaPolar.size(); l++)
			{
				pOldPolar = (Polar*)Polar::s_oaPolar[l];
				if (pOldPolar->foilName() == pPolar->foilName() &&
					pOldPolar->polarName()  == pPolar->polarName())
				{
					//just overwrite...
					Polar::s_oaPolar.removeAt(l);
					delete pOldPolar;
					//... and continue to add
				}
			}
			Polar::addPolar(pPolar);
		}
	}
}



bool MainFrame::SaveProject(QString PathName)
{
	QString Filter = "XFLR5 v6 Project File (*.xfl)";
	QString FileName = s_ProjectName;

	if(!PathName.length())
	{
		if(FileName.right(1)=="*") 	FileName = FileName.left(FileName.length()-1);

        PathName = QFileDialog::getSaveFileName(this, tr("Save the Project File"),
												Settings::s_LastDirName+"/"+FileName,
												"XFLR5 v6 Project File (*.xfl)",
												&Filter);

		if(!PathName.length()) return false;//nothing more to do

		int pos = PathName.indexOf(".xfl", Qt::CaseInsensitive);
		if(pos<0) PathName += ".xfl";

        PathName.replace(QDir::separator(), "/"); // Qt sometimes uses the windows \ separator

        pos = PathName.lastIndexOf("/");
		if(pos>0) Settings::s_LastDirName = PathName.left(pos);
	}

	QFile fp(PathName);

	if (!fp.open(QIODevice::WriteOnly))
	{
		QMessageBox::warning(window(), tr("Warning"), tr("Could not open the file for writing"));
		return false;
	}

	QDataStream ar(&fp);
	SerializeProjectXFL(ar,true);

	m_FileName = PathName;
	fp.close();

	SaveSettings();

	setSaveState(true);

	return true;
}



void MainFrame::OnSavePlaneAsProject()
{
	QMiarex *pMiarex = (QMiarex*)m_pMiarex;
	pMiarex->m_bArcball = false;

	QString strong;
	if(pMiarex->m_pCurPlane) strong = pMiarex->m_pCurPlane->planeName();
	else
	{
		QMessageBox::warning(this, tr("Warning"), tr("Nothing to save"));
		return ;
	}

	QString PathName;
	QString Filter = "XFLR5 v6 Project File (*.xfl)";
	QString FileName = strong;

	PathName = QFileDialog::getSaveFileName(this, tr("Save the Project File"),
											Settings::s_LastDirName+"/"+FileName,
											"XFLR5 v6 Project File (*.xfl)",
											&Filter);

	if(!PathName.length()) return;//nothing more to do
	int pos = PathName.indexOf(".xfl", Qt::CaseInsensitive);
	if(pos<0) PathName += ".xfl";
	PathName.replace(QDir::separator(), "/"); // Qt sometimes uses the windows \ separator
	pos = PathName.lastIndexOf("/");
	if(pos>0) Settings::s_LastDirName = PathName.left(pos);


	QFile fp(PathName);

	if (!fp.open(QIODevice::WriteOnly))
	{
		QMessageBox::warning(window(), tr("Warning"), tr("Could not open the file for writing"));
		return;
	}

	QDataStream ar(&fp);

	SerializePlaneProject(ar);
	fp.close();
}



bool MainFrame::SerializePlaneProject(QDataStream &ar)
{
	QMiarex * pMiarex = (QMiarex*)m_pMiarex;
	if(!pMiarex->m_pCurPlane)
	{
		QMessageBox::warning(this, tr("Warning"), tr("Nothing to save"));
		return false;
	}

	QAFoil *pAFoil = (QAFoil*)m_pAFoil;

	WPolar *pWPolar = NULL;
	Polar *pPolar   = NULL;
	Foil *pFoil     = NULL;

	QString PlaneName = pMiarex->m_pCurPlane->planeName();

	bool bIsStoring = true;
	int i, iSize;

	int ArchiveFormat = 200001;
	ar << ArchiveFormat;
	// 200001 : First instance of new ".xfl" format

	//Save unit data
	ar << Units::lengthUnitIndex();
	ar << Units::areaUnitIndex();
	ar << Units::weightUnitIndex();
	ar << Units::s_SpeedUnit;
	ar << Units::s_ForceUnit;
	ar << Units::s_MomentUnit;

	//Save default Polar data. Not in the Settings, since this is Project dependant
	if(WPolarDlg::s_WPolar.m_WPolarType==XFLR5::FIXEDSPEEDPOLAR)      ar<<1;
	else if(WPolarDlg::s_WPolar.m_WPolarType==XFLR5::FIXEDLIFTPOLAR)  ar<<2;
	else if(WPolarDlg::s_WPolar.m_WPolarType==XFLR5::FIXEDAOAPOLAR)   ar<<4;
	else if(WPolarDlg::s_WPolar.m_WPolarType==XFLR5::BETAPOLAR)       ar<<5;
	else if(WPolarDlg::s_WPolar.m_WPolarType==XFLR5::STABILITYPOLAR)  ar<<7;
	else ar << 0;

	if(WPolarDlg::s_WPolar.m_AnalysisMethod==XFLR5::LLTMETHOD)        ar << 1;
	else if(WPolarDlg::s_WPolar.m_AnalysisMethod==XFLR5::VLMMETHOD)   ar << 2;
	else if(WPolarDlg::s_WPolar.m_AnalysisMethod==XFLR5::PANELMETHOD) ar << 3;
	else ar << 0;

	ar << WPolarDlg::s_WPolar.m_Mass;
	ar << WPolarDlg::s_WPolar.m_QInfSpec;
	ar << WPolarDlg::s_WPolar.m_CoG.x;
	ar << WPolarDlg::s_WPolar.m_CoG.y;
	ar << WPolarDlg::s_WPolar.m_CoG.z;

	ar << WPolarDlg::s_WPolar.m_Density;
	ar << WPolarDlg::s_WPolar.m_Viscosity;
	ar << WPolarDlg::s_WPolar.m_AlphaSpec;
	ar << WPolarDlg::s_WPolar.m_BetaSpec;

	ar << WPolarDlg::s_WPolar.m_bTiltedGeom;
	ar << WPolarDlg::s_WPolar.m_bWakeRollUp;

	// save the plane
	ar << 1;
	pMiarex->m_pCurPlane->SerializePlaneXFL(ar, bIsStoring);

	// save the WPolars associated to this plane
	//count the polars
	iSize = 0;
	for (i=0; i<Objects3D::s_oaWPolar.size();i++)
	{
		pWPolar = (WPolar*)Objects3D::s_oaWPolar.at(i);
		if(pWPolar->planeName()==PlaneName) iSize++;
	}
	ar << iSize;
	for (i=0; i<Objects3D::s_oaWPolar.size();i++)
	{
		pWPolar = (WPolar*)Objects3D::s_oaWPolar.at(i);
		if(pWPolar->planeName()==PlaneName) pWPolar->SerializeWPlrXFL(ar, bIsStoring);
	}

	ar << 0; //no need to save the operating points

	// then the foils
	// list the foils associated to this Plane's wings
	QList<Foil*> foilList;
	for(i=0; i<Foil::s_oaFoil.size(); i++)
	{
		pFoil = (Foil*)Foil::s_oaFoil.at(i);
		for(int iw=0; iw<MAXWINGS; iw++)
		{
			if(pMiarex->m_pCurPlane->m_Wing[iw].isWingFoil(pFoil))
			{
				foilList.append(pFoil);
				break;
			}
		}
	}

	ar << foilList.count();
	for(int iFoil=0; iFoil<foilList.size(); iFoil++)
	{
		foilList.at(iFoil)->SerializeFoilXFL(ar, bIsStoring);
	}

	// the foil polars
	// list the foil polars associated to this Plane's wings
	QList<Polar*> polarList;
	for(i=0; i<Polar::s_oaPolar.size(); i++)
	{
		pPolar = (Polar*)Polar::s_oaPolar.at(i);
		for(int iFoil=0; iFoil<foilList.count(); iFoil++)
		{
			if(pPolar->m_FoilName == foilList.at(iFoil)->foilName())
			{
				bool bListed = false;
				for(int ip=0; ip<polarList.count(); ip++)
				{
					if(pPolar==polarList.at(ip))
					{
						bListed = true;
						break; // polar list
					}
				}
				if(!bListed)
				{
					polarList.append(pPolar);
					break;  //foil list
				}
			}
		}
	}

	ar << polarList.size();
	for (int ip=0; ip<polarList.size();ip++)
	{
		polarList.at(ip)->SerializePolarXFL(ar, true);
	}

	ar << 0; //no need to save the operating points

	// and the spline foil whilst we're at it
	pAFoil->m_pSF->Serialize(ar, bIsStoring);

	return true;
}


void MainFrame::SaveSettings()
{
	QAFoil *pAFoil = (QAFoil*)m_pAFoil;
	QMiarex *pMiarex = (QMiarex*)m_pMiarex;
	QXDirect *pXDirect = (QXDirect*)m_pXDirect;
	QXInverse *pXInverse = (QXInverse*)m_pXInverse;

	if(!m_bSaveSettings) return;
#ifdef Q_OS_MAC
        QSettings settings(QSettings::NativeFormat,QSettings::UserScope,"sourceforge.net","xflr5");
#else
	QSettings settings(QSettings::IniFormat,QSettings::UserScope,"XFLR5");
#endif
	settings.beginGroup("MainFrame");
	{
		settings.setValue("SettingsFormat", SETTINGSFORMAT);
		settings.setValue("FrameGeometryx", frameGeometry().x());
		settings.setValue("FrameGeometryy", frameGeometry().y());
		settings.setValue("SizeWidth", size().width());
		settings.setValue("SizeHeight", size().height());
		settings.setValue("SizeMaximized", isMaximized());
		settings.setValue("StyleName", Settings::s_StyleName);

		if (Settings::s_ExportFileType==XFLR5::TXT) settings.setValue("ExportFileType", 0);
		else                                        settings.setValue("ExportFileType", 1);

		settings.setValue("GraphExportFilter", m_GraphExportFilter);
		settings.setValue("Miarex_Float", m_pctrlMiarexWidget->isFloating());
		settings.setValue("XDirect_Float", m_pctrlXDirectWidget->isFloating());
		settings.setValue("AFoil_Float", m_pctrlAFoilWidget->isFloating());
		settings.setValue("XInverse_Float", m_pctrlXInverseWidget->isFloating());
		settings.setValue("StabView_Float", m_pctrlStabViewWidget->isFloating());
		settings.setValue("Miarex_x", m_pctrlMiarexWidget->frameGeometry().x());
		settings.setValue("Miarex_y", m_pctrlMiarexWidget->frameGeometry().y());
		settings.setValue("XDirect_x", m_pctrlXDirectWidget->frameGeometry().x());
		settings.setValue("XDirect_y", m_pctrlXDirectWidget->frameGeometry().y());
		settings.setValue("AFoil_x", m_pctrlAFoilWidget->frameGeometry().x());
		settings.setValue("AFoil_y", m_pctrlAFoilWidget->frameGeometry().y());
		settings.setValue("XInverse_x", m_pctrlXInverseWidget->frameGeometry().x());
		settings.setValue("XInverse_y", m_pctrlXInverseWidget->frameGeometry().y());
		settings.setValue("StabView_x", m_pctrlStabViewWidget->frameGeometry().x());
		settings.setValue("StabView_y", m_pctrlStabViewWidget->frameGeometry().y());

		settings.setValue("XDirectSize", m_pctrlXDirectWidget->size());
		settings.setValue("AFoilSize", m_pctrlAFoilWidget->size());
		settings.setValue("XInverseSize", m_pctrlXInverseWidget->size());
		settings.setValue("MiarexSize", m_pctrlMiarexWidget->size());
		settings.setValue("StabSize", m_pctrlStabViewWidget->size());

		settings.setValue("ImageDirName", m_ImageDirName);
		settings.setValue("ExportLastDirName", m_ExportLastDirName);

		settings.setValue("LengthUnit",  Units::lengthUnitIndex());
		settings.setValue("AreaUnit",    Units::areaUnitIndex());
		settings.setValue("WeightUnit",  Units::weightUnitIndex());
		settings.setValue("SpeedUnit",   Units::speedUnitIndex());
		settings.setValue("ForceUnit",   Units::forceUnitIndex());
		settings.setValue("MomentUnit",  Units::momentUnitIndex());

		settings.setValue("LanguageFilePath", s_LanguageFilePath);
		settings.setValue("ImageFormat", m_ImageFormat);
		settings.setValue("SaveOpps", m_bSaveOpps);
		settings.setValue("SaveWOpps", m_bSaveWOpps);
		settings.setValue("RecentFileSize", m_RecentFiles.size());
		settings.setValue("ShowStyleSheets", Settings::s_bStyleSheets);
		settings.setValue("StyleSheetName", Settings::s_StyleSheetName);


		QString RecentF;
		for(int i=0; i<m_RecentFiles.size() && i<MAXRECENTFILES; i++)
		{
			RecentF = QString("RecentFile_%1").arg(i);
			if(m_RecentFiles[i].length()) settings.setValue(RecentF, m_RecentFiles.at(i));
			else                          settings.setValue(RecentF, "");
		}
		for(int i=m_RecentFiles.size(); i<MAXRECENTFILES; i++)
		{
			RecentF = QString("RecentFile_%1").arg(i);
			settings.setValue(RecentF, "");
		}
	}
	settings.endGroup();


	Settings::SaveSettings(&settings);
	pAFoil->SaveSettings(&settings);
	pXDirect->saveSettings(&settings);
	pMiarex->saveSettings(&settings);
	pXInverse->SaveSettings(&settings);
	GL3DScales::SaveSettings(&settings);
	W3dPrefsDlg::SaveSettings(&settings);
}


void MainFrame::setMainFrameCentralWidget()
{

	if(m_iApp==XFLR5::MIAREX)
	{
		QMiarex *pMiarex = (QMiarex*)m_pMiarex;
		if(pMiarex->m_iView==XFLR5::WOPPVIEW || pMiarex->m_iView==XFLR5::WPOLARVIEW || pMiarex->m_iView==XFLR5::STABPOLARVIEW  || pMiarex->m_iView==XFLR5::STABTIMEVIEW)
		{
			m_pctrlCentralWidget->setCurrentWidget(m_pMiarexTileWidget);
			pMiarex->setGraphTiles();
			m_pMiarexTileWidget->setFocus();
		}
		else if(pMiarex->m_iView==XFLR5::W3DVIEW)
		{
			m_pctrlCentralWidget->setCurrentWidget(m_p3dWidget);
			m_p3dWidget->setFocus();
		}
	}
	else if(m_iApp==XFLR5::DIRECTDESIGN)
	{
		m_pctrlCentralWidget->setCurrentWidget(m_pDirect2dWidget);
		m_pDirect2dWidget->setFocus();
	}
	else if(m_iApp==XFLR5::XFOILANALYSIS)
	{
		QXDirect *pXDirect = (QXDirect*)m_pXDirect;
		m_pctrlCentralWidget->setCurrentWidget(m_pXDirectTileWidget);
		pXDirect->setGraphTiles();
		m_pXDirectTileWidget->setFocus();
	}
	else if(m_iApp==XFLR5::INVERSEDESIGN)
	{
		m_pctrlCentralWidget->setCurrentWidget(m_p2dWidget);
		m_p2dWidget->setFocus();
	}
}


void MainFrame::SelectFoil(void*pFoilPtr)
{
	Foil *pFoil = (Foil*)pFoilPtr;
	if(!m_pctrlFoil->count()) return;

	m_pctrlFoil->blockSignals(true);

	if (pFoil)
	{
		int pos = m_pctrlFoil->findText(pFoil->foilName());

		if (pos>=0) m_pctrlFoil->setCurrentIndex(pos);
		else        m_pctrlFoil->setCurrentIndex(0);
	}
	else
	{
		m_pctrlFoil->setCurrentIndex(0);
	}
	m_pctrlFoil->blockSignals(false);
}




void MainFrame::SelectPolar(void*pPolarPtr)
{
	Polar *pPolar = (Polar*)pPolarPtr;
	if(!m_pctrlPolar->count()) return;

	m_pctrlPolar->blockSignals(true);

	if (pPolar)
	{
		int pos = m_pctrlPolar->findText(pPolar->polarName());

		if (pos>=0) m_pctrlPolar->setCurrentIndex(pos);
		else        m_pctrlPolar->setCurrentIndex(0);
	}
	else
	{
		m_pctrlPolar->setCurrentIndex(0);
	}

	m_pctrlPolar->blockSignals(false);
}



/**
 *Selects the operating point in the combobox and returns true
 *On error, selects the first and returns false
 */
void MainFrame::SelectOpPoint(void *pOppPtr)
{
	Polar *pCurPlr = Polar::curPolar();
	OpPoint *pOpp = (OpPoint*)pOppPtr;
	if(!pOpp || !pCurPlr) return;
	if(!m_pctrlOpPoint->count()) return;

	QString strange;
	bool bOK;

	m_pctrlOpPoint->blockSignals(true);

	for(int i=0; i<m_pctrlOpPoint->count(); i++)
	{
		strange = m_pctrlOpPoint->itemText(i).trimmed();

		double val = locale().toDouble(strange, &bOK);

		if(pCurPlr->polarType() == XFLR5::FIXEDAOAPOLAR)
		{
			if(bOK && qAbs(val-pOpp->Reynolds)<1.0)
			{
				m_pctrlOpPoint->setCurrentIndex(i);
				break;
			}
		}
		else
		{
			if(bOK && qAbs(val-pOpp->Alpha)<0.001)
			{
				m_pctrlOpPoint->setCurrentIndex(i);
				break;
			}
		}
	}
    m_pctrlOpPoint->blockSignals(false);
}


void MainFrame::SelectPlane(void *pPlanePtr)
{
	if(!m_pctrlPlane->count()) return;
	m_pctrlPlane->blockSignals(true);

	Plane *pPlane = (Plane*)pPlanePtr;

	if(pPlane)
	{
		int pos = m_pctrlPlane->findText(pPlane->planeName());
		if(pos>=0) m_pctrlPlane->setCurrentIndex(pos);
		else if(m_pctrlPlane->count())
			m_pctrlPlane->setCurrentIndex(0);
	}
	else
	{
		m_pctrlPlane->setCurrentIndex(0);
	}

	m_pctrlPlane->blockSignals(false);
}


void MainFrame::SelectWPolar(void *pWPolarPtr)
{
	if(!m_pctrlPlanePolar->count()) return;

	WPolar *pWPolar = (WPolar*)pWPolarPtr;

	m_pctrlPlanePolar->blockSignals(true);

	if(pWPolarPtr)
	{
		int pos = m_pctrlPlanePolar->findText(pWPolar->polarName());
		if (pos>=0) m_pctrlPlanePolar->setCurrentIndex(pos);
		else if(m_pctrlPlanePolar->count())
			m_pctrlPlanePolar->setCurrentIndex(0);
	}
	else
	{
		m_pctrlPlanePolar->setCurrentIndex(0);
	}
	m_pctrlPlanePolar->blockSignals(false);
}


void MainFrame::SelectPlaneOpp(void *pPlaneOppPtr)
{
	double x = 0.0;
	PlaneOpp *pPlaneOpp = (PlaneOpp*)pPlaneOppPtr;

	if(pPlaneOpp)
	{
		if(pPlaneOpp->m_WPolarType<XFLR5::FIXEDAOAPOLAR)        x = pPlaneOpp->m_Alpha;
		else if(pPlaneOpp->m_WPolarType==XFLR5::FIXEDAOAPOLAR)  x = pPlaneOpp->m_QInf;
		else if(pPlaneOpp->m_WPolarType==XFLR5::BETAPOLAR)      x = pPlaneOpp->m_Beta;
		else if(pPlaneOpp->m_WPolarType==XFLR5::STABILITYPOLAR) x = pPlaneOpp->m_Ctrl;
	}
	//Selects a pOpp in the combobox
	QMiarex *pMiarex = (QMiarex*)m_pMiarex;
	WPolar *pCurWPlr    = pMiarex->m_pCurWPolar;
	double val;
	bool bOK;
	QString strange;

	m_pctrlPlaneOpp->blockSignals(true);
//	x = (double)qRound(x*100.0)/100.0;

	for(int i=0; i<m_pctrlPlaneOpp->count(); i++)
	{
		strange = m_pctrlPlaneOpp->itemText(i);
		val = locale().toDouble(strange, &bOK);

		if(pCurWPlr->m_WPolarType<XFLR5::FIXEDAOAPOLAR)
		{
			if(bOK && qAbs(val-x)<0.001)
			{
				m_pctrlPlaneOpp->setCurrentIndex(i);
				break;
			}
		}
		else if(pCurWPlr->m_WPolarType==XFLR5::FIXEDAOAPOLAR)
		{
			if(bOK && qAbs(val-x)<1.0)
			{
				m_pctrlPlaneOpp->setCurrentIndex(i);
				break;
			}
		}
		else if(pCurWPlr->m_WPolarType==XFLR5::BETAPOLAR)
		{
			if(bOK && qAbs(val-x)<0.001)
			{
				m_pctrlPlaneOpp->setCurrentIndex(i);
				break;
			}
		}
		else if(pCurWPlr->m_WPolarType==XFLR5::STABILITYPOLAR)
		{
			if(bOK && qAbs(val-x)<.001)
			{
				m_pctrlPlaneOpp->setCurrentIndex(i);
				break;
			}
		}
	}
	m_pctrlPlaneOpp->blockSignals(false);
}





bool MainFrame::SerializeProjectXFL(QDataStream &ar, bool bIsStoring)
{
	QAFoil *pAFoil = (QAFoil*)m_pAFoil;

	WPolar *pWPolar = NULL;
	PlaneOpp *pPOpp = NULL;
	Plane *pPlane   = NULL;
	Polar *pPolar   = NULL;
	Foil *pFoil     = NULL;
	OpPoint *pOpp    = NULL;

	int i, n;

	if (bIsStoring)
	{
		// storing code
		int ArchiveFormat = 200001;
		ar << ArchiveFormat;
		// 200001 : First instance of new ".xfl" format

		//Save unit data
		ar << Units::lengthUnitIndex();
		ar << Units::areaUnitIndex();
		ar << Units::weightUnitIndex();
		ar << Units::s_SpeedUnit;
		ar << Units::s_ForceUnit;
		ar << Units::s_MomentUnit;


		//Save default Polar data. Not in the Settings, since this is Project dependant
		if(WPolarDlg::s_WPolar.m_WPolarType==XFLR5::FIXEDSPEEDPOLAR)      ar<<1;
		else if(WPolarDlg::s_WPolar.m_WPolarType==XFLR5::FIXEDLIFTPOLAR)  ar<<2;
		else if(WPolarDlg::s_WPolar.m_WPolarType==XFLR5::FIXEDAOAPOLAR)   ar<<4;
		else if(WPolarDlg::s_WPolar.m_WPolarType==XFLR5::BETAPOLAR)       ar<<5;
		else if(WPolarDlg::s_WPolar.m_WPolarType==XFLR5::STABILITYPOLAR)  ar<<7;
		else ar << 0;

		if(WPolarDlg::s_WPolar.m_AnalysisMethod==XFLR5::LLTMETHOD)        ar << 1;
		else if(WPolarDlg::s_WPolar.m_AnalysisMethod==XFLR5::VLMMETHOD)   ar << 2;
		else if(WPolarDlg::s_WPolar.m_AnalysisMethod==XFLR5::PANELMETHOD) ar << 3;
		else ar << 0;

		ar << WPolarDlg::s_WPolar.m_Mass;
		ar << WPolarDlg::s_WPolar.m_QInfSpec;
		ar << WPolarDlg::s_WPolar.m_CoG.x;
		ar << WPolarDlg::s_WPolar.m_CoG.y;
		ar << WPolarDlg::s_WPolar.m_CoG.z;

		ar << WPolarDlg::s_WPolar.m_Density;
		ar << WPolarDlg::s_WPolar.m_Viscosity;
		ar << WPolarDlg::s_WPolar.m_AlphaSpec;
		ar << WPolarDlg::s_WPolar.m_BetaSpec;

		ar << WPolarDlg::s_WPolar.m_bTiltedGeom;
		ar << WPolarDlg::s_WPolar.m_bWakeRollUp;

		// save the planes...
		ar << Objects3D::s_oaPlane.size();
		for (i=0; i<Objects3D::s_oaPlane.size();i++)
		{
			pPlane = (Plane*)Objects3D::s_oaPlane.at(i);
			pPlane->SerializePlaneXFL(ar, bIsStoring);
		}

		// save the WPolars
		ar << Objects3D::s_oaWPolar.size();
		for (i=0; i<Objects3D::s_oaWPolar.size();i++)
		{
			pWPolar = (WPolar*)Objects3D::s_oaWPolar.at(i);
			pWPolar->SerializeWPlrXFL(ar, bIsStoring);
		}

		if(m_bSaveWOpps)
		{
			// not forgetting their POpps
			ar << Objects3D::s_oaPOpp.size();
			for (i=0; i<Objects3D::s_oaPOpp.size();i++)
			{
				pPOpp = (PlaneOpp*)Objects3D::s_oaPOpp.at(i);
				pPOpp->SerializePOppXFL(ar, bIsStoring);
			}
		}
		else ar << 0;

		// then the foils
		ar << Foil::s_oaFoil.size();
		for(i=0; i<Foil::s_oaFoil.size(); i++)
		{
			pFoil = (Foil*)Foil::s_oaFoil.at(i);
			pFoil->SerializeFoilXFL(ar, bIsStoring);
		}

		//the foil polars
		ar << Polar::s_oaPolar.size();
		for (i=0; i<Polar::s_oaPolar.size();i++)
		{
			pPolar = (Polar*)Polar::s_oaPolar.at(i);
			pPolar->SerializePolarXFL(ar, bIsStoring);
		}

		//the oppoints
		if(m_bSaveOpps)
		{
			ar << OpPoint::s_oaOpp.size();
			for (i=0; i<OpPoint::s_oaOpp.size();i++)
			{
				pOpp = (OpPoint*)OpPoint::s_oaOpp.at(i);
				pOpp->SerializeOppXFL(ar, bIsStoring);
			}
		}
		else ar << 0;

		// and the spline foil whilst we're at it
		pAFoil->m_pSF->Serialize(ar, bIsStoring);

		//add provisions
		// space allocation for the future storage of more data, without need to change the format
		for (int i=0; i<20; i++) ar << i;
		for (int i=0; i<50; i++) ar << (double)i;
	}
	else
	{
		// LOADING CODE

		int ArchiveFormat;
		ar >> ArchiveFormat;
		if(ArchiveFormat!=200001) return false;

		//Load unit data
		ar >> Units::s_LengthUnit;
		ar >> Units::s_AreaUnit;
		ar >> Units::s_WeightUnit;
		ar >> Units::s_SpeedUnit;
		ar >> Units::s_ForceUnit;
		ar >> Units::s_MomentUnit;


		Units::SetUnitConversionFactors();


		//Load the default Polar data. Not in the Settings, since this is Project dependant
		ar >> n;
		if(n==1)      WPolarDlg::s_WPolar.m_WPolarType=XFLR5::FIXEDSPEEDPOLAR;
		else if(n==2) WPolarDlg::s_WPolar.m_WPolarType=XFLR5::FIXEDLIFTPOLAR;
		else if(n==4) WPolarDlg::s_WPolar.m_WPolarType=XFLR5::FIXEDAOAPOLAR;
		else if(n==5) WPolarDlg::s_WPolar.m_WPolarType=XFLR5::BETAPOLAR;
		else if(n==7) WPolarDlg::s_WPolar.m_WPolarType=XFLR5::STABILITYPOLAR;

		ar >> n;
		if(n==1)      WPolarDlg::s_WPolar.m_AnalysisMethod=XFLR5::LLTMETHOD;
		else if(n==2) WPolarDlg::s_WPolar.m_AnalysisMethod=XFLR5::VLMMETHOD;
		else if(n==3) WPolarDlg::s_WPolar.m_AnalysisMethod=XFLR5::PANELMETHOD;

		ar >> WPolarDlg::s_WPolar.m_Mass;
		ar >> WPolarDlg::s_WPolar.m_QInfSpec;
		ar >> WPolarDlg::s_WPolar.m_CoG.x;
		ar >> WPolarDlg::s_WPolar.m_CoG.y;
		ar >> WPolarDlg::s_WPolar.m_CoG.z;

		ar >> WPolarDlg::s_WPolar.m_Density;
		ar >> WPolarDlg::s_WPolar.m_Viscosity;
		ar >> WPolarDlg::s_WPolar.m_AlphaSpec;
		ar >> WPolarDlg::s_WPolar.m_BetaSpec;

		ar >> WPolarDlg::s_WPolar.m_bTiltedGeom;
		ar >> WPolarDlg::s_WPolar.m_bWakeRollUp;

		// load the planes...
		// assumes all object have been deleted and the array cleared.
		ar >> n;
		for(i=0; i<n; i++)
		{
			pPlane = new Plane();
			if(pPlane->SerializePlaneXFL(ar, bIsStoring)) Objects3D::s_oaPlane.append(pPlane);
			else
			{
				QMessageBox::warning(this,tr("Warning"), tr("Error reading the file")+"\n"+tr("Saved the valid part"));
				return false;
			}
		}

		// load the WPolars
		ar >> n;
		for(i=0; i<n; i++)
		{
			pWPolar = new WPolar();
			if(pWPolar->SerializeWPlrXFL(ar, bIsStoring))
			{
				// clean up : the project may be carrying useless WPolars due to past programming errors
				pPlane = Objects3D::getPlane(pWPolar->planeName());
				if(pPlane) Objects3D::s_oaWPolar.append(pWPolar);
				else
				{
				}
			}
			else
			{
				QMessageBox::warning(this,tr("Warning"), tr("Error reading the file")+"\n"+tr("Saved the valid part"));
				return false;
			}
		}

		// the PlaneOpps
		ar >> n;
		for(i=0; i<n; i++)
		{
			pPOpp = new PlaneOpp();
			if(pPOpp->SerializePOppXFL(ar, bIsStoring))
			{
				//just append, since POpps have been sorted when first inserted
				pPlane = Objects3D::getPlane(pPOpp->planeName());
				pWPolar = Objects3D::getWPolar(pPlane, pPOpp->polarName());

				// clean up : the project may be carrying useless PlaneOpps due to past programming errors
				if(pPlane && pWPolar) Objects3D::s_oaPOpp.append(pPOpp);
				else
				{
				}
			}
			else
			{
				QMessageBox::warning(this,tr("Warning"), tr("Error reading the file")+"\n"+tr("Saved the valid part"));
				return false;
			}
		}

		// load the Foils
		ar >> n;
		for(i=0; i<n; i++)
		{
			pFoil = new Foil();
			if(pFoil->SerializeFoilXFL(ar, bIsStoring))
			{
				// delete any former foil with that name - necessary in the case of project insertion to avoid duplication
				// there is a risk that old plane results are not consisent with the new foil, but difficult to avoid that
				Foil *pOldFoil = Foil::foil(pFoil->m_FoilName);
				if(pOldFoil) Foil::deleteFoil(pOldFoil);
				Foil::s_oaFoil.append(pFoil);
			}
			else
			{
				QMessageBox::warning(this,tr("Warning"), tr("Error reading the file")+"\n"+tr("Saved the valid part"));
				return false;
			}
		}

		// load the Polars
		ar >> n;
		for(i=0; i<n; i++)
		{
			pPolar = new Polar();
			if(pPolar->SerializePolarXFL(ar, bIsStoring)) Polar::s_oaPolar.append(pPolar);
			else
			{
				QMessageBox::warning(this,tr("Warning"), tr("Error reading the file")+"\n"+tr("Saved the valid part"));
				return false;
			}
		}

		// OpPoints
		ar >> n;
		for(i=0; i<n; i++)
		{
			pOpp = new OpPoint();
			if(pOpp->SerializeOppXFL(ar, bIsStoring))  OpPoint::s_oaOpp.append(pOpp);
			else
			{
				QMessageBox::warning(this,tr("Warning"), tr("Error reading the file")+"\n"+tr("Saved the valid part"));
				return false;
			}
		}


		// and the spline foil whilst we're at it
		pAFoil->m_pSF->Serialize(ar, bIsStoring);

		// space allocation
/*		int k ;
		double dble;
		for (int i=0; i<20; i++) ar >> k;
		for (int i=0; i<50; i++) ar >> dble;*/
	}

	return true;
}



bool MainFrame::SerializeProjectWPA(QDataStream &ar, bool bIsStoring)
{
	QMiarex *pMiarex = (QMiarex*)m_pMiarex;
	QAFoil *pAFoil = (QAFoil*)m_pAFoil;

	Wing *pWing     = NULL;
	WPolar *pWPolar = NULL;
	WingOpp *pWOpp  = NULL;
	PlaneOpp *pPOpp = NULL;
	Plane *pPlane   = NULL;
	Body *pBody     = NULL;
	Polar *pPolar   = NULL;
	Foil *pFoil     = NULL;

	QString str;
	int i, n, j, k;
	float f;

	if (bIsStoring)
	{
		//not storing to .wpa format anymore
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
			ar >> Units::s_LengthUnit;
			ar >> Units::s_AreaUnit;
			ar >> Units::s_WeightUnit;
			ar >> Units::s_SpeedUnit;
			ar >> Units::s_ForceUnit;

			if(ArchiveFormat>=100005)
			{
				ar >> Units::s_MomentUnit;
			}

			Units::SetUnitConversionFactors();

			if(ArchiveFormat>=100004)
			{
				ar >>k;
				if(k==1)      WPolarDlg::s_WPolar.m_WPolarType = XFLR5::FIXEDSPEEDPOLAR;
				else if(k==2) WPolarDlg::s_WPolar.m_WPolarType = XFLR5::FIXEDLIFTPOLAR;
				else if(k==4) WPolarDlg::s_WPolar.m_WPolarType = XFLR5::FIXEDAOAPOLAR;
				else if(k==5) WPolarDlg::s_WPolar.m_WPolarType = XFLR5::BETAPOLAR;
				else if(k==7) WPolarDlg::s_WPolar.m_WPolarType = XFLR5::STABILITYPOLAR;

				ar >> f; WPolarDlg::s_WPolar.m_Mass=f;
				ar >> f; WPolarDlg::s_WPolar.m_QInfSpec=f;
				if(ArchiveFormat>=100013)
				{
					ar >> f; WPolarDlg::s_WPolar.m_CoG.x=f;
					ar >> f; WPolarDlg::s_WPolar.m_CoG.y=f;
					ar >> f; WPolarDlg::s_WPolar.m_CoG.z=f;
				}
				else
				{
					ar >> f; WPolarDlg::s_WPolar.m_CoG.x=f;
					WPolarDlg::s_WPolar.m_CoG.y=0;
					WPolarDlg::s_WPolar.m_CoG.z=0;
				}
				if(ArchiveFormat<100010) WPolarDlg::s_WPolar.m_CoG.x=f/1000.0;
				ar >> f; WPolarDlg::s_WPolar.m_Density   = f;
				ar >> f; WPolarDlg::s_WPolar.m_Viscosity = f;
				ar >> f; WPolarDlg::s_WPolar.m_AlphaSpec     = f;
				if(ArchiveFormat>=100012)
				{
					ar >>f; WPolarDlg::s_WPolar.m_BetaSpec=f;
				}

				ar >> k;
				if(k==1)      WPolarDlg::s_WPolar.m_AnalysisMethod=XFLR5::LLTMETHOD;
				else if(k==2) WPolarDlg::s_WPolar.m_AnalysisMethod=XFLR5::VLMMETHOD;
				else if(k==3) WPolarDlg::s_WPolar.m_AnalysisMethod=XFLR5::PANELMETHOD;
			}
			if(ArchiveFormat>=100006)
			{
				ar >> k;
				if (k) WPolarDlg::s_WPolar.m_bVLM1 = true;
				else   WPolarDlg::s_WPolar.m_bVLM1 = false;

				ar >> k;
			}

			if(ArchiveFormat>=100008)
			{
				ar >> k;
				if (k) WPolarDlg::s_WPolar.m_bTiltedGeom = true;
				else   WPolarDlg::s_WPolar.m_bTiltedGeom = false;

				ar >> k;
				if (k) WPolarDlg::s_WPolar.m_bWakeRollUp = true;
				else   WPolarDlg::s_WPolar.m_bWakeRollUp = false;
			}
			// and read n again
			ar >> n;
		}

		// WINGS FIRST
		for (i=0;i<n; i++)
		{
			pWing = new Wing;

			if (!pWing->SerializeWingWPA(ar, bIsStoring))
			{
				if(pWing) delete pWing;
				return false;
			}
			if(pWing)
			{
				//create a plane with this wing
				pPlane = new Plane();
				pPlane->setPlaneName(pWing->wingName());
				pPlane->m_Wing[0].Duplicate(pWing);
				pPlane->setBody(NULL);
				pPlane->setWings(false, false, false);
				Objects3D::addPlane(pPlane);
				delete pWing;
			}
		}

		//THEN WPOLARS

		ar >> n;// number of WPolars to load
		bool bWPolarOK;

		for (i=0;i<n; i++)
		{
			pWPolar = new WPolar;
			bWPolarOK = pWPolar->SerializeWPlrWPA(ar, bIsStoring);
			//force compatibilty
			if(pWPolar->analysisMethod()==XFLR5::PANELMETHOD && pWPolar->m_WPolarType==XFLR5::STABILITYPOLAR) pWPolar->m_bThinSurfaces = true;

			if (!bWPolarOK)
			{
				if(pWPolar) delete pWPolar;
				return false;
			}
			if(!pWPolar->analysisMethod()==XFLR5::LLTMETHOD && ArchiveFormat <100003)	pWPolar->ClearData();//former VLM version was flawed
//			if(pWPolar->m_WPolarType==STABILITYPOLAR)	pWPolar->bThinSurfaces() = true;

			if(pWPolar->m_PolarFormat!=1020 || pWPolar->m_WPolarType!=XFLR5::STABILITYPOLAR) Objects3D::addWPolar(pWPolar);
		}

		//THEN WOPPS
		ar >> n;// number of WOpps to load
		bool bWOppOK;
		for (i=0;i<n; i++)
		{
			pWOpp = new WingOpp();
			if (ArchiveFormat<=100001)
			{
				return false;
			}
			else
			{
				bWOppOK = pWOpp->SerializeWingOppWPA(ar, bIsStoring);
				if (!bWOppOK)
				{
					if(pWOpp) delete pWOpp;
					return false;
				}
			}

			delete pWOpp;
		}

//		ar >> n;
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
			for (j=0; j<Polar::s_oaPolar.size(); j++)
			{
				pPolar = (Polar*)Polar::s_oaPolar.at(j);
				for (k=0; k<Foil::s_oaFoil.size(); k++)
				{
					pFoil = (Foil*)Foil::s_oaFoil.at(k);
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
				pBody = new Body();

				if (pBody->SerializeBodyWPA(ar, bIsStoring))
				{
					Objects3D::s_oaBody.append(pBody);

				}
				else
				{
					if(pBody) delete pBody;
					return false;
				}
			}
		}
		if(ArchiveFormat>=100006)
		{ //read the planes
			ar >> n;
			for (i=0; i<n;i++)
			{
				pPlane = new Plane();
				if(pPlane)
				{
					if(pPlane->SerializePlane(ar, bIsStoring))
					{
						Objects3D::addPlane(pPlane);
					}
					else
					{
						if(pPlane) delete pPlane;
						return false;
					}
				}
			}

			// attach the body pointers to the Plane objects
			for (int ib=0; ib<Objects3D::s_oaBody.size(); ib++)
			{
				Body *pBody = (Body*)Objects3D::s_oaBody.at(ib);
				Objects3D::addBody(pBody);
			}


			//and their pPolars
			if(ArchiveFormat <100007)
			{
				ar >> n;// number of WPolars to load
				for (i=0;i<n; i++)
				{
					pWPolar = new WPolar();

					if (!pWPolar->SerializeWPlrWPA(ar, bIsStoring))
					{
						if(pWPolar) delete pWPolar;
						return false;
					}
					if(!pWPolar->analysisMethod()==XFLR5::LLTMETHOD && ArchiveFormat <100003)
						pWPolar->ClearData();
					Objects3D::addWPolar(pWPolar);
				}
			}

			ar >> n;// number of PlaneOpps to load
			for (i=0;i<n; i++)
			{
				pPOpp = new PlaneOpp();

				if (!pPOpp->SerializePOppWPA(ar, bIsStoring))
				{
					if(pPOpp) delete pPOpp;
					return false;
				}
				Objects3D::InsertPOpp(pPOpp);
//				Objects3D::s_oaPOpp.append(pPOpp);
			}
		}
		pMiarex->m_pCurPOpp = NULL;

		pAFoil->m_pSF->Serialize(ar, bIsStoring);

		if(m_iApp==XFLR5::MIAREX) pMiarex->setPlane();

		pMiarex->updateUnits();

		return true;
	}
}



void MainFrame::setMenus()
{
	if(m_iApp==XFLR5::NOAPP)
	{
		menuBar()->clear();
		menuBar()->addMenu(m_pFileMenu);
		menuBar()->addMenu(m_pOptionsMenu);
		menuBar()->addMenu(m_pHelpMenu);
	}
	else if(m_iApp==XFLR5::XFOILANALYSIS)
	{
		menuBar()->clear();
		menuBar()->addMenu(m_pFileMenu);
		menuBar()->addMenu(m_pXDirectViewMenu);
		menuBar()->addMenu(m_pXDirectFoilMenu);
		menuBar()->addMenu(m_pDesignMenu);
		menuBar()->addMenu(m_pXFoilAnalysisMenu);
		menuBar()->addMenu(m_pPolarMenu);
		menuBar()->addMenu(m_pOpPointMenu);
		menuBar()->addMenu(m_pGraphMenu);
		menuBar()->addMenu(m_pOptionsMenu);
		menuBar()->addMenu(m_pHelpMenu);
	}
	else if(m_iApp==XFLR5::INVERSEDESIGN)
	{
		menuBar()->clear();
		menuBar()->addMenu(m_pFileMenu);
		menuBar()->addMenu(m_pXInverseViewMenu);
		menuBar()->addMenu(m_pXInverseGraphMenu);
		menuBar()->addMenu(m_pXInverseFoilMenu);
		menuBar()->addMenu(m_pOptionsMenu);
		menuBar()->addMenu(m_pHelpMenu);
	}
	else if(m_iApp==XFLR5::DIRECTDESIGN)
	{
		menuBar()->clear();
		menuBar()->addMenu(m_pFileMenu);
		menuBar()->addMenu(m_pAFoilViewMenu);
		menuBar()->addMenu(m_pAFoilDesignMenu);
		menuBar()->addMenu(m_pAFoilSplineMenu);
		menuBar()->addMenu(m_pOptionsMenu);
		menuBar()->addMenu(m_pHelpMenu);
	}
	else if(m_iApp== XFLR5::MIAREX)
	{
		menuBar()->clear();
		menuBar()->addMenu(m_pFileMenu);
		menuBar()->addMenu(m_pMiarexViewMenu);
		menuBar()->addMenu(m_pPlaneMenu);
		menuBar()->addMenu(m_pMiarexWPlrMenu);
		menuBar()->addMenu(m_pMiarexWOppMenu);
		menuBar()->addMenu(m_pMiarexAnalysisMenu);
		menuBar()->addMenu(m_pGraphMenu);
		menuBar()->addMenu(m_pOptionsMenu);
		menuBar()->addMenu(m_pHelpMenu);
	}
}


void MainFrame::SetProjectName(QString PathName)
{
	m_FileName = PathName;
	int pos = PathName.lastIndexOf("/");
	if (pos>0) s_ProjectName = PathName.right(PathName.length()-pos-1);
	else       s_ProjectName = PathName;

	if(s_ProjectName.length()>4)
	{
		s_ProjectName = s_ProjectName.left(s_ProjectName.length()-4);
		m_pctrlProjectName->setText(s_ProjectName);
	}
}


void MainFrame::setSaveState(bool bSave)
{
	s_bSaved = bSave;

	int len = s_ProjectName.length();
	if(s_ProjectName.right(1)=="*") s_ProjectName = s_ProjectName.left(len-1);
	if (!bSave)
	{
		s_ProjectName += "*";
	}
	m_pctrlProjectName->setText(s_ProjectName);
}



void MainFrame::setGraphSettings(QGraph *pGraph)
{
	QXDirect *pXDirect   = (QXDirect*)m_pXDirect;
	QMiarex *pMiarex     = (QMiarex*)m_pMiarex;
	QXInverse *pXInverse = (QXInverse*)m_pXInverse;

	pXDirect->m_CpGraph.copySettings(pGraph, false);
	for(int ig=0; ig<qMax(MAXPOLARGRAPHS,pXDirect->m_PlrGraph.count()); ig++) pXDirect->m_PlrGraph[ig]->copySettings(pGraph, false);

	pXInverse->m_QGraph.copySettings(pGraph, false);

	pMiarex->m_CpGraph.copySettings(pGraph, false);

	for(int ig=0; ig<pMiarex->m_WingGraph.count(); ig++) pMiarex->m_WingGraph[ig]->copySettings(pGraph, false);
	for(int ig=0; ig<pMiarex->m_WPlrGraph.count(); ig++) pMiarex->m_WPlrGraph[ig]->copySettings(pGraph, false);
	for(int ig=0; ig<pMiarex->m_TimeGraph.count(); ig++) pMiarex->m_TimeGraph[ig]->copySettings(pGraph, false);
	for(int ig=0; ig<pMiarex->m_StabPlrGraph.count(); ig++) pMiarex->m_StabPlrGraph[ig]->copySettings(pGraph, false);

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

/**
* Fills the combobox with  plane names.
* then selects the current wing or plane, if any;
* else selects the first, if any;
* else disables the combobox.
*/
void MainFrame::updatePlaneListBox()
{
	int i;
	QMiarex *pMiarex= (QMiarex*)m_pMiarex;
	Plane *pPlane, *pCurPlane;

	m_pctrlPlane->blockSignals(true);
	m_pctrlPlane->clear();

	pCurPlane = pMiarex->m_pCurPlane;
	QStringList PlaneNames;
	for (i=0; i<Objects3D::s_oaPlane.size(); i++)
	{
		pPlane = (Plane*)Objects3D::s_oaPlane[i];
		PlaneNames.append(pPlane->planeName());
	}

#if QT_VERSION >= 0x050000
	PlaneNames.sort(Qt::CaseInsensitive);
#else
	PlaneNames.sort();
#endif

	m_pctrlPlane->addItems(PlaneNames);

	//select the current Plane, if any...
	if(pCurPlane) SelectPlane(pCurPlane);

	m_pctrlPlane->setEnabled(m_pctrlPlane->count());
	m_pctrlPlane->blockSignals(false);

	UpdateWPolarListBox();
}



void MainFrame::UpdateWPolarListBox()
{
//	fills the combobox with WPolar names associated to Miarex's current wing
//	then selects Miarex current WPolar if any, else selects the first, if any
//	else disables the combobox
	QMiarex * pMiarex = (QMiarex*)m_pMiarex;
	WPolar *pWPolar;
	QString PlaneName;
	int i;

	m_pctrlPlanePolar->blockSignals(true);
	m_pctrlPlanePolar->clear();

	Plane  *pCurPlane = pMiarex->m_pCurPlane;
	WPolar *pCurWPlr  = pMiarex->m_pCurWPolar;

	if(pCurPlane)      PlaneName = pCurPlane->planeName();
	else               PlaneName = "";

	if(!PlaneName.length())
	{
		pMiarex->m_pCurWPolar = NULL;
		m_pctrlPlanePolar->setEnabled(false);
		m_pctrlPlaneOpp->setEnabled(false);
		return;
	}

	for (i=0; i<Objects3D::s_oaWPolar.size(); i++)
	{
		pWPolar = (WPolar*)Objects3D::s_oaWPolar[i];
		if(pWPolar->m_PlaneName == PlaneName)
		{
			m_pctrlPlanePolar->addItem(pWPolar->polarName());
		}
	}

    m_pctrlPlanePolar->setEnabled(m_pctrlPlanePolar->count());

	if(pCurWPlr) SelectWPolar(pCurWPlr);

	m_pctrlPlanePolar->blockSignals(false);
    UpdatePOppListBox();
}


/**
 * Fills the combobox with the WOpp parameters associated to Miarex' current WPLr,
 * then selects the current WingOpp or PlaneOpp if any, else selects the first, if any,
 * else disables the combobox.
 */
void MainFrame::UpdatePOppListBox()
{
	m_pctrlPlaneOpp->blockSignals(true);
	QMiarex * pMiarex = (QMiarex*)m_pMiarex;

	PlaneOpp *pPOpp;
	Plane  *pCurPlane   = pMiarex->m_pCurPlane;
	WPolar *pCurWPlr    = pMiarex->m_pCurWPolar;

	QString str;
	m_pctrlPlaneOpp->clear();


	if (!pCurPlane || !pCurPlane->planeName().length() || !pCurWPlr || !pCurWPlr->m_WPlrName.length())
	{
		m_pctrlPlaneOpp->setEnabled(false);
		return;
	}

	for (int iPOpp=0; iPOpp<Objects3D::s_oaPOpp.size(); iPOpp++)
	{
		pPOpp = (PlaneOpp*)Objects3D::s_oaPOpp.at(iPOpp);
		if (pPOpp->planeName()==pCurPlane->planeName() && pPOpp->polarName()==pCurWPlr->polarName())
		{
			if(pCurWPlr->polarType()<XFLR5::FIXEDAOAPOLAR)        str = QString("%L1").arg(pPOpp->m_Alpha,8,'f',3);
			else if(pCurWPlr->polarType()==XFLR5::FIXEDAOAPOLAR)  str = QString("%L1").arg(pPOpp->m_QInf,8,'f',3);
			else if(pCurWPlr->polarType()==XFLR5::BETAPOLAR)      str = QString("%L1").arg(pPOpp->m_Beta,8,'f',3);
			else if(pCurWPlr->polarType()==XFLR5::STABILITYPOLAR) str = QString("%L1").arg(pPOpp->m_Ctrl,8,'f',3);
			m_pctrlPlaneOpp->addItem(str);
		}
	}

	if(pMiarex->m_pCurPOpp) SelectPlaneOpp(pMiarex->m_pCurPOpp);
	else                    m_pctrlPlaneOpp->setCurrentIndex(0);

	// otherwise disable control
	m_pctrlPlaneOpp->setEnabled(m_pctrlPlaneOpp->count());

	m_pctrlPlaneOpp->blockSignals(false);
}


/**
 * Fills the combobox with the Foil names,
 * then selects the current Foil if any, else selects the first, if any,
 * else disables the combobox
 */
void MainFrame::UpdateFoilListBox()
{
	m_pctrlFoil->blockSignals(true);
	m_pctrlFoil->clear();

	QStringList foilList;
	for (int iFoil=0; iFoil<Foil::s_oaFoil.size(); iFoil++)
	{
		Foil *pFoil = (Foil*)Foil::s_oaFoil[iFoil];
		foilList.append(pFoil->m_FoilName);
	}

#if QT_VERSION >= 0x050000
	foilList.sort(Qt::CaseInsensitive);
#else
	foilList.sort();
#endif

	m_pctrlFoil->addItems(foilList);
	m_pctrlFoil->setEnabled(m_pctrlFoil->count());
	SelectFoil(Foil::curFoil());

	m_pctrlFoil->blockSignals(false);

	UpdatePolarListBox();
}


/**
 * Fills the combobox with polar names associated to the current foil,
 * then selects XDirect current polar if any, else selects the first, if any,
 * else disables the combobox,
 */
void MainFrame::UpdatePolarListBox()
{
	m_pctrlPolar->blockSignals(true);
	m_pctrlPolar->clear();

	if(!Foil::curFoil())
	{
	}
	else
	{
		QStringList polarList;
		for (int i=0; i<Polar::s_oaPolar.size(); i++)
		{
			Polar *pPolar = (Polar*)Polar::s_oaPolar[i];
			if(pPolar->m_FoilName == Foil::curFoil()->foilName())
			{
				polarList.append(pPolar->polarName());
			}
		}

#if QT_VERSION >= 0x050000
		polarList.sort(Qt::CaseInsensitive);
#else
		polarList.sort();
#endif

		m_pctrlPolar->addItems(polarList);
	}

	m_pctrlPolar->setEnabled(m_pctrlPolar->count());
	SelectPolar(Polar::curPolar());

	m_pctrlPolar->blockSignals(false);

	UpdateOppListBox();
}


/**
 * Fills the combobox with the OpPoint values associated to the current foil,
 * then selects the current OpPoint if any, else selects the first, if any,
 * else disables the combobox.
 */
void MainFrame::UpdateOppListBox()
{
	m_pctrlOpPoint->blockSignals(true);
	m_pctrlOpPoint->clear();

	Polar *pCurPlr    = Polar::curPolar();

	if (!Foil::curFoil()  || !pCurPlr)
	{
		m_pctrlOpPoint->clear();
		m_pctrlOpPoint->setEnabled(false);
		m_pctrlOpPoint->blockSignals(false);
	}
	else
	{
		QStringList oppList;
		QString str;
		for (int iOpp=0; iOpp<OpPoint::s_oaOpp.size(); iOpp++)
		{
			OpPoint *pOpp = (OpPoint*)OpPoint::s_oaOpp[iOpp];
			if (pOpp->foilName()==Foil::curFoil()->foilName() && pOpp->polarName()==pCurPlr->polarName())
			{
				if (pCurPlr->m_PolarType != XFLR5::FIXEDAOAPOLAR)
					str = QString("%L1").arg(pOpp->Alpha,8,'f',3);
				else
					str = QString("%L1").arg(pOpp->Reynolds,8,'f',0);
				oppList.append(str);
			}
		}
/*
#if QT_VERSION >= 0x050000
		oppList.sort(Qt::CaseInsensitive);
#else
		oppList.sort();
#endif
*/
		m_pctrlOpPoint->addItems(oppList);
	}

	m_pctrlOpPoint->setEnabled(m_pctrlOpPoint->count());
	SelectOpPoint(OpPoint::curOpp());

	m_pctrlOpPoint->blockSignals(false);
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



void MainFrame::updateView()
{
	switch(m_iApp)
	{
		case XFLR5::XFOILANALYSIS:
		{
			QXDirect *pXDirect = (QXDirect*)m_pXDirect;
			pXDirect->updateView();
			break;
		}
		case XFLR5::DIRECTDESIGN:
		{
			m_pDirect2dWidget->update();
			break;
		}
		case XFLR5::MIAREX:
		{
			QMiarex *pMiarex= (QMiarex*)m_pMiarex;
			pMiarex->updateView();
			break;
		}
		case XFLR5::INVERSEDESIGN:
		{
			QXInverse *pXInverse = (QXInverse*)m_pXInverse;
			pXInverse->updateView();
			break;
		}
		default:
			break;
	}
}


void MainFrame::WritePolars(QDataStream &ar, void *pFoilPtr)
{
	Foil *pFoil = (Foil*)pFoilPtr;
	int i;
	if(!pFoil)
	{
		int ArchiveFormat = 100003;
		ar << ArchiveFormat;
		//100003 : added foil comment
		//100002 : means we are serializings opps in the new numbered format
		//100001 : transferred NCrit, XTopTr, XBotTr to polar file
		//first write foils
		ar << Foil::s_oaFoil.size();

		for (i=0; i<Foil::s_oaFoil.size(); i++)
		{
			pFoil = (Foil*)Foil::s_oaFoil.at(i);
			pFoil->Serialize(ar, true);
		}

		//then write polars
		ar << Polar::s_oaPolar.size();
		Polar * pPolar ;
		for (i=0; i<Polar::s_oaPolar.size();i++)
		{
			pPolar = (Polar*)Polar::s_oaPolar.at(i);
			pPolar->Serialize(ar, true);
		}
	}
	else
	{
		ar << 100003;
		//100003 : added foil comment
		//100002 : means we are serializings opps in the new numbered format
		//100001 : transferred NCrit, XTopTr, XBotTr to polar file
		//first write foil
		ar << 1;//only one this time
		pFoil->Serialize(ar,true);
		//count polars associated to the foil
		Polar * pPolar ;
		int n=0;
		for (i=0; i<Polar::s_oaPolar.size();i++)
		{
			pPolar = (Polar*)Polar::s_oaPolar.at(i);
			if (pPolar->m_FoilName == pFoil->m_FoilName) n++;
		}
		//then write polars
		ar << n;
		for (i=0; i<Polar::s_oaPolar.size();i++)
		{
			pPolar = (Polar*)Polar::s_oaPolar.at(i);
			if (pPolar->m_FoilName == pFoil->m_FoilName) pPolar->Serialize(ar, true);
		}		
	}

	if(m_bSaveOpps)
	{
		ar << OpPoint::s_oaOpp.size();
		OpPoint * pOpp ;
		for (i=0; i<OpPoint::s_oaOpp.size();i++)
		{
			pOpp = (OpPoint*)OpPoint::s_oaOpp.at(i);
			pOpp->SerializeOppWPA(ar,true,100002);
		}
	}
	else ar << 0;

}

 

void MainFrame::SetupDataDir()
{
#ifdef Q_OS_MAC
	s_TranslationDir.setPath(qApp->applicationDirPath()+"/translations/");
	s_StylesheetDir.setPath(qApp->applicationDirPath()+"/qss/");
#endif
#ifdef Q_OS_WIN
    s_TranslationDir.setPath(qApp->applicationDirPath()+"/translations");
    s_StylesheetDir.setPath(qApp->applicationDirPath()+"/qss");
#endif
#ifdef Q_OS_LINUX
	s_TranslationDir.setPath("/usr/share/xflr5/translations");
	s_StylesheetDir.setPath("/usr/share/xflr5/qss");
#endif

}


void MainFrame::ReadStyleSheet(QString styleSheetName, QString &styleSheet)
{
	QFile file(s_StylesheetDir.canonicalPath()+"/"+styleSheetName+".qss");

    if(file.open(QFile::ReadOnly))
	{
		styleSheet = QLatin1String(file.readAll());
		qApp->setStyleSheet(styleSheet);
	}
}



void MainFrame::onProjectModified()
{
	setSaveState(false);
}



/**
 * The user has requested the launch of the interface to manage Foil objects.
 */
void MainFrame::onManageFoils()
{
	ManageFoilsDlg mfDlg(this);

	QString FoilName = "";
	if(Foil::curFoil()) FoilName = Foil::curFoil()->foilName();
	mfDlg.InitDialog(FoilName);
	mfDlg.exec();

	if(mfDlg.m_bChanged) setSaveState(false);

	if(m_iApp==XFLR5::XFOILANALYSIS)
	{
		QXDirect *pXDirect = (QXDirect*)m_pXDirect;
		pXDirect->setFoil(mfDlg.m_pFoil);
		UpdateFoilListBox();
		pXDirect->setControls();
	}
	else if(m_iApp==XFLR5::DIRECTDESIGN)
	{
		QAFoil *pAFoil = (QAFoil*)m_pAFoil;
		pAFoil->FillFoilTable();
		pAFoil->SelectFoil();
	}

	updateView();
}



void MainFrame::checkGraphActions()
{
	for(int ig=0; ig<MAXGRAPHS; ig++)	m_pSingleGraph[ig]->setChecked(false);
	m_pTwoGraphs->setChecked(false);
	m_pFourGraphs->setChecked(false);
	m_pAllGraphs->setChecked(false);
	m_pSingleGraph[0]->setEnabled(true);

	if(m_iApp==XFLR5::MIAREX)
	{
		QMiarex *pMiarex = (QMiarex*)m_pMiarex;
		switch(pMiarex->m_iView)
		{
			case XFLR5::WOPPVIEW:
			{
				if(pMiarex->m_iWingView == XFLR5::ONEGRAPH)        m_pSingleGraph[m_pMiarexTileWidget->activeGraphIndex()]->setChecked(true);
				else if(pMiarex->m_iWingView == XFLR5::TWOGRAPHS)  m_pTwoGraphs->setChecked(true);
				else if(pMiarex->m_iWingView == XFLR5::FOURGRAPHS) m_pFourGraphs->setChecked(true);
				else if(pMiarex->m_iWingView == XFLR5::ALLGRAPHS)  m_pAllGraphs->setChecked(true);
				m_pSingleGraph[1]->setEnabled(true);
				m_pSingleGraph[2]->setEnabled(true);
				m_pSingleGraph[3]->setEnabled(true);
				m_pSingleGraph[4]->setEnabled(true);
				m_pSingleGraph[5]->setEnabled(true);
				m_pTwoGraphs->setEnabled(true);
				m_pFourGraphs->setEnabled(true);
				m_pAllGraphs->setEnabled(true);
				break;
			}
			case XFLR5::WPOLARVIEW:
			{
				if(pMiarex->m_iWPlrView == XFLR5::ONEGRAPH)        m_pSingleGraph[m_pMiarexTileWidget->activeGraphIndex()]->setChecked(true);
				else if(pMiarex->m_iWPlrView == XFLR5::TWOGRAPHS)  m_pTwoGraphs->setChecked(true);
				else if(pMiarex->m_iWPlrView == XFLR5::FOURGRAPHS) m_pFourGraphs->setChecked(true);
				else if(pMiarex->m_iWPlrView == XFLR5::ALLGRAPHS)  m_pAllGraphs->setChecked(true);
				m_pSingleGraph[1]->setEnabled(true);
				m_pSingleGraph[2]->setEnabled(true);
				m_pSingleGraph[3]->setEnabled(true);
				m_pSingleGraph[4]->setEnabled(true);
				m_pSingleGraph[5]->setEnabled(true);
				m_pTwoGraphs->setEnabled(true);
				m_pFourGraphs->setEnabled(true);
				m_pAllGraphs->setEnabled(true);
				break;
			}
			case XFLR5::STABPOLARVIEW:
			{
				if(pMiarex->m_bLongitudinal== XFLR5::ONEGRAPH)      m_pSingleGraph[0]->setChecked(true);
				else if(pMiarex->m_bLongitudinal== XFLR5::ONEGRAPH) m_pSingleGraph[1]->setChecked(true);
				m_pSingleGraph[2]->setEnabled(false);
				m_pSingleGraph[3]->setEnabled(false);
				m_pSingleGraph[4]->setEnabled(false);
				m_pSingleGraph[5]->setEnabled(false);
				m_pTwoGraphs->setEnabled(false);
				m_pFourGraphs->setEnabled(false);
				m_pAllGraphs->setEnabled(false);
				break;
			}
			case XFLR5::STABTIMEVIEW:
			{
				if(pMiarex->m_iStabTimeView == XFLR5::ONEGRAPH)        m_pSingleGraph[m_pMiarexTileWidget->activeGraphIndex()]->setChecked(true);
				else if(pMiarex->m_iStabTimeView == XFLR5::TWOGRAPHS)  m_pTwoGraphs->setChecked(true);
				else if(pMiarex->m_iStabTimeView == XFLR5::FOURGRAPHS) m_pFourGraphs->setChecked(true);
				else if(pMiarex->m_iStabTimeView == XFLR5::ALLGRAPHS)  m_pAllGraphs->setChecked(true);
				m_pSingleGraph[1]->setEnabled(true);
				m_pSingleGraph[2]->setEnabled(true);
				m_pSingleGraph[3]->setEnabled(true);
				m_pSingleGraph[4]->setEnabled(false);
				m_pSingleGraph[5]->setEnabled(false);
				m_pTwoGraphs->setEnabled(true);
				m_pFourGraphs->setEnabled(true);
				m_pAllGraphs->setEnabled(false);
				break;
			}
			default:
			{
				break;
			}
		}
	}
	else if (m_iApp==XFLR5::XFOILANALYSIS)
	{
		QXDirect *pXDirect = (QXDirect*)m_pXDirect;
		if(!pXDirect->m_bPolarView)
		{
			m_pSingleGraph[0]->setChecked(true);

			m_pSingleGraph[1]->setEnabled(false);
			m_pSingleGraph[2]->setEnabled(false);
			m_pSingleGraph[3]->setEnabled(false);
			m_pSingleGraph[4]->setEnabled(false);
			m_pSingleGraph[5]->setEnabled(false);
			m_pTwoGraphs->setEnabled(false);
			m_pFourGraphs->setEnabled(false);
			m_pAllGraphs->setEnabled(false);
		}
		else
		{
			if(pXDirect->m_iPlrView == XFLR5::ONEGRAPH)        m_pSingleGraph[m_pXDirectTileWidget->activeGraphIndex()]->setChecked(true);
			else if(pXDirect->m_iPlrView == XFLR5::TWOGRAPHS)  m_pTwoGraphs->setChecked(true);
			else if(pXDirect->m_iPlrView == XFLR5::FOURGRAPHS) m_pFourGraphs->setChecked(true);
			else if(pXDirect->m_iPlrView == XFLR5::ALLGRAPHS)  m_pAllGraphs->setChecked(true);

			m_pSingleGraph[1]->setEnabled(true);
			m_pSingleGraph[2]->setEnabled(true);
			m_pSingleGraph[3]->setEnabled(true);
			m_pSingleGraph[4]->setEnabled(true);
			m_pSingleGraph[5]->setEnabled(false);
			m_pTwoGraphs->setEnabled(true);
			m_pFourGraphs->setEnabled(true);
			m_pAllGraphs->setEnabled(true);
		}
	}
	else if (m_iApp==XFLR5::INVERSEDESIGN)
	{
		m_pSingleGraph[0]->setChecked(true);
	}
}


void MainFrame::onResetCurGraphScales()
{
	switch(m_iApp)
	{
		case XFLR5::MIAREX:
		{
			m_pMiarexTileWidget->onResetCurGraphScales();
			break;
		}
		case XFLR5::XFOILANALYSIS:
		{
			m_pXDirectTileWidget->onResetCurGraphScales();
			break;
		}
		case XFLR5::INVERSEDESIGN:
		{
			QXInverse *pXInverse = (QXInverse*)m_pXInverse;
			pXInverse->m_QGraph.setAuto(true);
			pXInverse->ReleaseZoom();
			pXInverse->updateView();
			return;
		}
		default:
			return;
	}

}


void MainFrame::onExportCurGraph()
{
	switch(m_iApp)
	{
		case XFLR5::MIAREX:
		{
			m_pMiarexTileWidget->onExportCurGraph();
			break;
		}
		case XFLR5::XFOILANALYSIS:
		{
			m_pXDirectTileWidget->onExportCurGraph();
			break;
		}
		case XFLR5::INVERSEDESIGN:
		{
			QXInverse *pXInverse = (QXInverse*)m_pXInverse;
			pXInverse->m_QGraph.exportGraph();
			break;
		}
		default:
			break;
	}
}





void MainFrame::onCurGraphSettings()
{
	switch(m_iApp)
	{
		case XFLR5::MIAREX:
		{
			m_pMiarexTileWidget->onCurGraphSettings();
			break;
		}
		case XFLR5::XFOILANALYSIS:
		{
			m_pXDirectTileWidget->onCurGraphSettings();
			break;
		}
		case XFLR5::INVERSEDESIGN:
		{
			QXInverse *pXInverse = (QXInverse*)m_pXInverse;
			pXInverse->onQGraphSettings();
			break;
		}
		default:
			break;
	}
}










