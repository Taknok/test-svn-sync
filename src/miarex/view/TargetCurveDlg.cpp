/****************************************************************************

	TargetCurveDlg Class
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

#include "TargetCurveDlg.h"

#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>

TargetCurveDlg::TargetCurveDlg(QWidget *pParent) : QDialog(pParent)
{
	m_BellCurveExp = 1;
	m_bMaxCL = true;
	m_bShowEllipticCurve = false;
	m_bShowBellCurve = false;
	setupLayout();
}


void TargetCurveDlg::initDialog(bool bShowElliptic, bool bShowBell, bool bMaxCl, double curveExp)
{
	m_bShowEllipticCurve = bShowElliptic;
	m_bShowBellCurve = bShowBell;
	m_bMaxCL = bMaxCl;
	m_BellCurveExp = curveExp;
	m_pctrlShowEllipticCurve->setChecked(m_bShowEllipticCurve);
	m_pctrlShowBellCurve->setChecked(m_bShowBellCurve);
	m_pCtrlExptEdit->setValue(m_BellCurveExp);
	m_pCtrlRadio1->setChecked(m_bMaxCL);
	m_pCtrlRadio2->setChecked(!m_bMaxCL);
}


void TargetCurveDlg::setupLayout()
{
	QVBoxLayout*pFormLayout = new QVBoxLayout;
	{
		m_pctrlShowEllipticCurve = new QCheckBox(tr("Show Elliptic Curve"));
		m_pctrlShowBellCurve = new QCheckBox(tr("Show Bell Curve"));
		QHBoxLayout * pExpLayout = new QHBoxLayout;
		{
			QLabel *pExpLabel = new QLabel(tr("Bell Curve exponent:"));
			m_pCtrlExptEdit = new DoubleEdit(1, 2);
			pExpLayout->addWidget(pExpLabel);
			pExpLayout->addWidget(m_pCtrlExptEdit);
		}
		QGroupBox *pctrlCLBox = new QGroupBox(tr("Cl Adjustment"));
		{
			QHBoxLayout *pCLLayout = new QHBoxLayout;
			{
				m_pCtrlRadio1 = new QRadioButton(tr("Max local Cl"));
				m_pCtrlRadio2 = new QRadioButton(tr("Wing CL"));
				pCLLayout->addWidget(m_pCtrlRadio1);
				pCLLayout->addWidget(m_pCtrlRadio2);
			}
			pctrlCLBox->setLayout(pCLLayout);
		}
		pFormLayout->addWidget(m_pctrlShowEllipticCurve);
		pFormLayout->addWidget(m_pctrlShowBellCurve);
		pFormLayout->addLayout(pExpLayout);
		pFormLayout->addWidget(pctrlCLBox);
	}
	QPushButton *pOKButton = new QPushButton(tr("Close"));
	connect(pOKButton, SIGNAL(clicked()), this, SLOT(onOK()));
	pFormLayout->addStretch();
	pFormLayout->addWidget(pOKButton);
	setLayout(pFormLayout);
}


void TargetCurveDlg::onOK()
{
	m_bShowEllipticCurve = m_pctrlShowEllipticCurve->isChecked();
	m_bShowBellCurve = m_pctrlShowBellCurve->isChecked();
	m_BellCurveExp = m_pCtrlExptEdit->value();
	m_bMaxCL = m_pCtrlRadio1->isChecked();
	accept();
}


