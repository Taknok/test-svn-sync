/****************************************************************************

    MOPSOTask Class
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

#include <QWidget>
#include <QEvent>

#include <xdirect/optim2d/optimtask.h>
#include <xflanalysis/analysis3d_enums.h>
#include <xflcore/xflevents.h>

#include "particle.h"
#include "optstructures.h"


/**
 * @brief Abstract base class for MOPSO algorithms
 * Requires derivation and implementation of calcFitness() and error() functions
 */
class MOPSOTask : public OptimTask
{
    Q_OBJECT

    public:
        MOPSOTask();

        void setNObjectives(int nObj) {m_Objective.resize(nObj);}
        OptObjective const &objective(int iobj) const {return m_Objective.at(iobj);}

        void makeSwarm() override;
        void makeRandomParticle(Particle *pParticle) const override;

        int nObjectives() const {return m_Objective.size();}
        void setObjective(int iobj, const OptObjective &obj) {m_Objective[iobj] = obj;}

        Particle const &bestParticle() const {return m_Bestparticle;}

        static void restoreDefaults();
        static void setWeights(double in, double cog, double soc) {s_InertiaWeight=in; s_CognitiveWeight=cog; s_SocialWeight=soc;}

    protected:
        virtual double error(const Particle &particle, int iObjective) const override;


    private:        
        void makeParetoFront();
        void moveParticle(Particle *pParticle);
        void postIterEvent(int iBest) override;

    private slots:
        void onIteration() override;

    protected:
        QVector<Particle> m_Pareto; /**< the particles which make the Pareto front */
        Particle m_Bestparticle;

        // size = nObjectives
        QVector<OptObjective> m_Objective;

    public:
        static int    s_ArchiveSize;
        static double s_InertiaWeight;
        static double s_CognitiveWeight;
        static double s_SocialWeight;

};





