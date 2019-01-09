/****************************************************************************

    Updater Class
    Copyright (C) 2018-2019 Andre Deperrois

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

#include <QtXml/QDomDocument>
#include <QtXml/QDomNodeList>


#include "updater.h"
#include <globals/mainframe.h>
#include <globals/gui_params.h>

bool Updater::s_bAutoCheck = true;
int Updater::s_AvailableMajorVersion=-1;
int Updater::s_AvailableMinorVersion=-1;
QDate Updater::s_LastCheckDate;

MainFrame *Updater::s_pMainFrame = nullptr;

Updater::Updater(MainFrame *pMainFrame)
{
	s_pMainFrame = pMainFrame;
	m_Date.clear();
	m_Description.clear();
}


Updater::~Updater()
{
	delete m_pNetworkAcessManager;
}


void Updater::checkForUpdates()
{
	QUrl url("https://www.xflr5.com/rss/rssfeed.xml");

	QNetworkRequest request;
	request.setUrl(url);
	request.setRawHeader("User-Agent", "MyOwnBrowser 1.0");

	m_pNetworkAcessManager = new QNetworkAccessManager;
/*	switch(m_pNetworkAcessManager->networkAccessible())
	{
		case QNetworkAccessManager::UnknownAccessibility:
			qDebug()<<"UnknownAccessibility";
			return;
		case QNetworkAccessManager::NotAccessible:
			qDebug()<<"Netw<ork Not Accessible";
			return;
		case QNetworkAccessManager::Accessible:
			qDebug()<<"Accessible";
			break;
	}*/

	m_pNetworkReply = m_pNetworkAcessManager->get(request);
	connect(m_pNetworkAcessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(onReplyFinished(QNetworkReply*)));

/*	qDebug()<<m_pNetworkReply->isOpen();
	qDebug()<<m_pNetworkReply->isReadable();
	qDebug()<<m_pNetworkReply->isWritable();*/
	connect(m_pNetworkReply, SIGNAL(readyRead()),                        this, SLOT(onReadyRead()));
	connect(m_pNetworkReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
	connect(m_pNetworkReply, SIGNAL(sslErrors(QList<QSslError>)),        this, SLOT(slotSslErrors(QList<QSslError>)));

/*	QUrl ud("http://www.xflr5.com/rss/updatedescription.txt");
	if(ud.isValid())
	{
		m_pNetworkAcessManager->get(QNetworkRequest(ud));
		QObject::connect(m_pNetworkAcessManager, SIGNAL(finished(QNetworkReply*)), SLOT(onDownloadFinished(QNetworkReply*)));
	}*/
}


void Updater::slotError(QNetworkReply::NetworkError)
{
	qDebug()<<"some network error";
}


void Updater::slotSslErrors(QList<QSslError>)
{
	qDebug()<<"SSL error";
}


void Updater::onReadyRead()
{
//	qDebug()<<"readyRead()";
}



void Updater::onReplyFinished(QNetworkReply* netReply)
{
	QString str (netReply->readAll());

	/* If we are redirected, try again. TODO: Limit redirection count. */
	QVariant vt = netReply->attribute(QNetworkRequest::RedirectionTargetAttribute);

	m_pNetworkReply->deleteLater();

	if (!vt.isNull())
	{
		qDebug() << "Redirected to:" << vt.toUrl().toString();
		m_pNetworkReply = m_pNetworkAcessManager->get(QNetworkRequest(vt.toUrl()));
//		connect(m_pNetworkReply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));
	}
	else
	{
		// We have something.
		QDomDocument doc;
		QString error;
		if (!doc.setContent(str, false, &error))
		{
//			m_pWebEngineView->setHtml(QString("<h1>Error</h1>") + error);
		}
		else
		{
			QDomElement docElem = doc.documentElement();
			QDomNodeList nodeList = docElem.elementsByTagName("item");

			for (int i=0; i<nodeList.length(); i++)
			{
				QDomNode node = nodeList.item(i);
				QDomElement e = node.toElement();
/*				QString strTitle =  e.elementsByTagName("title").item(0).firstChild().nodeValue();
				QString strLink = e.elementsByTagName("link").item(0).firstChild().nodeValue();
				QString strDescription = e.elementsByTagName("description").item(0).firstChild().nodeValue();
                qDebug()<<"    "<<strTitle;
                qDebug()<<"    "<<strLink;
                qDebug()<<"    "<<strDescription;*/
			}
			nodeList = docElem.elementsByTagName("xflversion");

			for (int i=0; i<nodeList.length(); i++)
			{
				QDomNode node = nodeList.item(i);
				QDomElement e = node.toElement();
                s_AvailableMajorVersion  = e.elementsByTagName("majorversion").item(0).firstChild().nodeValue().toInt();
                s_AvailableMinorVersion  = e.elementsByTagName("minorversion").item(0).firstChild().nodeValue().toInt();
				m_Date = e.elementsByTagName("date").item(0).firstChild().nodeValue();
				m_Description = e.elementsByTagName("description").item(0).firstChild().nodeValue();
			}
/*			QUrl ud("http://www.xflr5.com/rss/updatedescription.txt");
			if(ud.isValid())
			{
				m_NetworkAcessManager.get(QNetworkRequest(ud));
				QObject::connect(&m_NetworkAcessManager, SIGNAL(finished(QNetworkReply*)), SLOT(onDownloadFinished(QNetworkReply*)));
			}*/

            s_LastCheckDate = QDate::currentDate();
		}
	}
	emit finishedUpdate();
}


/** downloads a file and prints it */
void Updater::onDownloadFinished(QNetworkReply *pResponse)
{
	  pResponse->deleteLater();

	  if (pResponse->error() != QNetworkReply::NoError) return;
	  QString contentType = pResponse->header(QNetworkRequest::ContentTypeHeader).toString();
/*	  if (!contentType.contains("charset=utf-8")) {
		  qWarning() << "Content charsets other than utf-8 are not implemented yet.";
		  return;
	  }*/
	  QString html = QString::fromUtf8(pResponse->readAll());
	  qDebug()<<html;
}


bool Updater::hasUpdate()
{
    if(s_AvailableMajorVersion>MAJOR_VERSION) return true;
    else if(s_AvailableMajorVersion==MAJOR_VERSION)
    {
        if(s_AvailableMinorVersion>MINOR_VERSION) return true;
    }
    return false;
}


void Updater::loadSettings(QSettings *pSettings)
{
    pSettings->beginGroup("Updater");
    {
        s_bAutoCheck = pSettings->value("AutoUpdateCheck", true).toBool();
        s_LastCheckDate = pSettings->value("LastCheckDate", false).toDate();
    }
//    qDebug()<<"loading lastcheckdate"<<s_LastCheckDate.toString("yyyy.MM.dd");
    pSettings->endGroup();
}


void Updater::saveSettings(QSettings *pSettings)
{
    pSettings->beginGroup("Updater");
    {
//        qDebug()<<"saving lastcheckdate"<<s_LastCheckDate.toString("yyyy.MM.dd");
        pSettings->setValue("AutoUpdateCheck", s_bAutoCheck);
        pSettings->setValue("LastCheckDate", s_LastCheckDate);
    }
    pSettings->endGroup();

}
