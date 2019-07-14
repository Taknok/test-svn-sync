/****************************************************************************

    ObjectPropsDlg Class
    Copyright (C) 2010 Andre Deperrois 

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

#ifndef ObjectPropsDlg_H
#define ObjectPropsDlg_H
#include <QTextEdit>
#include <QDialog>

class ObjectPropsDlg : public QDialog
{
    Q_OBJECT

public:
    ObjectPropsDlg(QWidget *pParent);
    void initDialog(QString title, QString props);

    QSize sizeHint() const {return QSize(700,550);}

private:
    void setupLayout();

    QTextEdit *m_pctrlDescription;

};

#endif // ObjectPropsDlg_H
