/****************************************************************************

    BatchThreadDlg Class
    Copyright (C) 2003-2016 Andre Deperrois

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

#include <QDialog>
#include <QCheckBox>
#include <QFile>
#include <QPushButton>
#include <QLabel>
#include <QRadioButton>
#include <QPlainTextEdit>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <xflcore/core_enums.h>

class Foil;
class Polar;
class IntEdit;
class DoubleEdit;
class XFoilTask;
class XFoilTaskEvent;
struct FoilAnalysis;
class XDirect;

/**
 * @brief Abstract base class for BatchThreadDlg and BacthCtrlDlg
 */
class BatchAbstractDlg : public QDialog
{
    Q_OBJECT
    friend class XDirect;
    friend class MainFrame;
    friend class XFoilTask;

    public:
        BatchAbstractDlg(QWidget *pParent=nullptr);
        ~BatchAbstractDlg();

        virtual void initDialog();
        QSize sizeHint() const override {return QSize(1100,900);}

    protected:
        void keyPressEvent(QKeyEvent  *pEvent) override;
        void showEvent(    QShowEvent *pEvent) override;
        void hideEvent(    QHideEvent *pEvent) override;
        void reject() override;

        virtual void cleanUp();
        virtual void connectSignals();
        virtual void readParams();
        virtual void setupLayout() = 0;

        void makeCommonWidgets();
        void outputFoilList();
        void outputReList();
        void setFileHeader();
        void setPlrName(Polar *pNewPolar);
        void writeString(QString &strong);

    protected slots:
        virtual void onAnalyze() = 0;
        void onAcl();
        void onAdvancedSettings();
        void onButton(QAbstractButton *pButton);
        void onClose();
        void onEditReList();
        void onFoilList();
        void onFoilSelectionType();
        void onFromZero(int);
        void onInitBL(int);
        void onRange();
        void onSpecChanged();
        void onUpdatePolarView();


    protected:
        QRadioButton *m_prbFoil1, *m_prbFoil2;
        QPushButton *m_ppbFoilList;
        QRadioButton *m_rbRange1, *m_rbRange2;
        QRadioButton *m_prbAlpha, *m_prbCl;
        QPushButton *m_ppbEditList;
        DoubleEdit *m_pdeReMin, *m_pdeReMax, *m_pdeReDelta, *m_pdeMach;
        DoubleEdit *m_pdeSpecMin, *m_pdeSpecMax, *m_pdeSpecDelta;
        DoubleEdit *m_pdeACrit, *m_pdeXTopTr, *m_pdeXBotTr;
        IntEdit *m_pieMaxThreads;
        QLabel *m_plabSpecVar;
        QLabel *m_plabMaType, *m_plabReType;
        QCheckBox *m_pchInitBL, *m_pchFromZero, *m_pchUpdatePolarView;

        QGroupBox *m_pFoilBox,*m_pBatchVarsGroupBox, *m_pRangeVarsGroupBox, *m_pTransVarsGroupBox;
        QFrame *m_pOptionsFrame;

        QDialogButtonBox *m_pButtonBox;
        QPushButton *m_ppbAnalyze, *m_ppbAdvancedSettings;
        QPlainTextEdit *m_pteTextOutput;

        double m_ReMin;             /**< the min Re for a range analysis */
        double m_ReMax;             /**< the max Re for a range analysis */
        double m_ReInc;             /**< the incement Re for a range analysis */
        double m_AlphaMin;          /**< The starting aoa */
        double m_AlphaMax;          /**< The ending aoa */
        double m_AlphaInc;          /**< The aoa increment */
        double m_ClMin;             /**< The starting Cl coefficient  */
        double m_ClMax;             /**< The ending Cl coefficient */
        double m_ClInc;             /**< The Cl increment  */

        //    bool m_bOutput;             /**< true if the output should be displayed in the text widget */
        bool m_bAlpha;              /**< true if the analysis should be performed for a range of aoa rather than lift coefficient */
        bool m_bFromList;           /**< true if the analysis should be performed for a list of Re values rather than for a range */
        bool m_bFromZero;           /**< true if the iterations should start from aoa=0 rather than aoa=alpha_min */
        bool m_bInitBL;             /**< true if the boundary layer should be restored to the default value before each polar analysis */
        bool m_bCancel;             /**< true if the user has clicked the cancel button */
        bool m_bIsRunning;          /**< true until all the pairs of (foil, polar) have been calculated */

        Xfl::enumPolarType m_PolarType;  /**< the type of analysis to perform */

        double m_Mach;              /**< the Mach number used if not from the list of Re numbers */
        double m_ACrit;             /**< the transition criterion used if not from the list of Re numbers */

        double m_XTop;            /**< the point of forced transition on the upper surface */
        double m_XBot;            /**< the point of forced transition on the lower surface */


        QFile *m_pXFile;                   /**< a pointer to the output log file */

        Foil *m_pCurFoil;                  /**< a pointer to the current Foil */

        QStringList m_FoilList;            /**< the list of foils to analyze */

        static QByteArray s_Geometry;
        static XDirect* s_pXDirect;           /**< a void pointer to the unique instance of the QXDirect class */
        static bool s_bCurrentFoil;        /**< true if the analysis should be performed only for the current foil */
        static bool s_bUpdatePolarView;    /**< true if the polar graphs should be updated during the analysis */
        static int s_nThreads;             /**< the number of available threads */
};



