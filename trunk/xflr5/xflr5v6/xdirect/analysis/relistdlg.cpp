/****************************************************************************

    ReListDlg Class
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


#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QStringList>
#include <QMenu>

#include "relistdlg.h"

#include <xflwidgets/customwts/doubleedit.h>
#include <xflwidgets/customwts/cptableview.h>
#include <xflwidgets/customwts/actiondelegate.h>
#include <xflwidgets/customwts/actionitemmodel.h>


QByteArray ReListDlg::s_WindowGeometry;


ReListDlg::ReListDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("Reynolds Number List"));

    m_pInsertBeforeAct	= new QAction(tr("Insert before"), this);
    m_pInsertAfterAct	= new QAction(tr("Insert after"), this);
    m_pDeleteAct	    = new QAction(tr("Delete"), this);

    setupLayout();
    connectSignals();
}


ReListDlg::~ReListDlg()
{
    if(m_pFloatDelegate) delete m_pFloatDelegate;
}


void ReListDlg::connectSignals()
{
    connect(m_pcptReTable, SIGNAL(clicked(QModelIndex)),                 SLOT(onReTableClicked(QModelIndex)));
    connect(m_pReModel,    SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(onCellChanged(QModelIndex,QModelIndex)));

    connect(m_pDeleteAct,       SIGNAL(triggered(bool)), SLOT(onDelete()));
    connect(m_pInsertBeforeAct, SIGNAL(triggered(bool)), SLOT(onInsertBefore()));
    connect(m_pInsertAfterAct,  SIGNAL(triggered(bool)), SLOT(onInsertAfter()));
}


void ReListDlg::initDialog(QVector<double> ReList, QVector<double> MachList, QVector<double> NCritList)
{
    m_ReList.clear();
    m_MachList.clear();
    m_NCritList.clear();

    m_ReList.append(ReList);
    m_MachList.append(MachList);
    m_NCritList.append(NCritList);

    fillReModel();
}


void ReListDlg::showEvent(QShowEvent *)
{
    restoreGeometry(s_WindowGeometry);
    m_pButtonBox->setFocus();
}


void ReListDlg::hideEvent(QHideEvent*)
{
    s_WindowGeometry = saveGeometry();
}


void ReListDlg::resizeEvent(QResizeEvent *)
{
    double w = double(m_pcptReTable->width())/100.0;
    m_pcptReTable->setColumnWidth(0,int(25.0*w));
    m_pcptReTable->setColumnWidth(1,int(25.0*w));
    m_pcptReTable->setColumnWidth(2,int(25.0*w));
    m_pcptReTable->setColumnWidth(3,int(15.0*w));
    QHeaderView *pHorizontalHeader = m_pcptReTable->horizontalHeader();
    pHorizontalHeader->setStretchLastSection(true);
}


/** @todo never called? */
void ReListDlg::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_pButtonBox->hasFocus()) m_pButtonBox->setFocus();
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


void ReListDlg::setupLayout()
{
    QHBoxLayout *pMainLayout = new QHBoxLayout(this);
    {
        m_pcptReTable = new CPTableView(this);
        {
            m_pcptReTable->setEditable(true);
            m_pcptReTable->setEditTriggers(QAbstractItemView::CurrentChanged |
                                           QAbstractItemView::DoubleClicked |
                                           QAbstractItemView::SelectedClicked |
                                           QAbstractItemView::EditKeyPressed |
                                           QAbstractItemView::AnyKeyPressed);

            m_pReModel = new ActionItemModel(this);
            m_pReModel->setRowCount(1);//temporary
            m_pReModel->setColumnCount(4);
            m_pReModel->setActionColumn(3);
            m_pReModel->setHeaderData(0, Qt::Horizontal, QObject::tr("Re"));
            m_pReModel->setHeaderData(1, Qt::Horizontal, QObject::tr("Mach"));
            m_pReModel->setHeaderData(2, Qt::Horizontal, QObject::tr("NCrit"));
            m_pReModel->setHeaderData(3, Qt::Horizontal, QObject::tr("Actions"));

            m_pcptReTable->setModel(m_pReModel);

            int n = m_pReModel->actionColumn();
            QHeaderView *pHHeader = m_pcptReTable->horizontalHeader();
            pHHeader->setSectionResizeMode(n, QHeaderView::Stretch);
            pHHeader->resizeSection(n, 1);

            m_pFloatDelegate = new ActionDelegate(this);
            m_pFloatDelegate->setActionColumn(3);
            QVector<int>m_Precision = {0,2,2};
            m_pFloatDelegate->setDigits(m_Precision);
            m_pcptReTable->setItemDelegate(m_pFloatDelegate);
        }

        QVBoxLayout *pRightSideLayout = new QVBoxLayout;
        {
            m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Vertical, this);
            {
                connect(m_pButtonBox, &QDialogButtonBox::accepted, this, &ReListDlg::accept);
                connect(m_pButtonBox, &QDialogButtonBox::rejected, this, &ReListDlg::reject);
            }
            pRightSideLayout->addStretch(30);
            pRightSideLayout->addWidget(m_pButtonBox);
        }

        pMainLayout->addWidget(m_pcptReTable);
        pMainLayout->addLayout(pRightSideLayout);
    }
    setLayout(pMainLayout);
}


void ReListDlg::fillReModel()
{
    m_pReModel->setRowCount(m_ReList.count());

    m_pReModel->blockSignals(true);
    QModelIndex index;
    for (int i=0; i<m_ReList.count(); i++)
    {
        index = m_pReModel->index(i, 0, QModelIndex());
        m_pReModel->setData(index, m_ReList.at(i));

        index =m_pReModel->index(i, 1, QModelIndex());
        m_pReModel->setData(index, m_MachList.at(i));

        index = m_pReModel->index(i, 2, QModelIndex());
        m_pReModel->setData(index, m_NCritList.at(i));

        index = m_pReModel->index(i, 3, QModelIndex());
        m_pReModel->setData(index, QString("..."));
    }
    m_pReModel->blockSignals(false);

/*    for(int i=0; i<m_pReModel->rowCount(); i++)
    {
        QModelIndex index = m_pReModel->index(i, 0, QModelIndex());
        qDebug("%11f", m_pReModel->data(index).toDouble());
    }*/
    m_pcptReTable->resizeRowsToContents();
}


void ReListDlg::onDelete()
{
    QModelIndex index = m_pcptReTable->currentIndex();
    int sel = index.row();

    if(sel<0 || sel>=m_ReList.count()) return;

    m_ReList.removeAt(sel);
    m_MachList.removeAt(sel);
    m_NCritList.removeAt(sel);

    fillReModel();
}


void ReListDlg::onInsertBefore()
{
    int sel = m_pcptReTable->currentIndex().row();
    m_ReList.insert(sel, 0.0);
    m_MachList.insert(sel, 0.0);
    m_NCritList.insert(sel, 0.0);

    if     (sel>0)   m_ReList[sel]    = (m_ReList.at(sel-1)+m_ReList.at(sel+1)) /2.0;
    else if(sel==0)  m_ReList[sel]    =  m_ReList.at(sel+1)                     /2.0;
    else             m_ReList[0]      = 100000.0;

    if(sel>=0)
    {
        m_MachList[sel]  = m_MachList.at(sel+1);
        m_NCritList[sel] = m_NCritList.at(sel+1);
    }
    else
    {
        sel = 0;
        m_MachList[sel]  = 0.0;
        m_NCritList[sel] = 0.0;
    }

    fillReModel();

    QModelIndex index = m_pReModel->index(sel, 0, QModelIndex());
    m_pcptReTable->setCurrentIndex(index);
    m_pcptReTable->selectRow(index.row());
}


void ReListDlg::onInsertAfter()
{
    int sel = m_pcptReTable->currentIndex().row()+1;

    m_ReList.insert(sel, 0.0);
    m_MachList.insert(sel, 0.0);
    m_NCritList.insert(sel, 0.0);

    if(sel==m_ReList.size()-1) m_ReList[sel]    = m_ReList[sel-1]*2.0;
    else if(sel>0)             m_ReList[sel]    = (m_ReList[sel-1]+m_ReList[sel+1]) /2.0;
    else if(sel==0)            m_ReList[sel]    = m_ReList[sel+1]                   /2.0;

    if(sel>0)
    {
        m_MachList[sel]  = m_MachList[sel-1];
        m_NCritList[sel] = m_NCritList[sel-1];
    }
    else
    {
        sel = 0;
        m_MachList[sel]  = 0.0;
        m_NCritList[sel] = 0.0;
    }

    fillReModel();

    QModelIndex index = m_pReModel->index(sel, 0, QModelIndex());
    m_pcptReTable->setCurrentIndex(index);
    m_pcptReTable->selectRow(index.row());
}


void ReListDlg::accept()
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


void ReListDlg::onReTableClicked(QModelIndex index)
{
    if(!index.isValid())
    {
    }
    else
    {
        m_pcptReTable->selectRow(index.row());

        switch(index.column())
        {
            case 3:
            {
                QRect itemrect = m_pcptReTable->visualRect(index);
                QPoint menupos = m_pcptReTable->mapToGlobal(itemrect.topLeft());
                QMenu *pReTableRowMenu = new QMenu(tr("Section"),this);
                pReTableRowMenu->addAction(m_pInsertBeforeAct);
                pReTableRowMenu->addAction(m_pInsertAfterAct);
                pReTableRowMenu->addAction(m_pDeleteAct);
                pReTableRowMenu->exec(menupos, m_pInsertBeforeAct);

                break;
            }
            default:
            {
                break;
            }
        }
    }
}


void ReListDlg::onCellChanged(QModelIndex topLeft, QModelIndex )
{
    if(topLeft.column()==0)
        sortData();
}


void ReListDlg::sortData()
{
    m_ReList.clear();
    m_MachList.clear();
    m_NCritList.clear();

    for (int i=0; i<m_pReModel->rowCount(); i++)
    {
        m_ReList.append(   m_pReModel->index(i, 0, QModelIndex()).data().toDouble());
        m_MachList.append( m_pReModel->index(i, 1, QModelIndex()).data().toDouble());
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
    int indx(0), indx2(0);
    double Retemp(0), Retemp2(0);
    double Matemp(0), Matemp2(0);
    double NCtemp(0), NCtemp2(0);
    int flipped(0);

    if (m_ReList.size()<=1) return;

    indx = 1;
    do
    {
        flipped = 0;
        for (indx2=m_ReList.size()-1; indx2>=indx; indx2--)
        {
            Retemp  = m_ReList.at(indx2);
            Retemp2 = m_ReList.at(indx2 - 1);
            Matemp  = m_MachList.at(indx2);
            Matemp2 = m_MachList.at(indx2 - 1);
            NCtemp  = m_NCritList.at(indx2);
            NCtemp2 = m_NCritList.at(indx2 - 1);
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





