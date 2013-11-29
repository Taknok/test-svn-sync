/****************************************************************************

	DisplaySettingsDlg Class
	Copyright (C) 2008-2008 Andre Deperrois adeperrois@xflr5.com

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

#include "DisplaySettingsDlg.h"
#include "../mainframe.h"
#include "../miarex/Miarex.h"
#include "../xdirect/XDirect.h"
#include "../xinverse/XInverse.h"
#include "../graph/GraphDlg.h"
#include <QApplication>
#include <QGroupBox>
#include <QColorDialog>
#include <QFontDialog>
#include <QStyleFactory>
#include <QDir>
#include <QtDebug>

bool DisplaySettingsDlg::s_bStyleSheets = true;
QString DisplaySettingsDlg::s_StyleName;
QString DisplaySettingsDlg::s_StyleSheetName;



DisplaySettingsDlg::DisplaySettingsDlg(QWidget *pParent) : QDialog(pParent)
{
	setWindowTitle(tr("General Display Settings"));
	m_pMainFrame = pParent;
	m_bIsGraphModified = false;
	m_bReverseZoom = false;
	m_bAlphaChannel = true;


#ifdef Q_WS_MAC
	m_StyleSheetDir.setPath(qApp->applicationDirPath());
#endif
#ifdef Q_WS_WIN
	m_StyleSheetDir.setPath(qApp->applicationDirPath());
#endif
#ifdef Q_OS_LINUX
	m_StyleSheetDir.setPath("/usr/share/xflr5");
#endif

	SetupLayout();

	connect(m_pctrlStyles, SIGNAL(activated(const QString &)),this, SLOT(OnStyleChanged(const QString &)));

	connect(m_pctrlBackColor, SIGNAL(clicked()),this, SLOT(OnBackgroundColor()));
	connect(m_pctrlGraphSettings, SIGNAL(clicked()),this, SLOT(OnGraphSettings()));
	connect(m_pctrlTextClr, SIGNAL(clicked()),this, SLOT(OnTextColor()));
	connect(m_pctrlTextFont, SIGNAL(clicked()),this, SLOT(OnTextFont()));

	connect(OKButton, SIGNAL(clicked()),this, SLOT(accept()));
	connect(CancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}


void DisplaySettingsDlg::SetupLayout()
{
	QVBoxLayout *MainLayout = new QVBoxLayout;

	m_pctrlStyles = new QComboBox;

	QRegExp regExp("Q(.*)Style");
	QString defaultStyle = QApplication::style()->metaObject()->className();
	if (defaultStyle == QLatin1String("QMacStyle"))
		defaultStyle = QLatin1String("Macintosh (Aqua)");
	else if (defaultStyle == QLatin1String("OxygenStyle"))
		defaultStyle = QLatin1String("Oxygen");
	else if (regExp.exactMatch(defaultStyle))
		defaultStyle = regExp.cap(1);

	m_pctrlStyles->addItems(QStyleFactory::keys());
	m_pctrlStyles->setCurrentIndex(m_pctrlStyles->findText(defaultStyle));

	// add custom style sheets

	QString fileName = "*.qss";
	QStringList filesList = MainFrame::s_StylesheetDir.entryList(QStringList(fileName), QDir::Files | QDir::NoSymLinks);


	for(int is=0; is<filesList.count(); is++)
	{
		QString styleSheetName = filesList.at(is);
		int len = styleSheetName.length();
		styleSheetName = styleSheetName.left(len-4);
		m_pctrlStyles->addItem(styleSheetName);
	}

//	m_pctrlStyleSheets = new QCheckBox(tr("Use Stylesheets"));

	QGroupBox *GraphBox = new QGroupBox(tr("Graph Settings"));
	{
		QHBoxLayout *GraphLayout = new QHBoxLayout;
		{
			m_pctrlGraphSettings  = new QPushButton(tr("All Graph Settings"));
			m_pctrlGraphSettings->setMinimumWidth(120);
			GraphLayout->addWidget(m_pctrlGraphSettings);
		}
		GraphBox->setLayout(GraphLayout);
	}


	QGroupBox *BackBox = new QGroupBox(tr("Background Color"));
	{
		QHBoxLayout *BackLayout = new QHBoxLayout;
		{
			m_pctrlBackColor      = new ColorButton;
			m_pctrlBackColor->setMinimumWidth(120);
			BackLayout->addWidget(m_pctrlBackColor);
		}
		BackBox->setLayout(BackLayout);
	}

	QGroupBox *FontBox = new QGroupBox(tr("Font"));
	{
		QHBoxLayout *FontLayout = new QHBoxLayout;
		{
			m_pctrlTextFont       = new QPushButton;
			m_pctrlTextClr        = new QPushButton(tr("Text Color"));
			m_pctrlTextFont->setMinimumWidth(120);
			m_pctrlTextClr->setMinimumWidth(120);
			FontLayout->addWidget(m_pctrlTextFont);
			FontLayout->addWidget(m_pctrlTextClr);
		}
		FontBox->setLayout(FontLayout);
	}

	m_pctrlReverseZoom = new QCheckBox(tr("Reverse zoom direction using mouse wheel"));
	m_pctrlAlphaChannel = new QCheckBox(tr("Enable 3D transparency"));

	QHBoxLayout *CommandButtons = new QHBoxLayout;
	{
		OKButton = new QPushButton(tr("OK"));
		OKButton->setAutoDefault(false);
		CancelButton = new QPushButton(tr("Cancel"));
		CancelButton->setAutoDefault(false);
		CommandButtons->addStretch(1);
		CommandButtons->addWidget(OKButton);
		CommandButtons->addStretch(1);
		CommandButtons->addWidget(CancelButton);
		CommandButtons->addStretch(1);
	}

	MainLayout->addStretch(1);
	MainLayout->addWidget(m_pctrlStyles);
//	MainLayout->addWidget(m_pctrlStyleSheets);
	MainLayout->addStretch(1);
	MainLayout->addWidget(BackBox);
	MainLayout->addStretch(1);
	MainLayout->addWidget(FontBox);
	MainLayout->addStretch(1);
	MainLayout->addWidget(GraphBox);
	MainLayout->addStretch(1);
	MainLayout->addWidget(m_pctrlReverseZoom);
	MainLayout->addStretch(1);
	MainLayout->addWidget(m_pctrlAlphaChannel);
	MainLayout->addSpacing(20);
	MainLayout->addStretch(1);
	MainLayout->addLayout(CommandButtons);
	MainLayout->addStretch(1);

	setLayout(MainLayout);
}


void DisplaySettingsDlg::InitDialog()
{
//	m_pctrlStyleSheets->setChecked(m_bStyleSheets);
	m_MemGraph.CopySettings(m_pRefGraph);
	m_pctrlBackColor->SetColor(m_BackgroundColor);

	QString FontName = m_TextFont.family() + QString(" %1").arg(m_TextFont.pointSize());
	m_pctrlTextFont->setText(FontName);

	if(m_pctrlStyles->findText(s_StyleName)>=0)
		m_pctrlStyles->setCurrentIndex(m_pctrlStyles->findText(s_StyleName));
	else if(m_pctrlStyles->findText(s_StyleSheetName)>=0)
		m_pctrlStyles->setCurrentIndex(m_pctrlStyles->findText(s_StyleSheetName));


	m_pctrlReverseZoom->setChecked(m_bReverseZoom);
	m_pctrlAlphaChannel->setChecked(m_bAlphaChannel);

	QPalette palette = m_pctrlTextClr->palette();
	QColor listColor = palette.color(QPalette::Button);
	if(listColor.isValid())
	{
		palette.setColor(QPalette::Button, m_BackgroundColor);
		palette.setColor(QPalette::ButtonText, m_TextColor);
		m_pctrlTextClr->setPalette(palette);
		m_pctrlTextClr->setFont(m_TextFont);
	}
}


void DisplaySettingsDlg::OnStyleChanged(const QString &StyleName)
{
	//test for style sheet

	QString fileName = "*.qss";
	QStringList filesList = MainFrame::s_StylesheetDir.entryList(QStringList(fileName), QDir::Files | QDir::NoSymLinks);

	for(int is=0; is<filesList.count(); is++)
	{
		QString styleSheetName = filesList.at(is);
		qDebug()<<styleSheetName;
		int len = styleSheetName.length();
		styleSheetName = styleSheetName.left(len-4);
		if(styleSheetName.compare(StyleName)==0)
		{
			s_bStyleSheets = true;
			s_StyleSheetName = styleSheetName;
			s_StyleName.clear();
			QString styleSheet;
			MainFrame::ReadStyleSheet(styleSheetName, styleSheet);
			return;
		}
	}

	s_bStyleSheets = false;
	s_StyleSheetName.clear();
	qApp->setStyleSheet(styleSheet());
	qApp->setStyle(StyleName);
	s_StyleName = StyleName;
}


void DisplaySettingsDlg::OnBackgroundColor()
{
	QColor Color = QColorDialog::getColor(m_BackgroundColor);
	if(Color.isValid()) m_BackgroundColor = Color;

	m_pctrlBackColor->SetColor(m_BackgroundColor);

	QPalette palette = m_pctrlTextClr->palette();
	QColor listColor = palette.color(QPalette::Button);
	if(listColor.isValid())
	{
//		palette.setColor(QPalette::Background, m_BackgroundColor);
		palette.setColor(QPalette::Button, m_BackgroundColor);
		palette.setColor(QPalette::ButtonText, m_TextColor);
		m_pctrlTextClr->setPalette(palette);
	}
}


void DisplaySettingsDlg::reject()
{
	MainFrame *pMainFrame = (MainFrame*)m_pMainFrame;
	QXDirect *pXDirect   = (QXDirect*)pMainFrame->m_pXDirect;
	QMiarex *pMiarex     = (QMiarex*)pMainFrame->m_pMiarex;
	QXInverse *pXInverse = (QXInverse*)pMainFrame->m_pXInverse;

	pXDirect->m_pCpGraph->CopySettings(&m_MemGraph);
	pXDirect->m_pCpGraph->SetInverted(true);

	for(int ig=0; ig<MAXPOLARGRAPHS; ig++)
		pXDirect->m_PlrGraph[ig].CopySettings(&m_MemGraph);

	for(int ig=0; ig<4; ig++)
	{
		pMiarex->m_WingGraph[ig].CopySettings(&m_MemGraph);
		pMiarex->m_TimeGraph[ig].CopySettings(&m_MemGraph);
		pMiarex->m_WPlrGraph[ig].CopySettings(&m_MemGraph);
	}

	pXInverse->m_QGraph.CopySettings(&m_MemGraph);
	pXInverse->m_QGraph.SetInverted(true);

	QDialog::reject();
}



void DisplaySettingsDlg::OnGraphSettings()
{
	if(!m_pRefGraph) return;
	MainFrame *pMainFrame = (MainFrame*)m_pMainFrame;
	QXDirect *pXDirect   = (QXDirect*)pMainFrame->m_pXDirect;
	QMiarex *pMiarex     = (QMiarex*)pMainFrame->m_pMiarex;
	QXInverse *pXInverse = (QXInverse*)pMainFrame->m_pXInverse;

    GraphDlg dlg(this);

	dlg.m_GraphArray[0] = pXDirect->m_pCpGraph;
	for(int ig=0; ig<MAXPOLARGRAPHS; ig++)
		dlg.m_GraphArray[ig+1] = pXDirect->m_PlrGraph+ig;

	dlg.m_GraphArray[6] = pMiarex->m_WingGraph;
	dlg.m_GraphArray[7] = pMiarex->m_WingGraph+1;
	dlg.m_GraphArray[8] = pMiarex->m_WingGraph+2;
	dlg.m_GraphArray[9] = pMiarex->m_WingGraph+3;
	dlg.m_GraphArray[10] = pMiarex->m_WPlrGraph;
	dlg.m_GraphArray[11] = pMiarex->m_WPlrGraph+1;
	dlg.m_GraphArray[12] = pMiarex->m_WPlrGraph+2;
	dlg.m_GraphArray[13] = pMiarex->m_WPlrGraph+3;
	dlg.m_GraphArray[14] = pMiarex->m_TimeGraph;
	dlg.m_GraphArray[15] = pMiarex->m_TimeGraph+1;
	dlg.m_GraphArray[16] = pMiarex->m_TimeGraph+2;
	dlg.m_GraphArray[17] = pMiarex->m_TimeGraph+3;
	dlg.m_GraphArray[18] = &pMiarex->m_LongRLGraph;
	dlg.m_GraphArray[19] = &pMiarex->m_LatRLGraph;

	dlg.m_GraphArray[20] = &pXInverse->m_QGraph;

	dlg.m_NGraph = 21;

	QGraph graph;
	graph.CopySettings(m_pRefGraph);

	dlg.m_pMemGraph = m_pRefGraph;
	dlg.m_pGraph = &graph;

	dlg.SetParams();

	if(dlg.exec() == QDialog::Accepted)
	{
		m_pRefGraph->CopySettings(dlg.m_pGraph);
		m_bIsGraphModified = true;

		pXDirect->m_pCpGraph->SetInverted(true);
		pXInverse->m_QGraph.SetInverted(true);
	}
}


void DisplaySettingsDlg::OnTextColor()
{
	QColor Color = QColorDialog::getColor(m_TextColor);
	if(Color.isValid()) m_TextColor = Color;

	QPalette palette = m_pctrlTextClr->palette();
	QColor listColor = palette.color(QPalette::Button);
	if(listColor.isValid())
	{
//		palette.setColor(QPalette::Background, m_BackgroundColor);
		palette.setColor(QPalette::Button, m_BackgroundColor);
		palette.setColor(QPalette::ButtonText, m_TextColor);
		m_pctrlTextClr->setPalette(palette);
//		m_pctrlTextClr->setAutoFillBackground(true);
	}
}



void DisplaySettingsDlg::OnTextFont()
{
	bool ok;
	QFont TextFont;
	TextFont.setStyleHint(QFont::TypeWriter, QFont::OpenGLCompatible);

#ifdef Q_WS_MAC
	//20090604 Mac OS Native font dialog does not work well under QT 4.5.1
	//QFont font = QFontDialog::getFont(&ok, m_TextFont, this);
        //20110324 Works again under QT 4.6, though it loses focus is mouse is moved outside of it (QT bug?)
        //QFont font = QFontDialog::getFont(&ok, m_TextFont, this, "",QFontDialog::DontUseNativeDialog);
	   TextFont = QFontDialog::getFont(&ok, m_TextFont, this);
#else
	TextFont = QFontDialog::getFont(&ok, m_TextFont, this);
#endif

	if (ok)
	{
		m_TextFont = TextFont;
		m_pctrlTextFont->setText(m_TextFont.family());
		m_pctrlTextFont->setFont(m_TextFont);
		m_pctrlTextClr->setFont(m_TextFont);
	}
}








