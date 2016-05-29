/****************************************************************************

	XFoilTaskEvent Class
	   Copyright (C) 2016-2016 Andre Deperrois adeperrois@xflr5.com

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

#ifndef XFOILTASKEVENT_H
#define XFOILTASKEVENT_H
#include <QEvent>
#include <QString>

// Custom event identifier
const QEvent::Type XFOIL_TASK_EVENT = static_cast<QEvent::Type>(QEvent::User + 1);


class XFoilTaskEvent : public QEvent
{

public:
	XFoilTaskEvent(void * pFoil, void *pPolar): QEvent(XFOIL_TASK_EVENT),
		m_pFoil(pFoil),
		m_pPolar(pPolar)
	{
	}

	void * foilPtr() const	{return m_pFoil;}
	void * polarPtr() const	{return m_pPolar;}

private:
	void *m_pFoil;
	void *m_pPolar;
};

#endif // XFOILTASKEVENT_H
