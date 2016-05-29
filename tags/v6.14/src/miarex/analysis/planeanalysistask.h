/****************************************************************************

	PlaneAnalysisTask Class
	   Copyright (C) 2016-2016 Andre Deperrois adeperrois@xflr5.com

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

#ifndef PLANEANALYSISTASK_H
#define PLANEANALYSISTASK_H

#include <QRunnable>
#include <QTextStream>

class Plane;
class WPolar;
class PlaneOpp;
class Surface;
class Panel;
class CVector;
class LLTAnalysis;
class PanelAnalysis;



struct AnalysisData
{
	float m_min, m_max, m_delta;
	bool m_bStorePOpp;
};


class PlaneAnalysisTask  : public QRunnable
{
public:
	PlaneAnalysisTask(void *pParent = NULL);
	void   initializeTask(Plane *pPlane, WPolar *pWPolar, AnalysisData &Data);

	bool   allocatePanelArrays(int &memsize);
	int    calculateMatSize(Plane *pPlane, WPolar *pWPolar);
	int    createBodyElements(Plane *pCurPlane);
	bool   createWakeElems(int PanelIndex, Plane *pPlane, WPolar *pWPolar);
	int    createWingElements(Plane *pPlane, WPolar *pWPolar, Surface *pSurface);
	bool   initializePanels(Plane *pPlane, WPolar *pWPolar);
	void   initPanelAnalysis(Plane* pPlane, WPolar* pWPolar, double V0, double VMax, double VDelta, bool bSequence);
	void   insertPOpp(PlaneOpp *pPOpp);
	int    isNode(CVector &Pt);
	int    isWakeNode(CVector &Pt);
	void   joinSurfaces(WPolar*pWPolar, Surface *pLeftSurf, Surface *pRightSurf, int pl, int pr);
	void   rotateGeomY(Panel *pPanel, CVector *pNode, Panel *pWakePanel, CVector *pWakeNode, int nPanels, int nWakePanels, int nNodes, int nWakeNodes, int NWakeColumn, double const &Alpha, CVector const &P, int NXWakePanels);
	void   rotateGeomZ(Panel *pPanel, CVector *pNode, Panel *pWakePanel, CVector *pWakeNode, int nPanels, int nWakePanels, int nNodes, int nWakeNodes, int NWakeColumn, double const &Beta, CVector const &P, int NXWakePanels);
	void   releasePanelMemory();
	void   setStaticPointers();
	void   setControlPositions(Plane *pPlane, WPolar *pWPolar, Panel *pPanel, CVector *pNode, double t, int &NCtrls, QString &out, bool bBCOnly);


	WPolar *  setWPolarObject(WPolar *pWPolar);
	Plane *   setPlaneObject(Plane *pPlane);


private:
	void run();

	void *m_pParent;

	Plane *m_pPlane;
	WPolar *m_pWPolar;
	AnalysisData m_Data;

	PanelAnalysis *m_pPanelAnalysis;
	LLTAnalysis *m_pLLTAnalysis;

	//data arrays
	CVector *m_Node;              /**< the node array for the currently loaded UFO*/
	CVector *m_MemNode;           /**< used if the analysis should be performed on the tilted geometry */
	CVector *m_WakeNode;          /**< the current wake node array */
	CVector *m_RefWakeNode;       /**< the reference wake node array if wake needs to be reset */

	Panel *m_Panel;               /**< the panel array for the currently loaded UFO */
	Panel *m_MemPanel;            /**< used if the analysis should be performed on the tilted geometry */
	Panel *m_WakePanel;           /**< the reference current wake panel array */
	Panel *m_RefWakePanel;        /**< the reference wake panel array if wake= new CVector needs to be reset */

	int m_MatSize;                     /**< the matrix size, wich is also the size of the panel array */
	int m_WakeSize;                    /**< the size of the wake matrix, if a wake is included in the analysis */
	int m_nNodes;                      /**< the number of nodes corresponding to the panel array */
	int m_nWakeNodes;                  /**< the number of nodes in the wake */
	int m_NWakeColumn;                 /**< the number of wake columns */

	int m_MaxPanelSize;                  /**< the maximum matrix size consistent <ith the current memory allocation */

	QTextStream m_OutStream;
	QTextStream m_AnalysisStream;
	QString m_OutMessage;
	QString m_AnalysisLog;


	QList<Surface *> m_SurfaceList;	    /**< An array holding the pointers to the wings Surface objects */
};

#endif // PLANEANALYSISTASK_H
















