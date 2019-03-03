/****************************************************************************

    OpenGLInfoDlg Class
    Copyright (C) 2015-2019 Andre Deperrois

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

#include <QVBoxLayout>
#include <QComboBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QSplitter>
#include <QSurfaceFormat>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QCoreApplication>
#include <QDebug>

#include "openglinfodlg.h"
#include "glrenderwindow.h"
#include <misc/text/MinTextEdit.h>

struct Version {
	const char *str;
	int major;
	int minor;
};


static struct Version versions[] = {
	{ "1.0", 1, 0 },
	{ "1.1", 1, 1 },
	{ "1.2", 1, 2 },
	{ "1.3", 1, 3 },
	{ "1.4", 1, 4 },
	{ "1.5", 1, 5 },
	{ "2.0", 2, 0 },
	{ "2.1", 2, 1 },
	{ "3.0", 3, 0 },
	{ "3.1", 3, 1 },
	{ "3.2", 3, 2 },
	{ "3.3", 3, 3 },
	{ "4.0", 4, 0 },
	{ "4.1", 4, 1 },
	{ "4.2", 4, 2 },
	{ "4.3", 4, 3 },
	{ "4.4", 4, 4 },
	{ "4.5", 4, 5 }
};


struct Profile {
	const char *str;
	QSurfaceFormat::OpenGLContextProfile profile;
};


static struct Profile profiles[] = {
	{ "none", QSurfaceFormat::NoProfile },
	{ "core", QSurfaceFormat::CoreProfile },
	{ "compatibility", QSurfaceFormat::CompatibilityProfile }
};


struct Option {
	const char *str;
	QSurfaceFormat::FormatOption option;
};

static struct Option options[] = {
	{ "deprecated functions (not forward compatible)", QSurfaceFormat::DeprecatedFunctions },
	{ "debug context", QSurfaceFormat::DebugContext },
	{ "stereo buffers", QSurfaceFormat::StereoBuffers },
	// This is not a QSurfaceFormat option but is helpful to determine if the driver
	// allows compiling old-style shaders with core profile.
	{ "force version 110 shaders", QSurfaceFormat::FormatOption(0) }
};


OpenGLInfoDlg::OpenGLInfoDlg(QWidget *pParent) : QDialog(pParent)
{
	setWindowTitle("OpenGL context info");
	setupLayout();
}


void OpenGLInfoDlg::addVersions(QLayout *layout)
{
	QHBoxLayout *pHBoxLayout = new QHBoxLayout;
	{
		pHBoxLayout->setSpacing(20);
        QLabel *label = new QLabel(QObject::tr("Context &version: "));
		pHBoxLayout->addWidget(label);
		m_version = new QComboBox;
		m_version->setMinimumWidth(60);
		label->setBuddy(m_version);
		pHBoxLayout->addWidget(m_version);
		for (size_t i = 0; i < sizeof(versions) / sizeof(Version); ++i) {
			m_version->addItem(QString::fromLatin1(versions[i].str));
			if (versions[i].major == 3 && versions[i].minor == 3)
				m_version->setCurrentIndex(m_version->count() - 1);
		}

        QPushButton *btn = new QPushButton(QObject::tr("Create context"));
		connect(btn, &QPushButton::clicked, this, &OpenGLInfoDlg::start);
		btn->setMinimumSize(120, 40);
		pHBoxLayout->addWidget(btn);
	}

	layout->addItem(pHBoxLayout);
}


void OpenGLInfoDlg::addProfiles(QLayout *layout)
{
    QGroupBox *pGroupBox = new QGroupBox(QObject::tr("Profile"));
	{
		QVBoxLayout *pVBoxLayout = new QVBoxLayout;
		{
			for (size_t i = 0; i < sizeof(profiles) / sizeof(Profile); ++i)
				pVBoxLayout->addWidget(new QRadioButton(QString::fromLatin1(profiles[i].str)));
			static_cast<QRadioButton *>(pVBoxLayout->itemAt(0)->widget())->setChecked(true);
		}
		pGroupBox->setLayout(pVBoxLayout);
		m_profiles = pVBoxLayout;
	}
	layout->addWidget(pGroupBox);
}


void OpenGLInfoDlg::addOptions(QLayout *layout)
{
    QGroupBox *pGroupBox = new QGroupBox(QObject::tr("Options"));
	{
		QVBoxLayout *pVBoxLayout = new QVBoxLayout;
		{
			for (size_t i = 0; i < sizeof(options) / sizeof(Option); ++i)
				pVBoxLayout->addWidget(new QCheckBox(QString::fromLatin1(options[i].str)));
			pGroupBox->setLayout(pVBoxLayout);
		}
		m_options = pVBoxLayout;
	}
	layout->addWidget(pGroupBox);
}



void OpenGLInfoDlg::addRenderWindow()
{
    m_pRenderWindowLayout->addWidget(m_renderWindowContainer);
}


QWidget *widgetWithLayout(QLayout *layout)
{
	QWidget *w = new QWidget;
	w->setLayout(layout);
	return w;
}


void OpenGLInfoDlg::setupLayout()
{
	QVBoxLayout *pMainLayout = new QVBoxLayout;
	{
		QSplitter *pVSplit = new QSplitter(Qt::Vertical);
		{
			pMainLayout->addWidget(pVSplit);

			QSplitter *pHSplit = new QSplitter;
			{
				QVBoxLayout *pSettingsLayout = new QVBoxLayout;
				{
					addVersions(pSettingsLayout);
					addProfiles(pSettingsLayout);
					addOptions(pSettingsLayout);
				}
				pHSplit->addWidget(widgetWithLayout(pSettingsLayout));


				m_glOutput = new QTextEdit;
				m_glOutput->setReadOnly(true);
				pHSplit->addWidget(m_glOutput);

				pHSplit->setStretchFactor(0, 4);
				pHSplit->setStretchFactor(1, 6);
			}
			pVSplit->addWidget(pHSplit);


            m_pRenderWindowLayout = new QVBoxLayout;
            pVSplit->addWidget(widgetWithLayout(m_pRenderWindowLayout));
			pVSplit->setStretchFactor(1, 7);
		}

		m_renderWindowContainer = new QWidget;
		addRenderWindow();

	}
	setLayout(pMainLayout);
}


void OpenGLInfoDlg::start()
{
	QSurfaceFormat fmt;

	int idx = m_version->currentIndex();
	if (idx < 0)
		return;
	fmt.setVersion(versions[idx].major, versions[idx].minor);

	for (size_t i=0; i<sizeof(profiles)/sizeof(Profile); ++i)
		if (static_cast<QRadioButton *>(m_profiles->itemAt(int(i))->widget())->isChecked()) {
			fmt.setProfile(profiles[i].profile);
			break;
		}

	bool forceGLSL110 = false;
	for (size_t i=0; i<sizeof(options)/sizeof(Option); ++i)
		if (static_cast<QCheckBox *>(m_options->itemAt(int(i))->widget())->isChecked()) {
			if (options[i].option)
				fmt.setOption(options[i].option);
			else if (i == 3)
				forceGLSL110 = true;
		}

	fmt.setRenderableType(QSurfaceFormat::OpenGL);

	// The example rendering will need a depth buffer.
	fmt.setDepthBufferSize(16);

	m_glOutput->clear();

    m_pRenderWindowLayout->removeWidget(m_renderWindowContainer);
	delete m_renderWindowContainer;

	GLRenderWindow *renderWindow = new GLRenderWindow(fmt);
	if (!renderWindow->context())
	{
        m_glOutput->append(QObject::tr("Failed to create context"));
		delete renderWindow;
		m_renderWindowContainer = new QWidget;
		addRenderWindow();
		return;
	}
	m_surface = renderWindow;

	m_glOutput->append("OpenGL support:");

	QString strange;
	strange = "    Desktop OpenGL";
	qApp->testAttribute(Qt::AA_UseDesktopOpenGL) ? strange += ": true" : strange+=": false";
	m_glOutput->append(strange);

	strange = "    OpenGL ES";
	qApp->testAttribute(Qt::AA_UseOpenGLES) ? strange += ": true" : strange+=": false";
	m_glOutput->append(strange);

	strange = "    Software OpenGL";
	qApp->testAttribute(Qt::AA_UseSoftwareOpenGL) ? strange += ": true" : strange+=": false";
	m_glOutput->append(strange+"\n");



	renderWindow->setForceGLSL110(forceGLSL110);
	connect(renderWindow, &GLRenderWindow::ready, this, &OpenGLInfoDlg::renderWindowReady);
	connect(renderWindow, &GLRenderWindow::error, this, &OpenGLInfoDlg::renderWindowError);

	m_renderWindowContainer = QWidget::createWindowContainer(renderWindow);
	addRenderWindow();
}


void OpenGLInfoDlg::printFormat(const QSurfaceFormat &format)
{
    m_glOutput->append(QObject::tr("OpenGL version: %1.%2").arg(format.majorVersion()).arg(format.minorVersion()));

	for (size_t i = 0; i < sizeof(profiles) / sizeof(Profile); ++i)
		if (profiles[i].profile == format.profile()) {
            m_glOutput->append(QObject::tr("Profile: %1").arg(QString::fromLatin1(profiles[i].str)));
			break;
		}

	QString opts;
	for (size_t i = 0; i < sizeof(options) / sizeof(Option); ++i)
		if (format.testOption(options[i].option))
			opts += QString::fromLatin1(options[i].str) + QStringLiteral(" ");
    m_glOutput->append(QString("Options: %1").arg(opts));
    m_glOutput->append(QString("Depth buffer size: %1").arg(QString::number(format.depthBufferSize())));
    m_glOutput->append(QString("Stencil buffer size: %1").arg(QString::number(format.stencilBufferSize())));
    m_glOutput->append(QString("Samples: %1").arg(QString::number(format.samples())));
    m_glOutput->append(QString("Red buffer size: %1").arg(QString::number(format.redBufferSize())));
    m_glOutput->append(QString("Green buffer size: %1").arg(QString::number(format.greenBufferSize())));
    m_glOutput->append(QString("Blue buffer size: %1").arg(QString::number(format.blueBufferSize())));
    m_glOutput->append(QString("Alpha buffer size: %1").arg(QString::number(format.alphaBufferSize())));
    m_glOutput->append(QString("Swap interval: %1").arg(QString::number(format.swapInterval())));
}


void OpenGLInfoDlg::renderWindowReady()
{
	QOpenGLContext *context = QOpenGLContext::currentContext();
	Q_ASSERT(context);

	QString vendor, renderer, version, glslVersion;
	const GLubyte *p;
	QOpenGLFunctions *f = context->functions();
	if ((p = f->glGetString(GL_VENDOR)))
		vendor = QString::fromLatin1(reinterpret_cast<const char *>(p));
	if ((p = f->glGetString(GL_RENDERER)))
		renderer = QString::fromLatin1(reinterpret_cast<const char *>(p));
	if ((p = f->glGetString(GL_VERSION)))
		version = QString::fromLatin1(reinterpret_cast<const char *>(p));
	if ((p = f->glGetString(GL_SHADING_LANGUAGE_VERSION)))
		glslVersion = QString::fromLatin1(reinterpret_cast<const char *>(p));

    m_glOutput->append(QString("*** Context information ***"));
    m_glOutput->append(QString("Vendor: %1").arg(vendor));
    m_glOutput->append(QString("Renderer: %1").arg(renderer));
    m_glOutput->append(QString("OpenGL version: %1").arg(version));
    m_glOutput->append(QString("GLSL version: %1").arg(glslVersion));

    m_glOutput->append(QString("\n*** QSurfaceFormat from context ***"));
	printFormat(context->format());

    m_glOutput->append(QString("\n*** QSurfaceFormat from window surface ***"));
	printFormat(m_surface->format());

    m_glOutput->append(QString("\n*** Qt build information ***"));
	const char *gltype[] = { "Desktop", "GLES 2", "GLES 1" };
    m_glOutput->append(QString("Qt OpenGL configuration: %1")
					 .arg(QString::fromLatin1(gltype[QOpenGLContext::openGLModuleType()])));
    m_glOutput->append(QString("Qt OpenGL library handle: %1")
					 .arg(QString::number(qintptr(QOpenGLContext::openGLModuleHandle()), 16)));

	m_glOutput->moveCursor(QTextCursor::Start);
}


void OpenGLInfoDlg::renderWindowError(const QString &msg)
{
    m_glOutput->append(QString("An error has occurred:\n%1").arg(msg));
}


