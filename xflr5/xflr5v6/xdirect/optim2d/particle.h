/****************************************************************************

    Particle Class
    Copyright (C) 2021 Andre Deperrois

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

#pragma once

#include <QVector>

class Particle
{
    public:
        Particle() {m_Error = m_BestError = 1.0e10; m_Fitness=0.0; m_Dim=0;}

        void setDim(int dim) {m_Dim=dim; m_Position.resize(m_Dim); m_BestPosition.resize(m_Dim); m_Velocity.resize(m_Dim);}

        double error() const {return m_Error;}
        double bestError() const {return m_BestError;}
        void setError(double err) {m_Error=err;}
        void setBestError(double err) {m_BestError=err;}

        QVector<double> const &velocity()     const {return m_Velocity;}
        QVector<double> const &position()     const {return m_Position;}
        QVector<double> const &bestPosition() const {return m_BestPosition;}

        void setVelocity(double const*vel) {for(int i=0; i<m_Dim; i++) m_Velocity[i]=vel[i];}
        void setBestPosition(double const*pos) {for(int i=0; i<m_Dim; i++) m_BestPosition[i]=pos[i];}
        void setPosition(double const*pos) {for(int i=0; i<m_Dim; i++) m_Position[i]=pos[i];}

        void storePosition() {m_BestPosition=m_Position;}

        void setPos(int i, double dble) {m_Position[i]=dble;}
        void setVel(int i, double dble) {m_Velocity[i]=dble;}

        double pos(int i) const {return m_Position.at(i);}
        double bestPos(int i) const {return m_BestPosition.at(i);}
        double vel(int i) const {return m_Velocity.at(i);}

        int dim() const {return m_Dim;}

        void setFitness(double f) {m_Fitness=f;}
        double fitness() const {return m_Fitness;}

    public:
        double m_Error, m_BestError;
        QVector<double> m_Position; // Camb, XCamb, Thick, XThick
        QVector<double> m_Velocity;
        QVector<double> m_BestPosition;

        double m_Fitness;

        double m_Dim;
};
