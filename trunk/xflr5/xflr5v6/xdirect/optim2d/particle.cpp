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

#include <QRandomGenerator>

#include "particle.h"
#include <xflcore/constants.h>


Particle::Particle()
{
    resizeArrays(0, 1, 1);
}


void Particle::resizeArrays(int ndim, int nobj, int nbest)
{
    m_nBest = nbest;

    m_Position.resize(ndim);
    m_Velocity.resize(ndim);

    m_Fitness.resize(nobj);
    m_Error.resize(nobj);
    m_Error.fill(LARGEVALUE);


    m_BestPosition.resize(m_nBest);
    m_BestError.resize(m_nBest);
    for(int i=0; i<m_nBest; i++)
    {
        m_BestPosition[i].resize(ndim);
        m_BestPosition[i].fill(0);

        m_BestError[i].resize(nobj);
        m_BestError[i].fill(LARGEVALUE);
    }
}


void Particle::initializeBest()
{
    for(int i=0; i<m_nBest; i++)
    {
        m_BestPosition[i] = m_Position;
        m_BestError[i].fill(LARGEVALUE);
    }
}


/** Stores the current best if it is non-dominated by the existing solutions */
void Particle::updateBest()
{
    bool bIsDominated = false;
    for(int i=0; i<m_nBest; i++)
    {
        QVector<double> const &olderror = m_BestError.at(i);

        bool bIsWorse = true;
        for(int j=0; j<m_Error.size(); j++)
        {
            if(m_Error.at(j)<olderror.at(j))
            {
                bIsWorse = false;
                break;
            }
        }
        if(bIsWorse)
        {
            bIsDominated = true;
            break;
        }
    }

    if(!bIsDominated)
    {
        // replace at random one particle of the Pareto front
        int irand = QRandomGenerator::global()->bounded(m_nBest);
        m_BestError[irand] = m_Error;
        m_BestPosition[irand] = m_Position;
    }
}
















