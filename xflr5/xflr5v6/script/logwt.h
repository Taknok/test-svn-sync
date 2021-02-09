/****************************************************************************

    LogWt Class

    Copyright (C) 2019 Andre Deperrois

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

#include <QApplication>
#include <QTimer>
#include <QPushButton>
#include <QSettings>
#include <QPlainTextEdit>


class LogWt : public QWidget
{
    Q_OBJECT
//    friend class MainFrame;

    public:
        LogWt(QWidget *pParent=nullptr);

        void keyPressEvent(QKeyEvent *event);
        void showEvent(QShowEvent *event);
        void hideEvent(QHideEvent *event);
        QPushButton *ctrlButton() {return m_pctrlButton;}

        QSize sizeHint() const {return QSize(900,550);}

        void setCancelButton(bool bCancel);
        void setFinished(bool bFinished);

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    public slots:
        void onUpdate(QString msg);

    private slots:
        void onCancelClose();
        void onTimer() {qApp->processEvents();}

    private:
        void setupLayout();

        bool m_bFinished;

        QPlainTextEdit *m_pctrlLogView;
        QPushButton *m_pctrlButton;

        QTimer m_Timer;

        static QByteArray s_Geometry;
};


