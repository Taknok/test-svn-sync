/****************************************************************************

	WPolarDlg Class
	Copyright (C) 2009 Andre Deperrois XFLR5@yahoo.com

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

#include "../MainFrame.h"
#include "../Globals.h"
#include "../Objects/WPolar.h"
#include "WPolarDlg.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <math.h>
 
WPolarDlg::WPolarDlg()
{
	m_pMainFrame = NULL;
	m_pWing      = NULL;
	m_pPlane     = NULL;

	m_WPolarName = "WPolar Name";

	m_bAutoName = TRUE;

	m_QInf       = 10.0;//m/s
	m_Weight     = 1.0;
	m_XCmRef     = 0.0;
	m_Alpha      = 0.0;
	m_Beta       = 0.0;
	m_Type       = 1;
	m_WingLoad   = 1.0;
	m_Density    = 1.225;
	m_Viscosity  = 1.5e-5;
	m_Height     = 0.0;
	m_pWing	     = NULL;
	m_pPlane     = NULL;

	m_AnalysisType = 1;

	m_bVLM1         = true;
	m_bThinSurfaces = true;
	m_bTiltedGeom   = false;
	m_bWakeRollUp   = false;
	m_bViscous      = true;
	m_bGround       = false;

	m_NXWakePanels    = 5;
	m_TotalWakeLength = 100.0;//x mac
	m_WakePanelFactor = 1.1;

//	m_SymbolFont.CreatePointFont(100, "Symbol");
	m_UnitType  = 1;

	SetupLayout();
	Connect();
}

void WPolarDlg::Connect()
{
	connect(m_pctrlMethod1, SIGNAL(toggled(bool)), this, SLOT(OnMethod()));
	connect(m_pctrlMethod2, SIGNAL(toggled(bool)), this, SLOT(OnMethod()));
	connect(m_pctrlMethod3, SIGNAL(toggled(bool)), this, SLOT(OnMethod()));
	connect(m_pctrlMethod4, SIGNAL(toggled(bool)), this, SLOT(OnMethod()));

	connect(m_pctrlUnit1, SIGNAL(toggled(bool)), this, SLOT(OnUnit()));
	connect(m_pctrlUnit2, SIGNAL(toggled(bool)), this, SLOT(OnUnit()));

	connect(m_pctrlType1, SIGNAL(toggled(bool)), this, SLOT(OnWPolarType()));
	connect(m_pctrlType2, SIGNAL(toggled(bool)), this, SLOT(OnWPolarType()));
	connect(m_pctrlType4, SIGNAL(toggled(bool)), this, SLOT(OnWPolarType()));

	connect(m_pctrlAutoName, SIGNAL(stateChanged(int)), this, SLOT(OnAutoName(int)));
	connect(m_pctrlWakeRollUp, SIGNAL(stateChanged(int)), this, SLOT(OnWakeRollUp(int)));
	connect(m_pctrlTiltGeom, SIGNAL(stateChanged(int)), this, SLOT(OnTiltedGeom(int)));
	connect(m_pctrlViscous, SIGNAL(stateChanged(int)), this, SLOT(OnViscous(int)));
	connect(m_pctrlGroundEffect, SIGNAL(stateChanged(int)), this, SLOT(OnGroundEffect(int)));

	connect(m_pctrlXCmRef, SIGNAL(editingFinished()), this, SLOT(OnEditingFinished()));
	connect(m_pctrlDensity, SIGNAL(editingFinished()), this, SLOT(OnEditingFinished()));
	connect(m_pctrlViscosity, SIGNAL(editingFinished()), this, SLOT(OnEditingFinished()));
	connect(m_pctrlAlpha, SIGNAL(editingFinished()), this, SLOT(OnEditingFinished()));
	connect(m_pctrlBeta, SIGNAL(editingFinished()), this, SLOT(OnEditingFinished()));
	connect(m_pctrlWeight, SIGNAL(editingFinished()), this, SLOT(OnEditingFinished()));
	connect(m_pctrlQInf, SIGNAL(editingFinished()), this, SLOT(OnEditingFinished()));
	connect(m_pctrlHeight, SIGNAL(editingFinished()), this, SLOT(OnEditingFinished()));
	connect(m_pctrlWPolarName, SIGNAL(editingFinished()), this, SLOT(OnEditingFinished()));

	connect(OKButton, SIGNAL(clicked()),this, SLOT(OnOK()));
	connect(CancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

void WPolarDlg::EnableControls()
{
	switch (m_Type)
	{
		case 1:
		{
			m_pctrlQInf->setEnabled(true);
			m_pctrlAlpha->setEnabled(false);
			break;
		}
		case 2:
		{
			m_pctrlQInf->setEnabled(false);
			m_pctrlRRe->setText(" ");
			m_pctrlSRe->setText(" ");
			m_pctrlAlpha->setEnabled(false);
			break;
		}
		case 4:
		{
			m_pctrlQInf->setEnabled(false);
			m_pctrlRRe->setText(" ");
			m_pctrlSRe->setText(" ");
			m_pctrlAlpha->setEnabled(true);
			break;
		}
		default:
		{
			m_pctrlQInf->setEnabled(true);
			break;
		}
	}
	if(!m_pPlane) m_pctrlMethod1->setEnabled(TRUE);

	if (m_pctrlMethod1->isChecked())
	{
		m_pctrlViscous->setEnabled(false);
	}
	else if (m_pctrlMethod2->isChecked() || m_pctrlMethod3->isChecked())
	{
		m_pctrlViscous->setEnabled(true);
	}
	else if (m_pctrlMethod4->isChecked())
	{
		m_pctrlViscous->setEnabled(true);
	}
}

void WPolarDlg::InitDialog()
{
	MainFrame* pMainFrame = (MainFrame*)m_pMainFrame;
	QString str;
	QString str1,str2;

	int i;

	if(m_pPlane)
	{
		if(m_AnalysisType==0 || m_AnalysisType==1) m_AnalysisType=2;
		m_pctrlMethod1->setEnabled(false);
	}

	bool bWarning = false;
	for (i=0; i<m_pWing->m_NPanel-1; i++)
	{
		if(m_pWing->m_NXPanels[i]!= m_pWing->m_NXPanels[i+1])
			{
				QMessageBox::warning(this, "QFLR5","The number of chordwise VLM-panels across the span is not uniform\n Panel method is disabled");
				bWarning = true;
				break;
			}
	}

	QFont SymbolFont("Symbol");
	m_pctrlRho->setFont(SymbolFont);
	m_pctrlNu->setFont(SymbolFont);


	m_bAutoName = TRUE;
	m_pctrlAutoName->setChecked(true);

	m_pctrlQInf->SetValue(m_QInf);
	m_pctrlWeight->SetValue(m_Weight);


	if(m_UnitType==1) m_pctrlUnit1->setChecked(true);
	else              m_pctrlUnit2->setChecked(true);

	if(m_Type==1) m_pctrlType1->setChecked(true);
	else if(m_Type==2) m_pctrlType2->setChecked(true);
	else if(m_Type==4) m_pctrlType4->setChecked(true);

	m_pctrlWeight->SetPrecision(3);
	m_pctrlDensity->SetPrecision(5);
	m_pctrlViscosity->SetPrecision(3);

	OnUnit();

	m_pctrlHeight->SetValue(m_Height*pMainFrame->m_mtoUnit);

	if(m_bGround)
	{
		m_pctrlHeight->setEnabled(true);
		m_pctrlGroundEffect->setChecked(true);
	}
	else
	{
		m_pctrlHeight->setEnabled(false);
		m_pctrlGroundEffect->setChecked(false);
	}

	GetSpeedUnit(str, pMainFrame->m_SpeedUnit);
	m_pctrlSpeedUnit->setText(str);

	SetWingLoad();

	GetLengthUnit(str, pMainFrame->m_LengthUnit);
	m_pctrlLengthUnit1->setText(str);
	m_pctrlLengthUnit2->setText(str);
	m_pctrlXCmRef->SetValue(m_XCmRef*pMainFrame->m_mtoUnit);

	GetWeightUnit(str1, pMainFrame->m_WeightUnit);
	m_pctrlWeightUnit->setText(str1);

	m_pctrlQInf->SetValue(m_QInf*pMainFrame->m_mstoUnit);
	m_pctrlWeight->SetValue(m_Weight*pMainFrame->m_kgtoUnit);

	m_pctrlBeta->SetPrecision(2);
	m_pctrlBeta->SetValue(m_Beta);

	m_pctrlAlpha->SetValue(m_Alpha);

	SetWingLoad();
	SetReynolds();

	if(bWarning)
	{
		m_pctrlMethod4->setEnabled(false);
		if(m_AnalysisType==3) m_AnalysisType=2;
	}

	if(m_bViscous)		m_pctrlViscous->setChecked(true);
	if(m_bTiltedGeom)	m_pctrlTiltGeom->setChecked(true);
	if(m_bWakeRollUp) 	m_pctrlWakeRollUp->setChecked(true);

	OnTiltedGeom(m_bTiltedGeom);

	if(m_AnalysisType==0) m_AnalysisType=2;//former m_bLLT=false;
	if(m_AnalysisType==1)
	{
		m_pctrlMethod1->setChecked(true);
		m_pctrlViscous->setChecked(true);
		m_pctrlViscous->setEnabled(false);
		m_pctrlWakeRollUp->setChecked(false);
		m_pctrlWakeParams->setEnabled(false);
	}
	else if(m_AnalysisType==2)
	{
		m_pctrlWakeRollUp->setEnabled(false);
		m_pctrlWakeParams->setEnabled(false);

		if(m_bVLM1) m_pctrlMethod2->setChecked(true);
		else        m_pctrlMethod3->setChecked(true);
		m_pctrlViscous->setEnabled(true);
	}
	else if(m_AnalysisType==3)
	{
		m_pctrlWakeRollUp->setEnabled(true);
		m_pctrlWakeParams->setEnabled(true);

		m_pctrlMethod4->setChecked(true);
		m_pctrlViscous->setEnabled(true);
	}


//	m_pctrlWakeParams->setEnabled(false);
//	m_pctrlWakeRollUp->setEnabled(false);
	m_bWakeRollUp = false;

	SetWPolarName();


	m_pctrlWingName->setText(m_UFOName);
//	CheckType();
//	EnableControls();

	OnMethod();


//	m_pctrlThinSurfaces->setEnabled(FALSE);
//	if(m_bThinSurfaces)	m_pctrlThinSurfaces.SetCheck(TRUE);
//	else                m_pctrlThinSurfaces.SetCheck(FALSE);

//	m_WakeParamsdlg.m_NXWakePanels    = m_NXWakePanels;
//	m_WakeParamsdlg.m_TotalWakeLength = m_TotalWakeLength;
//	m_WakeParamsdlg.m_WakePanelFactor = m_WakePanelFactor;

	m_pctrlQInf->setSelection(0,-1);
	m_pctrlQInf->setFocus();
}



void WPolarDlg::keyPressEvent(QKeyEvent *event)
{
	// Prevent Return Key from closing App
	switch (event->key())
	{
		case Qt::Key_Return:
		{
			if(!OKButton->hasFocus() && !CancelButton->hasFocus())
			{
				ReadValues();
				SetWPolarName();
				OKButton->setFocus();
				return;
			}
			else
			{
				OnOK();
				return;
			}
			break;
		}
		default:
			QDialog::keyPressEvent(event);
	}

	QDialog::keyPressEvent(event);
}



void WPolarDlg::OnEditingFinished()
{
	ReadValues();
	SetWPolarName();
}


void WPolarDlg::OnAutoName(int state)
{
	m_bAutoName = m_pctrlAutoName->isChecked();
}

void WPolarDlg::OnTiltedGeom(int state)
{
	m_bTiltedGeom = m_pctrlTiltGeom->isChecked();
	SetWPolarName();
	EnableControls();}

void WPolarDlg::OnViscous(int state)
{
	m_bViscous = m_pctrlViscous->isChecked();
	SetWPolarName();
	EnableControls();}

void WPolarDlg::OnGroundEffect(int state)
{
	m_bGround = m_pctrlGroundEffect->isChecked();
	m_pctrlHeight->setEnabled(m_bGround);
	SetWPolarName();
}


void WPolarDlg::OnMethod()
{
	if (m_pctrlMethod1->isChecked())
	{
		m_bViscous      = true;
		m_bThinSurfaces = true;
		m_bWakeRollUp   = false;
		m_bTiltedGeom   = false;
		m_AnalysisType  = 1;
		m_pctrlWakeRollUp->setChecked(false);
		m_pctrlTiltGeom->setChecked(false);
		m_pctrlTiltGeom->setEnabled(false);
		m_pctrlBeta->setEnabled(false);
//		m_pctrlThinSurfaces.SetCheck(TRUE);
//		m_pctrlThinSurfaces.setEnabled(false);

		m_pctrlWakeRollUp->setEnabled(false);
		m_pctrlWakeParams->setEnabled(false);
	}
	else if (m_pctrlMethod2->isChecked() || m_pctrlMethod3->isChecked())
	{
		m_bThinSurfaces = true;
		m_AnalysisType=2;
		m_pctrlTiltGeom->setEnabled(true);
		m_pctrlBeta->setEnabled(true);
		m_pctrlWakeRollUp->setEnabled(false);
		m_pctrlWakeParams->setEnabled(false);
		m_bVLM1 = m_pctrlMethod2->isChecked();
	}
	else if (m_pctrlMethod4->isChecked())
	{
		m_AnalysisType=3;
		m_pctrlTiltGeom->setEnabled(true);
		m_bThinSurfaces = false;
		m_pctrlBeta->setEnabled(true);

//		m_pctrlWakeRollUp.setEnabled(TRUE);
//		if(m_bWakeRollUp)	m_pctrlWakeParams.setEnabled(TRUE);
//		else				m_pctrlWakeParams.setEnabled(FALSE);

	}
	EnableControls();
	SetWPolarName();
}


void WPolarDlg::OnOK()
{
	CWPolar * pWPolarNew;

	m_WPolarName = m_pctrlWPolarName->text();

	int LineLength = m_WPolarName.length();
	if(!LineLength)
	{
		QMessageBox::warning(this, "QFLR5","Must enter a name for the polar");
		m_pctrlWPolarName->setFocus();
		return;
	}
	else
	{
		int size = (int)m_poaWPolar->size();
		for (int j=0; j<size; j++)
		{
			pWPolarNew = (CWPolar*)m_poaWPolar->at(j);
			if (pWPolarNew->m_PlrName == m_WPolarName &&pWPolarNew->m_UFOName  == m_UFOName)
			{
				QMessageBox::warning(this, "QFLR5","The polar's name already exists");
//				m_ctrlWPolarName.SetSel(0,-1);
//				m_ctrlWPolarName.SetFocus();
				return;
			}
		}
	}
/* TODO : restore
	if(!m_bWakeRollUp && m_AnalysisType==3)
	{
		m_TotalWakeLength = 100.0;
		m_NXWakePanels    = 1;
	}*/
	accept();
}

void WPolarDlg::OnUnit()
{
	if(m_pctrlUnit1->isChecked())
	{
		m_UnitType   = 1;
		m_pctrlViscosity->SetValue(m_Viscosity);
		m_pctrlDensityUnit->setText("kg/m3");
		m_pctrlViscosityUnit->setText("m2/s");
	}
	else
	{
		m_UnitType   = 2;
		m_pctrlViscosity->SetValue(m_Viscosity* 10.7182881);
		m_pctrlDensityUnit->setText("slugs/ft3");
		m_pctrlViscosityUnit->setText("ft2/s");
	}
	SetDensity();
}

void WPolarDlg::OnWakeRollUp(int state)
{
}

void WPolarDlg::OnWPolarType()
{
	if (m_pctrlType1->isChecked())
	{
		m_Type = 1;
	}
	else if(m_pctrlType2->isChecked())
	{
		m_Type = 2;
	}
	else if(m_pctrlType4->isChecked())
	{
		m_Type = 4;
	}
	EnableControls();
	SetReynolds();
	SetWPolarName();
}


void WPolarDlg::ReadValues()
{
	MainFrame* pMainFrame = (MainFrame*)m_pMainFrame;
	m_Alpha     = m_pctrlAlpha->GetValue();
	m_Beta      = m_pctrlBeta->GetValue();
	m_Weight    = m_pctrlWeight->GetValue() / pMainFrame->m_kgtoUnit;
	m_XCmRef    = m_pctrlXCmRef->GetValue() / pMainFrame->m_mtoUnit;
	m_QInf      = m_pctrlQInf->GetValue() / pMainFrame->m_mstoUnit;
	m_Height    = m_pctrlHeight->GetValue() / pMainFrame->m_mtoUnit;

	if(m_pctrlUnit1->isChecked())
	{
		m_Viscosity = m_pctrlViscosity->GetValue();
		m_Density   = m_pctrlDensity->GetValue();
	}
	else
	{
		m_Density   = m_pctrlDensity->GetValue() / 0.00194122;
		m_Viscosity = m_pctrlViscosity->GetValue() / 10.7182881;
	}

	SetDensity();

	SetWingLoad();


}



void WPolarDlg::SetDensity()
{
	int exp, precision;
	if(m_pctrlUnit1->isChecked())
	{
		exp = (int)log(m_Density);
		if(exp>1) precision = 1;
		else if(exp<-4) precision = 4;
		else precision = 3-exp;
		m_pctrlDensity->SetPrecision(precision);
		m_pctrlDensity->SetValue(m_Density);
	}
	else
	{
		exp = (int)log(m_Density* 0.00194122);
		if(exp>1) precision = 1;
		else if(exp<-4) precision = 4;
		else precision = 3-exp;
		m_pctrlDensity->SetPrecision(precision);
		m_pctrlDensity->SetValue(m_Density* 0.00194122);
	}
}


void WPolarDlg::SetupLayout()
{
	QDesktopWidget desktop;
	QRect r = desktop.geometry();
//	setMinimumHeight(r.height()/3);
	move(r.width()/3, r.height()/6);

	QVBoxLayout *NameLayout = new QVBoxLayout;
	m_pctrlWingName = new QLabel("WingName");
	m_pctrlAutoName = new QCheckBox("Auto Analysis Name");
	m_pctrlWPolarName = new QLineEdit("Polar Name");
	NameLayout->addWidget(m_pctrlWingName);
	NameLayout->addWidget(m_pctrlAutoName);
	NameLayout->addWidget(m_pctrlWPolarName);
	QGroupBox *NameGroup = new QGroupBox("Polar Name");
	NameGroup->setLayout(NameLayout);

	QVBoxLayout *TypeLayout = new QVBoxLayout;
	m_pctrlType1 = new QRadioButton("Type 1 (Fixed Speed)");
	m_pctrlType2 = new QRadioButton("Type 2 (Fixed Lift)");
	m_pctrlType4 = new QRadioButton("Type 4 (Fixed aoa)");
	TypeLayout->addWidget(m_pctrlType1);
	TypeLayout->addWidget(m_pctrlType2);
	TypeLayout->addWidget(m_pctrlType4);
	QGroupBox *TypeGroup = new QGroupBox("Polar Type");
	TypeGroup->setLayout(TypeLayout);

	QGridLayout *PlaneLayout = new QGridLayout;
	QLabel *lab1 = new QLabel("Free Stream Speed");
	QLabel *lab2 = new QLabel("Plane Weight");
	QLabel *lab3 = new QLabel("Mom. ref. location");
	QLabel *lab4 = new QLabel("Angle of Attack");
	QLabel *lab5 = new QLabel("Side Slip");
	PlaneLayout->addWidget(lab1,1,1);
	PlaneLayout->addWidget(lab2,2,1);
	PlaneLayout->addWidget(lab3,3,1);
	PlaneLayout->addWidget(lab4,4,1);
	PlaneLayout->addWidget(lab5,5,1);
	m_pctrlQInf    = new FloatEdit("10.05");
	m_pctrlWeight  = new FloatEdit("1.234");
	m_pctrlXCmRef  = new FloatEdit("100.00");
	m_pctrlAlpha   = new FloatEdit("1.00");
	m_pctrlBeta    = new FloatEdit("0.00");
	PlaneLayout->addWidget(m_pctrlQInf,1,2);
	PlaneLayout->addWidget(m_pctrlWeight,2,2);
	PlaneLayout->addWidget(m_pctrlXCmRef,3,2);
	PlaneLayout->addWidget(m_pctrlAlpha,4,2);
	PlaneLayout->addWidget(m_pctrlBeta,5,2);
	m_pctrlSpeedUnit   = new QLabel("m/s");
	m_pctrlWeightUnit  = new QLabel("kg");
	m_pctrlLengthUnit1 = new QLabel("mm");
	QLabel *lab6 = new QLabel("deg.");
	QLabel *lab7 = new QLabel("deg.");
	PlaneLayout->addWidget(m_pctrlSpeedUnit ,1,3);
	PlaneLayout->addWidget(m_pctrlWeightUnit ,2,3);
	PlaneLayout->addWidget(m_pctrlLengthUnit1 ,3,3);
	PlaneLayout->addWidget(lab6 ,4,3);
	PlaneLayout->addWidget(lab7 ,5,3);
	QGroupBox *PlaneGroup = new QGroupBox("Plane and Flight Data");
	PlaneGroup->setLayout(PlaneLayout);

	QVBoxLayout *FlightLayout = new QVBoxLayout;
	m_pctrlWingLoad = new QLabel("Wing Loading = 0.033 kg/dm2");
	m_pctrlSRe       = new QLabel("SRe");
	m_pctrlRRe       = new QLabel("RRe");
	m_pctrlQInfCl    = new QLabel("QInfCl");
	m_pctrlReRoot    = new QLabel("Re at Root = 100 000");
	m_pctrlReTip     = new QLabel("Re at Tip = 70 000");
	m_pctrlWingLoad->setAlignment(Qt::AlignRight | Qt::AlignCenter);
	m_pctrlSRe->setAlignment(Qt::AlignRight | Qt::AlignCenter);
	m_pctrlRRe->setAlignment(Qt::AlignRight | Qt::AlignCenter);
	m_pctrlQInfCl->setAlignment(Qt::AlignRight | Qt::AlignCenter);
	m_pctrlReTip->setAlignment(Qt::AlignRight | Qt::AlignCenter);
	m_pctrlReRoot->setAlignment(Qt::AlignRight | Qt::AlignCenter);
	FlightLayout->addWidget(m_pctrlWingLoad);
	FlightLayout->addWidget(m_pctrlSRe);
	FlightLayout->addWidget(m_pctrlRRe);
	FlightLayout->addWidget(m_pctrlQInfCl);
	FlightLayout->addWidget(m_pctrlReRoot);
	FlightLayout->addWidget(m_pctrlReTip);
	QGroupBox *FlightGroup = new  QGroupBox("Flight Characteristics");
	FlightGroup->setLayout(FlightLayout);

	QVBoxLayout *MethodLayout = new QVBoxLayout;
	m_pctrlMethod1 = new QRadioButton("LLT");
	m_pctrlMethod2 = new QRadioButton("VLM1 : Classic");
	m_pctrlMethod3 = new QRadioButton("VLM2 : Quads");
	m_pctrlMethod4 = new QRadioButton("3D Panels");
	MethodLayout->addWidget(m_pctrlMethod1);
	MethodLayout->addWidget(m_pctrlMethod2);
	MethodLayout->addWidget(m_pctrlMethod3);
	MethodLayout->addWidget(m_pctrlMethod4);
	QGroupBox *MethodGroup = new QGroupBox;
	MethodGroup->setLayout(MethodLayout);


	QGridLayout *AeroDataLayout = new QGridLayout;
	QLabel *lab9 = new QLabel("Unit");
	m_pctrlUnit1 = new QRadioButton("International");
	m_pctrlUnit2 = new QRadioButton("Imperial");
	m_pctrlRho = new QLabel("r");
	m_pctrlDensity = new FloatEdit("1.500e-5",3);
	m_pctrlDensityUnit = new QLabel("kg/m3");
	m_pctrlNu = new QLabel("n");
	m_pctrlViscosity = new FloatEdit("1.225",3);
	m_pctrlViscosityUnit = new QLabel("m2/s");
	AeroDataLayout->addWidget(lab9,1,1);
	AeroDataLayout->addWidget(m_pctrlUnit1,1,2);
	AeroDataLayout->addWidget(m_pctrlUnit2,1,3);
	AeroDataLayout->addWidget(m_pctrlRho,2,1);
	AeroDataLayout->addWidget(m_pctrlDensity,2,2);
	AeroDataLayout->addWidget(m_pctrlDensityUnit,2,3);
	AeroDataLayout->addWidget(m_pctrlNu,3,1);
	AeroDataLayout->addWidget(m_pctrlViscosity,3,2);
	AeroDataLayout->addWidget(m_pctrlViscosityUnit,3,3);
	QGroupBox *AeroDataGroup = new QGroupBox("Aerodynamic Data");
	AeroDataGroup->setLayout(AeroDataLayout);

	QGridLayout *OptionsLayout = new QGridLayout;
	m_pctrlViscous = new QCheckBox("Viscous");
	m_pctrlTiltGeom = new QCheckBox("Tilt. Geom.");
	m_pctrlWakeRollUp = new QCheckBox("Wake Roll-up");
	m_pctrlWakeParams = new QPushButton("Wake...");
	OptionsLayout->addWidget(m_pctrlViscous,1,1);
	OptionsLayout->addWidget(m_pctrlTiltGeom,1,2);
	OptionsLayout->addWidget(m_pctrlWakeRollUp,2,1);
	OptionsLayout->addWidget(m_pctrlWakeParams,2,2);
	QGroupBox *OptionsGroup = new QGroupBox("Options");
	OptionsGroup->setLayout(OptionsLayout);

	QGridLayout * GroundLayout =new QGridLayout;
	m_pctrlGroundEffect = new QCheckBox("Ground Effect");
	QLabel *lab10 = new QLabel("Height =");
	m_pctrlHeight = new FloatEdit("0.00",2);
	m_pctrlLengthUnit2 = new QLabel("mm");
	GroundLayout->addWidget(m_pctrlGroundEffect,1,1);
	GroundLayout->addWidget(lab10,2,1);
	GroundLayout->addWidget(m_pctrlHeight,2,2);
	GroundLayout->addWidget(m_pctrlLengthUnit2,2,3);
	QGroupBox *GroundGroup =  new QGroupBox("Ground Effect");
	GroundGroup->setLayout(GroundLayout);

	QGridLayout *DataLayout = new QGridLayout;
	DataLayout->addWidget(NameGroup,1,1);
	DataLayout->addWidget(TypeGroup, 1,2);
	DataLayout->addWidget(PlaneGroup,2,1);
	DataLayout->addWidget(FlightGroup,2,2);
	DataLayout->addWidget(MethodGroup,3,1);
	DataLayout->addWidget(AeroDataGroup,3,2);
	DataLayout->addWidget(OptionsGroup,4,1);
	DataLayout->addWidget(GroundGroup,4,2);

	QHBoxLayout *CommandButtons = new QHBoxLayout;
	OKButton = new QPushButton(tr("OK"));
	OKButton->setDefault(true);
	CancelButton = new QPushButton(tr("Cancel"));
	CommandButtons->addStretch(1);
	CommandButtons->addWidget(OKButton);
	CommandButtons->addStretch(1);
	CommandButtons->addWidget(CancelButton);
	CommandButtons->addStretch(1);

	QVBoxLayout * MainLayout = new QVBoxLayout(this);
	MainLayout->addLayout(DataLayout);
	MainLayout->addStretch(1);
	MainLayout->addLayout(CommandButtons);
	MainLayout->addStretch(1);
	setLayout(MainLayout);
}



void WPolarDlg::SetWPolarName()
{
	if(!m_bAutoName) return;
	QString str, strong;

	MainFrame* pMainFrame = (MainFrame*)m_pMainFrame;

	if (m_Type==1)
	{
		GetSpeedUnit(str, pMainFrame->m_SpeedUnit);
		m_WPolarName = QString("T1-%1 ").arg(m_QInf * pMainFrame->m_mstoUnit,0,'f',1);
		m_WPolarName += str;
	}
	else if(m_Type==2)
	{
		GetWeightUnit(str, pMainFrame->m_WeightUnit);
		m_WPolarName = QString("T2-%1 ").arg(m_Weight*pMainFrame->m_kgtoUnit,0,'f',3);
		m_WPolarName += str;
	}
	else if(m_Type==4)
	{
		m_WPolarName = QString("T4-%1deg").arg(m_Alpha,0,'f',3);
	}

	if(m_AnalysisType==1) m_WPolarName += "-LLT";
	else if(m_AnalysisType==2)
	{
		if(m_bVLM1)	m_WPolarName += "-VLM1";
		else		m_WPolarName += "-VLM2";
	}
	else if(m_AnalysisType==3) m_WPolarName += "-Panel";

/*	if(m_bThinSurfaces && m_AnalysisType==3)
	{
		m_WPolarName += "-Thin";
	}*/

	GetLengthUnit(str, pMainFrame->m_LengthUnit);
	strong = QString("-%1").arg(m_XCmRef*pMainFrame->m_mtoUnit,6,'f',2);
	m_WPolarName += strong + str;

	if(abs(m_Beta) > .001)
	{
		strong = QString("-b%1°").arg(m_Beta,0,'f',1);
		m_WPolarName += strong;
	}

	if(m_bTiltedGeom)
	{
		m_WPolarName += "-TG";
		if(m_bWakeRollUp) m_WPolarName += "-W";
	}

	if(!m_bViscous)
	{
		m_WPolarName += "-Inviscid";
	}

	if(m_bGround)
	{
		strong = QString("%1").arg(m_Height,0,'f',2),
		m_WPolarName += "-G"+strong;
	}
	m_pctrlWPolarName->setText(m_WPolarName);
}



void WPolarDlg::SetReynolds()
{
	QString strange, str, strUnit;
	MainFrame* pMainFrame = (MainFrame*)m_pMainFrame;
	GetSpeedUnit(strUnit, pMainFrame->m_SpeedUnit);

	if (m_Type ==1)
	{
		double RRe = m_pWing->m_TChord[0] * m_QInf/m_Viscosity;
		ReynoldsFormat(str, RRe);
		strange = "Root Re =";
		m_pctrlRRe->setText(strange+str);

		double SRe = m_pWing->m_TChord[m_pWing->m_NPanel] * m_QInf/m_Viscosity;
		ReynoldsFormat(str, SRe);
		strange = "Tip Re =";
		m_pctrlSRe->setText(strange+str);

//		m_pctrlQInfStat->setText(" ");
//		m_pctrlQInfCl->setText(" ");
	}
	else if (m_Type ==2)
	{
		double QCl =  sqrt(2.* 9.81 /m_Density* m_Weight /m_pWing->m_Area) * pMainFrame->m_mstoUnit;
		str = QString("%1").arg(QCl,5,'f',2);
		str += strUnit;
		strange = "Qinf.sqrt(Cl) =";
		m_pctrlQInfCl->setText(strange+str);

		double RRe = m_pWing->m_TChord[0] * QCl/m_Viscosity;
		ReynoldsFormat(str, RRe);
		strange = "Root Re.sqrt(Cl) =";
		m_pctrlRRe->setText(strange+str);

		double SRe = m_pWing->m_TChord[m_pWing->m_NPanel] * QCl/m_Viscosity;
		ReynoldsFormat(str, SRe);
		strange = "Tip Re.sqrt(Cl) =";
		m_pctrlSRe->setText(strange+str);
	}
	else if (m_Type ==4)
	{
//		m_pctrlQInfStat->setText(" ");
//		m_pctrlQInfCl->setText(" ");
		m_pctrlRRe->setText(" ");
		m_pctrlSRe->setText(" ");
		m_pctrlReRoot->setText(" ");
		m_pctrlReTip->setText(" ");
	}
}




void WPolarDlg::SetWingLoad()
{
	QString str,str1, str2;
	MainFrame* pMainFrame = (MainFrame*)m_pMainFrame;
	m_WingLoad = m_Weight/m_pWing->m_Area;//kg/dm2

	str = QString("%1").arg(m_WingLoad * pMainFrame->m_kgtoUnit / pMainFrame->m_m2toUnit,7,'f',3);

	GetWeightUnit(str1, pMainFrame->m_WeightUnit);
	GetAreaUnit(str2, pMainFrame->m_AreaUnit);
	m_pctrlWingLoad->setText("Wing Loading = "+str+str1+"/"+str2);


}


















