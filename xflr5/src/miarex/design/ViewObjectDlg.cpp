/****************************************************************************

	ViewObjectDlg Class
	Copyright (C) 2015 Andre Deperrois adeperrois@xflr5.com

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

#include "../../misc/Units.h"
#include "../../misc/Settings.h"
#include "../../miarex/view/GLCreateLists.h"
#include "../../misc/GLLightDlg.h"
#include "../../misc/W3dPrefsDlg.h"
#include "../view/GLCreateBodyLists.h"

#include "ViewObjectDlg.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QTreeWidgetItem>
#include <QFontMetrics>
#include <QShowEvent>
#include <QHideEvent>
#include <QtDebug>

QSize ViewObjectDlg::s_WindowSize(911,783);
QPoint ViewObjectDlg::s_WindowPosition(131, 77);
bool ViewObjectDlg::s_bWindowMaximized =false;

#define SECTIONHIGHLIGHT    1702



ViewObjectDlg::ViewObjectDlg(QWidget *pParent) : QDialog(pParent)
{
	setWindowTitle("Plane object explorer");

	m_pPlane = NULL;
	m_pStruct = NULL;
	m_pDelegate = NULL;
	m_pModel = NULL;

	m_enumActiveWingType = XFLR5::OTHERWING;
	m_iActiveSection   = -1;
	m_iActiveFrame     = -1;


	m_bResetglSectionHighlight = true;
	m_bResetglPlane            = true;
	m_bResetglBody             = true;
	m_bChanged                 = false;

	setupLayout();
}


/**
 * Overrides the base class showEvent method. Moves the window to its former location.
 * @param event the showEvent.
 */
void ViewObjectDlg::showEvent(QShowEvent *event)
{
	move(s_WindowPosition);
	resize(s_WindowSize);

	if(s_bWindowMaximized) setWindowState(Qt::WindowMaximized);

	m_pGLWidget->updateGL();


	event->accept();
}


/**
 * Overrides the base class hideEvent method. Stores the window's current position.
 * @param event the hideEvent.
 */
void ViewObjectDlg::hideEvent(QHideEvent *event)
{
	s_WindowPosition = pos();
	event->accept();
}


void ViewObjectDlg::resizeEvent(QResizeEvent *event)
{
//	m_pStruct->setMinimumWidth(width()/2);
	int ColumnWidth = (int)((double)(m_pStruct->width())/6);
	m_pStruct->setColumnWidth(0,ColumnWidth*2);
	m_pStruct->setColumnWidth(1,ColumnWidth*2);
	m_pStruct->setColumnWidth(2,ColumnWidth);
	event->accept();
}


void ViewObjectDlg::setupLayout()
{
	QStringList labels;
	labels << tr("Object") << tr("Field")<<tr("Value")<<tr("Unit");

	m_pStruct = new QTreeView;

	m_pStruct->header()->setSectionResizeMode(QHeaderView::Interactive);
//	m_pPlaneStruct->header()->setDefaultSectionSize(239);
	m_pStruct->header()->setStretchLastSection(true);
	m_pStruct->header()->setDefaultAlignment(Qt::AlignCenter);

	m_pStruct->setEditTriggers(QAbstractItemView::AllEditTriggers);
	m_pStruct->setSelectionBehavior (QAbstractItemView::SelectRows);
//	m_pStruct->setIndentation(31);
	m_pStruct->setWindowTitle(tr("Objects"));

	m_pModel = new QStandardItemModel(this);
	m_pModel->setColumnCount(4);
	m_pModel->clear();
	m_pModel->setHorizontalHeaderLabels(labels);
	m_pStruct->setModel(m_pModel);


	m_pDelegate = new ViewObjectDelegate(this);
	m_pStruct->setItemDelegate(m_pDelegate);

	QFont fnt;
	QFontMetrics fm(fnt);
	m_pStruct->setColumnWidth(0, fm.averageCharWidth()*37);
	m_pStruct->setColumnWidth(1, fm.averageCharWidth()*29);
	m_pStruct->setColumnWidth(2, fm.averageCharWidth()*17);


	QItemSelectionModel *selectionModel = new QItemSelectionModel(m_pModel);
	m_pStruct->setSelectionModel(selectionModel);
	connect(selectionModel, SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(OnItemClicked(QModelIndex)));


	m_pDelegate = new ViewObjectDelegate(this);
	m_pStruct->setItemDelegate(m_pDelegate);
	connect(m_pDelegate,  SIGNAL(closeEditor(QWidget *)), this, SLOT(OnCellChanged(QWidget *)));


	QSizePolicy szPolicyMinimumExpanding;
	szPolicyMinimumExpanding.setHorizontalPolicy(QSizePolicy::MinimumExpanding);
	szPolicyMinimumExpanding.setVerticalPolicy(QSizePolicy::MinimumExpanding);

	QSizePolicy szPolicyMinimum;
	szPolicyMinimum.setHorizontalPolicy(QSizePolicy::Minimum);
	szPolicyMinimum.setVerticalPolicy(QSizePolicy::Minimum);

	QSizePolicy szPolicyPreferred;
	szPolicyPreferred.setHorizontalPolicy(QSizePolicy::Preferred);
	szPolicyPreferred.setVerticalPolicy(QSizePolicy::Preferred);

	QSizePolicy szPolicyFixed;
	szPolicyFixed.setHorizontalPolicy(QSizePolicy::Fixed);
	szPolicyFixed.setVerticalPolicy(QSizePolicy::Fixed);

	QHBoxLayout *pHLayout = new QHBoxLayout;
	{
		QVBoxLayout *pLeftSideLayout = new QVBoxLayout;
		{
			pLeftSideLayout->addWidget(m_pStruct);

			m_pctrlRedraw = new QPushButton(tr("Redraw plane"));
			pLeftSideLayout->addWidget(m_pctrlRedraw);

			QHBoxLayout *pCommandButtons = new QHBoxLayout;
			{
				pOKButton = new QPushButton(tr("Save and Close"));
				pOKButton->setAutoDefault(true);
				QPushButton *pCancelButton = new QPushButton(tr("Cancel"));
				pCancelButton->setAutoDefault(false);
				pCommandButtons->addWidget(pOKButton);
				pCommandButtons->addWidget(pCancelButton);
				connect(pOKButton, SIGNAL(clicked()),this, SLOT(OnOK()));
				connect(pCancelButton, SIGNAL(clicked()),this, SLOT(reject()));
			}
			pLeftSideLayout->addLayout(pCommandButtons);

		}

		QVBoxLayout *pRightSideLayout = new QVBoxLayout;
		{
			m_pGLWidget = new ThreeDWidget(this);
			m_pStruct->setSizePolicy(szPolicyMinimumExpanding);
			m_pGLWidget->m_iView = GLPLANEVIEW;
			QGroupBox *p3DCtrlBox = new QGroupBox;
			{
				QHBoxLayout *pThreeDViewControlsLayout = new QHBoxLayout;
				{
					QGridLayout *pThreeDParamsLayout = new QGridLayout;
					{
						m_pctrlAxes         = new QCheckBox(tr("Axes"), this);
		//				m_pctrlLight      = new QCheckBox(tr("Light"), this);
						m_pctrlSurfaces     = new QCheckBox(tr("Surfaces"), this);
						m_pctrlOutline      = new QCheckBox(tr("Outline"), this);
						m_pctrlPanels       = new QCheckBox(tr("Panels"), this);
						m_pctrlFoilNames    = new QCheckBox(tr("Foil Names"), this);
						m_pctrlVortices     = new QCheckBox(tr("Vortices"), this);
						m_pctrlPanelNormals = new QCheckBox(tr("Normals"), this);
						m_pctrlShowMasses       = new QCheckBox(tr("Masses"), this);

						pThreeDParamsLayout->addWidget(m_pctrlAxes, 1,1);
						pThreeDParamsLayout->addWidget(m_pctrlPanels, 1,2);
						pThreeDParamsLayout->addWidget(m_pctrlPanelNormals, 2,1);
						pThreeDParamsLayout->addWidget(m_pctrlVortices, 2,2);
						pThreeDParamsLayout->addWidget(m_pctrlSurfaces, 3,1);
						pThreeDParamsLayout->addWidget(m_pctrlOutline, 3,2);
						pThreeDParamsLayout->addWidget(m_pctrlFoilNames, 4,1);
						pThreeDParamsLayout->addWidget(m_pctrlShowMasses, 4,2);
					}

					QVBoxLayout *pRightColLayout = new QVBoxLayout;
					{
						QHBoxLayout *pAxisViewLayout = new QHBoxLayout;
						{
							m_pctrlX          = new QToolButton;
							m_pctrlY          = new QToolButton;
							m_pctrlZ          = new QToolButton;
							m_pctrlIso        = new QToolButton;
							if(m_pctrlX->iconSize().height()<=48)
							{
								m_pctrlX->setIconSize(QSize(24,24));
								m_pctrlY->setIconSize(QSize(24,24));
								m_pctrlZ->setIconSize(QSize(24,24));
								m_pctrlIso->setIconSize(QSize(24,24));
							}
							m_pXView   = new QAction(QIcon(":/images/OnXView.png"), tr("X View"), this);
							m_pYView   = new QAction(QIcon(":/images/OnYView.png"), tr("Y View"), this);
							m_pZView   = new QAction(QIcon(":/images/OnZView.png"), tr("Z View"), this);
							m_pIsoView = new QAction(QIcon(":/images/OnIsoView.png"), tr("Iso View"), this);
							m_pXView->setCheckable(true);
							m_pYView->setCheckable(true);
							m_pZView->setCheckable(true);
							m_pIsoView->setCheckable(true);

							m_pctrlX->setDefaultAction(m_pXView);
							m_pctrlY->setDefaultAction(m_pYView);
							m_pctrlZ->setDefaultAction(m_pZView);
							m_pctrlIso->setDefaultAction(m_pIsoView);
							pAxisViewLayout->addWidget(m_pctrlX);
							pAxisViewLayout->addWidget(m_pctrlY);
							pAxisViewLayout->addWidget(m_pctrlZ);
							pAxisViewLayout->addWidget(m_pctrlIso);
						}


						QHBoxLayout *pClipLayout = new QHBoxLayout;
						{
							QLabel *ClipLabel = new QLabel(tr("Clip:"));
							m_pctrlClipPlanePos = new QSlider(Qt::Horizontal);
							m_pctrlClipPlanePos->setMinimum(-300);
							m_pctrlClipPlanePos->setMaximum(300);
							m_pctrlClipPlanePos->setSliderPosition(0);
							m_pctrlClipPlanePos->setTickInterval(30);
							m_pctrlClipPlanePos->setTickPosition(QSlider::TicksBelow);
							pClipLayout->addWidget(ClipLabel);
							pClipLayout->addWidget(m_pctrlClipPlanePos,1);
						}

							m_pctrlReset = new QPushButton(tr("Reset view"));


						pRightColLayout->addLayout(pAxisViewLayout);
						pRightColLayout->addWidget(m_pctrlReset);
						pRightColLayout->addLayout(pClipLayout);
					}

					pThreeDViewControlsLayout->addLayout(pThreeDParamsLayout);
					pThreeDViewControlsLayout->addLayout(pRightColLayout);

				}
				p3DCtrlBox->setLayout(pThreeDViewControlsLayout);
			}
			pRightSideLayout->addWidget(m_pGLWidget);
			pRightSideLayout->addWidget(p3DCtrlBox);
		}
		pHLayout->addLayout(pLeftSideLayout);
		pHLayout->addLayout(pRightSideLayout);
	}
	setLayout(pHLayout);
	Connect();
//	resize(s_Size);
}



void ViewObjectDlg::OnOK()
{
	int j;

	m_pPlane->ComputePlane();


	//check the number of surfaces
	int nSurfaces = 0;
	for (j=0; j<m_pPlane->wing()->NWingSection()-1; j++)
	{
		if(qAbs(m_pPlane->wing()->YPosition(j)-m_pPlane->wing()->YPosition(j+1)) > Wing::s_MinPanelSize) nSurfaces+=2;
	}
	if(m_pPlane->stab())
	{
		for (j=0; j<m_pPlane->stab()->NWingSection()-1; j++)
		{
			if(qAbs(m_pPlane->stab()->YPosition(j)-m_pPlane->stab()->YPosition(j+1)) > Wing::s_MinPanelSize) nSurfaces+=2;
		}
	}

	if(m_pPlane->fin())
	{
		for (j=0; j<m_pPlane->fin()->NWingSection()-1; j++)
		{
			if(qAbs(m_pPlane->fin()->YPosition(j)-m_pPlane->fin()->YPosition(j+1)) > Wing::s_MinPanelSize)
			{
				if((m_pPlane->m_bSymFin) || (m_pPlane->m_bDoubleFin && m_pPlane->m_bDoubleSymFin))
					nSurfaces += 2;
				else
					nSurfaces += 1;
			}
		}
	}

	m_pPlane->ComputeBodyAxisInertia();

	s_bWindowMaximized= isMaximized();
	s_WindowPosition = pos();
	s_WindowSize = size();

	accept();
}



void ViewObjectDlg::initDialog(Plane *pPlane)
{
	m_pPlane = pPlane;
	m_pPlane->CreateSurfaces();
	fillPlaneTreeView();

	m_pctrlSurfaces->setChecked(ThreeDWidget::s_bSurfaces);
	m_pctrlOutline->setChecked(ThreeDWidget::s_bOutline);
	m_pctrlAxes->setChecked(ThreeDWidget::s_bAxes);
	m_pctrlPanels->setChecked(ThreeDWidget::s_bVLMPanels);
	m_pctrlFoilNames->setChecked(ThreeDWidget::s_bFoilNames);
	m_pctrlShowMasses->setChecked(ThreeDWidget::s_bShowMasses);
}




void ViewObjectDlg::keyPressEvent(QKeyEvent *event)
{
//	bool bShift = false;
//	bool bCtrl  = false;
//	if(event->modifiers() & Qt::ShiftModifier)   bShift =true;
//	if(event->modifiers() & Qt::ControlModifier) bCtrl =true;

	switch (event->key())
	{
		case Qt::Key_Return:
		case Qt::Key_Enter:
		{
			if(!pOKButton->hasFocus()) pOKButton->setFocus();
			else                       accept();

			break;
		}
		case Qt::Key_Escape:
		{
			reject();
			return;
		}

		default:
			event->ignore();
	}
}



void ViewObjectDlg::reject()
{
	s_bWindowMaximized= isMaximized();
	s_WindowPosition = pos();
	s_WindowSize = size();

	if(m_bChanged)
	{
		QString strong = tr("Save the changes ?");
		int Ans = QMessageBox::question(this, tr("Question"), strong,
										QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
		if (QMessageBox::Yes == Ans)
		{
			OnOK();
			return;
		}
		else if(QMessageBox::Cancel == Ans) return;
	}

//	reject();
	done(QDialog::Rejected);
}



void ViewObjectDlg::GLDraw3D()
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	m_pGLWidget->makeCurrent();
	glClearColor(Settings::s_BackgroundColor.redF(), Settings::s_BackgroundColor.greenF(), Settings::s_BackgroundColor.blueF(),0.0);


	if(m_bResetglSectionHighlight || m_bResetglPlane)
	{
		if(glIsList(SECTIONHIGHLIGHT))
		{
			glDeleteLists(SECTIONHIGHLIGHT,1);
		}
		if(m_iActiveSection>=0)
		{
			switch(m_enumActiveWingType)
			{
				case XFLR5::MAINWING:   GLCreateWingSectionHighlight(m_pPlane->wing());  break;
				case XFLR5::SECONDWING: GLCreateWingSectionHighlight(m_pPlane->wing2()); break;
				case XFLR5::ELEVATOR:   GLCreateWingSectionHighlight(m_pPlane->stab());  break;
				case XFLR5::FIN:        GLCreateWingSectionHighlight(m_pPlane->fin());   break;
				case XFLR5::OTHERWING: break;
			}
			m_bResetglSectionHighlight = false;
		}
		else if(m_iActiveFrame>=0)
		{
			GLCreateBodyFrameHighlight(m_pPlane->body(),m_pPlane->bodyPos(), m_iActiveFrame);
			m_bResetglSectionHighlight = false;
		}
	}

	if(m_pPlane->body())
	{
		if(m_bResetglPlane || m_bResetglBody)
		{
			if(glIsList(BODYGEOMBASE))
			{
				glDeleteLists(BODYGEOMBASE,1);
				glDeleteLists(BODYGEOMBASE+MAXBODIES,1);
			}
			if(m_pPlane->body()->bodyType()==XFLR5::BODYPANELTYPE)	     GLCreateBody3DFlatPanels(BODYGEOMBASE, m_pPlane->body());
			else if(m_pPlane->body()->bodyType()==XFLR5::BODYSPLINETYPE) GLCreateBody3DSplines(   BODYGEOMBASE, m_pPlane->body(), 47, 37);

			m_bResetglBody = false;
		}
	}

	if(m_bResetglPlane)
	{
		Wing *pWingList[MAXWINGS] = {m_pPlane->wing(), m_pPlane->wing2(), m_pPlane->stab(), m_pPlane->fin()};

		if(m_pPlane->body())
		{
			Body TranslatedBody;
			TranslatedBody.Duplicate(m_pPlane->body());
			TranslatedBody.Translate(m_pPlane->bodyPos());
			GLCreateGeom(WINGSURFACES, pWingList, &TranslatedBody);
		}
		else GLCreateGeom(WINGSURFACES, pWingList, NULL);

		if(glIsList(MESHPANELS))
		{
			glDeleteLists(MESHPANELS,2);
		}

		GLCreateMesh(pWingList);

		m_bResetglPlane = false;
	}
	QApplication::restoreOverrideCursor();
//	m_pctrlRedraw->setEnabled(true);

}



void ViewObjectDlg::GLRenderView()
{
	QString MassUnit;
	Units::getWeightUnitLabel(MassUnit);
	double glScaled = m_pGLWidget->m_glScaled;

	Wing *pWingList[MAXWINGS] = {m_pPlane->wing(), m_pPlane->wing2(), m_pPlane->stab(), m_pPlane->fin()};

	if(ThreeDWidget::s_bAxes)  m_pGLWidget->GLDrawAxes(1.0/glScaled, W3dPrefsDlg::s_3DAxisColor, W3dPrefsDlg::s_3DAxisStyle, W3dPrefsDlg::s_3DAxisWidth);

	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);

	if(ThreeDWidget::s_bOutline)
	{
		for(int iw=0;iw<MAXWINGS; iw++)
			if(pWingList[iw]) glCallList(WINGSURFACES+iw+4);

		if(m_pPlane->body())
		{
			glTranslated((m_pPlane)->bodyPos().x, 0.0, (m_pPlane)->bodyPos().z);
			glCallList(BODYGEOMBASE+MAXBODIES);
			glTranslated(-(m_pPlane)->bodyPos().x, 0.0, -(m_pPlane)->bodyPos().z);
		}
	}

	if(m_iActiveSection>=0 || m_iActiveFrame>=0)
	{
		glCallList(SECTIONHIGHLIGHT);
	}

	if(GLLightDlg::IsLightOn())
	{
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
	}
	else
	{
		glDisable(GL_LIGHTING);
		glDisable(GL_LIGHT0);
	}

	if(ThreeDWidget::s_bSurfaces)
	{
		for(int iw=0;iw<MAXWINGS; iw++)
			if(pWingList[iw]) glCallList(WINGSURFACES+iw);
		if(m_pPlane->body())
		{
			glTranslated((m_pPlane)->bodyPos().x, 0.0, (m_pPlane)->bodyPos().z);
			glCallList(BODYGEOMBASE);
			glTranslated(-(m_pPlane)->bodyPos().x, 0.0, -(m_pPlane)->bodyPos().z);
		}
	}

	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);

	if(ThreeDWidget::s_bVLMPanels)
	{
		if(!ThreeDWidget::s_bSurfaces) glCallList(MESHBACK);
		glCallList(MESHPANELS);
	}

	if(ThreeDWidget::s_bFoilNames)
	{
		for(int iw=0;iw<MAXWINGS; iw++)
			if(pWingList[iw]) m_pGLWidget->GLDrawFoils(pWingList[iw]);
	}

	if(ThreeDWidget::s_bShowMasses)
	{
		GLDrawMasses(m_pGLWidget, m_pPlane, m_pGLWidget->m_glScaled);
	}
}


void ViewObjectDlg::GLCreateBodyFrameHighlight(Body *pBody, CVector bodyPos, int iFrame)
{
	int k;
	CVector Point;
	double hinc, u, v;
	int nh,style, width;
	QColor color;

	if(iFrame<0)
	{
		glNewList(SECTIONHIGHLIGHT,GL_COMPILE);
		glEndList();
		return;
	}
	Frame *pFrame = pBody->getFrame(iFrame);

	nh = 23;
//	xinc = 0.1;
	hinc = 1.0/(double)(nh-1);


	//create 3D Splines or Lines to overlay on the body
	glNewList(SECTIONHIGHLIGHT,GL_COMPILE);
	{
		glEnable(GL_DEPTH_TEST);
		glEnable (GL_LINE_STIPPLE);

		color = QColor(0,0,255);
		style = 0;
		width = 3;
		glLineWidth(width);


		if     (style == Qt::DashLine)       glLineStipple (1, 0xCFCF);
		else if(style == Qt::DotLine)        glLineStipple (1, 0x6666);
		else if(style == Qt::DashDotLine)    glLineStipple (1, 0xFF18);
		else if(style == Qt::DashDotDotLine) glLineStipple (1, 0x7E66);
		else                                 glLineStipple (1, 0xFFFF);


		glColor3d(color.redF(), color.greenF(), color.blueF());

		if(pBody->bodyType() == XFLR5::BODYSPLINETYPE)
		{
			if(pBody->activeFrame())
			{
				u = pBody->Getu(pFrame->m_Position.x);

				glBegin(GL_LINE_STRIP);
				{
					v = 0.0;
					for (k=0; k<nh; k++)
					{
						pBody->GetPoint(u,v,true, Point);
						glVertex3d(Point.x+bodyPos.x, Point.y, Point.z+bodyPos.z);
						v += hinc;
					}
				}
				glEnd();
				glBegin(GL_LINE_STRIP);
				{
					v = 0.0;
					for (k=0; k<nh; k++)
					{
						pBody->GetPoint(u,v,false, Point);
						glVertex3d(Point.x+bodyPos.x, Point.y, Point.z+bodyPos.z);
						v += hinc;
					}
				}
				glEnd();
			}
		}
		else
		{
			glBegin(GL_LINE_STRIP);
			{
				for (k=0; k<pFrame->PointCount();k++)
					glVertex3d( pFrame->m_Position.x+bodyPos.x,
								pFrame->m_CtrlPoint[k].y, pFrame->m_CtrlPoint[k].z+bodyPos.z);
			}
			glEnd();
			glBegin(GL_LINE_STRIP);
			{
				for (k=0; k<pFrame->PointCount();k++)
					glVertex3d(pFrame->m_Position.x+bodyPos.x,
							  -pFrame->m_CtrlPoint[k].y, pFrame->m_CtrlPoint[k].z+bodyPos.z);
			}
			glEnd();
		}
		glDisable (GL_LINE_STIPPLE);
		glDisable(GL_DEPTH_TEST);
	}
	glEndList();
}



void ViewObjectDlg::GLCreateWingSectionHighlight(Wing *pWing)
{
	if(!pWing) return;

	int iSection = 0;
	int jSurf = 0;
	for(int jSection=0; jSection<pWing->NWingSection(); jSection++)
	{
		if(jSection==m_iActiveSection) break;
		if(qAbs(pWing->YPosition(jSection+1)-pWing->YPosition(jSection)) > Wing::s_MinPanelSize)
			iSection++;
	}

	glNewList(SECTIONHIGHLIGHT,GL_COMPILE);
	{
		glDisable(GL_DEPTH_TEST);
		glDisable (GL_LINE_STIPPLE);
		glColor3d(1.0, 0.0, 0.0);
		glLineWidth(3);

		if(iSection==0)
		{
			//define the inner left side surface
			if(!pWing->isFin())  jSurf = pWing->m_Surface.size()/2 - 1;
			else                 jSurf = pWing->m_Surface.size()   - 1;

			//plot B side outline
			glBegin(GL_LINE_STRIP);
			{
				for (int lx=0; lx<pWing->m_Surface.at(jSurf)->m_NXPanels; lx++)
				{
					pWing->m_Surface.at(jSurf)->GetPanel(pWing->m_Surface.at(jSurf)->m_NYPanels-1, lx, TOPSURFACE);
					glVertex3d(pWing->m_Surface.at(jSurf)->TB.x,
							   pWing->m_Surface.at(jSurf)->TB.y,
							   pWing->m_Surface.at(jSurf)->TB.z);
				}

				glVertex3d(pWing->m_Surface.at(jSurf)->LB.x,
						   pWing->m_Surface.at(jSurf)->LB.y,
						   pWing->m_Surface.at(jSurf)->LB.z);

				for (int lx=pWing->m_Surface.at(jSurf)->m_NXPanels-1; lx>=0; lx--)
				{
					pWing->m_Surface.at(jSurf)->GetPanel(pWing->m_Surface.at(jSurf)->m_NYPanels-1, lx, BOTSURFACE);
					glVertex3d(pWing->m_Surface.at(jSurf)->TB.x,
							   pWing->m_Surface.at(jSurf)->TB.y,
							   pWing->m_Surface.at(jSurf)->TB.z);
				}
			}
			glEnd();
		}
		else
		{
			if((pWing->m_bSymetric) && !pWing->m_bIsFin)
			{
				jSurf = pWing->m_Surface.size()/2 + iSection -1;

				glBegin(GL_LINE_STRIP);
				{
					for (int lx=0; lx<pWing->m_Surface.at(jSurf)->m_NXPanels; lx++)
					{
						pWing->m_Surface.at(jSurf)->GetPanel(pWing->m_Surface.at(jSurf)->m_NYPanels-1, lx, TOPSURFACE);
						glVertex3d(pWing->m_Surface.at(jSurf)->TB.x,
								 pWing->m_Surface.at(jSurf)->TB.y,
								 pWing->m_Surface.at(jSurf)->TB.z);
					}

					glVertex3d(pWing->m_Surface.at(jSurf)->LB.x,
							 pWing->m_Surface.at(jSurf)->LB.y,
							 pWing->m_Surface.at(jSurf)->LB.z);

					for (int lx=pWing->m_Surface.at(jSurf)->m_NXPanels-1; lx>=0; lx--)
					{
						pWing->m_Surface.at(jSurf)->GetPanel(pWing->m_Surface.at(jSurf)->m_NYPanels-1, lx, BOTSURFACE);
						glVertex3d(pWing->m_Surface.at(jSurf)->TB.x,
								 pWing->m_Surface.at(jSurf)->TB.y,
								 pWing->m_Surface.at(jSurf)->TB.z);
					}
				}
				glEnd();
			}

			if(pWing->m_bSymetric)
			{
				if(!pWing->m_bIsFin) jSurf = pWing->m_Surface.size()/2 - iSection;
				else                 jSurf = pWing->m_Surface.size()   - iSection;

				//plot A side outline
				glBegin(GL_LINE_STRIP);
				{
					for (int lx=0; lx<pWing->m_Surface.at(jSurf)->m_NXPanels; lx++)
					{
						pWing->m_Surface.at(jSurf)->GetPanel(0, lx, TOPSURFACE);
						glVertex3d(pWing->m_Surface.at(jSurf)->TA.x,
								   pWing->m_Surface.at(jSurf)->TA.y,
								   pWing->m_Surface.at(jSurf)->TA.z);
					}

					glVertex3d(pWing->m_Surface.at(jSurf)->LA.x,
							   pWing->m_Surface.at(jSurf)->LA.y,
							   pWing->m_Surface.at(jSurf)->LA.z);

					for (int lx=pWing->m_Surface.at(jSurf)->m_NXPanels-1; lx>=0; lx--)
					{
						pWing->m_Surface.at(jSurf)->GetPanel(0, lx, BOTSURFACE);
						glVertex3d(pWing->m_Surface.at(jSurf)->TA.x,
								   pWing->m_Surface.at(jSurf)->TA.y,
								   pWing->m_Surface.at(jSurf)->TA.z);
					}
				}
				glEnd();
			}
		}
	}
	glEndList();
}



void ViewObjectDlg::Connect()
{
	connect(m_pctrlIso,        SIGNAL(clicked()), m_pGLWidget, SLOT(On3DIso()));
	connect(m_pctrlX,          SIGNAL(clicked()), m_pGLWidget, SLOT(On3DFront()));
	connect(m_pctrlY,          SIGNAL(clicked()), m_pGLWidget, SLOT(On3DLeft()));
	connect(m_pctrlZ,          SIGNAL(clicked()), m_pGLWidget, SLOT(On3DTop()));

	connect(m_pctrlRedraw,     SIGNAL(clicked()), this, SLOT(OnRedraw()));
	connect(m_pctrlReset,      SIGNAL(clicked()), this, SLOT(On3DReset()));
	connect(m_pctrlFoilNames,  SIGNAL(clicked()), this, SLOT(OnFoilNames()));
	connect(m_pctrlShowMasses, SIGNAL(clicked()), this, SLOT(OnShowMasses()));

	connect(m_pctrlAxes,       SIGNAL(clicked()), this, SLOT(OnAxes()));
	connect(m_pctrlPanels,     SIGNAL(clicked()), this, SLOT(OnPanels()));
	connect(m_pctrlSurfaces,   SIGNAL(clicked()), this, SLOT(OnSurfaces()));
	connect(m_pctrlOutline,    SIGNAL(clicked()), this, SLOT(OnOutline()));
}


void ViewObjectDlg::OnSurfaces()
{
	ThreeDWidget::s_bSurfaces = m_pctrlSurfaces->isChecked();
	m_pGLWidget->updateGL();
}


void ViewObjectDlg::OnOutline()
{
	ThreeDWidget::s_bOutline = m_pctrlOutline->isChecked();
	m_pGLWidget->updateGL();
}


void ViewObjectDlg::OnPanels()
{
	ThreeDWidget::s_bVLMPanels = m_pctrlPanels->isChecked();
	m_pGLWidget->updateGL();
}

void ViewObjectDlg::On3DReset()
{
//	SetWingScale();
	m_pGLWidget->On3DReset();
}


void ViewObjectDlg::OnRedraw()
{
	m_pctrlRedraw->setEnabled(false);
	readPlaneTree();

	m_pPlane->CreateSurfaces();

	m_bResetglPlane = true;
	m_bChanged = true;
	m_pGLWidget->updateGL();
}


void ViewObjectDlg::OnAxes()
{
	ThreeDWidget::s_bAxes = m_pctrlAxes->isChecked();
	m_pGLWidget->updateGL();

}


void ViewObjectDlg::OnFoilNames()
{
	ThreeDWidget::s_bFoilNames = m_pctrlFoilNames->isChecked();
	m_pGLWidget->updateGL();

}



void ViewObjectDlg::OnShowMasses()
{
	ThreeDWidget::s_bShowMasses = m_pctrlShowMasses->isChecked();
	m_pGLWidget->updateGL();

}



bool ViewObjectDlg::IntersectObject(CVector AA,  CVector U, CVector &I)
{
	double dist=0.0;

	Wing *pWingList[MAXWINGS] = {m_pPlane->wing(), m_pPlane->wing2(), m_pPlane->stab(), m_pPlane->fin()};

	for(int iw=0; iw<MAXWINGS; iw++)
	{
		if(pWingList[iw])
		{
			for(int j=0; j<pWingList[iw]->m_Surface.size(); j++)
			{
				if ( Intersect(pWingList[iw]->m_Surface.at(j)->m_LA,
							   pWingList[iw]->m_Surface.at(j)->m_LB,
							   pWingList[iw]->m_Surface.at(j)->m_TA,
							   pWingList[iw]->m_Surface.at(j)->m_TB,
							   pWingList[iw]->m_Surface.at(j)->Normal,
							   AA, U, I, dist))
					return true;
			}
		}
	}

	/** @todo intersect body also */
	return false;
}


QList<QStandardItem *> ViewObjectDlg::prepareRow(const QString &object, const QString &field, const QString &value,  const QString &unit)
{
	QList<QStandardItem *> rowItems;
	rowItems << new QStandardItem(object)  << new QStandardItem(field)  << new QStandardItem(value) << new QStandardItem(unit);
	for(int ii=0; ii<rowItems.size(); ii++) rowItems.at(ii)->setData(XFLR5::STRING, Qt::UserRole);
	return rowItems;
}


QList<QStandardItem *> ViewObjectDlg::prepareBoolRow(const QString &object, const QString &field, const bool &value)
{
	QList<QStandardItem *> rowItems;
	rowItems.append(new QStandardItem(object));
	rowItems.append(new QStandardItem(field));
	rowItems.append(new QStandardItem);
	rowItems.at(2)->setData(value, Qt::DisplayRole);
	rowItems.append(new QStandardItem);

	rowItems.at(0)->setData(XFLR5::STRING, Qt::UserRole);
	rowItems.at(1)->setData(XFLR5::STRING, Qt::UserRole);
	rowItems.at(2)->setData(XFLR5::BOOL, Qt::UserRole);
	rowItems.at(3)->setData(XFLR5::STRING, Qt::UserRole);

	return rowItems;
}


QList<QStandardItem *> ViewObjectDlg::prepareIntRow(const QString &object, const QString &field, const int &value)
{
	QList<QStandardItem *> rowItems;
	rowItems.append(new QStandardItem(object));
	rowItems.append(new QStandardItem(field));
	rowItems.append(new QStandardItem);
	rowItems.at(2)->setData(value, Qt::DisplayRole);
	rowItems.append(new QStandardItem);

	rowItems.at(0)->setData(XFLR5::STRING, Qt::UserRole);
	rowItems.at(1)->setData(XFLR5::STRING, Qt::UserRole);
	rowItems.at(2)->setData(XFLR5::INTEGER, Qt::UserRole);
	rowItems.at(3)->setData(XFLR5::STRING, Qt::UserRole);

	return rowItems;
}


QList<QStandardItem *> ViewObjectDlg::prepareDoubleRow(const QString &object, const QString &field, const double &value,  const QString &unit)
{
	QList<QStandardItem *> rowItems;
	rowItems.append(new QStandardItem(object));
	rowItems.append(new QStandardItem(field));
	rowItems.append(new QStandardItem);
	rowItems.at(2)->setData(value, Qt::DisplayRole);
	rowItems.append(new QStandardItem(unit));

	rowItems.at(0)->setData(XFLR5::STRING, Qt::UserRole);
	rowItems.at(1)->setData(XFLR5::STRING, Qt::UserRole);
	rowItems.at(2)->setData(XFLR5::DOUBLE, Qt::UserRole);
	rowItems.at(3)->setData(XFLR5::STRING, Qt::UserRole);

	return rowItems;
}


void ViewObjectDlg::fillPlaneTreeView()
{
	QStandardItem *rootItem = m_pModel->invisibleRootItem();
	rootItem->setText(m_pPlane->planeName());

	QList<QStandardItem*> planeRootItem = prepareRow("Plane", "Name", m_pPlane->planeName());
	rootItem->appendRow(planeRootItem);

	QModelIndex ind = m_pModel->index(0,0);
	m_pStruct->expand(ind);

	fillPlaneMetaData(planeRootItem.first());

	fillBodyTreeView(planeRootItem.first());

	QList<QStandardItem*> wingFolder = prepareRow("Wings");
	planeRootItem.first()->appendRow(wingFolder);
	{
		QModelIndex ind = m_pModel->indexFromItem(wingFolder.first());
		m_pStruct->expand(ind);

		for(int iw=0; iw<MAXWINGS; iw++)
		{
			if(m_pPlane->wing(iw))
			{
				fillWingTreeView(iw, wingFolder);
			}
		}
	}
}



void ViewObjectDlg::fillPlaneMetaData(QStandardItem *item)
{
	QList<QStandardItem*> descriptionDataItem = prepareRow("Description", "Description", m_pPlane->planeDescription());
	item->appendRow(descriptionDataItem);

	if(m_pPlane->m_PointMass.size())
	{
		QList<QStandardItem*> PlaneInertiaFolder = prepareRow("Inertia");
		item->appendRow(PlaneInertiaFolder);
		{
			for(int iwm=0; iwm<m_pPlane->m_PointMass.size(); iwm++)
			{
				PointMass *pm = m_pPlane->m_PointMass.at(iwm);
				QList<QStandardItem*> planePointMassFolder = prepareRow("Point mass");

				PlaneInertiaFolder.first()->appendRow(planePointMassFolder);
				{
					QList<QStandardItem*> dataItem = prepareRow("", "Tag", pm->tag());
					dataItem.at(2)->setData(XFLR5::STRING, Qt::UserRole);
					planePointMassFolder.first()->appendRow(dataItem);

					dataItem = prepareRow("", "mass", QString("%1").arg(pm->mass()*Units::kgtoUnit()), Units::weightUnitLabel());
					dataItem.at(2)->setData(XFLR5::DOUBLE, Qt::UserRole);
					planePointMassFolder.first()->appendRow(dataItem);

					dataItem = prepareDoubleRow("", "x", pm->position().x*Units::mtoUnit(), Units::lengthUnitLabel());
					dataItem.at(2)->setData(XFLR5::DOUBLE, Qt::UserRole);
					planePointMassFolder.first()->appendRow(dataItem);

					dataItem = prepareDoubleRow("", "y", pm->position().y*Units::mtoUnit(), Units::lengthUnitLabel());
					dataItem.at(2)->setData(XFLR5::DOUBLE, Qt::UserRole);
					planePointMassFolder.first()->appendRow(dataItem);

					dataItem = prepareDoubleRow("", "z", pm->position().z*Units::mtoUnit(), Units::lengthUnitLabel());
					dataItem.at(2)->setData(XFLR5::DOUBLE, Qt::UserRole);
					planePointMassFolder.first()->appendRow(dataItem);
				}
			}
		}
	}
}



void ViewObjectDlg::fillWingTreeView(int iw, QList<QStandardItem*> &planeRootItem)
{
	Wing *pWing = m_pPlane->wing(iw);

	QList<QStandardItem*> wingFolder = prepareRow("Wing", "Type", wingType(pWing->wingType()));
	wingFolder.at(2)->setData(XFLR5::WINGTYPE, Qt::UserRole);
	planeRootItem.first()->appendRow(wingFolder);

	QList<QStandardItem*> dataItem = prepareRow("Name", "Name", pWing->wingName());
	wingFolder.first()->appendRow(dataItem);

	dataItem = prepareRow("Symetric", "Symetric", pWing->isSymetric() ? "true": "false");
	dataItem.at(2)->setData(XFLR5::BOOL, Qt::UserRole);
	wingFolder.first()->appendRow(dataItem);

	dataItem = prepareDoubleRow("Pitch angle", "Angle", m_pPlane->WingTiltAngle(iw),"");
	dataItem.at(2)->setData(XFLR5::DOUBLE, Qt::UserRole);
	wingFolder.first()->appendRow(dataItem);

	QList<QStandardItem*> wingColorFolder = prepareRow("Color");
	wingFolder.first()->appendRow(wingColorFolder);
	{
		dataItem = prepareIntRow("", "red", pWing->wingColor().red());
		wingColorFolder.first()->appendRow(dataItem);

		dataItem = prepareIntRow("", "green", pWing->wingColor().green());
		wingColorFolder.first()->appendRow(dataItem);

		dataItem = prepareIntRow("", "blue", pWing->wingColor().blue());
		wingColorFolder.first()->appendRow(dataItem);

		dataItem = prepareIntRow("", "alpha", pWing->wingColor().alpha());
		wingColorFolder.first()->appendRow(dataItem);
	}

	QList<QStandardItem*> finDataFolder = prepareRow("Fin data");
	wingFolder.first()->appendRow(finDataFolder);
	{
		QList<QStandardItem*> dataItem = prepareRow("", "is Fin:", pWing->isFin() ? "true": "false");
		dataItem.at(2)->setData(XFLR5::BOOL, Qt::UserRole);
		finDataFolder.first()->appendRow(dataItem);

		dataItem = prepareRow("", "is Symetric Fin:", pWing->isSymFin() ? "true": "false");
		dataItem.at(2)->setData(XFLR5::BOOL, Qt::UserRole);
		finDataFolder.first()->appendRow(dataItem);

		dataItem = prepareRow("", "is Double Fin:", pWing->isDoubleFin() ? "true": "false");
		dataItem.at(2)->setData(XFLR5::BOOL, Qt::UserRole);
		finDataFolder.first()->appendRow(dataItem);
	}

	QList<QStandardItem*> wingPositionFolder = prepareRow("Position");
	wingFolder.first()->appendRow(wingPositionFolder);
	{
		dataItem = prepareDoubleRow("", "x", m_pPlane->WingLE(iw).x*Units::mtoUnit(), Units::lengthUnitLabel());
		dataItem.at(2)->setData(XFLR5::DOUBLE, Qt::UserRole);
		wingPositionFolder.first()->appendRow(dataItem);

		dataItem = prepareDoubleRow("", "z", m_pPlane->WingLE(iw).z*Units::mtoUnit(), Units::lengthUnitLabel());
		dataItem.at(2)->setData(XFLR5::DOUBLE, Qt::UserRole);
		wingPositionFolder.first()->appendRow(dataItem);
	}

	QList<QStandardItem*> wingInertiaFolder = prepareRow("Inertia");
	wingFolder.first()->appendRow(wingInertiaFolder);
	{
		QList<QStandardItem*> dataItem = prepareDoubleRow( "", "Volume mass", pWing->volumeMass()*Units::kgtoUnit(), Units::weightUnitLabel());
		dataItem.at(2)->setData(XFLR5::DOUBLE, Qt::UserRole);
		wingInertiaFolder.first()->appendRow(dataItem);

		for(int iwm=0; iwm<pWing->m_PointMass.size(); iwm++)
		{
			PointMass *pm = pWing->m_PointMass.at(iwm);
			QList<QStandardItem*> wingPointMassFolder = prepareRow("Point mass");

			wingInertiaFolder.first()->appendRow(wingPointMassFolder);
			{
				QList<QStandardItem*> dataItem = prepareRow("", "Tag", pm->tag());
				dataItem.at(2)->setData(XFLR5::STRING, Qt::UserRole);
				wingPointMassFolder.first()->appendRow(dataItem);

				dataItem = prepareDoubleRow("", "mass", pm->mass()*Units::kgtoUnit(), Units::weightUnitLabel());
				dataItem.at(2)->setData(XFLR5::DOUBLE, Qt::UserRole);
				wingPointMassFolder.first()->appendRow(dataItem);

				dataItem = prepareDoubleRow("", "x", pm->position().x*Units::mtoUnit(), Units::lengthUnitLabel());
				dataItem.at(2)->setData(XFLR5::DOUBLE, Qt::UserRole);
				wingPointMassFolder.first()->appendRow(dataItem);

				dataItem = prepareDoubleRow("", "y", pm->position().y*Units::mtoUnit(), Units::lengthUnitLabel());;
				dataItem.at(2)->setData(XFLR5::DOUBLE, Qt::UserRole);
				wingPointMassFolder.first()->appendRow(dataItem);

				dataItem = prepareDoubleRow("", "z", pm->position().z*Units::mtoUnit(), Units::lengthUnitLabel());
				dataItem.at(2)->setData(XFLR5::DOUBLE, Qt::UserRole);
				wingPointMassFolder.first()->appendRow(dataItem);
			}
		}
	}

	QList<QStandardItem*> wingSectionFolder = prepareRow("Sections");
	wingFolder.first()->appendRow(wingSectionFolder);
	{
		for(int iws=0; iws<pWing->m_WingSection.size(); iws++)
		{
			WingSection *wingsec = pWing->m_WingSection.at(iws);

			QList<QStandardItem*> sectionFolder = prepareRow(QString("Section_%1").arg(iws+1));
			wingSectionFolder.first()->appendRow(sectionFolder);
			{
				dataItem = prepareDoubleRow("", "span position", wingsec->m_YPosition*Units::mtoUnit(), Units::lengthUnitLabel());
				sectionFolder.first()->appendRow(dataItem);

				dataItem = prepareDoubleRow("", "chord", wingsec->m_Chord*Units::mtoUnit(), Units::lengthUnitLabel());
				sectionFolder.first()->appendRow(dataItem);

				dataItem = prepareDoubleRow("", "offset", wingsec->m_Offset*Units::mtoUnit(), Units::lengthUnitLabel());
				sectionFolder.first()->appendRow(dataItem);

				dataItem = prepareDoubleRow("", "dihedral", wingsec->m_Dihedral, QString::fromUtf8("°"));
				sectionFolder.first()->appendRow(dataItem);

				dataItem = prepareDoubleRow("", "twist", wingsec->m_Twist, QString::fromUtf8("°"));
				sectionFolder.first()->appendRow(dataItem);

				dataItem = prepareIntRow("", "x-panels", wingsec->m_NXPanels);
				sectionFolder.first()->appendRow(dataItem);

				dataItem = prepareRow("", "x-distribution", distributionType(wingsec->m_XPanelDist));
				dataItem.at(2)->setData(XFLR5::PANELDISTRIBUTION, Qt::UserRole);
				sectionFolder.first()->appendRow(dataItem);

				dataItem = prepareIntRow("", "y-panels",wingsec->m_NYPanels);
				sectionFolder.first()->appendRow(dataItem);

				dataItem = prepareRow("", "y-distribution", distributionType(wingsec->m_YPanelDist));
				sectionFolder.first()->appendRow(dataItem);
				dataItem.at(2)->setData(XFLR5::PANELDISTRIBUTION, Qt::UserRole);

				dataItem = prepareRow("", "Left side foil name", wingsec->m_LeftFoilName.length() ? wingsec->m_LeftFoilName : "No left foil defined");
				dataItem.at(2)->setData(XFLR5::FOILNAME, Qt::UserRole);
				sectionFolder.first()->appendRow(dataItem);

				dataItem = prepareRow("", "Right side foil name", wingsec->m_LeftFoilName.length() ? wingsec->m_RightFoilName : "No left foil defined");
				dataItem.at(2)->setData(XFLR5::FOILNAME, Qt::UserRole);
				sectionFolder.first()->appendRow(dataItem);
			}
		}
	}
}



void ViewObjectDlg::fillBodyTreeView(QStandardItem*planeRootItem)
{
	Body *pBody = m_pPlane->body();

	QList<QStandardItem*> bodyFolder = prepareRow("Body", "Name");
	planeRootItem->appendRow(bodyFolder);

	QList<QStandardItem*> dataItem = prepareBoolRow("", "Active body", m_pPlane->body() ? true : false);
	bodyFolder.first()->appendRow(dataItem);

	if(!m_pPlane->body()) return;

	dataItem = prepareRow("", "Name", m_pPlane->body()->bodyName());
	bodyFolder.first()->appendRow(dataItem);

	dataItem = prepareRow("", "Type", bodyPanelType(pBody->bodyType()));
	dataItem.at(2)->setData(XFLR5::BODYTYPE, Qt::UserRole);
	bodyFolder.first()->appendRow(dataItem);

	QList<QStandardItem*> bodyColorFolder = prepareRow("Color");
	bodyFolder.first()->appendRow(bodyColorFolder);
	{
		QList<QStandardItem*> dataItem = prepareIntRow("", "red", pBody->bodyColor().red());
		bodyColorFolder.first()->appendRow(dataItem);

		dataItem = prepareIntRow("", "green", pBody->bodyColor().green());
		bodyColorFolder.first()->appendRow(dataItem);

		dataItem = prepareIntRow("", "blue", pBody->bodyColor().blue());
		bodyColorFolder.first()->appendRow(dataItem);

		dataItem = prepareIntRow("", "alpha", pBody->bodyColor().alpha());
		bodyColorFolder.first()->appendRow(dataItem);
	}

	QList<QStandardItem*> bodyPositionFolder = prepareRow("Position");
	bodyFolder.first()->appendRow(bodyPositionFolder);
	{
		QList<QStandardItem*> dataItem = prepareDoubleRow("", "x", m_pPlane->bodyPos().x*Units::mtoUnit(), Units::lengthUnitLabel());
		bodyPositionFolder.first()->appendRow(dataItem);

		dataItem = prepareDoubleRow("", "z", m_pPlane->bodyPos().z*Units::mtoUnit(), Units::lengthUnitLabel());
		bodyPositionFolder.first()->appendRow(dataItem);
	}

	QList<QStandardItem*> bodyInertiaFolder = prepareRow("Inertia");
	bodyFolder.first()->appendRow(bodyInertiaFolder);
	{
		QList<QStandardItem*> dataItem = prepareDoubleRow( "Structural (volume) mass", "", pBody->m_VolumeMass*Units::kgtoUnit(), Units::weightUnitLabel());
		bodyInertiaFolder.first()->appendRow(dataItem);

		for(int iwm=0; iwm<pBody->m_PointMass.size(); iwm++)
		{
			PointMass *pm = pBody->m_PointMass.at(iwm);
			QList<QStandardItem*> bodyPointMassFolder = prepareRow("Point mass");

			bodyInertiaFolder.first()->appendRow(bodyPointMassFolder);
			{
				QList<QStandardItem*> dataItem = prepareRow("", "Tag", pm->tag());
				bodyPointMassFolder.first()->appendRow(dataItem);

				dataItem = prepareDoubleRow("", "mass", pm->mass()*Units::kgtoUnit(), Units::weightUnitLabel());
				bodyPointMassFolder.first()->appendRow(dataItem);

				dataItem = prepareDoubleRow("", "x",pm->position().x*Units::mtoUnit(), Units::lengthUnitLabel());
				bodyPointMassFolder.first()->appendRow(dataItem);

				dataItem = prepareDoubleRow("", "y", pm->position().y*Units::mtoUnit(), Units::lengthUnitLabel());;
				bodyPointMassFolder.first()->appendRow(dataItem);

				dataItem = prepareDoubleRow("", "z", pm->position().z*Units::mtoUnit(), Units::lengthUnitLabel());
				bodyPointMassFolder.first()->appendRow(dataItem);
			}
		}
	}

	QList<QStandardItem*> NURBSFolder = prepareRow("NURBS");
	bodyFolder.first()->appendRow(NURBSFolder);
	{
		QList<QStandardItem*> dataItem = prepareIntRow("", "NURBS degree (lengthwise)", pBody->splineSurface()->uDegree());
		NURBSFolder.first()->appendRow(dataItem);

		dataItem = prepareIntRow("", "NURBS degree (hoop)", pBody->splineSurface()->vDegree());
		NURBSFolder.first()->appendRow(dataItem);

		dataItem = prepareIntRow("", "Mesh panels (lengthwise)", pBody->m_nxPanels);
		NURBSFolder.first()->appendRow(dataItem);

		dataItem = prepareIntRow("", "Mesh panels (hoop)", pBody->m_nhPanels);
		NURBSFolder.first()->appendRow(dataItem);
	}

	QList<QStandardItem*> hoopFolder = prepareRow("Hoop_panels (FLATPANELS case)");
	bodyFolder.first()->appendRow(hoopFolder);
	{
		for(int isl=0; isl<pBody->SideLineCount(); isl++)
		{
			QList<QStandardItem*> dataItem = prepareIntRow("", QString("Hoop panels in stripe %1").arg(isl+1), pBody->m_hPanels.at(isl));
			hoopFolder.first()->appendRow(dataItem);
		}
	}

	QList<QStandardItem*> bodyFrameFolder = prepareRow("Frames");
	bodyFolder.first()->appendRow(bodyFrameFolder);
	{
		for(int iFrame=0; iFrame<pBody->splineSurface()->frameCount(); iFrame++)
		{
			Frame *pFrame = pBody->splineSurface()->frameAt(iFrame);

			QList<QStandardItem*> sectionFolder = prepareRow(QString("Frame_%1").arg(iFrame+1));
			bodyFrameFolder.first()->appendRow(sectionFolder);
			{
				dataItem = prepareIntRow("", "Lengthwise panels (FLATPANELS case)", pBody->m_xPanels.at(iFrame));
				sectionFolder.first()->appendRow(dataItem);

				QList<QStandardItem*> positionFolder = prepareRow("Position");
				sectionFolder.first()->appendRow(positionFolder);
				{
					QList<QStandardItem*> dataItem = prepareDoubleRow("", "x", pFrame->m_Position.x*Units::mtoUnit(), Units::lengthUnitLabel());
					positionFolder.first()->appendRow(dataItem);

					dataItem = prepareDoubleRow("", "z", pFrame->m_Position.z*Units::mtoUnit(), Units::lengthUnitLabel());
					positionFolder.first()->appendRow(dataItem);
				}

				for(int iPt=0; iPt<pFrame->PointCount(); iPt++)
				{
					QList<QStandardItem*> pointFolder = prepareRow(QString("Point %1").arg(iPt));
					sectionFolder.first()->appendRow(pointFolder);
					{
						CVector Pt(pFrame->Point(iPt));
						QList<QStandardItem*> dataItem = prepareDoubleRow("", "x", Pt.x*Units::mtoUnit(), Units::lengthUnitLabel());
						pointFolder.first()->appendRow(dataItem);

						dataItem = prepareDoubleRow("", "y", Pt.y*Units::mtoUnit(), Units::lengthUnitLabel());
						pointFolder.first()->appendRow(dataItem);

						dataItem = prepareDoubleRow("", "z", Pt.z*Units::mtoUnit(), Units::lengthUnitLabel());
						pointFolder.first()->appendRow(dataItem);
					}
				}
			}
		}
	}
}



void ViewObjectDlg::readPlaneTree()
{
	readViewLevel(m_pModel->index(0,0));
}



void ViewObjectDlg::readViewLevel(QModelIndex indexLevel)
{
	QString object, field, value;
	QModelIndex dataIndex, subIndex;

	int iWing = 0;
	int iw=0;
	do
	{
		if(indexLevel.child(0,0).isValid())
		{
			object = indexLevel.sibling(indexLevel.row(),0).data().toString();
			field = indexLevel.sibling(indexLevel.row(),1).data().toString();
			value = indexLevel.sibling(indexLevel.row(),2).data().toString();

			if(object.compare("Body", Qt::CaseInsensitive)==0)
				readBodyTree(m_pPlane->body(), indexLevel.child(0,0));
			else if(object.compare("Wing", Qt::CaseInsensitive)==0)
			{
				Wing newWing;
				newWing.ClearPointMasses();
				newWing.ClearWingSections();
				newWing.ClearSurfaces();
				newWing.rWingName() = value;
				CVector wingPos;
				double wingTiltAngle;
				readWingTree(&newWing, wingPos, wingTiltAngle, indexLevel.child(0,0));


				if(newWing.isFin())
					iWing = 3;
				else if(iw==0)
					iWing = 0;
				else if(iw==1)
					iWing = 2;

				m_pPlane->m_Wing[iWing].Duplicate(&newWing);
				m_pPlane->WingLE(iWing)        = wingPos;
				m_pPlane->WingTiltAngle(iWing) = wingTiltAngle;
				iw++;
			}
			else readViewLevel(indexLevel.child(0,0));
		}
		indexLevel = indexLevel.sibling(indexLevel.row()+1,0);
	} while(indexLevel.isValid());
}


void ViewObjectDlg::readWingTree(Wing *pWing, CVector &pos, double &tiltAngle, QModelIndex indexLevel)
{
	QString object, field, value;

	do
	{
		if(indexLevel.child(0,0).isValid())
		{
			object = indexLevel.sibling(indexLevel.row(),0).data().toString();

			if(object.compare("Position", Qt::CaseInsensitive)==0)      readVectorTree(pos, indexLevel.child(0,0));
			else if(object.compare("Color", Qt::CaseInsensitive)==0)
			{
				QModelIndex subIndex = indexLevel.child(0,0);
				do
				{
					object = subIndex.sibling(subIndex.row(),0).data().toString();
					field = subIndex.sibling(subIndex.row(),1).data().toString();
					value = subIndex.sibling(subIndex.row(),2).data().toString();

					QModelIndex dataIndex = subIndex.sibling(subIndex.row(),2);

					if(field.compare("red", Qt::CaseInsensitive)==0)         pWing->wingColor().setRed(dataIndex.data().toInt());
					else if(field.compare("green", Qt::CaseInsensitive)==0)  pWing->wingColor().setGreen(dataIndex.data().toInt());
					else if(field.compare("blue", Qt::CaseInsensitive)==0)   pWing->wingColor().setBlue(dataIndex.data().toInt());
					else if(field.compare("alpha", Qt::CaseInsensitive)==0)  pWing->wingColor().setAlpha(dataIndex.data().toInt());

					subIndex = subIndex.sibling(subIndex.row()+1,0);
				}while(subIndex.isValid());
			}
			else if(object.compare("Fin data", Qt::CaseInsensitive)==0)
			{
				QModelIndex subIndex = indexLevel.child(0,0);
				do
				{
					object = subIndex.sibling(subIndex.row(),0).data().toString();
					field = subIndex.sibling(subIndex.row(),1).data().toString();
					value = subIndex.sibling(subIndex.row(),2).data().toString();

					if(field.compare("is Fin:", Qt::CaseInsensitive)==0)               pWing->isFin() = stringToBool(value);
					else if(field.compare("is Symetric Fin:", Qt::CaseInsensitive)==0) pWing->isSymFin() = stringToBool(value);
					else if(field.compare("is Double Fin:", Qt::CaseInsensitive)==0)   pWing->isDoubleFin() = stringToBool(value);

					subIndex = subIndex.sibling(subIndex.row()+1,0);
				}while(subIndex.isValid());
			}
			else if(object.compare("Inertia", Qt::CaseInsensitive)==0)  readInertiaTree(pWing->volumeMass(), pWing->m_PointMass, indexLevel.child(0,0));
			else if(object.compare("Sections", Qt::CaseInsensitive)==0)
			{
				QModelIndex subIndex = indexLevel.child(0,0);
				do
				{
					readWingSectionTree(pWing, subIndex.child(0,0));
					subIndex = subIndex.sibling(subIndex.row()+1,0);
				}while(subIndex.isValid());
			}
		}
		else
		{
			//no more children
			QString object = indexLevel.sibling(indexLevel.row(),0).data().toString();
			QString field = indexLevel.sibling(indexLevel.row(),1).data().toString();
			QString value = indexLevel.sibling(indexLevel.row(),2).data().toString();

			if     (field.compare("Name", Qt::CaseInsensitive)==0)     pWing->rWingName() = value;
			else if(field.compare("Type", Qt::CaseInsensitive)==0)     pWing->wingType() = wingType(value);
			else if(field.compare("Angle", Qt::CaseInsensitive)==0)    tiltAngle = value.toDouble();
			else if(field.compare("Symetric", Qt::CaseInsensitive)==0) pWing->isSymetric() = stringToBool(value);
		}

		indexLevel = indexLevel.sibling(indexLevel.row()+1,0);

	} while(indexLevel.isValid());
}



void ViewObjectDlg::readBodyTree(Body *pBody, QModelIndex indexLevel)
{
	if(!pBody) return;

	QString object, field, value;
	QModelIndex dataIndex, subIndex;

	do
	{
		if(indexLevel.child(0,0).isValid())
		{
			object = indexLevel.sibling(indexLevel.row(),0).data().toString();
			field = indexLevel.sibling(indexLevel.row(),1).data().toString();
			value = indexLevel.sibling(indexLevel.row(),2).data().toString();

			if(object.compare("Position", Qt::CaseInsensitive)==0) readVectorTree(m_pPlane->bodyPos(), indexLevel.child(0,0));
			else if(object.compare("Color", Qt::CaseInsensitive)==0)
			{
				subIndex = indexLevel.child(0,0);
				do
				{
					object = subIndex.sibling(subIndex.row(),0).data().toString();
					field = subIndex.sibling(subIndex.row(),1).data().toString();
					value = subIndex.sibling(subIndex.row(),2).data().toString();

					dataIndex = subIndex.sibling(subIndex.row(),2);

					if(field.compare("red", Qt::CaseInsensitive)==0)         pBody->bodyColor().setRed(dataIndex.data().toInt());
					else if(field.compare("green", Qt::CaseInsensitive)==0)  pBody->bodyColor().setGreen(dataIndex.data().toInt());
					else if(field.compare("blue", Qt::CaseInsensitive)==0)   pBody->bodyColor().setBlue(dataIndex.data().toInt());
					else if(field.compare("alpha", Qt::CaseInsensitive)==0)  pBody->bodyColor().setAlpha(dataIndex.data().toInt());

					subIndex = subIndex.sibling(subIndex.row()+1,0);
				}while(subIndex.isValid());
			}
			else if(object.compare("Inertia", Qt::CaseInsensitive)==0) 	readInertiaTree(pBody->volumeMass(), pBody->m_PointMass, indexLevel.child(0,0));
			else if(object.compare("NURBS", Qt::CaseInsensitive)==0)
			{
				subIndex = indexLevel.child(0,0);
				do
				{
					object = subIndex.sibling(subIndex.row(),0).data().toString();
					field = subIndex.sibling(subIndex.row(),1).data().toString();
					value = subIndex.sibling(subIndex.row(),2).data().toString();

					dataIndex = subIndex.sibling(subIndex.row(),2);

					if(field.compare("NURBS degree (lengthwise)", Qt::CaseInsensitive)==0)     pBody->splineSurface()->SetuDegree(dataIndex.data().toInt());
					else if(field.compare("NURBS degree (hoop)", Qt::CaseInsensitive)==0)      pBody->splineSurface()->SetvDegree(dataIndex.data().toInt());
					else if(field.compare("Mesh panels (lengthwise)", Qt::CaseInsensitive)==0) pBody->m_nxPanels = dataIndex.data().toInt();
					else if(field.compare("Mesh panels (hoop)", Qt::CaseInsensitive)==0)       pBody->m_nhPanels = dataIndex.data().toInt();

					subIndex = subIndex.sibling(subIndex.row()+1,0);
				}
				while(subIndex.isValid());
			}
			else if(object.compare("Hoop_panels (FLATPANELS case)", Qt::CaseInsensitive)==0)
			{
				subIndex = indexLevel.child(0,0);
				do
				{
					object = subIndex.sibling(subIndex.row(),0).data().toString();
					field = subIndex.sibling(subIndex.row(),1).data().toString();
					value = subIndex.sibling(subIndex.row(),2).data().toString();
					dataIndex = subIndex.sibling(subIndex.row(),2);

					int idx = field.right(field.length()-22).toInt()-1;
					pBody->m_hPanels[idx] =  dataIndex.data().toInt();

					subIndex = subIndex.sibling(subIndex.row()+1,0);
				}
				while(subIndex.isValid());
			}
			else if(object.compare("Frames", Qt::CaseInsensitive)==0)
			{
				pBody->m_SplineSurface.ClearFrames();
				QModelIndex subIndex = indexLevel.child(0,0);
				do
				{
					object = subIndex.sibling(subIndex.row(),0).data().toString();
					if(object.indexOf("Frame_")>=0)
					{
						Frame *pFrame = new Frame;
						readBodyFrameTree(pBody, pFrame, subIndex.child(0,0));
						pBody->m_SplineSurface.appendFrame(pFrame);
					}

					subIndex = subIndex.sibling(subIndex.row()+1,0);
				}
				while(subIndex.isValid());
			}
		}
		else
		{
			//no more children
			object = indexLevel.sibling(indexLevel.row(),0).data().toString();
			field = indexLevel.sibling(indexLevel.row(),1).data().toString();
			value = indexLevel.sibling(indexLevel.row(),2).data().toString();

			dataIndex = indexLevel.sibling(indexLevel.row(),2);

			if     (field.compare("Name", Qt::CaseInsensitive)==0)     pBody->bodyName() = value;
			else if(field.compare("Type", Qt::CaseInsensitive)==0)     pBody->bodyType() = XFLR5::BODYSPLINETYPE; /** @todo improve */
			else if(field.compare("Active body", Qt::CaseInsensitive)==0)
				m_pPlane->hasBody() = stringToBool(value);
		}

		indexLevel = indexLevel.sibling(indexLevel.row()+1,0);

	} while(indexLevel.isValid());
}


void ViewObjectDlg::readBodyFrameTree(Body *pBody, Frame *pFrame, QModelIndex indexLevel)
{
	QString object, field, value;
	QModelIndex dataIndex;

	do
	{
		object = indexLevel.sibling(indexLevel.row(),0).data().toString();
		field = indexLevel.sibling(indexLevel.row(),1).data().toString();
		value = indexLevel.sibling(indexLevel.row(),2).data().toString();

		dataIndex = indexLevel.sibling(indexLevel.row(),2);

		if (field.compare("Lengthwise panels (FLATPANELS case)", Qt::CaseInsensitive)==0)   pBody->m_xPanels.append(dataIndex.data().toInt());
		else if (object.compare("Position", Qt::CaseInsensitive)==0) readVectorTree(pFrame->m_Position, indexLevel.child(0,0));
		else if (object.indexOf("Point", Qt::CaseInsensitive)==0)
		{
			CVector Pt;
			readVectorTree(Pt, indexLevel.child(0,0));
			pFrame->AppendPoint(Pt);
		}
		indexLevel = indexLevel.sibling(indexLevel.row()+1,0);
	} while(indexLevel.isValid());

}


void ViewObjectDlg::readWingSectionTree(Wing *pWing, QModelIndex indexLevel)
{
	QString object, field, value;
	QModelIndex dataIndex;

	// not expecting any more children
	WingSection *pWS = new WingSection;
	do
	{
		object = indexLevel.sibling(indexLevel.row(),0).data().toString();
		field = indexLevel.sibling(indexLevel.row(),1).data().toString();
		value = indexLevel.sibling(indexLevel.row(),2).data().toString();

//qDebug()<<object<<field<<value;

		dataIndex = indexLevel.sibling(indexLevel.row(),2);

		if (field.compare("span position", Qt::CaseInsensitive)==0)             pWS->m_YPosition     = dataIndex.data().toDouble()/Units::mtoUnit();
		else if (field.compare("chord", Qt::CaseInsensitive)==0)                pWS->m_Chord         = dataIndex.data().toDouble()/Units::mtoUnit();
		else if (field.compare("offset", Qt::CaseInsensitive)==0)               pWS->m_Offset        = dataIndex.data().toDouble()/Units::mtoUnit();
		else if (field.compare("dihedral", Qt::CaseInsensitive)==0)             pWS->m_Dihedral      = dataIndex.data().toDouble();
		else if (field.compare("twist", Qt::CaseInsensitive)==0)                pWS->m_Twist         = dataIndex.data().toDouble();
		else if (field.compare("x-panels", Qt::CaseInsensitive)==0)             pWS->m_NXPanels      = dataIndex.data().toInt();
		else if (field.compare("y-panels", Qt::CaseInsensitive)==0)             pWS->m_NYPanels      = dataIndex.data().toInt();
		else if (field.compare("x-distribution", Qt::CaseInsensitive)==0)       pWS->m_XPanelDist    = distributionType(value);
		else if (field.compare("y-distribution", Qt::CaseInsensitive)==0)       pWS->m_YPanelDist    = distributionType(value);
		else if (field.compare("Left side foil name", Qt::CaseInsensitive)==0)  pWS->m_LeftFoilName  = value;
		else if (field.compare("Right side foil name", Qt::CaseInsensitive)==0) pWS->m_RightFoilName = value;

		indexLevel = indexLevel.sibling(indexLevel.row()+1,0);
	} while(indexLevel.isValid());

	pWing->m_WingSection.append(pWS);

}



void ViewObjectDlg::readInertiaTree(double &volumeMass, QList<PointMass*>&pointMasses, QModelIndex indexLevel)
{
	pointMasses.clear();

	QString object, field, value;
	QModelIndex dataIndex;
	do
	{
		if(indexLevel.child(0,0).isValid())
		{
			object = indexLevel.sibling(indexLevel.row(),0).data().toString();
			if(object.compare("Point mass", Qt::CaseInsensitive)==0)
			{
				PointMass *ppm = new PointMass;
				readPointMassTree(ppm, indexLevel.child(0,0));
				pointMasses.append(ppm);
			}
		}
		else
		{
			//no more children
			object = indexLevel.sibling(indexLevel.row(),0).data().toString();
			field = indexLevel.sibling(indexLevel.row(),1).data().toString();
			value = indexLevel.sibling(indexLevel.row(),2).data().toString();
			dataIndex = indexLevel.sibling(indexLevel.row(),2);

			if     (field.compare("Volume Mass", Qt::CaseInsensitive)==0)   volumeMass = dataIndex.data().toDouble()/Units::kgtoUnit();
		}

		indexLevel = indexLevel.sibling(indexLevel.row()+1,0);

	} while(indexLevel.isValid());
}


void ViewObjectDlg::readPointMassTree(PointMass *ppm, QModelIndex indexLevel)
{
	QString object, field, value;
	QModelIndex dataIndex;
	// not expecting any more children
	do
	{
		object = indexLevel.sibling(indexLevel.row(),0).data().toString();
		field = indexLevel.sibling(indexLevel.row(),1).data().toString();
		value = indexLevel.sibling(indexLevel.row(),2).data().toString();
		dataIndex = indexLevel.sibling(indexLevel.row(),2);

		if      (field.compare("mass", Qt::CaseInsensitive)==0) ppm->mass() = dataIndex.data().toDouble()/Units::kgtoUnit();
		else if (field.compare("tag",  Qt::CaseInsensitive)==0) ppm->tag()  = value;
		else if (field.compare("x",    Qt::CaseInsensitive)==0) ppm->position().x = dataIndex.data().toDouble()/Units::mtoUnit();
		else if (field.compare("y",    Qt::CaseInsensitive)==0) ppm->position().y = dataIndex.data().toDouble()/Units::mtoUnit();
		else if (field.compare("z",    Qt::CaseInsensitive)==0) ppm->position().z = dataIndex.data().toDouble()/Units::mtoUnit();

		indexLevel = indexLevel.sibling(indexLevel.row()+1,0);

	} while(indexLevel.isValid());
}



void ViewObjectDlg::readVectorTree(CVector &V, QModelIndex indexLevel)
{
	QString object, field, value;
	QModelIndex dataIndex;
	// not expecting any more children
	do
	{
		object = indexLevel.sibling(indexLevel.row(),0).data().toString();
		field = indexLevel.sibling(indexLevel.row(),1).data().toString();
		value = indexLevel.sibling(indexLevel.row(),2).data().toString();
		dataIndex = indexLevel.sibling(indexLevel.row(),2);

		if (field.compare("x", Qt::CaseInsensitive)==0)      V.x = dataIndex.data().toDouble()/Units::mtoUnit();
		else if (field.compare("y", Qt::CaseInsensitive)==0) V.y = dataIndex.data().toDouble()/Units::mtoUnit();
		else if (field.compare("z", Qt::CaseInsensitive)==0) V.z = dataIndex.data().toDouble()/Units::mtoUnit();

		indexLevel = indexLevel.sibling(indexLevel.row()+1,0);
	} while(indexLevel.isValid());

}



void ViewObjectDlg::OnItemClicked(const QModelIndex &index)
{
	highlightSelection(index);
	m_pGLWidget->updateGL();
}




void ViewObjectDlg::OnCellChanged(QWidget *)
{
	m_bChanged = true;
	m_pGLWidget->updateGL();
}




void ViewObjectDlg::highlightSelection(const QModelIndex &indexSel)
{
	// we highlight wing sections and body frames
	// so check if the user's selection is one of these
	QModelIndex indexLevel = indexSel;
	QString object, value;
	do
	{
		object = indexLevel.sibling(indexLevel.row(),0).data().toString();

		if(object.indexOf("Section_", 0, Qt::CaseInsensitive)>=0)
		{
			m_iActiveSection = object.right(object.length()-8).toInt() -1;

			//get the wing identification
			indexLevel = indexLevel.parent();
			do
			{
				object = indexLevel.sibling(indexLevel.row(),0).data().toString();

				if(object.compare("Wing", Qt::CaseInsensitive)==0)
				{
					object = indexLevel.sibling(indexLevel.row(),0).data().toString();
					m_enumActiveWingType = wingType(indexLevel.sibling(indexLevel.row(),2).data().toString());
				}
				indexLevel = indexLevel.parent();
			} while(indexLevel.isValid());




			m_iActiveFrame = -1;
			m_bResetglSectionHighlight = true;
			return;
		}
		else if(object.indexOf("Frame_", 0, Qt::CaseInsensitive)>=0)
		{
			m_iActiveFrame = object.right(object.length()-6).toInt() -1;
			m_iActiveSection = -1;
			m_bResetglSectionHighlight = true;
			return;
		}
		indexLevel = indexLevel.parent();
	} while(indexLevel.isValid());

}














