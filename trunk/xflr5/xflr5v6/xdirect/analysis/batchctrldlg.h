/****************************************************************************

    BatchCtrlDlg Class
    Copyright (C) André Deperrois

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


#pragma once

#include <QModelIndex>
#include <QSettings>
#include "batchabstractdlg.h"

class DoubleEdit;
class CPTableView;
class ActionDelegate;
class ActionItemModel;


/**
 * @brief Extension of the BacthThreadDlg class to include flaps and other control parameters
 */
class BatchCtrlDlg : public BatchAbstractDlg
{
    Q_OBJECT
    friend class XDirect;
    friend class MainFrame;
    friend class XFoilTask;

    public:
        BatchCtrlDlg(QWidget *pParent=nullptr);
        ~BatchCtrlDlg();

        void initDialog() override;

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:
        void connectSignals();
        void setupLayout();
        void readParams() override;
        void startAnalyses();
        void customEvent(QEvent * pEvent) override;
        void fillReModel();
        void sortRe();

    private slots:
        void onAnalyze() override;
        void onDelete();
        void onInsertBefore();
        void onInsertAfter();
        void onCellChanged(QModelIndex topLeft, QModelIndex botRight);
        void onReTableClicked(QModelIndex index);

    private:
        int m_nTasks, m_TaskCounter;

        DoubleEdit *m_pdeXHinge, *m_pdeYHinge;
        DoubleEdit *m_pdeAngleMin, *m_pdeAngleMax, *m_pdeAngleDelta;
        CPTableView *m_pcptReTable;
        ActionItemModel *m_pReModel;
        ActionDelegate *m_pFloatDelegate;

        QAction *m_pInsertBeforeAct, *m_pInsertAfterAct, *m_pDeleteAct;


        static double s_XHinge, s_YHinge;
        static double s_AngleMin, s_AngleMax, s_AngleDelta;
};



