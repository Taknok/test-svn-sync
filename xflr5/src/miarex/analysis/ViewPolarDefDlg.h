/****************************************************************************

	ViewPolarDefDlg Class
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

#ifndef VIEWPOLARDEFDLG_H
#define VIEWPOLARDEFDLG_H

#include <QDialog>
#include <QTreeView>
#include <QStandardItemModel>

#include <WPolar.h>
#include "../objects/Plane.h"
#include "../design/EditObjectDelegate.h"


class ViewPolarDefDlg : public QDialog
{
	Q_OBJECT

	friend class Wing;
	friend class Plane;
	friend class QMiarex;
	friend class WPolar;

private slots:
	void OnOK();

public:
	ViewPolarDefDlg(QWidget *pParent=NULL);

	void showEvent(QShowEvent *event);
	void hideEvent(QHideEvent *event);
	void resizeEvent(QResizeEvent *event);
	void keyPressEvent(QKeyEvent *event);

	void initDialog(Plane *pPlane, WPolar *pWPolar);
	void setupLayout();
	void setWPolarName();
	void showWPolar();
	void FillControlFields(QList<QStandardItem *> stabControlFolder);
	void ReadData();
	void readViewLevel(QModelIndex indexLevel);
	void readControlFields(QModelIndex indexLevel);

	QList<QStandardItem *> prepareRow(const QString &object, const QString &field="", const QString &value="",  const QString &unit="");
	QList<QStandardItem *> prepareBoolRow(const QString &first, const QString &second, const bool &third);
	QList<QStandardItem *> prepareIntRow(const QString &object, const QString &field, const int &value);
	QList<QStandardItem *> prepareDoubleRow(const QString &object, const QString &field, const double &value,  const QString &unit);


	static QPoint s_Position;   /**< the position on the client area of the dialog's topleft corner */
	static QSize s_Size;	    /**< the window size in the client area */

private:
	static WPolar s_StabPolar;

	WPolar * m_pWPolar;
	Plane * m_pPlane;
	QTreeView * m_pStruct;
	QStandardItemModel *m_pModel;
	EditObjectDelegate *m_pDelegate;


	QPushButton *pOKButton, *pCancelButton;

};

#endif // VIEWPOLARDEFDLG_H




