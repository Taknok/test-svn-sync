/****************************************************************************

	Techwing Application

	Copyright (C) Andre Deperrois techwinder@gmail.com

	All rights reserved.

*****************************************************************************/

#include "language.h"
#include <mainframe.h>

#include <QHBoxLayout>
#include <QLabel>
#include <QStringList>
#include <QListWidgetItem>
#include <QMutableStringListIterator>
#include <QDir>
#include <QMessageBox>
#include <QTranslator>
#include <QtDebug>


LanguageOptions::LanguageOptions(QWidget *pParent): QWidget(pParent)
{
	setWindowTitle(tr("Language settings"));
	QString LanguageName = tr("English");// will be translated in the ts & qm files and this will be used to fill the QListWidget

	setupLayout();
}


void LanguageOptions::onOK()
{
	//read user language selection and exit
	QListWidgetItem *pItem =  m_pctrlLanguageList->currentItem();
	if(pItem)
	{
		if(pItem->text()=="English") MainFrame::s_LanguageFilePath = "";
		else                         MainFrame::s_LanguageFilePath = qmFileForLanguage[pItem->text()];
	}
	else
	{
		MainFrame::s_LanguageFilePath = "";
	}
	QMessageBox::warning(this, tr("Warning"), tr("The change will take effect at the next session"));
}


void LanguageOptions::setupLayout()
{
	QLabel *lab = new QLabel(tr("Select the application's default language:"));
	m_pctrlLanguageList = new QListWidget;
	m_pctrlLanguageList->setMinimumHeight(300);
	connect(m_pctrlLanguageList, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(onOK()));

	QVBoxLayout *pMainLayout = new QVBoxLayout;
	{
		pMainLayout->addWidget(lab);
		pMainLayout->addStretch(1);
		pMainLayout->addWidget(m_pctrlLanguageList);
		pMainLayout->addStretch(1);
	}

	setLayout(pMainLayout);
}


void LanguageOptions::initWidget()
{
	QStringList qmFiles = findQmFiles();
	qmFiles.sort();
	qmFileForLanguage.insert("English", "English");
	m_pctrlLanguageList->clear();
	m_pctrlLanguageList->addItem("English (default)");

	for (int i=0; i<qmFiles.count(); ++i)
	{
		qDebug()<<languageName(qmFiles[i]);
        qmFileForLanguage.insert(languageName(qmFiles[i]), qmFiles[i]);
        m_pctrlLanguageList->addItem(languageName(qmFiles[i]));
	}

	m_pctrlLanguageList->setCurrentRow(0);
	for (int i=0; i<qmFiles.count(); ++i)
	{
		if(qmFiles[i]==MainFrame::s_LanguageFilePath)
		{
			m_pctrlLanguageList->setCurrentRow(i+1);
			break;
		}
	}
}


QStringList LanguageOptions::findQmFiles()
{
	if(!MainFrame::s_TranslationDir.exists())
	{
		QMessageBox::warning(this, tr("Warning"), tr("The directory ")+MainFrame::s_TranslationDir.path()+tr(" does not exist"));
	}

	QStringList fileNames = MainFrame::s_TranslationDir.entryList(QStringList("*.qm"), QDir::Files, QDir::Name);
	QMutableStringListIterator i(fileNames);
	while (i.hasNext())
	{
		i.next();
		i.setValue(MainFrame::s_TranslationDir.filePath(i.value()));
	}

	return fileNames;
}


QString LanguageOptions::languageName(const QString &qmFile)
{
	QTranslator translator;
	translator.load(qmFile);

	return translator.translate("LanguageOptions", "English");
}




