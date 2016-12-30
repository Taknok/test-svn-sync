/****************************************************************************

	Globals Class
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

//Global functions

/**@file This file contains the definitions of methods used throughout the program and not specific to one application. */

#include <xfoil_globals.h>
#include <QtDebug>
#include <QFile>
#include <QTextStream>
#include <QByteArray>
#include <math.h>
#include <QtCore>




/**
* Reads the RGB int values of a color from binary datastream and returns a QColor. Inherited from the MFC versions of XFLR5.
*@param ar the binary datastream
*@param r the red component
*@param g the green component
*@param b the blue component
*/
void readCOLORREF(QDataStream &ar, int &r, int &g, int &b)
{
	qint32 colorref;

	ar >> colorref;
	b = (int)(colorref/256/256);
	colorref -= b*256*256;
	g = (int)(colorref/256);
	r = colorref - g*256;
}



/**
* Writes the RGB int values of a color to a binary datastream. Inherited from the MFC versions of XFLR5.
*@param ar the binary datastream
*@param r the red component
*@param g the green component
*@param b the blue component

*/
void WriteCOLORREF(QDataStream &ar, int r, int g, int b)
{
	qint32 colorref;

	colorref = b*256*256+g*256+r;
	ar << colorref;
}



/**
* Reads the RGB int values of a color from binary datastream and returns a QColor. Inherited from the MFC versions of XFLR5.
*@param ar the binary datastream
*@param r the red component
*@param g the green component
*@param b the blue component
*@param a the alpha component
*/
void readqColor(QDataStream &ar, int &r, int &g, int &b, int &a)
{
	uchar byte=0;

	ar>>byte;//probably a format identificator
	ar>>byte>>byte;
	a = (int)byte;
	ar>>byte>>byte;
	r = (int)byte;
	ar>>byte>>byte;
	g = (int)byte;
	ar>>byte>>byte;
	b = (int)byte;
	ar>>byte>>byte; //
}

/**
* Writes the RGB int values of a color to a binary datastream. Inherited from the MFC versions of XFLR5.
*@param ar the binary datastream
*@param r the red component
*@param g the green component
*@param b the blue component
*@param a the alpha component
*/
void writeqColor(QDataStream &ar, int r, int g, int b, int a)
{
	uchar byte;

	byte = 1;
	ar<<byte;
	byte = a & 0xFF;
	ar << byte<<byte;
	byte = r & 0xFF;
	ar << byte<<byte;
	byte = g & 0xFF;
	ar << byte<<byte;
	byte = b & 0xFF;
	ar << byte<<byte;
	byte = 0;
	ar << byte<<byte;
}




/**
* Reads a sequence of characters from a binary stream and returns a QString. Inherited from the MFC versions of XFLR5.
*@param ar the binary datastream
*@param strong the QString read from the stream
*/
void readCString(QDataStream &ar, QString &strong)
{
	qint8 qi, ch;
	char c;

	ar >> qi;
	strong.clear();
	for(int j=0; j<qi;j++)
	{
		strong += " ";
		ar >> ch;
		c = char(ch);
		strong[j] = c;
	}
}

/**
* Writes a sequence of characters from a QStrinf to a binary stream. Inherited from the MFC versions of XFLR5.
*@param ar the binary datastream
*@param strong the QString to output to the stream
*/
void writeCString(QDataStream &ar, QString const &strong)
{
	qint8 qi = strong.length();

	QByteArray textline;
	char *text;
	textline = strong.toLatin1();
	text = textline.data();
	ar << qi;
	ar.writeRawData(text, qi);
}
