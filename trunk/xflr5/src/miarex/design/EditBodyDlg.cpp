/****************************************************************************

	EditBodyDlg Class
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

#include "EditBodyDlg.h"


QSize EditBodyDlg::s_WindowSize(1031,783);
QPoint EditBodyDlg::s_WindowPosition(131, 77);
bool EditBodyDlg::s_bWindowMaximized =false;
QByteArray EditBodyDlg::m_HorizontalSplitterSizes;

#define SECTIONHIGHLIGHT    1702



EditBodyDlg::EditBodyDlg(QWidget *pParent) : QDialog(pParent)
{
	setWindowTitle("Body object explorer");
	setWindowFlags(Qt::Window);

	m_pStruct = NULL;
	m_pDelegate = NULL;
	m_pModel = NULL;

	m_enumActiveWingType = XFLR5::OTHERWING;
	m_iActivePointMass = -1;

	m_bResetglFrameHighlight   = true;
	m_bResetglBody             = true;
	m_bChanged                 = false;

	m_pInsertBefore  = new QAction(tr("Insert Before"), this);
	m_pInsertAfter   = new QAction(tr("Insert after"), this);
	m_pDeleteItem = new QAction(tr("Delete"), this);

	m_pContextMenu = new QMenu(tr("Section"),this);
	m_pContextMenu->addAction(m_pInsertBefore);
	m_pContextMenu->addAction(m_pInsertAfter);
	m_pContextMenu->addAction(m_pDeleteItem);

	m_PixText = QPixmap(107, 97);
	m_PixText.fill(Qt::transparent);

	setupLayout();
}


/**
 * Overrides the base class showEvent method. Moves the window to its former location.
 * @param event the showEvent.
 */
void EditBodyDlg::showEvent(QShowEvent *event)
{
	if(m_HorizontalSplitterSizes.length()>0)
		m_pHorizontalSplitter->restoreState(m_HorizontalSplitterSizes);

	move(s_WindowPosition);
	resize(s_WindowSize);

	if(s_bWindowMaximized) setWindowState(Qt::WindowMaximized);

	m_pGLWidget->update();

	event->accept();
}



/**
 * Overrides the base class hideEvent method. Stores the window's current position.
 * @param event the hideEvent.
 */
void EditBodyDlg::hideEvent(QHideEvent *event)
{
	m_HorizontalSplitterSizes  = m_pHorizontalSplitter->saveState();
//	m_LeftSplitterSizes        = m_pLeftSplitter->saveState();
//	m_MiddleSplitterSizes      = m_pMiddleSplitter->saveState();
	s_WindowPosition = pos();
	event->accept();
}


void EditBodyDlg::resizeEvent(QResizeEvent *event)
{
	QList<int> leftSizes;
	leftSizes.append((int)(height()*95/100));
	leftSizes.append((int)(height()*5/100));
	m_pLeftSplitter->setSizes(leftSizes);

	QList<int> midlleSizes;
	midlleSizes.append((int)(height()*45/100));
	midlleSizes.append((int)(height()*45/100));
	midlleSizes.append((int)(height()*5/100));
	m_pMiddleSplitter->setSizes(midlleSizes);


	int ColumnWidth = (int)((double)(m_pStruct->width())/15);
	m_pStruct->setColumnWidth(0,ColumnWidth*6);
	m_pStruct->setColumnWidth(1,ColumnWidth*3);
	m_pStruct->setColumnWidth(2,ColumnWidth*3);

	if(m_pGLWidget->width()>0 && m_pGLWidget->height()>0)
	{
		m_PixText = m_PixText.scaled(m_pGLWidget->rect().size());
		m_PixText.fill(Qt::transparent);
	}
	event->accept();
}


void EditBodyDlg::contextMenuEvent(QContextMenuEvent *event)
{
	// Display the context menu

	if(!m_pBody->activeFrame() && m_iActivePointMass<0 ) return;
	if(m_pBody->activeFrame())
	{
		m_pInsertBefore->setText(tr("Insert body frame before"));
		m_pInsertAfter->setText(tr("Insert body frame after"));
		m_pDeleteItem->setText(tr("Delete body frame"));
	}
	else if(m_iActivePointMass>=0)
	{
		m_pInsertBefore->setText(tr("Insert point mass before"));
		m_pInsertAfter->setText(tr("Insert point mass after"));
		m_pDeleteItem->setText(tr("Delete point Mass"));
	}

	if(m_pStruct->geometry().contains(event->pos())) m_pContextMenu->exec(event->globalPos());
}


void EditBodyDlg::setupLayout()
{
	QStringList labels;
	labels << tr("Object") << tr("Field")<<tr("Value")<<tr("Unit");

	m_pStruct = new QTreeView;

#if QT_VERSION >= 0x050000
	m_pStruct->header()->setSectionResizeMode(QHeaderView::Interactive);
#endif

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


	QFont fnt;
	QFontMetrics fm(fnt);
	m_pStruct->setColumnWidth(0, fm.averageCharWidth()*37);
	m_pStruct->setColumnWidth(1, fm.averageCharWidth()*29);
	m_pStruct->setColumnWidth(2, fm.averageCharWidth()*17);


	QItemSelectionModel *selectionModel = new QItemSelectionModel(m_pModel);
	m_pStruct->setSelectionModel(selectionModel);
	connect(selectionModel, SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(onItemClicked(QModelIndex)));


	m_pDelegate = new EditObjectDelegate(this);
	m_pStruct->setItemDelegate(m_pDelegate);
	connect(m_pDelegate,  SIGNAL(closeEditor(QWidget *)), this, SLOT(onRedraw()));


	QSizePolicy szPolicyMinimumExpanding;
	szPolicyMinimumExpanding.setHorizontalPolicy(QSizePolicy::MinimumExpanding);
	szPolicyMinimumExpanding.setVerticalPolicy(QSizePolicy::MinimumExpanding);

	QSizePolicy szPolicyMaximum;
	szPolicyMaximum.setHorizontalPolicy(QSizePolicy::Maximum);
	szPolicyMaximum.setVerticalPolicy(QSizePolicy::Maximum);

	m_pHorizontalSplitter = new QSplitter(Qt::Horizontal, this);
	{
		m_pLeftSplitter = new QSplitter(Qt::Vertical, this);;
		{
			m_pStruct->setSizePolicy(szPolicyMinimumExpanding);
			QWidget *pCommandBox = new QWidget;
			{
				QHBoxLayout *pCommandLayout = new QHBoxLayout;
				{
					m_pctrlRedraw = new QPushButton(tr("Regenerate") + "\t(F4)");
					pOKButton = new QPushButton(tr("Save and Close"));
					pOKButton->setAutoDefault(true);
					QPushButton *pCancelButton = new QPushButton(tr("Cancel"));
					pCancelButton->setAutoDefault(false);

					m_pctrlRedraw->setSizePolicy(szPolicyMaximum);
					pOKButton->setSizePolicy(szPolicyMaximum);
					pCancelButton->setSizePolicy(szPolicyMaximum);

					pCommandLayout->addWidget(m_pctrlRedraw);
					pCommandLayout->addWidget(pOKButton);
					pCommandLayout->addWidget(pCancelButton);
					connect(pOKButton, SIGNAL(clicked()),this, SLOT(onOK()));
					connect(pCancelButton, SIGNAL(clicked()),this, SLOT(reject()));
				}
				pCommandBox->setLayout(pCommandLayout);
				pCommandBox->setSizePolicy(szPolicyMaximum);
			}
			m_pLeftSplitter->addWidget(m_pStruct);
			m_pLeftSplitter->addWidget(pCommandBox);
		}


		m_pMiddleSplitter = new QSplitter(Qt::Vertical, this);
		{
			m_pBodyLineWidget = new BodyLineWidget(this);
			m_pBodyLineWidget->setSizePolicy(szPolicyMaximum);

			m_pGLWidget = new ThreeDWidget(this);
			m_pGLWidget->m_iView = GLEDITBODYVIEW;

			QWidget *p3DCtrlBox = new QWidget;
			{
				p3DCtrlBox->setSizePolicy(szPolicyMaximum);
				QHBoxLayout *pThreeDViewControlsLayout = new QHBoxLayout;
				{
					QGridLayout *pThreeDParamsLayout = new QGridLayout;
					{
						m_pctrlAxes         = new QCheckBox(tr("Axes"), this);
						m_pctrlSurfaces     = new QCheckBox(tr("Surfaces"), this);
						m_pctrlOutline      = new QCheckBox(tr("Outline"), this);
						m_pctrlPanels       = new QCheckBox(tr("Panels"), this);
						m_pctrlShowMasses   = new QCheckBox(tr("Masses"), this);

						m_pctrlAxes->setSizePolicy(szPolicyMaximum);
						m_pctrlSurfaces->setSizePolicy(szPolicyMaximum);
						m_pctrlOutline->setSizePolicy(szPolicyMaximum);
						m_pctrlPanels->setSizePolicy(szPolicyMaximum);
						m_pctrlShowMasses->setSizePolicy(szPolicyMaximum);

						pThreeDParamsLayout->addWidget(m_pctrlAxes, 1,1);
						pThreeDParamsLayout->addWidget(m_pctrlPanels, 1,3);
						pThreeDParamsLayout->addWidget(m_pctrlSurfaces, 1,2);
						pThreeDParamsLayout->addWidget(m_pctrlOutline, 2,2);
						pThreeDParamsLayout->addWidget(m_pctrlShowMasses, 2,3);
					}

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
					QVBoxLayout *pRightColLayout = new QVBoxLayout;
					{


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


						pRightColLayout->addWidget(m_pctrlReset);
						pRightColLayout->addLayout(pClipLayout);
					}
					pThreeDViewControlsLayout->addLayout(pThreeDParamsLayout);
					pThreeDViewControlsLayout->addStretch();
					pThreeDViewControlsLayout->addLayout(pAxisViewLayout);
					pThreeDViewControlsLayout->addStretch();
					pThreeDViewControlsLayout->addLayout(pRightColLayout);

				}
				p3DCtrlBox->setLayout(pThreeDViewControlsLayout);
			}
			m_pBodyLineWidget->sizePolicy().setVerticalStretch(2);
			m_pGLWidget->sizePolicy().setVerticalStretch(5);
			p3DCtrlBox->sizePolicy().setVerticalStretch(2);

			m_pMiddleSplitter->addWidget(m_pBodyLineWidget);
			m_pMiddleSplitter->addWidget(m_pGLWidget);
			m_pMiddleSplitter->addWidget(p3DCtrlBox);
		}

		m_pFrameWidget = new BodyFrameWidget(this);

		m_pHorizontalSplitter->addWidget(m_pLeftSplitter);
		m_pHorizontalSplitter->addWidget(m_pMiddleSplitter);
		m_pHorizontalSplitter->addWidget(m_pFrameWidget);

		QList<int> horizontalSizes;
		horizontalSizes.append(30);
		horizontalSizes.append(60);
		horizontalSizes.append(10);
		m_pHorizontalSplitter->setSizes(horizontalSizes);

		QList<int> leftSplitterSizes;
		leftSplitterSizes.append(95);
		leftSplitterSizes.append( 5);
		m_pLeftSplitter->setSizes(leftSplitterSizes);

		QList<int> middleSplitterSizes;
		middleSplitterSizes.append(30);
		middleSplitterSizes.append(60);
		middleSplitterSizes.append(5);
		m_pMiddleSplitter->setSizes(middleSplitterSizes);
	}

	QHBoxLayout *pMainLayout = new QHBoxLayout;
	{
		pMainLayout->addWidget(m_pHorizontalSplitter);
	}
	setLayout(pMainLayout);
	Connect();
//	resize(s_Size);
}



void EditBodyDlg::onOK()
{
	s_bWindowMaximized= isMaximized();
	s_WindowPosition = pos();
	s_WindowSize = size();

	accept();
}



void EditBodyDlg::initDialog(Body *pBody)
{
	m_pBody = pBody;
	m_pBodyLineWidget->setBody(pBody);
	m_pFrameWidget->setBody(m_pBody);
	m_pGLWidget->setScale(m_pBody->Length());

	fillBodyTreeView();

	m_pctrlSurfaces->setChecked(ThreeDWidget::s_bSurfaces);
	m_pctrlOutline->setChecked(ThreeDWidget::s_bOutline);
	m_pctrlAxes->setChecked(ThreeDWidget::s_bAxes);
	m_pctrlPanels->setChecked(ThreeDWidget::s_bVLMPanels);
	m_pctrlShowMasses->setChecked(ThreeDWidget::s_bShowMasses);
	m_pctrlClipPlanePos->setValue((int)(m_pGLWidget->m_ClipPlanePos*100.0));
}




void EditBodyDlg::keyPressEvent(QKeyEvent *event)
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
		case Qt::Key_F4:
		{
			onRedraw();
			return;
		}

		default:
			event->ignore();
	}
}



void EditBodyDlg::reject()
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
			onOK();
			return;
		}
		else if(QMessageBox::Cancel == Ans) return;
	}

//	reject();
	done(QDialog::Rejected);
}



void EditBodyDlg::GLDraw3D()
{
	m_pGLWidget->makeCurrent();
	glClearColor(Settings::s_BackgroundColor.redF(), Settings::s_BackgroundColor.greenF(), Settings::s_BackgroundColor.blueF(),0.0);


	if(m_bResetglFrameHighlight || m_bResetglBody)
	{
		if(glIsList(SECTIONHIGHLIGHT))
		{
			glDeleteLists(SECTIONHIGHLIGHT,1);
		}
		if(m_pBody->activeFrame())
		{
			GLCreateBodyFrameHighlight(m_pBody,CVector(0.0,0.0,0.0), m_pBody->m_iActiveFrame);
			m_bResetglFrameHighlight = false;
		}
	}


	if(m_bResetglBody)
	{
		if(glIsList(BODYGEOMBASE))
		{
			glDeleteLists(BODYGEOMBASE,1);
			glDeleteLists(BODYGEOMBASE+MAXBODIES,1);
		}
		if(m_pBody->bodyType()==XFLR5::BODYPANELTYPE)	    GLCreateBody3DFlatPanels(BODYGEOMBASE, m_pBody);
		else if(m_pBody->bodyType()==XFLR5::BODYSPLINETYPE) GLCreateBody3DSplines(   BODYGEOMBASE, m_pBody, 47, 37);

		m_bResetglBody = false;
		if(glIsList(BODYMESHBASE))
		{
			glDeleteLists(BODYMESHBASE,1);
			glDeleteLists(BODYMESHBASE+MAXBODIES,1);
		}
		GLCreateBodyMesh(BODYMESHBASE, m_pBody);
	}
}



void EditBodyDlg::GLRenderView()
{
	QString MassUnit;
	Units::getWeightUnitLabel(MassUnit);


	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);

	if(ThreeDWidget::s_bOutline)
	{
		glCallList(BODYGEOMBASE+MAXBODIES);
	}

	if(m_pBody->activeFrame())
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
		glCallList(BODYGEOMBASE);
	}

	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);

	if(ThreeDWidget::s_bVLMPanels)
	{
		glCallList(BODYMESHBASE);
		if(!ThreeDWidget::s_bSurfaces) glCallList(BODYMESHBASE+MAXBODIES);
	}


	if(ThreeDWidget::s_bShowMasses)
	{
		m_pGLWidget->GLDrawMasses(m_pBody->volumeMass(),
								  CVector(0.0,0.0,0.0),
								  m_pBody->bodyName(),
								  m_pBody->m_PointMass);
	}
}


void EditBodyDlg::GLCreateBodyFrameHighlight(Body *pBody, CVector bodyPos, int iFrame)
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
	Frame *pFrame = m_pBody->frame(iFrame);

	nh = 23;
//	xinc = 0.1;
	hinc = 1.0/(double)(nh-1);


	//create 3D Splines or Lines to overlay on the body
	glNewList(SECTIONHIGHLIGHT,GL_COMPILE);
	{
		glEnable(GL_DEPTH_TEST);
		glEnable (GL_LINE_STIPPLE);

		color = QColor(255,0,0);
		style = 0;
		width = 3;
		glLineWidth(width);

		GLLineStipple(style);

		glColor3d(color.redF(), color.greenF(), color.blueF());

		if(m_pBody->bodyType() == XFLR5::BODYSPLINETYPE)
		{
			if(m_pBody->activeFrame())
			{
				u = m_pBody->getu(pFrame->m_Position.x);

				glBegin(GL_LINE_STRIP);
				{
					v = 0.0;
					for (k=0; k<nh; k++)
					{
						pBody->getPoint(u,v,true, Point);
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
						pBody->getPoint(u,v,false, Point);
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


void EditBodyDlg::Connect()
{
	connect(m_pBodyLineWidget, SIGNAL(objectModified()), this, SLOT(onRefillBodyTree()));
	connect(m_pFrameWidget,    SIGNAL(objectModified()), this, SLOT(onRefillBodyTree()));

	connect(m_pInsertBefore,   SIGNAL(triggered()), this, SLOT(onInsertBefore()));
	connect(m_pInsertAfter,    SIGNAL(triggered()), this, SLOT(onInsertAfter()));
	connect(m_pDeleteItem,     SIGNAL(triggered()), this, SLOT(onDelete()));

	connect(m_pctrlRedraw,     SIGNAL(clicked()), this, SLOT(onRedraw()));

	connect(m_pctrlReset,      SIGNAL(clicked()), m_pGLWidget, SLOT(on3DReset()));

	connect(m_pctrlAxes,       SIGNAL(clicked(bool)), m_pGLWidget, SLOT(onAxes(bool)));
	connect(m_pctrlPanels,     SIGNAL(clicked(bool)), m_pGLWidget, SLOT(onPanels(bool)));
	connect(m_pctrlSurfaces,   SIGNAL(clicked(bool)), m_pGLWidget, SLOT(onSurfaces(bool)));
	connect(m_pctrlOutline,    SIGNAL(clicked(bool)), m_pGLWidget, SLOT(onOutline(bool)));
	connect(m_pctrlShowMasses, SIGNAL(clicked(bool)), m_pGLWidget, SLOT(onShowMasses(bool)));

	connect(m_pctrlIso,        SIGNAL(clicked()), m_pGLWidget, SLOT(on3DIso()));
	connect(m_pctrlX,          SIGNAL(clicked()), m_pGLWidget, SLOT(on3DFront()));
	connect(m_pctrlY,          SIGNAL(clicked()), m_pGLWidget, SLOT(on3DLeft()));
	connect(m_pctrlZ,          SIGNAL(clicked()), m_pGLWidget, SLOT(on3DTop()));

	connect(m_pctrlClipPlanePos, SIGNAL(sliderMoved(int)), m_pGLWidget, SLOT(onClipPlane(int)));
}



bool EditBodyDlg::IntersectObject(CVector AA,  CVector U, CVector &I)
{
	return m_pBody->intersectFlatPanels(AA, AA+U*10, I);
}



QList<QStandardItem *> EditBodyDlg::prepareRow(const QString &object, const QString &field, const QString &value,  const QString &unit)
{
	QList<QStandardItem *> rowItems;
	rowItems << new QStandardItem(object)  << new QStandardItem(field)  << new QStandardItem(value) << new QStandardItem(unit);
	for(int ii=0; ii<rowItems.size(); ii++) rowItems.at(ii)->setData(XFLR5::STRING, Qt::UserRole);
	return rowItems;
}


QList<QStandardItem *> EditBodyDlg::prepareBoolRow(const QString &object, const QString &field, const bool &value)
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


QList<QStandardItem *> EditBodyDlg::prepareIntRow(const QString &object, const QString &field, const int &value)
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


QList<QStandardItem *> EditBodyDlg::prepareDoubleRow(const QString &object, const QString &field, const double &value,  const QString &unit)
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





void EditBodyDlg::fillBodyTreeView()
{
	m_pModel->removeRows(0, m_pModel->rowCount());


	QList<QStandardItem*> dataItem;
	QStandardItem *rootItem = m_pModel->invisibleRootItem();

	QModelIndex ind = m_pModel->index(0,0);
	m_pStruct->expand(ind);


	QList<QStandardItem*> bodyFolder = prepareRow("Body");
	rootItem->appendRow(bodyFolder);

	m_pStruct->expand(m_pModel->indexFromItem(bodyFolder.first()));

	dataItem = prepareRow("", "Name", m_pBody->bodyName());
	bodyFolder.first()->appendRow(dataItem);

	dataItem = prepareRow("", "Type", bodyPanelType(m_pBody->bodyType()));
	dataItem.at(2)->setData(XFLR5::BODYTYPE, Qt::UserRole);
	bodyFolder.first()->appendRow(dataItem);

	QList<QStandardItem*> bodyColorFolder = prepareRow("Color");
	bodyFolder.first()->appendRow(bodyColorFolder);
	{
		QList<QStandardItem*> dataItem = prepareIntRow("", "red", m_pBody->bodyColor().red());
		bodyColorFolder.first()->appendRow(dataItem);

		dataItem = prepareIntRow("", "green", m_pBody->bodyColor().green());
		bodyColorFolder.first()->appendRow(dataItem);

		dataItem = prepareIntRow("", "blue", m_pBody->bodyColor().blue());
		bodyColorFolder.first()->appendRow(dataItem);

		dataItem = prepareIntRow("", "alpha", m_pBody->bodyColor().alpha());
		bodyColorFolder.first()->appendRow(dataItem);
	}

	QList<QStandardItem*> bodyInertiaFolder = prepareRow("Inertia");
	bodyFolder.first()->appendRow(bodyInertiaFolder);
	{
		if(m_iActivePointMass>=0) m_pStruct->expand(m_pModel->indexFromItem(bodyInertiaFolder.first()));

		QList<QStandardItem*> dataItem = prepareDoubleRow( "Structural (volume) mass", "", m_pBody->m_VolumeMass*Units::kgtoUnit(), Units::weightUnitLabel());
		bodyInertiaFolder.first()->appendRow(dataItem);

		for(int iwm=0; iwm<m_pBody->m_PointMass.size(); iwm++)
		{
			PointMass *pm = m_pBody->m_PointMass.at(iwm);
			QList<QStandardItem*> bodyPointMassFolder = prepareRow(QString("Point_mass_%1").arg(iwm+1));

			bodyInertiaFolder.first()->appendRow(bodyPointMassFolder);
			{
				if(m_iActivePointMass==iwm)
				{
					m_pStruct->expand(m_pModel->indexFromItem(bodyPointMassFolder.first()));
				}
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
		QList<QStandardItem*> dataItem = prepareIntRow("", "NURBS degree (lengthwise)", m_pBody->splineSurface()->uDegree());
		NURBSFolder.first()->appendRow(dataItem);

		dataItem = prepareIntRow("", "NURBS degree (hoop)", m_pBody->splineSurface()->vDegree());
		NURBSFolder.first()->appendRow(dataItem);

		dataItem = prepareIntRow("", "Mesh panels (lengthwise)", m_pBody->m_nxPanels);
		NURBSFolder.first()->appendRow(dataItem);

		dataItem = prepareIntRow("", "Mesh panels (hoop)", m_pBody->m_nhPanels);
		NURBSFolder.first()->appendRow(dataItem);
	}

	QList<QStandardItem*> hoopFolder = prepareRow("Hoop_panels (FLATPANELS case)");
	bodyFolder.first()->appendRow(hoopFolder);
	{
		for(int isl=0; isl<m_pBody->sideLineCount(); isl++)
		{
			QList<QStandardItem*> dataItem = prepareIntRow("", QString("Hoop panels in stripe %1").arg(isl+1), m_pBody->m_hPanels.at(isl));
			hoopFolder.first()->appendRow(dataItem);
		}
	}

	QList<QStandardItem*> bodyFrameFolder = prepareRow("Frames");
	bodyFolder.first()->appendRow(bodyFrameFolder);
	{
		m_pStruct->expand(m_pModel->indexFromItem(bodyFrameFolder.first()));

		for(int iFrame=0; iFrame <m_pBody->splineSurface()->frameCount(); iFrame++)
		{
			Frame *pFrame = m_pBody->splineSurface()->frameAt(iFrame);

			QList<QStandardItem*> sectionFolder = prepareRow(QString("Frame_%1").arg(iFrame+1));
			bodyFrameFolder.first()->appendRow(sectionFolder);
			{
				if(m_pBody->m_iActiveFrame==iFrame) m_pStruct->expand(m_pModel->indexFromItem(sectionFolder.first()));

				dataItem = prepareIntRow("", "Lengthwise panels (FLATPANELS case)", m_pBody->m_xPanels.at(iFrame));
				sectionFolder.first()->appendRow(dataItem);

				QList<QStandardItem*> dataItem = prepareDoubleRow("x_Position", "x", pFrame->m_Position.x*Units::mtoUnit(), Units::lengthUnitLabel());
				sectionFolder.first()->appendRow(dataItem);

				for(int iPt=0; iPt<pFrame->PointCount(); iPt++)
				{
					QList<QStandardItem*> pointFolder = prepareRow(QString("Point %1").arg(iPt+1));
					sectionFolder.first()->appendRow(pointFolder);
					{
						if(Frame::s_iSelect==iPt) m_pStruct->expand(m_pModel->indexFromItem(pointFolder.first()));

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


void EditBodyDlg::updateViews()
{
	m_pGLWidget->update();
	m_pFrameWidget->update();
	m_pBodyLineWidget->update();

}




void EditBodyDlg::onRedraw()
{
	readBodyTree(m_pModel->index(0,0).child(0,0));

	m_bResetglBody = true;
	m_bChanged = true;
	updateViews();
}


void EditBodyDlg::onRefillBodyTree()
{
	fillBodyTreeView();
	m_bResetglBody = true;
	m_pGLWidget->update();
	m_pBodyLineWidget->update();
	m_pFrameWidget->update();
}



void EditBodyDlg::readBodyTree(QModelIndex indexLevel)
{
	QString object, field, value;
	QModelIndex dataIndex, subIndex;

	do
	{
		object = indexLevel.sibling(indexLevel.row(),0).data().toString();
		field = indexLevel.sibling(indexLevel.row(),1).data().toString();
		value = indexLevel.sibling(indexLevel.row(),2).data().toString();

		if(indexLevel.child(0,0).isValid())
		{
			if(object.compare("Color", Qt::CaseInsensitive)==0)
			{
				subIndex = indexLevel.child(0,0);
				do
				{
					object = subIndex.sibling(subIndex.row(),0).data().toString();
					field = subIndex.sibling(subIndex.row(),1).data().toString();
					value = subIndex.sibling(subIndex.row(),2).data().toString();

					dataIndex = subIndex.sibling(subIndex.row(),2);

					if(field.compare("red", Qt::CaseInsensitive)==0)         m_pBody->bodyColor().setRed(dataIndex.data().toInt());
					else if(field.compare("green", Qt::CaseInsensitive)==0)  m_pBody->bodyColor().setGreen(dataIndex.data().toInt());
					else if(field.compare("blue", Qt::CaseInsensitive)==0)   m_pBody->bodyColor().setBlue(dataIndex.data().toInt());
					else if(field.compare("alpha", Qt::CaseInsensitive)==0)  m_pBody->bodyColor().setAlpha(dataIndex.data().toInt());

					subIndex = subIndex.sibling(subIndex.row()+1,0);
				}while(subIndex.isValid());
			}
			else if(object.compare("Inertia", Qt::CaseInsensitive)==0) 	readInertiaTree(m_pBody->volumeMass(), m_pBody->m_PointMass, indexLevel.child(0,0));
			else if(object.compare("NURBS", Qt::CaseInsensitive)==0)
			{
				subIndex = indexLevel.child(0,0);
				do
				{
					object = subIndex.sibling(subIndex.row(),0).data().toString();
					field = subIndex.sibling(subIndex.row(),1).data().toString();
					value = subIndex.sibling(subIndex.row(),2).data().toString();

					dataIndex = subIndex.sibling(subIndex.row(),2);

					if(field.compare("NURBS degree (lengthwise)", Qt::CaseInsensitive)==0)     m_pBody->splineSurface()->SetuDegree(dataIndex.data().toInt());
					else if(field.compare("NURBS degree (hoop)", Qt::CaseInsensitive)==0)      m_pBody->splineSurface()->SetvDegree(dataIndex.data().toInt());
					else if(field.compare("Mesh panels (lengthwise)", Qt::CaseInsensitive)==0) m_pBody->m_nxPanels = dataIndex.data().toInt();
					else if(field.compare("Mesh panels (hoop)", Qt::CaseInsensitive)==0)       m_pBody->m_nhPanels = dataIndex.data().toInt();

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
					m_pBody->m_hPanels[idx] =  dataIndex.data().toInt();

					subIndex = subIndex.sibling(subIndex.row()+1,0);
				}
				while(subIndex.isValid());
			}
			else if(object.compare("Frames", Qt::CaseInsensitive)==0)
			{
				m_pBody->m_SplineSurface.ClearFrames();
				QModelIndex subIndex = indexLevel.child(0,0);
				do
				{
					object = subIndex.sibling(subIndex.row(),0).data().toString();
					if(object.indexOf("Frame_")>=0)
					{
						Frame *pFrame = new Frame;
						readBodyFrameTree(pFrame, subIndex.child(0,0));
						m_pBody->m_SplineSurface.appendFrame(pFrame);
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

			if     (field.compare("Name", Qt::CaseInsensitive)==0) m_pBody->bodyName() = value;
			else if(field.compare("Type", Qt::CaseInsensitive)==0) m_pBody->bodyType() = bodyPanelType(value);
		}

		indexLevel = indexLevel.sibling(indexLevel.row()+1,0);

	} while(indexLevel.isValid());
}


void EditBodyDlg::readBodyFrameTree(Frame *pFrame, QModelIndex indexLevel)
{
	QString object, field, value;
	QModelIndex dataIndex;
	double x;

	do
	{
		object = indexLevel.sibling(indexLevel.row(),0).data().toString();
		field = indexLevel.sibling(indexLevel.row(),1).data().toString();
		value = indexLevel.sibling(indexLevel.row(),2).data().toString();

		dataIndex = indexLevel.sibling(indexLevel.row(),2);

		if (field.compare("Lengthwise panels (FLATPANELS case)", Qt::CaseInsensitive)==0)   m_pBody->m_xPanels.append(dataIndex.data().toInt());
		else if (object.compare("x_Position", Qt::CaseInsensitive)==0) x = dataIndex.data().toDouble()/Units::mtoUnit();
		else if (object.indexOf("Point", Qt::CaseInsensitive)==0)
		{
			CVector Pt;
			readVectorTree(Pt, indexLevel.child(0,0));
			pFrame->AppendPoint(Pt);
		}
		indexLevel = indexLevel.sibling(indexLevel.row()+1,0);
	} while(indexLevel.isValid());

	pFrame->SetuPosition(x);
}



void EditBodyDlg::readInertiaTree(double &volumeMass, QList<PointMass*>&pointMasses, QModelIndex indexLevel)
{
	pointMasses.clear();

	QString object, field, value;
	QModelIndex dataIndex;
	do
	{
		if(indexLevel.child(0,0).isValid())
		{
			object = indexLevel.sibling(indexLevel.row(),0).data().toString();
			if(object.indexOf("Point_mass_", Qt::CaseInsensitive)>=0)
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


void EditBodyDlg::readPointMassTree(PointMass *ppm, QModelIndex indexLevel)
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



void EditBodyDlg::readVectorTree(CVector &V, QModelIndex indexLevel)
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



void EditBodyDlg::onItemClicked(const QModelIndex &index)
{
	identifySelection(index);
	update();
}



void EditBodyDlg::identifySelection(const QModelIndex &indexSel)
{
	// we highlight wing sections and body frames
	// so check if the user's selection is one of these
	m_enumActiveWingType = XFLR5::OTHERWING;
	m_iActivePointMass = -1;

	QModelIndex indexLevel = indexSel;
	QString object, value;
	do
	{
		object = indexLevel.sibling(indexLevel.row(),0).data().toString();

		if(object.indexOf("Frame_", 0, Qt::CaseInsensitive)>=0)
		{
			setActiveFrame(object.right(object.length()-6).toInt() -1);
			Frame::s_iSelect = -1;
			m_iActivePointMass = -1;
			return;
		}
		else if(object.indexOf("Point_Mass_", 0, Qt::CaseInsensitive)>=0)
		{
			m_iActivePointMass = object.right(object.length()-11).toInt() -1;
			setActiveFrame(-1);
			return;
		}
		else if(object.indexOf("Point", 0, Qt::CaseInsensitive)==0)
		{
			Frame::s_iSelect = object.right(object.length()-6).toInt() -1;
			//identify the parent Frame object

			indexLevel = indexLevel.parent();
			do
			{
				object = indexLevel.sibling(indexLevel.row(),0).data().toString();

				if(object.indexOf("Frame_", 0, Qt::CaseInsensitive)>=0)
				{
					setActiveFrame(object.right(object.length()-6).toInt() -1);
					return;
				}

				indexLevel = indexLevel.parent();
			} while(indexLevel.isValid());

			setActiveFrame(-1);
			return;
		}
		indexLevel = indexLevel.parent();
	} while(indexLevel.isValid());
}


void EditBodyDlg::setActiveFrame(int iFrame)
{
	m_pBody->setActiveFrame(m_pBody->frame(iFrame));
	m_bResetglFrameHighlight = true;
	m_pGLWidget->update();
}


void EditBodyDlg::onInsertBefore()
{
	 if(m_pBody && m_pBody->activeFrame())
	{
		m_pBody->insertFrameBefore(m_pBody->m_iActiveFrame);

		m_pStruct->closePersistentEditor(m_pStruct->currentIndex());
		fillBodyTreeView();

		m_bChanged = true;
		m_bResetglFrameHighlight = true;
		m_bResetglBody   = true;
		m_pGLWidget->update();
	}
	else if(m_iActivePointMass>=0)
	{
		m_pBody->m_PointMass.insert(m_iActivePointMass, new PointMass);

		m_pStruct->closePersistentEditor(m_pStruct->currentIndex());

		m_bChanged = true;
		m_bResetglFrameHighlight = true;
		m_pGLWidget->update();

	}
}



void EditBodyDlg::onInsertAfter()
{
	if(m_pBody && m_pBody->activeFrame())
	{
		m_pBody->insertFrameAfter(m_pBody->m_iActiveFrame);

		m_pStruct->closePersistentEditor(m_pStruct->currentIndex());
		fillBodyTreeView();

		m_pBody->m_iActiveFrame++;
		m_bChanged = true;
		m_bResetglFrameHighlight = true;
		m_bResetglBody   = true;
		m_pGLWidget->update();
	}
	else if(m_iActivePointMass>=0)
	{
		if(m_pBody)
		{
			m_pBody->m_PointMass.insert(m_iActivePointMass+1, new PointMass);
			m_iActivePointMass++;
		}


		m_pStruct->closePersistentEditor(m_pStruct->currentIndex());
		fillBodyTreeView();

		m_bChanged = true;
		m_bResetglFrameHighlight = true;
		m_pGLWidget->update();

	}
}


void EditBodyDlg::onDelete()
{
	if(m_pBody && m_pBody->activeFrame())
	{
		m_pBody->removeActiveFrame();

		m_pStruct->closePersistentEditor(m_pStruct->currentIndex());
		fillBodyTreeView();

		m_bChanged = true;
		m_bResetglFrameHighlight = true;
		m_bResetglBody   = true;
		m_pGLWidget->update();
	}
	else if(m_iActivePointMass>=0)
	{
		if(m_pBody)
		{
			m_pBody->m_PointMass.removeAt(m_iActivePointMass);
		}


		m_pStruct->closePersistentEditor(m_pStruct->currentIndex());
		fillBodyTreeView();

		m_bChanged = true;
		m_bResetglFrameHighlight = true;
		m_pGLWidget->update();
	}
}




/**
 * Draws the wing legend in the 2D operating point view
 * @param painter a reference to the QPainter object on which the view shall be drawn
 */
void EditBodyDlg::paintBodyLegend(QPainter &painter)
{
	painter.save();

	QPen textPen(Settings::s_TextColor);
	painter.setPen(textPen);
	painter.setFont(Settings::s_TextFont);
	painter.setRenderHint(QPainter::Antialiasing);

	painter.restore();
}



bool EditBodyDlg::loadSettings(QSettings *pSettings)
{
	pSettings->beginGroup("EditBodyDlg");
	{
	//  we're reading/loading
		s_WindowSize              = pSettings->value("WindowSize", QSize(1031,783)).toSize();
		s_bWindowMaximized        = pSettings->value("WindowMaximized", false).toBool();
		s_WindowPosition          = pSettings->value("WindowPosition", QPoint(131, 77)).toPoint();
		m_HorizontalSplitterSizes = pSettings->value("HorizontalSplitterSizes").toByteArray();
//		m_LeftSplitterSizes       = pSettings->value("LeftSplitterSizes").toByteArray();
//		m_MiddleSplitterSizes     = pSettings->value("RightSplitterSizes").toByteArray();
	}
	pSettings->endGroup();
	return true;
}




bool EditBodyDlg::saveSettings(QSettings *pSettings)
{
	pSettings->beginGroup("EditBodyDlg");
	{
		pSettings->setValue("WindowSize", s_WindowSize);
		pSettings->setValue("WindowMaximized", s_bWindowMaximized);
		pSettings->setValue("WindowPosition", s_WindowPosition);
		pSettings->setValue("HorizontalSplitterSizes", m_HorizontalSplitterSizes);
//		pSettings->setValue("LeftSplitterSizes",       m_LeftSplitterSizes);
//		pSettings->setValue("RightSplitterSizes",      m_MiddleSplitterSizes);
	}
	pSettings->endGroup();

	return true;
}




