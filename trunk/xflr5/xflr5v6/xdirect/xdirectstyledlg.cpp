/****************************************************************************

    QXDirectStyleDlg Class
    Copyright (C) 2009 Andre Deperrois

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

#include <QGridLayout>
#include <QLabel>
#include <QPushButton>

#include "xdirectstyledlg.h"
#include <xdirect/xdirect.h>
#include <misc/line/linepickerdlg.h>
#include <viewwidgets/oppointwt.h>


#include <misc/line/linebtn.h>


XDirectStyleDlg::XDirectStyleDlg(OpPointWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("XDirect Styles"));
    m_pOpPointWt = pParent;

    m_NeutralStyle  = m_pOpPointWt->m_NeutralStyle;
    m_BLStyle  = m_pOpPointWt->m_BLStyle;
    m_PressureStyle  = m_pOpPointWt->m_PressureStyle;

    setupLayout();

    m_plbNeutral->setTheStyle(m_NeutralStyle);
    m_plbBL->setTheStyle(m_BLStyle);
    m_plbPressure->setTheStyle(m_PressureStyle);
}


void XDirectStyleDlg::setupLayout()
{
    QGridLayout *pStyleLayout = new QGridLayout;
    {
        m_plbNeutral  = new LineBtn(this);
        m_plbBL       = new LineBtn(this);
        m_plbPressure = new LineBtn(this);
        QLabel *lab1 = new QLabel(tr("Neutral Line"));
        QLabel *lab2 = new QLabel(tr("Boundary Layer"));
        QLabel *lab3 = new QLabel(tr("Pressure"));
        pStyleLayout->addWidget(lab1,1,1);
        pStyleLayout->addWidget(lab2,2,1);
        pStyleLayout->addWidget(lab3,3,1);
        pStyleLayout->addWidget(m_plbNeutral,1,2);
        pStyleLayout->addWidget(m_plbBL,2,2);
        pStyleLayout->addWidget(m_plbPressure,3,2);
        connect(m_plbNeutral,  SIGNAL(clickedLB()),this, SLOT(onNeutralStyle()));
        connect(m_plbBL,       SIGNAL(clickedLB()),this, SLOT(onBLStyle()));
        connect(m_plbPressure, SIGNAL(clickedLB()),this, SLOT(onPressureStyle()));
    }

    QHBoxLayout *pCommandButtons = new QHBoxLayout;
    {
        m_ppbOKButton = new QPushButton(tr("OK"));
        QPushButton *DefaultsButton = new QPushButton(tr("Defaults"));
        QPushButton *CancelButton   = new QPushButton(tr("Cancel"));
        pCommandButtons->addStretch(1);
        pCommandButtons->addWidget(m_ppbOKButton);
        pCommandButtons->addStretch(1);
        pCommandButtons->addWidget(DefaultsButton);
        pCommandButtons->addStretch(1);
        pCommandButtons->addWidget(CancelButton);
        pCommandButtons->addStretch(1);
        connect(m_ppbOKButton, SIGNAL(clicked()),this, SLOT(accept()));
        connect(DefaultsButton, SIGNAL(clicked()),this, SLOT(onRestoreDefaults()));
        connect(CancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addLayout(pStyleLayout);
        pMainLayout->addStretch(1);
        pMainLayout->addLayout(pCommandButtons);
        pMainLayout->addStretch(1);
    }

    setLayout(pMainLayout);
}



void XDirectStyleDlg::onNeutralStyle()
{
    LinePickerDlg dlg(this);
    dlg.initDialog(m_NeutralStyle, false, false);

    if(QDialog::Accepted==dlg.exec())
    {
        m_NeutralStyle = dlg.theStyle();
        m_plbNeutral->setTheStyle(m_NeutralStyle);
    }
    m_ppbOKButton->setFocus();
}


void XDirectStyleDlg::onPressureStyle()
{
    LinePickerDlg dlg(this);
    dlg.initDialog(m_PressureStyle, false, false);

    if(QDialog::Accepted==dlg.exec())
    {
        m_PressureStyle = dlg.theStyle();
        m_plbPressure->setTheStyle(m_PressureStyle);
    }
    m_ppbOKButton->setFocus();
}


void XDirectStyleDlg::onBLStyle()
{
    LinePickerDlg dlg(this);
    dlg.initDialog(m_BLStyle, false, false);

    if(QDialog::Accepted==dlg.exec())
    {
        m_BLStyle = dlg.theStyle();
        m_plbBL->setTheStyle(m_BLStyle);
    }

    m_ppbOKButton->setFocus();
}


void XDirectStyleDlg::onRestoreDefaults()
{
    m_NeutralStyle = {true, Line::DASHDOT, 1, QColor(155,155,155), Line::NOSYMBOL};
    m_plbNeutral->setTheStyle(m_NeutralStyle);

    m_BLStyle      = {true, Line::DASH, 1, QColor(205,55,55), Line::NOSYMBOL};
    m_plbBL->setTheStyle(m_BLStyle);

    m_PressureStyle = {true, Line::DASH, 1, QColor(55,155,55), Line::NOSYMBOL};
    m_plbPressure->setTheStyle(m_PressureStyle);
}


void XDirectStyleDlg::keyPressEvent(QKeyEvent *pEvent)
{
    // Prevent Return Key from closing App
    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_ppbOKButton->hasFocus())
            {
                m_ppbOKButton->setFocus();
                return;
            }
            else
            {
                accept();
                return;
            }
        }
        case Qt::Key_Escape:
        {
            reject();
            break;
        }
    }
}

