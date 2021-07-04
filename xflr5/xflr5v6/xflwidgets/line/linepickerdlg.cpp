/****************************************************************************

    LinePicker Class
    Copyright (C) 2009 André Deperrois

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

#include <QPushButton>
#include <QKeyEvent>
#include <QColorDialog>
#include <QLabel>
#include <QGridLayout>

#include "linepickerdlg.h"
#include <xflcore/displayoptions.h>
#include <xflcore/xflcore.h>
#include <xflwidgets/line/linebtn.h>
#include <xflwidgets/line/linecbbox.h>
#include <xflwidgets/line/linedelegate.h>
#include <xflwidgets/line/linepickerwt.h>

LinePickerDlg::LinePickerDlg(QWidget *pParent): QDialog(pParent)
{
    setWindowTitle(tr("Line Picker"));

    m_bAcceptPointStyle = false;

    setupLayout();

    connect(m_pLinePicker,    SIGNAL(styleChanged(LineStyle)),  SLOT(onLineStyle(LineStyle)));
}


void LinePickerDlg::onButton(QAbstractButton *pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Ok)     == pButton)  accept();
    else if (m_pButtonBox->button(QDialogButtonBox::Cancel) == pButton)  reject();
}


void LinePickerDlg::fillBoxes()
{
    m_pLinePicker->setTheStyle(m_LineStyle);
    m_pLinePicker->fillBoxes();
    m_pLinePicker->enableBoxes(true);
}


void LinePickerDlg::initDialog(LineStyle const &ls, bool bAcceptPointStyle, bool bFlowDownEnable)
{
    initDialog(ls.m_Symbol, ls.m_Stipple, ls.m_Width, ls.m_Color, bAcceptPointStyle, bFlowDownEnable);
}


void LinePickerDlg::initDialog(Line::enumPointStyle pointStyle, Line::enumLineStipple linestipple, int lineWidth, QColor const&lineColor,
                               bool bAcceptPointStyle, bool bFlowDownEnable)
{
    m_bAcceptPointStyle = bAcceptPointStyle;
    m_LineStyle.m_Symbol = pointStyle;
    m_LineStyle.m_Stipple = linestipple;
    m_LineStyle.m_Width = lineWidth;
    m_LineStyle.m_Color = lineColor;
    initDialog(bFlowDownEnable);
}


void LinePickerDlg::initDialog(bool bFlowDownEnable)
{
    m_pchFlowDownStyle->setVisible(bFlowDownEnable);
    m_pchFlowDownStyle->setChecked(DisplayOptions::isAlignedChildrenStyle());

    fillBoxes();
}


void LinePickerDlg::keyPressEvent(QKeyEvent *event)
{
    // Prevent Return Key from closing App
    switch (event->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_pButtonBox->hasFocus())
            {
                m_pButtonBox->setFocus();
            }
            break;
        }
        case Qt::Key_Escape:
        {
            reject();
            return;
        }
    }
}


void LinePickerDlg::accept()
{
    DisplayOptions::setAlignedChildrenStyle(m_pchFlowDownStyle->isChecked());
    QDialog::accept();
}


void LinePickerDlg::reject()
{
    DisplayOptions::setAlignedChildrenStyle(m_pchFlowDownStyle->isChecked());
    QDialog::reject();
}


void LinePickerDlg::onLineStyle(LineStyle ls)
{
    m_LineStyle = ls;
    fillBoxes();
}



void LinePickerDlg::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        m_pchFlowDownStyle = new QCheckBox(tr("Flow down style"));
        QString tip = tr("If activated:\n"
                         "all changes made to the style of the polar objects will flow down to the operating points\n"
                         "all changes made to the style of the foil objects will flow down to the polars and to the operating points");
        m_pchFlowDownStyle->setToolTip(tip);

        m_pLinePicker = new LinePickerWt(this);

        m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        {
            connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
        }

        pMainLayout->addStretch(1);
        pMainLayout->addWidget(m_pchFlowDownStyle);
        pMainLayout->addWidget(m_pLinePicker);
        pMainLayout->addStretch(1);
        pMainLayout->addWidget(m_pButtonBox);
        pMainLayout->addStretch(1);
    }

    setLayout(pMainLayout);
}




