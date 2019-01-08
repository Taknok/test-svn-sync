/****************************************************************************

    OpenGLInfoDlg Class
    Copyright (C) 2015 Andre Deperrois

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

#ifndef OPENGLINFODLG_H
#define OPENGLINFODLG_H

#include <QWidget>
#include <QDialog>
#include <QComboBox>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QSurface>



class MinTextEdit;

class OpenGLInfoDlg : public QDialog
{

public:
	OpenGLInfoDlg(QWidget *pParent=NULL);

private slots:
	void start();
	void renderWindowReady();
	void renderWindowError(const QString &msg);

private:
	void addVersions(QLayout *layout);
	void addProfiles(QLayout *layout);
	void addOptions(QLayout *layout);
	void addRenderWindow();
	void printFormat(const QSurfaceFormat &format);
	void setupLayout();

	QComboBox *m_version;
	QLayout *m_profiles;
	QLayout *m_options;
    QTextEdit *m_glOutput;
    QVBoxLayout *m_pRenderWindowLayout;
	QWidget *m_renderWindowContainer;
	QSurface *m_surface;
};

#endif // OPENGLINFODLG_H
