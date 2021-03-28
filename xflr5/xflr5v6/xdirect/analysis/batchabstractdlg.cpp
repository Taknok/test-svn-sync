/****************************************************************************

    BatchAbstractDlg Class
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

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QDir>
#include <QDateTime>
#include <QCoreApplication>
#include <QThread>
#include <QThreadPool>
#include <QTimer>
#include <QFontDatabase>
#include <QtDebug>

#include "batchabstractdlg.h"
#include "relistdlg.h"
#include "xfoiladvanceddlg.h"
#include <xflcore/xflcore.h>
#include <xflcore/gui_params.h>
#include <misc/options/settings.h>
#include <misc/text/doubleedit.h>
#include <misc/text/intedit.h>
#include <xdirect/analysis/xfoiltask.h>
#include <xdirect/objects2d.h>
#include <xdirect/xdirect.h>
#include <xflcore/xflevents.h>
#include <xflobjects/objects2d/foil.h>
#include <xflobjects/objects2d/polar.h>
#include <xinverse/foilselectiondlg.h>


bool BatchAbstractDlg::s_bCurrentFoil=true;
bool BatchAbstractDlg::s_bUpdatePolarView = false;
XDirect * BatchAbstractDlg::s_pXDirect;

int BatchAbstractDlg::s_nThreads = 1;

QByteArray BatchAbstractDlg::s_Geometry;

/**
 * The public contructor
 */
BatchAbstractDlg::BatchAbstractDlg(QWidget *pParent) : QDialog(pParent)
{
    m_pXFile = nullptr;
    m_pCurFoil = nullptr;

    m_PolarType = Xfl::FIXEDSPEEDPOLAR;

    m_FoilList.clear();

    m_Mach  = 0.0;
    m_ReMin = 100000.0;
    m_ReMax = 300000.0;
    m_ReInc =  50000.0;

    m_ClMin = 0.0;
    m_ClMax = 1.0;
    m_ClInc = 0.1;

    m_ACrit  = 9.0;
    m_XTop = 1.0;
    m_XBot = 1.0;

    m_bAlpha          = true;
    m_bFromList       = false;
    m_bFromZero       = false;
    m_bInitBL         = false;
    m_bCancel         = false;

    m_bIsRunning      = false;

    XFoil::setCancel(false);
    XFoilTask::s_bSkipOpp = false;
    XFoilTask::s_bSkipPolar = false;

    makeCommonWidgets();
}


/**
 * This course of action will lead us to destruction.
 */
BatchAbstractDlg::~BatchAbstractDlg()
{
    if(m_pXFile)     delete m_pXFile;
}


/**
 * Sets up the GUI
 */
void BatchAbstractDlg::makeCommonWidgets()
{
    m_pFoilBox = new QGroupBox(tr("Foil Selection"));
    {
        QHBoxLayout *pFoilLayout = new QHBoxLayout;
        m_prbFoil1 = new QRadioButton(tr("Current foil only"), this);
        m_prbFoil2 = new QRadioButton(tr("Foil list"), this);
        m_ppbFoilList = new QPushButton(tr("Foil list"), this);
        pFoilLayout->addWidget(m_prbFoil1);
        pFoilLayout->addWidget(m_prbFoil2);
        pFoilLayout->addStretch(1);
        pFoilLayout->addWidget(m_ppbFoilList);
        m_pFoilBox->setLayout(pFoilLayout);
    }

    m_pBatchVarsGroupBox = new QGroupBox(tr("Batch Variables"));
    {
        QGridLayout *pBatchVarsLayout = new QGridLayout;
        {
            m_rbRange1 = new QRadioButton(tr("Range"), this);
            m_rbRange2 = new QRadioButton(tr("Re List"), this);
            m_ppbEditList = new QPushButton(tr("Edit List"));
            QLabel *MinVal   = new QLabel(tr("Min"));
            QLabel *MaxVal   = new QLabel(tr("Max"));
            QLabel *DeltaVal = new QLabel(tr("Increment"));
            MinVal->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
            MaxVal->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
            DeltaVal->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);

            m_plabReType  = new QLabel("Reynolds=");
            m_plabMaType  = new QLabel("Mach=");
            QLabel *NCritLabel = new QLabel(tr("NCrit="));
            m_plabReType->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
            NCritLabel->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
            m_plabMaType->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
            m_pdeACrit   = new DoubleEdit(9.00,2, this);

            m_pdeReMin   = new DoubleEdit(100000,0,this);
            m_pdeReMax   = new DoubleEdit(150000,0,this);
            m_pdeReDelta = new DoubleEdit(50000,0, this);
            m_pdeMach    = new DoubleEdit(0.00, 3, this);

            pBatchVarsLayout->addWidget(MinVal, 2, 2);
            pBatchVarsLayout->addWidget(MaxVal, 2, 3);
            pBatchVarsLayout->addWidget(DeltaVal, 2, 4);
            pBatchVarsLayout->addWidget(m_plabReType, 3, 1);
            pBatchVarsLayout->addWidget(m_pdeReMin, 3, 2);
            pBatchVarsLayout->addWidget(m_pdeReMax, 3, 3);
            pBatchVarsLayout->addWidget(m_pdeReDelta, 3, 4);
            pBatchVarsLayout->addWidget(m_plabMaType, 4, 1);
            pBatchVarsLayout->addWidget(m_pdeMach, 4, 2);
            pBatchVarsLayout->addWidget(NCritLabel, 5,1);
            pBatchVarsLayout->addWidget(m_pdeACrit, 5, 2);
        }

        QHBoxLayout *pRangeSpecLayout = new QHBoxLayout;
        {
            pRangeSpecLayout->addWidget(m_rbRange1);
            pRangeSpecLayout->addWidget(m_rbRange2);
            pRangeSpecLayout->addStretch(1);
            pRangeSpecLayout->addWidget(m_ppbEditList);
        }

        QVBoxLayout *pBatchVarsGroupLayout = new QVBoxLayout;
        {
            pBatchVarsGroupLayout->addLayout(pRangeSpecLayout);
            pBatchVarsGroupLayout->addLayout(pBatchVarsLayout);
            m_pBatchVarsGroupBox->setLayout(pBatchVarsGroupLayout);
        }
    }

    m_pRangeVarsGroupBox = new QGroupBox(tr("Analysis Range"));
    {
        QHBoxLayout *pRangeSpecLayout = new QHBoxLayout;
        {
            QLabel *Spec = new QLabel(tr("Specify:"));
            m_prbAlpha = new QRadioButton(tr("Alpha"));
            m_prbCl = new QRadioButton(tr("Cl"));
            m_pchFromZero   = new QCheckBox(tr("From Zero"));
            pRangeSpecLayout->addWidget(Spec);
            pRangeSpecLayout->addWidget(m_prbAlpha);
            pRangeSpecLayout->addWidget(m_prbCl);
            pRangeSpecLayout->addStretch(1);
            pRangeSpecLayout->addWidget(m_pchFromZero);
        }

        QGridLayout *pRangeVarsLayout = new QGridLayout;
        {
            QLabel *SpecMin   = new QLabel(tr("Min"));
            QLabel *SpecMax   = new QLabel(tr("Max"));
            QLabel *SpecDelta = new QLabel(tr("Increment"));
            SpecMin->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
            SpecMax->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
            SpecDelta->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
            m_plabSpecVar    = new QLabel(tr("Spec ="));
            m_pdeSpecMin    = new DoubleEdit(0.00,3);
            m_pdeSpecMax    = new DoubleEdit(1.00,3);
            m_pdeSpecDelta  = new DoubleEdit(0.50,3);
            pRangeVarsLayout->addWidget(SpecMin, 1, 2);
            pRangeVarsLayout->addWidget(SpecMax, 1, 3);
            pRangeVarsLayout->addWidget(SpecDelta, 1, 4);
            pRangeVarsLayout->addWidget(m_plabSpecVar, 2, 1);
            pRangeVarsLayout->addWidget(m_pdeSpecMin, 2, 2);
            pRangeVarsLayout->addWidget(m_pdeSpecMax, 2, 3);
            pRangeVarsLayout->addWidget(m_pdeSpecDelta, 2, 4);
        }

        QVBoxLayout *pRangeVarsGroupLayout = new QVBoxLayout;
        {
            pRangeVarsGroupLayout->addLayout(pRangeSpecLayout);
            pRangeVarsGroupLayout->addLayout(pRangeVarsLayout);
            m_pRangeVarsGroupBox->setLayout(pRangeVarsGroupLayout);
        }
    }

    m_pTransVarsGroupBox = new QGroupBox(tr("Forced Transitions"));
    {
        QGridLayout *pTransVars = new QGridLayout;
        {
            pTransVars->setColumnStretch(0,4);
            pTransVars->setColumnStretch(1,1);
            QLabel *TopTransLabel = new QLabel(tr("Top transition location (x/c)"));
            QLabel *BotTransLabel = new QLabel(tr("Bottom transition location (x/c)"));
            TopTransLabel->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
            BotTransLabel->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
            m_pdeXTopTr = new DoubleEdit(1.00);
            m_pdeXBotTr = new DoubleEdit(1.00);

            pTransVars->addWidget(TopTransLabel, 2, 1);
            pTransVars->addWidget(m_pdeXTopTr, 2, 2);
            pTransVars->addWidget(BotTransLabel, 3, 1);
            pTransVars->addWidget(m_pdeXBotTr, 3, 2);
        }
        m_pTransVarsGroupBox->setLayout(pTransVars);
    }


    m_pteTextOutput = new QPlainTextEdit;
    m_pteTextOutput->setReadOnly(true);
    m_pteTextOutput->setLineWrapMode(QPlainTextEdit::NoWrap);
    m_pteTextOutput->setWordWrapMode(QTextOption::NoWrap);
    m_pteTextOutput->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_pteTextOutput->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    QFontMetrics fm(Settings::s_TableFont);
    m_pteTextOutput->setMinimumWidth(67*fm.averageCharWidth());

    m_pchInitBL          = new QCheckBox(tr("Initialize BLs between polars"));


    m_pOptionsFrame = new QFrame;
    {
        QHBoxLayout *pOptionsLayout = new QHBoxLayout;
        {
            QLabel *pLab1 = new QLabel(tr("Max. Threads to use for the analysis:"));
            int maxThreads = QThread::idealThreadCount();
            m_pieMaxThreads = new IntEdit(std::min(s_nThreads, maxThreads));
            QLabel *pLab2 = new QLabel(QString("/%1").arg(maxThreads));

            m_pchUpdatePolarView = new QCheckBox(tr("Update polar view"));
            m_pchUpdatePolarView->setToolTip(tr("Update the polar graphs after the completion of each foil/polar pair.\nUncheck for increased analysis speed."));

            pOptionsLayout->addWidget(pLab1);
            pOptionsLayout->addWidget(m_pieMaxThreads);
            pOptionsLayout->addWidget(pLab2);
            pOptionsLayout->addStretch();
            pOptionsLayout->addWidget(m_pchUpdatePolarView);
        }
        m_pOptionsFrame->setLayout(pOptionsLayout);
    }

    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
    {
        m_ppbAdvancedSettings =  new QPushButton(tr("Advanced Settings"));
        m_pButtonBox->addButton(m_ppbAdvancedSettings, QDialogButtonBox::ActionRole);

        QPushButton *ppbClearBtn = new QPushButton(tr("Clear Output"));
        connect(ppbClearBtn, SIGNAL(clicked()), m_pteTextOutput, SLOT(clear()));
        m_pButtonBox->addButton(ppbClearBtn, QDialogButtonBox::ActionRole);

        m_ppbAnalyze   = new QPushButton(tr("Analyze"));
        m_pButtonBox->addButton(m_ppbAnalyze, QDialogButtonBox::ActionRole);

        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
    }
}


void BatchAbstractDlg::connectSignals()
{
    connect(m_prbFoil1,           SIGNAL(clicked()),         this, SLOT(onFoilSelectionType()));
    connect(m_prbFoil2,           SIGNAL(clicked()),         this, SLOT(onFoilSelectionType()));
    connect(m_ppbFoilList,        SIGNAL(clicked()),         this, SLOT(onFoilList()));
    connect(m_prbAlpha,           SIGNAL(toggled(bool)),     this, SLOT(onAcl()));
    connect(m_prbCl,              SIGNAL(toggled(bool)),     this, SLOT(onAcl()));
    connect(m_rbRange1,             SIGNAL(toggled(bool)),     this, SLOT(onRange()));
    connect(m_ppbEditList,        SIGNAL(clicked()),         this, SLOT(onEditReList()));
    connect(m_pchFromZero,        SIGNAL(stateChanged(int)), this, SLOT(onFromZero(int)));
    connect(m_pdeSpecMin,         SIGNAL(editingFinished()), this, SLOT(onSpecChanged()));
    connect(m_pdeSpecMax,         SIGNAL(editingFinished()), this, SLOT(onSpecChanged()));
    connect(m_pdeSpecDelta,       SIGNAL(editingFinished()), this, SLOT(onSpecChanged()));
    connect(m_pchUpdatePolarView, SIGNAL(clicked(bool)),     this, SLOT(onUpdatePolarView()));
}


void BatchAbstractDlg::onButton(QAbstractButton *pButton)
{
    if      (pButton == m_pButtonBox->button(QDialogButtonBox::Close)) onClose();
    else if (pButton == m_ppbAnalyze)                                  onAnalyze();
    else if (pButton == m_ppbAdvancedSettings)                         onAdvancedSettings();
}


/**
 * Clean-up is performed when all the threads have finished
 */
void BatchAbstractDlg::cleanUp()
{
    if(m_pXFile->isOpen())
    {
        QTextStream out(m_pXFile);
        out<<m_pteTextOutput->toPlainText();
        m_pXFile->close();
    }
    m_pButtonBox->button(QDialogButtonBox::Close)->setEnabled(true);
    m_ppbAnalyze->setText(tr("Analyze"));
    m_bIsRunning = false;
    m_bCancel    = false;
    XFoil::setCancel(false);
    m_pButtonBox->button(QDialogButtonBox::Close)->setFocus();
}


/**
 * Overrides the base class keyPressEvent method
 * @param event the keyPressEvent.
 */
void BatchAbstractDlg::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(m_pButtonBox->button(QDialogButtonBox::Close)->hasFocus())   done(1);
            else if(m_ppbAnalyze->hasFocus())  onAnalyze();
            else                               m_ppbAnalyze->setFocus();
            break;
        }
        case Qt::Key_Escape:
        {
            if(m_bIsRunning)
            {
                m_bCancel = true;
                XFoilTask::s_bCancel = true;
                XFoil::setCancel(true);
            }
            else
            {
                onClose(); // will close the dialog box
            }
            break;
        }
        default:
            pEvent->ignore();
    }
    pEvent->accept();
}


/**
 * Initializes the dialog and the GUI interface
 */
void BatchAbstractDlg::initDialog()
{
    if(!XDirect::curFoil()) return;
    blockSignals(true);

    m_pteTextOutput->clear();
    m_pteTextOutput->setFont(Settings::s_TableFont);

    m_ACrit     = XDirect::s_RefPolar.NCrit();
    m_XBot      = XDirect::s_RefPolar.XtrBot();
    m_XTop      = XDirect::s_RefPolar.XtrTop();
    m_Mach      = XDirect::s_RefPolar.Mach();

    m_PolarType = Xfl::FIXEDSPEEDPOLAR; //no choice...

    m_prbFoil1->setChecked(s_bCurrentFoil);
    m_prbFoil2->setChecked(!s_bCurrentFoil);
    onFoilSelectionType();

    m_pdeReMin->setDigits(0);
    m_pdeReMax->setDigits(0);
    m_pdeReDelta->setDigits(0);

    m_pdeSpecMin->setDigits(3);
    m_pdeSpecMax->setDigits(3);
    m_pdeSpecDelta->setDigits(3);

    if(m_ReMin<=0.0) m_ReMin = qAbs(m_ReInc);
    m_pdeReMin->setValue(m_ReMin);
    m_pdeReMax->setValue(m_ReMax);
    m_pdeReDelta->setValue(m_ReInc);
    m_pdeSpecMin->setValue(m_AlphaMin);
    m_pdeSpecMax->setValue(m_AlphaMax);
    m_pdeSpecDelta->setValue(m_AlphaInc);

    m_pdeMach->setValue(m_Mach);
    m_pdeACrit->setValue(m_ACrit);
    m_pdeXTopTr->setValue(m_XTop);
    m_pdeXBotTr->setValue(m_XBot);

    if(m_bAlpha) m_prbAlpha->setChecked(true);
    else         m_prbCl->setChecked(m_bAlpha);
    onAcl();


    if(!m_bFromList)  m_rbRange1->setChecked(true);
    else              m_rbRange2->setChecked(true);

    m_ppbEditList->setEnabled(m_bFromList);
    m_pdeReMin->setEnabled(!m_bFromList);
    m_pdeReMax->setEnabled(!m_bFromList);
    m_pdeReDelta->setEnabled(!m_bFromList);
    m_pdeMach->setEnabled(!m_bFromList);
    m_pdeACrit->setEnabled(!m_bFromList);

    if(m_bFromZero)  m_pchFromZero->setChecked(true);
    else             m_pchFromZero->setChecked(false);

    m_pchInitBL->setChecked(true);
    m_pchUpdatePolarView->setChecked(s_bUpdatePolarView);
    blockSignals(false);
}


/**
 * The user has switched between aoa and lift coeficient.
 * Initializes the interface with the corresponding values.
 */
void BatchAbstractDlg::onAcl()
{
    if(m_PolarType==Xfl::FIXEDAOAPOLAR) return;
    m_bAlpha = m_prbAlpha->isChecked();
    if(m_bAlpha)
    {
        m_plabSpecVar->setText(tr("Alpha"));
        m_pdeSpecMin->setValue(m_AlphaMin);
        m_pdeSpecMax->setValue(m_AlphaMax);
        m_pdeSpecDelta->setValue(m_AlphaInc);
        m_pchFromZero->setEnabled(true);
    }
    else
    {
        m_plabSpecVar->setText(tr("CL"));
        m_pdeSpecMin->setValue(m_ClMin);
        m_pdeSpecMax->setValue(m_ClMax);
        m_pdeSpecDelta->setValue(m_ClInc);
        m_bFromZero = false;
        m_pchFromZero->setChecked(false);
        m_pchFromZero->setEnabled(false);
    }
}

/**
 * The user has changed the range of Re values to analyze
 */
void BatchAbstractDlg::onSpecChanged()
{
    readParams();
}


/**
 * The user has requested to quit the dialog box
 */
void BatchAbstractDlg::onClose()
{
    if(m_bIsRunning) return;

    m_bCancel = true;
    XFoilTask::s_bCancel = true;
    QThreadPool::globalInstance()->waitForDone();
    readParams();

    XDirect::s_RefPolar.setNCrit(m_ACrit);
    XDirect::s_RefPolar.setXtrBot(m_XBot);
    XDirect::s_RefPolar.setXtrTop(m_XTop);
    XDirect::s_RefPolar.setMach(m_Mach);

    accept();
}


/**
 * Overrides the base class reject() method, to prevent window closure when an analysis is running.
 * If the analysis is running, cancels it and returns.
 * If not, closes the window.
 */
void BatchAbstractDlg::reject()
{
    if(m_bIsRunning)
    {
        m_bCancel    = true;
        XFoil::setCancel(true);
    }
    else
    {
        QDialog::reject();
        //close the dialog box
    }
}


/**
 * The user has requested an edition of the Re list
*/
void BatchAbstractDlg::onEditReList()
{
    ReListDlg dlg(this);
    dlg.initDialog(XDirect::s_ReList,XDirect::s_MachList, XDirect::s_NCritList);

    if(QDialog::Accepted == dlg.exec())
    {
        XDirect::s_ReList.clear();
        XDirect::s_MachList.clear();
        XDirect::s_NCritList.clear();

        XDirect::s_ReList.append(dlg.ReList());
        XDirect::s_MachList.append(dlg.MachList());
        XDirect::s_NCritList.append(dlg.NCritList());
    }
}


/**
 * The user has requested an edition of the list of Foil objects to analyze
 */
void BatchAbstractDlg::onFoilList()
{
    FoilSelectionDlg dlg(this);
    //    dlg.SetSelectionMode(true);

    dlg.initDialog(Objects2d::pOAFoil(), m_FoilList);

    m_FoilList.clear();

    if(QDialog::Accepted == dlg.exec())
    {
        m_FoilList.append(dlg.foilSelectionList());
    }
    outputFoilList();
}


/**
 *The user has changed between single Foil and list of Foil objects to analyze
 */
void BatchAbstractDlg::onFoilSelectionType()
{
    s_bCurrentFoil = m_prbFoil1->isChecked();
    m_ppbFoilList->setEnabled(!s_bCurrentFoil);

    if(s_bCurrentFoil)
    {
        m_FoilList.clear();
        m_FoilList.append(XDirect::curFoil()->name());
    }

    outputFoilList();
}

/**
 * The user has clicked the "start from zero" checkbox
 * @param state the new state of the checkbox
 */
void BatchAbstractDlg::onFromZero(int )
{
    if(m_pchFromZero->isChecked()) m_bFromZero = true;
    else                           m_bFromZero = false;
}


/**
 * The user has clicked the checkbox which specifies the initialization of the boundary layer
 **/
void BatchAbstractDlg::onInitBL(int)
{
    if (m_pchInitBL->isChecked()) m_bInitBL = true;
    else                          m_bInitBL = false;
}

/**
 * The user has clicked the checkbox specifying which range of Re should be analyzed
 */
void BatchAbstractDlg::onRange()
{
    if(m_rbRange1->isChecked())
        m_bFromList = false;
    else
        m_bFromList = true;

    m_ppbEditList->setEnabled(m_bFromList);
    m_pdeReMin->setEnabled(!m_bFromList);
    m_pdeReMax->setEnabled(!m_bFromList);
    m_pdeReDelta->setEnabled(!m_bFromList);
    m_pdeMach->setEnabled(!m_bFromList);
    m_pdeACrit->setEnabled(!m_bFromList);
}


/**
 * Reads the value of the input parameters from the widgets and maps the data
 */
void BatchAbstractDlg::readParams()
{
    m_bAlpha = m_prbAlpha->isChecked();

    if(m_PolarType!=Xfl::FIXEDAOAPOLAR)
    {
        m_ReInc = m_pdeReDelta->value();
        m_ReMax = m_pdeReMax->value();
        m_ReMin = m_pdeReMin->value();

        if(m_ReMin>m_ReMax)
        {
            double tmp = m_ReMin;
            m_ReMin = m_ReMax;
            m_ReMax = tmp;
        }

        if(m_bAlpha)
        {
            m_AlphaInc = qAbs(m_pdeSpecDelta->value());
            m_AlphaMax = m_pdeSpecMax->value();
            m_AlphaMin = m_pdeSpecMin->value();
        }
        else
        {
            m_ClInc = qAbs(m_pdeSpecDelta->value());
            m_ClMin = m_pdeSpecMin->value();
            m_ClMax = m_pdeSpecMax->value();
        }
    }

    if(m_ReMin<=0.0) m_ReMin = qAbs(m_ReInc);
    if(m_ReMax<=0.0) m_ReMax = qAbs(m_ReMax);

    m_Mach     = m_pdeMach->value();
    if(m_Mach<=0.0) m_Mach = 0.0;
    m_ACrit    = m_pdeACrit->value();
    m_XTop   = m_pdeXTopTr->value();
    m_XBot   = m_pdeXBotTr->value();

    s_nThreads = m_pieMaxThreads->value();
    s_nThreads = std::min(s_nThreads, QThread::idealThreadCount());
    m_pieMaxThreads->setValue(s_nThreads);
}


/**
 * Initializes the header of the log file
 */
void BatchAbstractDlg::setFileHeader()
{
    if(!m_pXFile) return;
    QTextStream out(m_pXFile);

    out << "\n";
    out << VERSIONNAME;
    out << "\n";
    if(s_bCurrentFoil)
    {
        out << XDirect::curFoil()->name();
        out << "\n";
    }


    QDateTime dt = QDateTime::currentDateTime();
    QString str = dt.toString("dd.MM.yyyy  hh:mm:ss");

    out << str;
    out << "\n___________________________________\n\n";
}


/**
 * Creates the polar name for the input Polar
 * @param pNewPolar a pointer to the Polar object to name
 */
void BatchAbstractDlg::setPlrName(Polar *pNewPolar)
{
    if(!pNewPolar) return;
    pNewPolar->setAutoPolarName();
}


/**
 * Adds a text message to the log file
 * @param str the message to output
 */
void BatchAbstractDlg::writeString(QString &strong)
{
    if(!m_pXFile || !m_pXFile->isOpen()) return;
    QTextStream ds(m_pXFile);
    ds << strong;
}




/**
 * Outputs the list of the Foil names selected for analysis to the output text window.
 */
void BatchAbstractDlg::outputFoilList()
{
    m_pteTextOutput->appendPlainText(tr("Foils to analyze:")+"\n");
    for(int i=0; i<m_FoilList.count();i++)
    {
        m_pteTextOutput->appendPlainText("   "+m_FoilList.at(i)+"\n");
    }
    m_pteTextOutput->appendPlainText("\n");
}


/**
 * Outputs the list of the Re values selected for analysis to the output text window.
 */
void BatchAbstractDlg::outputReList()
{
    m_pteTextOutput->appendPlainText(tr("Reynolds numbers to analyze:")+"\n");
    if(m_bFromList)
    {
        for(int i=0; i<XDirect::s_ReList.count(); i++)
        {
            QString strong = QString("   Re = %L1  /  Mach = %L2  /  NCrit = %L3")
                    .arg(XDirect::s_ReList.at(i), 10,'f',0)
                    .arg(XDirect::s_MachList.at(i), 5,'f',3)
                    .arg(XDirect::s_NCritList.at(i), 5, 'f', 2);
            m_pteTextOutput->appendPlainText(strong+"\n");
        }
    }
    else
    {
        for(double Re=m_ReMin; Re<m_ReMax; Re+=m_ReInc)
        {
            QString strong = QString("   Re = %L1  /  Mach = %L2  /  NCrit = %L3")
                    .arg(Re, 10,'f',0)
                    .arg(m_Mach, 5,'f',3)
                    .arg(m_ACrit, 5, 'f', 2);
            m_pteTextOutput->appendPlainText(strong+"\n");
        }
    }
    m_pteTextOutput->appendPlainText("\n");
}


/**
 * Overrides the base class showEvent method. Moves the window to its former location.
 * @param event the showEvent.
 */
void BatchAbstractDlg::showEvent(QShowEvent *)
{
    restoreGeometry(s_Geometry);
}


/**
 * Overrides the base class hideEvent method. Stores the window's current position.
 * @param event the hideEvent.
 */
void BatchAbstractDlg::hideEvent(QHideEvent *)
{
    s_Geometry = saveGeometry();
}


void BatchAbstractDlg::onAdvancedSettings()
{
    XFoilAdvancedDlg xfaDlg(this);
    xfaDlg.m_IterLimit   = XFoilTask::s_IterLim;
    xfaDlg.m_bAutoInitBL = XFoilTask::s_bAutoInitBL;
    xfaDlg.m_VAccel      = XFoil::VAccel();
    xfaDlg.m_bFullReport = XFoil::fullReport();
    xfaDlg.initDialog();

    if (QDialog::Accepted == xfaDlg.exec())
    {
        XFoil::setVAccel(xfaDlg.m_VAccel);
        XFoil::setFullReport(xfaDlg.m_bFullReport);
        XFoilTask::s_bAutoInitBL  = xfaDlg.m_bAutoInitBL;
        XFoilTask::s_IterLim      = xfaDlg.m_IterLimit;
    }
}

void BatchAbstractDlg::onUpdatePolarView()
{
    s_bUpdatePolarView = m_pchUpdatePolarView->isChecked();
    s_pXDirect->updateView();
}






