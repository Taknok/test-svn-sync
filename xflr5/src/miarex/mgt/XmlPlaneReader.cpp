/****************************************************************************

	XMLPlaneReader Class
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

#include "XmlPlaneReader.h"
#include <QMessageBox>


XMLPlaneReader::XMLPlaneReader(QFile &file, Plane *pPlane)
{
	m_pPlane = pPlane;
	setDevice(&file);
}



void XMLPlaneReader::readXMLPlaneFile()
{
	double lengthunit = 1.0;
	double massunit = 1.0;

	if (readNextStartElement())
	{
		if (name() == "explane" && attributes().value("version") == "1.0")
		{
			while(!atEnd() && !hasError() && readNextStartElement() )
			{
				if (name().toString().compare("units", Qt::CaseInsensitive)==0)
				{
					while(!atEnd() && !hasError() && readNextStartElement() )
					{
						if (name().compare("length_unit_to_meter",      Qt::CaseInsensitive)==0)
						{
							lengthunit = readElementText().toDouble();
						}
						else if (name().compare("mass_unit_to_kg",      Qt::CaseInsensitive)==0)
						{
							massunit = readElementText().toDouble();
						}
						else
							skipCurrentElement();
					}
				}
				else if (name().toString().compare("plane", Qt::CaseInsensitive)==0)
				{
					readPlane(m_pPlane, lengthunit, massunit);
				}
				else if (name().toString().compare("body", Qt::CaseInsensitive)==0)
				{
					if(m_pPlane->body()) delete m_pPlane->body();
					m_pPlane->setBody(new Body);
					readBody(m_pPlane->body(), m_pPlane->bodyPos(), lengthunit, massunit);
				}
			}
		}
		else
			raiseError(QObject::tr("The file is not an xflr5 plane version 1.0 file."));
	}
}




/** */
void XMLPlaneReader::readPlane(Plane *pPlane, double lengthUnit, double massUnit)
{
	int iw=0;
	while(!atEnd() && !hasError() && readNextStartElement() && iw<MAXWINGS)
	{
		if (name().toString().compare("name",Qt::CaseInsensitive) ==0)
		{
			pPlane->rPlaneName() = readElementText();
		}
		else if (name().toString().compare("has_body",Qt::CaseInsensitive) ==0)
		{
			pPlane->hasBody() = readElementText().compare("true", Qt::CaseInsensitive)==0;
		}
		else if (name().toString().compare("description", Qt::CaseInsensitive)==0)
		{
			pPlane->rPlaneDescription() = readElementText();
		}
		else if (name().compare("Inertia",         Qt::CaseInsensitive)==0)
		{
			while(!atEnd() && !hasError() && readNextStartElement() )
			{
				if (name().compare("point_mass", Qt::CaseInsensitive)==0)
				{
					PointMass* ppm = new PointMass;
					pPlane->m_PointMass.append(ppm);
					readPointMass(ppm, massUnit, lengthUnit);
				}
				else
					skipCurrentElement();
			}
		}
		else if (name().toString().compare("body", Qt::CaseInsensitive)==0)
		{
			pPlane->setBody(new Body);
			readBody(pPlane->body(), pPlane->bodyPos(), lengthUnit, massUnit);
		}
		else if (name().toString().compare("wing", Qt::CaseInsensitive)==0)
		{
			double xw=0.0, zw=0.0, ta=0.0;
			Wing newWing;
			{
				newWing.m_WingSection.clear();

				while(!atEnd() && !hasError() && readNextStartElement() )
				{
					if (name().compare("name",                 Qt::CaseInsensitive)==0)
					{
						newWing.rWingName() = readElementText();
					}
					else if (name().compare("color",           Qt::CaseInsensitive)==0)
					{
						readColor(newWing.wingColor());
					}
					else if (name().compare("description",     Qt::CaseInsensitive)==0)
					{
						newWing.rWingDescription() = readElementText();
					}
					else if (name().compare("position",        Qt::CaseInsensitive)==0)
					{
						QStringList coordList = readElementText().split(",");
						if(coordList.length()>=3)
						{
							xw = coordList.at(0).toDouble()*lengthUnit;
							zw = coordList.at(2).toDouble()*lengthUnit;
						}
					}
					else if (name().compare("tilt_angle",      Qt::CaseInsensitive)==0)
					{
						ta = readElementText().toDouble();
					}
					else if (name().compare("Symetric",        Qt::CaseInsensitive)==0)
					{
						newWing.isSymetric() = readElementText().compare("true", Qt::CaseInsensitive)==0;
					}
					else if (name().compare("isFin",           Qt::CaseInsensitive)==0)
					{
						newWing.isFin() = readElementText().compare("true", Qt::CaseInsensitive)==0;
					}
					else if (name().compare("isDoubleFin",     Qt::CaseInsensitive)==0)
					{
						newWing.isDoubleFin() = readElementText().compare("true", Qt::CaseInsensitive)==0;
					}
					else if (name().compare("isSymFin",        Qt::CaseInsensitive)==0)
					{
						newWing.isSymFin() = readElementText().compare("true", Qt::CaseInsensitive)==0;
					}
					else if (name().compare("Inertia",         Qt::CaseInsensitive)==0)
					{
						while(!atEnd() && !hasError() && readNextStartElement() )
						{
							if (name().compare("volume_mass", Qt::CaseInsensitive)==0)
							{
								newWing.volumeMass() = readElementText().toDouble();
							}
							else if (name().compare("point_mass", Qt::CaseInsensitive)==0)
							{
								PointMass* ppm = new PointMass;
								newWing.m_PointMass.append(ppm);
								readPointMass(ppm, massUnit, lengthUnit);
							}
							else
								skipCurrentElement();
						}
					}
					else if (name().compare("Sections",        Qt::CaseInsensitive)==0)
					{
						while(!atEnd() && !hasError() && readNextStartElement() )
						{
							if (name().compare("Section",  Qt::CaseInsensitive)==0)
							{
								WingSection *pWingSec = new WingSection;
								newWing.m_WingSection.append(pWingSec);
								while(!atEnd() && !hasError() && readNextStartElement() )
								{
									if (name().compare("x_number_of_panels", Qt::CaseInsensitive)==0)
									{
										pWingSec->m_NXPanels = readElementText().toInt();
									}
									else if (name().compare("y_number_of_panels", Qt::CaseInsensitive)==0)
									{
										pWingSec->m_NYPanels = readElementText().toInt();
									}
									else if (name().compare("x_panel_distribution", Qt::CaseInsensitive)==0)
									{
										QString strPanelDist = readElementText();
										pWingSec->m_XPanelDist = distributionType(strPanelDist);
									}
									else if (name().compare("y_panel_distribution", Qt::CaseInsensitive)==0)
									{
										QString strPanelDist = readElementText();
										pWingSec->m_YPanelDist = distributionType(strPanelDist);
									}
									else if (name().compare("Chord", Qt::CaseInsensitive)==0)
									{
										pWingSec->m_Chord = readElementText().toDouble()*lengthUnit;
									}
									else if (name().compare("y_position", Qt::CaseInsensitive)==0)
									{
										pWingSec->m_YPosition = readElementText().toDouble()*lengthUnit;
									}
									else if (name().compare("xOffset", Qt::CaseInsensitive)==0)
									{
										pWingSec->m_Offset = readElementText().toDouble()*lengthUnit;
									}
									else if (name().compare("Dihedral", Qt::CaseInsensitive)==0)
									{
										pWingSec->m_Dihedral = readElementText().toDouble();
									}
									else if (name().compare("Twist", Qt::CaseInsensitive)==0)
									{
										pWingSec->m_Twist = readElementText().toDouble();
									}
									else if (name().compare("Left_Side_FoilName", Qt::CaseInsensitive)==0)
									{
										pWingSec->m_LeftFoilName = readElementText();
									}
									else if (name().compare("Right_Side_FoilName", Qt::CaseInsensitive)==0)
									{
										pWingSec->m_RightFoilName = readElementText();
									}
									else
										skipCurrentElement();
								}
							}
							else
								skipCurrentElement();
						}
					}
					else
						skipCurrentElement();
				}

				int iWing = 0;
				if(newWing.isFin()) iWing = 3;
				else if(iw==0)      iWing = 0;
				else if(iw==1)      iWing = 2;

				pPlane->m_Wing[iWing].Duplicate(&newWing);
				pPlane->WingLE(iWing).x      = xw;
				pPlane->WingLE(iWing).z      = zw;
				pPlane->WingTiltAngle(iWing) = ta;

				iw++;
			}
		}
		else
			skipCurrentElement();
	}
}



void XMLPlaneReader::readPointMass(PointMass *ppm, double massUnit, double lengthUnit)
{
	while (readNextStartElement())
	{
		if (name().compare("tag", Qt::CaseInsensitive)==0)       ppm->tag() = readElementText();
		else if (name().compare("mass", Qt::CaseInsensitive)==0) ppm->mass() =  readElementText().toDouble()*massUnit;
		else if (name().compare("coordinates", Qt::CaseInsensitive)==0)
		{
			QStringList coordList = readElementText().split(",");
			if(coordList.length()>=3)
			{
				ppm->position().x = coordList.at(0).toDouble()*lengthUnit;
				ppm->position().y = coordList.at(1).toDouble()*lengthUnit;
				ppm->position().z = coordList.at(2).toDouble()*lengthUnit;
			}
		}
//		else if (name().compare("x", Qt::CaseInsensitive)==0)    ppm->m_Position.x =  readElementText().toDouble()*lengthUnit;
//		else if (name().compare("y", Qt::CaseInsensitive)==0)    ppm->m_Position.y =  readElementText().toDouble()*lengthUnit;
//		else if (name().compare("z", Qt::CaseInsensitive)==0)    ppm->m_Position.z =  readElementText().toDouble()*lengthUnit;
		else skipCurrentElement();

	}
}



void XMLPlaneReader::readBody(Body *pBody, CVector &position, double lengthUnit, double massUnit)
{
	pBody->splineSurface()->ClearFrames();

	while(!atEnd() && !hasError() && readNextStartElement() )
	{
		if (name().toString().compare("name",Qt::CaseInsensitive) ==0)
		{
			pBody->bodyName() = readElementText();
		}
		else if (name().toString().compare("color", Qt::CaseInsensitive)==0)
		{
			readColor(pBody->bodyColor());
		}
		else if (name().toString().compare("description", Qt::CaseInsensitive)==0)
		{
			pBody->bodyDescription() = readElementText();
		}
		else if (name().compare("Inertia",         Qt::CaseInsensitive)==0)
		{
			while(!atEnd() && !hasError() && readNextStartElement() )
			{
				if (name().compare("volume_mass", Qt::CaseInsensitive)==0)
				{
					pBody->m_VolumeMass = readElementText().toDouble();
				}
				else if (name().compare("point_mass", Qt::CaseInsensitive)==0)
				{
					PointMass* ppm = new PointMass;
					pBody->m_PointMass.append(ppm);
					readPointMass(ppm, massUnit, lengthUnit);
				}
				else
					skipCurrentElement();
			}
		}
		else if (name().compare("position", Qt::CaseInsensitive)==0)
		{
			QStringList coordList = readElementText().split(",");
			if(coordList.length()>=3)
			{
				position.x = coordList.at(0).toDouble()*lengthUnit;
				position.z = coordList.at(2).toDouble()*lengthUnit;
			}
		}		else if (name().compare("type", Qt::CaseInsensitive)==0)
		{
			if(readElementText().compare("NURBS", Qt::CaseInsensitive)==0) pBody->bodyType()=XFLR5::BODYSPLINETYPE;
			else                                                               pBody->bodyType()=XFLR5::BODYPANELTYPE;
		}
		else if (name().compare("x_degree",    Qt::CaseInsensitive)==0) pBody->splineSurface()->m_iuDegree = readElementText().toInt();
		else if (name().compare("hoop_degree", Qt::CaseInsensitive)==0) pBody->splineSurface()->m_ivDegree = readElementText().toInt();
		else if (name().compare("x_panels",    Qt::CaseInsensitive)==0) pBody->m_nxPanels = readElementText().toInt();
		else if (name().compare("hoop_panels", Qt::CaseInsensitive)==0) pBody->m_nhPanels = readElementText().toInt();

		//read frames
		else if (name().compare("frame", Qt::CaseInsensitive)==0)
		{
			Frame *pFrame = pBody->splineSurface()->appendNewFrame();
			while(!atEnd() && !hasError() && readNextStartElement() )
			{
				if(name().compare("x_panels", Qt::CaseInsensitive)==0) pBody->m_xPanels.append(readElementText().toInt());
				if(name().compare("h_panels", Qt::CaseInsensitive)==0) pBody->m_hPanels.append(readElementText().toInt());
				else if (name().compare("position", Qt::CaseInsensitive)==0)
				{
					QStringList coordList = readElementText().split(",");
					if(coordList.length()>=3)
					{
						pFrame->m_Position.x = coordList.at(0).toDouble()*lengthUnit;
						pFrame->m_Position.z = coordList.at(2).toDouble()*lengthUnit;
					}
				}
				else if (name().compare("point", Qt::CaseInsensitive)==0)
				{
					CVector ctrlPt;
					QStringList coordList = readElementText().split(",");
					if(coordList.length()>=3)
					{
						ctrlPt.x = coordList.at(0).toDouble()*lengthUnit;
						ctrlPt.y = coordList.at(1).toDouble()*lengthUnit;
						ctrlPt.z = coordList.at(2).toDouble()*lengthUnit;
						pFrame->AppendPoint(ctrlPt);
					}
				}
			}
		}
	}
}




void XMLPlaneReader::readColor(QColor &color)
{
	color.setRgb(0,0,0,255);
	while (readNextStartElement())
	{
		if (name().compare("red", Qt::CaseInsensitive)==0)         color.setRed(readElementText().toInt());
		else if (name().compare("green", Qt::CaseInsensitive)==0)  color.setGreen(readElementText().toInt());
		else if (name().compare("blue", Qt::CaseInsensitive)==0)   color.setBlue(readElementText().toInt());
		else if (name().compare("alpha", Qt::CaseInsensitive)==0)  color.setAlpha(readElementText().toInt());
		else skipCurrentElement();

	}
}
