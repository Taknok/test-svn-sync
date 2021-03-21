/****************************************************************************

    W3dPrefsDlg Class
    Copyright (C) 2009-2016 Andre Deperrois XFLR5@yahoo.com

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


#ifndef W3DPREFSDLG_H
#define W3DPREFSDLG_H

#include <QDialog>
#include <QCheckBox>
#include <QSettings>

#include <xflcore/ls2.h>

class LineBtn;
class ColorButton;
class IntEdit;

class W3dPrefsDlg : public QDialog
{
    Q_OBJECT

    friend class Miarex;
    friend class MainFrame;
    friend class GL3dBodyDlg;

public:
    W3dPrefsDlg(QWidget *pParent);
    void initDialog();

    static int chordwiseRes() {return s_iChordwiseRes;}
    static int bodyAxialRes() {return s_iBodyAxialRes;}
    static int bodyHoopRes() {return s_iBodyHoopRes;}

private slots:
    void on3DAxis();
    void onOutline();
    void onTopTrans();
    void onBotTrans();
    void onXCP();
    void onMoments();
    void onIDrag();
    void onVDrag();
    void onDownwash();
    void onWakePanels();
    void onStreamLines();
    void onVLMMesh();
    void onMasses();
    void onResetDefaults();
    void onOK();

private:
    void setupLayout();
    void readSettings();
    static void saveSettings(QSettings &settings);
    static void loadSettings(QSettings &settings);
    static void resetDefaults();

    LineBtn *m_plbAxis, *m_plbOutline, *m_plbVLMMesh, *m_plbTopTrans, *m_plbBotTrans;
    LineBtn *m_plbLift, *m_plbMoments, *m_plbInducedDrag, *m_plbViscousDrag, *m_plbDownwash;
    LineBtn *m_plbStreamLines, *m_plbWakePanels;

    ColorButton *m_pcbMassColor;

    QCheckBox *m_pchAnimateTransitions, *m_pchAutoAdjustScale;
    QCheckBox *m_pchEnableClipPlane;
    IntEdit *m_pieChordwiseRes, *m_pieBodyAxialRes, *m_pcieBodyHoopRes;

public:
    static double s_MassRadius;
    static QColor s_MassColor;

    static LS2 s_3DAxisStyle;
    static LS2 s_VLMStyle;
    static LS2 s_OutlineStyle;
    static LS2 s_XCPStyle;
    static LS2 s_MomentStyle;
    static LS2 s_IDragStyle;
    static LS2 s_VDragStyle;
    static LS2 s_TopStyle;
    static LS2 s_BotStyle;
    static LS2 s_DownwashStyle;
    static LS2 s_StreamStyle;
    static LS2 s_WakeStyle;
    static LS2 s_CpStyle;


    static bool s_bWakePanels;

    static bool s_bAutoAdjustScale;

    static int s_iChordwiseRes,s_iBodyAxialRes, s_iBodyHoopRes;
    static bool s_bAnimateTransitions;
    static bool s_bEnableClipPlane;
};

#endif // W3DPREFSDLG_H

