/****************************************************************************

    GridSettingsDlg Class
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
#include <QVBoxLayout>
#include <QLabel>

#include "afoil.h"
#include "gridsettingsdlg.h"
#include <misc/line/linepickerdlg.h>
#include <misc/line/linebtn.h>
#include <misc/text/doubleedit.h>


GridSettingsDlg::GridSettingsDlg(QWidget *pParent): QDialog(pParent)
{
    setWindowTitle(tr("Grid Options"));

    m_bScale = false;

    m_bXGrid     = false;
    m_XUnit  = 0.05;
    m_XStyle = 1;
    m_XWidth = 1;
    m_XColor = QColor(150,150,150);

    m_bYGrid     = false;
    m_YUnit  = 0.05;
    m_YStyle = 1;
    m_YWidth = 1;
    m_YColor = QColor(150,150,150);

    m_bXMinGrid  = false;
    m_XMinUnit = 0.01;
    m_XMinStyle  = 2;
    m_XMinWidth  = 1;
    m_XMinColor  = QColor(70,70,70);

    m_bYMinGrid  = false;
    m_YMinUnit = 0.01;
    m_YMinStyle  = 2;
    m_YMinWidth  = 1;
    m_YMinColor  = QColor(70,70,70);

    m_NeutralStyle.m_Stipple = Line::DASHDOT;
    m_NeutralStyle.m_Width = 1;
    m_NeutralStyle.m_Color = QColor(70,70,70);

    setupLayout();

    connect(m_ppbOKButton, SIGNAL(clicked()),this, SLOT(onOK()));
    connect(m_ppbCancelButton, SIGNAL(clicked()), this, SLOT(reject()));

    connect(m_pchScale, SIGNAL(clicked()), this, SLOT(onScale()));
    connect(m_pchNeutralShow, SIGNAL(clicked(bool)), this, SLOT(onNeutralShow(bool)));
    connect(m_pcHXMajShow, SIGNAL(clicked(bool)), this, SLOT(onXMajShow(bool)));
    connect(m_pcHYMajShow, SIGNAL(clicked(bool)), this, SLOT(onYMajShow(bool)));
    connect(m_pchXMinShow, SIGNAL(clicked(bool)), this, SLOT(onXMinShow(bool)));
    connect(m_pchYMinShow, SIGNAL(clicked(bool)), this, SLOT(onYMinShow(bool)));
    connect(m_plbNeutralStyle, SIGNAL(clickedLB()), this, SLOT(onNeutralStyle()));
    connect(m_plbXMajStyle, SIGNAL(clickedLB()), this, SLOT(onXMajStyle()));
    connect(m_plbYMajStyle, SIGNAL(clickedLB()), this, SLOT(onYMajStyle()));
    connect(m_plbXMinStyle, SIGNAL(clickedLB()), this, SLOT(onXMinStyle()));
    connect(m_plbYMinStyle, SIGNAL(clickedLB()), this, SLOT(onYMinStyle()));

}

void GridSettingsDlg::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
        case Qt::Key_Escape:
        {
            done(0);
            break;
        }
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_ppbOKButton->hasFocus() && !m_ppbCancelButton->hasFocus())
            {
                m_ppbOKButton->setFocus();
//                m_bApplied  = true;
            }
            else
            {
                QDialog::accept();
            }
            break;
        }
        default:
            event->ignore();
    }
}


void GridSettingsDlg::initDialog()
{
    m_plbNeutralStyle->setTheStyle(m_NeutralStyle);

    m_plbXMajStyle->setTheStyle(m_XStyle, m_XWidth, m_XColor,0);
    m_plbXMinStyle->setTheStyle(m_XMinStyle, m_XMinWidth, m_XMinColor,0);
    m_plbYMajStyle->setTheStyle(m_YStyle, m_YWidth, m_YColor,0);
    m_plbYMinStyle->setTheStyle(m_YMinStyle, m_YMinWidth, m_YMinColor,0);

    m_plbNeutralStyle->setEnabled(m_NeutralStyle.m_bIsVisible);
    m_plbXMajStyle->setEnabled(m_bXGrid);
    m_plbYMajStyle->setEnabled(m_bYGrid);
    m_plbXMinStyle->setEnabled(m_bXMinGrid);
    m_plbYMinStyle->setEnabled(m_bYMinGrid);
    m_pdeXUnit->setEnabled(m_bXGrid);
    m_pdeYUnit->setEnabled(m_bYGrid);
    m_pdeXMinUnit->setEnabled(m_bXMinGrid);
    m_pdeYMinUnit->setEnabled(m_bYMinGrid);

    m_pchScale->setChecked(m_bScale);
    m_pchNeutralShow->setChecked(m_NeutralStyle.m_bIsVisible);
    m_pcHXMajShow->setChecked(m_bXGrid);
    m_pcHYMajShow->setChecked(m_bYGrid);
    m_pchXMinShow->setChecked(m_bXMinGrid);
    m_pchYMinShow->setChecked(m_bYMinGrid);

    m_pdeXUnit->setValue(m_XUnit);
    m_pdeYUnit->setValue(m_YUnit);
    m_pdeXMinUnit->setValue(m_XMinUnit);
    m_pdeYMinUnit->setValue(m_YMinUnit);
}


void GridSettingsDlg::setupLayout()
{
    QGridLayout *pGridData = new QGridLayout;
    {
        m_pchNeutralShow = new QCheckBox(tr("Neutral Line"));
        m_pchScale       = new QCheckBox(tr("X-Scale"));
        m_pcHXMajShow = new QCheckBox(tr("X Major Grid"));
        m_pcHYMajShow = new QCheckBox(tr("Y Major Grid"));
        m_pchXMinShow = new QCheckBox(tr("X Minor Grid"));
        m_pchYMinShow = new QCheckBox(tr("Y Minor Grid"));

        m_plbNeutralStyle = new LineBtn(this);
        m_plbXMajStyle = new LineBtn(this);
        m_plbYMajStyle = new LineBtn(this);
        m_plbXMinStyle = new LineBtn(this);
        m_plbYMinStyle = new LineBtn(this);

        m_pdeXUnit = new DoubleEdit;
        m_pdeYUnit = new DoubleEdit;
        m_pdeXMinUnit = new DoubleEdit;
        m_pdeYMinUnit = new DoubleEdit;
        m_pdeXUnit->setDigits(3);
        m_pdeYUnit->setDigits(3);
        m_pdeXMinUnit->setDigits(3);
        m_pdeYMinUnit->setDigits(3);

        pGridData->addWidget(m_pchNeutralShow,1,1);
        pGridData->addWidget(m_pcHXMajShow,2,1);
        pGridData->addWidget(m_pcHYMajShow,3,1);
        pGridData->addWidget(m_pchXMinShow,4,1);
        pGridData->addWidget(m_pchYMinShow,5,1);

        pGridData->addWidget(m_plbNeutralStyle,1,2);
        pGridData->addWidget(m_plbXMajStyle,2,2);
        pGridData->addWidget(m_plbYMajStyle,3,2);
        pGridData->addWidget(m_plbXMinStyle,4,2);
        pGridData->addWidget(m_plbYMinStyle,5,2);

        pGridData->addWidget(m_pchScale,1,3);
        pGridData->addWidget(m_pdeXUnit,2,3);
        pGridData->addWidget(m_pdeYUnit,3,3);
        pGridData->addWidget(m_pdeXMinUnit,4,3);
        pGridData->addWidget(m_pdeYMinUnit,5,3);
    }

    QHBoxLayout *pCommandButtons = new QHBoxLayout;
    {
        m_ppbOKButton      = new QPushButton(tr("Accept"));
        m_ppbCancelButton  = new QPushButton(tr("Cancel"));

        pCommandButtons->addStretch(1);
        pCommandButtons->addWidget(m_ppbOKButton);
        pCommandButtons->addStretch(1);
        pCommandButtons->addWidget(m_ppbCancelButton);
        pCommandButtons->addStretch(1);
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addLayout(pGridData);
        pMainLayout->addLayout(pCommandButtons);
    }
    setLayout(pMainLayout);
}


void GridSettingsDlg::onNeutralStyle()
{
    LinePickerDlg dlg(this);
    dlg.initDialog(m_NeutralStyle, false, false);

    if(QDialog::Accepted==dlg.exec())
    {
        m_NeutralStyle = dlg.theStyle();
        m_plbNeutralStyle->setStipple(dlg.lineStipple());
        m_plbNeutralStyle->setWidth(dlg.lineWidth());
        m_plbNeutralStyle->setColor(dlg.lineColor());
    }
}


void GridSettingsDlg::onXMajStyle()
{
    LinePickerDlg dlg(this);
    dlg.initDialog(0,m_XStyle,m_XWidth,m_XColor, false, false);

    if(QDialog::Accepted==dlg.exec())
    {
        m_XStyle = dlg.lineStipple();
        m_XWidth = dlg.lineWidth();
        m_XColor = dlg.lineColor();
        m_plbXMajStyle->setStipple(dlg.lineStipple());
        m_plbXMajStyle->setWidth(dlg.lineWidth());
        m_plbXMajStyle->setColor(dlg.lineColor());
    }
}

void GridSettingsDlg::onXMinStyle()
{
    LinePickerDlg dlg(this);
    dlg.initDialog(0,m_XMinStyle,m_XMinWidth,m_XMinColor, false, false);

    if(QDialog::Accepted==dlg.exec())
    {
        m_XMinStyle = dlg.lineStipple();
        m_XMinWidth = dlg.lineWidth();
        m_XMinColor = dlg.lineColor();
        m_plbXMinStyle->setStipple(dlg.lineStipple());
        m_plbXMinStyle->setWidth(dlg.lineWidth());
        m_plbXMinStyle->setColor(dlg.lineColor());
    }

}

void GridSettingsDlg::onYMajStyle()
{
    LinePickerDlg dlg(this);

    dlg.initDialog(0,m_YStyle,m_YWidth,m_YColor, false, false);

    if(QDialog::Accepted==dlg.exec())
    {
        m_YStyle = dlg.lineStipple();
        m_YWidth = dlg.lineWidth();
        m_YColor = dlg.lineColor();
        m_plbYMajStyle->setStipple(dlg.lineStipple());
        m_plbYMajStyle->setWidth(dlg.lineWidth());
        m_plbYMajStyle->setColor(dlg.lineColor());
    }
}

void GridSettingsDlg::onYMinStyle()
{
    LinePickerDlg dlg(this);

    dlg.initDialog(0,m_YMinStyle,m_YMinWidth,m_YMinColor, false, false);

    if(QDialog::Accepted==dlg.exec())
    {
        m_YMinStyle = dlg.lineStipple();
        m_YMinWidth = dlg.lineWidth();
        m_YMinColor = dlg.lineColor();
        m_plbYMinStyle->setStipple(dlg.lineStipple());
        m_plbYMinStyle->setWidth(dlg.lineWidth());
        m_plbYMinStyle->setColor(dlg.lineColor());
    }
}

void GridSettingsDlg::onNeutralShow(bool bShow)
{
    m_NeutralStyle.m_bIsVisible = bShow;
    m_plbNeutralStyle->setEnabled(m_NeutralStyle.m_bIsVisible);
}

void GridSettingsDlg::onScale()
{
    m_bScale = m_pchScale->isChecked();
}


void GridSettingsDlg::onXMajShow(bool bShow)
{
    m_bXGrid = bShow;
    m_plbXMajStyle->setEnabled(m_bXGrid);
    m_pdeXUnit->setEnabled(m_bXGrid);
}





void GridSettingsDlg::onYMajShow(bool bShow)
{
    m_bYGrid = bShow;
    m_plbYMajStyle->setEnabled(m_bYGrid);
    m_pdeYUnit->setEnabled(m_bYGrid);
}


void GridSettingsDlg::onXMinShow(bool bShow)
{
    m_bXMinGrid = bShow;
    m_plbXMinStyle->setEnabled(m_bXMinGrid);
    m_pdeXMinUnit->setEnabled(m_bXMinGrid);
}


void GridSettingsDlg::onYMinShow(bool bShow)
{
    m_bYMinGrid = bShow;
    m_plbYMinStyle->setEnabled(m_bYMinGrid);
    m_pdeYMinUnit->setEnabled(m_bYMinGrid);
}

void GridSettingsDlg::onOK()
{
    m_XUnit = m_pdeXUnit->value();
    m_YUnit = m_pdeYUnit->value();
    m_XMinUnit = m_pdeXMinUnit->value();
    m_YMinUnit = m_pdeYMinUnit->value();
    accept();
}





