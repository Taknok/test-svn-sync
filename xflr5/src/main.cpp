/****************************************************************************

	XFLR5 Application

	Copyright (C) 2008-2016 Andre Deperrois adeperrois@xflr5.com

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


#include "XFLR5Application.h"
#include "mainframe.h"
#include <QSplashScreen>
#include <QDebug>
#include <QSurfaceFormat>

/**
*The app's point of entry !
*/
int main(int argc, char *argv[])
{
#ifdef Q_OS_MACX
    /*
    if ( QSysInfo::MacintoshVersion > QSysInfo::MV_10_8 )
	{
		// fix Mac OS X 10.9 (mavericks) font issue
		// https://bugreports.qt-project.org/browse/QTBUG-32789
		QFont::insertSubstitution(".Lucida Grande UI", "Lucida Grande");
	}
    */
#endif

/* Note: Calling QSurfaceFormat::setDefaultFormat() before constructing the QApplication instance
 * is mandatory on some platforms (for example, OS X) when an OpenGL core profile context is requested.
 * This is to ensure that resource sharing between contexts stays functional as all internal contexts
 *  are created using the correct version and profile.
 * */

#ifdef QT_DEBUG
	QString strange;
	strange.sprintf("Default OpengGl format:%d.%d", QSurfaceFormat::defaultFormat().majorVersion(),QSurfaceFormat::defaultFormat().minorVersion());
	qDebug()<<strange;
#endif

/*	QSurfaceFormat defaultFormat = QSurfaceFormat::defaultFormat();
	defaultFormat.setVersion(3, 0);
	defaultFormat.setProfile(QSurfaceFormat::CompatibilityProfile);
	QSurfaceFormat::setDefaultFormat(defaultFormat);

#ifdef QT_DEBUG
	strange.sprintf("App default OpengGl format:%d.%d", QSurfaceFormat::defaultFormat().majorVersion(),QSurfaceFormat::defaultFormat().minorVersion());
	qDebug()<<strange;
	switch (QSurfaceFormat::defaultFormat().profile()) {
		case QSurfaceFormat::NoProfile:
				qDebug()<<"   No Profile";
			break;
		case QSurfaceFormat::CoreProfile:
				qDebug()<<"   Core Profile";
			break;
		case QSurfaceFormat::CompatibilityProfile:
				qDebug()<<"   Compatibility Profile";
			break;
		default:
			break;
	}
	qDebug()<<"/main";
	qDebug()<<"";
#endif
*/
	XFLR5Application app(argc, argv);
	return app.exec();
}


