/****************************************************************************

    ReListDlg Class
    Copyright (C) 2009-2016 André Deperrois

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

#include <QDialog>

#include <QPushButton>
#include <QTableView>
#include <QStandardItemModel>
#include <QDialogButtonBox>

class FloatEditDelegate;


class ReListDlg : public QDialog
{
    Q_OBJECT

    public:
        ReListDlg(QWidget *pParent=nullptr);
        ~ReListDlg() override;

        QSize sizeHint() const override {return QSize(600,700);}

        void initDialog(QVector<double> ReList, QVector<double> MachList, QVector<double> NCritList);

        QVector<double> const &ReList()    const {return m_ReList;}
        QVector<double> const &MachList()  const {return m_MachList;}
        QVector<double> const &NCritList() const {return m_NCritList;}

    private slots:
        void onDelete();
        void onInsert();
        void onOK();
        void onCellChanged(QModelIndex topLeft, QModelIndex botRight);
        void onButton(QAbstractButton *pButton);

    private:
        void fillReModel();
        void setupLayout();
        void sortData();
        void sortRe();
        void keyPressEvent(QKeyEvent *pEvent) override;
        void resizeEvent(QResizeEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;

    private:
        QPushButton *m_ppbInsert, *m_ppbDelete;
        QDialogButtonBox *m_pButtonBox;

        QTableView *m_ptvRe;
        QStandardItemModel *m_pReModel;
        FloatEditDelegate *m_pFloatDelegate;

        QVector<double> m_ReList;
        QVector<double> m_MachList;
        QVector<double> m_NCritList;
};


