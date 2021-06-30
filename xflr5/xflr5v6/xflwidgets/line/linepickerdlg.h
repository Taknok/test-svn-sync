/****************************************************************************

    LinePicker Class
    Copyright (C) Andre Deperrois

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
#include <QCheckBox>
#include <QDialogButtonBox>

#include <xflcore/linestyle.h>


class LineBtn;
class LineCbBox;
class LineDelegate;
class LinePickerWt;

class LinePickerDlg : public QDialog
{
    Q_OBJECT

    public:
        LinePickerDlg(QWidget *pParent);

        void initDialog(bool bFlowDownEnable);
        void initDialog(LineStyle const &ls, bool bAcceptPointStyle, bool bFlowDownEnable);
        void initDialog(Line::enumPointStyle pointStyle, Line::enumLineStipple linestipple, int lineWidth, QColor const&lineColor, bool bAcceptPointStyle, bool bFlowDownEnable);
        void keyPressEvent(QKeyEvent *event) override;

        LineStyle const &theStyle()      const {return m_LineStyle;}

        void setTheStyle(LineStyle const &ls) {m_LineStyle = ls;}

        void setTheStyle(Line::enumLineStipple stipple, int w, const QColor &clr, Line::enumPointStyle pointstyle)
        {
            m_LineStyle.m_Stipple = stipple;
            m_LineStyle.m_Width = w;
            m_LineStyle.m_Color = clr;
            m_LineStyle.m_Symbol = pointstyle;
        }


        void fillBoxes();
        void setupLayout();

        bool bFlowDownStyle() const {return m_pchFlowDownStyle->isChecked();}

    private slots:
        void onLineStyle(LineStyle ls);
        void onButton(QAbstractButton *pButton);

        void accept() override;
        void reject() override;

    private:

        LineStyle m_LineStyle;

        LinePickerWt *m_pLinePicker;
        QCheckBox *m_pchFlowDownStyle;

        QDialogButtonBox *m_pButtonBox;


        bool m_bAcceptPointStyle;

};

