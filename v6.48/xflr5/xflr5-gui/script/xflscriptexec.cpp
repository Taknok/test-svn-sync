/****************************************************************************

    xflScriptExec Class
    Copyright (C) 2016-2016 Andre Deperrois

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

#include <iostream>

#include <QCoreApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QThreadPool>
#include <QDebug>

#include "xflscriptexec.h"
#include <analysis3d/plane_analysis/planeanalysistask.h>
#include <globals/globals.h>
#include <globals/gui_params.h>
#include <miarex/mgt/xmlplanereader.h>
#include <miarex/mgt/xmlwpolarreader.h>
#include <misc/options/settings.h>
#include <xdirect/objects2d.h>
#include <xdirect/analysis/xfoiltask.h>
#include <xdirect/analysis/xfoiltaskevent.h>
#include <xdirect/xml/xmlpolarreader.h>

QString XflScriptExec::s_VersionName;


XflScriptExec::XflScriptExec(MainFrame *pMainFrame)
{
    m_pXFile = nullptr;
    m_pMainFrame = pMainFrame;

    m_bStdOutStream = false;
    m_bCancel = false;

    m_nTaskStarted = m_nTaskDone =0;
    m_nThreads = 1;
}


XflScriptExec::~XflScriptExec()
{
    if(m_pXFile)
    {
        m_pXFile->close();
        delete m_pXFile;
    }

    for(int ix=0; ix<m_FoilExecList.size(); ix++)
        delete m_FoilExecList.at(ix);
}


void XflScriptExec::makeFoilAnalysisList()
{
    m_FoilExecList.clear();

    QStringList filters = {"*.xml"};
    QStringList xmlanalyseslist = m_ScriptReader.m_PolarList;
    if(m_ScriptReader.bRunAllAnalyses())
    {
        findFiles(m_ScriptReader.xmlAnalysisDirPath(), filters, false, xmlanalyseslist);
    }

    for(int ip=0; ip<m_oaFoil.count(); ip++)
    {
        Foil *pFoil = m_oaFoil.at(ip);
        if(pFoil)
        {
            for(int ip=0; ip<xmlanalyseslist.count(); ip++)
            {
                Polar *pPolar = makePolar(xmlanalyseslist.at(ip));
                if(pPolar)
                {
                    pPolar->setFoilName(pFoil->foilName());
                    pPolar->setPolarStyle(pFoil->foilLineStyle());
                    pPolar->setPolarWidth(pFoil->foilLineWidth());
                    pPolar->setColor(pFoil->foilColor().red(), pFoil->foilColor().green(), pFoil->foilColor().blue());
                    FoilAnalysis *pFoilAnalysis = new FoilAnalysis;
                    pFoilAnalysis->pFoil = pFoil;
                    pFoilAnalysis->pPolar = pPolar;
                    switch(pPolar->polarType())
                    {
                        case XFLR5::FIXEDSPEEDPOLAR:
                        case XFLR5::FIXEDLIFTPOLAR:
                        {
                            pFoilAnalysis->vMin = m_ScriptReader.m_aoaMin;
                            pFoilAnalysis->vMax = m_ScriptReader.m_aoaMax;
                            pFoilAnalysis->vInc = m_ScriptReader.m_aoaInc;
                            break;
                        }
                        case XFLR5::FIXEDAOAPOLAR:
                        {
                            pFoilAnalysis->vMin = m_ScriptReader.m_ReMin;
                            pFoilAnalysis->vMax = m_ScriptReader.m_ReMax;
                            pFoilAnalysis->vInc = m_ScriptReader.m_ReInc;
                            break;
                        }
                        default:
                        {
                            break;
                        }
                    }
                    m_FoilExecList.append(pFoilAnalysis);
                    traceLog("   added analysis for foil "+pFoil->foilName()+" and "+pPolar->polarName() +"\n");
                }
            }
        }
    }
}


bool XflScriptExec::makeFoils()
{
    traceLog("Reading foil files\n");
    QStringList filters =  {"*.dat"};
    for(int ifo=0; ifo<m_ScriptReader.m_FoilList.count(); ifo++)
    {
        QFile datFile;
        QString datPathName;

        if (!findFile(m_ScriptReader.m_FoilList.at(ifo), m_ScriptReader.datFoilDirPath(), filters, true, datPathName))
        {
            QString strange = "   ...failed to find the file "+m_ScriptReader.m_FoilList.at(ifo);
            traceLog(strange+"\n");
            return false;
        }

        datFile.setFileName(datPathName);
        if (!datFile.open(QIODevice::ReadOnly))
        {
            QString strange = "   ...Could not open the file "+m_ScriptReader.m_FoilList.at(ifo);
            traceLog(strange+"\n");
            return false;
        }
        else
        {
            Foil *pFoil = readFoilFile(datFile);
            if(pFoil)
            {
                pFoil->setLineWidth(2);

                Objects2d::insertThisFoil(pFoil);
                m_oaFoil.append(pFoil);
                traceLog("   added foil: "+pFoil->foilName()+"\n");
            }
            else
            {
                traceLog("   ...failed to add the foil from "+m_ScriptReader.m_FoilList.at(ifo)+"\n");
                return false;
            }
        }
    }
    traceLog("\n");

    return true;
}


Polar* XflScriptExec::makePolar(QString fileName)
{
    QString pathName = m_ScriptReader.m_PolarBinDirPath+QDir::separator()+fileName;
    QFile xmlFile(pathName);
    if (!xmlFile.open(QIODevice::ReadOnly))
    {
        QString strange = "Could not read the file"+pathName;
        traceLog(strange);
        return nullptr;
    }
    else
    {
        Polar *pPolar = new Polar;
        XmlPolarReader xpReader(xmlFile, pPolar);
        xpReader.readXMLPolarFile();
        Objects2d::appendPolar(pPolar);
        return pPolar;
    }
}


bool XflScriptExec::setLogFile()
{
    m_LogFileName = m_OutputPath + "/script_"+QTime::currentTime().toString("hhmmss")+".log";

    m_pXFile = new QFile(m_LogFileName);
    if(!m_pXFile->open(QIODevice::WriteOnly | QIODevice::Text))
    {
        traceLog("Could not create the log file... aborting\n");
        return false;
    }
    m_OutLogStream.setDevice(m_pXFile);
    m_OutLogStream << "\n";
    m_OutLogStream << s_VersionName;
    m_OutLogStream << "\n";
    QDateTime dt = QDateTime::currentDateTime();
    QString str = dt.toString("dd.MM.yyyy  hh:mm:ss");
    m_OutLogStream << str<<"\n";

    m_OutLogStream.flush();
    return true;
}


void XflScriptExec::traceLog(QString strMsg)
{
    if(m_bStdOutStream)
    {
        std::cout<<strMsg.toStdString();
    }

    if(m_OutLogStream.device())
    {
        m_OutLogStream << strMsg;
        m_OutLogStream.flush();
    }
    emit msgUpdate(strMsg);
    qApp->processEvents();
}


bool XflScriptExec::readScript(QString scriptpathname)
{
    QFile xmlFile(scriptpathname);
    if (!xmlFile.open(QIODevice::ReadOnly))
    {
        QString strange = "Could not read the file"+scriptpathname;
        traceLog(strange);
        return false;
    }

    m_ScriptReader.setDevice(&xmlFile);
    m_ScriptReader.readScript();
    if(m_ScriptReader.hasError())
    {
        QString strange;
        strange = QString::asprintf("\nline %d column %d", int(m_ScriptReader.lineNumber()), int(m_ScriptReader.columnNumber()));
        QString errorMsg = m_ScriptReader.errorString() + strange;
        traceLog(errorMsg);
    }
    else
    {
    }
    return true;
}


QString XflScriptExec::projectFilePathName() const
{
    QString projectFileName = m_ScriptReader.projectFileName();
    QFileInfo fi(projectFileName);

    return m_OutputPath + QDir::separator() + fi.fileName();
}


void XflScriptExec::closeLogFile()
{
    m_OutLogStream.setDevice(nullptr);
    if(m_pXFile)
    {
        m_pXFile->close(); delete m_pXFile; m_pXFile=nullptr;
    }
}


void XflScriptExec::onCancel()
{
    m_bCancel=true;
    XFoilTask::cancelTask();

    traceLog("\n_____________Analysis cancellation request received_____________\n\n");
    emit (cancelTask());
}


bool XflScriptExec::makeExportDirectories()
{
    bool bOK = true;
    QString projectFileName = m_ScriptReader.projectFileName();
    QFileInfo fi(projectFileName);

    m_OutputPath = m_ScriptReader.outputDirPath();
    m_OutputPath += QDir::separator() + fi.baseName();


    QDir outputDir(m_OutputPath);
    if(!outputDir.exists())
    {
        if(!outputDir.mkpath(m_OutputPath))
        {
            traceLog("Could not make the directory: "+m_OutputPath+"\n");
            bOK = false;
        }
    }

    m_FoilPolarsTextPath = m_OutputPath+QDir::separator()+"Foil_polars";
    QDir exportFoilPolarsDir(m_FoilPolarsTextPath);
    if(!exportFoilPolarsDir.exists())
    {
        if(!exportFoilPolarsDir.mkpath(m_FoilPolarsTextPath))
        {
            traceLog("Could not make the directory: "+m_FoilPolarsTextPath+"\n");
            bOK = false;
        }
    }

    if(m_ScriptReader.binPolarDirPath().length()) m_FoilPolarsBinPath = m_ScriptReader.binPolarDirPath();
    else                                          m_FoilPolarsBinPath = m_FoilPolarsTextPath;

    return bOK;
}


bool XflScriptExec::runScript()
{
    XFoilTask::setCancelled(false);
    QString strange;

    if(!makeExportDirectories())
    {
        traceLog("Error making directories ...aborting\n");
        return false;
    }

    if(!setLogFile()) return false;

    if(m_bCancel) return false;

    makeFoils();
    if(m_bCancel) return false;

    makeFoilAnalysisList();
    if(m_bCancel) return false;

    if(m_FoilExecList.size())
    {
        runFoilAnalyses();
    }
    else traceLog("No Foil analysis requested\n\n");


    traceLog("Finished script successfully\n");
    return true;
}


/**
 * Clean-up is performed when all the threads are terminated
 */
void XflScriptExec::cleanUpFoilAnalyses()
{
    XFoil::s_bCancel = false;

    //in case we cancelled, delete all Analysis that are left
    for(int ia=m_FoilExecList.count()-1; ia>=0; ia--)
    {
        FoilAnalysis *pAnalysis = m_FoilExecList.at(ia);
        delete pAnalysis;
        m_FoilExecList.removeAt(ia);
    }
    QThreadPool::globalInstance()->setMaxThreadCount(QThread::idealThreadCount());
}


void XflScriptExec::runFoilAnalyses()
{
    QString strong;

    //	QThreadPool::globalInstance()->setExpiryTimeout(60000);//ms
    traceLog("\n\n");

    strong = "_____Starting foil analysis_____\n\n";
    traceLog(strong);

    m_nThreads = m_ScriptReader.m_nMaxThreads;
    QThreadPool::globalInstance()->setMaxThreadCount(m_nThreads);
    strong = QString::asprintf("Running with %d thread(s)\n", m_ScriptReader.m_nMaxThreads);
    traceLog(strong+"\n");

    //build an array of all analysis pairs to run
    m_nTaskDone = 0;
    m_nTaskStarted = 0;

    strong = QString::asprintf("Found %d (foil, polar) pairs to analyze.\n",m_FoilExecList.size());
    traceLog(strong+"\n");

    XFoilTask::s_bCancel = false;

    while(m_FoilExecList.size()>0 && !m_bCancel)
    {
        if (QThreadPool::globalInstance()->activeThreadCount()<QThreadPool::globalInstance()->maxThreadCount())
        {
            startXFoilTaskThread(); // analyze a new pair
        }

        QThread::msleep(100);
    }


    QThreadPool::globalInstance()->waitForDone();


    cleanUpFoilAnalyses();
    if(m_bCancel) strong = "\n_____Foil analysis cancelled_____\n";
    else          strong = "\n_____Foil analysis completed_____\n";
    traceLog(strong);
}


/**
 * Starts an individual thread
 */
void XflScriptExec::startXFoilTaskThread()
{
    FoilAnalysis *pAnalysis = nullptr;
    QString strong;
    //  browse through the array until we find an available thread

    XFoilTask *pXFoilTask = new XFoilTask(this);

    //take the last analysis in the array
    pAnalysis = m_FoilExecList.at(m_FoilExecList.size()-1);

    pAnalysis->pPolar->setVisible(true);

    //initiate the task

    pXFoilTask->initializeTask(pAnalysis, false, true, true, true);
    if(pAnalysis->pPolar->polarType()<XFLR5::FIXEDAOAPOLAR)
        pXFoilTask->setSequence(true, pAnalysis->vMin, pAnalysis->vMax, pAnalysis->vInc);
    else if(pAnalysis->pPolar->isFixedaoaPolar())
        pXFoilTask->setReRange(pAnalysis->vMin, pAnalysis->vMax, pAnalysis->vInc);

    //launch it
    m_nTaskStarted++;
    strong = "Starting "+pAnalysis->pFoil->foilName()+" / "+pAnalysis->pPolar->polarName()+"\n";
    traceLog(strong);

//    QThreadPool::globalInstance()->start(pXFoilTask);
    pXFoilTask->run();

    //remove it from the array of pairs to analyze
    pAnalysis = m_FoilExecList.last();
    m_FoilExecList.removeLast();
    delete pAnalysis;
}


void XflScriptExec::customEvent(QEvent *pEvent)
{
    // When the execution reaches this point, we've crossed the thread boundary and are now
    // executing in this object's thread
    if(pEvent->type() == XFOIL_END_TASK_EVENT)
    {
        // Now we can safely do something with our Qt objects.
        m_nTaskDone++; //one down, more to go

        XFoilTaskEvent *pXFEvent = dynamic_cast<XFoilTaskEvent*>(pEvent);
        QString str = "   ...Finished "+ (pXFEvent->foilPtr())->foilName()+" / "
                +(pXFEvent->polarPtr())->polarName()+"\n";

        traceLog(str);
    }
    else if(pEvent->type() == XFOIL_END_OPP_EVENT)
    {
        //		m_pRmsGraph->resetYLimits();
        XFoilOppEvent *pOppEvent = dynamic_cast<XFoilOppEvent*>(pEvent);
        Objects2d::addOpPoint(pOppEvent->foilPtr(), pOppEvent->polarPtr(), pOppEvent->oppPtr(), false);
    }
}


