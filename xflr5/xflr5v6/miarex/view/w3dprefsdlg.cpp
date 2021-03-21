/****************************************************************************

    W3dPrefsDlg Class
    Copyright (C) 2009-2016 Andre Deperrois XFLR5@yahoo.com

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

#include <QGroupBox>
#include <QLabel>
#include <QCheckBox>
#include <QGridLayout>
#include <QColorDialog>
#include <QPushButton>

#include "w3dprefsdlg.h"
#include <misc/line/linepickerdlg.h>

#include <misc/line/linebtn.h>
#include <misc/color/colorbutton.h>
#include <misc/text/intedit.h>


bool W3dPrefsDlg::s_bAutoAdjustScale = true;
bool W3dPrefsDlg::s_bWakePanels = false;

double W3dPrefsDlg::s_MassRadius = .017;
QColor W3dPrefsDlg::s_MassColor = QColor(67, 151, 169);


LS2 W3dPrefsDlg::s_3DAxisStyle    = {true, Line::DASHDOT, 2, QColor(150,150,150),     Line::NOSYMBOL};
LS2 W3dPrefsDlg::s_VLMStyle       = {true, Line::SOLID,   2, QColor(180,180,180),     Line::NOSYMBOL};

LS2 W3dPrefsDlg::s_OutlineStyle   = {true, Line::SOLID,   2, QColor(73,73,73),        Line::NOSYMBOL};
LS2 W3dPrefsDlg::s_XCPStyle       = {true, Line::SOLID,   2, QColor(50, 150, 50),     Line::NOSYMBOL};
LS2 W3dPrefsDlg::s_MomentStyle    = {true, Line::SOLID,   2, QColor(200, 100, 100),   Line::NOSYMBOL};
LS2 W3dPrefsDlg::s_IDragStyle     = {true, Line::SOLID,   2, QColor(255,200,0),       Line::NOSYMBOL};
LS2 W3dPrefsDlg::s_DownwashStyle  = {true, Line::SOLID,   2, QColor(255, 100, 100),   Line::NOSYMBOL};
LS2 W3dPrefsDlg::s_WakeStyle      = {true, Line::SOLID,   2, QColor(0, 150, 200),     Line::NOSYMBOL};
LS2 W3dPrefsDlg::s_CpStyle        = {true, Line::SOLID,   2, QColor(255,0,0),         Line::NOSYMBOL};
LS2 W3dPrefsDlg::s_StreamStyle    = {true, Line::SOLID,   2, QColor(200, 150, 255),   Line::NOSYMBOL};

LS2 W3dPrefsDlg::s_VDragStyle     = {true, Line::SOLID,   2, QColor(200,100,220),     Line::NOSYMBOL};
LS2 W3dPrefsDlg::s_TopStyle       = {true, Line::SOLID,   2, QColor(171, 103, 220),   Line::NOSYMBOL};
LS2 W3dPrefsDlg::s_BotStyle       = {true, Line::DASH,    2, QColor(171, 103, 220),   Line::NOSYMBOL};

int W3dPrefsDlg::s_iChordwiseRes = 73;
int W3dPrefsDlg::s_iBodyAxialRes = 23;
int W3dPrefsDlg::s_iBodyHoopRes  = 17;
bool W3dPrefsDlg::s_bAnimateTransitions = true;
bool W3dPrefsDlg::s_bEnableClipPlane = false;


W3dPrefsDlg::W3dPrefsDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("3D Styles"));
    setupLayout();

    connect(m_plbAxis,               SIGNAL(clickedLB()),   SLOT(on3DAxis()));
    connect(m_plbOutline,            SIGNAL(clickedLB()),   SLOT(onOutline()));
    connect(m_plbVLMMesh,            SIGNAL(clickedLB()),   SLOT(onVLMMesh()));
    connect(m_plbTopTrans,           SIGNAL(clickedLB()),   SLOT(onTopTrans()));
    connect(m_plbBotTrans,           SIGNAL(clickedLB()),   SLOT(onBotTrans()));
    connect(m_plbLift,               SIGNAL(clickedLB()),   SLOT(onXCP()));
    connect(m_plbMoments,            SIGNAL(clickedLB()),   SLOT(onMoments()));
    connect(m_plbInducedDrag,        SIGNAL(clickedLB()),   SLOT(onIDrag()));
    connect(m_plbViscousDrag,        SIGNAL(clickedLB()),   SLOT(onVDrag()));
    connect(m_plbDownwash,           SIGNAL(clickedLB()),   SLOT(onDownwash()));
    connect(m_plbStreamLines,        SIGNAL(clickedLB()),   SLOT(onStreamLines()));
    connect(m_plbWakePanels,         SIGNAL(clickedLB()),   SLOT(onWakePanels()));
    connect(m_pcbMassColor,          SIGNAL(clicked()),     SLOT(onMasses()));
}


void W3dPrefsDlg::initDialog()
{
    m_plbAxis->setTheStyle(s_3DAxisStyle);
    m_plbOutline->setTheStyle(s_OutlineStyle);
    m_plbVLMMesh->setTheStyle(s_VLMStyle);
    m_plbLift->setTheStyle(s_XCPStyle);
    m_plbMoments->setTheStyle(s_MomentStyle);
    m_plbInducedDrag->setTheStyle(s_IDragStyle);
    m_plbViscousDrag->setTheStyle(s_VDragStyle);
    m_plbDownwash->setTheStyle(s_DownwashStyle);
    m_plbWakePanels->setTheStyle(s_WakeStyle);
    m_plbStreamLines->setTheStyle(s_StreamStyle);
    m_plbTopTrans->setTheStyle(s_TopStyle);
    m_plbBotTrans->setTheStyle(s_BotStyle);

    m_pcbMassColor->setColor(s_MassColor);
    m_pieChordwiseRes->setValue(s_iChordwiseRes);
    m_pieBodyAxialRes->setValue(s_iBodyAxialRes);
    m_pcieBodyHoopRes->setValue(s_iBodyHoopRes);

    m_pchAnimateTransitions->setChecked(s_bAnimateTransitions);
    m_pchAutoAdjustScale->setChecked(s_bAutoAdjustScale);
    m_pchEnableClipPlane->setChecked(s_bEnableClipPlane);
}



void W3dPrefsDlg::setupLayout()
{
    QLabel *lab1 = new QLabel(tr("Axis"));
    QLabel *lab2 = new QLabel(tr("Outline"));
    QLabel *lab3 = new QLabel(tr("VLM Mesh"));
    QLabel *lab4 = new QLabel(tr("Top transition"));
    QLabel *lab5 = new QLabel(tr("Bottom transition"));
    QLabel *lab6 = new QLabel(tr("Lift"));
    QLabel *lab7 = new QLabel(tr("Moments"));
    QLabel *lab8 = new QLabel(tr("Induced Drag"));
    QLabel *lab9 = new QLabel(tr("Viscous Drag"));
    QLabel *lab10 = new QLabel(tr("Downwash"));
    QLabel *lab11 = new QLabel(tr("WakePanels"));
    QLabel *lab12 = new QLabel(tr("Streamlines"));
    QLabel *lab13 = new QLabel(tr("Masses"));

    lab1->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
    lab2->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
    lab3->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
    lab4->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
    lab5->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
    lab6->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
    lab7->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
    lab8->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
    lab9->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
    lab10->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
    lab11->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
    lab12->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
    lab13->setAlignment(Qt::AlignVCenter|Qt::AlignRight);

    m_plbAxis     = new LineBtn(this);
    m_plbOutline  = new LineBtn(this);
    m_plbVLMMesh  = new LineBtn(this);
    m_plbTopTrans = new LineBtn(this);
    m_plbBotTrans = new LineBtn(this);
    m_plbLift     = new LineBtn(this);
    m_plbMoments  = new LineBtn(this);
    m_plbInducedDrag = new LineBtn(this);
    m_plbViscousDrag = new LineBtn(this);
    m_plbDownwash    = new LineBtn(this);
    m_plbWakePanels  = new LineBtn(this);
    m_plbStreamLines  = new LineBtn(this);
    m_pcbMassColor = new ColorButton;

    QGroupBox *pColorPrefsBox = new QGroupBox(tr("Color settings"));
    {
        QGridLayout *pColorPrefsLayout = new QGridLayout;
        {
            pColorPrefsLayout->setColumnStretch(1,1);
            pColorPrefsLayout->setColumnStretch(2,2);
            pColorPrefsLayout->setColumnStretch(3,1);
            pColorPrefsLayout->setColumnStretch(4,2);
            pColorPrefsLayout->addWidget(lab1,1,1);
            pColorPrefsLayout->addWidget(lab2,2,1);
            pColorPrefsLayout->addWidget(lab3,3,1);
            pColorPrefsLayout->addWidget(lab4,4,1);
            pColorPrefsLayout->addWidget(lab5,5,1);
            pColorPrefsLayout->addWidget(lab6,6,1);
            pColorPrefsLayout->addWidget(lab7,1,3);
            pColorPrefsLayout->addWidget(lab8,2,3);
            pColorPrefsLayout->addWidget(lab9,3,3);
            pColorPrefsLayout->addWidget(lab10,4,3);
            pColorPrefsLayout->addWidget(lab11,5,3);
            pColorPrefsLayout->addWidget(lab12,6,3);
            pColorPrefsLayout->addWidget(lab13,7,3);

            pColorPrefsLayout->addWidget(m_plbAxis,1,2);
            pColorPrefsLayout->addWidget(m_plbOutline,2,2);
            pColorPrefsLayout->addWidget(m_plbVLMMesh,3,2);
            pColorPrefsLayout->addWidget(m_plbTopTrans,4,2);
            pColorPrefsLayout->addWidget(m_plbBotTrans,5,2);
            pColorPrefsLayout->addWidget(m_plbLift,6,2);
            pColorPrefsLayout->addWidget(m_plbMoments,1,4);
            pColorPrefsLayout->addWidget(m_plbInducedDrag,2,4);
            pColorPrefsLayout->addWidget(m_plbViscousDrag,3,4);
            pColorPrefsLayout->addWidget(m_plbDownwash,4,4);
            pColorPrefsLayout->addWidget(m_plbWakePanels,5,4);
            pColorPrefsLayout->addWidget(m_plbStreamLines,6,4);
            pColorPrefsLayout->addWidget(m_pcbMassColor,7,4);
        }
        pColorPrefsBox->setLayout(pColorPrefsLayout);
    }

    QGroupBox *pTessBox = new QGroupBox(tr("Tessellation"));
    {
/*        QGridLayout *pTessLayout = new QGridLayout;
        {
            pTessLayout->addWidget(pTessBodyAxial,3,1);
            pTessLayout->addWidget(m_pctrlBodyAxialRes,3,2);
            pTessLayout->addWidget(pTessBodyHoop,4,1);
            pTessLayout->addWidget(m_pctrlBodyHoopRes,4,2);
        }*/

        QVBoxLayout *pTessLayout = new QVBoxLayout;
        {
            QLabel *pTessLabel = new QLabel(tr("Increase the number of points to improve the resolution\n"
                                               "of the surfaces.This may reduce the display speed.\n"));
            QLabel *pTessChords = new QLabel(tr("Wing chordwise direction"));
            m_pieChordwiseRes = new IntEdit(37,this);
            QLabel *pTessBodyAxial = new QLabel(tr("Body axial direction"));
            m_pieBodyAxialRes = new IntEdit(29, this);
            QLabel *pTessBodyHoop = new QLabel(tr("Body hoop direction"));
            m_pcieBodyHoopRes = new IntEdit(17,this);

            QHBoxLayout *pChordResLayout = new QHBoxLayout;
            {
                pChordResLayout->addStretch();
                pChordResLayout->addWidget(pTessChords);
                pChordResLayout->addWidget(m_pieChordwiseRes);
            }
            QHBoxLayout *pAxialResLayout = new QHBoxLayout;
            {
                pAxialResLayout->addStretch();
                pAxialResLayout->addWidget(pTessBodyAxial);
                pAxialResLayout->addWidget(m_pieBodyAxialRes);
            }
            QHBoxLayout *pHoopResLayout = new QHBoxLayout;
            {
                pHoopResLayout->addStretch();
                pHoopResLayout->addWidget(pTessBodyHoop);
                pHoopResLayout->addWidget(m_pcieBodyHoopRes);
            }
            pTessLayout->addWidget(pTessLabel);
            pTessLayout->addLayout(pChordResLayout);
            pTessLayout->addLayout(pAxialResLayout);
            pTessLayout->addLayout(pHoopResLayout);
        }
        pTessBox->setLayout(pTessLayout);
    }

    m_pchAnimateTransitions = new QCheckBox(tr("Animate view transitions"));
    m_pchAutoAdjustScale = new QCheckBox(tr("Auto Adjust 3D scale"));
    m_pchAutoAdjustScale->setToolTip(tr("Automatically adjust the 3D scale to fit the plane in the display when switching between planes"));
    m_pchEnableClipPlane = new QCheckBox(tr("Enable clip plane"));

    QHBoxLayout *pCommandButtons = new QHBoxLayout;
    {
        QPushButton *pOKButton = new QPushButton(tr("Close"));
        pOKButton->setDefault(true);
        QPushButton *pResetButton = new QPushButton(tr("Reset Defaults"));
        pCommandButtons->addStretch(1);
        pCommandButtons->addWidget(pResetButton);
        pCommandButtons->addStretch(1);
        pCommandButtons->addWidget(pOKButton);
        pCommandButtons->addStretch(1);
        connect(pResetButton, SIGNAL(clicked()), SLOT(onResetDefaults()));
        connect(pOKButton,    SIGNAL(clicked()), SLOT(onOK()));
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addStretch(1);
        pMainLayout->addWidget(pColorPrefsBox);
        pMainLayout->addStretch(1);
        pMainLayout->addWidget(pTessBox);
        pMainLayout->addStretch(1);
        pMainLayout->addWidget(m_pchAnimateTransitions);
        pMainLayout->addStretch(1);
        pMainLayout->addWidget(m_pchAutoAdjustScale);
        pMainLayout->addStretch(1);
        pMainLayout->addWidget(m_pchEnableClipPlane);
        pMainLayout->addSpacing(20);
        pMainLayout->addLayout(pCommandButtons);
        pMainLayout->addStretch(1);
    }
    setLayout(pMainLayout);
}


void W3dPrefsDlg::onOutline()
{
    LinePickerDlg LPdlg(this);
    LPdlg.setTheStyle(s_OutlineStyle);
    LPdlg.initDialog(false);
    if (QDialog::Accepted == LPdlg.exec())
    {
        s_OutlineStyle = LPdlg.ls2();
        m_plbOutline->setTheStyle(s_OutlineStyle);
    }
}

void W3dPrefsDlg::on3DAxis()
{
    LinePickerDlg LPdlg(this);
    LPdlg.setTheStyle(s_3DAxisStyle);
    LPdlg.initDialog(false);

    if (QDialog::Accepted == LPdlg.exec())
    {
        s_3DAxisStyle = LPdlg.ls2();
        m_plbAxis->setTheStyle(s_3DAxisStyle);
    }
}

void W3dPrefsDlg::onTopTrans()
{
    LinePickerDlg LPdlg(this);
    LPdlg.setTheStyle(s_TopStyle);
    LPdlg.initDialog(false);

    if (QDialog::Accepted == LPdlg.exec())
    {
        s_TopStyle = LPdlg.ls2();
        m_plbTopTrans->setTheStyle(s_TopStyle);
    }
}

void W3dPrefsDlg::onBotTrans()
{
    LinePickerDlg LPdlg(this);
    LPdlg.setTheStyle(s_BotStyle);
    LPdlg.initDialog(false);

    if (QDialog::Accepted == LPdlg.exec())
    {
        s_BotStyle = LPdlg.ls2();
        m_plbBotTrans->setTheStyle(s_BotStyle);
    }
}

void W3dPrefsDlg::onIDrag()
{
    LinePickerDlg LPdlg(this);
    LPdlg.setTheStyle(s_IDragStyle);
    LPdlg.initDialog(false);

    if (QDialog::Accepted == LPdlg.exec())
    {
        s_IDragStyle = LPdlg.ls2();
        m_plbInducedDrag->setTheStyle(s_IDragStyle);
    }
}

void W3dPrefsDlg::onVDrag()
{
    LinePickerDlg LPdlg(this);
    LPdlg.setTheStyle(s_VDragStyle);
    LPdlg.initDialog(false);

    if (QDialog::Accepted == LPdlg.exec())
    {
        s_VDragStyle = LPdlg.ls2();
        m_plbViscousDrag->setTheStyle(s_VDragStyle);
    }
}

void W3dPrefsDlg::onXCP()
{
    LinePickerDlg LPdlg(this);
    LPdlg.setTheStyle(s_XCPStyle);
    LPdlg.initDialog(false);

    if (QDialog::Accepted == LPdlg.exec())
    {
        s_XCPStyle = LPdlg.ls2();
        m_plbLift->setTheStyle(s_XCPStyle);
    }
}

void W3dPrefsDlg::onMoments()
{
    LinePickerDlg LPdlg(this);
    LPdlg.setTheStyle(s_MomentStyle);
    LPdlg.initDialog(false);

    if (QDialog::Accepted == LPdlg.exec())
    {
        s_MomentStyle = LPdlg.ls2();
        m_plbMoments->setTheStyle(s_MomentStyle);
    }
}

void W3dPrefsDlg::onDownwash()
{
    LinePickerDlg LPdlg(this);
    LPdlg.setTheStyle(s_DownwashStyle);
    LPdlg.initDialog(false);

    if (QDialog::Accepted == LPdlg.exec())
    {
        s_DownwashStyle = LPdlg.ls2();
        m_plbDownwash->setTheStyle(s_DownwashStyle);
    }
}

void W3dPrefsDlg::onStreamLines()
{
    LinePickerDlg LPdlg(this);
    LPdlg.setTheStyle(s_StreamStyle);
    LPdlg.initDialog(false);

    if (QDialog::Accepted == LPdlg.exec())
    {
        s_StreamStyle = LPdlg.ls2();
        m_plbStreamLines->setTheStyle(s_StreamStyle);
    }
}

void W3dPrefsDlg::onWakePanels()
{
    LinePickerDlg LPdlg(this);
    LPdlg.setTheStyle(s_WakeStyle);
    LPdlg.initDialog(false);

    if (QDialog::Accepted == LPdlg.exec())
    {
        s_WakeStyle = LPdlg.ls2();
        m_plbWakePanels->setTheStyle(s_WakeStyle);
    }
}

void W3dPrefsDlg::onVLMMesh()
{
    LinePickerDlg LPdlg(this);
    LPdlg.setTheStyle(s_VLMStyle);
    LPdlg.initDialog(false);

    if (QDialog::Accepted == LPdlg.exec())
    {
        s_VLMStyle = LPdlg.ls2();
        m_plbVLMMesh->setTheStyle(s_VLMStyle);
    }
    repaint();
}


void W3dPrefsDlg::onMasses()
{
    QColorDialog::ColorDialogOptions dialogOptions = QColorDialog::ShowAlphaChannel;
#ifdef Q_OS_MAC
#if QT_VERSION >= 0x040700
    dialogOptions |= QColorDialog::DontUseNativeDialog;
#endif
#endif
    QColor Color = QColorDialog::getColor(s_MassColor,
                                   this, "Select the color", dialogOptions);
    if(Color.isValid()) s_MassColor = Color;
    m_pcbMassColor->setColor(s_MassColor);

    update();
}


void W3dPrefsDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("3dPrefs_2");
    {
        s_3DAxisStyle.loadSettings(  settings, "3DAxisStyle");
        s_VLMStyle.saveSettings(     settings, "VLMStyle");
        s_OutlineStyle .saveSettings(settings, "OutlineStyle");
        s_XCPStyle.saveSettings(     settings, "XCPStyle");
        s_MomentStyle.saveSettings(  settings, "MomentStyle");
        s_IDragStyle.saveSettings(   settings, "IDragStyle");
        s_VDragStyle.saveSettings(   settings, "VDragStyle");
        s_DownwashStyle.saveSettings(settings, "DownwashStyle");
        s_WakeStyle.saveSettings(    settings, "WakeStyle");
        s_CpStyle.saveSettings(      settings, "CpStyle");
        s_TopStyle.saveSettings(     settings, "TopStyle");
        s_BotStyle.saveSettings(     settings, "BotStyle");
        s_StreamStyle.saveSettings(  settings, "StreamLinesStyle");


        settings.setValue("showWakePanels", s_bWakePanels);

        settings.setValue("MassColor", s_MassColor);

        settings.setValue("AutoAdjustScale", s_bAutoAdjustScale);
        settings.setValue("AnimateTransitions", s_bAnimateTransitions);
        settings.setValue("EnableClipPlane", s_bEnableClipPlane);

        settings.setValue("ChordwiseResolution", s_iChordwiseRes);
        settings.setValue("BodyAxialRes", s_iBodyAxialRes);
        settings.setValue("BodyHoopRes", s_iBodyHoopRes);

    }
    settings.endGroup();
}


void W3dPrefsDlg::loadSettings(QSettings &settings)
{
    resetDefaults();
    settings.beginGroup("3dPrefs_2");
    {
        s_3DAxisStyle.loadSettings(  settings, "3DAxisStyle");
        s_VLMStyle.loadSettings(     settings, "VLMStyle");
        s_OutlineStyle .loadSettings(settings, "OutlineStyle");
        s_XCPStyle.loadSettings(     settings, "XCPStyle");
        s_MomentStyle.loadSettings(  settings, "MomentStyle");
        s_IDragStyle.loadSettings(   settings, "IDragStyle");
        s_VDragStyle.loadSettings(   settings, "VDragStyle");
        s_DownwashStyle.loadSettings(settings, "DownwashStyle");
        s_WakeStyle.loadSettings(    settings, "WakeStyle");
        s_CpStyle.loadSettings(      settings, "CpStyle");
        s_TopStyle.loadSettings(     settings, "TopStyle");
        s_BotStyle.loadSettings(     settings, "BotStyle");
        s_StreamStyle.loadSettings(  settings, "StreamLinesStyle");

        s_MassColor = settings.value("MassColor", QColor(67, 151, 169)).value<QColor>();
        s_bWakePanels = settings.value("showWakePanels", true).toBool();
        s_bEnableClipPlane = settings.value("EnableClipPlane", false).toBool();

        s_bAutoAdjustScale = settings.value("AutoAdjustScale", true).toBool();
        s_bAnimateTransitions = settings.value("AnimateTransitions", true).toBool();
        s_iChordwiseRes = settings.value("ChordwiseResolution", 50).toInt();
        s_iBodyAxialRes = settings.value("BodyAxialRes", 23).toInt();
        s_iBodyHoopRes = settings.value("BodyHoopRes", 17).toInt();
    }
    settings.endGroup();
}


void W3dPrefsDlg::onResetDefaults()
{
    resetDefaults();
    initDialog();
}


void W3dPrefsDlg::resetDefaults()
{
    s_bWakePanels = false;

    s_MassColor = QColor(67, 151, 169);



    s_bAnimateTransitions = false;
    s_iChordwiseRes=29;
    s_iBodyAxialRes=23;
    s_iBodyHoopRes = 17;

}


void W3dPrefsDlg::onOK()
{
    readSettings();
    accept();
}

void W3dPrefsDlg::readSettings()
{
    s_bAnimateTransitions = m_pchAnimateTransitions->isChecked();
    s_bAutoAdjustScale = m_pchAutoAdjustScale->isChecked();
    s_bEnableClipPlane = m_pchEnableClipPlane->isChecked();

    s_iChordwiseRes = m_pieChordwiseRes->value();
    s_iBodyAxialRes = m_pieBodyAxialRes->value();
    s_iBodyHoopRes  = m_pcieBodyHoopRes->value();

}





