/****************************************************************************

	AFoil Class
	Copyright (C) 2009-2016 Andre Deperrois adeperrois@xflr5.com

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

#include <QAction>
#include <QMenu>
#include <QStatusBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QDesktopWidget>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QtDebug>


#include "AFoil.h"
#include "GridSettingsDlg.h"
#include "AFoilTableDlg.h"
#include "SplineCtrlsDlg.h"
#include "LECircleDlg.h"
#include "../globals.h"
#include "../mainframe.h"
#include "../misc/Settings.h"
#include "../misc/LinePickerDlg.h"
#include "../misc/RenameDlg.h"
#include "../xdirect/analysis/XFoil.h"
#include "../xdirect/geometry/NacaFoilDlg.h"
#include "../xdirect/geometry/CAddDlg.h"
#include "../xdirect/geometry/TwoDPanelDlg.h"
#include "../xdirect/geometry/TEGapDlg.h"
#include "../xdirect/geometry/LEDlg.h"
#include "../xdirect/geometry/FlapDlg.h"
#include "../xdirect/geometry/FoilCoordDlg.h"
#include "../xdirect/geometry/FoilGeomDlg.h"
#include "../xdirect/geometry/InterpolateFoilsDlg.h"



void *QAFoil::s_pMainFrame = NULL;

/**
 * The public constructor
 * @param parent a pointer to the MainFrame window
 */
QAFoil::QAFoil(QWidget *parent)
	: QWidget(parent)
{
	setAttribute(Qt::WA_DeleteOnClose);

	m_p2DWidget = NULL;


	m_StackPos = 0;


	m_poaFoil  = NULL;
	m_pctrlFoilTable = NULL;
	m_precision = NULL;

	m_pSF = new SplineFoil();
	m_pSF->m_bModified = false;
	m_pSF->initSplineFoil();

	MainFrame *pMainFrame =(MainFrame*)parent;
	pMainFrame->UndoAFoilAct = pMainFrame->RedoAFoilAct = NULL;

	clearStack();
	takePicture();

	m_bStored        = false;


	m_pBufferFoil = new Foil();

	m_StackPos = 0;

	setupLayout();

	SplineCtrlsDlg::s_pAFoil    = this;
}


/**
 * The public destructor
 */
QAFoil::~QAFoil()
{
	Trace("Destroying AFoil");

	clearStack(-1);
	if(m_pSF) delete m_pSF;
	if(m_pBufferFoil) delete m_pBufferFoil;
	if(m_precision) delete [] m_precision;
}


/**
 * Initializes the state of the button widgets and of the QAction objects.
 */
void QAFoil::setControls()
{
	MainFrame *pMainFrame = (MainFrame*)s_pMainFrame;

	pMainFrame->AFoilDelete->setEnabled(Foil::curFoil());
	pMainFrame->AFoilRename->setEnabled(Foil::curFoil());
	pMainFrame->AFoilExport->setEnabled(Foil::curFoil());
	pMainFrame->pAFoilDuplicateFoil->setEnabled(Foil::curFoil());
	pMainFrame->ShowCurrentFoil->setEnabled(Foil::curFoil());
	pMainFrame->HideCurrentFoil->setEnabled(Foil::curFoil());

	pMainFrame->AFoilDerotateFoil->setEnabled(Foil::curFoil());
	pMainFrame->AFoilEditCoordsFoil->setEnabled(Foil::curFoil());
	pMainFrame->AFoilInterpolateFoils->setEnabled(Foil::curFoil());
	pMainFrame->AFoilNormalizeFoil->setEnabled(Foil::curFoil());
	pMainFrame->AFoilRefineGlobalFoil->setEnabled(Foil::curFoil());
	pMainFrame->AFoilRefineLocalFoil->setEnabled(Foil::curFoil());
	pMainFrame->AFoilScaleFoil->setEnabled(Foil::curFoil());
	pMainFrame->AFoilSetFlap->setEnabled(Foil::curFoil());
	pMainFrame->AFoilSetLERadius->setEnabled(Foil::curFoil());
	pMainFrame->AFoilSetTEGap->setEnabled(Foil::curFoil());

	pMainFrame->m_pShowLegend->setChecked(m_p2DWidget->m_bShowLegend);

	pMainFrame->m_pAFoilSplineMenu->setEnabled(!Foil::curFoil());
	pMainFrame->InsertSplinePt->setEnabled(!Foil::curFoil());
	pMainFrame->RemoveSplinePt->setEnabled(!Foil::curFoil());

	pMainFrame->UndoAFoilAct->setEnabled(m_StackPos>0);
	pMainFrame->RedoAFoilAct->setEnabled(m_StackPos<m_UndoStack.size()-1);
	pMainFrame->statusBar()->clearMessage();
}



/**
 * Fills the table with the data from the Foil objects.
 */
void QAFoil::fillFoilTable()
{
	int i;
	m_pFoilModel->setRowCount(m_poaFoil->size()+1);

	QString name;
	QModelIndex ind;

	double Thickness, xThickness, Camber, xCamber;
	Thickness = xThickness = Camber = xCamber = 0;
	int points = 0;

	if(m_pSF)
	{
		name = tr("Spline foil");
		Thickness  = m_pSF->m_fThickness;
		xThickness = m_pSF->m_fxThickMax;
		Camber     = m_pSF->m_fCamber;
		xCamber    = m_pSF->m_fxCambMax;
		points     = m_pSF->m_OutPoints;
	}

	ind = m_pFoilModel->index(0, 0, QModelIndex());
	m_pFoilModel->setData(ind,name);

	ind = m_pFoilModel->index(0, 1, QModelIndex());
	m_pFoilModel->setData(ind, Thickness);

	ind = m_pFoilModel->index(0, 2, QModelIndex());
	m_pFoilModel->setData(ind, xThickness);

	ind = m_pFoilModel->index(0, 3, QModelIndex());
	m_pFoilModel->setData(ind, Camber);

	ind = m_pFoilModel->index(0, 4, QModelIndex());
	m_pFoilModel->setData(ind,xCamber);

	ind = m_pFoilModel->index(0, 5, QModelIndex());
	m_pFoilModel->setData(ind, points);

/*	ind = m_pFoilModel->index(0,12, QModelIndex());
	if(m_pSF->m_bVisible) m_pFoilModel->setData(ind, Qt::Checked, Qt::CheckStateRole);
	else                  m_pFoilModel->setData(ind, Qt::Unchecked, Qt::CheckStateRole);

	QStandardItem *pItem = m_pFoilModel->item(0,12);
	pItem->setFlags(Qt::ItemIsEnabled|Qt::ItemIsUserCheckable);


	ind = m_pFoilModel->index(0,13, QModelIndex());
	if(m_pSF->m_bOutPoints) m_pFoilModel->setData(ind, Qt::Checked, Qt::CheckStateRole);
	else                    m_pFoilModel->setData(ind, Qt::Unchecked, Qt::CheckStateRole);

	pItem = m_pFoilModel->item(0,13);
	pItem->setFlags(Qt::ItemIsEnabled|Qt::ItemIsUserCheckable);


	ind = m_pFoilModel->index(0,14, QModelIndex());
	if(m_pSF->m_bCenterLine) m_pFoilModel->setData(ind, Qt::Checked,   Qt::CheckStateRole);
	else                     m_pFoilModel->setData(ind, Qt::Unchecked, Qt::CheckStateRole);

	pItem = m_pFoilModel->item(0,14);
	pItem->setFlags(Qt::ItemIsEnabled|Qt::ItemIsUserCheckable);
*/

	for(i=0; i<m_poaFoil->size(); i++)
	{
		fillTableRow(i+1);
	}
}


/**
 * Fills the data from a Foil object in the specified table row.
 * @param row the index of the row to be filled
 */
void QAFoil::fillTableRow(int row)
{
	QModelIndex ind;

	Foil *pFoil = (Foil*)m_poaFoil->at(row-1);

	ind = m_pFoilModel->index(row, 0, QModelIndex());
	m_pFoilModel->setData(ind,pFoil->m_FoilName);

	ind = m_pFoilModel->index(row, 1, QModelIndex());
	m_pFoilModel->setData(ind, pFoil->m_fThickness);

	ind = m_pFoilModel->index(row, 2, QModelIndex());
	m_pFoilModel->setData(ind, pFoil->m_fXThickness);

	ind = m_pFoilModel->index(row, 3, QModelIndex());
	m_pFoilModel->setData(ind, pFoil->m_fCamber);

	ind = m_pFoilModel->index(row, 4, QModelIndex());
	m_pFoilModel->setData(ind,pFoil->m_fXCamber);

	ind = m_pFoilModel->index(row, 5, QModelIndex());
	m_pFoilModel->setData(ind,pFoil->n);


	if(pFoil && pFoil->m_bTEFlap)
	{
		ind = m_pFoilModel->index(row, 6, QModelIndex());
		m_pFoilModel->setData(ind,pFoil->m_TEFlapAngle);

		ind = m_pFoilModel->index(row, 7, QModelIndex());
		m_pFoilModel->setData(ind,pFoil->m_TEXHinge/100.0);

		ind = m_pFoilModel->index(row, 8, QModelIndex());
		m_pFoilModel->setData(ind,pFoil->m_TEYHinge/100.0);

	}
	if(pFoil && pFoil->m_bLEFlap)
	{
		ind = m_pFoilModel->index(row, 9, QModelIndex());
		m_pFoilModel->setData(ind,pFoil->m_LEFlapAngle);

		ind = m_pFoilModel->index(row, 10, QModelIndex());
		m_pFoilModel->setData(ind,pFoil->m_LEXHinge/100.0);

		ind = m_pFoilModel->index(row, 11, QModelIndex());
		m_pFoilModel->setData(ind,pFoil->m_LEYHinge/100.0);
	}

	ind = m_pFoilModel->index(row, 12, QModelIndex());
	if(pFoil->m_bVisible) m_pFoilModel->setData(ind, Qt::Checked, Qt::CheckStateRole);
	else                  m_pFoilModel->setData(ind, Qt::Unchecked, Qt::CheckStateRole);
	QStandardItem *pItem = m_pFoilModel->item(row,12);
	if(pItem) pItem->setFlags(Qt::ItemIsEnabled|Qt::ItemIsUserCheckable);
	
	ind = m_pFoilModel->index(row, 13, QModelIndex());
	if(pFoil->m_bPoints) m_pFoilModel->setData(ind, Qt::Checked, Qt::CheckStateRole);
	else                 m_pFoilModel->setData(ind, Qt::Unchecked, Qt::CheckStateRole);
	pItem = m_pFoilModel->item(row,13);
	if(pItem) pItem->setFlags(Qt::ItemIsEnabled|Qt::ItemIsUserCheckable);

	ind = m_pFoilModel->index(row, 14, QModelIndex());
	if(pFoil->m_bCenterLine) m_pFoilModel->setData(ind, Qt::Checked, Qt::CheckStateRole);
	else                     m_pFoilModel->setData(ind, Qt::Unchecked, Qt::CheckStateRole);
	pItem = m_pFoilModel->item(row,14);
	if(pItem) pItem->setFlags(Qt::ItemIsEnabled|Qt::ItemIsUserCheckable);
}


/**
 * Overrides the QWidget's keyPressEvent method.
 * Dispatches the key press event
 * @param event the QKeyEvent
 */
void QAFoil::keyPressEvent(QKeyEvent *event)
{
	MainFrame *pMainFrame = (MainFrame*)s_pMainFrame;
	bool bShift = false;
	if(event->modifiers() & Qt::ShiftModifier)   bShift =true;
	bool bCtrl = false;
	if(event->modifiers() & Qt::ControlModifier)   bCtrl =true;

	switch (event->key())
	{
		case Qt::Key_F2:
		{
			onRenameFoil();
			break;
		}
		case Qt::Key_F3:
		{
			if(Foil::curFoil())
			{
				if(bShift) onAFoilCadd();
				else       onAFoilPanels();
			}
			break;
		}
		case Qt::Key_F4:
		{
			onStoreSplines();
			break;
		}
		case Qt::Key_F9:
		{
			onAFoilFoilGeom();
			break;
		}
		case Qt::Key_F10:
		{
			onAFoilSetFlap();
			break;
		}
		case Qt::Key_F11:
		{
			onAFoilInterpolateFoils();
			break;
		}

		case Qt::Key_3:
			if(bCtrl)
			{
				pMainFrame->onXInverse();
				return;
			}

		case Qt::Key_4:
			if(bCtrl)
			{
				pMainFrame->onXInverseMixed();
				return;
			}

		case Qt::Key_5:
			if(bCtrl)
			{
				pMainFrame->onXDirect();
				return;
			}

		case Qt::Key_6:
			if(bCtrl)
			{
				pMainFrame->onMiarex();
				return;
			}

		default:
			QWidget::keyPressEvent(event);
	}
}



/**
 * Loads the user's default settings from the application QSettings object.
 * @param pSettings a pointer to the QSettings object
 */
void QAFoil::loadSettings(QSettings *pSettings)
{
	int style, width;
	QColor color;

	pSettings->beginGroup("DirectDesign");
	{
		style  = pSettings->value("SFStyle", 0).toInt();
		width  = pSettings->value("SFWidth", 1).toInt();
		color  = pSettings->value("SFColor",QColor(216,183,83)).value<QColor>();
		m_pSF->SetCurveParams(style, width, color);


		m_pSF->m_bVisible    = pSettings->value("SFVisible").toBool();
		m_pSF->m_bOutPoints  = pSettings->value("SFOutPoints").toBool();
		m_pSF->m_bCenterLine = pSettings->value("SFCenterLine").toBool();

		m_pSF->m_Intrados.m_iRes =  pSettings->value("LowerRes",79).toInt();
		m_pSF->m_Extrados.m_iRes =  pSettings->value("UpperRes",79).toInt();
		m_pSF->m_Extrados.splineCurve();
		m_pSF->m_Intrados.splineCurve();


		m_p2DWidget->m_bLECircle          = pSettings->value("LECircle").toBool();
		m_p2DWidget->m_bShowLegend        = pSettings->value("Legend").toBool();

		QString str;
		for(int i=0; i<16; i++)
		{
			str = QString("Column_%1").arg(i);
			m_pctrlFoilTable->setColumnWidth(i, pSettings->value(str,40).toInt());
			if(pSettings->value(str+"_hidden", false).toBool()) m_pctrlFoilTable->hideColumn(i);
		}
	}
	pSettings->endGroup();
}





/**
 * The user has requested that the foil be derotated.
 */
void QAFoil::onAFoilDerotateFoil()
{
	if(!Foil::curFoil()) return;

	m_pBufferFoil->copyFoil(Foil::curFoil());
	m_pBufferFoil->m_bVisible = true;
	m_pBufferFoil->m_FoilName = Foil::curFoil()->m_FoilName;
	m_pBufferFoil->m_FoilColor  = QColor(160,160,160);
	m_pBufferFoil->m_FoilStyle = 1;
	m_pBufferFoil->m_FoilWidth = 1;

	m_p2DWidget->update();;

	double angle = m_pBufferFoil->derotate();
	QString str = QString(tr("Foil has been de-rotated by %1 degrees")).arg(angle,6,'f',3);
	MainFrame* pMainFrame = (MainFrame*)s_pMainFrame;
	pMainFrame->statusBar()->showMessage(str);

	//then duplicate the buffer foil and add it
	Foil *pNewFoil = new Foil();
	pNewFoil->copyFoil(m_pBufferFoil);
	pNewFoil->m_FoilColor  = MainFrame::getColor(0);
	pNewFoil->m_FoilStyle = 0;
	pNewFoil->m_FoilWidth = 1;

	addNewFoil(pNewFoil);
	fillFoilTable();
	selectFoil(pNewFoil);

	m_pBufferFoil->m_bVisible = false;

	m_p2DWidget->update();;
}


/**
 * The user has requested that the Foil be normalized to unit length.
 */
void QAFoil::onAFoilNormalizeFoil()
{
	if(!Foil::curFoil()) return;
	double length = Foil::curFoil()->normalizeGeometry();
	Foil::curFoil()->initFoil();
	QString str = QString(tr("Foil has been normalized from %1  to 1.000")).arg(length,7,'f',3);

	MainFrame* pMainFrame = (MainFrame*)s_pMainFrame;
	pMainFrame->statusBar()->showMessage(str);

	m_p2DWidget->update();;
}


/**
 * The user has requested a local refinement of the panels of the current Foil.
 */
void QAFoil::onAFoilCadd()
{
	if(!Foil::curFoil()) return;

	MainFrame* pMainFrame = (MainFrame*)s_pMainFrame;

	m_pBufferFoil->copyFoil(Foil::curFoil());
	m_pBufferFoil->m_FoilName   = Foil::curFoil()->m_FoilName;
	m_pBufferFoil->m_FoilColor  = QColor(160,160,160);
	m_pBufferFoil->m_FoilStyle = 1;
	m_pBufferFoil->m_FoilWidth = 1;
	m_pBufferFoil->m_bPoints    = true;
	m_pBufferFoil->m_bVisible   = true;

	m_p2DWidget->update();;

	CAddDlg caDlg(pMainFrame);
	caDlg.m_pBufferFoil = m_pBufferFoil;
	caDlg.m_pMemFoil    = Foil::curFoil();
	caDlg.InitDialog();

	if(QDialog::Accepted == caDlg.exec())
	{
		//then duplicate the buffer foil and add it
		Foil *pNewFoil = new Foil();
		pNewFoil->copyFoil(m_pBufferFoil);
		pNewFoil->m_FoilColor  = MainFrame::getColor(0);
		pNewFoil->m_FoilStyle = 0;
		pNewFoil->m_FoilWidth = 1;
		pNewFoil->m_bPoints = false;

		addNewFoil(pNewFoil);
		fillFoilTable();
		selectFoil(pNewFoil);
	}
	else
	{
		fillFoilTable();
		selectFoil(Foil::curFoil());
		((XFoil*)m_pXFoil)->m_FoilName ="";

	}
	m_pBufferFoil->m_bVisible = false;
	m_p2DWidget->update();;
}

/**
 * The user has requested the display of a circle at the L.E. position.
 */
void QAFoil::onAFoilLECircle()
{
    LECircleDlg LECircleDlg(this);
	LECircleDlg.m_Radius      = m_p2DWidget->m_LERad;
	LECircleDlg.m_bShowRadius = m_p2DWidget->m_bLECircle;
	LECircleDlg.m_pAFoil      = this;
	LECircleDlg.InitDialog();

	if(LECircleDlg.exec()==QDialog::Accepted)
	{
		m_p2DWidget->m_LERad = LECircleDlg.m_Radius;
		m_p2DWidget->m_bLECircle = LECircleDlg.m_bShowRadius;
	}
	m_p2DWidget->update();;
}


/**
 * The user has requested the launch of the interface to refine globally the Foil.
*/
void QAFoil::onAFoilPanels()
{
	if(!Foil::curFoil()) return;

	MainFrame* pMainFrame = (MainFrame*)s_pMainFrame;

	m_pBufferFoil->copyFoil(Foil::curFoil());
	m_pBufferFoil->m_FoilName  = Foil::curFoil()->m_FoilName;
	m_pBufferFoil->m_FoilColor  = QColor(160,160,160);
	m_pBufferFoil->m_FoilStyle = 1;
	m_pBufferFoil->m_FoilWidth = 1;
	m_pBufferFoil->m_bPoints   = true;
	m_pBufferFoil->m_bVisible  = true;

	m_p2DWidget->update();;

    TwoDPanelDlg tdpDlg(pMainFrame);
    tdpDlg.m_pBufferFoil = m_pBufferFoil;
	tdpDlg.m_pMemFoil    = Foil::curFoil();
    tdpDlg.InitDialog();

    if(QDialog::Accepted == tdpDlg.exec())
	{
		//then duplicate the buffer foil and add it
		Foil *pNewFoil = new Foil();
		pNewFoil->copyFoil(m_pBufferFoil);
		pNewFoil->m_FoilColor  = MainFrame::getColor(0);
		pNewFoil->m_FoilStyle = 0;
		pNewFoil->m_FoilWidth = 1;
		pNewFoil->m_bPoints = false;

		addNewFoil(pNewFoil);
		fillFoilTable();
		selectFoil(pNewFoil);
	}
	else
	{
		fillFoilTable();
		selectFoil(Foil::curFoil());
//		m_pXFoil->m_FoilName ="";

	}

	m_pBufferFoil->m_bVisible = false;
	m_p2DWidget->update();;
}

/**
 * The user has requested the launch of the interface to edit the Foil coordinates manually.
*/
void QAFoil::onAFoilFoilCoordinates()
{
	if(!Foil::curFoil()) return;
	MainFrame* pMainFrame = (MainFrame*)s_pMainFrame;

	m_pBufferFoil->copyFoil(Foil::curFoil());
	m_pBufferFoil->m_bPoints  = true;
	m_pBufferFoil->m_bVisible = true;
	m_pBufferFoil->m_FoilName = Foil::curFoil()->m_FoilName;
	m_pBufferFoil->m_FoilColor  = QColor(160,160,160);
	m_pBufferFoil->m_FoilStyle = 1;
	m_pBufferFoil->m_FoilWidth = 1;

	m_p2DWidget->update();;

    FoilCoordDlg fcDlg(pMainFrame);
	fcDlg.m_pMemFoil    = Foil::curFoil();
    fcDlg.m_pBufferFoil = m_pBufferFoil;
	fcDlg.InitDialog();

    if(QDialog::Accepted == fcDlg.exec())
	{
		//then duplicate the buffer foil and add it
		Foil *pNewFoil = new Foil();
		pNewFoil->copyFoil(m_pBufferFoil);
		pNewFoil->m_bPoints = false;
		pNewFoil->m_FoilColor  = MainFrame::getColor(0);
		pNewFoil->m_FoilStyle = 0;
		pNewFoil->m_FoilWidth = 1;
		pNewFoil->m_iHighLight = -1;

		addNewFoil(pNewFoil);
		fillFoilTable();
		selectFoil(pNewFoil);
	}
	else
	{
		fillFoilTable();
		selectFoil(Foil::curFoil());
		((XFoil*)m_pXFoil)->m_FoilName ="";
	}
	m_pBufferFoil->m_bVisible = false;
	m_p2DWidget->update();;
}


/**
 * The user has requested to perform an edition of the current foil's thickness and camber properties.
 */
void QAFoil::onAFoilFoilGeom()
{
	if(!Foil::curFoil()) return;
	MainFrame* pMainFrame = (MainFrame*)s_pMainFrame;

	m_pBufferFoil->copyFoil(Foil::curFoil());
	m_pBufferFoil->m_bPoints  = true;
	m_pBufferFoil->m_bVisible = true;
	m_pBufferFoil->m_FoilName = Foil::curFoil()->m_FoilName;
	m_pBufferFoil->m_FoilColor  = QColor(160,160,160);
	m_pBufferFoil->m_FoilStyle = 1;
	m_pBufferFoil->m_FoilWidth = 1;

	m_p2DWidget->update();;

    FoilGeomDlg fgeDlg(pMainFrame);
	fgeDlg.m_pMemFoil    = Foil::curFoil();
    fgeDlg.m_pBufferFoil = m_pBufferFoil;
    fgeDlg.InitDialog();

    if(QDialog::Accepted == fgeDlg.exec())
	{
		//then duplicate the buffer foil and add it
		Foil *pNewFoil = new Foil();
		pNewFoil->copyFoil(m_pBufferFoil);
		pNewFoil->m_FoilColor  = MainFrame::getColor(0);
		pNewFoil->m_FoilStyle = 0;
		pNewFoil->m_FoilWidth = 1;
		pNewFoil->m_bPoints = false;

		addNewFoil(pNewFoil);
		fillFoilTable();
		selectFoil(pNewFoil);
	}
	else
	{
		fillFoilTable();
		selectFoil(Foil::curFoil());
//		m_pXFoil->m_FoilName ="";
	}
	m_pBufferFoil->m_bVisible = false;
	m_p2DWidget->update();;
}


/**
 * The user has requested the launch of the interface to modify the gap at the Foil's trailing edge.
 */
void QAFoil::onAFoilSetTEGap()
{
	if(!Foil::curFoil()) return;

	MainFrame* pMainFrame = (MainFrame*)s_pMainFrame;

	m_pBufferFoil->copyFoil(Foil::curFoil());
	m_pBufferFoil->m_bPoints    = false;
	m_pBufferFoil->m_bVisible   = true;
	m_pBufferFoil->m_FoilName   = Foil::curFoil()->m_FoilName;
	m_pBufferFoil->m_FoilColor  = QColor(160,160,160);
	m_pBufferFoil->m_FoilStyle = 1;
	m_pBufferFoil->m_FoilWidth = 1;

	m_p2DWidget->update();;

    TEGapDlg teDlg(pMainFrame);
    teDlg.m_pBufferFoil = m_pBufferFoil;
	teDlg.m_pMemFoil    = Foil::curFoil();
	teDlg.InitDialog();

    if(QDialog::Accepted == teDlg.exec())
	{
		//then duplicate the buffer foil and add it
		Foil *pNewFoil = new Foil();
		pNewFoil->copyFoil(m_pBufferFoil);
		pNewFoil->m_FoilColor  = MainFrame::getColor(0);
		pNewFoil->m_FoilStyle = 0;
		pNewFoil->m_FoilWidth = 1;
		pNewFoil->m_bPoints    = false;

		addNewFoil(pNewFoil);
		fillFoilTable();
		selectFoil(pNewFoil);
	}
	else
	{
		fillFoilTable();
		selectFoil(Foil::curFoil());
//		m_pXFoil->m_FoilName ="";
		//to un-initialize XFoil in case user switches to XInverse
		//Thanks Jean-Marc !
	}

	m_pBufferFoil->m_bVisible = false;
	m_p2DWidget->update();;
}


/**
 * The user has requested the launch of the interface to modify the radius of the Foil's leading edge.
 */
void QAFoil::onAFoilSetLERadius()
{
	if(!Foil::curFoil()) return;

	MainFrame* pMainFrame = (MainFrame*)s_pMainFrame;

	m_pBufferFoil->copyFoil(Foil::curFoil());
	m_pBufferFoil->m_bVisible   = true;
	m_pBufferFoil->m_bPoints    = false;
	m_pBufferFoil->m_FoilName   = Foil::curFoil()->m_FoilName;
	m_pBufferFoil->m_FoilColor  = QColor(160,160,160);
	m_pBufferFoil->m_FoilStyle = 1;
	m_pBufferFoil->m_FoilWidth = 1;

	m_p2DWidget->update();;

    LEDlg leDlg(pMainFrame);
    leDlg.m_pBufferFoil = m_pBufferFoil;
	leDlg.m_pMemFoil    = Foil::curFoil();
    leDlg.InitDialog();

    if(QDialog::Accepted == leDlg.exec())
	{
		//then duplicate the buffer foil and add it
		Foil *pNewFoil = new Foil();
		pNewFoil->copyFoil(m_pBufferFoil);
		pNewFoil->m_FoilColor  = MainFrame::getColor(0);
		pNewFoil->m_FoilStyle = 0;
		pNewFoil->m_FoilWidth = 1;
		pNewFoil->m_bPoints    = false;

		addNewFoil(pNewFoil);
		fillFoilTable();
		selectFoil(pNewFoil);
	}
	else
	{
		fillFoilTable();
		selectFoil(Foil::curFoil());
//		m_pXFoil->m_FoilName ="";

	}

	m_pBufferFoil->m_bVisible = false;
	m_p2DWidget->update();;
}


/**
 * The user has requested the launch of the interface to create a foil from the interpolation of two existing Foil objects.
 */
void QAFoil::onAFoilInterpolateFoils()
{
	MainFrame* pMainFrame = (MainFrame*)s_pMainFrame;
	if(m_poaFoil->size()<2)
	{
		QMessageBox::warning(pMainFrame,tr("Warning"), tr("At least two foils are required"));
		return;
	}

	if(!Foil::curFoil()) selectFoil();
	if(!Foil::curFoil()) return;
	m_pBufferFoil->copyFoil(Foil::curFoil());
	m_pBufferFoil->m_FoilName  = Foil::curFoil()->m_FoilName;
	m_pBufferFoil->m_FoilColor  = QColor(160,160,160);
	m_pBufferFoil->m_FoilStyle = 1;
	m_pBufferFoil->m_FoilWidth = 1;
	m_pBufferFoil->m_bPoints   = false;
	m_pBufferFoil->m_bVisible  = true;

	m_p2DWidget->update();;

	InterpolateFoilsDlg ifDlg(pMainFrame);
    ifDlg.m_poaFoil = m_poaFoil;
    ifDlg.m_pBufferFoil = m_pBufferFoil;
    ifDlg.InitDialog();

    if(QDialog::Accepted == ifDlg.exec())
	{
		//then duplicate the buffer foil and add it
		Foil *pNewFoil = new Foil();
		pNewFoil->copyFoil(m_pBufferFoil);
		pNewFoil->m_FoilColor  = MainFrame::getColor(0);
		pNewFoil->m_FoilStyle = 0;
		pNewFoil->m_FoilWidth = 1;
		pNewFoil->m_bPoints = false;
        pNewFoil->m_FoilName = ifDlg.m_NewFoilName;

		addNewFoil(pNewFoil);
		fillFoilTable();
		selectFoil(pNewFoil);

	}
	else
	{
		fillFoilTable();
		selectFoil(Foil::curFoil());
	}
	m_pBufferFoil->m_bVisible = false;
	m_p2DWidget->update();;
}


/**
 * The user has requested the launch of the interface used to create a NACA type Foil.
 */
void QAFoil::onAFoilNacaFoils()
{
	MainFrame* pMainFrame = (MainFrame*)s_pMainFrame;

	m_pBufferFoil->setNaca009();
	m_pBufferFoil->m_bPoints  = true;
	m_pBufferFoil->m_bVisible = true;
	m_pBufferFoil->m_FoilName = "Naca xxxx";
	m_pBufferFoil->m_FoilColor  = QColor(160,160,160);
	m_pBufferFoil->m_FoilStyle = 1;
	m_pBufferFoil->m_FoilWidth = 1;

	m_p2DWidget->update();;

    NacaFoilDlg nacaDlg(pMainFrame);
    nacaDlg.m_pBufferFoil = m_pBufferFoil;

    if(QDialog::Accepted == nacaDlg.exec())
	{
		//then duplicate the buffer foil and add it
		QString str;

        if(nacaDlg.s_Digits>0 && log10((double)nacaDlg.s_Digits)<4)
            str = QString("%1").arg(nacaDlg.s_Digits,4,10,QChar('0'));
		else
            str = QString("%1").arg(nacaDlg.s_Digits);
		str = "NACA "+ str;

		Foil *pNewFoil    = new Foil();
		pNewFoil->copyFoil(m_pBufferFoil);
		pNewFoil->m_FoilColor  = MainFrame::getColor(0);
		pNewFoil->m_FoilStyle = 0;
		pNewFoil->m_FoilWidth = 1;
		pNewFoil->m_bPoints    = false;
		pNewFoil->m_FoilName   = str;

		addNewFoil(pNewFoil);
		fillFoilTable();
		selectFoil(pNewFoil);
	}
	else
	{
		fillFoilTable();;
		if(Foil::curFoil()) selectFoil(Foil::curFoil());
		((XFoil*)m_pXFoil)->m_FoilName ="";

	}

	m_pBufferFoil->m_bVisible = false;
	m_p2DWidget->update();;
}


/**
 * The user has requested the launch of the interface to define a L.E. or T.E. flap.
 */
void QAFoil::onAFoilSetFlap()
{
	if(!Foil::curFoil()) return;

	MainFrame* pMainFrame = (MainFrame*)s_pMainFrame;

	m_pBufferFoil->copyFoil(Foil::curFoil());
	m_pBufferFoil->m_bVisible = true;
	m_pBufferFoil->m_FoilName = Foil::curFoil()->m_FoilName;
	m_pBufferFoil->m_FoilColor  = QColor(160,160,160);
	m_pBufferFoil->m_FoilStyle = 1;
	m_pBufferFoil->m_FoilWidth = 1;

	m_p2DWidget->update();;

    FlapDlg flDlg(pMainFrame);
	flDlg.m_pXFoil      = m_pXFoil;
	flDlg.m_pMemFoil    = Foil::curFoil();
    flDlg.m_pBufferFoil = m_pBufferFoil;
    flDlg.InitDialog();

    if(QDialog::Accepted == flDlg.exec())
	{
		//then duplicate the buffer foil and add it
		Foil *pNewFoil = new Foil();
		pNewFoil->copyFoil(m_pBufferFoil);
		pNewFoil->m_FoilColor  = MainFrame::getColor(0);
		pNewFoil->m_FoilStyle = 0;
		pNewFoil->m_FoilWidth = 1;

		addNewFoil(pNewFoil);
		fillFoilTable();
		selectFoil(pNewFoil);
	}
	else
	{
		fillFoilTable();
		selectFoil(Foil::curFoil());
		((XFoil*)m_pXFoil)->m_FoilName ="";
	}
	m_pBufferFoil->m_bVisible = false;
	m_p2DWidget->update();;
}


/**
 * The user has requested the deletion of the current Foil.
 */
void QAFoil::onDeleteCurFoil()
{
	if(!Foil::curFoil()) return;

	QString strong;
	strong = tr("Are you sure you want to delete")  +"\n"+ Foil::curFoil()->foilName() +"\n";
	strong+= tr("and all associated OpPoints and Polars ?");

	int resp = QMessageBox::question((MainFrame*)s_pMainFrame, tr("Question"), strong,  QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
	if(resp != QMessageBox::Yes) return;

	Foil*pNextFoil = Foil::deleteFoil(Foil::curFoil());

	fillFoilTable();
	selectFoil(pNextFoil);
	m_p2DWidget->update();;
}


/**
 * The user has requested the duplication of the current Foil.
 */
void QAFoil::onDuplicate()
{
	if(!Foil::curFoil()) return;
	Foil *pNewFoil = new Foil;
	pNewFoil->copyFoil(Foil::curFoil());
	pNewFoil->m_FoilColor = MainFrame::getColor(0);
	pNewFoil->initFoil();

	addNewFoil(pNewFoil);
	fillFoilTable();
	selectFoil(pNewFoil);
}


/**
 * The user has requested the export of the current Foil to a text file.
 */
void QAFoil::onExportCurFoil()
{
	if(!Foil::curFoil())	return;

	QString FileName;

	FileName = Foil::curFoil()->m_FoilName;
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
 * The user has requested the export of the current SplineFoil to a text file.
 */
void QAFoil::onExportSplinesToFile()
{
	QString FoilName = tr("Spline Foil");
	QString FileName, strong;
	QString strOut;

	MainFrame *pMainFrame = (MainFrame*)s_pMainFrame;

	// deselect points so as not to interfere with other mouse commands
	m_pSF->m_Intrados.m_iSelect = -10;
	m_pSF->m_Extrados.m_iSelect = -10;

	//check that the number of output points is consistent with the array's size

	if(m_pSF->m_Extrados.m_iRes>IQX2)
	{
		strong = QString(tr("Too many output points on upper surface\n Max =%1")).arg(IQX2);
		QMessageBox::warning(pMainFrame, tr("Warning"), strong, QMessageBox::Ok);
		return;
	}
	if(m_pSF->m_Intrados.m_iRes>IQX2)
	{
		strong = QString(tr("Too many output points on lower surface\n Max =%1")).arg(IQX2);
		QMessageBox::warning(pMainFrame, tr("Warning"), strong, QMessageBox::Ok);
		return;
	}


	FileName.replace("/", " ");
	FileName = QFileDialog::getSaveFileName(this, tr("Export Splines"), 
				Settings::s_LastDirName,
				tr("Text File (*.dat)"));

	if(!FileName.length()) return;
	int pos;
	pos = FileName.lastIndexOf("/");
	if(pos>0) Settings::s_LastDirName = FileName.left(pos);

	QFile XFile(FileName);

	if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

	QTextStream out(&XFile);


	strOut = FoilName + "\n";
	out << strOut;

	m_pSF->exportToFile(out);
	XFile.close();
}


/**
 * A row has been clicked in the table of Foil objects.
 * @param index a QModelIndex object clicked in the table
 */
void QAFoil::onFoilClicked(const QModelIndex& index)
{
	m_pctrlFoilTable->blockSignals(true);
	m_pFoilModel->blockSignals(true);

	if(index.row()>=m_poaFoil->size()+1) return;
	QStandardItem *pItem = m_pFoilModel->item(index.row(),0);
	if(!pItem) return;

	m_pctrlFoilTable->selectRow(index.row());

	if(index.row()==0)
	{
		Foil::setCurFoil(NULL);
		if(index.column()==12)
		{
			m_pSF->m_bVisible = !m_pSF->m_bVisible;
		}
		else if(index.column()==13)
		{
			m_pSF->m_bOutPoints = !m_pSF->m_bOutPoints;
		}
		else if(index.column()==14)
		{
			m_pSF->m_bCenterLine = !m_pSF->m_bCenterLine;
		}
		m_p2DWidget->update();;
	}
	else if(index.row()>0)
	{
		Foil *pFoil= Foil::foil(pItem->text());
		Foil::setCurFoil(pFoil);

		if(pFoil)
		{
			if(index.column()==12)
			{
				pFoil->m_bVisible = !pFoil->m_bVisible;
			}
			else if(index.column()==13)
			{
				pFoil->m_bPoints = !pFoil->m_bPoints;
			}
			else if(index.column()==14)
			{
				pFoil->m_bCenterLine = !pFoil->m_bCenterLine;
			}
		}
		emit projectModified();
		m_p2DWidget->update();;
	}

	if(index.column()==15) onFoilStyle();

	setControls();

	m_pctrlFoilTable->blockSignals(false);
	m_pFoilModel->blockSignals(false);
}



/**
 * The user has requested an edition of the style of the active Foil.
 */
void QAFoil::onFoilStyle()
{
	if(!Foil::curFoil())
	{
        LinePickerDlg dlg(this);
		dlg.initDialog(m_pSF->m_FoilStyle, m_pSF->m_FoilWidth, m_pSF->m_FoilColor);

		if(QDialog::Accepted==dlg.exec())
		{
			m_pSF->SetCurveParams(dlg.setStyle(), dlg.width(), dlg.setColor());
			m_p2DWidget->update();;
		}
	}
	else
	{
        LinePickerDlg dlg(this);
		dlg.initDialog(Foil::curFoil()->m_FoilStyle, Foil::curFoil()->m_FoilWidth, Foil::curFoil()->m_FoilColor);

		if(QDialog::Accepted==dlg.exec())
		{
			emit projectModified();
			Foil::curFoil()->m_FoilStyle = dlg.setStyle();
			Foil::curFoil()->m_FoilWidth = dlg.width();
			Foil::curFoil()->m_FoilColor = dlg.setColor();
			m_p2DWidget->update();;
		}
	}
}



/**
 * The user has requested that the visibility of all Foil objects be turned off.
 */
void QAFoil::onHideAllFoils()
{
	emit projectModified();
	Foil*pFoil;
	for (int k=0; k<m_poaFoil->size(); k++)
	{
		pFoil = (Foil*)m_poaFoil->at(k);
		pFoil->m_bVisible = false;
	}
	fillFoilTable();
	m_p2DWidget->update();;
}



/**
 * The user has requested that the visibility of the active Foil object be turned off.
 */
void QAFoil::onHideCurrentFoil()
{
	if(!Foil::curFoil()) return;
	showFoil(Foil::curFoil(), false);
	m_p2DWidget->update();;

}


/**
 * The user has requested to restore the default settings for the splines.
 */
void QAFoil::onNewSplines()
{
	MainFrame *pMainFrame = (MainFrame*)s_pMainFrame;
	if(m_pSF->m_bModified)
	{
		if (QMessageBox::Yes != QMessageBox::question(pMainFrame, tr("Question"), tr("Discard changes to Splines ?"),
													  QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel))
		{
			return;
		}
	}
	m_pSF->initSplineFoil();

	m_StackPos  = 0;
	clearStack(0);
	takePicture();

	emit projectModified();
	m_p2DWidget->update();;
}


/**
 * The user has requested to rename the Foil object
 */
void QAFoil::onRenameFoil()
{
	if(!Foil::curFoil()) return;

	Foil*pOldFoil;

	QStringList NameList;
	for(int k=0; k<Foil::s_oaFoil.size(); k++)
	{
		pOldFoil = (Foil*)Foil::s_oaFoil.at(k);
		NameList.append(pOldFoil->m_FoilName);
	}

	RenameDlg renDlg(this);
	renDlg.InitDialog(&NameList, Foil::curFoil()->foilName(), tr("Enter the foil's new name"));

	if(renDlg.exec() !=QDialog::Rejected)
	{
		Foil::curFoil()->renameThisFoil(renDlg.newName());
	}

	fillFoilTable();
	m_p2DWidget->update();;
}


/**
 * The user has requested that the visibility of all Foil objects be turned on.
 */
void QAFoil::onShowAllFoils()
{
	emit projectModified();
	Foil*pFoil;
	for (int k=0; k<m_poaFoil->size(); k++)
	{
		pFoil = (Foil*)m_poaFoil->at(k);
		pFoil->m_bVisible = true;
	}
	fillFoilTable();
	m_p2DWidget->update();;
}


/**
 * The user has requested that the visibility of the active Foil object be turned on.
 */
void QAFoil::onShowCurrentFoil()
{
	if(!Foil::curFoil()) return;
	showFoil(Foil::curFoil(), true);
	m_p2DWidget->update();;

}


/**
 * The user has toggled the visibility of the legend
 */
void QAFoil::onShowLegend()
{
	m_p2DWidget->m_bShowLegend = !m_p2DWidget->m_bShowLegend;
	m_p2DWidget->update();;
	setControls();
}


/**
 * The user has requested to convert the SplineFoil object to a Foil, and to store it in the database.
 */
void QAFoil::onStoreSplines()
{
	MainFrame *pMainFrame = (MainFrame*)s_pMainFrame;

	if(m_pSF->m_Extrados.m_iRes>IQX2)
	{
		QString strong = QString(tr("Too many output points on upper surface\n Max =%1")).arg(IQX2);
		QMessageBox::warning(pMainFrame, tr("Warning"), strong, QMessageBox::Ok);
		return;
	}
	if(m_pSF->m_Intrados.m_iRes>IQX2)
	{
		QString strong = QString(tr("Too many output points on lower surface\n Max =%1")).arg(IQX2);
		QMessageBox::warning(pMainFrame, tr("Warning"), strong, QMessageBox::Ok);
		return;
	}


	Foil *pNewFoil = new Foil();
	m_pSF->exportToBuffer(pNewFoil);
	pNewFoil->m_FoilName = m_pSF->m_strFoilName;
	addNewFoil(pNewFoil);
	fillFoilTable();
	selectFoil(pNewFoil);

	m_p2DWidget->update();;
}




/**
 * The user has requested the launch of the interface to edit SplineFoil data.
 */
void QAFoil::onSplineControls()
{
    SplineCtrlsDlg dlg(this);
	dlg.m_pSF = m_pSF;
	dlg.initDialog();

	SplineFoil memSF;
	memSF.copy(m_pSF);

	if(dlg.exec() == QDialog::Accepted)
	{
		takePicture();
	}
	else m_pSF->copy(&memSF);
}





/**
 * Saves the user-defined settings.
 * @param pSettings a pointer to the QSetting object.
 */
void QAFoil::SaveSettings(QSettings *pSettings)
{
	pSettings->beginGroup("DirectDesign");
	{
		pSettings->setValue("SFStyle", m_pSF->m_FoilStyle);
		pSettings->setValue("SFWidth", m_pSF->m_FoilWidth);
		pSettings->setValue("SFColor", m_pSF->m_FoilColor);

		pSettings->setValue("SFVisible", m_pSF->m_bVisible);
		pSettings->setValue("SFOutPoints", m_pSF->m_bOutPoints);
		pSettings->setValue("SFCenterLine", m_pSF->m_bCenterLine);

		pSettings->setValue("LowerRes", m_pSF->m_Intrados.m_iRes);
		pSettings->setValue("UpperRes", m_pSF->m_Extrados.m_iRes);

		pSettings->setValue("LECircle", m_p2DWidget->m_bLECircle);
		pSettings->setValue("Legend", m_p2DWidget->m_bShowLegend );
		
		QString str;
		for(int i=0; i<16; i++)
		{
			str = QString("Column_%1").arg(i);
			pSettings->setValue(str,m_pctrlFoilTable->columnWidth(i));
		}
		for(int i=0; i<16; i++)
		{
			str = QString("Column_%1").arg(i);
			pSettings->setValue(str+"_hidden", m_pctrlFoilTable->isColumnHidden(i));
		}

	}
	pSettings->endGroup();
}





/**
 * The user has requested the context menu associated to the Foil table.
 * @param position the right-click positon
 */
void QAFoil::onFoilTableCtxMenu(const QPoint &)
{
//	m_CurrentColumn = m_pctrlFoilTable->columnAt(position.x());
	MainFrame* pMainFrame = (MainFrame*)s_pMainFrame;
	pMainFrame->m_pAFoilTableCtxMenu->exec(cursor().pos());
}


/**
 * Sets up the GUI.
 */
void QAFoil::setupLayout()
{
	m_pctrlFoilTable   = new QTableView(this);
	m_pctrlFoilTable->setSelectionMode(QAbstractItemView::SingleSelection);
	m_pctrlFoilTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_pctrlFoilTable->setContextMenuPolicy(Qt::CustomContextMenu);
	m_pctrlFoilTable->setWordWrap(false);
	m_pctrlFoilTable->setFont(Settings::s_TableFont);
	m_pctrlFoilTable->horizontalHeader()->setFont(Settings::s_TableFont);

//	connect(m_pctrlFoilTable, SIGNAL(pressed(const QModelIndex &)), this, SLOT(OnFoilClicked(const QModelIndex&)));
	connect(m_pctrlFoilTable, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(onFoilTableCtxMenu(const QPoint &)));

	QHBoxLayout *MainLayout = new QHBoxLayout;
	MainLayout->addWidget(m_pctrlFoilTable);
	setLayout(MainLayout);


	m_pFoilModel = new QStandardItemModel(this);
	m_pFoilModel->setRowCount(10);//temporary
	m_pFoilModel->setColumnCount(16);

	m_pFoilModel->setHeaderData(0,  Qt::Horizontal, tr("Name"));
	m_pFoilModel->setHeaderData(1,  Qt::Horizontal, tr("Thickness (%)"));
	m_pFoilModel->setHeaderData(2,  Qt::Horizontal, tr("at (%)"));
	m_pFoilModel->setHeaderData(3,  Qt::Horizontal, tr("Camber (%)"));
	m_pFoilModel->setHeaderData(4,  Qt::Horizontal, tr("at (%)"));
	m_pFoilModel->setHeaderData(5,  Qt::Horizontal, tr("Points"));
	m_pFoilModel->setHeaderData(6,  Qt::Horizontal, tr("TE Flap (")+QString::fromUtf8("°")+")");
	m_pFoilModel->setHeaderData(7,  Qt::Horizontal, tr("TE XHinge"));
	m_pFoilModel->setHeaderData(8,  Qt::Horizontal, tr("TE YHinge"));
	m_pFoilModel->setHeaderData(9,  Qt::Horizontal, tr("LE Flap (")+QString::fromUtf8("°")+")");
	m_pFoilModel->setHeaderData(10, Qt::Horizontal, tr("LE XHinge"));
	m_pFoilModel->setHeaderData(11, Qt::Horizontal, tr("LE YHinge"));
	m_pFoilModel->setHeaderData(12, Qt::Horizontal, tr("Show"));
	m_pFoilModel->setHeaderData(13, Qt::Horizontal, tr("Points"));
	m_pFoilModel->setHeaderData(14, Qt::Horizontal, tr("Centerline"));
	m_pFoilModel->setHeaderData(15, Qt::Horizontal, tr("Style"));
	m_pctrlFoilTable->setModel(m_pFoilModel);
	m_pctrlFoilTable->setWindowTitle(tr("Foils"));
	m_pctrlFoilTable->horizontalHeader()->setStretchLastSection(true);

	m_pFoilDelegate = new FoilTableDelegate(this);
	m_pFoilDelegate->m_pAFoil = this;
	m_pctrlFoilTable->setItemDelegate(m_pFoilDelegate);
	m_pFoilDelegate->m_pFoilModel = m_pFoilModel;

/*	int unitwidth = (int)(750.0/16.0);
	m_pctrlFoilTable->setColumnWidth(0, 3*unitwidth);
	for(int i=1; i<16; i++)		m_pctrlFoilTable->setColumnWidth(i, unitwidth);*/
	m_pctrlFoilTable->setColumnHidden(9, true);
	m_pctrlFoilTable->setColumnHidden(10, true);
	m_pctrlFoilTable->setColumnHidden(11, true);


	m_precision = new int[16];
	m_precision[0]  = 2;
	m_precision[1]  = 2;
	m_precision[2]  = 2;
	m_precision[3]  = 2;
	m_precision[4]  = 2;
	m_precision[5]  = 0;
	m_precision[6]  = 2;
	m_precision[7]  = 2;
	m_precision[8]  = 2;
	m_precision[9]  = 2;
	m_precision[10] = 2;
	m_precision[11] = 2;
	m_precision[12] = 2;
	m_precision[13] = 2;
	m_precision[14] = 2;
	m_precision[15] = 2;

	m_pFoilDelegate->m_Precision = m_precision;
//	connect(m_pFoilDelegate,  SIGNAL(closeEditor(QWidget *)), this, SLOT(OnCellChanged(QWidget *)));

	connect(this, SIGNAL(projectModified()), (MainFrame*)s_pMainFrame, SLOT(onProjectModified()));
}


/**
 * Selects the specified foil in the table of Foil objects. This will highlight the corresponding row.
 * @param pFoil
 */
void QAFoil::selectFoil(Foil* pFoil)
{
	int i;

	if(pFoil)
	{
		QModelIndex ind;
		QString FoilName;

		for(i=0; i< m_pFoilModel->rowCount(); i++)
		{
			ind = m_pFoilModel->index(i, 0, QModelIndex());
			FoilName = ind.model()->data(ind, Qt::EditRole).toString();

			if(FoilName == pFoil->m_FoilName)
			{
				m_pctrlFoilTable->selectRow(i);
				break;
			}
		}
	}
	else
	{
		m_pctrlFoilTable->selectRow(0);
	}
	Foil::setCurFoil(pFoil);
}



/**
 * Initializes the Foil table, the QWidget and the QAction objects from the data.
 * Selects the current foil in the table
 */
void QAFoil::setAFoilParams()
{
	setTableFont();
	fillFoilTable();

	selectFoil(Foil::curFoil());
	setControls();
}


/**
 * Turns on or off the display of the current Foil object.
 * @param pFoil a pointer to the Foil object to show
 * @param bShow the new visibility status of the Foil
 */
void QAFoil::showFoil(Foil* pFoil, bool bShow)
{
	if(!pFoil) return;
	Foil::curFoil()->m_bVisible = bShow;
	emit projectModified();
}


/**
 * Copies the current SplineFoil object to a new SplineFoil object and pushes it on the stack.
 */
void QAFoil::takePicture()
{
	//clear the downstream part of the stack which becomes obsolete
	clearStack(m_StackPos);

	// append a copy of the current object
	m_UndoStack.append(SplineFoil(m_pSF));

	// the new current position is the top of the stack
	m_StackPos = m_UndoStack.size()-1;

	m_bStored = true;

	MainFrame *pMainFrame = (MainFrame*)s_pMainFrame;
	if(s_pMainFrame && pMainFrame->UndoAFoilAct && pMainFrame->RedoAFoilAct)
	{
		pMainFrame->UndoAFoilAct->setEnabled(m_StackPos>0);
		pMainFrame->RedoAFoilAct->setEnabled(m_StackPos<m_UndoStack.size()-1);
	}
}



/**
 * Restores a SplineFoil definition from the current position in the stack.
 */
void QAFoil::setPicture()
{
	SplineFoil SF = m_UndoStack.at(m_StackPos);
	m_pSF->copy(&SF);
	m_pSF->m_Intrados.splineKnots();
	m_pSF->m_Intrados.splineCurve();
	m_pSF->m_Extrados.splineKnots();
	m_pSF->m_Extrados.splineCurve();
	m_pSF->updateSplineFoil();

	m_p2DWidget->update();;
}


/**
 * The user has requested to Undo the last modification to the SplineFoil object.
 */
void QAFoil::onUndo()
{
	MainFrame *pMainFrame = (MainFrame*)s_pMainFrame;

	if(m_StackPos>0)
	{
		m_StackPos--;
		setPicture();
		pMainFrame->UndoAFoilAct->setEnabled(m_StackPos>0);
		pMainFrame->RedoAFoilAct->setEnabled(m_StackPos<m_UndoStack.size()-1);
	}
	else
	{
		//nothing to restore
	}
}


/**
 *The user has requested a Redo operation after an undo.
 */
void QAFoil::onRedo()
{
	MainFrame *pMainFrame = (MainFrame*)s_pMainFrame;
	if(m_StackPos<m_UndoStack.size()-1)
	{
		m_StackPos++;
		setPicture();
		pMainFrame->UndoAFoilAct->setEnabled(m_StackPos>0);
		pMainFrame->RedoAFoilAct->setEnabled(m_StackPos<m_UndoStack.size()-1);
	}
}


/**
  * Clears the stack starting at a given position.
  * @param the first stack element to remove
  */
void QAFoil::clearStack(int pos)
{
	for(int il=m_UndoStack.size()-1; il>pos; il--)
	{
		m_UndoStack.removeAt(il);     // remove from the stack
	}
	m_StackPos = m_UndoStack.size()-1;
}




/**
 * The user has requested that the width of the columns of the Foil table be reset to default values.
 */
void QAFoil::onResetColumnWidths()
{
	int unitwidth = (int)((double)m_pctrlFoilTable->width()/16.0);
	m_pctrlFoilTable->setColumnWidth(0, 3*unitwidth);
	for(int i=1; i<16; i++) m_pctrlFoilTable->setColumnWidth(i, unitwidth);
	m_pctrlFoilTable->setColumnHidden(9, true);
	m_pctrlFoilTable->setColumnHidden(10, true);
	m_pctrlFoilTable->setColumnHidden(11, true);
}

/**
 * The user has requested the lanuch of the interface to show or hide the columns of the Foil table.
 */
void QAFoil::onAFoilTableColumns()
{
	AFoilTableDlg dlg((MainFrame*)s_pMainFrame);

	dlg.m_bFoilName    = !m_pctrlFoilTable->isColumnHidden(0);
	dlg.m_bThickness   = !m_pctrlFoilTable->isColumnHidden(1);
	dlg.m_bThicknessAt = !m_pctrlFoilTable->isColumnHidden(2);
	dlg.m_bCamber      = !m_pctrlFoilTable->isColumnHidden(3);
	dlg.m_bCamberAt    = !m_pctrlFoilTable->isColumnHidden(4);
	dlg.m_bPoints      = !m_pctrlFoilTable->isColumnHidden(5);
	dlg.m_bTEFlapAngle = !m_pctrlFoilTable->isColumnHidden(6);
	dlg.m_bTEXHinge    = !m_pctrlFoilTable->isColumnHidden(7);
	dlg.m_bTEYHinge    = !m_pctrlFoilTable->isColumnHidden(8);
	dlg.m_bLEFlapAngle = !m_pctrlFoilTable->isColumnHidden(9);
	dlg.m_bLEXHinge    = !m_pctrlFoilTable->isColumnHidden(10);
	dlg.m_bLEYHinge    = !m_pctrlFoilTable->isColumnHidden(11);

	dlg.initDialog();

	if(dlg.exec()==QDialog::Accepted)
	{
		m_pctrlFoilTable->setColumnHidden(0,  !dlg.m_bFoilName);
		m_pctrlFoilTable->setColumnHidden(1,  !dlg.m_bThickness);
		m_pctrlFoilTable->setColumnHidden(2,  !dlg.m_bThicknessAt);
		m_pctrlFoilTable->setColumnHidden(3,  !dlg.m_bCamber);
		m_pctrlFoilTable->setColumnHidden(4,  !dlg.m_bCamberAt);
		m_pctrlFoilTable->setColumnHidden(5,  !dlg.m_bPoints);
		m_pctrlFoilTable->setColumnHidden(6,  !dlg.m_bTEFlapAngle);
		m_pctrlFoilTable->setColumnHidden(7,  !dlg.m_bTEXHinge);
		m_pctrlFoilTable->setColumnHidden(8,  !dlg.m_bTEYHinge);
		m_pctrlFoilTable->setColumnHidden(9,  !dlg.m_bLEFlapAngle);
		m_pctrlFoilTable->setColumnHidden(10, !dlg.m_bLEXHinge);
		m_pctrlFoilTable->setColumnHidden(11, !dlg.m_bLEYHinge);
	}
}



/**
 * The client area has been resized. Update the column widths.
 * @param event the QResizeEvent.
 */
void QAFoil::resizeEvent(QResizeEvent *event)
{
	int ncol = m_pctrlFoilTable->horizontalHeader()->count() - m_pctrlFoilTable->horizontalHeader()->hiddenSectionCount();
	//add 1 to get double width for the name
	ncol++;

	//get column width and spare 10% for horizontal header
	int unitwidth = (int)((double)(m_pctrlFoilTable->width())/(double)ncol/1.1);

	m_pctrlFoilTable->setColumnWidth(0, 2*unitwidth);
	for(int i=1; i<16; i++)	m_pctrlFoilTable->setColumnWidth(i, unitwidth);
	event->accept();
}



/** Sets the display font for the Foil table using the default defined in the MainFrame class/ */
void QAFoil::setTableFont()
{
	m_pctrlFoilTable->setFont(Settings::s_TableFont);
}



Foil* QAFoil::addNewFoil(Foil *pFoil)
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
		emit projectModified();
		return pFoil;
	}
	return NULL;
}


void QAFoil::initDialog(void *p2DWidget, QList<void*> *poaFoil, void *pXFoil)
{
	m_poaFoil = poaFoil;
	m_pXFoil = pXFoil;
	m_p2DWidget = (Direct2dDesign*)p2DWidget;
	m_p2DWidget->setObjects(m_pBufferFoil, m_pSF, m_poaFoil);
}

