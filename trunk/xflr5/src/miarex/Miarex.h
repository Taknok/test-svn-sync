/****************************************************************************

	Miarex    Copyright (C) 2008-2014 Andre Deperrois adeperrois@xflr5.com

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

/**
 *@file
 *
 * This file implementsthe QMiarex class associated to the GUI of 3D analysis
 *
 */


#ifndef QMIAREX_H
#define QMIAREX_H



#include "../params.h"
#include "threedwidget.h"
#include <QWidget>
#include <QPixmap>
#include <QLabel>
#include <QCheckBox>
#include <QSlider>
#include <QStackedWidget>
#include <QToolButton>
#include <QPushButton>
#include <QRadioButton>
#include <QList>
#include <QDialog>
#include <QSettings>
#include <QXmlStreamWriter>
#include "./analysis/PanelAnalysisDlg.h"
#include "./analysis/LLTAnalysisDlg.h"
#include "../misc/LineBtn.h"
#include "../misc/DoubleEdit.h"
#include "../misc/MinTextEdit.h"
#include "../misc/LineCbBox.h"
#include "../misc/LineDelegate.h"
#include <Body.h>
#include <Wing.h>
#include <Plane.h>
#include <WPolar.h>
#include <WingOpp.h>
#include <PlaneOpp.h>
#include <QGraph.h>
#include "graphtilewidget.h"

/**
 *@class QMiarex
 *@brief This is the class associated to the 3D calculations

  It is the handling class for the Miarex toolbar and it dispatches user commands towards
  object definition, analysis and post-processing.
  The main methods of this class are:
	- Management of wings, planes, polars, and operating points
	- Construction of the panels for 3D calculations
	- Management of the display
	- Management of the LLT and Panel Analysis,
	- Mapping of the analyis results to the operating point and polar objects
*/
class QMiarex : public QWidget
{
	friend class MainFrame;
	friend class ThreeDWidget;
	friend class TwoDWidget;
	friend class GL3DScales;
	friend class GL3dBodyDlg;
	friend class GLLightDlg;
	friend class WingDlg;
	friend class Wing;
	friend class LLTAnalysisDlg;
	friend class StabPolarDlg;
	friend class StabViewDlg;
	friend class PanelAnalysisDlg;
	friend class Plane;
	friend class PlaneDlg;
	friend class ManageBodiesDlg;
	friend class ManageUFOsDlg;
	friend class GL3dWingDlg;
	friend class Settings;
	friend class UFOTableDelegate;
	friend class WPolarDlg;
	friend class WPolar;

	Q_OBJECT


public:

	QMiarex(QWidget *parent = NULL);
	~QMiarex();

signals:
	void projectModified();

private slots:
	void On3DCp();
	void On3DPrefs();
	void On3DReset();
	void on3DView();
	void onAdjustToWing();
	void OnAdvancedSettings();
	void OnAnalyze();
	void OnAnimateWOpp();
	void onAnimateWOppSingle();
	void onAnimateWOppSpeed(int val);
	void OnAnimateModeSingle(bool bStep=true);
	void OnAxes();
	void OnCpSection(int pos);
	void OnCpPosition();
	void OnCpView();
	void OnCurveColor();
	void OnCurveStyle(int index);
	void OnCurveWidth(int index);
	void OnCurWOppOnly();
	void onDefineStabPolar();
	void onDefineWPolar();
	void OnDefineWPolarObject();
	void OnDeleteAllWOpps();
	void OnDeleteAllWPlrOpps();
	void OnDeleteCurPlane();
	void OnDeleteCurWOpp();
	void OnDeleteCurWPolar();
	void OnDeletePlaneOpps();
	void OnDeletePlaneWPolars();
	void OnDownwash();
	void OnDuplicateCurPlane();
	void onEditCurWPolar();
	void OnEditCurWPolarPts();
	void OnEditCurWing();
	void onEditCurBody();
	void onEditCurBodyObject();
	void OnEditCurPlane();
	void OnEditCurObject();
	void onEditCurWPolarObject();
	void OnExporttoAVL();
	void OnExportCurWOpp();
	void OnExportCurWPolar();
	void onExportPlanetoXML();
	void onImportPlanefromXML();
	void OnFinCurve();
	void OnFoilNames();
	void OnFourGraphs();
	void OnGL3DScale();
	void OnHideAllWOpps();
	void OnHideAllWPlrOpps();
	void OnHideAllWPolars();
	void OnHidePlaneOpps();
	void OnHidePlaneWPolars();
	void OnHighlightWOpp();
	void OnInitLLTCalc();
	void OnLight();
	void OnImportWPolar();
	void OnKeepCpSection();
	void OnManagePlanes();
	void OnMasses();
	void OnModalView();
	void OnMoment();
	void OnNewPlane();
	void OnNewPlaneObject();
	void OnOutline();
	void OnPanelForce();
	void OnPanels();
	void OnPolarFilter();
	void OnReadAnalysisData();
	void OnRenameCurPlane();
	void OnRenameCurWPolar();
	void OnResetCpSection();
	void onRootLocusView();
	void OnResetCurWPolar();
	void OnSetupLight();
	void OnScaleWing();
	void OnSequence();
	void OnShowAllWOpps();
	void OnShowAllWPlrOpps();
	void OnShowAllWPolars();
	void OnShowCurve();
	void onShowEllipticCurve();
	void OnShowLift();
	void OnShowIDrag();
	void OnShowTransitions();
	void OnShowPlaneWPolars();
	void OnShowPlaneWPolarsOnly();
	void OnShowPlaneOpps();
	void OnShowVDrag();
	void onShowXCmRef();
	void OnStabCurve();
	void OnStabilityDirection();
	void OnStoreWOpp();
	void OnStreamlines();
	void OnSurfaces();
	void OnSurfaceSpeeds();
	void onStabTimeView();
	void OnTwoGraphs();
	void OnPlaneInertia();
	void OnVortices();
	void OnNormals();
	void OnWing2Curve();
	void OnPlaneOppProperties();
	void onWOppView();
	void onWPolarView();
	void OnWPolarProperties();

public:
	//overrides
	void keyPressEvent(QKeyEvent *event);
	void keyReleaseEvent(QKeyEvent *event);
	void showEvent(QShowEvent *event);

public:
	//class methods
	void doubleClickEvent(QPoint pos);

	WPolar* AddWPolar(WPolar* pWPolar);
	int CalculateMatSize();
	void Connect();
	void createCpCurves();
	void createWPolarCurves();
	void CreateWOppCurves();
	void CreateStabilityCurves();
	void CreateStabRLCurves();
	void CreateStabTimeCurves();
	void CreateStabRungeKuttaCurves();
	void DrawCpLegend(QPainter &painter, QPoint place, int bottom);
	void drawTextLegend();
	void DuplicatePlane();
	void FillComboBoxes(bool bEnable = true);
	void FillWPlrCurve(Curve *pCurve, WPolar *pWPolar, int XVar, int YVar);
	void FillWOppCurve(WingOpp *pWOpp, Graph *pGraph, Curve *pCurve);
	void FillStabCurve(Curve *pCurve, WPolar *pWPolar, int iMode);
	void GLCallViewLists();
	void GLDraw3D();
	void GLDrawMasses();
	void GLRenderView();
	bool IntersectObject(CVector O,  CVector U, CVector &I);
	void LLTAnalyze(double V0, double VMax, double VDelta, bool bSequence, bool bInitCalc);	
	bool loadSettings(QSettings *pSettings);
	void paintCpLegendText(QPainter &painter);
	void paintPanelForceLegendText(QPainter &painter);
	void PanelAnalyze(double V0, double VMax, double VDelta, bool bSequence);
	void paintPlaneLegend(QPainter &painter, Plane *pPlane, WPolar *pWPolar, QRect drawRect);
	void paintPlaneOppLegend(QPainter &painter, QRect drawRect);
	void RenamePlane(QString PlaneName);
	bool saveSettings(QSettings *pSettings);
	void Set3DScale();
	void SetAnalysisParams();
	void setControls();
	void setCurveParams();
	void setGraphTiles();
	bool setPlaneOpp(bool bCurrent, double x = 0.0);
	void SetScale();
	void SetStabGraphTitles();
	void setPlane(QString PlaneName="");
	void SetupLayout();
	void SetViewControls();
	void setView(XFLR5::enumGraphView eView);
	void SetWGraphScale();
	void setWGraphTitles(Graph* pGraph);
	void setWPolar(bool bCurrent = true, QString WPlrName = "");
	void snapClient(QString const &FileName);
	void stopAnimate();
	void updateCurve();
	void updateUnits();
	void updateView();


	PlaneOpp *curPOpp(){return m_pCurPOpp;}
	bool curPOppOnly(){return m_bCurPOppOnly;}

//____________________Variables______________________________________
//

public:
	LLTAnalysis  *m_pLLT;                         /**< the one and only instance of the LLT analysis class. */
	LLTAnalysisDlg *m_pLLTDlg;                    /**< the dialog class which manages the LLT calculations */

	PanelAnalysisDlg *m_pPanelAnalysisDlg;        /**< the dialog class which manages the 3D VLM and Panel calculations */

	// Widget variables ... self explicit, not documented
	QPushButton *m_pctrlKeepCpSection, *m_pctrlResetCpSection;
	QSlider *m_pctrlCpSectionSlider;
	DoubleEdit *m_pctrlSpanPos;
	QCheckBox *m_pctrlSequence;
	DoubleEdit *m_pctrlAlphaMin;
	DoubleEdit *m_pctrlAlphaMax;
	DoubleEdit *m_pctrlAlphaDelta;
	QCheckBox *m_pctrlInitLLTCalc;
	QCheckBox *m_pctrlStoreWOpp;
	QPushButton *m_pctrlAnalyze;

	QCheckBox *m_pctrlPanelForce, *m_pctrlLift, *m_pctrlIDrag, *m_pctrlVDrag, *m_pctrlTrans, *m_pctrlWOppAnimate;
	QSlider *m_pctrlAnimateWOppSpeed;
	QCheckBox *m_pctrlMoment,  *m_pctrlDownwash, *m_pctrlCp,*m_pctrlSurfVel, *m_pctrlStream;

	QCheckBox *m_pctrlShowCurve;
	QCheckBox *m_pctrlShowPoints;
	LineCbBox *m_pctrlCurveStyle;
	LineCbBox *m_pctrlCurveWidth;
	LineBtn *m_pctrlCurveColor;
	LineDelegate *m_pStyleDelegate, *m_pWidthDelegate;

	QCheckBox *m_pctrlAxes, *m_pctrlLight, *m_pctrlSurfaces, *m_pctrlOutline, *m_pctrlPanels;
	QCheckBox *m_pctrlFoilNames, *m_pctrlVortices, *m_pctrlPanelNormals, *m_pctrlMasses;

	QAction *m_pXView, *m_pYView, *m_pZView, *m_pIsoView;
	QToolButton *m_pctrlX, *m_pctrlY, *m_pctrlZ, *m_pctrlIso;

	QPushButton *m_pctrlReset;
	QSlider *m_pctrlClipPlanePos;

	QLabel *m_pctrlUnit1, *m_pctrlUnit2, *m_pctrlUnit3;
	QLabel *m_pctrlParameterName;

	//stability widgets
	MinTextEdit *m_pctrlPolarProps;
	QStackedWidget *m_pctrBottomControls, *m_pctrlMiddleControls;


public:

	static bool s_bOutline;                   /**< true if the surface outlines are to be displayed in the 3D view*/
	static bool s_bSurfaces;                  /**< true if the surfaces are to be displayed in the 3D view*/
	static bool s_bVLMPanels;                 /**< true if the panels are to be displayed in the 3D view*/
	static bool s_bAxes;                      /**< true if the axes are to be displayed in the 3D view*/

	// Class variables

	bool m_b3DCp;                      /**< true if the Cp Colors are to be displayed on the 3D openGl view */
	bool m_bAnimateWOpp;               /**< true if there is an animation going on for an operating point */
	bool m_bAnimateMode;               /**< true if there is an animation going on for a Mode */
	bool m_bAnimateWOppPlus;           /**< true if the animation is going in aoa crescending order */
	bool m_bArcball;                   /**< true if the arcball is to be displayed */
	bool m_bAutoScales;                /**< true if automatic scales are requested for the graphs */
	bool m_bCrossPoint;                /**< true if the control point on the arcball is to be displayed */
	bool m_bCurPOppOnly;               /**< true if only the current WOpp is to be displayed */
	bool m_bCurFrameOnly;              /**< true if only the currently selected body frame is to be displayed */
	bool m_bCurvePoints;               /**< true if the points of the active curve are to be displayed */
	bool m_bCurveVisible;              /**< true if the active curve is to be displayed */
	bool m_bDirichlet;                 /**< true if Dirichlet BC are applied in 3D panel analysis, false if Neumann */
	bool m_bDownwash;                  /**< true if the arrows represeting downwash are to be displayed on the 3D openGl view */
	bool m_bHighlightOpp;              /**< true if the currently selected operating point is to be highlighted on the polar graph */
	bool m_bInitLLTCalc;               /**< true if the LLT parameters should be set to default prior to the analysis. Otherwise, the iterations will start at the results of the previous calculation */
	bool m_bIs2DScaleSet;              /**< true if the 3D scale has been set, false if needs to be reset */
	bool m_bIs3DScaleSet;              /**< true if the 3D scale has been set, false if needs to be reset */
	bool m_bLogFile;			       /**< true if the log file warning is turned on */
	bool m_bLongitudinal;              /**< true if longitudinal stability results are to be displayed, false if lateral */
	bool m_bMoments;                   /**< true if the arrows representing moments are to be displayed on the 3D openGl view */
	bool m_bPanelForce;                /**< true if the forces acting on the panels are to be displayed in the 3D view */
	bool m_bPickCenter;                /**< true if the user is in the process of picking a new center for OpenGL display */
	bool m_bResetWake;                 /**< true if the wake geometry should be reset to its default shape prior to the analysis */
	bool m_bSequence;                  /**< true if a sequential analysis is to be performed */
	bool m_bShowCp;                    /**< true if the active curve should be displayed in Cp view */
	bool m_bShowCpScale;               /**< true if the Cp Scale in Miarex is to be displayed */
	bool m_bShowCpPoints;              /**< true if the points of the active curve should be displayed in Cp view */
	bool m_bShowElliptic;              /**< true if the elliptic loading should be displayed in the local lift graph */
	bool m_bShowWingCurve[MAXWINGS];   /**< true if various plane's wing curves shoud be displayed*/
	bool m_bSurfVelocities;                    /**< true if the velocities should be displayed in the operating point or 3D view*/
	bool m_bStream;                    /**< defines whether the streamlines should be displayed in the operating point or 3D view*/
	bool m_bTrans;                     /**< true if the view is being dragged */
	bool m_bTransGraph;	               /**< true if a graph is being dragged */
	bool m_bType1;                     /**< true if polars of type 1 are to be displayed */
	bool m_bType2;                     /**< true if polars of type 2 are to be displayed */
	bool m_bType4;                     /**< true if polars of type 4 are to be displayed */
	bool m_bType7;                     /**< true if polars of type 71 are to be displayed */
	bool m_bVortices;                  /**< true if the panel vortices should be displayed */
	bool m_bPanelNormals;                   /**< true if the panel normals should be displayed */
	bool m_bXCmRef; 	               /**< true if the position of the reference point for the moments should be displayed in the operating point view*/
	bool m_bXBot;                      /**< true if the transition on the bottom surface should be displayed in the operating point or in 3D view*/
	bool m_bXCP;                       /**< true if the lift curve should be displayed in the operating point or in the 3D view*/
	bool m_bXTop;                      /**< true if the transition on the top surface should be displayed in the operating point or in 3D view */
	bool m_bXPressed;                  /**< true if the X key is pressed */
	bool m_bYPressed;                  /**< true if the Y key is pressed */

	static bool m_bResetglGeom;               /**< true if the geometry OpenGL list needs to be re-generated */
	static bool m_bResetglMesh;               /**< true if the mesh OpenGL list needs to be re-generated */
	static bool m_bResetglWake;               /**< true if the wake OpenGL list needs to be re-generated */
	static bool m_bResetglOpp;                /**< true if the OpenGL lists need to be re-generated */
	static bool m_bResetglLift;               /**< true if the OpenGL lists need to be re-generated */
	static bool m_bResetglDrag;               /**< true if the OpenGL lists need to be re-generated */
	static bool m_bResetglDownwash;           /**< true if the OpenGL lists need to be re-generated */
	static bool m_bResetglPanelForce;         /**< true if the OpenGL lists need to be re-generated */
	static bool m_bResetglPanelCp;            /**< true if the OpenGL lists need to be re-generated */
	static bool m_bResetglStream;             /**< true if the streamlines OpenGL list needs to be re-generated */
	static bool m_bResetglLegend;             /**< true if the legend needs to be reset if the window has been resized */
	static bool m_bResetglBody;               /**< true if the openGL list for the body needs to be re-generated */
	static bool m_bResetglBodyMesh;           /**< true if the openGL list for panel mesh needs to be re-generated */
	static bool m_bResetglSurfVelocities;               /**< true if the crossflow OpenGL list needs to be refreshed */

	static bool s_bResetCurves;               /**< true if the curves of the active view should be regenerated before the next view update >*/

	static bool s_bICd;                       /**< true if the induced drag forces should be displayed in the operating point or 3D view*/
	static bool s_bVCd;                       /**< true if the viscous drag forces should be displayed in the operating point or 3D view*/

	static bool s_bAutoCpScale;		       /**< true if the Cp scale should be set automatically */
	static double s_LegendMin;                /**< minimum value of the Cp scale in 3D view */
	static double s_LegendMax;                /**< maximum value of the Cp scale in 3D view */

	static double s_LiftScale;                /**< scaling factor for the lift display in 3D view */
	static double s_VelocityScale;            /**< scaling factor for the velocity display in 3D view */
	static double s_DragScale;                /**< scaling factor for the drag display in 3D view */

	PlaneOpp * m_pCurPOpp;           /**< a pointer to the active Plane Operating Point, or NULL if none is active*/




	int m_CurveStyle;             /**< The style of the active curve */
	int m_CurveWidth;             /**< The width of the active curve */
	QColor m_CurveColor;          /**< The color of the active curve */


	QTimer *m_pTimerWOpp;         /**< A pointer to the timer which signals the animation in the operating point and 3D view */
	QTimer *m_pTimerMode;         /**< A pointer to the timer which signals the animation of the modes in the 3D view */


	WingOpp *m_pWOpp[MAXWINGS];   /**< an array of pointers to the operating points of the four wings of the currently selected plane */


	static QList<void *> *m_poaPlane;			/**< for convenienece, a pointer to the array of pointers to plane objects */
	static QList<void *> *m_poaWPolar;			/**< for convenienece, a pointer to the array of UFO polar objects */
	static QList<void *> *m_poaPOpp;			/**< for convenienece, a pointer to the array of Plane OpPoint objects */
	static QList<void *> *m_poaBody;			/**< for convenienece, a pointer to the array of Body objects */


	Plane * m_pCurPlane;          /**< the currently active Plane */
	WPolar * m_pCurWPolar;        /**< the currently active WPolar */
	Wing *m_pWingList[MAXWINGS];  /**< for convenience, an array of pointers to the four wings of the currently selected plane */


	int m_StabilityResponseType;   /**< 0 = initial conditions, 1=forced response, 2=modal response */

	XFLR5::enumGraphView m_iWPlrView;              /**< defines how many graphs will be displayed in WPolar view */
	XFLR5::enumGraphView m_iWingView;              /**< defines how many graphs will be displayed in the operating point view */
	XFLR5::enumGraphView m_iStabTimeView;          /**< defines how many graphs will be displayed in the stability time view */

	int m_InducedDragPoint;     /**< 0 if downwash is at panel's centroid, 1 if averaged over panel length; used in CWing::VLMTrefftz */
	int m_LLTMaxIterations;     /**< the number of iterations for LLT */
	int m_posAnimateWOpp;       /**< the current animation aoa ind ex for WOpp animation */
	int m_posAnimateMode;       /**< the current animation aoa index for Mode animation */
	int m_WakeInterNodes;		/**< number of intermediate nodes between wake panels */

	XFLR5::enumMiarexViews m_iView;    /**< defines the currently active view */


	double m_CurSpanPos;        /**< Span position for Cp Graph  */

	double m_AlphaMin;          /**< the min value of the aoa for sequential analysis*/
	double m_AlphaMax;          /**< the max value of the aoa for sequential analysis*/
	double m_AlphaDelta;        /**< the increment value of the aoa for sequential analysis*/
	double m_BetaMin;           /**< the min value of the sideslip for sequential analysis*/
	double m_BetaMax;           /**< the max value of the sideslip for sequential analysis*/
	double m_BetaDelta;         /**< the increment value of the sideslip for sequential analysis*/
	double m_ControlMin;        /**< the min value of the control parameter for sequential stability analysis*/
	double m_ControlMax;        /**< the max value of the control parameter for sequential stability analysis*/
	double m_ControlDelta;      /**< the increment value of the control parameter for sequential stability analysis*/
	double m_QInfMin;           /**< the min value of the velocity for sequential analysis */
	double m_QInfMax;           /**< the max value of the velocity for sequential analysis */
	double m_QInfDelta;         /**< the increment value of the velocity for sequential analysis */

	// mode animation data
	double m_ModeState[6];      /**< defines the value of the 6 dofs (x, y, z, rx, ry, rz) to display the position and orientation of the geometry */
	double m_ModeNorm;          /**< defines the amplitude of the modal animation */
	double m_ModeTime;          /**< defines the time moment for the modal animation */
	double m_Modedt;            /**< defines the time increment for the modal animation */
	
	// time curve data
	double m_TimeInput[4];      /**< defines the initial state for the display of the dynamic response in stability time graphs */
	double m_TotalTime;         /**< defines the total time for the display of the dynamic response in stability time graphs */
	double m_Deltat;            /**< defines the time increment for the display of the dynamic response in stability time graphs */
	double m_RampTime;          /**< defines the ramp time for the display of the dynamic response in stability time graphs */
	double m_RampAmplitude;     /**< defines the ramp amplitude for the display of the dynamic response in stability time graphs */

	static int s_GLList;               /**< the number of existing OpenGl list at a given time. @deprecated, for development purposes only */

	QFile* m_pXFile;            /**< a pointer to the output .log file */


	QGraph m_CpGraph;                       /**< the Cp Graph in 3D panel analysis */
	QList<QGraph*> m_WingGraph;             /**< the array of pointer to the OpPoint graphs */
	QList<QGraph*> m_WPlrGraph;             /**< the array of pointers to the WPolar graphs */
	QList<QGraph*> m_StabPlrGraph;          /**< the array of pointers to the two root locus graphs */
	QList<QGraph*> m_TimeGraph;             /**< the array of pointers to the time response graphs in stability view */

	bool m_bResetTextLegend;
	QPixmap m_PixText;

public:
	static void *s_pMainFrame;          /**< a pointer to the frame class */
	ThreeDWidget *m_p3dWidget;            /**< a pointer to the openGL widget where 3d calculations and rendering are performed */

	static CVector *s_pNode;              /**< a static pointer to the node array for the currently loaded Plane*/
	static CVector *s_pMemNode;           /**< a static pointer to used if the analysis should be performed on the tilted geometry */
	static CVector *s_pWakeNode;          /**< a static pointer to the current wake node array */
	static CVector *s_pRefWakeNode;       /**< a static pointer to the reference wake node array if wake needs to be reset */

	static Panel *s_pPanel;               /**< a static pointer to the panel array for the currently loaded Plane */
	static Panel *s_pMemPanel;            /**< a static pointer to an array used if the analysis should be performed on the tilted geometry */
	static Panel *s_pWakePanel;           /**< a static pointer to the reference current wake panel array */
	static Panel *s_pRefWakePanel;        /**< a static pointer to the reference wake panel array if wake= new CVector needs to be reset */

public:
	QColor m_CpColor;
	int m_CpStyle, m_CpWidth;

};

#endif // QMIAREX_H
