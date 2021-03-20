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


        m_pctrlRefFoil   = new LineBtn(this);
        m_pctrlModFoil   = new LineBtn(this);
        m_pctrlSpline    = new LineBtn(this);
        m_pctrlReflected = new LineBtn(this);

        pStyleLayout->addWidget(lab1,1,1);
        pStyleLayout->addWidget(lab2,2,1);
        pStyleLayout->addWidget(lab3,3,1);
        pStyleLayout->addWidget(lab4,4,1);
        pStyleLayout->addWidget(m_pctrlRefFoil,1,2);
        pStyleLayout->addWidget(m_pctrlModFoil,2,2);
        pStyleLayout->addWidget(m_pctrlSpline,3,2);
        pStyleLayout->addWidget(m_pctrlReflected,4,2);
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

    connect(m_pctrlRefFoil,   SIGNAL(clickedLB()), this, SLOT(onRefStyle()));
    connect(m_pctrlModFoil,   SIGNAL(clickedLB()), this, SLOT(onModStyle()));
    connect(m_pctrlSpline,    SIGNAL(clickedLB()), this, SLOT(onSplineStyle()));
    connect(m_pctrlReflected, SIGNAL(clickedLB()), this, SLOT(onReflectedStyle()));
}


void InverseOptionsDlg::initDialog()
{
    m_pctrlRefFoil->setTheStyle(m_pXInverse->m_pRefFoil->foilLineStyle(), m_pXInverse->m_pRefFoil->foilLineWidth(), colour(m_pXInverse->m_pRefFoil),0);
    m_pctrlModFoil->setTheStyle(m_pXInverse->m_pModFoil->foilLineStyle(), m_pXInverse->m_pModFoil->foilLineWidth(), colour(m_pXInverse->m_pModFoil),0);
    m_pctrlSpline->setTheStyle(m_pXInverse->m_Spline.style(), m_pXInverse->m_Spline.width(), m_pXInverse->m_Spline.color(),0);
    m_pctrlReflected->setTheStyle(m_pXInverse->m_ReflectedStyle, m_pXInverse->m_ReflectedWidth, m_pXInverse->m_ReflectedClr,0);
}


void InverseOptionsDlg::onRefStyle()
{
    LinePickerDlg dlg(this);
    dlg.initDialog(m_pXInverse->m_pRefFoil->foilPointStyle(), m_pXInverse->m_pRefFoil->foilLineStyle(), m_pXInverse->m_pRefFoil->foilLineWidth(),
                   colour(m_pXInverse->m_pRefFoil), false, false);

    if(QDialog::Accepted==dlg.exec())
    {
        m_pctrlRefFoil->setTheStyle(dlg.theStyle());
        m_pXInverse->m_pRefFoil->setTheStyle(dlg.lineStipple(), dlg.lineWidth(), dlg.lineColor(), dlg.pointStyle());
    }
}


void InverseOptionsDlg::onModStyle()
{
    LinePickerDlg dlg(this);
    dlg.initDialog(m_pXInverse->m_pModFoil->foilPointStyle(),m_pXInverse->m_pModFoil->foilLineStyle(), m_pXInverse->m_pModFoil->foilLineWidth(),
                   colour(m_pXInverse->m_pModFoil), false, false);

    if(QDialog::Accepted==dlg.exec())
    {
        m_pctrlModFoil->setTheStyle(dlg.theStyle());
        m_pXInverse->m_pModFoil->setTheStyle(dlg.lineStipple(), dlg.lineWidth(), dlg.lineColor(), dlg.pointStyle());
    }
}


void InverseOptionsDlg::onSplineStyle()
{
    LinePickerDlg dlg(this);
    dlg.initDialog(0, m_pXInverse->m_Spline.style(), m_pXInverse->m_Spline.width(), m_pXInverse->m_Spline.color(), false, false);

    if(QDialog::Accepted==dlg.exec())
    {
        m_pctrlSpline->setTheStyle(dlg.lineStipple(),dlg.lineWidth(),dlg.lineColor(),0);
        m_pXInverse->m_Spline.setStyle(dlg.lineStipple());
        m_pXInverse->m_Spline.setWidth(dlg.lineWidth());
        m_pXInverse->m_Spline.setColor(dlg.lineColor());
    }
}


void InverseOptionsDlg::onReflectedStyle()
{
    LinePickerDlg dlg(this);
    dlg.initDialog(0, m_pXInverse->m_ReflectedStyle, m_pXInverse->m_ReflectedWidth, m_pXInverse->m_ReflectedClr, false, false);

    if(QDialog::Accepted==dlg.exec())
    {
        m_pctrlReflected->setTheStyle(dlg.lineStipple(),dlg.lineWidth(),dlg.lineColor(),0);
        m_pXInverse->m_ReflectedStyle = dlg.lineStipple();
        m_pXInverse->m_ReflectedWidth = dlg.lineWidth();
        m_pXInverse->m_ReflectedClr   = dlg.lineColor();
    }
}


