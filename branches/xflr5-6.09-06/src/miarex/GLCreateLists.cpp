/****************************************************************************

	GLCreateLists 
	Copyright (C) 2010-2010 Andre Deperrois adeperrois@xflr5.com

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


#include "../globals.h"
#include "../mainframe.h"
#include "GLCreateLists.h"
#include "Miarex.h"
#include "GL3DScales.h"
#include "StabViewDlg.h"
#include "../objects/Wing.h"
#include <QProgressDialog>


#define SIDEPOINTS 51

void GLCreateGeom(void *pQMiarex, Wing *pWing, int List, Body *pBody)
{
	if(!pWing) return;
	QMiarex * pMiarex = (QMiarex*)pQMiarex;

	static int j, l ;
	static double x, xDistrib[SIDEPOINTS];
	static CVector Pt, PtA, PtB, PtNormal, A, B, C, D, N, BD, AC;
	static CVector PtILeft[2*SIDEPOINTS],PtIRight[2*SIDEPOINTS];
	static Foil * pFoilA, *pFoilB;

	N.Set(0.0, 0.0, 0.0);

	double xx;
	for(int i=0; i<SIDEPOINTS; i++)
	{
		xx = (double)i/(double)(SIDEPOINTS-1);
        xDistrib[i] = 1.0/2.0*(1.0-cos(xx*PI));
	}

	glNewList(List, GL_COMPILE);
	{
		pMiarex->m_GLList++;

		if(MainFrame::s_bAlphaChannel)
		{
			glColor4d(pWing->m_WingColor.redF(),pWing->m_WingColor.greenF(),pWing->m_WingColor.blueF(), pWing->m_WingColor.alphaF());
			glEnable (GL_BLEND);
			glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		else
		{
			glColor3d(pWing->m_WingColor.redF(),pWing->m_WingColor.greenF(),pWing->m_WingColor.blueF());
			glDisable (GL_BLEND);
		}
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glPolygonOffset(1.0, 1.0);

		//top surface
		for (j=0; j<pWing->m_NSurfaces; j++)
		{
			glBegin(GL_QUAD_STRIP);
			{
				for (l=0; l<SIDEPOINTS; l++)
				{
					x = xDistrib[l];
					pWing->m_Surface[j].GetSurfacePointNormal(x,x,0.0,PtA, PtNormal,1);
					pWing->m_Surface[j].GetSurfacePointNormal(x,x,1.0,PtB, PtNormal,1);

					if(pBody && pWing->m_Surface[j].IsCenterSurf() && pWing->m_Surface[j].IsLeftSurf())
					{
						pBody->Intersect(PtA, PtB, PtB, false);
						PtIRight[l] = PtB;
					}
					else if(pBody && pWing->m_Surface[j].IsCenterSurf() && pWing->m_Surface[j].IsRightSurf())
					{
						pBody->Intersect(PtA, PtB, PtA, true);
						PtILeft[l] = PtA;
					}

					glNormal3d(PtNormal.x, PtNormal.y, PtNormal.z);
					glVertex3d(PtA.x, PtA.y, PtA.z);
					glVertex3d(PtB.x, PtB.y, PtB.z);
				}
			}
			glEnd();
		}

		//bottom surface
		for (j=0; j<pWing->m_NSurfaces; j++)
		{
			glBegin(GL_QUAD_STRIP);
			{
				for (l=0; l<SIDEPOINTS; l++)
				{
					x = xDistrib[l];
					pWing->m_Surface[j].GetSurfacePointNormal(x,x,0.0,PtA, PtNormal,-1);
					pWing->m_Surface[j].GetSurfacePointNormal(x,x,1.0,PtB, PtNormal,-1);

					if(pBody && pWing->m_Surface[j].IsCenterSurf() && pWing->m_Surface[j].IsLeftSurf())
					{
						pBody->Intersect(PtA, PtB, PtB, false);
						PtIRight[l+SIDEPOINTS] = PtB;
					}
					else if(pBody && pWing->m_Surface[j].IsCenterSurf() && pWing->m_Surface[j].IsRightSurf())
					{
						pBody->Intersect(PtA, PtB, PtA, true);
						PtILeft[l+SIDEPOINTS] = PtA;
					}
					glNormal3d(PtNormal.x, PtNormal.y, PtNormal.z);
					glVertex3d(PtA.x, PtA.y, PtA.z);
					glVertex3d(PtB.x, PtB.y, PtB.z);
				}
			}
			glEnd();
		}

		for (j=0; j<pWing->m_NSurfaces; j++)
		{
			//Tip left surface
			if(pWing->m_Surface[j].IsTipLeft())
			{
				glBegin(GL_QUAD_STRIP);
				{
					pWing->m_Surface[j].GetPanel(0, 0, BOTSURFACE);
					C. Copy(pWing->m_Surface[0].LA);
					D. Copy(pWing->m_Surface[0].TA);
					pWing->m_Surface[j].GetPanel(0, 0, TOPSURFACE);
					A. Copy(pWing->m_Surface[0].TA);
					B. Copy(pWing->m_Surface[0].LA);

					BD = D-B;
					AC = C-A;
					N  = AC*BD;
					N.Normalize();
					glNormal3d( N.x, N.y, N.z);

					for (l=0; l<SIDEPOINTS; l++)
					{
						x = xDistrib[l];
						pWing->m_Surface[j].GetSurfacePointNormal(x,x,0.0,Pt, PtNormal,1);

						glVertex3d(Pt.x, Pt.y, Pt.z);

						pWing->m_Surface[j].GetSurfacePointNormal(x,x,0.0,Pt, PtNormal,-1);
						glVertex3d(Pt.x, Pt.y, Pt.z);
					}
				}
				glEnd();
			}
			if (pWing->m_Surface[j].IsTipRight() &&
			   (!pWing->IsFin() || (pWing->IsFin()&&!pBody)) )
			{
				//Tip right surface
				glBegin(GL_QUAD_STRIP);
				{
					pWing->m_Surface[j].GetPanel(pWing->m_Surface[j].m_NYPanels-1,0, TOPSURFACE);
					A. Copy(pWing->m_Surface[0].TB);
					B. Copy(pWing->m_Surface[0].LB);
					pWing->m_Surface[j].GetPanel(pWing->m_Surface[j].m_NYPanels-1,0, BOTSURFACE);
					C. Copy(pWing->m_Surface[0].LB);
					D. Copy(pWing->m_Surface[0].TB);

					BD = D-B;
					AC = C-A;
					N  = BD * AC;
					N.Normalize();
					glNormal3d( N.x,  N.y,  N.z);

					for (l=0; l<SIDEPOINTS; l++)
					{
						x = xDistrib[l];
						pWing->m_Surface[j].GetSurfacePointNormal(x,x,1.0,Pt, PtNormal,1);

						glVertex3d(Pt.x, Pt.y, Pt.z);

						pWing->m_Surface[j].GetSurfacePointNormal(x,x,1.0,Pt, PtNormal,-1);
						glVertex3d(Pt.x, Pt.y, Pt.z);
					}
				}
				glEnd();
			}
		}
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_POLYGON_OFFSET_FILL);
		glDisable(GL_BLEND);
	}
	glEndList();

	//OUTLINE
	glNewList(List+4,GL_COMPILE);
	{
		pMiarex->m_GLList++;

//		glPolygonMode(GL_FRONT,GL_LINE);
		glEnable (GL_LINE_STIPPLE);

		if     (pMiarex->m_OutlineStyle == 1) glLineStipple (1, 0xCFCF);
		else if(pMiarex->m_OutlineStyle == 2) glLineStipple (1, 0x6666);
		else if(pMiarex->m_OutlineStyle == 3) glLineStipple (1, 0xFF18);
		else if(pMiarex->m_OutlineStyle == 4) glLineStipple (1, 0x7E66);
		else                                  glLineStipple (1, 0xFFFF);

		glColor3d(pMiarex->m_OutlineColor.redF(),pMiarex->m_OutlineColor.greenF(),pMiarex->m_OutlineColor.blueF());
		glLineWidth((GLfloat)pMiarex->m_OutlineWidth);

		//TOP outline
		for (j=0; j<pWing->m_NSurfaces; j++)
		{
			glBegin(GL_LINE_STRIP);
			{
				for (l=0; l<SIDEPOINTS; l++)
				{
					//top left surface outline
					if(pBody && pWing->m_Surface[j].IsCenterSurf() && pWing->m_Surface[j].IsRightSurf())
					{
//						pBody->Intersect(PtA, PtB, PtA, true);
						PtA = PtILeft[l];
					}
					else
					{
						x = xDistrib[l];
						pWing->m_Surface[j].GetSurfacePointNormal(x ,x ,0.0 ,PtA, PtNormal, 1);
						pWing->m_Surface[j].GetSurfacePointNormal(x, x, 1.0, PtB, PtNormal, 1);
					}
					glVertex3d(PtA.x, PtA.y, PtA.z);
				}
			}
			glEnd();

			glBegin(GL_LINE_STRIP);
			{
				for (l=0; l<SIDEPOINTS; l++)
				{
					//bottom left surface outline
					if(pBody && pWing->m_Surface[j].IsCenterSurf() && pWing->m_Surface[j].IsRightSurf())
					{
//						pBody->Intersect(PtA, PtB, PtA, true);
						PtA = PtILeft[l+SIDEPOINTS];
					}
					else
					{
						x = (double)l/(double)(SIDEPOINTS-1);
						pWing->m_Surface[j].GetSurfacePointNormal(x ,x ,0.0 ,PtA, PtNormal, -1);
						pWing->m_Surface[j].GetSurfacePointNormal(x, x, 1.0, PtB, PtNormal, -1);
					}

					glVertex3d(PtA.x, PtA.y, PtA.z);
				}
			}
			glEnd();

			glBegin(GL_LINE_STRIP);
			{
				//top right surface outline
				for (l=0; l<SIDEPOINTS; l++)
				{
					if(pBody && pWing->m_Surface[j].IsCenterSurf() && pWing->m_Surface[j].IsLeftSurf())
					{
//						pBody->Intersect(PtA, PtB, PtB, false);
						PtB = PtIRight[l];
					}
					else
					{
						x = xDistrib[l];
						pWing->m_Surface[j].GetSurfacePointNormal(x ,x ,0.0 ,PtA, PtNormal, 1);
						pWing->m_Surface[j].GetSurfacePointNormal(x, x, 1.0, PtB, PtNormal, 1);
					}

					glVertex3d(PtB.x, PtB.y, PtB.z);
				}
			}
			glEnd();
			glBegin(GL_LINE_STRIP);
			{
				//bottom right surface outline
				for (l=0; l<SIDEPOINTS; l++)
				{
					if(pBody && pWing->m_Surface[j].IsCenterSurf() && pWing->m_Surface[j].IsLeftSurf())
					{
//						pBody->Intersect(PtA, PtB, PtB, false);
						PtB = PtIRight[l+SIDEPOINTS];
					}
					else
					{
						x = xDistrib[l];
						pWing->m_Surface[j].GetSurfacePointNormal(x ,x ,0.0 ,PtA, PtNormal, -1);
						pWing->m_Surface[j].GetSurfacePointNormal(x, x, 1.0, PtB, PtNormal, -1);
					}

					glVertex3d(PtB.x, PtB.y, PtB.z);
				}
			}
			glEnd();
		}

		//WingContour
		//Leading edge outline
		for (j=0; j<pWing->m_NSurfaces; j++)
		{
			glBegin(GL_LINES);
			{
				pWing->m_Surface[j].GetPanel(0,pWing->m_Surface[j].m_NXPanels-1, MIDSURFACE);
				glVertex3d(pWing->m_Surface[j].LA.x,
						   pWing->m_Surface[j].LA.y,
						   pWing->m_Surface[j].LA.z);
				pWing->m_Surface[j].GetPanel( pWing->m_Surface[j].m_NYPanels-1,pWing->m_Surface[j].m_NXPanels-1, MIDSURFACE);
				glVertex3d(pWing->m_Surface[j].LB.x,
						   pWing->m_Surface[j].LB.y,
						   pWing->m_Surface[j].LB.z);
			}
			glEnd();
		}
		//Trailing edge outline
		for (j=0; j<pWing->m_NSurfaces; j++)
		{
			glBegin(GL_LINES);
			{
				pWing->m_Surface[j].GetPanel(0,0, MIDSURFACE);
				glVertex3d(pWing->m_Surface[j].TA.x,
						   pWing->m_Surface[j].TA.y,
						   pWing->m_Surface[j].TA.z);
				pWing->m_Surface[j].GetPanel( pWing->m_Surface[j].m_NYPanels-1, 0, MIDSURFACE);
				glVertex3d(pWing->m_Surface[j].TB.x,
						   pWing->m_Surface[j].TB.y,
						   pWing->m_Surface[j].TB.z);
			}
			glEnd();
		}
		//flap outline....
		for (j=0; j<pWing->m_NSurfaces; j++)
		{
			pFoilA = pWing->m_Surface[j].m_pFoilA;
			pFoilB = pWing->m_Surface[j].m_pFoilB;
			if(pFoilA && pFoilB && pFoilA->m_bTEFlap && pFoilB->m_bTEFlap)
			{
				glBegin(GL_LINES);
				{
					if(pFoilA->m_bTEFlap)
						pWing->m_Surface[j].GetSurfacePoint(pWing->m_Surface[j].m_pFoilA->m_TEXHinge/100.0,
															pWing->m_Surface[j].m_pFoilA->m_TEXHinge/100.0,
															0.0, Pt, 1);
					else 	pWing->m_Surface[j].GetSurfacePoint(1.0, 1.0, 0.0, Pt, 1);
					glVertex3d(Pt.x, Pt.y, Pt.z);

					if(pFoilB->m_bTEFlap)
						pWing->m_Surface[j].GetSurfacePoint(pWing->m_Surface[j].m_pFoilB->m_TEXHinge/100.0,
															pWing->m_Surface[j].m_pFoilB->m_TEXHinge/100.0,
															1.0, Pt, 1);
					else 	pWing->m_Surface[j].GetSurfacePoint(1.0, 1.0, 1.0, Pt, 1);
					glVertex3d(Pt.x, Pt.y, Pt.z);
				}
				glEnd();
				glBegin(GL_LINES);
				{
					if(pFoilA->m_bTEFlap)
						pWing->m_Surface[j].GetSurfacePoint(pWing->m_Surface[j].m_pFoilA->m_TEXHinge/100.0,
															pWing->m_Surface[j].m_pFoilA->m_TEXHinge/100.0,
															0.0, Pt, -1);
					else 	pWing->m_Surface[j].GetSurfacePoint(1.0, 1.0, 0.0, Pt, -1);
					glVertex3d(Pt.x, Pt.y, Pt.z);

					if(pFoilB->m_bTEFlap)
						pWing->m_Surface[j].GetSurfacePoint(pWing->m_Surface[j].m_pFoilB->m_TEXHinge/100.0,
															pWing->m_Surface[j].m_pFoilB->m_TEXHinge/100.0,
															1.0, Pt, -1);
					else 	pWing->m_Surface[j].GetSurfacePoint(1.0, 1.0, 1.0, Pt, -1);
					glVertex3d(Pt.x, Pt.y, Pt.z);
				}
				glEnd();
			}
		}
		for (j=0; j<pWing->m_NSurfaces; j++)
		{
			pFoilA = pWing->m_Surface[j].m_pFoilA;
			pFoilB = pWing->m_Surface[j].m_pFoilB;
			if(pFoilA && pFoilB && pFoilA->m_bLEFlap && pFoilB->m_bLEFlap)
			{
				glBegin(GL_LINES);
				{
					if(pFoilA->m_bLEFlap)
						pWing->m_Surface[j].GetSurfacePoint(pWing->m_Surface[j].m_pFoilA->m_LEXHinge/100.0,
															pWing->m_Surface[j].m_pFoilA->m_LEXHinge/100.0,
															0.0, Pt, 1);
					else 	pWing->m_Surface[j].GetSurfacePoint(1.0, 1.0, 0.0, Pt, 1);
					glVertex3d(Pt.x, Pt.y, Pt.z);

					if(pFoilB->m_bLEFlap)
						pWing->m_Surface[j].GetSurfacePoint(pWing->m_Surface[j].m_pFoilB->m_LEXHinge/100.0,
															pWing->m_Surface[j].m_pFoilB->m_LEXHinge/100.0,
															1.0, Pt, 1);
					else 	pWing->m_Surface[j].GetSurfacePoint(1.0, 1.0, 1.0, Pt, 1);
					glVertex3d(Pt.x, Pt.y, Pt.z);
				}
				glEnd();
				glBegin(GL_LINES);
				{
					if(pFoilA->m_bLEFlap)
						pWing->m_Surface[j].GetSurfacePoint(pWing->m_Surface[j].m_pFoilA->m_LEXHinge/100.0,
															pWing->m_Surface[j].m_pFoilA->m_LEXHinge/100.0,
															0.0, Pt, -1);
					else pWing->m_Surface[j].GetSurfacePoint(1.0, 1.0, 0.0, Pt, -1);
					glVertex3d(Pt.x, Pt.y, Pt.z);

					if(pFoilB->m_bLEFlap)
						pWing->m_Surface[j].GetSurfacePoint(pWing->m_Surface[j].m_pFoilB->m_LEXHinge/100.0,
															pWing->m_Surface[j].m_pFoilB->m_LEXHinge/100.0,
															1.0, Pt, -1);
					else pWing->m_Surface[j].GetSurfacePoint(1.0, 1.0, 1.0, Pt, -1);
					glVertex3d(Pt.x, Pt.y, Pt.z);
				}
				glEnd();
			}
		}
		glDisable (GL_LINE_STIPPLE);
	}
	glEndList();
}


void GLCreateCp(void *pQMiarex, CVector *pNode, Panel *pPanel, WingOpp *pWOpp, PlaneOpp *pPOpp)
{
	QMiarex * pMiarex = (QMiarex*)pQMiarex;
    if((!pWOpp && !pPOpp) || !pPanel || !pNode || !pMiarex->m_MatSize)
	{
		glNewList(PANELCP,GL_COMPILE);
		glEndList();
		return;
	}

	int p, pp, n, averageInf, averageSup, average100;
	int nPanels;
	double color;
	double lmin, lmax, range;
	float *tab;

    double *CpInf = new double[2*QMiarex::s_MaxMatSize];
    double *CpSup = new double[2*QMiarex::s_MaxMatSize];
    double *Cp100 = new double[2*QMiarex::s_MaxMatSize];

	CVector LA,LB,TA,TB;

	glNewList(PANELCP, GL_COMPILE);
	{
		pMiarex->m_GLList++;
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glPolygonOffset(1.0, 1.0);

		if(pPOpp)
		{
			tab = pPOpp->m_Cp;
			nPanels = pPOpp->m_NPanels;
		}
		else
		{
			tab = pWOpp->m_Cp;
			nPanels = pWOpp->m_NVLMPanels;
		}

		lmin = 10000.0;
		lmax = -10000.0;
		// find min and max Cp for scale set
		for (n=0; n<pMiarex->m_nNodes; n++)
		{
			averageInf = 0; averageSup = 0; average100 = 0;
			CpInf[n] = 0.0; CpSup[n] = 0.0; Cp100[n] = 0.0;
			for (pp=0; pp< nPanels; pp++)
			{
				if (pNode[pPanel[pp].m_iLA].IsSame(pNode[n]) || pNode[pPanel[pp].m_iTA].IsSame(pNode[n]) ||
					pNode[pPanel[pp].m_iTB].IsSame(pNode[n]) || pNode[pPanel[pp].m_iLB].IsSame(pNode[n]))
				{
					if(pPanel[pp].m_Pos==TOPSURFACE)
					{
						CpSup[n] +=tab[pp];
						averageSup++;
					}
					else if(pPanel[pp].m_Pos<=MIDSURFACE)
					{
						CpInf[n] +=tab[pp];
						averageInf++;
					}
					else if(pPanel[pp].m_Pos==BODYSURFACE)
					{
						Cp100[n] +=tab[pp];
						average100++;
					}
				}
			}
			if(averageSup>0)
			{
				CpSup[n] /= averageSup;
				if(CpSup[n]<lmin) lmin = CpSup[n];
				if(lmax<CpSup[n]) lmax = CpSup[n];
			}
			if(averageInf>0)
			{
				CpInf[n] /= averageInf;
				if(CpInf[n]<lmin) lmin = CpInf[n];
				if(lmax<CpInf[n]) lmax = CpInf[n];
			}
			if(average100>0)
			{
				Cp100[n] /= average100;
				if(Cp100[n]<lmin) lmin = Cp100[n];
				if(lmax<Cp100[n]) lmax = Cp100[n];
			}
		}

		if(pMiarex->m_bAutoCpScale)
		{
			pMiarex->m_LegendMin = lmin;
			pMiarex->m_LegendMax = lmax;
		}
		else
		{
			lmin = pMiarex->m_LegendMin;
			lmax = pMiarex->m_LegendMax;
		}

		range = lmax - lmin;

		glLineWidth(1.0);
		for (p=0; p<pMiarex->m_MatSize; p++)
		{
			glBegin(GL_QUADS);
			{
				TA.Copy(pNode[pPanel[p].m_iTA]);
				TB.Copy(pNode[pPanel[p].m_iTB]);
				LA.Copy(pNode[pPanel[p].m_iLA]);
				LB.Copy(pNode[pPanel[p].m_iLB]);

				if(pPanel[p].m_Pos==TOPSURFACE) color = (CpSup[pPanel[p].m_iLA]-lmin)/range;
				else if(pPanel[p].m_Pos<=MIDSURFACE) color = (CpInf[pPanel[p].m_iLA]-lmin)/range;
				else                           color = (Cp100[pPanel[p].m_iLA]-lmin)/range;
				glColor3d(GLGetRed(color),GLGetGreen(color),GLGetBlue(color));
				glVertex3d(LA.x, LA.y, LA.z);

				if(pPanel[p].m_Pos==TOPSURFACE) color = (CpSup[pPanel[p].m_iTA]-lmin)/range;
				else if(pPanel[p].m_Pos<=MIDSURFACE) color = (CpInf[pPanel[p].m_iTA]-lmin)/range;
				else                           color = (Cp100[pPanel[p].m_iTA]-lmin)/range;
				glColor3d(GLGetRed(color),GLGetGreen(color),GLGetBlue(color));
				glVertex3d(TA.x, TA.y, TA.z);

				if(pPanel[p].m_Pos==TOPSURFACE) color = (CpSup[pPanel[p].m_iTB]-lmin)/range;
				else if(pPanel[p].m_Pos<=MIDSURFACE) color = (CpInf[pPanel[p].m_iTB]-lmin)/range;
				else                           color = (Cp100[pPanel[p].m_iTB]-lmin)/range;
				glColor3d(GLGetRed(color),GLGetGreen(color),GLGetBlue(color));
				glVertex3d(TB.x, TB.y, TB.z);

				if(pPanel[p].m_Pos==TOPSURFACE) color = (CpSup[pPanel[p].m_iLB]-lmin)/range;
				else if(pPanel[p].m_Pos<=MIDSURFACE) color = (CpInf[pPanel[p].m_iLB]-lmin)/range;
				else                           color = (Cp100[pPanel[p].m_iLB]-lmin)/range;
				glColor3d(GLGetRed(color),GLGetGreen(color),GLGetBlue(color));
				glVertex3d(LB.x, LB.y, LB.z);

			}
			glEnd();
		}
		glDisable(GL_POLYGON_OFFSET_FILL);
	}
	glEndList();
}


void GLCreateCpLegendClr(void *pQMiarex)
{
	int i;
	QMiarex *pMiarex = (QMiarex*)pQMiarex;

    QFontMetrics fm(MainFrame::s_TextFont);
    double fmw = (double) fm.averageCharWidth();

	double fi, ZPos,dz,Right1, Right2;
	double color = 0.0;

	double w = (double)pMiarex->m_r3DCltRect.width();
	double h = (double)pMiarex->m_r3DCltRect.height();
	double XPos;

	if(w>h)
	{
        XPos  = 1.0;
        dz    = h/w /40.0;
        ZPos  = h/w/10 - 12.0*dz;
	}
	else
	{
        XPos  = w/h;
        dz    = 1. /40.0;
        ZPos  = 1./10 - 12.0*dz;
	}

    Right1  = XPos - 8 * fmw/w;
    Right2  = XPos - 3 * fmw/w;

	glNewList(WOPPCPLEGENDCLR,GL_COMPILE);
	{
		pMiarex->m_GLList++;
		glDisable(GL_LIGHTING);
		glDisable(GL_LIGHT0);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glBegin(GL_QUAD_STRIP);
		{
			for (i=0; i<=20; i++)
			{
				fi = (double)i*dz;
				color += 0.05;

				glColor3d(GLGetRed(color),GLGetGreen(color),GLGetBlue(color));
                glVertex3d(Right1, ZPos+fi*2, 0.0);
                glVertex3d(Right2, ZPos+fi*2, 0.0);
			}
		}
		glEnd();
	}
	glEndList();
}


void GLCreateDownwash(void *pQMiarex, Wing *pWing, WingOpp *pWOpp, int List)
{
	// pWing is either the Wing, the stab, or the fin
	// pWOpp is related to the pWing

	QMiarex *pMiarex = (QMiarex*)pQMiarex;

	if(!pWing || !pWOpp) return;
	
	QColor color;
	int style, width;
	int i,j,k,p;
	double dih, xt, yt, zt, yob;
	double y1, y2, z1, z2, xs, ys, zs;
	CVector C;
	double factor, amp;

	double sina = -sin(pWOpp->m_Alpha*PI/180.0);
	double cosa =  cos(pWOpp->m_Alpha*PI/180.0);
	factor = pMiarex->m_VelocityScale/100.0;

	//DOWNWASH
	glNewList(List,GL_COMPILE);
	{
		pMiarex->m_GLList++;

		glEnable (GL_LINE_STIPPLE);

		color = pMiarex->m_DownwashColor;
		style = pMiarex->m_DownwashStyle;
		width = pMiarex->m_DownwashWidth;

		glColor3d(color.redF(), color.greenF(), color.blueF());

		glLineWidth((GLfloat)width);

		if     (style == 1) 	glLineStipple (1, 0xCFCF);
		else if(style == 2) 	glLineStipple (1, 0x6666);
		else if(style == 3) 	glLineStipple (1, 0xFF18);
		else if(style == 4) 	glLineStipple (1, 0x7E66);
		else					glLineStipple (1, 0xFFFF);


		if(pWOpp)
		{
			if(pWOpp->m_AnalysisMethod==LLTMETHOD)
			{
				for (i=1; i<pWOpp->m_NStation; i++)
				{
					yob = 2.0*pWOpp->m_SpanPos[i]/pWOpp->m_Span;
					xt = pWing->Offset(yob) + pWing->Chord(yob);
					pWing->GetViewYZPos(1., pWOpp->m_SpanPos[i], yt,zt,0);

					dih = -pWing->Dihedral(yob)*PI/180.0;
					amp = pWOpp->m_QInf*sin(pWOpp->m_Ai[i]*PI/180.0);
					amp *= factor;
					glBegin(GL_LINES);
					{
						glVertex3d(xt, yt, zt);
						glVertex3d(xt + amp * cos(dih)* sina,
								 yt + amp * sin(dih),
								 zt + amp * cos(dih)* cosa);
					}
					glEnd();
				}
				glBegin(GL_LINE_STRIP);
				{
					for (i=1; i<pWOpp->m_NStation; i++)
					{
						yob = 2.0*pWOpp->m_SpanPos[i]/pWOpp->m_Span;
						xt = pWing->Offset(yob) + pWing->Chord(yob);
						pWing->GetViewYZPos(1., pWOpp->m_SpanPos[i], yt,zt,0);

						dih = -pWing->Dihedral(yob)*PI/180.0;
						amp = pWOpp->m_QInf*sin(pWOpp->m_Ai[i]*PI/180.0);
						amp *= factor;

						glVertex3d(xt + amp * cos(dih)* sina,
								 yt + amp * sin(dih),
								 zt + amp * cos(dih)* cosa);
					}
				}
				glEnd();
			}
			else
			{
				p = 0;
				i = 0;
				for (j=0; j<pWing->m_NSurfaces; j++)
				{
					for (k=0; k< pWing->m_Surface[j].m_NYPanels; k++)
					{
//						m_pSurface[j+surf0]->GetTrailingPt(k, C);
						pWing->m_Surface[j].GetTrailingPt(k, C);
//						if (pWOpp->m_Vd[i].z>0) sign = 1.0; else sign = -1.0;
						glBegin(GL_LINES);
						{
							glVertex3d(C.x, C.y, C.z);
							glVertex3d(C.x+factor*pWOpp->m_Vd[i].z * sina,
									   C.y+factor*pWOpp->m_Vd[i].y,
									   C.z+factor*pWOpp->m_Vd[i].z * cosa);
						}
						glEnd();
						xs = C.x+factor*pWOpp->m_Vd[i].z*sina;
						ys = C.y+factor*pWOpp->m_Vd[i].y;
						zs = C.z+factor*pWOpp->m_Vd[i].z*cosa;
						y1 = ys - 0.085*factor*pWOpp->m_Vd[i].y      + 0.05*factor*pWOpp->m_Vd[i].z*cosa;
						z1 = zs - 0.085*factor*pWOpp->m_Vd[i].z*cosa - 0.05*factor*pWOpp->m_Vd[i].y;
						y2 = ys - 0.085*factor*pWOpp->m_Vd[i].y      - 0.05*factor*pWOpp->m_Vd[i].z*cosa;
						z2 = zs - 0.085*factor*pWOpp->m_Vd[i].z*cosa + 0.05*factor*pWOpp->m_Vd[i].y;

						glBegin(GL_LINES);
						{
							glVertex3d(xs, ys, zs);
							glVertex3d(xs, y1, z1);
						}
						glEnd();
						glBegin(GL_LINES);
						{
							glVertex3d(xs, ys, zs);
							glVertex3d(xs, y2, z2);
						}
						glEnd();

						i++;
					}
					p++;
				}
			}
		}
		glDisable (GL_LINE_STIPPLE);
	}
	glEndList();
}


void GLCreateDrag(void *pQMiarex, Wing *pWing, WPolar* pWPolar, WingOpp *pWOpp, int List)
{
	if(!pWing || !pWPolar || !pWOpp) return;
	CVector C;
	int i,j,k;
	int Istyle, Iwidth, Vstyle, Vwidth;
	QColor Icolor, Vcolor;
	QMiarex * pMiarex = (QMiarex*)pQMiarex;
	double coef = 5.0;

	GLushort IDash, VDash;

	static double Ir,Ig,Ib, Vr, Vg, Vb;
	static double amp, amp1, amp2;
	static double yob, xt, yt, zt, dih;
	static double cosa, cosb, sina, sinb;
	cosa =  cos(pWOpp->m_Alpha * PI/180.0);
	sina = -sin(pWOpp->m_Alpha * PI/180.0);
	cosb =  cos(pWPolar->sideSlip()*PI/180.0);
	sinb =  sin(pWPolar->sideSlip()*PI/180.0);

	Icolor = pMiarex->m_IDragColor;
	Istyle = pMiarex->m_IDragStyle;
	Iwidth = pMiarex->m_IDragWidth;

	Vcolor = pMiarex->m_VDragColor;
	Vstyle = pMiarex->m_VDragStyle;
	Vwidth = pMiarex->m_VDragWidth;

	if     (Istyle == 1) 	IDash = 0xCFCF;
	else if(Istyle == 2) 	IDash = 0x6666;
	else if(Istyle == 3) 	IDash = 0xFF18;
	else if(Istyle == 4) 	IDash = 0x7E66;
	else					IDash = 0xFFFF;

	if     (Vstyle == 1) 	VDash = 0xCFCF;
	else if(Vstyle == 2) 	VDash = 0x6666;
	else if(Vstyle == 3) 	VDash = 0xFF18;
	else if(Vstyle == 4) 	VDash = 0x7E66;
	else					VDash = 0xFFFF;

	Ir = Icolor.redF();
	Ig = Icolor.greenF();
	Ib = Icolor.blueF();
	Vr = Vcolor.redF();
	Vg = Vcolor.greenF();
	Vb = Vcolor.blueF();


	//DRAGLINE
	glNewList(List,GL_COMPILE);
	{
		pMiarex->m_GLList++;
		glEnable (GL_LINE_STIPPLE);
		glLineStipple (1, IDash);// Solid
		glLineWidth((GLfloat)(Iwidth));
//		glPolygonMode(GL_FRONT,GL_LINE);

		double q0 = 0.5 * pWPolar->m_Density * pWPolar->m_WArea * pWOpp->m_QInf * pWOpp->m_QInf;

		if(pWOpp)
		{
			if(pWOpp->m_AnalysisMethod==LLTMETHOD)
			{
				for (i=1; i<pWOpp->m_NStation; i++)
				{
					yob = 2.0*pWOpp->m_SpanPos[i]/pWOpp->m_Span;
					xt = pWing->Offset(yob) + pWOpp->m_Chord[i];
					pWing->GetViewYZPos(1.0, pWOpp->m_SpanPos[i],yt,zt,0);
					dih = pWing->Dihedral(yob)*PI/180.0;
					amp1 = q0*pWOpp->m_ICd[i]*pWOpp->m_Chord[i]/pWOpp->m_MAChord*pMiarex->m_DragScale/coef;
					amp2 = q0*pWOpp->m_PCd[i]*pWOpp->m_Chord[i]/pWOpp->m_MAChord*pMiarex->m_DragScale/coef;
					if(pMiarex->m_bICd)
					{
						glColor3f((GLfloat)Ir,(GLfloat)Ig,(GLfloat)Ib);
						glLineStipple (1, IDash);// Solid
						glLineWidth((GLfloat)(Iwidth));
						glBegin(GL_LINES);
						{
							glVertex3d(xt, yt, zt);
							glVertex3d(	xt + amp1 * cos(dih)*cosa,
										yt,
										zt - amp1 * cos(dih)*sina);
						}
						glEnd();
					}
					if(pMiarex->m_bVCd)
					{
						glColor3f((GLfloat)Vr,(GLfloat)Vg,(GLfloat)Vb);
						glLineStipple (1, VDash);// Solid
						glLineWidth((GLfloat)(Vwidth));
						glBegin(GL_LINES);
						{
							if(!pMiarex->m_bICd)
							{
								glVertex3d(xt, yt,zt);
								glVertex3d(xt + amp2 * cos(dih)*cosa,
										   yt,
										   zt - amp2 * cos(dih)*sina);
							}
							else {
								glVertex3d(xt + amp1 * cos(dih)*cosa,
										   yt,
										   zt - amp1 * cos(dih)*sina);
								glVertex3d(xt + (amp1+amp2) * cos(dih)*cosa,
										   yt,
										   zt - (amp1+amp2) * cos(dih)*sina);
							}
						}
						glEnd();
					}
				}
				if(pMiarex->m_bICd)
				{
					glColor3f((GLfloat)Ir,(GLfloat)Ig,(GLfloat)Ib);
					glLineStipple (1, IDash);// Solid
					glLineWidth((GLfloat)(Iwidth));
					glBegin(GL_LINE_STRIP);
					{
						for (i=1; i<pWOpp->m_NStation; i++)
						{
							yob = 2.0*pWOpp->m_SpanPos[i]/pWOpp->m_Span;
							xt = pWing->Offset(yob) + pWOpp->m_Chord[i];
							pWing->GetViewYZPos(1.0, pWOpp->m_SpanPos[i],yt,zt,0);

							dih = pWing->Dihedral(yob)*PI/180.0;

							amp  = q0*pWOpp->m_ICd[i]*pWOpp->m_Chord[i]/pWOpp->m_MAChord;
							amp *= pMiarex->m_DragScale/coef;

							glVertex3d(xt + amp * cos(dih)*cosa,
									   yt,
									   zt - amp * cos(dih)*sina);
						}
					}
					glEnd();
				}
				if(pMiarex->m_bVCd)
				{
					glColor3f((GLfloat)Vr,(GLfloat)Vg,(GLfloat)Vb);
					glLineStipple (1, VDash);// Solid
					glLineWidth((GLfloat)(Vwidth));
					glBegin(GL_LINE_STRIP);
					{
						for (i=1; i<pWOpp->m_NStation; i++)
						{
							yob = 2.0*pWOpp->m_SpanPos[i]/pWOpp->m_Span;
							xt = pWing->Offset(yob) + pWOpp->m_Chord[i];
							pWing->GetViewYZPos(1.0, pWOpp->m_SpanPos[i],yt,zt,0);

							dih = pWing->Dihedral(yob)*PI/180.0;
							amp=0.0;
							if(pMiarex->m_bICd) amp+=pWOpp->m_ICd[i];
							amp +=pWOpp->m_PCd[i];
							amp *= q0*pWOpp->m_Chord[i]/pWOpp->m_MAChord;
							amp *= pMiarex->m_DragScale/coef;

							glVertex3d( xt + amp * cos(dih)*cosa,
										yt ,
										zt - amp * cos(dih)*sina);
						}
					}
					glEnd();
				}
			}
			else
			{
				//VLM type drag
				i = 0;
				for (j=0; j<pWing->m_NSurfaces; j++)
				{
					//All surfaces
					for (k=0; k< pWing->m_Surface[j].m_NYPanels; k++)
					{
						pWing->m_Surface[j].GetTrailingPt(k, C);
						amp1 = q0*pWOpp->m_ICd[i]*pWOpp->m_Chord[i]/pWing->m_MAChord*pMiarex->m_DragScale/coef;
						amp2 = q0*pWOpp->m_PCd[i]*pWOpp->m_Chord[i]/pWing->m_MAChord*pMiarex->m_DragScale/coef;
						if(pMiarex->m_bICd)
						{
							glColor3f((GLfloat)Ir,(GLfloat)Ig,(GLfloat)Ib);
							glLineStipple (1, IDash);// Solid
							glLineWidth((GLfloat)(Iwidth));
							glBegin(GL_LINES);
							{
								glVertex3d(C.x, C.y, C.z);
								glVertex3d(C.x + amp1*cosa * cosb,
										 C.y + amp1*cosa * sinb,
										 C.z - amp1*sina);
							}
							glEnd();
						}
						if(pMiarex->m_bVCd)
						{
							glColor3f((GLfloat)Vr,(GLfloat)Vg,(GLfloat)Vb);
							glLineStipple (1, VDash);// Solid
							glLineWidth((GLfloat)(Vwidth));
							glBegin(GL_LINES);
							{
								if(!pMiarex->m_bICd)
								{
									glVertex3d(C.x, C.y, C.z);
									glVertex3d(C.x + amp2*cosa*cosb,
											 C.y + amp2*cosa*sinb,
											 C.z - amp2*sina);
								}
								else
								{
									glVertex3d(C.x + amp1*cosa*cosb,
											 C.y + amp1*cosa*sinb,
											 C.z - amp1*sina);
									glVertex3d(C.x + (amp1+amp2)*cosa*cosb,
											 C.y + (amp1+amp2)*cosa*sinb,
											 C.z - (amp1+amp2)*sina);
								}
							}
							glEnd();
						}

						i++;
					}
				}
				if(!pWing->IsFin())
				{
					if(pMiarex->m_bICd)
					{
						glColor3f((GLfloat)Ir,(GLfloat)Ig,(GLfloat)Ib);
						glLineStipple (1, IDash);// Solid
						glLineWidth((GLfloat)(Iwidth));
						glBegin(GL_LINE_STRIP);
						{
							i = 0;
							for (j=0; j<pWing->m_NSurfaces; j++)
							{
								for (k=0; k< pWing->m_Surface[j].m_NYPanels; k++)
								{
									pWing->m_Surface[j].GetTrailingPt(k, C);
									amp = q0*(pWOpp->m_ICd[i])*pWOpp->m_Chord[i]/pWing->m_MAChord;
									amp *= pMiarex->m_DragScale/coef;
									glVertex3d(C.x + amp*cosa*cosb,
											 C.y + amp*cosa*sinb,
											 C.z - amp*sina);
									i++;
								}
							}
						}
						glEnd();
					}
					if(pMiarex->m_bVCd)
					{
						glColor3f((GLfloat)Vr,(GLfloat)Vg,(GLfloat)Vb);
						glLineStipple (1, VDash);// Solid
						glLineWidth((GLfloat)(Vwidth));
						glBegin(GL_LINE_STRIP);
						{
							i = 0;
							for (j=0; j<pWing->m_NSurfaces; j++)
							{
								for (k=0; k< pWing->m_Surface[j].m_NYPanels; k++)
								{
									pWing->m_Surface[j].GetTrailingPt(k, C);
									amp=0.0;
									if(pMiarex->m_bICd) amp+=pWOpp->m_ICd[i];
									amp +=pWOpp->m_PCd[i];
									amp *= q0*pWOpp->m_Chord[i]/pWing->m_MAChord;
									amp *= pMiarex->m_DragScale/coef;

									glVertex3d(C.x + amp*cosa*cosb,
											 C.y + amp*cosa*sinb,
											 C.z - amp*sina);
									i++;
								}
							}
						}
						glEnd();
					}
				}
				else
				{
					if(pMiarex->m_bICd)
					{
						glColor3f((GLfloat)Ir,(GLfloat)Ig,(GLfloat)Ib);
						glLineStipple (1, IDash);// Solid
						glLineWidth((GLfloat)Iwidth);
						i = 0;
						for (j=0; j<pWing->m_NSurfaces; j++)
						{
							glBegin(GL_LINE_STRIP);
							{
								for (k=0; k< pWing->m_Surface[j].m_NYPanels; k++)
								{
									pWing->m_Surface[j].GetTrailingPt(k, C);
									amp = q0*(pWOpp->m_ICd[i])*pWOpp->m_Chord[i]/pWing->m_MAChord;
									amp *= pMiarex->m_DragScale/coef;
									glVertex3d(C.x + amp*cosa*cosb,
											 C.y + amp*cosa*sinb,
											 C.z - amp*sina);
									i++;
								}
							}
							glEnd();
						}
					}
					if(pMiarex->m_bVCd)
					{
						glColor3f((GLfloat)Vr,(GLfloat)Vg,(GLfloat)Vb);
						glLineStipple (1, VDash);// Solid
						glLineWidth((GLfloat)Vwidth);
						i = 0;
						for (j=0; j<pWing->m_NSurfaces; j++)
						{
							glBegin(GL_LINE_STRIP);
							{
								for (k=0; k< pWing->m_Surface[j].m_NYPanels; k++)
								{
									pWing->m_Surface[j].GetTrailingPt(k, C);
									amp=0.0;
									if(pMiarex->m_bICd) amp+=pWOpp->m_ICd[i];
									amp +=pWOpp->m_PCd[i];
									amp *= q0*pWOpp->m_Chord[i]/pWing->m_MAChord;
									amp *= pMiarex->m_DragScale/coef;

									glVertex3d(C.x + amp*cosa*cosb,
											 C.y + amp*cosa*sinb,
											 C.z - amp*sina);
									i++;
								}
							}
							glEnd();
						}
					}
				}
			}
		}
		glDisable (GL_LINE_STIPPLE);
	}
	glEndList();
}


void GLCreateMesh(int iList, int size, Panel *pPanel, CVector *pNode, QColor PanelColor, QColor BackColor, bool bBack)
{
    if(!pPanel || !pNode)
    {
        glNewList(iList, GL_COMPILE);
        glEndList();
        return;
    }
    int iLA, iLB, iTA, iTB;
    int p;
    CVector  N;
    N.Set(0.0, 0.0, 0.0);

    glNewList(iList, GL_COMPILE);
    {
        glDisable(GL_LINE_STIPPLE);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_POLYGON_OFFSET_LINE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glPolygonOffset(1.0, 1.0);

        glLineWidth(1.0);

        glColor3d(PanelColor.redF(),PanelColor.greenF(),PanelColor.blueF());

        for (p=0; p<size; p++)
        {
            glBegin(GL_QUADS);
            {
                iLA = pPanel[p].m_iLA;
                iLB = pPanel[p].m_iLB;
                iTA = pPanel[p].m_iTA;
                iTB = pPanel[p].m_iTB;

                glNormal3d(pPanel[p].Normal.x, pPanel[p].Normal.y, pPanel[p].Normal.z);
                glVertex3d(pNode[iLA].x, pNode[iLA].y, pNode[iLA].z);
                glVertex3d(pNode[iTA].x, pNode[iTA].y, pNode[iTA].z);
                glVertex3d(pNode[iTB].x, pNode[iTB].y, pNode[iTB].z);
                glVertex3d(pNode[iLB].x, pNode[iLB].y, pNode[iLB].z);
            }
            glEnd();
        }
        glDisable(GL_DEPTH_TEST);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDisable(GL_POLYGON_OFFSET_LINE);
    }
    glEndList();

    if(!bBack) return;

    glNewList(iList+1,GL_COMPILE);
    {
        glEnable(GL_DEPTH_TEST);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.0, 1.0);


        glColor3d(BackColor.redF(),BackColor.greenF(),BackColor.blueF());

        glLineWidth(1.0);
        glDisable (GL_LINE_STIPPLE);

        for (p=0; p<size; p++)
        {
            glBegin(GL_QUADS);
            {
                iLA = pPanel[p].m_iLA;
                iLB = pPanel[p].m_iLB;
                iTA = pPanel[p].m_iTA;
                iTB = pPanel[p].m_iTB;

                glVertex3d(pNode[iLA].x, pNode[iLA].y, pNode[iLA].z);
                glVertex3d(pNode[iTA].x, pNode[iTA].y, pNode[iTA].z);
                glVertex3d(pNode[iTB].x, pNode[iTB].y, pNode[iTB].z);
                glVertex3d(pNode[iLB].x, pNode[iLB].y, pNode[iLB].z);
                glNormal3d(pPanel[p].Normal.x, pPanel[p].Normal.y, pPanel[p].Normal.z);
            }
            glEnd();
        }

        glDisable(GL_POLYGON_OFFSET_FILL);
        glDisable(GL_DEPTH_TEST);
    }
    glEndList();
}


void GLCreateCtrlPts(void *pQMiarex, Panel *pPanel)
{
	QMiarex * pMiarex = (QMiarex*)pQMiarex;
    if(!pPanel || !pMiarex->m_MatSize)
    {
        glNewList(VLMCTRLPTS, GL_COMPILE);
        glEndList();
        return;
    }

	glNewList(VLMCTRLPTS,GL_COMPILE);
	{
		pMiarex->m_GLList++;
		glLineWidth(1.0);
		glColor3d(0.0,1.0,0.0);
		for (int p=0; p<pMiarex->m_MatSize; p++)
		{
			//All panels
			glBegin(GL_LINES);
			{
				glVertex3d(pPanel[p].CtrlPt.x, pPanel[p].CtrlPt.y, pPanel[p].CtrlPt.z);
				glVertex3d((pPanel[p].CtrlPt.x + pPanel[p].Normal.x * 0.04),
						   (pPanel[p].CtrlPt.y + pPanel[p].Normal.y * 0.04),
						   (pPanel[p].CtrlPt.z + pPanel[p].Normal.z * 0.04));

			}
			glEnd();
		}
	}
	glEndList();
}


void GLCreateVortices(void *pQMiarex, Panel *pPanel, CVector *pNode, WPolar *pWPolar)
{
	QMiarex * pMiarex = (QMiarex*)pQMiarex;
    if(!pNode || !pPanel || !pMiarex->m_MatSize)
    {
        glNewList(VLMVORTICES, GL_COMPILE);
        glEndList();
        return;
    }

	int p;
	CVector A, B, C, D, AC, BD;

	glNewList(VLMVORTICES,GL_COMPILE);
	{
		pMiarex->m_GLList++;

		glEnable (GL_LINE_STIPPLE);
		glLineStipple (1, 0xFFFF);

		glLineWidth(1.0);
		glColor3d(0.7,0.0,0.0);
		for (p=0; p<pMiarex->m_MatSize; p++)
		{
			if(!pPanel[p].m_bIsTrailing)
			{
				if(pPanel[p].m_Pos<=MIDSURFACE)
				{
					A = pPanel[p].VA;
					B = pPanel[p].VB;
					C = pPanel[p-1].VB;
					D = pPanel[p-1].VA;
				}
				else
				{
					A = pPanel[p].VA;
					B = pPanel[p].VB;
					C = pPanel[p+1].VB;
					D = pPanel[p+1].VA;
				}
			}
			else
			{
				A = pPanel[p].VA;
				B = pPanel[p].VB;
				// we define point AA=A+1 and BB=B+1
				C.x =  pNode[pPanel[p].m_iTB].x
					+ (pNode[pPanel[p].m_iTB].x-pPanel[p].VB.x)/3.0;
				C.y =  pNode[pPanel[p].m_iTB].y;
				C.z =  pNode[pPanel[p].m_iTB].z;
				D.x =  pNode[pPanel[p].m_iTA].x
					+ (pNode[pPanel[p].m_iTA].x-pPanel[p].VA.x)/3.0;
				D.y =  pNode[pPanel[p].m_iTA].y;
				D.z =  pNode[pPanel[p].m_iTA].z;
			}

			//next we "shrink" the points to avoid confusion with panels sides
			AC = C-A;
			BD = D-B;

			AC *= 0.03;
			A  += AC;
			C  -= AC;
			BD *= 0.03;
			B  += BD;
			D  -= BD;

			if(pWPolar && pWPolar->m_bVLM1)
			{
				glLineStipple (1, 0xFFFF);
				glBegin(GL_LINES);
				{
					glVertex3d(A.x, A.y, A.z);
					glVertex3d(B.x, B.y, B.z);
				}
				glEnd();
				glLineStipple (1, 0x0F0F);
				glBegin(GL_LINES);
				{
					glVertex3d(A.x, A.y, A.z);
					glVertex3d(D.x, D.y, D.z);
				}
				glEnd();
				glBegin(GL_LINES);
				{
					glVertex3d(B.x, B.y, B.z);
					glVertex3d(C.x, C.y, C.z);
				}
				glEnd();
			}
			else if(!pWPolar || (pWPolar && !pWPolar->m_bVLM1))
			{
				glBegin(GL_LINE_STRIP);
				{
					glVertex3d(A.x, A.y, A.z);
					glVertex3d(B.x, B.y, B.z);
					glVertex3d(C.x, C.y, C.z);
					glVertex3d(D.x, D.y, D.z);
					glVertex3d(A.x, A.y, A.z);
				}
				glEnd();
			}
		}
		glDisable (GL_LINE_STIPPLE);
	}
	glEndList();
}


void GLCreateLiftForce(void *pQMiarex, WPolar *pWPolar, WingOpp *pWOpp)
{
	if(!pWPolar || !pWOpp) return;
    int style;

    double forcez,forcex,glx, gly, glz;
	double sign;

	QMiarex * pMiarex = (QMiarex*)pQMiarex;

	glNewList(LIFTFORCE, GL_COMPILE);
	{
		pMiarex->m_GLList++;
		glEnable (GL_LINE_STIPPLE);

		style = pMiarex->m_XCPStyle;
		if     (style == 1) 	glLineStipple (1, 0xCFCF);
		else if(style == 2) 	glLineStipple (1, 0x6666);
		else if(style == 3) 	glLineStipple (1, 0xFF18);
		else if(style == 4) 	glLineStipple (1, 0x7E66);
		else					glLineStipple (1, 0xFFFF);

        glColor3d(pMiarex->m_XCPColor.redF(),pMiarex->m_XCPColor.greenF(),pMiarex->m_XCPColor.blueF());

		//Resulting force vector
        glLineWidth((GLfloat)(pMiarex->m_XCPWidth * 2.0));

		double force = 0.5*pWPolar->m_Density * pWPolar->m_WArea
						  *pWOpp->m_QInf*pWOpp->m_QInf
						  *pWOpp->m_CL;

		force *= pMiarex->m_LiftScale/500.0;

		forcez =  force * cos(pWOpp->m_Alpha * PI/180.0);
		forcex = -force * sin(pWOpp->m_Alpha * PI/180.0);

		if (force>0.0) sign = 1.0; else sign = -1.0;
		glLineStipple (1, 0xFFFF);// Solid
		glLineWidth(3.0);

		glx = (GLfloat)pWOpp->m_CP.x;
		gly = (GLfloat)pWOpp->m_CP.y;
		glz = (GLfloat)pWOpp->m_CP.z;

		glBegin(GL_LINES);
            glVertex3d(glx,gly,glz);
            glVertex3d(glx+forcex,gly,glz+forcez);

		glEnd();

		glBegin(GL_LINES);
            glVertex3d(glx+forcex, gly, glz+forcez);
            //glVertex3d(glx+forcex+0.008, gly+0.008, glz+forcez-0.012*sign/pMiarex->m_glScaled);
            glVertex3d(glx+forcex+0.008, gly+0.008, glz+forcez-0.012*sign);
        glEnd();

		glBegin(GL_LINES);
            glVertex3d(glx+forcex, gly, glz+forcez);
            //glVertex3d(glx+forcex-0.008, gly-0.008, glz+forcez-0.012*sign/pMiarex->m_glScaled);
            glVertex3d(glx+forcex-0.008, gly-0.008, glz+forcez-0.012*sign);
        glEnd();

		glDisable (GL_LINE_STIPPLE);

	}
	glEndList();
}


void GLCreateMoments(void *pQMiarex, Wing *pWing, WPolar *pWPolar, WingOpp *pWOpp)
{
//	The most common aeronautical convention defines
//	- the roll as acting about the longitudinal axis, positive with the starboard wing down.
//	- The yaw is about the vertical body axis, positive with the nose to starboard.
//	- Pitch is about an axis perpendicular to the longitudinal plane of symmetry, positive nose up.
//	-- Wikipedia flight dynamics --
	if(!pWing || !pWPolar || !pWOpp) return;

	int i;
	int style, width;
	QColor color;

	double sign, amp, radius;
	double angle=0.0;//radian
	double endx, endy, endz, dx, dy, dz,xae, yae, zae;
	double factor = 10.0;

	QMiarex * pMiarex = (QMiarex*)pQMiarex;
//	MainFrame *pMainFrame = (MainFrame*)pMiarex->m_pMainFrame;

	glNewList(VLMMOMENTS, GL_COMPILE);
	{
		pMiarex->m_GLList++;
		glEnable (GL_LINE_STIPPLE);

		color = pMiarex->m_MomentColor;
		style = pMiarex->m_MomentStyle;
		width = pMiarex->m_MomentWidth;

		if     (style == 1) 	glLineStipple (1, 0xCFCF);
		else if(style == 2) 	glLineStipple (1, 0x6666);
		else if(style == 3) 	glLineStipple (1, 0xFF18);
		else if(style == 4) 	glLineStipple (1, 0x7E66);
		else					glLineStipple (1, 0xFFFF);

		glColor3d(color.redF(),color.greenF(),color.blueF());

		glLineWidth((GLfloat)(width*2.0));

		amp = 0.5*pWPolar->m_Density * pWPolar->m_WArea * pWPolar->m_WSpan
			  *pWOpp->m_QInf*pWOpp->m_QInf	*pWOpp->m_GCm/3.0;

		amp *= pMiarex->m_LiftScale*factor;

		radius= pWing->m_PlanformSpan/4.0;

		if (amp>0.0) sign = -1.0; else sign = 1.0;

		glBegin(GL_LINE_STRIP);
		{
            for (i=0; i<=int(qAbs(amp)); i++)
			{
				angle = sign*(double)i/500.0*PI;
				glVertex3d(radius*cos(angle),0.0,radius*sin(angle));
			}
		}
		glEnd();

		endx = radius*cos(angle);
		endz = radius*sin(angle);

		dx = 0.03;
		dz = 0.03*sign;

		xae = (radius-dx)*cos(angle) +dz *sin(angle);
		zae = (radius-dx)*sin(angle) -dz *cos(angle);
		glBegin(GL_LINES);
		{
			glVertex3d(endx, 0.0, endz);
			glVertex3d(xae,  0.0, zae);
		}
		glEnd();

		xae = (radius+dx)*cos(angle) +dz *sin(angle);
		zae = (radius+dx)*sin(angle) -dz *cos(angle);
		glBegin(GL_LINES);
		{
			glVertex3d(endx, 0.0, endz);
			glVertex3d(xae,  0.0, zae);
		}
		glEnd();

		//Resulting Rolling Moment Arc vector

		amp = 0.5*pWPolar->m_Density * pWPolar->m_WArea * pWPolar->m_WMAChord
						*pWOpp->m_QInf*pWOpp->m_QInf
						*pWOpp->m_GRm/3.0;

		amp *= pMiarex->m_LiftScale*factor;

		if (amp>0.0) sign = -1.0; else sign = 1.0;

		glBegin(GL_LINE_STRIP);
            for (i=0; i<=int(qAbs(amp)); i++)
			{
				angle = sign*(double)i/500.0*PI;
				glVertex3d(0.0,radius*cos(angle),radius*sin(angle));
			}
		glEnd();

		endy = radius*cos(angle);
		endz = radius*sin(angle);

		dy = 0.03;
		dz = 0.03*sign;

		yae = (radius-dy)*cos(angle) +dz *sin(angle);
		zae = (radius-dy)*sin(angle) -dz *cos(angle);
		glBegin(GL_LINES);
			glVertex3d(0.0, endy, endz);
			glVertex3d(0.0, yae,  zae);
		glEnd();

		yae = (radius+dy)*cos(angle) +dz *sin(angle);
		zae = (radius+dy)*sin(angle) -dz *cos(angle);
		glBegin(GL_LINES);
			glVertex3d(0.0, endy, endz);
			glVertex3d(0.0, yae,  zae);
		glEnd();

		//Resulting Yawing Moment Arc vector

		amp = 0.5*pWPolar->m_Density * pWPolar->m_WArea * pWPolar->m_WSpan
						*pWOpp->m_QInf*pWOpp->m_QInf
						*(pWOpp->m_GYm)/3.0;

		amp *= pMiarex->m_LiftScale*factor;

		if (amp>0.0) sign = -1.0; else sign = 1.0;

		glBegin(GL_LINE_STRIP);
		{
            for (i=0; i<=int(qAbs(amp)); i++)
			{
				angle = sign*(double)i/500.0*PI;
				glVertex3d(-radius*cos(angle),-radius*sin(angle),0.0);
			}
		}
		glEnd();

		endx = -radius*cos(angle);
		endy = -radius*sin(angle);

		dx =   0.03;
		dy =  -0.03*sign;

		xae = (-radius+dx)*cos(angle) +dy *sin(angle);
		yae = (-radius+dx)*sin(angle) -dy *cos(angle);
		glBegin(GL_LINES);
		{
			glVertex3d(endx, endy, 0.0);
			glVertex3d(xae,  yae, 0.0);
		}
		glEnd();

		xae = (-radius-dx)*cos(angle) +dy *sin(angle);
		yae = (-radius-dx)*sin(angle) -dy *cos(angle);
		glBegin(GL_LINES);
		{
			glVertex3d(endx, endy, 0.0);
			glVertex3d(xae,  yae, 0.0);
		}
		glEnd();

		glDisable (GL_LINE_STIPPLE);
	}
	glEndList();
}


void GLCreateLiftStrip(void *pQMiarex, Wing *pWing, WPolar *pWPolar, WingOpp *pWOpp, int List)
{
	if(!pWing || !pWPolar || !pWOpp) return;
	int i,j,k;
	int style, width;
	CVector C, CL;

	QColor color;

	double amp, yob, xt, yt, zt, dih;
	double cosa =  cos(pWOpp->m_Alpha * PI/180.0);
	double sina = -sin(pWOpp->m_Alpha * PI/180.0);
	
	QMiarex * pMiarex = (QMiarex*)pQMiarex;

	//LIFTLINE
	glNewList(List,GL_COMPILE);
	{
		pMiarex->m_GLList++;
        glEnable (GL_LINE_STIPPLE);

		color = pMiarex->m_XCPColor;
		style = pMiarex->m_XCPStyle;
		width = pMiarex->m_XCPWidth;

		if     (style == 1) 	glLineStipple (1, 0xCFCF);
		else if(style == 2) 	glLineStipple (1, 0x6666);
		else if(style == 3) 	glLineStipple (1, 0xFF18);
		else if(style == 4) 	glLineStipple (1, 0x7E66);
		else					glLineStipple (1, 0xFFFF);

		glColor3d(color.redF(), color.greenF(), color.blueF());
		glLineWidth((GLfloat)width);

		//dynamic pressure x area
		double q0 = 0.5 * pWPolar->m_Density * pWOpp->m_QInf * pWOpp->m_QInf;

		if(pWOpp)
		{
			if(pWOpp->m_AnalysisMethod==LLTMETHOD)
			{
				for (i=1; i<pWOpp->m_NStation; i++)
				{
					yob = 2.0*pWOpp->m_SpanPos[i]/pWOpp->m_Span;
					xt = pWing->Offset(yob) + pWOpp->m_XCPSpanRel[i]*pWOpp->m_Chord[i];
					pWing->GetViewYZPos(pWOpp->m_XCPSpanRel[i], pWOpp->m_SpanPos[i], yt, zt, 0);
					dih = -pWing->Dihedral(yob)*PI/180.0;
					amp = q0*pWOpp->m_Cl[i]*pWOpp->m_Chord[i]/pWOpp->m_MAChord;
					amp *= pMiarex->m_LiftScale/1000.0;

					glBegin(GL_LINES);
					{
						glVertex3d(xt, yt, zt);
						glVertex3d((xt + amp * cos(dih)*sina),
									yt + amp * sin(dih),
									zt + amp * cos(dih)*cosa);
					}
					glEnd();
				}
				glBegin(GL_LINE_STRIP);
				{
					for (i=1; i<pWOpp->m_NStation; i++)
					{
						yob = 2.0*pWOpp->m_SpanPos[i]/pWOpp->m_Span;
						xt = pWing->Offset(yob) + pWOpp->m_XCPSpanRel[i]*pWOpp->m_Chord[i];
						pWing->GetViewYZPos(pWOpp->m_XCPSpanRel[i], pWOpp->m_SpanPos[i],yt,zt,0);

						dih = -pWing->Dihedral(yob)*PI/180.0;
						amp = q0*pWOpp->m_Cl[i]*pWOpp->m_Chord[i]/pWOpp->m_MAChord;
						amp *= pMiarex->m_LiftScale/1000.0;

						glVertex3d(xt + amp * cos(dih)*sina,
								   yt + amp * sin(dih),
								   zt + amp * cos(dih)*cosa);
					}
				}
				glEnd();
			}
			else
			{
				i = 0;
				for (j=0; j<pWing->m_NSurfaces; j++)
				{
					for (k=0; k< pWing->m_Surface[j].m_NYPanels; k++)
					{
						pWing->m_Surface[j].GetLeadingPt(k, C);
						amp = pWOpp->m_Chord[i] / pWOpp->m_StripArea[i] / pWing->m_MAChord * pMiarex->m_LiftScale/1000.0;
						C.x += pWOpp->m_XCPSpanRel[i] * pWing->m_Surface[j].GetChord(k);

						glBegin(GL_LINES);
						{
							glVertex3d(C.x, C.y, C.z);
							glVertex3d(C.x + pWOpp->m_F[i].x*amp,//F is in Body axes
									 C.y + pWOpp->m_F[i].y*amp,
									 C.z + pWOpp->m_F[i].z*amp);
						}
						glEnd();
						i++;
					}
				}

				//Lift strip on each surface
				i = 0;
				for (j=0; j<pWing->m_NSurfaces; j++)
				{
					if(j>0 && pWing->m_Surface[j-1].m_bJoinRight)
					{
						//then connect strip to previous surface's last point
						glBegin(GL_LINES);
						{
							glVertex3d(CL.x, CL.y, CL.z);

							k=0;
							pWing->m_Surface[j].GetLeadingPt(k, C);
							amp = pWOpp->m_Chord[i] / pWOpp->m_StripArea[i] / pWing->m_MAChord * pMiarex->m_LiftScale/1000.0;
							C.x += pWOpp->m_XCPSpanRel[i] * pWing->m_Surface[j].GetChord(k);
							glVertex3d(C.x + pWOpp->m_F[i].x*amp,
									 C.y + pWOpp->m_F[i].y*amp,
									 C.z + pWOpp->m_F[i].z*amp);

						}
						glEnd();
					}
					glBegin(GL_LINE_STRIP);
					{
						for (k=0; k< pWing->m_Surface[j].m_NYPanels; k++)
						{
							pWing->m_Surface[j].GetLeadingPt(k, C);
							amp = pWOpp->m_Chord[i] / pWOpp->m_StripArea[i] / pWing->m_MAChord * pMiarex->m_LiftScale/1000.0;
							C.x += pWOpp->m_XCPSpanRel[i] * pWing->m_Surface[j].GetChord(k);
							CL.x = C.x + pWOpp->m_F[i].x*amp;
							CL.y = C.y + pWOpp->m_F[i].y*amp;
							CL.z = C.z + pWOpp->m_F[i].z*amp;
							glVertex3d(CL.x, CL.y, CL.z);
							i++;
						}
					}
					glEnd();
				}
			}
		}
		glDisable (GL_LINE_STIPPLE);
	}
	glEndList();
}


void GLCreateTrans(void *pQMiarex, Wing *pWing, WingOpp *pWOpp, int List)
{
    if(!pWing || !pWOpp) return;
    int i,j,k,m, style;
    double yrel, xt, yt, zt, yob ;
    CVector Pt;

    QMiarex*pMiarex = (QMiarex*)pQMiarex;

    //TOP TRANSITION
    glNewList(List,GL_COMPILE);
    {
        pMiarex->m_GLList++;

        glEnable (GL_LINE_STIPPLE);
        glLineWidth((GLfloat)pMiarex->m_XTopWidth);

        style = pMiarex->m_XTopStyle;
        if     (style == 1) 	glLineStipple (1, 0xCFCF);
        else if(style == 2) 	glLineStipple (1, 0x6666);
        else if(style == 3) 	glLineStipple (1, 0xFF18);
        else if(style == 4) 	glLineStipple (1, 0x7E66);
        else					glLineStipple (1, 0xFFFF);

        glColor3d(pMiarex->m_XTopColor.redF(),pMiarex->m_XTopColor.greenF(),pMiarex->m_XTopColor.blueF());

        glLineWidth((GLfloat)pMiarex->m_XTopWidth);
        if(pWOpp)
        {
            if(pWOpp->m_AnalysisMethod==LLTMETHOD)
            {
                glBegin(GL_LINE_STRIP);
                {
                    for (i=1; i<pWOpp->m_NStation; i++)
                    {
                        yob = 2.0*pWOpp->m_SpanPos[i]/pWOpp->m_Span;
                        xt = pWing->Offset(yob) + pWOpp->m_XTrTop[i]*pWOpp->m_Chord[i];
                        pWing->GetViewYZPos(pWOpp->m_XTrTop[i], pWOpp->m_SpanPos[i],yt,zt,0);

                        glVertex3d(xt,yt,zt);
                    }
                }
                glEnd();
            }
            else
            {
                if(!pWing->IsFin())
                {
                    glBegin(GL_LINE_STRIP);
                    {
                        m = 0;
                        for(j=0; j<pWing->m_NSurfaces; j++)
                        {
                            for(k=0; k<pWing->m_Surface[j].m_NYPanels; k++)
                            {
                                yrel = pWing->yrel(pWOpp->m_SpanPos[m]);
                                pWing->m_Surface[j].GetSurfacePoint(pWOpp->m_XTrTop[m],pWOpp->m_XTrTop[m],yrel,Pt,1);
                                glVertex3d(Pt.x, Pt.y, Pt.z);
                                m++;
                            }
                        }
                    }
                    glEnd();
                }
                else
                {
                    m = 0;
                    for(j=0; j<pWing->m_NSurfaces; j++)
                    {
                        glBegin(GL_LINE_STRIP);
                        {
                            for(k=0; k<pWing->m_Surface[j].m_NYPanels; k++)
                            {
                                yrel = pWing->yrel(pWOpp->m_SpanPos[m]);
                                pWing->m_Surface[j].GetSurfacePoint(pWOpp->m_XTrTop[m],pWOpp->m_XTrTop[m],yrel,Pt,1);
                                glVertex3d(Pt.x, Pt.y, Pt.z);

                                m++;
                            }
                        }
                        glEnd();
                    }
                }
            }
        }
        glDisable (GL_LINE_STIPPLE);
    }
    glEndList();

    //BOTTOM TRANSITION
    glNewList(List+MAXWINGS,GL_COMPILE);
    {
        pMiarex->m_GLList++;
        glEnable (GL_LINE_STIPPLE);
        glLineWidth((GLfloat)pMiarex->m_XBotWidth);

        style = pMiarex->m_XBotStyle;
        if     (style == 1) 	glLineStipple (1, 0xCFCF);
        else if(style == 2) 	glLineStipple (1, 0x6666);
        else if(style == 3) 	glLineStipple (1, 0xFF18);
        else if(style == 4) 	glLineStipple (1, 0x7E66);
        else					glLineStipple (1, 0xFFFF);

        glColor3d(pMiarex->m_XBotColor.redF(),pMiarex->m_XBotColor.greenF(),pMiarex->m_XBotColor.blueF());

        glLineWidth((GLfloat)pMiarex->m_XBotWidth);
        if(pWOpp)
        {
            if(pWOpp->m_AnalysisMethod==LLTMETHOD)
            {
                glBegin(GL_LINE_STRIP);
                {
                    for (i=1; i<pWOpp->m_NStation; i++)
                    {
                        yob = 2.0*pWOpp->m_SpanPos[i]/pWOpp->m_Span;
                        xt = pWing->Offset(yob) + pWOpp->m_XTrBot[i]*pWOpp->m_Chord[i];
                        pWing->GetViewYZPos(pWOpp->m_XTrBot[i], pWOpp->m_SpanPos[i],yt,zt,0);

                        glVertex3d(xt,yt, zt);
                    }
                }
                glEnd();
            }
            else
            {
                if(!pWing->IsFin())
                {
                    glBegin(GL_LINE_STRIP);
                    {
                        int m = 0;
                        for(j=0; j<pWing->m_NSurfaces; j++)
                        {
                            for(k=0; k<pWing->m_Surface[j].m_NYPanels; k++)
                            {
                                yrel = pWing->yrel(pWOpp->m_SpanPos[m]);
                                pWing->m_Surface[j].GetSurfacePoint(pWOpp->m_XTrBot[m],pWOpp->m_XTrBot[m],yrel,Pt,-1);
                                glVertex3d(Pt.x, Pt.y, Pt.z);
                                m++;
                            }
                        }
                    }
                    glEnd();
                }
                else
                {
                    int m = 0;
                    for(j=0; j<pWing->m_NSurfaces; j++)
                    {
                        glBegin(GL_LINE_STRIP);
                        {
                            for(k=0; k<pWing->m_Surface[j].m_NYPanels; k++)
                            {
                                yrel = pWing->yrel(pWOpp->m_SpanPos[m]);
                                pWing->m_Surface[j].GetSurfacePoint(pWOpp->m_XTrBot[m],pWOpp->m_XTrBot[m],yrel,Pt,-1);
                                glVertex3d(Pt.x, Pt.y, Pt.z);
                                m++;
                            }
                        }
                        glEnd();
                    }
                }
            }
        }
        glDisable (GL_LINE_STIPPLE);
    }
    glEndList();
}


void GLCreateStreamLines(void *pQMiarex, Wing *PlaneWing[MAXWINGS], CVector *pNode, WPolar *pWPolar, WingOpp *pWOpp)
{
    QMiarex * pMiarex = (QMiarex*)pQMiarex;
    MainFrame *pMainFrame = (MainFrame*)pMiarex->s_pMainFrame;

    if(!PlaneWing[0] || !pWOpp || !pWPolar || pWPolar->m_AnalysisMethod==LLTMETHOD  || !pMiarex->m_MatSize)
	{
		glNewList(VLMSTREAMLINES,GL_COMPILE); glEndList();
		pMiarex->m_GLList++;
		return;
	}

	double memcoresize = pMiarex->coreSize();
	pMiarex->SetCoreSize(0.0005); //mm, just for the time needed to build the streamlines

	Wing *pWing;

	QProgressDialog dlg("Streamlines calculation", "Abort", 0, pMiarex->m_MatSize, pMainFrame);
//	dlg.setWindowModality(Qt::WindowModal);

	GL3DScales *p3DScales = (GL3DScales *)pMainFrame->m_pGL3DScales;
	bool bFound;
	int i;
	int m, p, style,  iWing;
	double ds;
	float *Mu, *Sigma;
	QColor color;

	CVector C, D, D1, VA, VAT, VB, VBT, VT, VInf, TC, TD;
	CVector RefPoint(0.0,0.0,0.0);

	D1.Set(987654321.0, 0.0, 0.0);

	PlaneOpp *pPOpp = pMiarex->m_pCurPOpp;

	if(pMiarex->m_pCurPOpp)
	{
		Mu    = pPOpp->m_G;
		Sigma = pPOpp->m_Sigma;
	}
	else if (pWOpp)
	{
		Mu    = pWOpp->m_G;
		Sigma = pWOpp->m_Sigma;
	}
	else
	{
		Mu    = NULL;
		Sigma = NULL;
	}

	pMiarex->m_pPanelDlg->m_pWing   = PlaneWing[0];

	glNewList(VLMSTREAMLINES,GL_COMPILE);
	{
		pMiarex->m_GLList++;

		glEnable (GL_LINE_STIPPLE);

		color = pMiarex->m_StreamLinesColor;
		style = pMiarex->m_StreamLinesStyle;
//		width = pMiarex->m_StreamLinesWidth;

		glLineWidth(pMiarex->m_WakeWidth);

		style = pMiarex->m_WakeStyle;

		if     (style == Qt::DashLine)       glLineStipple (1, 0xCFCF);
		else if(style == Qt::DotLine)        glLineStipple (1, 0x6666);
		else if(style == Qt::DashDotLine)    glLineStipple (1, 0xFF18);
		else if(style == Qt::DashDotDotLine) glLineStipple (1, 0x7E66);
		else                                 glLineStipple (1, 0xFFFF);

		glColor3d(color.redF(), color.greenF(), color.blueF());

		VInf.Set(pWOpp->m_QInf,0.0,0.0);

		m = 0;

		for (iWing=0; iWing<MAXWINGS; iWing++)
		{
			if(PlaneWing[iWing])
			{
				pWing = PlaneWing[iWing];

				for (p=0; p<pWing->m_MatSize; p++)
				{
					bFound = false;

					if(p3DScales->m_pos==0 && pWing->m_pWingPanel[p].m_bIsLeading && pWing->m_pWingPanel[p].m_Pos<=MIDSURFACE)
					{
						C.Set(pNode[pWing->m_pWingPanel[p].m_iLA]);
						D.Set(pNode[pWing->m_pWingPanel[p].m_iLB]);
						bFound = true;
					}
					else if(p3DScales->m_pos==1 && pWing->m_pWingPanel[p].m_bIsTrailing && pWing->m_pWingPanel[p].m_Pos<=MIDSURFACE)
					{
						C.Set(pNode[pWing->m_pWingPanel[p].m_iTA]);
						D.Set(pNode[pWing->m_pWingPanel[p].m_iTB]);
						bFound = true;
					}
					else if(p3DScales->m_pos==2 && pWing->m_pWingPanel[p].m_bIsLeading && pWing->m_pWingPanel[p].m_Pos<=MIDSURFACE)
					{
						C.Set(0.0, pNode[pWing->m_pWingPanel[p].m_iLA].y, 0.0);
						D.Set(0.0, pNode[pWing->m_pWingPanel[p].m_iLB].y, 0.0);
						bFound = true;
					}

					if(bFound)
					{
						TC = C;
						TD = D;
						TC.RotateY(RefPoint, pWOpp->m_Alpha);
						TD.RotateY(RefPoint, pWOpp->m_Alpha);
						TC -= C;
						TD -= D;
                        if(p3DScales->m_pos==1 && qAbs(p3DScales->m_XOffset)<0.001 && qAbs(p3DScales->m_ZOffset)<0.001)
						{
							//apply Kutta's condition : initial speed vector is parallel to the T.E. bisector angle
							VA.Set(pNode[pWing->m_pWingPanel[p].m_iTA] - pNode[pWing->m_pWingPanel[p].m_iLA]);
							VA. Normalize();
							VB.Set(pNode[pWing->m_pWingPanel[p].m_iTB] - pNode[pWing->m_pWingPanel[p].m_iLB]);
							VB. Normalize();
							if(pWing->m_pWingPanel[p].m_Pos==BOTSURFACE)
							{
								//corresponding upper panel is the next one coming up
								for (i=p; i<pWing->m_MatSize;i++)
									if(pWing->m_pWingPanel[i].m_Pos>MIDSURFACE && pWing->m_pWingPanel[i].m_bIsTrailing) break;
								VAT = pNode[pWing->m_pWingPanel[i].m_iTA] - pNode[pWing->m_pWingPanel[i].m_iLA];
								VAT.Normalize();
								VA = VA+VAT;
								VA.Normalize();//VA is parallel to the bisector angle

								VBT = pNode[pWing->m_pWingPanel[i].m_iTB] - pNode[pWing->m_pWingPanel[i].m_iLB];
								VBT.Normalize();
								VB = VB+VBT;
								VB.Normalize();//VB is parallel to the bisector angle
							}
							VA.RotateY(pWOpp->m_Alpha);
							VB.RotateY(pWOpp->m_Alpha);
						}
						if(!C.IsSame(D1))
						{
							C.x += p3DScales->m_XOffset;
							C.z += p3DScales->m_ZOffset;

							ds = p3DScales->m_DeltaL;

							// One very special case is where we initiate the streamlines exactly at the T.E.
							// without offset either in X ou Z directions
//							V1.Set(0.0,0.0,0.0);
							glBegin(GL_LINE_STRIP);
							{
								glVertex3d(C.x+TC.x, C.y+TC.y, C.z+TC.z);
								C   += VA *ds;
								glVertex3d(C.x+TC.x, C.y+TC.y, C.z+TC.z);
								ds *= p3DScales->m_XFactor;

								for (i=1; i< p3DScales->m_NX ;i++)
								{
									pMiarex->m_pPanelDlg->GetSpeedVector(C, Mu, Sigma, VT);

									VT += VInf;
									VT.Normalize();
									C   += VT* ds;
									glVertex3d(C.x+TC.x, C.y+TC.y, C.z+TC.z);
									ds *= p3DScales->m_XFactor;
								}
							}
							glEnd();
						}

						D1 = D;
						D.x += p3DScales->m_XOffset;
						D.z += p3DScales->m_ZOffset;

						ds = p3DScales->m_DeltaL;

//						V1.Set(0.0,0.0,0.0);

						glBegin(GL_LINE_STRIP);
						{
							glVertex3d(D.x+TD.x, D.y+TD.y, D.z+TD.z);
							D   += VB *ds;
							glVertex3d(D.x+TD.x, D.y+TD.y, D.z+TD.z);
							ds *= p3DScales->m_XFactor;

							for (i=1; i< p3DScales->m_NX ;i++)
							{
								pMiarex->m_pPanelDlg->GetSpeedVector(D, Mu, Sigma, VT);

								VT += VInf;
								VT.Normalize();
								D   += VT* ds;
								glVertex3d(D.x+TD.x, D.y+TD.y, D.z+TD.z);
								ds *= p3DScales->m_XFactor;
							}
						}
						glEnd();
					}

					dlg.setValue(m);
					m++;

//					qApp->processEvents();
					if(dlg.wasCanceled()) break;
				}
				if(dlg.wasCanceled()) break;
			}
			if(dlg.wasCanceled()) break;
		}

		glDisable (GL_LINE_STIPPLE);
	}
	glEndList();

	//restore things as they were
	pMiarex->SetCoreSize(memcoresize);
}


void GLCreateSurfSpeeds(void *pQMiarex, Panel *pPanel, WPolar *pWPolar, WingOpp *pWOpp)
{
	QMiarex * pMiarex = (QMiarex*)pQMiarex;

    if(!pWPolar || !pWOpp || pWOpp->m_AnalysisMethod==LLTMETHOD || !pPanel || !pMiarex->m_MatSize)
	{
		glNewList(SURFACESPEEDS, GL_COMPILE);
		pMiarex->m_GLList++;
		glEndList();
		return;
	}
	
	QProgressDialog dlg("Streamlines calculation", "Abort", 0, pMiarex->m_MatSize, (MainFrame*)QMiarex::s_pMainFrame);

	int p, style;
	double factor;
	double length, sinT, cosT;

	float *Mu, *Sigma;
	double x1, x2, y1, y2, z1, z2, xe, ye, ze, dlx, dlz;
	CVector C, V, VT;
	CVector RefPoint(0.0,0.0,0.0);

	factor = pMiarex->m_VelocityScale/100.0;

	PlaneOpp *pPOpp = pMiarex->m_pCurPOpp;
	if(pMiarex->m_pCurPOpp)
	{
		Mu    = pPOpp->m_G;
		Sigma = pPOpp->m_Sigma;
	}
	else if (pWOpp)
	{
		Mu    = pWOpp->m_G;
		Sigma = pWOpp->m_Sigma;
	}
	else
	{
		Mu    = NULL;
		Sigma = NULL;
	}

	glNewList(SURFACESPEEDS, GL_COMPILE);
	{
		pMiarex->m_GLList++;

		glEnable (GL_LINE_STIPPLE);

		glLineWidth(pMiarex->m_WakeWidth);
		style = pMiarex->m_WakeStyle;
		
		if     (style == 1) 	glLineStipple (1, 0xCFCF);
		else if(style == 2) 	glLineStipple (1, 0x6666);
		else if(style == 3) 	glLineStipple (1, 0xFF18);
		else if(style == 4) 	glLineStipple (1, 0x7E66);
		else					glLineStipple (1, 0xFFFF);

		glColor3d(pMiarex->m_WakeColor.redF(), pMiarex->m_WakeColor.greenF(), pMiarex->m_WakeColor.blueF());

		for (p=0; p<pMiarex->m_MatSize; p++)
		{
			VT.Set(pWOpp->m_QInf,0.0,0.0);

			if(pWPolar->m_AnalysisMethod==PANELMETHOD)
			{
				if(pPanel[p].m_Pos==MIDSURFACE) C.Copy(pPanel[p].CtrlPt);
				else                            C.Copy(pPanel[p].CollPt);
				pMiarex->m_pPanelDlg->GetSpeedVector(C, Mu, Sigma, V);

				VT += V;
//					if(!pWPolar->m_bTiltedGeom)
					C.RotateY(RefPoint, pWOpp->m_Alpha);
					//Tilt the geometry w.r.t. sideslip
//						C.RotateZ(RefPoint, -pWOpp->m_Beta);
			}

			length = VT.VAbs()*factor;
			xe     = C.x+factor*VT.x;
			ye     = C.y+factor*VT.y;
			ze     = C.z+factor*VT.z;
			if(length>0.0)
			{
				cosT   = (xe-C.x)/length;
				sinT   = (ze-C.z)/length;
				dlx     = 0.15*length;
				dlz     = 0.07*length;
			}
			else
			{
				cosT   = 0.0;
				sinT   = 0.0;
				dlx    = 0.0;
				dlz    = 0.0;
			}

			x1 = xe -dlx*cosT - dlz*sinT;
			y1 = ye;
			z1 = ze -dlx*sinT + dlz*cosT;

			x2 = xe -dlx*cosT + dlz*sinT;
			y2 = ye;
			z2 = ze -dlx*sinT - dlz*cosT;

			glBegin(GL_LINES);
			{
				glVertex3d(C.x, C.y, C.z);
				glVertex3d(xe,ye,ze);
			}
			glEnd();

			glBegin(GL_LINES);
			{
				glVertex3d(xe, ye, ze);
				glVertex3d(x1, y1, z1);
			}
			glEnd();

			glBegin(GL_LINES);
			{
				glVertex3d(xe, ye, ze);
				glVertex3d(x2, y2, z2);
			}
			glEnd();

			dlg.setValue(p);
			if(dlg.wasCanceled()) break;
		}

		glDisable (GL_LINE_STIPPLE);
	}
	glEndList();

	dlg.hide();
}


/**  Creates an OpenGl list of pressure arrows on the panelss*/
void GLCreatePanelForce(void *pQMiarex, WPolar *pWPolar, WingOpp *pWOpp, PlaneOpp *pPOpp)
{
    QMiarex * pMiarex = (QMiarex*)pQMiarex;
    Panel *pPanel = pMiarex->m_Panel;

    if((!pWOpp && !pPOpp) || !pWPolar || !pPanel || !pMiarex->m_MatSize)
	{
		glNewList(PANELFORCEARROWS,GL_COMPILE);
		glEndList();
		glNewList(PANELFORCELEGENDTXT,GL_COMPILE);
		glEndList();
		return;
	}

	int p;
	float *Cp;
	double force, cosa, sina2, cosa2, color;
	double rmin, rmax, range;
	double coef = 1.;
	Quaternion Qt; // Quaternion operator to align the reference arrow to the panel's normal
	CVector Omega; //rotation vector to align the reference arrow to the panel's normal
	CVector O;
	//The vectors defining the reference arrow
	CVector R(0.0,0.0,1.0);
	CVector R1( 0.05, 0.0, -0.1);
	CVector R2(-0.05, 0.0, -0.1);
	//The three vectors defining the arrow on the panel
	CVector P, P1, P2;


	//define the range of values to set the colors in accordance
	rmin = 1.e10;
	rmax = -rmin;


	if(pPOpp) Cp = pPOpp->m_Cp;
	else      Cp = pWOpp->m_Cp;

	for (int p=0; p<pMiarex->m_MatSize; p++)
	{
		rmax = qMax(rmax, Cp[p] * pMiarex->m_Panel[p].GetArea());
		rmin = qMin(rmin, Cp[p] * pMiarex->m_Panel[p].GetArea());
	}


	rmin *= 0.5*pWPolar->m_Density *pWOpp->m_QInf*pWOpp->m_QInf  *pMiarex->m_LiftScale *coef;
	rmax *= 0.5*pWPolar->m_Density *pWOpp->m_QInf*pWOpp->m_QInf  *pMiarex->m_LiftScale *coef;
	range = rmax - rmin;


	glNewList(PANELFORCEARROWS, GL_COMPILE);
	{
		pMiarex->m_GLList++;
		glLineWidth(1.0);
//		glColor3d(pMiarex->m_XCPColor.redF(), pMiarex->m_XCPColor.greenF(), pMiarex->m_XCPColor.blueF());


		for (p=0; p<pMiarex->m_MatSize; p++)
		{
			// plot Cp? f? f/s=q.Cp?
			force = 0.5*pWPolar->m_Density *pWOpp->m_QInf*pWOpp->m_QInf * Cp[p]*pPanel[p].GetArea();
			force *= pMiarex->m_LiftScale *coef;
			color = (force-rmin)/range;
			glColor3d(GLGetRed(color),GLGetGreen(color),GLGetBlue(color));

			if(pWPolar->m_AnalysisMethod==VLMMETHOD) O = pPanel[p].CtrlPt;
			else                                     O = pPanel[p].CollPt;

			// Rotate the reference arrow to align it with the panel normal

			if(R==P)
			{
				Qt.Set(0.0, 0.0,0.0,1.0); //Null quaternion
			}
			else
			{
				cosa   = R.dot(pPanel[p].Normal);
				sina2  = sqrt((1.0 - cosa)*0.5);
				cosa2  = sqrt((1.0 + cosa)*0.5);

				Omega = R * pPanel[p].Normal;//crossproduct
				Omega.Normalize();
				Omega *=sina2;
				Qt.Set(cosa2, Omega.x, Omega.y, Omega.z);
			}

			Qt.Conjugate(R,  P);
			Qt.Conjugate(R1, P1);
			Qt.Conjugate(R2, P2);

			// Scale the pressure vector
			P  *= force;
			P1 *= force;
			P2 *= force;

			// Plot
			if(pPanel[p].m_Pos==MIDSURFACE)
			{
				glBegin(GL_LINES);
				{
					glVertex3d(O.x, O.y, O.z);
					glVertex3d(O.x+P.x, O.y+P.y, O.z+P.z);
				}
				glEnd();

				if(force>0)
				{
					glBegin(GL_LINES);
					{
						glVertex3d(O.x+P.x, O.y+P.y, O.z+P.z);
						glVertex3d(O.x+P.x+P1.x, O.y+P.y+P1.y, O.z+P.z+P1.z);
					}
					glEnd();
					glBegin(GL_LINES);
					{
						glVertex3d(O.x+P.x, O.y+P.y, O.z+P.z);
						glVertex3d(O.x+P.x+P2.x, O.y+P.y+P2.y, O.z+P.z+P2.z);
					}
					glEnd();
				}
				else
				{
					glBegin(GL_LINES);
					{
						glVertex3d(O.x+P.x, O.y+P.y, O.z+P.z);
						glVertex3d(O.x+P.x+P1.x, O.y+P.y+P1.y, O.z+P.z+P1.z);
					}
					glEnd();
					glBegin(GL_LINES);
					{
						glVertex3d(O.x+P.x, O.y+P.y, O.z+P.z);
						glVertex3d(O.x+P.x+P2.x, O.y+P.y+P2.y, O.z+P.z+P2.z);
					}
					glEnd();
				}
			}
			else
			{
				if(Cp[p]>0)
				{
					// compression, point towards the surface
//					P.Set(-P.x, -P.y, -P.z);
					glBegin(GL_LINES);
					{
						glVertex3d(O.x, O.y, O.z);
						glVertex3d(O.x+P.x, O.y+P.y, O.z+P.z);
					}
					glEnd();
					glBegin(GL_LINES);
					{
						glVertex3d(O.x, O.y, O.z);
						glVertex3d(O.x-P1.x, O.y-P1.y, O.z-P1.z);
					}
					glEnd();
					glBegin(GL_LINES);
					{
						glVertex3d(O.x, O.y, O.z);
						glVertex3d(O.x-P2.x, O.y-P2.y, O.z-P2.z);
					}
					glEnd();
				}
				else
				{
					// depression, point outwards from the surface
					P.Set(-P.x, -P.y, -P.z);
					glBegin(GL_LINES);
					{
						glVertex3d(O.x, O.y, O.z);
						glVertex3d(O.x+P.x, O.y+P.y, O.z+P.z);
					}
					glEnd();
					glBegin(GL_LINES);
					{
						glVertex3d(O.x+P.x, O.y+P.y, O.z+P.z);
						glVertex3d(O.x+P.x-P1.x, O.y+P.y-P1.y, O.z+P.z-P1.z);
					}
					glEnd();
					glBegin(GL_LINES);
					{
						glVertex3d(O.x+P.x, O.y+P.y, O.z+P.z);
						glVertex3d(O.x+P.x-P2.x, O.y+P.y-P2.y, O.z+P.z-P2.z);
					}
					glEnd();
				}
			}
		}
	}
	glEndList();
}


void GLDrawCpLegend(void *pQMiarex)
{
    int i;
    QMiarex *pMiarex = (QMiarex*)pQMiarex;
    ThreeDWidget *pGLWidget = (ThreeDWidget*)pMiarex->s_p3dWidget;
    QString strong;

    double labellength;

    double f;
    double range, delta;

    QFontMetrics fm(MainFrame::s_TextFont);
    int back = fm.averageCharWidth() * 5;

    double h = (double)pMiarex->m_r3DCltRect.height();
    double y0 = 2.*h/5.0;


    int ixPos, iyPos, dy;

    ixPos  = pMiarex->m_r3DCltRect.width()-back;

    dy     = (int) (h/40.0);
    iyPos  = (int) (y0 - 12.0*dy);


    range = (pMiarex->m_LegendMax - pMiarex->m_LegendMin);
    delta = range / 20;


    //glNewList(WOPPCPLEGENDTXT,GL_COMPILE);
    {
        //pMiarex->m_GLList++;
        glDisable(GL_LIGHTING);
        glDisable(GL_LIGHT0);

        glColor3d(MainFrame::s_TextColor.redF(),MainFrame::s_TextColor.greenF(),MainFrame::s_TextColor.blueF());

        strong = "Cp";
        labellength = fm.width(strong)+5;
        pGLWidget->renderText(ixPos-labellength, iyPos-dy, strong, MainFrame::s_TextFont);

        for (i=0; i<=20; i ++)
        {
            f = pMiarex->m_LegendMax - (double)i * delta;
            strong = QString("%1").arg(f, 5,'f',2);
            labellength = (fm.width(strong)+5);
            pGLWidget->renderText(ixPos-labellength, iyPos+i*dy, strong, MainFrame::s_TextFont);
        }

    }
        //glEndList();
}


void GLDrawPanelForceLegend(void *pQMiarex, WPolar *pWPolar)
{
	if(!pWPolar) return;
	QMiarex * pMiarex = (QMiarex*)pQMiarex;
	ThreeDWidget *pGLWidget = (ThreeDWidget*)pMiarex->s_p3dWidget;
	int p, i;
    int labellength;
    double f;
	double rmin, rmax, range, delta;

	QString strong, strForce;
	GetForceUnit(strForce, MainFrame::s_ForceUnit);

	Wing *pWingList[MAXWINGS];
	WingOpp *pWOppList[MAXWINGS];
	for(int ip=0; ip<MAXWINGS; ip++)
	{
		pWingList[ip] = pMiarex->m_pWingList[ip];
		pWOppList[ip] = pMiarex->m_pWOpp[ip];
	}


	//define the range of values to set the colors in accordance
	rmin = 1.e10;
	rmax = -rmin;
	for(int iw=0; iw<MAXWINGS; iw++)
	{
		if(pWingList[iw])
		{
			for (p=0; p<pWingList[iw]->m_MatSize; p++)
			{
				rmax = qMax(rmax,pWOppList[iw]->m_Cp[p]*pWingList[iw]->m_pWingPanel[p].GetArea());
				rmin = qMin(rmin,pWOppList[iw]->m_Cp[p]*pWingList[iw]->m_pWingPanel[p].GetArea());
			}
		}
	}
	rmin *= 0.5*pWPolar->m_Density *pWOppList[0]->m_QInf*pWOppList[0]->m_QInf  *MainFrame::s_NtoUnit;
	rmax *= 0.5*pWPolar->m_Density *pWOppList[0]->m_QInf*pWOppList[0]->m_QInf  *MainFrame::s_NtoUnit;
	range = rmax - rmin;


    QFontMetrics fm(MainFrame::s_TextFont);
    int back = fm.averageCharWidth() * 5;

    double h = (double)pMiarex->m_r3DCltRect.height();
    double y0 = 2.*h/5.0;


    int ixPos, iyPos, dy;

    ixPos  = pMiarex->m_r3DCltRect.width()-back;

    dy     = (int) (h/40.0);
    iyPos  = (int) (y0 - 12.0*dy);

	delta = range / 20.0;

    //glNewList(WOPPCPLEGENDTXT,GL_COMPILE);
    {
        //pMiarex->m_GLList++;
        glDisable(GL_LIGHTING);
        glDisable(GL_LIGHT0);

        glColor3d(MainFrame::s_TextColor.redF(),MainFrame::s_TextColor.greenF(),MainFrame::s_TextColor.blueF());


        labellength = fm.width(strong)+5;
        pGLWidget->renderText(ixPos-labellength, iyPos-dy, strForce, MainFrame::s_TextFont);

        for (i=0; i<=20; i ++)
        {
            f = rmin + (double)i * delta;
            strong = QString("%1").arg(f, 5,'f',2);
            labellength = (fm.width(strong)+5);
            pGLWidget->renderText(ixPos-labellength, iyPos+i*dy, strong, MainFrame::s_TextFont);
        }

    }
    //glEndList();}

}