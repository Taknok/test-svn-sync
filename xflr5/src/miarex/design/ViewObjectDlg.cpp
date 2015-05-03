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

QSize ViewObjectDlg::s_Size(911,783);
QPoint ViewObjectDlg::s_Position(131, 77);

#define SECTIONHIGHLIGHT    1702



ViewObjectDlg::ViewObjectDlg(QWidget *pParent) : QDialog(pParent)
{
	setWindowTitle("Plane object explorer");

	m_pPlane = NULL;
	m_pStruct = NULL;
	m_pDelegate = NULL;
	m_pModel = NULL;

	m_iSection   = -1;


	m_bResetglSectionHighlight = true;
	m_bResetglPlane            = true;
	m_bChanged                 = false;

	setupLayout();
}


/**
 * Overrides the base class showEvent method. Moves the window to its former location.
 * @param event the showEvent.
 */
void ViewObjectDlg::showEvent(QShowEvent *event)
{
	move(s_Position);
	event->accept();
}


/**
 * Overrides the base class hideEvent method. Stores the window's current position.
 * @param event the hideEvent.
 */
void ViewObjectDlg::hideEvent(QHideEvent *event)
{
	s_Position = pos();
	event->accept();
}


void ViewObjectDlg::resizeEvent(QResizeEvent *event)
{
//	m_pStruct->setMinimumWidth(width()/2);
	s_Size = size();
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

//	m_pStruct->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
	m_pStruct->setEditTriggers(QAbstractItemView::NoEditTriggers);
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
		pHLayout->addWidget(m_pStruct);

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
		pHLayout->addLayout(pRightSideLayout);
	}
	setLayout(pHLayout);
	Connect();
	resize(s_Size);
}




void ViewObjectDlg::initDialog(Plane *pPlane)
{
	m_pPlane = pPlane;
	showPlane();

	m_pctrlSurfaces->setChecked(ThreeDWidget::s_bSurfaces);
	m_pctrlOutline->setChecked(ThreeDWidget::s_bOutline);
	m_pctrlAxes->setChecked(ThreeDWidget::s_bAxes);
	m_pctrlPanels->setChecked(ThreeDWidget::s_bVLMPanels);
	m_pctrlFoilNames->setChecked(ThreeDWidget::s_bFoilNames);
	m_pctrlShowMasses->setChecked(ThreeDWidget::s_bShowMasses);
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


void ViewObjectDlg::showPlane()
{
	QStandardItem *rootItem = m_pModel->invisibleRootItem();
	rootItem->setText(m_pPlane->planeName());

	QList<QStandardItem*> planeRootItem = prepareRow("Plane", "Name", m_pPlane->planeName());
	rootItem->appendRow(planeRootItem);

	QModelIndex ind = m_pModel->index(0,0);
	m_pStruct->expand(ind);

	showPlaneMetaData(planeRootItem.first());

	showBody(planeRootItem.first());

	QList<QStandardItem*> wingFolder = prepareRow("Wings");
	planeRootItem.first()->appendRow(wingFolder);
	{
		QModelIndex ind = m_pModel->indexFromItem(wingFolder.first());
		m_pStruct->expand(ind);

		for(int iw=0; iw<MAXWINGS; iw++)
		{
			if(m_pPlane->wing(iw))
			{
				showWing(iw, wingFolder);
			}
		}
	}
}



void ViewObjectDlg::showPlaneMetaData(QStandardItem *item)
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
				QList<QStandardItem*> wingPointMassFolder = prepareRow("Point mass");

				PlaneInertiaFolder.first()->appendRow(wingPointMassFolder);
				{
					QList<QStandardItem*> dataItem = prepareRow("", "Tag", pm->tag());
					dataItem.at(2)->setData(XFLR5::STRING, Qt::UserRole);

					dataItem = prepareRow("", "mass", QString("%1").arg(pm->mass()*Units::kgtoUnit()), Units::weightUnitLabel());
					dataItem.at(2)->setData(XFLR5::DOUBLE, Qt::UserRole);
					wingPointMassFolder.first()->appendRow(dataItem);

					dataItem = prepareDoubleRow("", "x", pm->position().x*Units::mtoUnit(), Units::lengthUnitLabel());
					dataItem.at(2)->setData(XFLR5::DOUBLE, Qt::UserRole);
					wingPointMassFolder.first()->appendRow(dataItem);

					dataItem = prepareDoubleRow("", "y", pm->position().y*Units::mtoUnit(), Units::lengthUnitLabel());
					dataItem.at(2)->setData(XFLR5::DOUBLE, Qt::UserRole);
					wingPointMassFolder.first()->appendRow(dataItem);

					dataItem = prepareDoubleRow("", "z", pm->position().z*Units::mtoUnit(), Units::lengthUnitLabel());
					dataItem.at(2)->setData(XFLR5::DOUBLE, Qt::UserRole);
					wingPointMassFolder.first()->appendRow(dataItem);
				}
			}
		}
	}
}



void ViewObjectDlg::showWing(int iw, QList<QStandardItem*> &planeRootItem)
{
	Wing *pWing = m_pPlane->wing(iw);

	QList<QStandardItem*> wingFolder = prepareRow("Wing", "Name", pWing->WingName());
	planeRootItem.first()->appendRow(wingFolder);

	QList<QStandardItem*> dataItem = prepareRow("Symetricity", "is symetric:", pWing->isSymetric() ? "true": "false");
	dataItem.at(2)->setData(XFLR5::BOOL, Qt::UserRole);
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
	}

	QList<QStandardItem*> finDataFolder = prepareRow("Fin data");
	wingFolder.first()->appendRow(finDataFolder);
	{
		QList<QStandardItem*> dataItem = prepareRow("", "is Fin:", pWing->IsFin() ? "true": "false");
		dataItem.at(2)->setData(XFLR5::BOOL, Qt::UserRole);
		finDataFolder.first()->appendRow(dataItem);

		dataItem = prepareRow("", "is Symetric Fin:", pWing->IsSymFin() ? "true": "false");
		dataItem.at(2)->setData(XFLR5::BOOL, Qt::UserRole);
		finDataFolder.first()->appendRow(dataItem);

		dataItem = prepareRow("", "is Double Fin:", pWing->IsDoubleFin() ? "true": "false");
		dataItem.at(2)->setData(XFLR5::BOOL, Qt::UserRole);
		finDataFolder.first()->appendRow(dataItem);
	}

	QList<QStandardItem*> wingPositionFolder = prepareRow("Position");
	wingFolder.first()->appendRow(wingPositionFolder);
	{
		dataItem = prepareDoubleRow("", "x", m_pPlane->WingLE(iw).x*Units::mtoUnit(), Units::lengthUnitLabel());
		wingPositionFolder.first()->appendRow(dataItem);

		dataItem = prepareDoubleRow("", "z", m_pPlane->WingLE(iw).z*Units::mtoUnit(), Units::lengthUnitLabel());
		wingPositionFolder.first()->appendRow(dataItem);

		dataItem = prepareDoubleRow("", "Tilt angle", m_pPlane->WingTiltAngle(iw), QString::fromUtf8("°"));
		wingPositionFolder.first()->appendRow(dataItem);
	}

	QList<QStandardItem*> wingInertiaFolder = prepareRow("Inertia");
	wingFolder.first()->appendRow(wingInertiaFolder);
	{
		QList<QStandardItem*> dataItem = prepareDoubleRow( "Structural (volume) mass", "", pWing->VolumeMass()*Units::kgtoUnit(), Units::weightUnitLabel());
		wingInertiaFolder.first()->appendRow(dataItem);

		for(int iwm=0; iwm<pWing->m_PointMass.size(); iwm++)
		{
			PointMass *pm = pWing->m_PointMass.at(iwm);
			QList<QStandardItem*> wingPointMassFolder = prepareRow("Point mass");

			wingInertiaFolder.first()->appendRow(wingPointMassFolder);
			{
				QList<QStandardItem*> dataItem = prepareRow("", "Tag", pm->tag());

				dataItem = prepareDoubleRow("", "mass", pm->mass()*Units::kgtoUnit(), Units::weightUnitLabel());
				wingPointMassFolder.first()->appendRow(dataItem);

				dataItem = prepareDoubleRow("", "x", pm->position().x*Units::mtoUnit(), Units::lengthUnitLabel());
				wingPointMassFolder.first()->appendRow(dataItem);

				dataItem = prepareDoubleRow("", "y", pm->position().y*Units::mtoUnit(), Units::lengthUnitLabel());;
				wingPointMassFolder.first()->appendRow(dataItem);

				dataItem = prepareDoubleRow("", "z", pm->position().z*Units::mtoUnit(), Units::lengthUnitLabel());
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

			QList<QStandardItem*> sectionFolder = prepareRow(QString("Section %1").arg(iws));
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

				dataItem = prepareIntRow("", "x-direction number of panels", wingsec->m_NXPanels);
				sectionFolder.first()->appendRow(dataItem);

				dataItem = prepareRow("", "x-direction distrib.", distributionType(wingsec->m_XPanelDist));
				dataItem.at(2)->setData(XFLR5::PANELDISTRIBUTION, Qt::UserRole);
				sectionFolder.first()->appendRow(dataItem);

				dataItem = prepareIntRow("", "y-direction number of panels",wingsec->m_NYPanels);
				sectionFolder.first()->appendRow(dataItem);

				dataItem = prepareRow("", "y-direction distrib.", distributionType(wingsec->m_YPanelDist));
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



void ViewObjectDlg::showBody(QStandardItem*planeRootItem)
{
	Body *pBody = m_pPlane->body();

	QList<QStandardItem*> bodyFolder = prepareRow("Body");
	planeRootItem->appendRow(bodyFolder);

	QList<QStandardItem*> dataItem = prepareRow("", "Active body", m_pPlane->body() ? "true" : "false");
	dataItem.at(2)->setData(XFLR5::BOOL, Qt::UserRole);
	bodyFolder.first()->appendRow(dataItem);

	if(!m_pPlane->body()) return;

	dataItem = prepareRow("", "Name", m_pPlane->body()->bodyName());
	dataItem.at(2)->setData(XFLR5::STRING, Qt::UserRole);
	bodyFolder.first()->appendRow(dataItem);

	dataItem = prepareRow("", "Type", bodyPanelType(pBody->bodyType()));
	dataItem.at(2)->setData(XFLR5::BODYTYPE, Qt::UserRole);
	bodyFolder.first()->appendRow(dataItem);

	QList<QStandardItem*> bodyColorFolder = prepareRow("Color");
	bodyFolder.first()->appendRow(bodyColorFolder);
	{
		QList<QStandardItem*> dataItem = prepareRow("", "red", QString("%1").arg(pBody->bodyColor().red()));
		dataItem.at(2)->setData(XFLR5::INTEGER, Qt::UserRole);
		bodyColorFolder.first()->appendRow(dataItem);

		dataItem = prepareRow("", "green", QString("%1").arg(pBody->bodyColor().green()));
		dataItem.at(2)->setData(XFLR5::INTEGER, Qt::UserRole);
		bodyColorFolder.first()->appendRow(dataItem);

		dataItem = prepareRow("", "blue", QString("%1").arg(pBody->bodyColor().blue()));
		dataItem.at(2)->setData(XFLR5::INTEGER, Qt::UserRole);
		bodyColorFolder.first()->appendRow(dataItem);
	}

	QList<QStandardItem*> bodyPositionFolder = prepareRow("Position");
	bodyFolder.first()->appendRow(bodyPositionFolder);
	{
		QList<QStandardItem*> dataItem = prepareRow("", "x", QString("%1").arg(m_pPlane->bodyPos().x*Units::mtoUnit()), Units::lengthUnitLabel());
		dataItem.at(2)->setData(XFLR5::DOUBLE, Qt::UserRole);
		bodyPositionFolder.first()->appendRow(dataItem);

		dataItem = prepareRow("", "z", QString("%1").arg(m_pPlane->bodyPos().z*Units::mtoUnit()), Units::lengthUnitLabel());
		dataItem.at(2)->setData(XFLR5::DOUBLE, Qt::UserRole);
		bodyPositionFolder.first()->appendRow(dataItem);
	}

	QList<QStandardItem*> bodyInertiaFolder = prepareRow("Inertia");
	bodyFolder.first()->appendRow(bodyInertiaFolder);
	{
		QList<QStandardItem*> dataItem = prepareRow( "Structural (volume) mass", "", QString("%1").arg(pBody->m_VolumeMass*Units::kgtoUnit()), Units::weightUnitLabel());
		dataItem.at(2)->setData(XFLR5::DOUBLE, Qt::UserRole);
		bodyInertiaFolder.first()->appendRow(dataItem);

		for(int iwm=0; iwm<pBody->m_PointMass.size(); iwm++)
		{
			PointMass *pm = pBody->m_PointMass.at(iwm);
			QList<QStandardItem*> bodyPointMassFolder = prepareRow("Point mass");

			bodyInertiaFolder.first()->appendRow(bodyPointMassFolder);
			{
				QList<QStandardItem*> dataItem = prepareRow("", "Tag", pm->tag());
				dataItem.at(2)->setData(XFLR5::STRING, Qt::UserRole);
				bodyPointMassFolder.first()->appendRow(dataItem);

				dataItem = prepareRow("", "mass", QString("%1").arg(pm->mass()*Units::kgtoUnit()), Units::weightUnitLabel());
				dataItem.at(2)->setData(XFLR5::DOUBLE, Qt::UserRole);
				bodyPointMassFolder.first()->appendRow(dataItem);

				dataItem = prepareRow("", "x", QString("%1").arg(pm->position().x*Units::mtoUnit()), Units::lengthUnitLabel());
				dataItem.at(2)->setData(XFLR5::DOUBLE, Qt::UserRole);
				bodyPointMassFolder.first()->appendRow(dataItem);

				dataItem = prepareRow("", "y", QString("%1").arg(pm->position().y*Units::mtoUnit()), Units::lengthUnitLabel());;
				dataItem.at(2)->setData(XFLR5::DOUBLE, Qt::UserRole);
				bodyPointMassFolder.first()->appendRow(dataItem);

				dataItem = prepareRow("", "z", QString("%1").arg(pm->position().z*Units::mtoUnit()), Units::lengthUnitLabel());
				dataItem.at(2)->setData(XFLR5::DOUBLE, Qt::UserRole);
				bodyPointMassFolder.first()->appendRow(dataItem);
			}
		}
	}

	QList<QStandardItem*> NURBSFolder = prepareRow("NURBS");
	bodyFolder.first()->appendRow(NURBSFolder);
	{
		QList<QStandardItem*> dataItem = prepareRow("", "NURBS degree in lengthwise direction", QString("%1").arg(pBody->splineSurface()->uDegree()));
		dataItem.at(2)->setData(XFLR5::INTEGER, Qt::UserRole);
		NURBSFolder.first()->appendRow(dataItem);

		dataItem = prepareRow("", "NURBS degree in hoop direction", QString("%1").arg(pBody->splineSurface()->vDegree()));
		dataItem.at(2)->setData(XFLR5::INTEGER, Qt::UserRole);
		NURBSFolder.first()->appendRow(dataItem);

		dataItem = prepareRow("", "Number of lengthwise mesh panels", QString("%1").arg(pBody->m_nxPanels));
		dataItem.at(2)->setData(XFLR5::INTEGER, Qt::UserRole);
		NURBSFolder.first()->appendRow(dataItem);

		dataItem = prepareRow("", "Number of hoop mesh panels", QString("%1").arg(pBody->m_nhPanels));
		dataItem.at(2)->setData(XFLR5::INTEGER, Qt::UserRole);
		NURBSFolder.first()->appendRow(dataItem);
	}

	QList<QStandardItem*> hoopFolder = prepareRow("Hoop panels (FLATPANELS case)");
	bodyFolder.first()->appendRow(hoopFolder);
	{
		for(int isl=0; isl<pBody->SideLineCount(); isl++)
		{
			QList<QStandardItem*> dataItem = prepareRow("", QString("Hoop panels in stripe %1").arg(isl), QString("%1").arg(pBody->m_hPanels.at(isl)));
			dataItem.at(2)->setData(XFLR5::INTEGER, Qt::UserRole);
			hoopFolder.first()->appendRow(dataItem);
		}
	}

	QList<QStandardItem*> bodyFrameFolder = prepareRow("Sections");
	bodyFolder.first()->appendRow(bodyFrameFolder);
	{
		for(int iFrame=0; iFrame<pBody->splineSurface()->frameCount(); iFrame++)
		{
			Frame *pFrame = pBody->splineSurface()->frameAt(iFrame);

			QList<QStandardItem*> sectionFolder = prepareRow(QString("Frame %1").arg(iFrame));
			bodyFrameFolder.first()->appendRow(sectionFolder);
			{
				dataItem = prepareRow("", "Lengthwise panels (FLATPANELS case)", QString("%1").arg(pBody->m_xPanels.at(iFrame)));
				dataItem.at(2)->setData(XFLR5::INTEGER, Qt::UserRole);
				sectionFolder.first()->appendRow(dataItem);

				QList<QStandardItem*> positionFolder = prepareRow("Position");
				sectionFolder.first()->appendRow(positionFolder);
				{
					QList<QStandardItem*> dataItem = prepareRow("", "x", QString("%1").arg(pFrame->m_Position.x*Units::mtoUnit()), Units::lengthUnitLabel());
					dataItem.at(2)->setData(XFLR5::DOUBLE, Qt::UserRole);
					positionFolder.first()->appendRow(dataItem);

					dataItem = prepareRow("", "z", QString("%1").arg(pFrame->m_Position.z*Units::mtoUnit()), Units::lengthUnitLabel());
					dataItem.at(2)->setData(XFLR5::DOUBLE, Qt::UserRole);
					positionFolder.first()->appendRow(dataItem);
				}

				for(int iPt=0; iPt<pFrame->PointCount(); iPt++)
				{
					QList<QStandardItem*> pointFolder = prepareRow(QString("Point %1").arg(iPt));
					sectionFolder.first()->appendRow(pointFolder);
					{
						CVector Pt(pFrame->Point(iPt));
						QList<QStandardItem*> dataItem = prepareRow("", "x", QString("%1").arg(Pt.x*Units::mtoUnit()), Units::lengthUnitLabel());
						dataItem.at(2)->setData(XFLR5::DOUBLE, Qt::UserRole);
						pointFolder.first()->appendRow(dataItem);

						dataItem = prepareRow("", "y", QString("%1").arg(Pt.y*Units::mtoUnit()), Units::lengthUnitLabel());
						dataItem.at(2)->setData(XFLR5::DOUBLE, Qt::UserRole);
						pointFolder.first()->appendRow(dataItem);

						dataItem = prepareRow("", "z", QString("%1").arg(Pt.z*Units::mtoUnit()), Units::lengthUnitLabel());
						dataItem.at(2)->setData(XFLR5::DOUBLE, Qt::UserRole);
						pointFolder.first()->appendRow(dataItem);
					}
				}
			}
		}
	}
}



void ViewObjectDlg::GLDraw3D()
{
	m_pGLWidget->makeCurrent();
	glClearColor(Settings::s_BackgroundColor.redF(), Settings::s_BackgroundColor.greenF(), Settings::s_BackgroundColor.blueF(),0.0);


	if(m_bResetglSectionHighlight || m_bResetglPlane)
	{
		if(glIsList(SECTIONHIGHLIGHT))
		{
			glDeleteLists(SECTIONHIGHLIGHT,1);
		}
		if(m_iSection>=0)
		{
			GLCreateSectionHighlight(m_pPlane->wing());
			m_bResetglSectionHighlight = false;
		}
	}

	if(m_bResetglPlane)
	{
		Wing *pWingList[MAXWINGS] = {m_pPlane->wing(), m_pPlane->wing2(), m_pPlane->stab(), m_pPlane->fin()};
		GLCreateGeom(WINGSURFACES, pWingList, m_pPlane->body());

		if(glIsList(MESHPANELS))
		{
			glDeleteLists(MESHPANELS,2);
		}

		GLCreateMesh(pWingList);

		m_bResetglPlane = false;
	}
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
	}

	if(m_iSection>=0)
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
		glColor3d(W3dPrefsDlg::s_MassColor.redF(),W3dPrefsDlg::s_MassColor.greenF(),W3dPrefsDlg::s_MassColor.blueF());
		for(int im=0; im<m_pPlane->wing()->m_PointMass.size(); im++)
		{
			glPushMatrix();
			{
				glTranslated(m_pPlane->wing()->m_PointMass[im]->position().x,
							 m_pPlane->wing()->m_PointMass[im]->position().y,
							 m_pPlane->wing()->m_PointMass[im]->position().z);
				glColor3d(W3dPrefsDlg::s_MassColor.redF(), W3dPrefsDlg::s_MassColor.greenF(), W3dPrefsDlg::s_MassColor.blueF());
				m_pGLWidget->GLRenderSphere(W3dPrefsDlg::s_MassRadius/glScaled);
				glColor3d(Settings::s_TextColor.redF(), Settings::s_TextColor.greenF(), Settings::s_TextColor.blueF());
				m_pGLWidget->renderText( 0.0, 0.0, 0.0 +.02,
										 m_pPlane->wing()->m_PointMass[im]->tag()
								+QString(" %1").arg(m_pPlane->wing()->m_PointMass[im]->mass()*Units::kgtoUnit(), 7,'g',3)
										+MassUnit);
			}
			glPopMatrix();
		}
	}
}




void ViewObjectDlg::GLCreateSectionHighlight(Wing *m_pWing)
{
	int iSection = 0;
	int jSurf = 0;
	for(int jSection=0; jSection<m_pWing->NWingSection(); jSection++)
	{
		if(jSection==m_iSection) break;
		if(qAbs(m_pWing->YPosition(jSection+1)-m_pWing->YPosition(jSection)) > Wing::s_MinPanelSize)
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
			if(!m_pWing->IsFin())  jSurf = m_pWing->m_Surface.size()/2 - 1;
			else                   jSurf = m_pWing->m_Surface.size()   - 1;

			//plot B side outline
			glBegin(GL_LINE_STRIP);
			{
				for (int lx=0; lx<m_pWing->m_Surface.at(jSurf)->m_NXPanels; lx++)
				{
					m_pWing->m_Surface.at(jSurf)->GetPanel(m_pWing->m_Surface.at(jSurf)->m_NYPanels-1, lx, TOPSURFACE);
					glVertex3d(m_pWing->m_Surface.at(jSurf)->TB.x,
							   m_pWing->m_Surface.at(jSurf)->TB.y,
							   m_pWing->m_Surface.at(jSurf)->TB.z);
				}

				glVertex3d(m_pWing->m_Surface.at(jSurf)->LB.x,
						   m_pWing->m_Surface.at(jSurf)->LB.y,
						   m_pWing->m_Surface.at(jSurf)->LB.z);

				for (int lx=m_pWing->m_Surface.at(jSurf)->m_NXPanels-1; lx>=0; lx--)
				{
					m_pWing->m_Surface.at(jSurf)->GetPanel(m_pWing->m_Surface.at(jSurf)->m_NYPanels-1, lx, BOTSURFACE);
					glVertex3d(m_pWing->m_Surface.at(jSurf)->TB.x,
							   m_pWing->m_Surface.at(jSurf)->TB.y,
							   m_pWing->m_Surface.at(jSurf)->TB.z);
				}
			}
			glEnd();
		}
		else
		{
//			if((m_pWing->m_bSymetric || m_bRightSide) && !m_pWing->m_bIsFin)
			if(!m_pWing->m_bIsFin)
			{
				jSurf = m_pWing->m_Surface.size()/2 + iSection -1;

				glBegin(GL_LINE_STRIP);
				{
					for (int lx=0; lx<m_pWing->m_Surface.at(jSurf)->m_NXPanels; lx++)
					{
						m_pWing->m_Surface.at(jSurf)->GetPanel(m_pWing->m_Surface.at(jSurf)->m_NYPanels-1, lx, TOPSURFACE);
						glVertex3d(m_pWing->m_Surface.at(jSurf)->TB.x,
								 m_pWing->m_Surface.at(jSurf)->TB.y,
								 m_pWing->m_Surface.at(jSurf)->TB.z);
					}

					glVertex3d(m_pWing->m_Surface.at(jSurf)->LB.x,
							 m_pWing->m_Surface.at(jSurf)->LB.y,
							 m_pWing->m_Surface.at(jSurf)->LB.z);

					for (int lx=m_pWing->m_Surface.at(jSurf)->m_NXPanels-1; lx>=0; lx--)
					{
						m_pWing->m_Surface.at(jSurf)->GetPanel(m_pWing->m_Surface.at(jSurf)->m_NYPanels-1, lx, BOTSURFACE);
						glVertex3d(m_pWing->m_Surface.at(jSurf)->TB.x,
								 m_pWing->m_Surface.at(jSurf)->TB.y,
								 m_pWing->m_Surface.at(jSurf)->TB.z);
					}
				}
				glEnd();
			}

			if(m_pWing->m_bSymetric)
			{
				if(!m_pWing->m_bIsFin) jSurf = m_pWing->m_Surface.size()/2 - iSection;
				else                   jSurf = m_pWing->m_Surface.size()   - iSection;

				//plot A side outline
				glBegin(GL_LINE_STRIP);
				{
					for (int lx=0; lx<m_pWing->m_Surface.at(jSurf)->m_NXPanels; lx++)
					{
						m_pWing->m_Surface.at(jSurf)->GetPanel(0, lx, TOPSURFACE);
						glVertex3d(m_pWing->m_Surface.at(jSurf)->TA.x,
								   m_pWing->m_Surface.at(jSurf)->TA.y,
								   m_pWing->m_Surface.at(jSurf)->TA.z);
					}

					glVertex3d(m_pWing->m_Surface.at(jSurf)->LA.x,
							   m_pWing->m_Surface.at(jSurf)->LA.y,
							   m_pWing->m_Surface.at(jSurf)->LA.z);

					for (int lx=m_pWing->m_Surface.at(jSurf)->m_NXPanels-1; lx>=0; lx--)
					{
						m_pWing->m_Surface.at(jSurf)->GetPanel(0, lx, BOTSURFACE);
						glVertex3d(m_pWing->m_Surface.at(jSurf)->TA.x,
								   m_pWing->m_Surface.at(jSurf)->TA.y,
								   m_pWing->m_Surface.at(jSurf)->TA.z);
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











