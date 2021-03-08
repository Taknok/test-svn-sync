/****************************************************************************

    Optim2d Class
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


#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPlainTextEdit>
#include <QRadioButton>
#include <QSplitter>
#include <QStackedWidget>
#include <QTabWidget>
#include <QTimer>

#include <xflobjects/objects2d/foil.h>
#include <xdirect/optim2d/particle.h>
#include <graph/graph.h>

class Foil;
class FoilWt;
class GraphWt;
class DoubleEdit;
class IntEdit;
class XFoilTask;

class Optim2d : public QDialog
{
    Q_OBJECT

    public:
        Optim2d(QWidget *pParent);
        ~Optim2d();

        void setFoil(const Foil *pFoil);

        bool isModified() const {return m_bModified;}

        QSize sizeHint() const override {return QSize(1300, 900);}

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:
        void reject() override;
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

        void setupLayout();
        void connectSignals();

        void readData();
        void outputText(QString const &msg) const;

        // common
        void makeRandomParticle(Particle *pParticle) const;
        void listPopulation() const;

        //PSO
        void PSO_moveParticle(Particle *pParticle) const;
        double Cl_error(double Cl) const;

        //GA
        void GA_crossOver();
        void GA_evaluatePopulation();
        void GA_evaluateParticle(Particle *pParticle) const;
        void GA_makeNewGen();
        void GA_makeSelection();
        void GA_mutateGaussian();

        //foil
        void makeFoil(Particle const *pParticle, Foil *pFoil) const;
        double foilFunc(Particle const *pParticle) const;
        double HH(double x, double t1, double t2) const;
        bool runXFoilTask(XFoilTask &task);
        void checkBounds(Particle *pParticle) const;

    private slots:
        void onButton(QAbstractButton *pButton);
        void onClose();

        void onAlgorithm();
        void onAnalyze();
        void onOptimize();
        void onMakeSwarm(bool bShow=true);
        void onIteration();
        void onStoreBestFoil();

    private:
        bool m_bSaved;
        bool m_bModified;

        Foil const *m_pFoil;

        Foil *m_pBestFoil; // used to animate the display

        QVector<Foil*> m_TempFoils; /**< pointers to debug foils to delete on exit */

        //Algo
        double m_Error;
        int m_Iter;
        int m_iBest;

        QVector<double> m_BestPosition; // population best

        int m_iLE;  /**< the index of the leading edge point for thee current aoa */

        QVector<Particle> m_Swarm; // the swarm or the population in the case of the GA

        //XFoil
        static double s_Alpha;
        static double s_Re;
        static double s_NCrit;
        static double s_XtrTop;
        static double s_XtrBot;

        //Target
        static double s_Cl;
        static double s_MaxError;

        static double s_HHt2;     /**< t2 parameter of the HH functions */
        static int    s_HHn;      /**< number of HH functions to use */
        static double s_HHmax;    /**< the max amplitude of the HH functions */

        //Common
        static bool   s_bPSO;
        static int    s_PopSize;
        static int    s_MaxIter;
        static int    s_Dt;
        static bool   s_bMultiThreaded;

        //PSO
        static double s_InertiaWeight;
        static double s_CognitiveWeight;
        static double s_SocialWeight;

        //GA
        static double s_ProbXOver;       /** probability of crossover */
        static double s_ProbMutation;    /** probability of mutation */
        static double s_SigmaMutation;   /** standard deviation of the gaussian mutation */

        // interface
        QTabWidget *m_ptwMain;
        Graph m_Graph;
        GraphWt *m_pGraphWt;
        FoilWt *m_pFoilWt;

        // XFoil
        DoubleEdit *m_pdeAlpha, *m_pdeRe, *m_pdeNCrit, *m_pdeXtrTop, *m_pdeXtrBot;
        QPushButton *m_ppbAnalyze;

        // Optim
        DoubleEdit *m_pdeCl, *m_pdeMaxError;

        // Hicks-Henne
        IntEdit *m_pieNHH;
        DoubleEdit *m_pdeHHt2, *m_pdeHHmax;

        //PSO
        DoubleEdit *m_pdeInertiaWeight;
        DoubleEdit *m_pdeCognitiveWeight;
        DoubleEdit *m_pdeSocialWeight;

        //GA
        DoubleEdit *m_pdeProbXOver, *m_pdeProbMutation, *m_pdeSigmaMutation;

        //Common
        QStackedWidget *m_pswAlgo;
        IntEdit *m_piePopSize;
        IntEdit *m_pieMaxIter, *m_pieUpdateDt;
        QRadioButton *m_prbPSO, *m_prbGA;
        QCheckBox *m_pchMultithread;

        QPushButton * m_ppbSwarm, *m_ppbMakeSwarm, *m_ppbStoreBestFoil;

        QSplitter *m_pHSplitter, *m_pVSplitter;
        QPlainTextEdit *m_ppt;

        QTimer m_Timer;

        QDialogButtonBox *m_pButtonBox;

        static QByteArray s_Geometry;
        static QByteArray s_HSplitterSizes, s_VSplitterSizes;
};


