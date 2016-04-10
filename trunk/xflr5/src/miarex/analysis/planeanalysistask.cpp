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

#include "planeanalysistask.h"
#include <objects/Plane.h>
#include <objects/WPolar.h>
#include <objects/Surface.h>
#include <miarex/analysis/LLTAnalysis.h>
#include <miarex/analysis/PanelAnalysis.h>


PlaneAnalysisTask::PlaneAnalysisTask(void *pParent)
{
	m_pPlane = NULL;
	m_pWPolar = NULL;
	m_pPanelAnalysis = NULL;
	m_pLLTAnalysis = NULL;

	m_Data.m_min = 0.0;
	m_Data.m_max = 1.0;
	m_Data.m_delta = 0.5;
	m_Data.m_bStorePOpp = false;

	m_pParent = pParent;
}


void PlaneAnalysisTask::initializeTask(Plane *pPlane, WPolar *pWPolar, AnalysisData &Data)
{
	m_pPlane = pPlane;
	m_pWPolar = pWPolar;
	memcpy(&m_Data, &Data, sizeof(AnalysisData));
}




void PlaneAnalysisTask::run()
{

}



/**
 * Sets the active plane
 * Constructs the surface
 * Calculates the inertia
 * @param PlaneName the name of the plane to be set as active
 */
Plane * PlaneAnalysisTask::setPlaneObject(Plane *pPlane)
{
	m_pPlane = pPlane;
	double dx=0, dz=0;

	if(pPlane->body())
	{
		dx = pPlane->bodyPos().x;
		dz = pPlane->bodyPos().z;
		pPlane->body()->setNURBSKnots();
		pPlane->body()->setPanelPos();
	}

	Body *pCurBody = pPlane->body();

	//create the array of wing Surface
	m_SurfaceList.clear();
	for(int iw=0; iw<MAXWINGS; iw++)
	{
		if(pPlane->wing(iw))
		{
//			if(!pPlane && iw==0)  pPlane->wing(iw)->CreateSurfaces(CVector(0,0,0), 0.0, 0.0);
			if(iw<3)         pPlane->wing(iw)->createSurfaces(pPlane->WingLE(iw),   0.0, pPlane->WingTiltAngle(iw));
			else if(iw==3)   pPlane->wing(iw)->createSurfaces(pPlane->WingLE(iw), -90.0, pPlane->WingTiltAngle(iw));
			for (int j=0; j<pPlane->wing(iw)->m_Surface.size(); j++)
			{
				pPlane->wing(iw)->m_Surface.at(j)->setSidePoints(pCurBody, dx, dz);
				m_SurfaceList.append(pPlane->wing(iw)->m_Surface.at(j));
			}
			pPlane->wing(iw)->computeBodyAxisInertia();
		}
	}

	pPlane->computeBodyAxisInertia();

	return pPlane;
}





/**
 * Sets the active polar
 * Builds the array of panels depending on the polar type
 * @param bCurrent if true, the active polar is set anew
 * @param WPlrName the name of the polar to set for the active wing or plane
 */
WPolar* PlaneAnalysisTask::setWPolarObject(WPolar *pWPolar)
{
	int NStation;
	double SpanPos;

	if(!m_pPlane || !pWPolar) return NULL; // we skipped something

	if (!initializePanels(m_pPlane, pWPolar)) return NULL;

	if(!pWPolar) return NULL;

	//initialize the analysis pointers.
	//do it now, in case the user asks for streamlines from an existing file
	m_pPanelAnalysis->setWPolar(pWPolar);
	m_pPanelAnalysis->setObjectPointers(m_pPlane, &m_SurfaceList);
	m_pPanelAnalysis->setArrayPointers(m_Panel, m_MemPanel, m_WakePanel, m_RefWakePanel, m_Node, m_MemNode, m_WakeNode, m_RefWakeNode);
	m_pPanelAnalysis->setArraySize(m_MatSize, m_WakeSize, m_nNodes, m_nWakeNodes, m_NWakeColumn);


	//set sideslip
	CVector RefPoint(0.0, 0.0, 0.0);
	if(qAbs(pWPolar->m_BetaSpec)>0.001 && !pWPolar->isBetaPolar())
	{
		// Standard Convention in mechanic of flight is to have Beta>0 with nose to the left
		// The yaw moement has the opposite convention...
		rotateGeomZ(m_MemPanel, m_MemNode, m_WakePanel, m_WakeNode,
					m_MatSize, m_WakeSize, m_nNodes, m_nWakeNodes, m_NWakeColumn,
					pWPolar->m_BetaSpec, RefPoint, pWPolar->m_NXWakePanels);
	}

	Wing *pWingList[MAXWINGS];
	pWingList[0] = m_pPlane->wing();
	pWingList[1] = m_pPlane->wing2();
	pWingList[2] = m_pPlane->stab();
	pWingList[3] = m_pPlane->fin();

	if(pWPolar->analysisMethod()>XFLR5::LLTMETHOD)
	{
		for(int iw=0; iw<MAXWINGS; iw++)
		{
			if(pWingList[iw])
			{
				pWingList[iw]->computeChords();

				NStation = 0;
				int m=0;
				SpanPos = 0;
				for (int j=0; j<pWingList[iw]->m_Surface.size(); j++)	NStation += pWingList[iw]->m_Surface.at(j)->NYPanels();

				for (int j=(int)(pWingList[iw]->m_Surface.size()/2); j<pWingList[iw]->m_Surface.size(); j++)
				{
					for(int k=0; k<pWingList[iw]->m_Surface.at(j)->NYPanels(); k++)
					{
						pWingList[iw]->m_SpanPos[m+NStation/2] = SpanPos + pWingList[iw]->m_Surface.at(j)->stripSpanPos(k);
						m++;
					}
					SpanPos += pWingList[iw]->m_Surface.at(j)->planformLength();
				}

				for(m=0; m<NStation/2; m++) pWingList[iw]->m_SpanPos[m] = -pWingList[iw]->m_SpanPos[NStation-m-1];
			}
		}
	}
	else if(pWPolar->analysisMethod()==XFLR5::LLTMETHOD)
	{
//			m_pPlane->m_Wing[0].m_NStation  = m_NStation;
//			m_pPlane->m_Wing[0].m_bLLT      = true;
	}


	/** @todo need to cancel results too if we modify the inertia */
	if(pWPolar->m_bAutoInertia)
	{
		if(m_pPlane)
		{
			pWPolar->mass()   = m_pPlane->totalMass();
			pWPolar->CoG()    = m_pPlane->CoG();
			pWPolar->CoGIxx() = m_pPlane->CoGIxx();
			pWPolar->CoGIyy() = m_pPlane->CoGIyy();
			pWPolar->CoGIzz() = m_pPlane->CoGIzz();
			pWPolar->CoGIxz() = m_pPlane->CoGIxz();
		}
	}

	return pWPolar;
}




/**
 * Following the selection of a wing or a plane, this subroutine creates the panels
 * associated to all of the surface objects.
 *
 * m_Panel is the array of panels in the following order
 *         main wing left side, main wing right side
 *         second wing
 *         elevator
 *         fin
 *         body
 * Each panel gets the index of its symmetric, if any, else this index is set to -1
 *
 * A copy of the panels is saved to the MemPanel and MemNode arrays
 *@return true if successful, false if the panels could not be properly created ot if no object is active
*/
bool PlaneAnalysisTask::initializePanels(Plane *pPlane, WPolar *pWPolar)
{
	if(!pPlane) return false;
	int Nel=0;

	// first check that the total number of panels that will be created does not exceed
	// the currently allocated memory size for the influence atrix.

	int PanelArraySize = calculateMatSize(pPlane, pWPolar);
	int memsize = 0;

	if(PanelArraySize>m_MaxPanelSize)
	{
//		Trace(QString("PlaneAnalysisTask::Requesting additional memory for %1 panels").arg(PanelArraySize));

		// allocate 10% more than needed to avoid repeating the operation if the user requirement increases sightly again.
		m_MaxPanelSize = (int)((double)PanelArraySize *1.1);
		releasePanelMemory();

		if(!allocatePanelArrays(memsize))
		{
			m_MaxPanelSize = 0;
			return false;
		}
	}

	//if a WPolar is defined, allocate the matrix
	if(pWPolar)
	{
		int MatrixSize=0;

		if(!m_pPanelAnalysis->allocateMatrix(m_MaxPanelSize, MatrixSize))
		{
			releasePanelMemory();
			return false;
		}

//		Trace("");
		memsize += MatrixSize;
	}


	// all set to create the panels

	m_MatSize     = 0;
	m_nNodes      = 0;
	m_NWakeColumn = 0;
	m_nWakeNodes  = 0;
	m_WakeSize    = 0;

	memset(m_Panel, 0, m_MaxPanelSize * sizeof(Panel));
	memset(m_Node,  0, 2 * m_MaxPanelSize * sizeof(CVector));

	Panel *ptr = m_Panel;

//	dlg.setValue(5);
//	int NXWakePanels;
//	if(pCurWPolar)	NXWakePanels = pCurWPolar->m_NXWakePanels;
//	else                NXWakePanels = 1;

	Wing *pWingList[MAXWINGS];
	pWingList[0] = pPlane->wing();
	pWingList[1] = pPlane->wing2();
	pWingList[2] = pPlane->stab();
	pWingList[3] = pPlane->fin();


	for(int iw=0; iw<MAXWINGS; iw++)
	{
		if(pWingList[iw])
		{
			pWingList[iw]->m_MatSize = 0;
			for(int jSurf=0; jSurf<pWingList[iw]->m_Surface.size(); jSurf++)
			{
				pWingList[iw]->m_Surface.at(jSurf)->resetFlap();
				Nel = createWingElements(pPlane, pWPolar, pWingList[iw]->m_Surface.at(jSurf));
				pWingList[iw]->m_MatSize += Nel;
			}
			pWingList[iw]->m_pWingPanel = ptr;
			ptr += pWingList[iw]->m_MatSize;
		}
	}

	bool bBodyEl = false;
	if(pPlane && pPlane->body())
	{
		if(!pWPolar) bBodyEl = true;//no risk...
		else if(pWPolar->analysisMethod()==XFLR5::PANELMETHOD && !pWPolar->bIgnoreBodyPanels())
		{
			bBodyEl = true;
		}
	}

	if(bBodyEl)
	{
		Nel = createBodyElements(pPlane);

		if(pPlane && pPlane->body())
			pPlane->body()->m_pBodyPanel = ptr;

	}

	//back-up the current geometry
	memcpy(m_MemPanel, m_Panel, m_MatSize* sizeof(Panel));
	memcpy(m_MemNode,  m_Node,  m_nNodes * sizeof(CVector));
	memcpy(m_RefWakePanel, m_WakePanel, m_WakeSize* sizeof(Panel));
	memcpy(m_RefWakeNode,  m_WakeNode,  m_nWakeNodes * sizeof(CVector));


//	dlg.setValue(100);

	return true;
}







/**
 * Creates the body panels for the active CBody object
 * The panels are created in the following order
 *    - for the port side  first, then for the starboard side
 *    - from bottom to top
 *    - from tip to tail
 * The panels are appended to the existing array of panels
 * @return the number of panels which have been created and appended
 */
int PlaneAnalysisTask::createBodyElements(Plane *pCurPlane)
{
	if(!pCurPlane) return 0;
	if(!pCurPlane->body()) return 0;

	Body *pCurBody = pCurPlane->body();

	int i,j,k,l;
	double uk, uk1, v, dj, dj1, dl1;
	double dpx, dpz;
	CVector LATB, TALB;
	CVector LA, LB, TA, TB;
	CVector PLA, PTA, PLB, PTB;

	int n0, n1, n2, n3, lnx, lnh;
	int nx = pCurBody->nxPanels();
	int nh = pCurBody->nhPanels();
	int p = 0;

	int InitialSize = m_MatSize;
	int FullSize =0;

	lnx = 0;

	if(pCurPlane && pCurPlane->body())
	{
		dpx = pCurPlane->bodyPos().x;
		dpz = pCurPlane->bodyPos().z;
	}
	else dpx=dpz=0.0;

	if(pCurBody->m_LineType==XFLR5::BODYPANELTYPE)
	{
		nx = 0;
		for(i=0; i<pCurBody->frameCount()-1; i++) nx+=pCurBody->m_xPanels[i];
		nh = 0;
		for(i=0; i<pCurBody->sideLineCount()-1; i++) nh+=pCurBody->m_hPanels[i];
		FullSize = nx*nh*2;
		pCurBody->nxPanels() = nx;
		pCurBody->nhPanels() = nh;

		for (i=0; i<pCurBody->frameCount()-1; i++)
		{
			for (j=0; j<pCurBody->m_xPanels[i]; j++)
			{
				dj  = (double) j   /(double)(pCurBody->m_xPanels[i]);
				dj1 = (double)(j+1)/(double)(pCurBody->m_xPanels[i]);

				//body left side
				lnh = 0;
				for (k=0; k<pCurBody->sideLineCount()-1; k++)
				{
					//build the four corner points of the strips
					PLB.x =  (1.0- dj) * pCurBody->framePosition(i)      +  dj * pCurBody->framePosition(i+1)       +dpx;
					PLB.y = -(1.0- dj) * pCurBody->frame(i)->m_CtrlPoint[k].y   -  dj * pCurBody->frame(i+1)->m_CtrlPoint[k].y;
					PLB.z =  (1.0- dj) * pCurBody->frame(i)->m_CtrlPoint[k].z   +  dj * pCurBody->frame(i+1)->m_CtrlPoint[k].z    +dpz;

					PTB.x =  (1.0-dj1) * pCurBody->framePosition(i)      + dj1 * pCurBody->framePosition(i+1)       +dpx;
					PTB.y = -(1.0-dj1) * pCurBody->frame(i)->m_CtrlPoint[k].y   - dj1 * pCurBody->frame(i+1)->m_CtrlPoint[k].y;
					PTB.z =  (1.0-dj1) * pCurBody->frame(i)->m_CtrlPoint[k].z   + dj1 * pCurBody->frame(i+1)->m_CtrlPoint[k].z    +dpz;

					PLA.x =  (1.0- dj) * pCurBody->framePosition(i)      +  dj * pCurBody->framePosition(i+1)       +dpx;
					PLA.y = -(1.0- dj) * pCurBody->frame(i)->m_CtrlPoint[k+1].y -  dj * pCurBody->frame(i+1)->m_CtrlPoint[k+1].y;
					PLA.z =  (1.0- dj) * pCurBody->frame(i)->m_CtrlPoint[k+1].z +  dj * pCurBody->frame(i+1)->m_CtrlPoint[k+1].z  +dpz;

					PTA.x =  (1.0-dj1) * pCurBody->framePosition(i)      + dj1 * pCurBody->framePosition(i+1)       +dpx;
					PTA.y = -(1.0-dj1) * pCurBody->frame(i)->m_CtrlPoint[k+1].y - dj1 * pCurBody->frame(i+1)->m_CtrlPoint[k+1].y;
					PTA.z =  (1.0-dj1) * pCurBody->frame(i)->m_CtrlPoint[k+1].z + dj1 * pCurBody->frame(i+1)->m_CtrlPoint[k+1].z  +dpz;

					LB = PLB;
					TB = PTB;

					for (l=0; l<pCurBody->m_hPanels[k]; l++)
					{
						dl1  = (double)(l+1) / (double)(pCurBody->m_hPanels[k]);
						LA = PLB * (1.0- dl1) + PLA * dl1;
						TA = PTB * (1.0- dl1) + PTA * dl1;

						n0 = isNode(LA);
						n1 = isNode(TA);
						n2 = isNode(LB);
						n3 = isNode(TB);

						if(n0>=0) {
							m_Panel[m_MatSize].m_iLA = n0;
						}
						else {
							m_Panel[m_MatSize].m_iLA = m_nNodes;
							m_Node[m_nNodes].copy(LA);
							m_nNodes++;
						}

						if(n1>=0) {
							m_Panel[m_MatSize].m_iTA = n1;
						}
						else {
							m_Panel[m_MatSize].m_iTA = m_nNodes;
							m_Node[m_nNodes].copy(TA);
							m_nNodes++;
						}

						if(n2>=0) {
							m_Panel[m_MatSize].m_iLB = n2;
						}
						else {
							m_Panel[m_MatSize].m_iLB = m_nNodes;
							m_Node[m_nNodes].copy(LB);
							m_nNodes++;
						}

						if(n3 >=0) {
							m_Panel[m_MatSize].m_iTB = n3;
						}
						else {
							m_Panel[m_MatSize].m_iTB = m_nNodes;
							m_Node[m_nNodes].copy(TB);
							m_nNodes++;
						}

						LATB = TB - LA;
						TALB = LB - TA;
						m_Panel[m_MatSize].Normal = LATB * TALB;
						m_Panel[m_MatSize].Area =  m_Panel[m_MatSize].Normal.VAbs()/2.0;
						m_Panel[m_MatSize].Normal.normalize();

						m_Panel[m_MatSize].m_bIsInSymPlane  = false;
						m_Panel[m_MatSize].m_bIsLeading     = false;
						m_Panel[m_MatSize].m_bIsTrailing    = false;
						m_Panel[m_MatSize].m_Pos = BODYSURFACE;
						m_Panel[m_MatSize].m_iElement = m_MatSize;
						m_Panel[m_MatSize].m_bIsLeftPanel  = true;
						m_Panel[m_MatSize].setPanelFrame(LA, LB, TA, TB);

						// set neighbour panels

						m_Panel[m_MatSize].m_iPD = m_MatSize + nh;
						m_Panel[m_MatSize].m_iPU = m_MatSize - nh;

						if(lnx==0)      m_Panel[m_MatSize].m_iPU = -1;// no panel downstream
						if(lnx==nx-1)	m_Panel[m_MatSize].m_iPD = -1;// no panel upstream

						m_Panel[m_MatSize].m_iPL = m_MatSize + 1;
						m_Panel[m_MatSize].m_iPR = m_MatSize - 1;

						if(lnh==0)     m_Panel[m_MatSize].m_iPR = InitialSize + FullSize - p - 1;
						if(lnh==nh-1)  m_Panel[m_MatSize].m_iPL = InitialSize + FullSize - p - 1;

						m_MatSize++;
						p++;
						LB = LA;
						TB = TA;
						lnh++;
					}
				}
				lnx++;
			}
		}
	}
	else if(pCurBody->m_LineType==XFLR5::BODYSPLINETYPE)
	{
		FullSize = 2*nx*nh;
		//start with left side... same as for wings
		for (k=0; k<nx; k++)
		{
			uk  = pCurBody->m_XPanelPos[k];
			uk1 = pCurBody->m_XPanelPos[k+1];

			pCurBody->getPoint(uk,  0, false, LB);
			pCurBody->getPoint(uk1, 0, false, TB);

			LB.x += dpx;
			LB.z += dpz;
			TB.x += dpx;
			TB.z += dpz;

			for (l=0; l<nh; l++)
			{
				//start with left side... same as for wings
				v = (double)(l+1) / (double)(nh);
				pCurBody->getPoint(uk,  v, false, LA);
				pCurBody->getPoint(uk1, v, false, TA);

				LA.x += dpx;
				LA.z += dpz;
				TA.x += dpx;
				TA.z += dpz;

				n0 = isNode(LA);
				n1 = isNode(TA);
				n2 = isNode(LB);
				n3 = isNode(TB);

				if(n0>=0) {
					m_Panel[m_MatSize].m_iLA = n0;
				}
				else {
					m_Panel[m_MatSize].m_iLA = m_nNodes;
					m_Node[m_nNodes].copy(LA);
					m_nNodes++;
				}

				if(n1>=0) {
					m_Panel[m_MatSize].m_iTA = n1;
				}
				else {
					m_Panel[m_MatSize].m_iTA = m_nNodes;
					m_Node[m_nNodes].copy(TA);
					m_nNodes++;
				}

				if(n2>=0) {
					m_Panel[m_MatSize].m_iLB = n2;
				}
				else {
					m_Panel[m_MatSize].m_iLB = m_nNodes;
					m_Node[m_nNodes].copy(LB);
					m_nNodes++;
				}

				if(n3 >=0) {
					m_Panel[m_MatSize].m_iTB = n3;
				}
				else {
					m_Panel[m_MatSize].m_iTB = m_nNodes;
					m_Node[m_nNodes].copy(TB);
					m_nNodes++;
				}

				LATB = TB - LA;
				TALB = LB - TA;
				m_Panel[m_MatSize].Normal = LATB * TALB;
				m_Panel[m_MatSize].Area =  m_Panel[m_MatSize].Normal.VAbs()/2.0;
				m_Panel[m_MatSize].Normal.normalize();

				m_Panel[m_MatSize].m_bIsInSymPlane  = false;
				m_Panel[m_MatSize].m_bIsLeading     = false;
				m_Panel[m_MatSize].m_bIsTrailing    = false;
				m_Panel[m_MatSize].m_Pos = BODYSURFACE;
				m_Panel[m_MatSize].m_iElement = m_MatSize;
				m_Panel[m_MatSize].m_bIsLeftPanel  = true;
				m_Panel[m_MatSize].setPanelFrame(LA, LB, TA, TB);

				// set neighbour panels

				m_Panel[m_MatSize].m_iPD = m_MatSize + nh;
				m_Panel[m_MatSize].m_iPU = m_MatSize - nh;

				if(k==0)    m_Panel[m_MatSize].m_iPU = -1;// no panel downstream
				if(k==nx-1)	m_Panel[m_MatSize].m_iPD = -1;// no panel upstream

				m_Panel[m_MatSize].m_iPL = m_MatSize + 1;
				m_Panel[m_MatSize].m_iPR = m_MatSize - 1;

				if(l==0)     m_Panel[m_MatSize].m_iPR = InitialSize + FullSize - p - 1;
				if(l==nh-1)  m_Panel[m_MatSize].m_iPL = InitialSize + FullSize - p - 1;

				m_MatSize++;
				p++;
				LB = LA;
				TB = TA;
			}
		}
	}

	//right side next
	i = m_MatSize;

	for (k=nx-1; k>=0; k--)
	{
		for (l=nh-1; l>=0; l--)
		{
			i--;
			LA = m_Node[m_Panel[i].m_iLB];
			TA = m_Node[m_Panel[i].m_iTB];
			LB = m_Node[m_Panel[i].m_iLA];
			TB = m_Node[m_Panel[i].m_iTA];

			LA.y = -LA.y;
			LB.y = -LB.y;
			TA.y = -TA.y;
			TB.y = -TB.y;

			n0 = isNode(LA);
			n1 = isNode(TA);
			n2 = isNode(LB);
			n3 = isNode(TB);

			if(n0>=0) {
				m_Panel[m_MatSize].m_iLA = n0;
			}
			else {
				m_Panel[m_MatSize].m_iLA = m_nNodes;
				m_Node[m_nNodes].copy(LA);
				m_nNodes++;
			}

			if(n1>=0) {
				m_Panel[m_MatSize].m_iTA = n1;
			}
			else {
				m_Panel[m_MatSize].m_iTA = m_nNodes;
				m_Node[m_nNodes].copy(TA);
				m_nNodes++;
			}

			if(n2>=0) {
				m_Panel[m_MatSize].m_iLB = n2;
			}
			else {
				m_Panel[m_MatSize].m_iLB = m_nNodes;
				m_Node[m_nNodes].copy(LB);
				m_nNodes++;
			}

			if(n3 >=0) {
				m_Panel[m_MatSize].m_iTB = n3;
			}
			else {
				m_Panel[m_MatSize].m_iTB = m_nNodes;
				m_Node[m_nNodes].copy(TB);
				m_nNodes++;
			}

			LATB = TB - LA;
			TALB = LB - TA;
			m_Panel[m_MatSize].Normal = LATB * TALB;
			m_Panel[m_MatSize].Area =  m_Panel[m_MatSize].Normal.VAbs()/2.0;
			m_Panel[m_MatSize].Normal.normalize();

			m_Panel[m_MatSize].m_bIsInSymPlane  = false;
			m_Panel[m_MatSize].m_bIsLeading     = false;
			m_Panel[m_MatSize].m_bIsTrailing    = false;
			m_Panel[m_MatSize].m_Pos = BODYSURFACE;
			m_Panel[m_MatSize].m_iElement = m_MatSize;
			m_Panel[m_MatSize].m_bIsLeftPanel  = false;
			m_Panel[m_MatSize].setPanelFrame(LA, LB, TA, TB);

			// set neighbour panels
			// valid only for Panel Analysis

			m_Panel[m_MatSize].m_iPD = m_MatSize - nh;
			m_Panel[m_MatSize].m_iPU = m_MatSize + nh;

			if(k==0)	m_Panel[m_MatSize].m_iPU = -1;// no panel downstream
			if(k==nx-1)	m_Panel[m_MatSize].m_iPD = -1;// no panel upstream

			m_Panel[m_MatSize].m_iPL = m_MatSize + 1;
			m_Panel[m_MatSize].m_iPR = m_MatSize - 1;

			if(l==0)     m_Panel[m_MatSize].m_iPL = InitialSize + FullSize - p - 1;
			if(l==nh-1)  m_Panel[m_MatSize].m_iPR = InitialSize + FullSize - p - 1;

			m_MatSize++;
			p++;
			LB = LA;
			TB = TA;
		}
	}
	pCurBody->m_NElements = m_MatSize-InitialSize;
	return pCurBody->m_NElements;
}


/**
* Creates the panel elements for the active surface.
* The panels are appended at the end of the existing array of panel
* The panels are created from left to right on a surface
* The panels are created depending on the current WPolar:
*   No WPolar --> panel elements on top & bottom surfaces, just for cosmetics
*   VLM       --> panel elements on mid camber line from T.E. to L.E.
*   3D Panels --> panel elements on left tip surface
*                 panel elements on each strip, starting on the bottom T.E. to the L.E back to the opt T.E
*                 panel elements on right tip surface
*
*@param a pointer to the surface for which the panels will be created
*@return the number of panels which have been created and appended
*/
int PlaneAnalysisTask::createWingElements(Plane *pPlane, WPolar *pWPolar, Surface *pSurface)
{
	//TODO : for  a gap at the wing's center, need to separate m_iPL and m_iPR at the tips;
	bool bNoJoinFlap;
	int k,l;
	int n0, n1, n2, n3;

	int InitialSize = m_MatSize;
	enumPanelPosition side;
	CVector LA, LB, TA, TB;

	bool bThickSurfaces = true;
	if(!pPlane->isWing()) bThickSurfaces= false;
	if(pWPolar)
	{
		if(pWPolar->analysisMethod() == XFLR5::LLTMETHOD) bThickSurfaces = false;
		if(pWPolar->analysisMethod() == XFLR5::VLMMETHOD) bThickSurfaces = false;
		if(pWPolar->bThinSurfaces()) bThickSurfaces = false;
	}

	if (bThickSurfaces && pWPolar && pSurface->isTipLeft())
	{
		//then left tip surface, add side panels
		for (l=0; l<pSurface->NXPanels(); l++)
		{
			m_Panel[m_MatSize].m_bIsLeading     = false;
			m_Panel[m_MatSize].m_bIsTrailing    = false;
			m_Panel[m_MatSize].m_bIsWakePanel   = false;
			m_Panel[m_MatSize].m_bIsInSymPlane  = false; //even if part of a fin

			pSurface->getPanel(0, l, BOTSURFACE);
			LA.copy(pSurface->LA);
			TA.copy(pSurface->TA);

			pSurface->getPanel(0, l, TOPSURFACE);
			LB.copy(pSurface->LA);
			TB.copy(pSurface->TA);

			n0 = isNode(LA);
			if(n0>=0) 	m_Panel[m_MatSize].m_iLA = n0;
			else {
				m_Panel[m_MatSize].m_iLA = m_nNodes;
				m_Node[m_nNodes].copy(LA);
				m_nNodes++;
			}

			n1 = isNode(TA);
			if(n1>=0) 	m_Panel[m_MatSize].m_iTA = n1;
			else {
				m_Panel[m_MatSize].m_iTA = m_nNodes;
				m_Node[m_nNodes].copy(TA);
				m_nNodes++;
			}

			n2 = isNode(LB);
			if(n2>=0) 	m_Panel[m_MatSize].m_iLB = n2;
			else {
				m_Panel[m_MatSize].m_iLB = m_nNodes;
				m_Node[m_nNodes].copy(LB);
				m_nNodes++;
			}

			n3 = isNode(TB);
			if(n3>=0) 	m_Panel[m_MatSize].m_iTB = n3;
			else {
				m_Panel[m_MatSize].m_iTB = m_nNodes;
				m_Node[m_nNodes].copy(TB);
				m_nNodes++;
			}

			m_Panel[m_MatSize].m_Pos = SIDESURFACE;
			m_Panel[m_MatSize].m_iElement = m_MatSize;
			m_Panel[m_MatSize].m_bIsLeftPanel  = pSurface->isLeftSurf();
			m_Panel[m_MatSize].setPanelFrame(LA, LB, TA, TB);
			m_Panel[m_MatSize].m_iWake = -1;

			m_Panel[m_MatSize].m_iPD = m_MatSize-1;
			m_Panel[m_MatSize].m_iPU = m_MatSize+1;
			if(l==0)                      m_Panel[m_MatSize].m_iPD = -1;// no panel downstream
			if(l==pSurface->NXPanels()-1) m_Panel[m_MatSize].m_iPU = -1;// no panel upstream
			m_Panel[m_MatSize].m_iPL = -1;
			m_Panel[m_MatSize].m_iPR = -1;

			m_MatSize++;
		}
	}

	for (k=0; k<pSurface->NYPanels(); k++)
	{
		//add "horizontal" panels, mid side, or following a strip from bot to top if 3D Panel
		if(bThickSurfaces)   side = BOTSURFACE;  //start with lower surf, as recommended by K&P
		else                 side = MIDSURFACE;
		//from T.E. to L.E.
		for (l=0; l<pSurface->NXPanels(); l++)
		{
			pSurface->getPanel(k,l,side);

			n0 = isNode(pSurface->LA);
			n1 = isNode(pSurface->TA);
			n2 = isNode(pSurface->LB);
			n3 = isNode(pSurface->TB);

			if(l==0)                      m_Panel[m_MatSize].m_bIsTrailing = true;
			if(l==pSurface->NXPanels()-1) m_Panel[m_MatSize].m_bIsLeading  = true;
			m_Panel[m_MatSize].m_bIsWakePanel   = false;
			m_Panel[m_MatSize].m_bIsInSymPlane  = pSurface->isInSymPlane();

			bNoJoinFlap = side==0 &&  l<pSurface->NXFlap() && k==0;

			if(n0>=0 && !bNoJoinFlap) // do not merge nodes if we are at a flap's side in VLM
			{
				m_Panel[m_MatSize].m_iLA = n0;
			}
			else {
				m_Panel[m_MatSize].m_iLA = m_nNodes;
				m_Node[m_nNodes].copy(pSurface->LA);
				m_nNodes++;
			}

			if(n1>=0 && !bNoJoinFlap) // do not merge nodes if we are at a flap's side in VLM
			{
				m_Panel[m_MatSize].m_iTA = n1;
			}
			else {
				m_Panel[m_MatSize].m_iTA = m_nNodes;
				m_Node[m_nNodes].copy(pSurface->TA);
				m_nNodes++;
			}

			bNoJoinFlap = side==0 &&  l<pSurface->NXFlap() && k==pSurface->NYPanels()-1;

			if(n2>=0 && !bNoJoinFlap) // do not merge nodes if we are at a flap's side in VLM
			{
				m_Panel[m_MatSize].m_iLB = n2;
			}
			else {
				m_Panel[m_MatSize].m_iLB = m_nNodes;
				m_Node[m_nNodes].copy(pSurface->LB);
				m_nNodes++;
			}

			if(n3>=0 && !bNoJoinFlap) // do not merge nodes if we are at a flap's side in VLM
			{
				m_Panel[m_MatSize].m_iTB = n3;
			}
			else {
				m_Panel[m_MatSize].m_iTB = m_nNodes;
				m_Node[m_nNodes].copy(pSurface->TB);
				m_nNodes++;
			}

			m_Panel[m_MatSize].m_Pos = side;
			m_Panel[m_MatSize].m_iElement = m_MatSize;
			m_Panel[m_MatSize].m_bIsLeftPanel  = pSurface->isLeftSurf();

			if(side==MIDSURFACE)        m_Panel[m_MatSize].setPanelFrame(pSurface->LA, pSurface->LB, pSurface->TA, pSurface->TB);
			else if (side==BOTSURFACE)  m_Panel[m_MatSize].setPanelFrame(pSurface->LB, pSurface->LA, pSurface->TB, pSurface->TA);

			// set neighbour panels
			// valid only for Panel 2-sided Analysis
			// we are on the bottom or middle surface
			m_Panel[m_MatSize].m_iPD = m_MatSize-1;
			m_Panel[m_MatSize].m_iPU = m_MatSize+1;
			if(l==0)                                         m_Panel[m_MatSize].m_iPD = -1;// no panel downstream
			if(l==pSurface->NXPanels()-1 && side==MIDSURFACE) m_Panel[m_MatSize].m_iPU = -1;// no panel upstream

			if(side!=MIDSURFACE)
			{
				//wings are modelled as thick surfaces
				m_Panel[m_MatSize].m_iPL = m_MatSize + 2*pSurface->NXPanels();
				m_Panel[m_MatSize].m_iPR = m_MatSize - 2*pSurface->NXPanels();
				//todo : do not link to right wing if there is a gap in between
				if(k==0                      && pSurface->isTipLeft())  m_Panel[m_MatSize].m_iPR = -1;
				if(k==pSurface->NYPanels()-1 && pSurface->isTipRight()) m_Panel[m_MatSize].m_iPL = -1;
			}
			else
			{
				//wings are modelled as thin surfaces
				m_Panel[m_MatSize].m_iPR = m_MatSize + pSurface->NXPanels();
				m_Panel[m_MatSize].m_iPL = m_MatSize - pSurface->NXPanels();
				if(k==0                      && pSurface->isTipLeft())  m_Panel[m_MatSize].m_iPL = -1;
				if(k==pSurface->NYPanels()-1 && pSurface->isTipRight()) m_Panel[m_MatSize].m_iPR = -1;
			}

			//do not link to next surfaces... will be done in JoinSurfaces() if surfaces are continuous
			if(k==0)                      m_Panel[m_MatSize].m_iPR = -1;
			if(k==pSurface->NYPanels()-1) m_Panel[m_MatSize].m_iPL = -1;

			if(pWPolar && m_Panel[m_MatSize].m_bIsTrailing && pWPolar->analysisMethod()==XFLR5::PANELMETHOD)
			{
				m_Panel[m_MatSize].m_iWake = m_WakeSize;//next wake element
				m_Panel[m_MatSize].m_iWakeColumn = m_NWakeColumn;
				if(pWPolar->bThinSurfaces())
				{
					createWakeElems(m_MatSize, pPlane, pWPolar);
					m_NWakeColumn++;
				}
			}

			if(l<pSurface->NXFlap()) pSurface->addFlapPanel(m_Panel+m_MatSize);

			m_MatSize++;
		}

		if (bThickSurfaces)
		{
			//add top side if 3D Panels
			side = TOPSURFACE; //next upper surf, as recommended by K&P
			//from L.E. to T.E.
			for (l=pSurface->NXPanels()-1;l>=0; l--)
			{
				pSurface->getPanel(k,l,side);
				n0 = isNode(pSurface->LA);
				n1 = isNode(pSurface->TA);
				n2 = isNode(pSurface->LB);
				n3 = isNode(pSurface->TB);

				if(l==0)                      m_Panel[m_MatSize].m_bIsTrailing = true;
				if(l==pSurface->NXPanels()-1) m_Panel[m_MatSize].m_bIsLeading  = true;
				m_Panel[m_MatSize].m_bIsWakePanel   = false;
				m_Panel[m_MatSize].m_bIsInSymPlane  = pSurface->isInSymPlane();

				if(n0>=0)
					m_Panel[m_MatSize].m_iLA = n0;
				else {
					m_Panel[m_MatSize].m_iLA = m_nNodes;
					m_Node[m_nNodes].copy(pSurface->LA);
					m_nNodes++;
				}

				if(n1>=0)
					m_Panel[m_MatSize].m_iTA = n1;
				else {
					m_Panel[m_MatSize].m_iTA = m_nNodes;
					m_Node[m_nNodes].copy(pSurface->TA);
					m_nNodes++;
				}

				if(n2>=0)
					m_Panel[m_MatSize].m_iLB = n2;
				else {
					m_Panel[m_MatSize].m_iLB = m_nNodes;
					m_Node[m_nNodes].copy(pSurface->LB);
					m_nNodes++;
				}

				if(n3 >=0)
					m_Panel[m_MatSize].m_iTB = n3;
				else {
					m_Panel[m_MatSize].m_iTB = m_nNodes;
					m_Node[m_nNodes].copy(pSurface->TB);
					m_nNodes++;
				}

				m_Panel[m_MatSize].m_Pos = side;
				m_Panel[m_MatSize].m_iElement = m_MatSize;
				m_Panel[m_MatSize].m_bIsLeftPanel  = pSurface->isLeftSurf();

				m_Panel[m_MatSize].setPanelFrame(pSurface->LA, pSurface->LB, pSurface->TA, pSurface->TB);

				// set neighbour panels
				// valid only for Panel 2-sided Analysis
				// we are on the top surface
				m_Panel[m_MatSize].m_iPD = m_MatSize+1;
				m_Panel[m_MatSize].m_iPU = m_MatSize-1;
				if(l==0)                      m_Panel[m_MatSize].m_iPD = -1;// no panel downstream
//				if(l==pSurface->NXPanels()-1) m_Panel[m_MatSize].m_iPU = -1;// no panel upstream

				m_Panel[m_MatSize].m_iPL = m_MatSize - 2*pSurface->NXPanels();//assuming all wing panels have same chordwise distribution
				m_Panel[m_MatSize].m_iPR = m_MatSize + 2*pSurface->NXPanels();//assuming all wing panels have same chordwise distribution

				if(k==0                      && pSurface->isTipLeft())	m_Panel[m_MatSize].m_iPL = -1;
				if(k==pSurface->NYPanels()-1 && pSurface->isTipRight())	m_Panel[m_MatSize].m_iPR = -1;

				//do not link to next surfaces... will be done in JoinSurfaces() if surfaces are continuous
				if(k==0)                      m_Panel[m_MatSize].m_iPL = -1;
				if(k==pSurface->NYPanels()-1) m_Panel[m_MatSize].m_iPR = -1;


				if(pWPolar && m_Panel[m_MatSize].m_bIsTrailing && pWPolar->analysisMethod()==XFLR5::PANELMETHOD)
				{
					m_Panel[m_MatSize].m_iWake = m_WakeSize;//next wake element
					m_Panel[m_MatSize].m_iWakeColumn = m_NWakeColumn;
					createWakeElems(m_MatSize, pPlane, pWPolar);
				}

				if(l<pSurface->NXFlap()) pSurface->addFlapPanel(m_Panel+m_MatSize);
				m_MatSize++;
			}
			m_NWakeColumn++;
		}
	}

	if (bThickSurfaces && pWPolar && pSurface->isTipRight())
	{	//right tip surface
		k = pSurface->NYPanels()-1;
		for (l=0; l< pSurface->NXPanels(); l++)
		{
			m_Panel[m_MatSize].m_bIsTrailing    = false;
			m_Panel[m_MatSize].m_bIsLeading     = false;
			m_Panel[m_MatSize].m_bIsWakePanel   = false;
			m_Panel[m_MatSize].m_bIsInSymPlane  = false;//even if part of a fin

			pSurface->getPanel(k,l,TOPSURFACE);
			LA.copy(pSurface->LB);
			TA.copy(pSurface->TB);

			pSurface->getPanel(k,l,BOTSURFACE);
			LB.copy(pSurface->LB);
			TB.copy(pSurface->TB);

			n0 = isNode(LA);//answer should be yes
			if(n0>=0) 				m_Panel[m_MatSize].m_iLA = n0;
			else {
				m_Panel[m_MatSize].m_iLA = m_nNodes;
				m_Node[m_nNodes].copy(LA);
				m_nNodes++;
			}
			n1 = isNode(TA);//answer should be yes
			if(n1>=0) 				m_Panel[m_MatSize].m_iTA = n1;
			else {
				m_Panel[m_MatSize].m_iTA = m_nNodes;
				m_Node[m_nNodes].copy(TA);
				m_nNodes++;
			}
			n2 = isNode(LB);//answer should be yes
			if(n2>=0) 				m_Panel[m_MatSize].m_iLB = n2;
			else {
				m_Panel[m_MatSize].m_iLB = m_nNodes;
				m_Node[m_nNodes].copy(LB);
				m_nNodes++;
			}
			n3 = isNode(TB);//answer should be yes
			if(n3>=0)				m_Panel[m_MatSize].m_iTB = n3;
			else {
				m_Panel[m_MatSize].m_iTB = m_nNodes;
				m_Node[m_nNodes].copy(TB);
				m_nNodes++;
			}


			m_Panel[m_MatSize].m_iPD = m_MatSize-1;
			m_Panel[m_MatSize].m_iPU = m_MatSize+1;
			if(l==0)                      m_Panel[m_MatSize].m_iPD = -1;// no panel downstream
			if(l==pSurface->NXPanels()-1) m_Panel[m_MatSize].m_iPU = -1;// no panel upstream
			m_Panel[m_MatSize].m_iPL = -1;
			m_Panel[m_MatSize].m_iPR = -1;

			m_Panel[m_MatSize].m_Pos = SIDESURFACE;
			m_Panel[m_MatSize].m_iElement = m_MatSize;
			m_Panel[m_MatSize].m_bIsLeftPanel  = pSurface->isLeftSurf();
			m_Panel[m_MatSize].setPanelFrame(LA, LB, TA, TB);
			m_Panel[m_MatSize].m_iWake = -1;
			m_MatSize++;
		}
	}

	pSurface->NElements() = m_MatSize-InitialSize;
	return pSurface->NElements();
}


/**
* Creates a column of wake elements shed from a panel at the trailing edge of the wing's surface
* @param PanelIndex the index of the panel on the trailing edge of the surface which will shed the column of wake panels
*/
bool PlaneAnalysisTask::createWakeElems(int PanelIndex, Plane *pPlane, WPolar* pWPolar)
{
	if(!pWPolar) return false;
	if(!m_Panel[PanelIndex].m_bIsTrailing) return false;
	//creates elements trailing panel p
	int l, n0, n1,n2, n3;
	double dxA, dxB;
	int mw = m_WakeSize;// number of wake panels
	CVector LATB, TALB;
	CVector LA, LB, TA,TB;//wake panel's corner points

	int NXWakePanels;
	double WakePanelFactor;
	double TotalWakeLength;

	NXWakePanels    = pWPolar->m_NXWakePanels;
	WakePanelFactor = pWPolar->m_WakePanelFactor;
	TotalWakeLength = pWPolar->m_TotalWakeLength;

	TA = m_Node[m_Panel[PanelIndex].m_iTA];
	TB = m_Node[m_Panel[PanelIndex].m_iTB];
	dxA = TotalWakeLength*pPlane->mac() - m_Node[m_Panel[PanelIndex].m_iTA].x;
	dxB = TotalWakeLength*pPlane->mac() - m_Node[m_Panel[PanelIndex].m_iTB].x;

	if(WakePanelFactor==1.0)
	{
		dxA /= NXWakePanels;
		dxB /= NXWakePanels;
	}
	else
	{
		dxA *= (1.0-WakePanelFactor)/(1.0-pow(WakePanelFactor, NXWakePanels));
		dxB *= (1.0-WakePanelFactor)/(1.0-pow(WakePanelFactor, NXWakePanels));
	}

	for (l=0; l<NXWakePanels; l++)
	{
		LA = TA;
		LB = TB;
		TA.x += dxA;
		TB.x += dxB;
//		TA += TE * dxA;
//		TB += TE * dxB;
		dxA *= WakePanelFactor;
		dxB *= WakePanelFactor;

		n0 = isWakeNode(LA);
		n1 = isWakeNode(TA);
		n2 = isWakeNode(LB);
		n3 = isWakeNode(TB);

		if(n0>=0) {
			m_WakePanel[mw].m_iLA = n0;
		}
		else {
			m_WakePanel[mw].m_iLA = m_nWakeNodes;
			m_WakeNode[m_nWakeNodes].copy(LA);
			m_nWakeNodes++;
		}

		if(n1>=0) {
			m_WakePanel[mw].m_iTA = n1;
		}
		else {
			m_WakePanel[mw].m_iTA = m_nWakeNodes;
			m_WakeNode[m_nWakeNodes].copy(TA);
			m_nWakeNodes++;
		}

		if(n2>=0) {
			m_WakePanel[mw].m_iLB = n2;
		}
		else {
			m_WakePanel[mw].m_iLB = m_nWakeNodes;
			m_WakeNode[m_nWakeNodes].copy(LB);
			m_nWakeNodes++;
		}

		if(n3 >=0) {
			m_WakePanel[mw].m_iTB = n3;
		}
		else {
			m_WakePanel[mw].m_iTB = m_nWakeNodes;
			m_WakeNode[m_nWakeNodes].copy(TB);
			m_nWakeNodes++;
		}

		LATB = TB - LA;
		TALB = LB - TA;

		m_WakePanel[mw].Normal = LATB * TALB;

		m_WakePanel[mw].m_Pos = MIDSURFACE;
		m_WakePanel[mw].m_bIsWakePanel = true;
		m_WakePanel[mw].Area =  m_WakePanel[mw].Normal.VAbs()/2.0;
		m_WakePanel[mw].Normal.normalize();
		m_WakePanel[mw].setPanelFrame(LA,LB, TA, TB);
		m_WakePanel[mw].m_bIsLeftPanel  = false;

		if(l==0)					m_WakePanel[mw].m_iPD = -1;// no panel downstream
		else if(l==NXWakePanels)	m_WakePanel[mw].m_iPU = -1;// no panel upstream
		else						m_WakePanel[mw].m_iPD = mw+1;

		m_WakePanel[mw].m_iPL = -1;
		m_WakePanel[mw].m_iPR = -1;

		mw++;
	}

	m_WakeSize = mw;
	return true;
}


int PlaneAnalysisTask::calculateMatSize(Plane *pPlane, WPolar *pWPolar)
{
	int nx, nh;
	int PanelArraySize = 0;

	if(!pPlane) return 0;

	//Count the wing panels
	for (int js=0; js<m_SurfaceList.size(); js++)
	{
		PanelArraySize += m_SurfaceList.at(js)->NXPanels() * m_SurfaceList.at(js)->NYPanels();
	}

	if(pPlane->isWing() && (!pWPolar || !pWPolar->bThinSurfaces()))
	{
		PanelArraySize *= 2;
		for (int js=0; js<m_SurfaceList.size(); js++)
		{
			if(m_SurfaceList.at(js)->isTipLeft() || m_SurfaceList.at(js)->isTipRight())
				PanelArraySize += m_SurfaceList.at(js)->NXPanels();//tip patches
		}
	}

	// add the number of body panels
	//create the body elements only if there is a body, and the analysis is not of the VLM Type
	if(pPlane && pPlane->body())
	{
		Body *pCurBody = pPlane->body();

		if(pWPolar && pWPolar->analysisMethod()==XFLR5::PANELMETHOD && pWPolar->bIgnoreBodyPanels())
		{
		}
		else
		{
			if(pCurBody->m_LineType==XFLR5::BODYPANELTYPE)
			{
				nx = 0;
				for(int i=0; i<pCurBody->frameCount()-1; i++) nx+=pCurBody->m_xPanels[i];
				nh = 0;
				for(int i=0; i<pCurBody->sideLineCount()-1; i++) nh+=pCurBody->m_hPanels[i];
				PanelArraySize += nx*nh*2;
			}
			else PanelArraySize += 2 * pCurBody->nxPanels() * pCurBody->nhPanels();
		}
	}
	return PanelArraySize;
}





/**
 * Releases the memory allocated to the Panel and node arrays.
 * Sets the pointers to NULL and the matrixsize to 0.
 */
void PlaneAnalysisTask::releasePanelMemory()
{
	if(m_Node)        delete[] m_Node;
	if(m_MemNode)     delete[] m_MemNode;
	if(m_WakeNode)    delete[] m_WakeNode;
	if(m_RefWakeNode) delete[] m_RefWakeNode;
	m_Node = m_MemNode = m_WakeNode = m_RefWakeNode = NULL;

	if(m_Panel)        delete[] m_Panel;
	if(m_MemPanel)     delete[] m_MemPanel;
	if(m_WakePanel)    delete[] m_WakePanel;
	if(m_RefWakePanel) delete[] m_RefWakePanel;
	m_Panel = m_MemPanel = m_WakePanel = m_RefWakePanel = NULL;

	m_MatSize = 0;
	m_nNodes = 0;
}



/**
 * Checks if the input point is close to a wake node within the tolerances set in the CVector class
 * Returns the index of a node if found, else returns -1
 *@param Pt : the point to identify
 *@return the index of the node with coordinates equal to the input Pt
*/
int PlaneAnalysisTask::isWakeNode(CVector &Pt)
{
	int in;
	for (in=0; in<m_nWakeNodes; in++)
	{
		if(Pt.isSame(m_WakeNode[in])) return in;
	}
	return -1;
}


/**
 * Checks if the input point is close to a mesh node within the tolerances set in the CVector class
 * Returns the index of a node if found, else returns -1
 *@param Pt : the point to identify
 *@return the index of the node with coordinates equal to the input Pt
*/
int PlaneAnalysisTask::isNode(CVector &Pt)
{
	int in;
	for (in=m_nNodes-1; in>=0; in--)
	{
		if(Pt.isSame(m_Node[in])) return in;
	}
	return -1;
}





/**
 * Reserves the memory necessary to all the arrays used in a Panel analysis.
 *@return true if the memory could be allocated, false otherwise.
 */
bool PlaneAnalysisTask::allocatePanelArrays(int &memsize)
{
//	Trace(QString("QMiarex::Allocating() %1 Panels").arg(m_MaxPanelSize));

	try
	{
		m_Node        = new CVector[2*m_MaxPanelSize];
		m_MemNode     = new CVector[2*m_MaxPanelSize];
		m_WakeNode    = new CVector[2*m_MaxPanelSize];
		m_RefWakeNode = new CVector[2*m_MaxPanelSize];

		m_Panel        = new Panel[m_MaxPanelSize];
		m_MemPanel     = new Panel[m_MaxPanelSize];
		m_WakePanel    = new Panel[m_MaxPanelSize];
		m_RefWakePanel = new Panel[m_MaxPanelSize];
	}
	catch(std::exception & e)
	{
		releasePanelMemory();
		m_MaxPanelSize = 0;

		Trace(e.what());
		QString strange = "Memory allocation error: the request for additional memory has been denied.\nPlease reduce the model's size.";
		Trace(strange);
		return false;
	}

	memsize  = sizeof(CVector) * 8 * 2 * m_MaxPanelSize; //bytes
	memsize += sizeof(Panel)   * 8 * 2 * m_MaxPanelSize; //bytes

//	Trace(QString("Objects3D::   ...Allocated %1MB for the panel and node arrays").arg((double)memsize/1024./1024.));

	Panel::s_pNode = m_Node;
	Panel::s_pWakeNode = m_WakeNode;

	Surface::setPanelPointers(m_Panel, m_Node);

//	QMiarex::s_pPanel = m_Panel;
//	QMiarex::s_pNode = m_Node;

	return true;
}




/**
 * Rotates all the panels about the z-axis
 * @param pPanel a pointer to the array of surface mesh panels
 * @param pNode  a pointer to the array of surface panel nodes
 * @param pWakePanel a pointer to the array of wake mesh panels
 * @param pWakeNode  a pointer to the array of wake panel nodes
 * @param beta the rotation angle in degrees
 * @param P the point of origin of the rotation
 */
void PlaneAnalysisTask::rotateGeomZ(Panel *pPanel, CVector *pNode, Panel *pWakePanel, CVector *pWakeNode,
							int nPanels, int nWakePanels, int nNodes, int/* nWakeNodes*/, int NWakeColumn,
							double const &Beta, CVector const &P, int NXWakePanels)
{
	int n, p, pw, kw, lw;
	int iLA, iLB, iTA, iTB;
	CVector Pt, Trans;

	for (n=0; n<nNodes; n++)	pNode[n].rotateZ(P, Beta);

	for (p=0; p<nPanels; p++)
	{
		// get the index of the panel's four corner points
		iLA = pPanel[p].m_iLA; iLB = pPanel[p].m_iLB;
		iTA = pPanel[p].m_iTA; iTB = pPanel[p].m_iTB;

		//set the new panel geometry
		if(pPanel[p].m_Pos>=MIDSURFACE)       pPanel[p].setPanelFrame(pNode[iLA], pNode[iLB], pNode[iTA], pNode[iTB]);
		else if (pPanel[p].m_Pos==BOTSURFACE) pPanel[p].setPanelFrame(pNode[iLB], pNode[iLA], pNode[iTB], pNode[iTA]);
	}

	// the wake array is not rotated but translated to remain at the wing's trailing edge and aligned with the freestream velocity vector
	pw=0;

	for (kw=0; kw<NWakeColumn; kw++)
	{
		//consider the first panel of the column;
		Pt = pWakeNode[pWakePanel[pw].m_iLA];
		Pt.rotateZ(P, Beta);
		//define the translation to be applied to the column's left points
		Trans = Pt-pWakeNode[pWakePanel[pw].m_iLA] ;

		//each wake column has m_NXWakePanels ... translate all left A nodes
		for(lw=0; lw<NXWakePanels; lw++)
		{
			if(lw==0) pWakeNode[pWakePanel[pw].m_iLA] += Trans;
			pWakeNode[pWakePanel[pw].m_iTA] += Trans;
			pw++;
		}
	}

	//last column, process B side of the column
	pw -= NXWakePanels;
	//consider the first panel of the column;
	Pt = pWakeNode[pWakePanel[pw].m_iLB];
	Pt.rotateZ(P, Beta);
	//define the translation to be applied to the column's left points
	Trans = Pt - pWakeNode[pWakePanel[pw].m_iLB];

	//each wake column has m_NXWakePanels ... translate all left A nodes
	for(lw=0; lw<NXWakePanels; lw++)
	{
		if(lw==0) pWakeNode[pWakePanel[pw].m_iLB] += Trans;
		pWakeNode[pWakePanel[pw].m_iTB] += Trans;
		pw++;
	}

	//Reset panel frame : CollPt has been translated
	for (pw=0; pw< nWakePanels; pw++)
	{
		// get the index of the panel's four corner points
		iLA = pWakePanel[pw].m_iLA; iLB = pWakePanel[pw].m_iLB;
		iTA = pWakePanel[pw].m_iTA; iTB = pWakePanel[pw].m_iTB;

		//set the new panel geometry
		pWakePanel[pw].setPanelFrame(pWakeNode[iLA], pWakeNode[iLB], pWakeNode[iTA], pWakeNode[iTB]);
	}
}



