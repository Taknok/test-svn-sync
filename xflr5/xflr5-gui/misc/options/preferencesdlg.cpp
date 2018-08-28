/****************************************************************************

    Preferences Class
    Copyright (C) 2018 Andre Deperrois

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


#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>

#include "preferencesdlg.h"
#include <globals/mainframe.h>
#include <misc/options/saveoptions.h>
#include <misc/options/language.h>
#include <misc/options/Units.h>
#include <misc/options/displayoptions.h>
#include <misc/updater.h>

PreferencesDlg::PreferencesDlg(QWidget *pParent) : QDialog(pParent)
{
	setupLayout();
}



void PreferencesDlg::setupLayout()
{
    QWidget *pUpdateFrame = new QWidget;
    {
        QVBoxLayout *pUpdateLayout = new QVBoxLayout;
        m_pctrlUpdateCheck = new QCheckBox("Check for updates on startup");
        m_pctrlUpdateCheck->setChecked(Updater::bAutoCheck());
        pUpdateLayout->addWidget(m_pctrlUpdateCheck);
        pUpdateLayout->addStretch();
        pUpdateFrame->setLayout(pUpdateLayout);
    }

	m_pDisplayOptionsWidget = new Settings(this);
	m_pSaveOptionsWidget = new SaveOptions(this);
	m_pLanguageOptionsWidget = new TranslatorDlg(this);
	m_pUnitsWidget = new Units(this);

	QHBoxLayout *pOptionsLayout = new QHBoxLayout;
	{
		m_pTabWidget = new QListWidget;
        m_pTabWidget->addItem(tr("Updates"));
        m_pTabWidget->addItem(tr("Save options"));
        m_pTabWidget->addItem(tr("Display options"));
		m_pTabWidget->addItem(tr("Language"));
		m_pTabWidget->addItem(tr("Units"));
		m_pPageStack = new QStackedWidget;
        m_pPageStack->addWidget(pUpdateFrame);
        m_pPageStack->addWidget(m_pSaveOptionsWidget);
        m_pPageStack->addWidget(m_pDisplayOptionsWidget);
		m_pPageStack->addWidget(m_pLanguageOptionsWidget);
		m_pPageStack->addWidget(m_pUnitsWidget);
		pOptionsLayout->addWidget(m_pTabWidget);
		pOptionsLayout->addWidget(m_pPageStack);
	}

	QHBoxLayout *pCommandButtonsLayout = new QHBoxLayout;
	{
		QPushButton *pOKButton = new QPushButton(tr("OK"));
		pOKButton->setAutoDefault(true);
		QPushButton *pCancelButton = new QPushButton(tr("Cancel"));
		connect(pOKButton, SIGNAL(clicked(bool)), this, SLOT(onOK()));
		connect(pCancelButton, SIGNAL(clicked(bool)), this, SLOT(reject()));
		pCancelButton->setAutoDefault(false);
		pCommandButtonsLayout->addStretch(1);
		pCommandButtonsLayout->addWidget(pOKButton);
		pCommandButtonsLayout->addWidget(pCancelButton);
	}
	QVBoxLayout *pMainLayout = new QVBoxLayout;
	{
		pMainLayout->addLayout(pOptionsLayout);
		pMainLayout->addLayout(pCommandButtonsLayout);
	}
	setLayout(pMainLayout);

	connect(m_pTabWidget, SIGNAL(currentRowChanged(int)), this, SLOT(onPage(int)));
}



void PreferencesDlg::onPage(int iRow)
{
	m_pPageStack->setCurrentIndex(iRow);
}


void PreferencesDlg::onOK()
{
    Updater::setAutoCheck(m_pctrlUpdateCheck->isChecked());
	m_pSaveOptionsWidget->onOK();
	m_pLanguageOptionsWidget->onOK();
	accept();
}
