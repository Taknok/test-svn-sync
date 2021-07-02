/****************************************************************************

    PointMass Class
    Copyright (C) 2013 André Deperrois 

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


#ifndef POINTMASS_H
#define POINTMASS_H

#include <QString>
#include <xflgeom/geom3d/vector3d.h>


class PointMass
{
public:
    /** The public constructor */
    PointMass()
    {
        m_Mass = 0.0;
    }

    /** Overloaded public constructor */
    PointMass(PointMass const* pPtMass)
    {
        m_Mass = pPtMass->m_Mass;
        m_Position = pPtMass->m_Position;
        m_Tag = pPtMass->m_Tag;
    }

    /** Overloaded public constructor */
    PointMass(double const &m, Vector3d const &p, QString const &tag)
    {
        m_Mass = m;
        m_Position = p;
        m_Tag = tag;
    }

    void setPointMass(double m, Vector3d const &pos, QString const &name)
    {
        m_Mass = m;
        m_Position = pos;
        m_Tag = name;
    }

    /** Returns the the value of the mass */
    double mass()  const {return m_Mass;}
    void setMass(double m) {m_Mass=m;}

    /** Returns the the position of the mass */
    Vector3d const &position() const {return m_Position;}
    void setPosition(Vector3d pos) {m_Position=pos;}
    void setXPos(double x) {m_Position.x=x;}
    void setYPos(double y) {m_Position.x=y;}
    void setZPos(double z) {m_Position.x=z;}


    /** Returns the the tag of the mass */
    QString const &tag() const {return m_Tag;}
    void setTag(QString name) {m_Tag=name;}

private:
    double m_Mass;          /**< the value of the point mass, in kg */
    Vector3d m_Position;      /**< the absolute position of the point mass */
    QString m_Tag;           /**< the description of the point mass */

};

#endif // POINTMASS_H
