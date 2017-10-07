/****************************************************************************

	Techwing Application

	Copyright (C) Andre Deperrois techwinder@gmail.com

	All rights reserved.

*****************************************************************************/


#pragma once

#include <QWidget>
#include <QPushButton>
#include <QListWidget>
#include <QStringList>


class LanguageOptions : public QWidget
{
	Q_OBJECT

	friend class MainFrame;
public:
    LanguageOptions(QWidget *pParent);

private slots:
	void onOK();

private:
	void setupLayout();
	void initWidget();
	QStringList findQmFiles();
	QString languageName(const QString &qmFile);

	QListWidget *m_pctrlLanguageList;
	QMap<QString, QString> qmFileForLanguage;
};

