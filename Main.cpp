/****************************************************************************

	QFLR5 Application

	Copyright (C) 2008 Andre Deperrois XFLR5@yahoo.com

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


#include <QtGui/QApplication>
#include "MainFrame.h"

int main(int argc, char *argv[])
{
	QString StyleName;

	QString FileName   = QDir::tempPath() + "/QFLR5.set";
	QFile *pXFile = new QFile(FileName);
	int a,b,c,d,k;
	a=b=50;
	c=800;
	d=500;
	bool bMaximized = true;
 
	if (pXFile->open(QIODevice::ReadOnly))
	{
		QDataStream ar(pXFile);
		ar >> k;//format

		ar >> a >> b >> c >> d;
		ar >> bMaximized;
		ar >> StyleName;
		pXFile->close();
	}
	QPoint pt(a,b);
	QSize sz(c,d);


	QCleanlooksStyle style;
	QApplication app(argc, argv);
	if(StyleName.length()) qApp->setStyle(StyleName);
	else                   qApp->setStyle(&style);

	MainFrame w;
	w.setWindowTitle("QFLR5");

	w.resize(sz);
	w.move(pt);

	if(w.m_bMaximized)	w.showMaximized();
	else                w.show();

	QString PathName, Extension;
	PathName=argv[1];

	Extension = PathName.right(4);
	if(Extension.compare(".wpa",Qt::CaseInsensitive)==0 ||
	   Extension.compare(".plr",Qt::CaseInsensitive)==0)
	{
		int app = w.LoadXFLR5File(PathName);

		if (app == MIAREX)             w.OnMiarex();
		else if (app == XFOILANALYSIS) w.OnXDirect();
	}

	return app.exec();
}

