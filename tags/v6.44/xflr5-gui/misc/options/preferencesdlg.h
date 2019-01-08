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

#pragma once

#include <QDialog>
#include <QTabWidget>
#include <QStackedWidget>
#include <QCheckBox>
#include <QRadioButton>
#include <QComboBox>
#include <QListWidget>
#include <QLabel>


class TranslatorDlg;
class SaveOptions;
class Units;
class Settings;
class IntEdit;
class TextClrBtn;
class ColorButton;

class PreferencesDlg : public QDialog
{
	friend class MainFrame;
	Q_OBJECT

public:
	PreferencesDlg(QWidget *pParent);

private:
	void setupLayout();

private slots:
	void onPage(int iRow);
	void onOK();

private:
	QListWidget *m_pTabWidget;
	QStackedWidget *m_pPageStack;

	SaveOptions *m_pSaveOptionsWidget;
	TranslatorDlg *m_pLanguageOptionsWidget;
	Units *m_pUnitsWidget;
	Settings *m_pDisplayOptionsWidget;

    QCheckBox *m_pctrlUpdateCheck;

};


