/****************************************************************************

	GLLightDlg class
	Copyright (C) 2009 Andre Deperrois xflr5@yahoo.com

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

#include "GLLightDlg.h"
#include <Units.h>
#include <gl3widget.h>
#include <QGroupBox>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QShowEvent>



Light GLLightDlg::s_Light;
Material GLLightDlg::s_Material;
Attenuation GLLightDlg::s_Attenuation;


GLLightDlg::GLLightDlg(QWidget *pParent) : QDialog(pParent)
{
	m_ModelSize = 3.0;
	setDefaults();

	setWindowTitle(tr("OpenGL Light Options"));
    setModal(false);
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

	setupLayout();

	connect(m_pctrlLight,    SIGNAL(clicked()), this, SLOT(onLight()));
	connect(m_pctrlClose,    SIGNAL(clicked()), this, SLOT(accept()));
	connect(m_pctrlDefaults, SIGNAL(clicked()), this, SLOT(onDefaults()));

	connect(m_pctrlRed,           SIGNAL(sliderMoved(int)), this, SLOT(onChanged()));
	connect(m_pctrlGreen,         SIGNAL(sliderMoved(int)), this, SLOT(onChanged()));
	connect(m_pctrlBlue,          SIGNAL(sliderMoved(int)), this, SLOT(onChanged()));
	connect(m_pctrlLightAmbient,  SIGNAL(sliderMoved(int)), this, SLOT(onChanged()));
	connect(m_pctrlLightDiffuse,  SIGNAL(sliderMoved(int)), this, SLOT(onChanged()));
	connect(m_pctrlLightSpecular, SIGNAL(sliderMoved(int)), this, SLOT(onChanged()));
	connect(m_pctrlXLight,        SIGNAL(sliderMoved(int)), this, SLOT(onChanged()));
	connect(m_pctrlYLight,        SIGNAL(sliderMoved(int)), this, SLOT(onChanged()));
	connect(m_pctrlZLight,        SIGNAL(sliderMoved(int)), this, SLOT(onChanged()));

	connect(m_pctrlMatAmbient,    SIGNAL(sliderMoved(int)), this, SLOT(onChanged()));
	connect(m_pctrlMatDiffuse,    SIGNAL(sliderMoved(int)), this, SLOT(onChanged()));
	connect(m_pctrlMatSpecular,   SIGNAL(sliderMoved(int)), this, SLOT(onChanged()));
	connect(m_pctrlMatShininess,  SIGNAL(sliderMoved(int)), this, SLOT(onChanged()));

	connect(m_pctrlConstantAttenuation,  SIGNAL(editingFinished()), this, SLOT(onChanged()));
	connect(m_pctrlLinearAttenuation,    SIGNAL(editingFinished()), this, SLOT(onChanged()));
	connect(m_pctrlQuadAttenuation,      SIGNAL(editingFinished()), this, SLOT(onChanged()));

	m_pgl3Widget = NULL;
}



void GLLightDlg::setupLayout()
{
	QGroupBox *pLightIntensityBox = new QGroupBox(tr("Light Intensity"));
	{
		QGridLayout *pLightIntensity = new QGridLayout;
		{
			QLabel *lab1 = new QLabel(tr("Diffuse"));
			QLabel *lab2 = new QLabel(tr("Ambient"));
			QLabel *lab3 = new QLabel(tr("Specular"));

			m_pctrlLightAmbient      = new QSlider(Qt::Horizontal);
			m_pctrlLightAmbient->setToolTip("Ambient:\n"
									   "Bounced light which has been scattered so much that it\n"
									   "is impossible to tell the direction to its source.\n"
									   "It is not attenuated by distance, and disappears if\n"
									   "the light is turned off.");
			m_pctrlLightDiffuse      = new QSlider(Qt::Horizontal);
			m_pctrlLightDiffuse->setToolTip("Diffuse:\n"
									   "Directional light which is brighter on perpendicular\n"
									   "surfaces. Its reflection is scattered evenly.");
			m_pctrlLightSpecular     = new QSlider(Qt::Horizontal);
			m_pctrlLightSpecular->setToolTip("Specular:\n"
										"Directional light which tends to reflect in a preferred\n"
										"direction. It is associated with shininess.");

			m_pctrlLightAmbient->setMinimum(0);
			m_pctrlLightAmbient->setMaximum(100);
			m_pctrlLightAmbient->setTickInterval(10);
			m_pctrlLightDiffuse->setMinimum(0);
			m_pctrlLightDiffuse->setMaximum(100);
			m_pctrlLightDiffuse->setTickInterval(10);
			m_pctrlLightSpecular->setMinimum(0);
			m_pctrlLightSpecular->setMaximum(100);
			m_pctrlLightSpecular->setTickInterval(10);
			m_pctrlLightDiffuse->setTickPosition(QSlider::TicksBelow);
			m_pctrlLightAmbient->setTickPosition(QSlider::TicksBelow);
			m_pctrlLightSpecular->setTickPosition(QSlider::TicksBelow);

			m_pctrlLightAmbientLabel = new QLabel;
			m_pctrlLightDiffuseLabel = new QLabel;
			m_pctrlLightSpecularLabel = new QLabel;
			pLightIntensity->addWidget(lab2,1,1);
			pLightIntensity->addWidget(lab1,2,1);
			pLightIntensity->addWidget(lab3,3,1);
			pLightIntensity->addWidget(m_pctrlLightAmbient,1,2);
			pLightIntensity->addWidget(m_pctrlLightDiffuse,2,2);
			pLightIntensity->addWidget(m_pctrlLightSpecular,3,2);
			pLightIntensity->addWidget(m_pctrlLightAmbientLabel,1,3);
			pLightIntensity->addWidget(m_pctrlLightDiffuseLabel,2,3);
			pLightIntensity->addWidget(m_pctrlLightSpecularLabel,3,3);
			pLightIntensityBox->setLayout(pLightIntensity);
		}
	}

	QGroupBox *pLightColorBox = new QGroupBox(tr("Light Color"));
	{
		QGridLayout *pLightColor = new QGridLayout;
		{
			QLabel *lab11 = new QLabel(tr("Red"));
			QLabel *lab12 = new QLabel(tr("Green"));
			QLabel *lab13 = new QLabel(tr("Blue"));
			m_pctrlRed    = new QSlider(Qt::Horizontal);
			m_pctrlGreen  = new QSlider(Qt::Horizontal);
			m_pctrlBlue   = new QSlider(Qt::Horizontal);
			m_pctrlRed->setMinimum(0);
			m_pctrlRed->setMaximum(100);
			m_pctrlRed->setTickInterval(10);
			m_pctrlGreen->setMinimum(0);
			m_pctrlGreen->setMaximum(100);
			m_pctrlGreen->setTickInterval(10);
			m_pctrlBlue->setMinimum(0);
			m_pctrlBlue->setMaximum(100);
			m_pctrlBlue->setTickInterval(10);
			m_pctrlRed->setTickPosition(QSlider::TicksBelow);
			m_pctrlGreen->setTickPosition(QSlider::TicksBelow);
			m_pctrlBlue->setTickPosition(QSlider::TicksBelow);

			m_pctrlLightRed   = new QLabel;
			m_pctrlLightGreen = new QLabel;
			m_pctrlLightBlue  = new QLabel;

			pLightColor->addWidget(lab11,1,1);
			pLightColor->addWidget(lab12,2,1);
			pLightColor->addWidget(lab13,3,1);
			pLightColor->addWidget(m_pctrlRed,1,2);
			pLightColor->addWidget(m_pctrlGreen,2,2);
			pLightColor->addWidget(m_pctrlBlue,3,2);
			pLightColor->addWidget(m_pctrlLightRed,1,3);
			pLightColor->addWidget(m_pctrlLightGreen,2,3);
			pLightColor->addWidget(m_pctrlLightBlue,3,3);
			pLightColorBox->setLayout(pLightColor);
		}
	}

	QGroupBox *pLightPositionBox = new QGroupBox(tr("Light Position"));
	{
		QGridLayout *pLightPosition = new QGridLayout;
		{
			QLabel *lab21 = new QLabel(tr("x"));
			QLabel *lab22 = new QLabel(tr("y"));
			QLabel *lab23 = new QLabel(tr("z"));

			m_pctrlXLight = new QSlider(Qt::Horizontal);
			m_pctrlYLight = new QSlider(Qt::Horizontal);
			m_pctrlZLight = new QSlider(Qt::Horizontal);
			m_pctrlXLight->setMinimum(0);
			m_pctrlXLight->setMaximum(100);
			m_pctrlXLight->setTickInterval(10);
			m_pctrlYLight->setMinimum(0);
			m_pctrlYLight->setMaximum(100);
			m_pctrlYLight->setTickInterval(10);
			m_pctrlZLight->setMinimum(0);
			m_pctrlZLight->setMaximum(100);
			m_pctrlZLight->setTickInterval(10);
			m_pctrlXLight->setTickPosition(QSlider::TicksBelow);
			m_pctrlYLight->setTickPosition(QSlider::TicksBelow);
			m_pctrlZLight->setTickPosition(QSlider::TicksBelow);
			m_pctrlposXValue = new QLabel(Units::lengthUnitLabel());
			m_pctrlposYValue = new QLabel(Units::lengthUnitLabel());
			m_pctrlposZValue = new QLabel(Units::lengthUnitLabel());

			pLightPosition->addWidget(lab21,1,1);
			pLightPosition->addWidget(lab22,2,1);
			pLightPosition->addWidget(lab23,3,1);
			pLightPosition->addWidget(m_pctrlXLight,1,2);
			pLightPosition->addWidget(m_pctrlYLight,2,2);
			pLightPosition->addWidget(m_pctrlZLight,3,2);
			pLightPosition->addWidget(m_pctrlposXValue,1,3);
			pLightPosition->addWidget(m_pctrlposYValue,2,3);
			pLightPosition->addWidget(m_pctrlposZValue,3,3);
			pLightPositionBox->setLayout(pLightPosition);
		}
	}

	QGroupBox *pAttenuationBox = new QGroupBox(tr("Attenuation factors"));
	{
		QGridLayout *pAttLayout = new QGridLayout;
		{
			QLabel *pConstant = new QLabel(tr("Constant"));
			QLabel *pLinear = new QLabel(tr("Linear"));
			QLabel *pQuadratic = new QLabel(tr("Quadratic"));
			m_pctrlConstantAttenuation = new DoubleEdit(0.0);
			m_pctrlLinearAttenuation = new DoubleEdit(0.0);
			m_pctrlQuadAttenuation = new DoubleEdit(0.0);
			m_pctrlAttenuation = new QLabel(QString::fromUtf8("Attenuation factor = 1.0/(1.0+2.0*d+3.0*d²)"));
			m_pctrlAttenuation->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
			pAttLayout->addWidget(pConstant,1,1);
			pAttLayout->addWidget(pLinear,2,1);
			pAttLayout->addWidget(pQuadratic,3,1);
			pAttLayout->addWidget(m_pctrlConstantAttenuation,1,2);
			pAttLayout->addWidget(m_pctrlLinearAttenuation,2,2);
			pAttLayout->addWidget(m_pctrlQuadAttenuation,3,2);
			pAttLayout->addWidget(m_pctrlAttenuation,4,1,1,2);
		}
		pAttenuationBox->setLayout(pAttLayout);
	}

	QGroupBox *pMaterialDataBox = new QGroupBox(tr("Material"));
	{
		QGridLayout *pMaterialData = new QGridLayout;
		{
			m_pctrlMatDiffuse   = new QSlider(Qt::Horizontal);
			m_pctrlMatAmbient   = new QSlider(Qt::Horizontal);
			m_pctrlMatSpecular  = new QSlider(Qt::Horizontal);
			m_pctrlMatShininess = new QSlider(Qt::Horizontal);
			m_pctrlMatAmbient->setRange(0, 100);
			m_pctrlMatAmbient->setTickInterval(10);
			m_pctrlMatDiffuse->setRange(0, 100);
			m_pctrlMatDiffuse->setTickInterval(10);
			m_pctrlMatSpecular->setRange(0, 100);
			m_pctrlMatSpecular->setTickInterval(10);
			m_pctrlMatShininess->setRange(0, 64);
			m_pctrlMatShininess->setTickInterval(2);
			m_pctrlMatDiffuse->setTickPosition(QSlider::TicksBelow);
			m_pctrlMatAmbient->setTickPosition(QSlider::TicksBelow);
			m_pctrlMatSpecular->setTickPosition(QSlider::TicksBelow);
			m_pctrlMatShininess->setTickPosition(QSlider::TicksBelow);


			QLabel *lab31 = new QLabel(tr("Diffuse"));
			QLabel *lab32 = new QLabel(tr("Ambient"));
			QLabel *lab33 = new QLabel(tr("Specular"));
			QLabel *lab35 = new QLabel(tr("Shininess"));
			m_pctrlMatDiffuseLabel   = new QLabel("1.0");
			m_pctrlMatAmbientLabel   = new QLabel("1.0");
			m_pctrlMatSpecularLabel  = new QLabel("1.0");
			m_pctrlMatShininessLabel = new QLabel("1");
			pMaterialData->addWidget(lab31,1,1);
			pMaterialData->addWidget(lab32,2,1);
			pMaterialData->addWidget(lab33,3,1);
			pMaterialData->addWidget(lab35,4,1);
			pMaterialData->addWidget(m_pctrlMatDiffuse,1,2);
			pMaterialData->addWidget(m_pctrlMatAmbient,2,2);
			pMaterialData->addWidget(m_pctrlMatSpecular,3,2);
			pMaterialData->addWidget(m_pctrlMatShininess,4,2);
			pMaterialData->addWidget(m_pctrlMatDiffuseLabel,1,3);
			pMaterialData->addWidget(m_pctrlMatAmbientLabel,2,3);
			pMaterialData->addWidget(m_pctrlMatSpecularLabel,3,3);
			pMaterialData->addWidget(m_pctrlMatShininessLabel,4,3);
			pMaterialDataBox->setLayout(pMaterialData);
		}
	}
	QHBoxLayout *pCommandButtons = new QHBoxLayout;
	{
		m_pctrlClose = new QPushButton(tr("Close"));
		m_pctrlDefaults = new QPushButton(tr("Reset Defaults"));
		m_pctrlDefaults->setDefault(false);
		m_pctrlDefaults->setAutoDefault(false);
		m_pctrlClose->setDefault(false);
		m_pctrlClose->setAutoDefault(false);
		pCommandButtons->addStretch(1);
		pCommandButtons->addWidget(m_pctrlDefaults);
		pCommandButtons->addStretch(1);
		pCommandButtons->addWidget(m_pctrlClose);
		pCommandButtons->addStretch(1);
	}

	QVBoxLayout *pSliderLayout = new QVBoxLayout;
	{
		pSliderLayout->addStretch(1);
		pSliderLayout->addWidget(pLightIntensityBox);
		pSliderLayout->addStretch(1);
		pSliderLayout->addWidget(pLightColorBox);
		pSliderLayout->addStretch(1);
		pSliderLayout->addWidget(pLightPositionBox);
		pSliderLayout->addStretch(1);
		pSliderLayout->addWidget(pMaterialDataBox);
		pSliderLayout->addStretch(1);
		pSliderLayout->addWidget(pAttenuationBox);
		pSliderLayout->addStretch(1);
	}


	QVBoxLayout *pMainLayout = new QVBoxLayout;
	{
		m_pctrlLight = new QCheckBox(tr("Light"));
		pMainLayout->addWidget(m_pctrlLight);
		pMainLayout->addLayout(pSliderLayout);
		pMainLayout->addLayout(pCommandButtons);
	}

	setLayout(pMainLayout);
}



void GLLightDlg::apply()
{
	readParams();
	setLabels();

	if(m_pgl3Widget)
	{
		GL3Widget *pgl3Widget =(GL3Widget*)m_pgl3Widget;
		pgl3Widget->glSetupLight();
		pgl3Widget->update();
	}
}



void GLLightDlg::onChanged()
{
	apply();
}


void GLLightDlg::onDefaults()
{
	setDefaults();
	setParams();
	setEnabled();

	if(m_pgl3Widget)
	{
		GL3Widget *pgl3Widget =(GL3Widget*)m_pgl3Widget;
		pgl3Widget->glSetupLight();
		pgl3Widget->update();
	}
}



void GLLightDlg::readParams(void)
{
	s_Light.m_bIsLightOn = m_pctrlLight->isChecked();

	s_Light.m_Red     = (float)m_pctrlRed->value()    /100.0f;
	s_Light.m_Green   = (float)m_pctrlGreen->value()  /100.0f;
	s_Light.m_Blue    = (float)m_pctrlBlue->value()   /100.0f;


	float factor = 10.0f;
	s_Light.m_X  = (float)m_pctrlXLight->value()/factor-5.0f;
	s_Light.m_Y  = (float)m_pctrlYLight->value()/factor-5.0f;
	s_Light.m_Z  = (float)m_pctrlZLight->value()/factor;

	s_Light.m_Ambient     = (float)m_pctrlLightAmbient->value()  / 20.0f;
	s_Light.m_Diffuse     = (float)m_pctrlLightDiffuse->value()  / 20.0f;
	s_Light.m_Specular    = (float)m_pctrlLightSpecular->value() / 20.0f;


	s_Material.m_Ambient      = (float)m_pctrlMatAmbient->value()   /100.0f;
	s_Material.m_Diffuse      = (float)m_pctrlMatDiffuse->value()   /100.0f;
	s_Material.m_Specular     = (float)m_pctrlMatSpecular->value()  /100.0f;
	s_Material.m_iShininess   = m_pctrlMatShininess->value();

	s_Attenuation.m_Constant  = m_pctrlConstantAttenuation->value();
	s_Attenuation.m_Linear    = m_pctrlLinearAttenuation->value();
	s_Attenuation.m_Quadratic = m_pctrlQuadAttenuation->value();
}


void GLLightDlg::setParams(void)
{
	m_pctrlLight->setChecked(s_Light.m_bIsLightOn);

	m_pctrlLightAmbient->setValue(   (int)(s_Light.m_Ambient  *20.0));
	m_pctrlLightDiffuse->setValue(   (int)(s_Light.m_Diffuse  *20.0));
	m_pctrlLightSpecular->setValue(  (int)(s_Light.m_Specular *20.0));

	float factor = 10.0f;
	m_pctrlXLight->setValue((int)((s_Light.m_X+5.0)*factor));
	m_pctrlYLight->setValue((int)((s_Light.m_Y+5.0)*factor));
	m_pctrlZLight->setValue((int)((s_Light.m_Z)*factor));


	m_pctrlRed->setValue(  (int)(s_Light.m_Red  *100.0));
	m_pctrlGreen->setValue((int)(s_Light.m_Green*100.0));
	m_pctrlBlue->setValue( (int)(s_Light.m_Blue *100.0));

	m_pctrlMatAmbient->setValue(   (int)(s_Material.m_Ambient  *100.0));
	m_pctrlMatDiffuse->setValue(   (int)(s_Material.m_Diffuse  *100.0));
	m_pctrlMatSpecular->setValue(  (int)(s_Material.m_Specular *100.0));
	m_pctrlMatShininess->setValue(s_Material.m_iShininess);

	m_pctrlConstantAttenuation->setValue(s_Attenuation.m_Constant);
	m_pctrlLinearAttenuation->setValue(s_Attenuation.m_Linear);
	m_pctrlQuadAttenuation->setValue(s_Attenuation.m_Quadratic);

	setLabels();
}


void GLLightDlg::setModelSize(double span)
{
	m_ModelSize = span;
}


void GLLightDlg::setLabels()
{
	QString strong;

	strong.sprintf("%7.1f", s_Light.m_Ambient);
	m_pctrlLightAmbientLabel->setText(strong);
	strong.sprintf("%7.1f", s_Light.m_Diffuse);
	m_pctrlLightDiffuseLabel->setText(strong);
	strong.sprintf("%7.1f", s_Light.m_Specular);
	m_pctrlLightSpecularLabel->setText(strong);	strong.sprintf("%7.1f", s_Light.m_X*Units::mtoUnit());
	m_pctrlposXValue->setText(strong + Units::lengthUnitLabel());
	strong.sprintf("%7.1f", s_Light.m_Y*Units::mtoUnit());
	m_pctrlposYValue->setText(strong + Units::lengthUnitLabel());
	strong.sprintf("%7.1f", s_Light.m_Z*Units::mtoUnit());
	m_pctrlposZValue->setText(strong + Units::lengthUnitLabel());	strong.sprintf("%7.1f", s_Light.m_Red);
	m_pctrlLightRed->setText(strong);
	strong.sprintf("%7.1f", s_Light.m_Green);
	m_pctrlLightGreen->setText(strong);
	strong.sprintf("%7.1f", s_Light.m_Blue);
	m_pctrlLightBlue->setText(strong);

	strong.sprintf("%7.1f", s_Material.m_Ambient);
	m_pctrlMatAmbientLabel->setText(strong);
	strong.sprintf("%7.1f", s_Material.m_Diffuse);
	m_pctrlMatDiffuseLabel->setText(strong);
	strong.sprintf("%7.1f", s_Material.m_Specular);
	m_pctrlMatSpecularLabel->setText(strong);
	strong.sprintf("%d", s_Material.m_iShininess);
	m_pctrlMatShininessLabel->setText(strong);

	strong.sprintf("Attenuation factor = 1.0/(%4.1f+%4.1f*d+%4.1f*d²)",
				   s_Attenuation.m_Constant,s_Attenuation.m_Linear, s_Attenuation.m_Quadratic);
	m_pctrlAttenuation->setText(strong);
}



bool GLLightDlg::loadSettings(QSettings *pSettings)
{
	pSettings->beginGroup("GLLight3");
	{
	//  we're reading/loading
		s_Light.m_Ambient           = pSettings->value("Ambient",0.3).toDouble();
		s_Light.m_Diffuse           = pSettings->value("Diffuse",1.2).toDouble();
		s_Light.m_Specular          = pSettings->value("Specular",0.50).toDouble();

		s_Light.m_X                 = pSettings->value("XLight", 0.300).toDouble();
		s_Light.m_Y                 = pSettings->value("YLight", 0.300).toDouble();
		s_Light.m_Z                 = pSettings->value("ZLight", 3.000).toDouble();

		s_Light.m_Red               = pSettings->value("RedLight",1.0).toDouble();
		s_Light.m_Green             = pSettings->value("GreenLight",1.0).toDouble();
		s_Light.m_Blue              = pSettings->value("BlueLight",1.0).toDouble();

		s_Material.m_Ambient        = pSettings->value("MatAmbient",1.0).toDouble();
		s_Material.m_Diffuse        = pSettings->value("MatDiffuser",1.0).toDouble();
		s_Material.m_Specular       = pSettings->value("MatSpecular",1.0).toDouble();
		s_Material.m_iShininess     = pSettings->value("MatShininess", 5).toInt();

		s_Attenuation.m_Constant    = pSettings->value("ConstantAtt",2.0).toDouble();
		s_Attenuation.m_Linear      = pSettings->value("LinearAtt",1.0).toDouble();
		s_Attenuation.m_Quadratic   = pSettings->value("QuadraticAtt",.5).toDouble();

		s_Light.m_bIsLightOn        = pSettings->value("bLight", true).toBool();
	}
	pSettings->endGroup();
	return true;
}


void GLLightDlg::setDefaults()
{
	s_Light.m_Red   = 1.0f;
	s_Light.m_Green = 1.0f;
	s_Light.m_Blue  = 1.0f;

	s_Light.m_Ambient      = 0.3f;
	s_Light.m_Diffuse      = 1.20f;
	s_Light.m_Specular     = 0.50f;

	s_Light.m_X   =  0.1f * m_ModelSize;
	s_Light.m_Y   =  0.1f * m_ModelSize;
	s_Light.m_Z   =  m_ModelSize;

	s_Material.m_Ambient   = 1.0f;
	s_Material.m_Diffuse   = 1.0f;
	s_Material.m_Specular  = 1.0f;
	s_Material.m_iShininess = 5;

	s_Attenuation.m_Constant  = 1.0;
	s_Attenuation.m_Linear    = 0.5;
	s_Attenuation.m_Quadratic = 0.0;

	s_Light.m_bIsLightOn = true;
}



bool GLLightDlg::saveSettings(QSettings *pSettings)
{
	pSettings->beginGroup("GLLight3");
	{
		pSettings->setValue("Ambient",      s_Light.m_Ambient);
		pSettings->setValue("Diffuse",      s_Light.m_Diffuse);
		pSettings->setValue("Specular",     s_Light.m_Specular);

		pSettings->setValue("XLight",       s_Light.m_X);
		pSettings->setValue("YLight",       s_Light.m_Y);
		pSettings->setValue("ZLight",       s_Light.m_Z);
		pSettings->setValue("RedLight",     s_Light.m_Red);
		pSettings->setValue("GreenLight",   s_Light.m_Green);
		pSettings->setValue("BlueLight",    s_Light.m_Blue);
		pSettings->setValue("bLight",       s_Light.m_bIsLightOn);

		pSettings->setValue("MatAmbient",   s_Material.m_Ambient);
		pSettings->setValue("MatDiffuser",  s_Material.m_Diffuse);
		pSettings->setValue("MatSpecular",  s_Material.m_Specular);
		pSettings->setValue("MatShininess", s_Material.m_iShininess);

		pSettings->setValue("ConstantAtt",  s_Attenuation.m_Constant);
		pSettings->setValue("LinearAtt",    s_Attenuation.m_Linear);
		pSettings->setValue("QuadraticAtt", s_Attenuation.m_Quadratic);

	}
	pSettings->endGroup();

	return true;
}


void GLLightDlg::showEvent(QShowEvent *event)
{
	setParams();
	setEnabled();
//	apply();
	event->accept();
}



QSize GLLightDlg::minimumSizeHint() const
{
	return QSize(250, 350);
}


QSize GLLightDlg::sizeHint() const
{
	return QSize(350, 400);
}


void GLLightDlg::onLight()
{
	s_Light.m_bIsLightOn = m_pctrlLight->isChecked();
	setEnabled();
	apply();
}


void GLLightDlg::setEnabled()
{
	m_pctrlRed->setEnabled(s_Light.m_bIsLightOn);
	m_pctrlGreen->setEnabled(s_Light.m_bIsLightOn);
	m_pctrlBlue->setEnabled(s_Light.m_bIsLightOn);

	m_pctrlLightAmbient->setEnabled(s_Light.m_bIsLightOn);
	m_pctrlLightDiffuse->setEnabled(s_Light.m_bIsLightOn);
	m_pctrlLightSpecular->setEnabled(s_Light.m_bIsLightOn);

	m_pctrlXLight->setEnabled(s_Light.m_bIsLightOn);
	m_pctrlYLight->setEnabled(s_Light.m_bIsLightOn);
	m_pctrlZLight->setEnabled(s_Light.m_bIsLightOn);

	m_pctrlMatAmbient->setEnabled(false);
	m_pctrlMatDiffuse->setEnabled(false);
	m_pctrlMatSpecular->setEnabled(false);
	m_pctrlMatShininess->setEnabled(s_Light.m_bIsLightOn);
}









