/****************************************************************************

    LinePicker Class
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
#include <QCheckBox>

#include <xflcore/ls2.h>


class LineBtn;
class LineCbBox;
class LineDelegate;

class LinePickerDlg : public QDialog
{
    Q_OBJECT

    public:
        LinePickerDlg(QWidget *pParent);

        void initDialog(bool bFlowDownEnable);
        void initDialog(LS2 const &ls, bool bAcceptPointStyle, bool bFlowDownEnable);
        void initDialog(int pointStyle, int lineStyle, int lineWidth, QColor lineColor, bool bAcceptPointStyle, bool bFlowDownEnable);

        void keyPressEvent(QKeyEvent *event) override;

        LS2 const &ls2()      const {return m_LineStyle;}
        LS2 const &theStyle() const {return m_LineStyle;}

        void setTheStyle(LS2 const &ls) {m_LineStyle = ls;}

        void setTheStyle(Line::enumLineStipple stipple, int w, const QColor &clr, Line::enumPointStyle pointstyle)
        {
            m_LineStyle.setStipple(stipple);
            m_LineStyle.m_Width = w;
            m_LineStyle.m_Color = clr;
            m_LineStyle.setPointStyle(pointstyle);
        }

        Line::enumLineStipple lineStipple2()  const {return m_LineStyle.m_Stipple;}
        Line::enumPointStyle  pointStyle2()   const {return m_LineStyle.m_PointStyle;}

        int pointStyle()   const {return m_LineStyle.m_PointStyle;}
        int lineStipple()  const {return m_LineStyle.m_Stipple;}
        int lineWidth()    const {return m_LineStyle.m_Width;}
        QColor lineColor() const {return m_LineStyle.m_Color;}

        void setPointStyle(int pointStyle);
        void setLineStipple(int lineStyle);
        void setLineWidth(int width);
        void setLineColor(QColor color);

        void fillBoxes();
        void setupLayout();

        bool bFlowDownStyle() const {return m_pchFlowDownStyle->isChecked();}

    private slots:
        void onPointStyle(int val);
        void onLineStyle(int val);
        void onLineWidth(int val);
        void onLineColor();

        void accept() override;
        void reject() override;

    private:
        LineBtn *m_plbLineColor;
        LineCbBox *m_plcbPointStyle, *m_plcbLineWidth, *m_plcbLineStyle;
        QCheckBox *m_pchFlowDownStyle;
        QPushButton *m_ppbOKButton, *m_ppbCancelButton;

        bool m_bAcceptPointStyle;

        LS2 m_LineStyle;
        LineDelegate *m_pPointStyleDelegate, *m_pLineStyleDelegate, *m_pWidthDelegate;
};

