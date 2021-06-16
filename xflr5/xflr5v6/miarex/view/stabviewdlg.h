/****************************************************************************

    StabViewDlg Class
    Copyright (C) 200-2016 Andre Deperrois 

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


#ifndef STABVIEWDLG_H
#define STABVIEWDLG_H

#include <QWidget>
#include <QLabel>
#include <QSlider>
#include <QDial>
#include <QRadioButton>
#include <QPushButton>
#include <QComboBox>
#include <QStackedWidget>
#include <QGroupBox>
#include <QTableView>
#include <QStandardItemModel>

class Miarex;
class DoubleEdit;
class FloatEditDelegate;
class Curve;

class StabViewDlg : public QWidget
{
    Q_OBJECT

    friend class Miarex;
    friend class MainFrame;

public:
    StabViewDlg(QWidget *parent);
    ~StabViewDlg();


private slots:
    void onAnimate();
    void onAnimateRestart();
    void onAnimationSpeed(int val);
    void onAnimationAmplitude(int val);
    void onCellChanged(QWidget *);
    void onModeSelection();
    void onPlotStabilityGraph();
    void onReadData();
    void onResponseType();
    void onAddCurve();
    void onDeleteCurve();
    void onRenameCurve();
    void onSelChangeCurve(int sel);
    
private:
    void keyPressEvent(QKeyEvent *event);
    
    void addCurve();
    void connectSignals();
//    void FillControlNames();
    void fillEigenThings();
    void fillCurveList();
    double getControlInput(const double &time);
    void setMode(int iMode=-1);
    void setupLayout();
    void setControls();
    void setTimeCurveStyle(QColor const &Color, int const&Style, int const &Width, bool const& bCurve, const int &PointStyle);
    void readControlModelData();
    void updateControlModelData();

    static Miarex *s_pMiarex;

    QRadioButton *m_pctrlLongDynamics,*m_pctrlLatDynamics;
    QRadioButton *m_pctrlRLMode1,*m_pctrlRLMode2,*m_pctrlRLMode3,*m_pctrlRLMode4;
    QRadioButton *m_pctrlTimeMode1,*m_pctrlTimeMode2,*m_pctrlTimeMode3,*m_pctrlTimeMode4;
    QLineEdit *m_pctrlEigenValue, *m_pctrlEigenVector1, *m_pctrlEigenVector2, *m_pctrlEigenVector3, *m_pctrlEigenVector4;

    QLabel *m_pctrlModeProperties;
//    QSlider *m_pctrlAnimationSpeed, *m_pctrlAnimationAmplitude;
    QDial *m_pctrlAnimationSpeed, *m_pctrlAnimationAmplitude;
    QPushButton *m_pctrlAnimate, *m_pctrlAnimateRestart;
    DoubleEdit *m_pctrlModeStep;
    
    QLabel *m_plabStab1, *m_plabStab2, *m_plabStab3;
    DoubleEdit  *m_pdeStabVar1, *m_pdeStabVar2, *m_pdeStabVar3;
    DoubleEdit *m_pdeTotalTime, *m_pdeDeltat;
    QPushButton *m_ppbPlotStabGraph;
    QPushButton *m_ppbAddCurve, *m_ppbDeleteCurve, *m_ppbRenameCurve;
    QComboBox *m_pcbCurveList;

    QLabel *m_plabUnit1, *m_plabUnit2, *m_plabUnit3;
    DoubleEdit *m_pctrlFreqN, *m_pctrlFreq1, *m_pctrlZeta, *m_pctrlT2, *m_pctrlTau;
    QStackedWidget *m_pctrlStackWidget, *m_pctrlInitialConditionsWidget, *m_pctrlModeViewType;


    QRadioButton *m_pctrlModalResponse, *m_pctrlInitCondResponse, *m_pctrlForcedResponse;

    QTableView *m_pctrlControlTable;
    QStandardItemModel *m_pControlModel;
    FloatEditDelegate *m_pCtrlDelegate;

    int m_ModeInterval;
    double m_ModeAmplitude;
    double m_vabs[4], m_phi[4];

public:
    int m_iCurrentMode;    
    Curve *m_pCurve;
    double m_Time[20], m_Amplitude[20];
};

#endif // STABVIEWDLG_H