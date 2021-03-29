/****************************************************************************

    ReListDlg Class
    Copyright (C) 2009-2016 Andre Deperrois

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

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QStringList>

#include "relistdlg.h"
#include <misc/options/settings.h>
#include <misc/text/doubleedit.h>
#include <misc/text/floateditdelegate.h>

ReListDlg::ReListDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("Reynolds Number List"));

    m_pFloatDelegate = nullptr;

    setupLayout();

    connect(m_ppbDelete, SIGNAL(clicked()), SLOT(onDelete()));
    connect(m_ppbInsert, SIGNAL(clicked()), SLOT(onInsert()));
}


ReListDlg::~ReListDlg()
{
    if(m_pFloatDelegate) delete m_pFloatDelegate;
}


void ReListDlg::initDialog(QVector<double> ReList, QVector<double> MachList, QVector<double> NCritList)
{
    m_ReList.clear();
    m_MachList.clear();
    m_NCritList.clear();

    m_ReList.append(ReList);
    m_MachList.append(MachList);
    m_NCritList.append(NCritList);

    m_pReModel = new QStandardItemModel(this);
    m_pReModel->setRowCount(5);//temporary
    m_pReModel->setColumnCount(3);

    m_pReModel->setColumnCount(3);
    m_pReModel->setHeaderData(0, Qt::Horizontal, QObject::tr("Re"));
    m_pReModel->setHeaderData(1, Qt::Horizontal, QObject::tr("Mach"));
    m_pReModel->setHeaderData(2, Qt::Horizontal, QObject::tr("NCrit"));

    m_ptvRe->setModel(m_pReModel);

    QHeaderView *pHorizontalHeader = m_ptvRe->horizontalHeader();
    pHorizontalHeader->setStretchLastSection(true);

    m_pFloatDelegate = new FloatEditDelegate(this);
    m_ptvRe->setItemDelegate(m_pFloatDelegate);

    QVector<int> precision = {0,2,2};
    m_pFloatDelegate->setPrecision(precision);

    //    connect(m_pFloatDelegate, SIGNAL(closeEditor(QWidget *)), this, SLOT(onCellChanged(QWidget *)));
    connect(m_pReModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(onCellChanged(QModelIndex ,QModelIndex)));

    fillReModel();
}


void ReListDlg::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_pButtonBox->hasFocus()) m_pButtonBox->setFocus();
            else                          accept();

            break;
        }
        case Qt::Key_Escape:
        {
            reject();
            return;
        }
        default:
            pEvent->ignore();
    }
}


void ReListDlg::onCellChanged(QModelIndex topLeft, QModelIndex )
{
    if(topLeft.column()==0)
        sortData();
}


void ReListDlg::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        QHBoxLayout * pListLayout = new QHBoxLayout;
        {
            m_ptvRe = new QTableView(this);
            m_ptvRe->setFont(Settings::s_TableFont);
            m_ptvRe->setSelectionBehavior(QAbstractItemView::SelectRows);
            m_ptvRe->setEditTriggers(QAbstractItemView::AllEditTriggers);
            m_ptvRe->setWindowTitle(QObject::tr("Re List"));

            QVBoxLayout *pCommandButtons = new QVBoxLayout;
            {
                m_ppbInsert    = new QPushButton(tr("Insert"));
                m_ppbDelete    = new QPushButton(tr("Delete"));

                pCommandButtons->addWidget(m_ppbInsert);
                pCommandButtons->addWidget(m_ppbDelete);
                pCommandButtons->addStretch(1);
            }
            pListLayout->addWidget(m_ptvRe);
            pListLayout->addLayout(pCommandButtons);
        }
        m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        {
            connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
        }

        pMainLayout->addLayout(pListLayout);
        pMainLayout->addWidget(m_pButtonBox);
        setLayout(pMainLayout);
    }
    setLayout(pMainLayout);
}


void ReListDlg::onButton(QAbstractButton *pButton)
{
    if (     m_pButtonBox->button(QDialogButtonBox::Ok)     == pButton)  onOK();
    else if (m_pButtonBox->button(QDialogButtonBox::Cancel) == pButton)  reject();
}

void ReListDlg::fillReModel()
{
    m_pReModel->setRowCount(m_ReList.count());

    m_pReModel->blockSignals(true);
    for (int i=0; i<m_ReList.count(); i++)
    {
        QModelIndex Xindex = m_pReModel->index(i, 0, QModelIndex());
        m_pReModel->setData(Xindex, m_ReList[i]);

        QModelIndex Yindex =m_pReModel->index(i, 1, QModelIndex());
        m_pReModel->setData(Yindex, m_MachList[i]);

        QModelIndex Zindex =m_pReModel->index(i, 2, QModelIndex());
        m_pReModel->setData(Zindex, m_NCritList[i]);
    }
    m_pReModel->blockSignals(false);
    m_ptvRe->resizeRowsToContents();
}


void ReListDlg::onDelete()
{
    QModelIndex index = m_ptvRe->currentIndex();
    int sel = index.row();

    if(sel<0 || sel>=m_ReList.count()) return;

    m_ReList.removeAt(sel);
    m_MachList.removeAt(sel);
    m_NCritList.removeAt(sel);


    fillReModel();
}


void ReListDlg::onInsert()
{
    int sel = m_ptvRe->currentIndex().row();
    if(sel<0)
    {
        m_ReList.prepend(0.0);
        m_MachList.prepend(0.0);
        m_NCritList.prepend(0.0);
    }
    else
    {
        m_ReList.insert(sel, 0.0);
        m_MachList.insert(sel, 0.0);
        m_NCritList.insert(sel, 0.0);
    }

    if(sel>0)        m_ReList[sel]    = (m_ReList[sel-1]+m_ReList[sel+1]) /2.0;
    else if(sel==0)  m_ReList[sel]    = m_ReList[sel+1]                   /2.0;
    else             m_ReList[0]      = 100000.0;

    if(sel>=0)
    {
        m_MachList[sel]  = m_MachList[sel+1];
        m_NCritList[sel] = m_NCritList[sel+1];
    }
    else
    {
        sel = 0;
        m_MachList[sel]  = 0.0;
        m_NCritList[sel] = 0.0;
    }

    fillReModel();

    QModelIndex index = m_pReModel->index(sel, 0, QModelIndex());
    m_ptvRe->setCurrentIndex(index);
    m_ptvRe->openPersistentEditor(index);
}


void ReListDlg::onOK()
{
    for (int i=0; i<m_ReList.count(); i++)
    {
        QModelIndex index = m_pReModel->index(i, 0, QModelIndex());
        m_ReList[i] = index.data().toDouble();

        index = m_pReModel->index(i, 1, QModelIndex());
        m_MachList[i] = index.data().toDouble();

        index = m_pReModel->index(i, 2, QModelIndex());
        m_NCritList[i] = index.data().toDouble();

    }
    done(1);
}



void ReListDlg::sortData()
{
    int i;
    m_ReList.clear();
    m_MachList.clear();
    m_NCritList.clear();

    for (i=0; i<m_pReModel->rowCount(); i++)
    {
        m_ReList.append(m_pReModel->index(i, 0, QModelIndex()).data().toDouble());
        m_MachList.append(m_pReModel->index(i, 1, QModelIndex()).data().toDouble());
        m_NCritList.append(m_pReModel->index(i, 2, QModelIndex()).data().toDouble());
    }
    sortRe();

    //and fill back the model
    fillReModel();
}


/**
* Bubble sort algorithm for the arrays of Reynolds, Mach and NCrit numbers.
* The arrays are sorted by crescending Re numbers.
*/
void ReListDlg::sortRe()
{
    int indx, indx2;
    double Retemp, Retemp2;
    double Matemp, Matemp2;
    double NCtemp, NCtemp2;
    int flipped;

    if (m_ReList.size() <= 1) return;

    indx = 1;
    do
    {
        flipped = 0;
        for (indx2 = m_ReList.size() - 1; indx2 >= indx; --indx2)
        {
            Retemp  = m_ReList[indx2];
            Retemp2 = m_ReList[indx2 - 1];
            Matemp  = m_MachList[indx2];
            Matemp2 = m_MachList[indx2 - 1];
            NCtemp  = m_NCritList[indx2];
            NCtemp2 = m_NCritList[indx2 - 1];
            if (Retemp2> Retemp)
            {
                m_ReList[indx2 - 1]    = Retemp;
                m_ReList[indx2]        = Retemp2;
                m_MachList[indx2 - 1]  = Matemp;
                m_MachList[indx2]      = Matemp2;
                m_NCritList[indx2 - 1] = NCtemp;
                m_NCritList[indx2]     = NCtemp2;
                flipped = 1;
            }
        }
    } while ((++indx < m_ReList.size()) && flipped);
}


void ReListDlg::showEvent(QShowEvent *)
{
    resizeEvent(nullptr);
}



void ReListDlg::resizeEvent(QResizeEvent *)
{
    int w3 = int(double(m_ptvRe->width())*0.9/3.0);
    m_ptvRe->setColumnWidth(0, w3);
    m_ptvRe->setColumnWidth(1, w3);
}









