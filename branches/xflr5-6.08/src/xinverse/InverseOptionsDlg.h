/****************************************************************************

	InverseOptionsDlg  Classes
	Copyright (C) 2009 Andre Deperrois adeperrois@xflr5.com

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

#ifndef INVERSEOPTIONSDLG_H
#define INVERSEOPTIONSDLG_H

#include "../misc/LineButton.h"
#include <QDialog>


class InverseOptionsDlg:public QDialog
{
	Q_OBJECT
	
	friend class QXInverse;

public:
    InverseOptionsDlg();

private slots:
	void OnRefStyle();
	void OnModStyle();
	void OnSplineStyle();
	void OnReflectedStyle();

private:
	void SetupLayout();
	void InitDialog();

	LineButton *m_pctrlRefFoil, *m_pctrlModFoil, *m_pctrlSpline, *m_pctrlReflected;

	void * m_pXInverse;
};

#endif // INVERSEOPTIONSDLG_H
