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

#include <QApplication>
#include <QtConcurrent/QtConcurrent>
#include <QFutureSynchronizer>
#include <QRandomGenerator>

#include "mopsotask.h"
#include <xdirect/optim2d/optimevent.h>
#include <xflcore/constants.h>


int    MOPSOTask::s_ArchiveSize       = 5;
double MOPSOTask::s_InertiaWeight     = 0.5;
double MOPSOTask::s_CognitiveWeight   = 0.5;
double MOPSOTask::s_SocialWeight      = 0.5;


MOPSOTask::MOPSOTask()
{
}


void MOPSOTask::restoreDefaults()
{
    s_PopSize           = 11;
    s_ArchiveSize       = 5;
    s_MaxIter           = 100;
    s_InertiaWeight     = 0.5;
    s_CognitiveWeight   = 0.5;
    s_SocialWeight      = 0.5;
}


void MOPSOTask::makeSwarm()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    outputMsg("Making swarm...\n");

    m_Swarm.resize(s_PopSize);


    if(s_bMultiThreaded)
    {
        QFutureSynchronizer<void> futureSync;
        for (int isw=0; isw<m_Swarm.size(); isw++)
        {
            Particle &particle = m_Swarm[isw];
            futureSync.addFuture(QtConcurrent::run(this, &MOPSOTask::makeRandomParticle, &particle));
        }
        futureSync.waitForFinished();
    }
    else
    {
        for (int i=0; i<m_Swarm.size(); ++i)
        {
            Particle &particle = m_Swarm[i];
            makeRandomParticle(&particle);
            outputMsg(QString::asprintf("   made particle %2d\n", i));
        }
    }

    outputMsg(QString::asprintf("Made %d random particles\n", s_PopSize));

    outputMsg("Initializing Pareto front...");
    makeParetoFront();
    if(m_Pareto.isEmpty()) outputMsg("   error\n");
    else                   outputMsg("   done\n");

    m_Iter = 0;
    QApplication::restoreOverrideCursor();
}


void MOPSOTask::makeParetoFront()
{
    // make a list of candidate indexes
    QVector<int> indexes;
    for(int i=0; i<m_Swarm.size(); i++) indexes.append(i);

    // remove all indexes of dominated particles
    for(int i=indexes.size()-1; i>=0; i--)
    {
        int index = indexes.at(i);
        Particle const &pi = m_Swarm.at(index);
        for(int j=0; j<m_Swarm.size(); j++)
        {
            if(j!=index)
            {
                Particle const &pj = m_Swarm.at(j);
                bool bIsWorse = true;
                for(int iobj=0; iobj<m_Objective.size(); iobj++)
                {
                    if(error(pi, iobj)<error(pj, iobj))
                    {
                        bIsWorse = false;
                        break;
                    }
                }
                if(bIsWorse)
                {
                    // pi is dominated by pj
                    indexes.remove(i);
                    break; // no need to compare to the rest of the swarm
                }
            }
        }
    }
    m_Pareto.clear();
    for(int i=0; i<indexes.size(); i++)
        m_Pareto.append(m_Swarm.at(indexes.at(i)));
}


void MOPSOTask::onIteration()
{
    if(m_Pareto.isEmpty())
    {
        QString strange("Empty Pareto front:  check input data and rebuild the swarm\n");
        outputMsg(strange);
        m_Status = XFLR5::FINISHED;
        postOptEndEvent(); // tell the GUI that the task is done
        // in case this task has been run in a worker thread, move it back to the main GUI thread so that it may be resumed
        moveToThread(QApplication::instance()->thread());
        return;
    }

    if(s_bMultiThreaded)
    {
        QFutureSynchronizer<void> futureSync;
        for (int isw=0; isw<m_Swarm.size(); isw++)
        {
            Particle &particle = m_Swarm[isw];
            futureSync.addFuture(QtConcurrent::run(this, &MOPSOTask::moveParticle, &particle));
        }
        futureSync.waitForFinished();
    }
    else
    {
        for (int isw=0; isw<m_Swarm.size(); ++isw)
        {
            Particle &particle = m_Swarm[isw];
            moveParticle(&particle);
        }
    }

    m_Iter++;

    makeParetoFront();

    // select the best first objective
    int iParetoBest = -1;
    bool bIsConverged = true;
    double maxerror = LARGEVALUE;
    for(int i=0; i<m_Pareto.size(); i++)
    {
        Particle const &particle = m_Pareto.at(i);
        // track the first objective for user information
        if(particle.error(0)<maxerror)
        {
            iParetoBest = i;
            maxerror = particle.error(0);
            m_Bestparticle = particle;
        }

        // check if the particle meets all criteria
        bIsConverged = true;
        for(int io=0; io<particle.nObjectives(); io++)
        {
            if(particle.error(io)>m_Objective.at(io).m_MaxError)
            {
                bIsConverged = false;
                break;
            }
        }
        if(bIsConverged) break;
    }

    postIterEvent(iParetoBest);

    if(m_Status==XFLR5::CANCELLED)
    {
        outputMsg("Task has been cancelled");
    }

    if(m_Iter>=s_MaxIter || bIsConverged || m_Status==XFLR5::CANCELLED)
    {       
        QString strange("Residual errors:");
        for(int io=0; io<m_Objective.size(); io++)
            strange += QString::asprintf("   err[%d]=%7g\n", io, m_Bestparticle.error(io));
        strange += "\n";
        outputMsg(strange);

        m_Status = XFLR5::FINISHED;

        postOptEndEvent(); // tell the GUI that the task is done

        // in case this task has been run in a worker thread, move it back to the main GUI thread so that it may be resumed
        moveToThread(QApplication::instance()->thread());
    }
}


void MOPSOTask::moveParticle(Particle *pParticle)
{
    double vel=0;
    double r1=0, r2=0;
    double probregen = 0.05;
    int igbest=0, ipbest=0;

    // optional: regenerate particles with random probability
    double regen = QRandomGenerator::global()->bounded(1.0);
    if (regen<probregen)
    {
        makeRandomParticle(pParticle);
    }
    else
    {
        // select a random best in the pareto front
        igbest = QRandomGenerator::global()->bounded(m_Pareto.size());
        Particle const &globalbest = m_Pareto.at(igbest);

        // select a random best in the personal bests
        ipbest = QRandomGenerator::global()->bounded(pParticle->nBest());

        // update the velocity
        for(int j=0; j<pParticle->dimension(); j++)
        {
            r1 = QRandomGenerator::global()->bounded(1.0);
            r2 = QRandomGenerator::global()->bounded(1.0);

            vel = s_InertiaWeight * pParticle->vel(j) +
                  s_CognitiveWeight * r1 * (pParticle->bestPos(ipbest, j) - pParticle->pos(j)) +
                  s_SocialWeight    * r2 * (globalbest.pos(j)             - pParticle->pos(j));
            pParticle->setVel(j, vel);

            // update the position
            pParticle->setPos(j, pParticle->pos(j) + pParticle->vel(j));
        }
    }

    checkBounds(*pParticle);

    calcFitness(*pParticle);

    for(int i=0; i<m_Objective.size(); i++)  pParticle->setError(i, error(*pParticle, i));

    pParticle->updateBest();
}


/** Posted when an iteration has ended */
void MOPSOTask::postIterEvent(int iBest)
{
    OptimEvent *pIterEvent = new OptimEvent(m_Iter, iBest, m_Swarm, m_Pareto);
    qApp->postEvent(m_pParent, pIterEvent);
    qApp->processEvents();
}


/** @param particle the parameter is passed by pointer instead of reference to enable the use of QtConcurrent */
void MOPSOTask::makeRandomParticle(Particle *pParticle) const
{
    int nBest = std::min(m_Objective.size(), 3);
    pParticle->resizeArrays(m_Variable.size(), m_Objective.size(), nBest);
    double pos=0, posmin=0, posmax=0;
    double deltapos = posmax-posmin;

    for(int i=0; i<pParticle->dimension(); i++)
    {
        posmin = m_Variable.at(i).m_Min;
        posmax = m_Variable.at(i).m_Max;
        deltapos = posmax-posmin;
        pos = posmin + QRandomGenerator::global()->bounded(deltapos);
        pParticle->setPos(i, pos);
        pParticle->initializeBest();

        pParticle->setVel(i, 0);

        calcFitness(*pParticle);
    }
}

/** This default method returns the absolute deviation to the objective. */
double MOPSOTask::error(const Particle &particle, int iObjective) const
{
    double err = fabs(particle.fitness(iObjective) - m_Objective.at(iObjective).m_Target);
    return err;
}





