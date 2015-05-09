/****************************************************************************

	ViewObjectDlg Class
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


#ifndef VIEWOBJECTDLG_H
#define VIEWOBJECTDLG_H

#include <QDialog>
#include <QTreeView>
#include <QStandardItemModel>
#include <QToolButton>
#include <threedwidget.h>
#include "../../objects/Plane.h"
#include "ViewObjectDelegate.h"



class ViewObjectDlg : public QDialog
{
	Q_OBJECT

	friend class Wing;
	friend class Plane;
	friend class QMiarex;

public:
	ViewObjectDlg(QWidget *pParent = NULL);

	void showEvent(QShowEvent *event);
	void hideEvent(QHideEvent *event);
	void resizeEvent(QResizeEvent *event);
	void keyPressEvent(QKeyEvent *event);

	bool IntersectObject(CVector AA,  CVector U, CVector &I);
	void Connect();
	void GLDraw3D();
	void GLRenderView();
	void GLCreateWingSectionHighlight(Wing *pWing);
	void GLCreateBodyFrameHighlight(Body *pBody, CVector bodyPos, int iFrame);
	void initDialog(Plane *pPlane);
	void setupLayout();
	void fillPlaneTreeView();
	void fillWingTreeView(int iw, QList<QStandardItem *> &planeRootItem);
	void fillBodyTreeView(QStandardItem *planeRootItem);
	void fillPlaneMetaData(QStandardItem *item);
	void readPlaneTree();
	void readViewLevel(QModelIndex indexLevel);
	void readBodyTree(Body *pBosy, QModelIndex indexLevel);
	void readWingTree(Wing *pWing, CVector &pos, double &tiltAngle, QModelIndex indexLevel);
	void readInertiaTree(double &volumeMass, QList<PointMass *> &pointMasses, QModelIndex indexLevel);
	void readVectorTree(CVector &V, QModelIndex indexLevel);
	void readWingSectionTree(Wing *pWing, QModelIndex indexLevel);
	void readPointMassTree(PointMass *ppm, QModelIndex indexLevel);
	void readBodyFrameTree(Body *pBody, Frame *pFrame, QModelIndex indexLevel);
	void highlightSelection(const QModelIndex &indexSel);

	QList<QStandardItem *> prepareRow(const QString &first, const QString &second="", const QString &third="",  const QString &fourth="");
	QList<QStandardItem *> prepareBoolRow(const QString &first, const QString &second, const bool &third);
	QList<QStandardItem *> prepareIntRow(const QString &first, const QString &second, const int &third);
	QList<QStandardItem *> prepareDoubleRow(const QString &first, const QString &second, const double &third,  const QString &fourth);

private slots:
	void OnOK();
	void OnAxes();
	void On3DReset();
	void OnRedraw();
	void OnCellChanged(QWidget *);
	void OnItemClicked(const QModelIndex &index);

	void OnSurfaces();
	void OnOutline();
	void OnPanels();
	void OnFoilNames();
	void OnShowMasses();

private:
	void reject();


public:
	static bool s_bWindowMaximized;
	static QPoint s_WindowPosition;   /**< the position on the client area of the dialog's topleft corner */
	static QSize s_WindowSize;	 /**< the window size in the client area */

private:
	Plane * m_pPlane;
	QTreeView * m_pStruct;
	ViewObjectDelegate *m_pDelegate;
	QStandardItemModel *m_pModel;

	ThreeDWidget *m_pGLWidget;

	QCheckBox *m_pctrlAxes, *m_pctrlLight, *m_pctrlSurfaces, *m_pctrlOutline, *m_pctrlPanels;
	QCheckBox *m_pctrlFoilNames, *m_pctrlVortices, *m_pctrlPanelNormals, *m_pctrlShowMasses;

	QAction *m_pXView, *m_pYView, *m_pZView, *m_pIsoView;
	QToolButton *m_pctrlX, *m_pctrlY, *m_pctrlZ, *m_pctrlIso;

	QPushButton *m_pctrlRedraw;
	QPushButton *m_pctrlReset;
	QPushButton *pOKButton;
	QSlider *m_pctrlClipPlanePos;

	bool m_bChanged;
	bool m_bResetglSectionHighlight;
	bool m_bResetglPlane, m_bResetglBody;

	XFLR5::enumWingType m_enumActiveWingType;
	int m_iActiveSection, m_iActiveFrame;
};

#endif // VIEWOBJECTDLG_H
