/****************************************************************************

	Techwing Application

	Copyright (C) Andre Deperrois adeperrois@xflr5.com

	All rights reserved.

*****************************************************************************/

#ifndef PLANEANALYSISTASK_H
#define PLANEANALYSISTASK_H


#include <QEvent>
#include <QTextStream>

#include <plane_analysis/LLTAnalysis.h>
#include <plane_analysis/PanelAnalysis.h>

class Plane;
class WPolar;
class PlaneOpp;
class Surface;
class Panel;
class Vector3d;


/**
 * @struct A set of a plane, a polar and a range to analyze, used by a thread to perform one analysis.
 */
struct PlaneAnalysis
{
	Plane *pPlane;            /**< a pointer to the Foil to be analyzed by the thread */
	WPolar *pWPolar;          /**< a pointer to the polar to be analyzed by the thread */
	double vMin, vMax, vInc;
};


class PlaneAnalysisTask
{
	friend class MainFrame;
	friend class QMiarex;
	friend class PanelAnalysisDlg;
	friend class LLTAnalysisDlg;
	friend class gl3dMiarexView;
	friend class gl3dView;

public:
	PlaneAnalysisTask();
	~PlaneAnalysisTask();

	void initializeTask(Plane *pPlane, WPolar *pWPolar, double vMin, double vMax, double VInc, bool bSequence = true);
	void initializeTask(PlaneAnalysis *pAnalysis);

	bool   allocatePanelArrays(int &memsize);
	int    calculateMatSize();
	int    createBodyElements(Plane *pCurPlane);
	bool   createWakeElems(int PanelIndex, Plane *pPlane, WPolar *pWPolar);
	int    createWingElements(Plane *pPlane, WPolar *pWPolar, Surface *pSurface);
	bool   initializePanels();
	void   insertPOpp(PlaneOpp *pPOpp);
	int    isNode(Vector3d &Pt);
	int    isWakeNode(Vector3d &Pt);
	void   joinSurfaces(WPolar*pWPolar, Surface *pLeftSurf, Surface *pRightSurf, int pl, int pr);
	void   releasePanelMemory();
	void   setParent(void *pParent);
	void   setStaticPointers();
	void   stitchSurfaces();
	void   setLLTAnalysis(LLTAnalysis &LLTAnalysis)       {m_ptheLLTAnalysis   = &LLTAnalysis;}
	void   setPanelAnalysis(PanelAnalysis &panelAnalysis) {m_pthePanelAnalysis = &panelAnalysis;}

	bool isFinished(){return m_bIsFinished;}

	WPolar *  setWPolarObject(Plane *pCurPlane, WPolar *pCurWPolar);
	Plane *   setPlaneObject(Plane *pPlane);

	void LLTAnalyze();
	void PanelAnalyze();
	void run();
	static void cancelTask(){s_bCancel=true;}

	PanelAnalysis *m_pthePanelAnalysis;
	LLTAnalysis *m_ptheLLTAnalysis;

	bool isLLTTask();
	bool isPanelTask();

private:
	void *m_pParent;

	Plane *m_pPlane;
	WPolar *m_pWPolar;


	//data arrays
	Vector3d *m_Node;              /**< the node array for the currently loaded UFO*/
	Vector3d *m_MemNode;           /**< used if the analysis should be performed on the tilted geometry */
	Vector3d *m_WakeNode;          /**< the current wake node array */
	Vector3d *m_RefWakeNode;       /**< the reference wake node array if wake needs to be reset */

	Panel *m_Panel;               /**< the panel array for the currently loaded UFO */
	Panel *m_MemPanel;            /**< used if the analysis should be performed on the tilted geometry */
	Panel *m_WakePanel;           /**< the reference current wake panel array */
	Panel *m_RefWakePanel;        /**< the reference wake panel array if wake= new Vector3d needs to be reset */

	int m_MatSize;                     /**< the matrix size, wich is also the size of the panel array */
	int m_WakeSize;                    /**< the size of the wake matrix, if a wake is included in the analysis */
	int m_nNodes;                      /**< the number of nodes corresponding to the panel array */
	int m_nWakeNodes;                  /**< the number of nodes in the wake */
	int m_NWakeColumn;                 /**< the number of wake columns */

	int m_MaxPanelSize;                  /**< the maximum matrix size consistent <ith the current memory allocation */

	QList<Surface *> m_SurfaceList;	    /**< An array holding the pointers to the wings Surface objects */

	double m_vMin, m_vMax, m_vInc;
	bool m_bSequence;
	bool m_bIsFinished;       /**< true if the calculation is over */
	static bool s_bCancel;    /**< true if all analysis should be cancelled */

};


#endif // PLANEANALYSISTASK_H




