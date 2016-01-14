/****************************************************************************

	GL3dWingDlg Class
	Copyright (C) 2009 Andre Deperrois adeperrois@xflr5.com
 
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

#ifndef GL3DWINGDLG_H
#define GL3DWINGDLG_H

#include <QDialog>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QSlider>
#include <QSettings>
#include <QLabel>
#include <QRadioButton>
#include <QToolButton>
#include <QStandardItemModel>
#include <QTableView>
#include <QPushButton>
#include <QSplitter>

#include "WingDelegate.h"
#include "../../objects/ArcBall.h"
#include "threedwidget.h"
#include "../../misc/DoubleEdit.h"
#include "../../misc/ColorButton.h"
#include "../../objects/Wing.h"



class GL3dWingDlg: public QDialog
{
	Q_OBJECT

	friend class MainFrame;
	friend class QMiarex;
	friend class ThreeDWidget;
	friend class Wing;
	friend class GLLightDlg;
	friend class PlaneDlg;
	friend class WingDelegate;

public:
	GL3dWingDlg(QWidget *pParent=NULL);
	~GL3dWingDlg();

	bool intersectObject(CVector AA,  CVector U, CVector &I);

private slots:
	void onAxes();
	void on3DReset();
	void onCheckViewIcons();
	void onSurfaces();
	void onOutline();
	void onPanels();
	void onFoilNames();
	void onShowMasses();

	void onOK();
	void onDescriptionChanged();
	void onCellChanged(QWidget *);
	void onItemClicked(const QModelIndex &index);
	void onWingColor();
	void onSide();
	void onSymetric();
	void onInsertBefore();
	void onInsertAfter();
	void onDeleteSection();
	void onResetSection();
	void onResetMesh();
	void onScaleWing();
	void onInertia();
	void onImportWing();
	void onExportWing();


private:
	void reject();

	void keyPressEvent(QKeyEvent *event);
	void resizeEvent(QResizeEvent *event);
	void showEvent(QShowEvent *event);
	void contextMenuEvent(QContextMenuEvent *event);

	bool checkWing();
	void createXPoints(int NXPanels, int XDist, Foil *pFoilA, Foil *pFoilB, double *xPointA, double *xPointB, int &NXLead, int &NXFlap);
	void computeGeometry();

	void setWingData();
	void fillDataTable();
	void fillTableRow(int row);
	void readParams();
	void readSectionData(int sel);
	void setCurrentSection(int section);
	void setScale();
	int VLMGetPanelTotal();
	bool VLMSetAutoMesh(int total=0);

	bool initDialog(Wing *pWing);
	void connectSignals();
	void setupLayout();
	void GLRenderView();
	void glCreateSectionHighlight();
	void glCallViewLists();
	void glDraw3D();
	void setWingScale();



private:
	static QList <void*> *s_poaFoil;
	static QPoint s_WindowPos;
	static QSize  s_WindowSize;
	static bool s_bWindowMaximized;


	ThreeDWidget *m_pGLWidget;

	QLineEdit *m_pctrlWingName;
	QTextEdit *m_pctrlWingDescription;
	QCheckBox *m_pctrlSymetric;
	QRadioButton *m_pctrlLeftSide, *m_pctrlRightSide;
	ColorButton *m_pctrlWingColor;
	QPushButton *m_pctrlResetMesh;
	QPushButton *m_pctrlInsertBefore, *m_pctrlInsertAfter, *m_pctrlDeleteSection;
	QLabel *m_pctrlWingSpan, *m_pctrlWingArea, *m_pctrlMAC, *m_pctrlGeomChord;
    QLabel *m_pctrlAspectRatio, *m_pctrlTaperRatio, *m_pctrlSweep, *m_pctrlNFlaps;
	QLabel *m_pctrlVLMPanels, *m_pctrl3DPanels;
	QLabel *m_pctrlProjectedArea, *m_pctrlProjectedSpan;

    QLabel *m_pctrlLength1, *m_pctrlLength2, *m_pctrlLength3, *m_pctrlLength4;
	QLabel *m_pctrlAreaUnit1, *m_pctrlAreaUnit2, * m_pctrlVolumeUnit;
	QTableView *m_pctrlWingTable;
	QStandardItemModel *m_pWingModel;
	WingDelegate *m_pWingDelegate;

	QPushButton *OKButton, *CancelButton;
    QCheckBox *m_pctrlAxes, *m_pctrlSurfaces, *m_pctrlOutline, *m_pctrlPanels, *m_pctrlFoilNames;
	QCheckBox *m_pctrlShowMasses;
	QPushButton *m_pctrlReset;

	QToolButton *m_pctrlX, *m_pctrlY, *m_pctrlZ, *m_pctrlIso;
	QAction *m_pXView, *m_pYView, *m_pZView, *m_pIsoView;

	QAction *m_pScaleWing, *m_pInertia;
	QAction *m_pExportWingAct, *m_pImportWingAct;

	QAction *m_pResetScales;
	

	QMenu *m_pContextMenu;
	QAction *m_pInsertBefore, *m_pInsertAfter, *m_pDeleteSection, *m_pResetSection;

	QWidget *m_pctrlControlsWidget;
	QSplitter *m_pLeftSideSplitter;

	Wing *m_pWing;

	bool m_bAcceptName;
	bool m_bRightSide;
	bool m_bChanged, m_bDescriptionChanged;
	bool m_bTrans;
	bool m_bStored;
	bool m_bEnableName;
	bool m_bResetglWing;
	bool m_bResetglSectionHighlight;


	int m_iSection;
	int m_GLList;


	QPoint m_MousePos;
	QPoint m_ptPopUp;
	QPoint m_LastPoint;


	CVector m_RealPopUp;

	Panel *m_pPanel;
	CVector *m_pNode;


	int  *m_precision;
};

#endif // GL3DWINGDLG_H
