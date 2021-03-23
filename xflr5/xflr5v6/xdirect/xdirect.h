/****************************************************************************

    XDirect Class
    Copyright (C) 2008-2019 Andre Deperrois

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

/** @file
 *
 * This file implements the QXDirect class which defines the GUI for foil analysis.
 *
*/

#pragma once

#include <QSettings>
#include <QVector>
#include <QStackedWidget>
#include <QLabel>
#include <QRadioButton>
#include <QSlider>

#include <xflobjects/objects2d/polar.h>
#include <xflobjects/objects2d/foil.h>
#include <xdirect/analysis/xfoilanalysisdlg.h>
#include <viewwidgets/oppointwt.h>
#include <xflcore/gui_enums.h>
#include <xflobjects/objects2d/oppoint.h>
#include <graph/graph.h>
#include <xflcore/ls2.h>

#include <xfoil.h>

class LineBtn;
class LineCbBox;
class LineDelegate;
class MainFrame; // to shut the compiler up
class DoubleEdit;
class MinTextEdit;

/**
* @class QXDirect
* @brief This class is the general interface for Foil direct analysis.
* This is the handling class for the QXDirect right toolbar.
* It provides the methods to modify the foil geometry, define the Polar analysis, perform the analysis, and post-process the results.
* One of the very first class of this project.
*/
class XDirect : public QWidget
{
    friend class MainFrame;
    friend class TwoDWidget;
    friend class XFoilAnalysisDlg;
    friend class BatchDlg;
    friend class BatchThreadDlg;
    friend class FoilPolarDlg;
    friend class EditPlrDlg;
    friend class XDirectStyleDlg;
    friend class Settings;

    friend class XDirectTileWidget;


    Q_OBJECT

    public:
        XDirect(QWidget *parent = nullptr);
        ~XDirect();

        void setView(Xfl::enumGraphView eView);

        bool bPolarView() {return m_bPolarView;}
        Graph *CpGraph(){return &m_CpGraph;}
        Graph *PlrGraph(int iPlrGraph){return m_PlrGraph.at(iPlrGraph);}
        int PlrGraphSize(){return m_PlrGraph.count();}

        static void setCurFoil(Foil*pFoil)    {m_pCurFoil = pFoil;}
        static void setCurPolar(Polar*pPolar) {m_pCurPolar = pPolar;}
        static void setCurOpp(OpPoint* pOpp)  {m_pCurOpp = pOpp;}

        static Foil *   curFoil()  {return m_pCurFoil;}
        static Polar*   curPolar() {return m_pCurPolar;}
        static OpPoint* curOpp()   {return m_pCurOpp;}

        static int timeUpdateInterval() {return s_TimeUpdateInterval;}
        static void setTimeUpdateInterval(int t) {s_TimeUpdateInterval=t;}

        static bool bKeepOpenOnErrors() {return s_bKeepOpenErrors;}
        static void setKeepOpenOnErrors(bool b) {s_bKeepOpenErrors=b;}

    signals:
        void projectModified();

    public slots:
        void updateView();

    public slots:
        void onAnalyze();
        void onAnimate(bool bChecked);
        void onAnimateSingle();
        void onAnimateSpeed(int val);
        void onBatchAnalysis();
        void onCadd();
        void onCpGraph();
        void onCpi();
        void onCurOppOnly();
        void onCurveColor();
        void onCurvePoints(int index);
        void onCurveStyle(int index);
        void onCurveWidth(int index);
        void onDefinePolar();
        void onDelCurOpp();
        void onDeleteCurFoil();
        void onDeleteCurPolar();
        void onDeleteFoilOpps();
        void onDeleteFoilPolars();
        void onDeletePolarOpps();
        void onDerotateFoil();
        void onDuplicateFoil();
        void onEditCurPolar();
        void onExportAllFoilPolars();
        void onExportAllPolarsTxt();
        void onExportAllPolarsTxt(QString DirName, Xfl::enumTextFileType);
        void onExportBLData();
        void onExportCurFoil();
        void onExportCurOpp();
        void onExportCurPolar();
        void onExportPolarOpps() ;
        void onExportXMLAnalysis();
        void onFoilCoordinates();
        void onFoilGeom();
        void onHideAllOpps();
        void onHideAllPolars();
        void onHideFoilOpps();
        void onHideFoilPolars();
        void onHidePolarOpps();
        void onImportJavaFoilPolar();
        void onImportXFoilPolars();
        void onImportXMLAnalysis();
        void onInputChanged();
        void onInterpolateFoils();
        void onMultiThreadedBatchAnalysis();
        void onNacaFoils();
        void onNormalizeFoil();
        void onOpPointProps();
        void onOpPointView();
        void onOptim2d();
        void onPolarFilter();
        void onPolarProps();
        void onPolarView();
        void onQGraph();
        void onRefinePanelsGlobally();
        void onRenameCurFoil();
        void onRenameCurPolar();
        void onResetAllPolarGraphsScales();
        void onResetCurPolar();
        void onSaveFoilPolars();
        void onSequence();
        void onSetFlap();
        void onSetLERadius();
        void onSetTEGap();
        void onShowAllOpps();
        void onShowAllPolars();
        void onShowCurve();
        void onShowFoilOpps();
        void onShowFoilPolars();
        void onShowFoilPolarsOnly();
        void onShowPolarOpps();
        void onSpec();
        void onStoreOpp();
        void onViscous();
        void onXFoilAdvanced();


    private:
        void keyPressEvent(QKeyEvent *event);
        void keyReleaseEvent(QKeyEvent *event);

        void setControls();
        void connectSignals();
        void createOppCurves(OpPoint *pOpp=nullptr);
        void createPolarCurves();
        void fillComboBoxes(bool bEnable = true);
        void fillPolarCurve(Curve *pCurve, Polar *pPolar, int XVar, int YVar);
        void fillOppCurve(OpPoint *pOpp, Graph *pGraph, Curve *pCurve, bool bInviscid=false);

        void importAnalysisFromXML(QFile &xmlFile);
        Polar *importXFoilPolar(QFile &txtFile);

        void loadSettings(QSettings &settings);
        void readParams();
        Foil *addNewFoil(Foil *pFoil);
        void renameFoil(Foil *pFoil);
        void saveSettings(QSettings &settings);
        void setCurveParams();
        void setFoilScale();
        void setGraphTiles();
        void setOpPointSequence();
        void setAnalysisParams();
        void setGraphTitles(Graph* pGraph, int iX, int iY);
        void setGraphTitles(Graph* pGraph);
        void setupLayout();
        void stopAnimate();
        void updateCurveStyle();

        QVector<double> * getVariable(Polar *pPolar, int iVar);

        Foil *setFoil(Foil* pFoil=nullptr);
        Polar *setPolar(Polar *pPolar=nullptr);
        OpPoint *setOpp(double Alpha=-123456789.0);





    private:
        XFoilAnalysisDlg* m_pXFADlg;
        OpPointWidget *m_pOpPointWidget;

        QStackedWidget *m_pswMiddleControls;

        QTimer *m_pAnimateTimer;

        QLabel *m_plabUnit1, *m_plabUnit2, *m_plabUnit3;

        QRadioButton *m_prbSpec1, *m_prbSpec2, *m_prbSpec3;

        QCheckBox *m_pchSequence;
        DoubleEdit *m_pdeAlphaMin, *m_pdeAlphaMax, *m_pdeAlphaDelta;

        QCheckBox *m_pchViscous;
        QCheckBox *m_pchInitBL;
        QCheckBox *m_pchStoreOpp;
        QPushButton *m_ppbAnalyze;

        QCheckBox *m_pchShowBL, *m_pchShowPressure;
        QCheckBox* m_pchAnimate;
        QSlider* m_pslAnimateSpeed;

        MinTextEdit *m_pctrlPolarProps;

        QCheckBox *m_pchShowCurve, *m_pchAlignChildren;

        LineCbBox *m_plcbCurveStyle, *m_plcbCurveWidth, *m_plcbPointStyle;
        LineBtn *m_plbCurveColor;

        LineDelegate *m_pStyleDelegate, *m_pWidthDelegate, *m_pPointDelegate;

        static MainFrame *s_pMainFrame;  /**< a static pointer to the instance of the application's MainFrame object */


        bool m_bPolarView;         /**< true if the polar view is selected, false if the operating point view is selected */
        bool m_bShowUserGraph;     /**< true if the 5th polar graph should be displayed */
        bool m_bAnimate;           /**< true if a result animation is underway */
        bool m_bAnimatePlus;       /**< true if the animation is going from lower to higher alpha, false if decreasing */
    //    bool m_bShowPanels;        /**< true if the panels should be displayed on the foil surface */
        bool m_bType1;             /**< true if the type 1 polars are to be displayed in the graphs */
        bool m_bType2;             /**< true if the type 2 polars are to be displayed in the graphs */
        bool m_bType3;             /**< true if the type 3 polars are to be displayed in the graphs */
        bool m_bType4;             /**< true if the type 4 polars are to be displayed in the graphs */
        bool m_bTrans;             /**< true if the user is dragging a view */
        bool m_bFromList;          /**< true if the batch analysis is based on a list of Re values */
        bool m_bShowTextOutput;    /**< true if the batch analysis should display text result output */
        bool m_bNeutralLine;       /**< true if the neutral line should be displayed */
        bool m_bCurOppOnly;        /**< true if only the current operating point should be displayed */
        bool m_bShowInviscid;      /**< true if the inviscid results should be displayed */
        bool m_bCpGraph;           /**< true if the Cp graph should be displayed */
        bool m_bSequence;          /**< true if a sequential analysis is to be performed */
        bool m_bXPressed;          /**< true if the 'X' key is pressed */
        bool m_bYPressed;          /**< true if the 'Y' key is pressed */
        bool m_bResetCurves;       /**< true if the graph curves need to be redrawn before the next view update */

        int m_posAnimate;          /**< the current aoa in the animation */



        int m_iPlrGraph;           /**< defines whch polar graph is selected if m_iPlrView=1 */
        Xfl::enumGraphView m_iPlrView;  /**< defines the number of graphs to be displayed in the polar view */
        int m_FoilYPos;            /**< y position for the foil display, in pixels from the bottom of the screen */

        double m_fFoilScale;        /**< the scale for foil display*/


        double m_Alpha;             /**< the min value of the aoa for a sequential analysis of Type 1, 2, or 3*/
        double m_AlphaMax;          /**< the max value of the aoa for a sequential analysis of Type 1, 2, or 3*/
        double m_AlphaDelta;        /**< the increment value of the aoa for a sequential analysis of Type 1, 2, or 3*/
        double m_Cl;                /**< the min value of the lift coefficient for a sequential analysis of Type 1, 2, or 3*/
        double m_ClMax;             /**< the max value of the aoa for a sequential analysis of Type 1, 2, or 3*/
        double m_ClDelta;           /**< the increment value of the aoa for a sequential analysis of Type 1, 2, or 3*/
        double m_Reynolds;          /**< the min value of the Reynolds number for a sequential analysis of Type 4*/
        double m_ReynoldsMax;       /**< the max value of the Reynolds number for a sequential analysis of Type 4*/
        double m_ReynoldsDelta;     /**< the increment value of the Reynolds number for a sequential analysis of Type 4*/


        QVector<Foil*> *m_poaFoil;    /**< pointer to the foil object array */
        QVector<Polar*> *m_poaPolar;  /**< pointer to the polar object array */
        QVector<OpPoint*> *m_poaOpp;  /**< pointer to the OpPoint object array */

        Graph m_CpGraph;           /**< the Cp graph for the OpPoint view */
        QVector<Graph*> m_PlrGraph;  /**< the array of pointer to the 5 Polar graphs */

        LS2 m_LineStyle;      /**< the style of the lines displayed in the comboboxes*/

        XFoil m_XFoil;                /**< the unique instance of the XFoil object */

        static bool s_bViscous;           /**< true if performing a viscous calculation, false if inviscid */
        static bool s_bAlpha;             /**< true if performing an analysis based on aoa, false if based on Cl */
        static bool s_bInitBL;            /**< true if the boundary layer should be initialized for the next xfoil calculation */
        static bool s_bFromZero;          /**< true if the batch analysis should start from Alpha=0 */
        static bool s_bKeepOpenErrors;    /**< true if the XfoilAnalysisDlg should be kept open if errors occured in the XFoil calculation */
        static bool s_bStoreOpp;          /**< true if the operating points should be stored */
        static Polar s_RefPolar;    /**< Used a static reference to store default data */
        static int s_TimeUpdateInterval;  /**< time interval in ms between two output display updates during an XFoil analysis */
        static QVector<double> s_ReList;        /**< the user-defined list of Re numbers, used for batch analysis */
        static QVector<double> s_MachList;      /**< the user-defined list of Mach numbers, used for batch analysis */
        static QVector<double> s_NCritList;     /**< the user-defined list of NCrit numbers, used for batch analysis */
        static Foil *m_pCurFoil;
        static Polar *m_pCurPolar;
        static OpPoint *m_pCurOpp;
};


