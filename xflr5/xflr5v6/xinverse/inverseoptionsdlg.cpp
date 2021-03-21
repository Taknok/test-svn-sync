/****************************************************************************

    InverseOptionsDlg  Classes
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

#include <QGridLayout>

#include <xflcore/xflcore.h>

#include "xinverse.h"
#include "inverseoptionsdlg.h"
#include <misc/line/linepickerdlg.h>
#include <xflobjects/objects2d/foil.h>
#include <misc/line/linebtn.h>


InverseOptionsDlg::InverseOptionsDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("XInverse Style"));
    m_pXInverse = nullptr;
    setupLayout();
}


void InverseOptionsDlg::setupLayout()
{
    QGridLayout *pStyleLayout = new QGridLayout;
    {
        QLabel * lab1 = new QLabel(tr("Reference Foil"));
        QLabel * lab2 = new QLabel(tr("Modified Foil"));
        QLabel * lab3 = new QLabel(tr("Spline"));
        QLabel * lab4 = new QLabel(tr("Reflected Curve"));
        lab1->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        lab2->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        lab3->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        lab4->setAlignment(Qt::AlignRight | Qt::AlignVCenter);


        m_plbRefFoil   = new LineBtn(this);
        m_plbModFoil   = new LineBtn(this);
        m_plbSpline    = new LineBtn(this);
        m_plbReflected = new LineBtn(this);

        pStyleLayout->addWidget(lab1,1,1);
        pStyleLayout->addWidget(lab2,2,1);
        pStyleLayout->addWidget(lab3,3,1);
        pStyleLayout->addWidget(lab4,4,1);
        pStyleLayout->addWidget(m_plbRefFoil,1,2);
        pStyleLayout->addWidget(m_plbModFoil,2,2);
        pStyleLayout->addWidget(m_plbSpline,3,2);
        pStyleLayout->addWidget(m_plbReflected,4,2);
    }

    QHBoxLayout *pCommandButtons = new QHBoxLayout;
    {
        QPushButton *pOKButton      = new QPushButton(tr("OK"));
        QPushButton *pCancelButton  = new QPushButton(tr("Cancel"));
        pCommandButtons->addStretch(1);
        pCommandButtons->addWidget(pOKButton);
        pCommandButtons->addStretch(1);
        pCommandButtons->addWidget(pCancelButton);
        pCommandButtons->addStretch(1);

        connect(pOKButton, SIGNAL(clicked()),this, SLOT(accept()));
        connect(pCancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addLayout(pStyleLayout);
        pMainLayout->addLayout(pCommandButtons);
    }

    setLayout(pMainLayout);

    connect(m_plbRefFoil,   SIGNAL(clickedLB()), this, SLOT(onRefStyle()));
    connect(m_plbModFoil,   SIGNAL(clickedLB()), this, SLOT(onModStyle()));
    connect(m_plbSpline,    SIGNAL(clickedLB()), this, SLOT(onSplineStyle()));
    connect(m_plbReflected, SIGNAL(clickedLB()), this, SLOT(onReflectedStyle()));
}


void InverseOptionsDlg::initDialog()
{
    m_plbRefFoil->setTheStyle(m_pXInverse->m_pRefFoil->theStyle());
    m_plbModFoil->setTheStyle(m_pXInverse->m_pModFoil->theStyle());
    m_plbSpline->setTheStyle(m_pXInverse->m_Spline.theStyle());
    m_plbReflected->setTheStyle(m_pXInverse->m_ReflectedStyle);
}


void InverseOptionsDlg::onRefStyle()
{
    LinePickerDlg dlg(this);
    dlg.initDialog(m_pXInverse->m_pRefFoil->theStyle(), false, false);

    if(QDialog::Accepted==dlg.exec())
    {
        m_plbRefFoil->setTheStyle(dlg.theStyle());
        m_pXInverse->m_pRefFoil->setTheStyle(dlg.theStyle());
    }
}


void InverseOptionsDlg::onModStyle()
{
    LinePickerDlg dlg(this);
    dlg.initDialog(m_pXInverse->m_pModFoil->theStyle(), false, false);

    if(QDialog::Accepted==dlg.exec())
    {
        m_plbModFoil->setTheStyle(dlg.theStyle());
        m_pXInverse->m_pModFoil->setTheStyle(dlg.theStyle());
    }
}


void InverseOptionsDlg::onSplineStyle()
{
    LinePickerDlg dlg(this);
    dlg.initDialog(m_pXInverse->m_Spline.theStyle(), false, false);

    if(QDialog::Accepted==dlg.exec())
    {
        m_plbSpline->setTheStyle(dlg.lineStipple(),dlg.lineWidth(),dlg.lineColor(),0);
        m_pXInverse->m_Spline.setTheStyle(dlg.theStyle());
    }
}


void InverseOptionsDlg::onReflectedStyle()
{
    LinePickerDlg dlg(this);
    dlg.initDialog(m_pXInverse->m_ReflectedStyle, false, false);

    if(QDialog::Accepted==dlg.exec())
    {
        m_plbReflected->setTheStyle(dlg.lineStipple(),dlg.lineWidth(),dlg.lineColor(),0);
        m_pXInverse->m_ReflectedStyle = dlg.ls2();
    }
}


