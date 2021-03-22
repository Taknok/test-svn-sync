/****************************************************************************

    GridSettingsDlg Class
    Copyright (C) 2009 Andre Deperrois 

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
#include <QPushButton>
#include <QCheckBox>

#include <xflcore/ls2.h>

class LineBtn;
class DoubleEdit;

class GridSettingsDlg : public QDialog
{
    Q_OBJECT

    friend class AFoil;
    friend class Section2dWidget;

    public:
        GridSettingsDlg(QWidget *pParent=nullptr);
        void initDialog();

    private slots:
        void onOK();
        void onScale();
        void onNeutralStyle();
        void onXMajStyle();
        void onXMinStyle();
        void onYMajStyle();
        void onYMinStyle();
        void onNeutralShow(bool bShow);
        void onXMajShow(bool bShow);
        void onYMajShow(bool bShow);
        void onXMinShow(bool bShow);
        void onYMinShow(bool bShow);

    private:
        void setupLayout();
        void keyPressEvent(QKeyEvent *event) override;

        QCheckBox  *m_pchNeutralShow, *m_pchScale, *m_pcHXMajShow, *m_pcHYMajShow, *m_pchXMinShow, *m_pchYMinShow;
        LineBtn *m_plbNeutralStyle, *m_plbXMajStyle, *m_plbYMajStyle, *m_plbXMinStyle, *m_plbYMinStyle;
        DoubleEdit *m_pdeXUnit, *m_pdeYUnit, *m_pdeXMinUnit, *m_pdeYMinUnit;
        QPushButton *m_ppbOKButton, *m_ppbCancelButton;

        bool m_bScale;

        LS2 m_XStyle, m_YStyle;
        LS2 m_XMinStyle, m_YMinStyle;

        double m_XUnit, m_YUnit;
        double m_XMinUnit, m_YMinUnit;


        LS2 m_NeutralStyle;
};

