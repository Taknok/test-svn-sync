/****************************************************************************

    OpenGlDlg Class
    Copyright (C) 2019 Andre Deperrois

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

#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDebug>
#include <QGroupBox>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QOpenGLContext>
#include <QPushButton>
#include <QRadioButton>
#include <QFontMetrics>
#include <QSplitter>
#include <QSurfaceFormat>
#include <QVBoxLayout>
#include <QPlainTextEdit>

#include "opengldlg.h"


#include <viewwidgets/glWidgets/gl3dtestview.h>

QByteArray OpenGlDlg::s_Geometry;

struct Profile {
    QSurfaceFormat::OpenGLContextProfile profile;
    const char *str = nullptr;
};


static struct Profile profiles[] = {
    { QSurfaceFormat::NoProfile, "none" },
    { QSurfaceFormat::CoreProfile, "core" },
    { QSurfaceFormat::CompatibilityProfile, "compatibility" }
};


struct Option {
    const char *str;
    QSurfaceFormat::FormatOption option;
};


static struct Option options[] = {
    { "deprecated functions (not forward compatible)", QSurfaceFormat::DeprecatedFunctions },
    { "debug context", QSurfaceFormat::DebugContext },
    { "stereo buffers", QSurfaceFormat::StereoBuffers }
    // This is not a QSurfaceFormat option but is helpful to determine if the driver
    // allows compiling old-style shaders with core profile.

};


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


OpenGlDlg::OpenGlDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle("OpenGL context info");
    setupLayout();

    // select the active default format in the CbBox
    QPair<int, int> oglversion = QSurfaceFormat::defaultFormat().version();
    if(oglversion.first==1)
    {
        if     (oglversion.second==0) m_pctrlVersionCbBox->setCurrentIndex(0);
        else if(oglversion.second==1) m_pctrlVersionCbBox->setCurrentIndex(1);
        else if(oglversion.second==2) m_pctrlVersionCbBox->setCurrentIndex(2);
        else if(oglversion.second==3) m_pctrlVersionCbBox->setCurrentIndex(3);
        else if(oglversion.second==4) m_pctrlVersionCbBox->setCurrentIndex(4);
        else if(oglversion.second==5) m_pctrlVersionCbBox->setCurrentIndex(5);
    }
    else if(oglversion.first==2)
    {
        if     (oglversion.second==0) m_pctrlVersionCbBox->setCurrentIndex(6);
        else if(oglversion.second==1) m_pctrlVersionCbBox->setCurrentIndex(7);
    }
    else if(oglversion.first==3)
    {
        if     (oglversion.second==0) m_pctrlVersionCbBox->setCurrentIndex(8);
        else if(oglversion.second==1) m_pctrlVersionCbBox->setCurrentIndex(9);
        else if(oglversion.second==2) m_pctrlVersionCbBox->setCurrentIndex(10);
        else if(oglversion.second==3) m_pctrlVersionCbBox->setCurrentIndex(11);
    }
    else if(oglversion.first==4)
    {
        if     (oglversion.second==0) m_pctrlVersionCbBox->setCurrentIndex(12);
        else if(oglversion.second==1) m_pctrlVersionCbBox->setCurrentIndex(13);
        else if(oglversion.second==2) m_pctrlVersionCbBox->setCurrentIndex(14);
        else if(oglversion.second==3) m_pctrlVersionCbBox->setCurrentIndex(15);
        else if(oglversion.second==4) m_pctrlVersionCbBox->setCurrentIndex(16);
        else if(oglversion.second==5) m_pctrlVersionCbBox->setCurrentIndex(17);
    }

    m_bVersionChanged = false;
}


void OpenGlDlg::onApply()
{
    if(m_bVersionChanged)
    {
        QString version = m_pctrlVersionCbBox->currentText();
        QString strange = "Set OpenGL "+ version +" as the future default?\n(Application restart required)";
        int resp = QMessageBox::question(this, tr("Close"), strange,
                                         QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel,
                                         QMessageBox::No);
        if(resp == QMessageBox::Yes)
        {
            QString vtext = m_pctrlVersionCbBox->currentText();
            float version = vtext.toFloat();
            int oglmaj = int(version);
            int oglmin = int(roundf(version*10.0f - float(oglmaj)*10.0f));
            gl3dView::setOGLVersion(oglmaj, oglmin);// saves the setting, but does not modify the QSurfaceFormat default

            // modify the QSurfaceFormat default format, although this won't affect already created contexts
            QSurfaceFormat fmt = QSurfaceFormat::defaultFormat();
            fmt.setVersion(oglmaj, oglmin);
            QSurfaceFormat::setDefaultFormat(fmt);
        }
    }
}


void OpenGlDlg::addVersions(QLayout *pLayout)
{
    QHBoxLayout *pHBoxLayout = new QHBoxLayout;
    {
        pHBoxLayout->setSpacing(20);
        QLabel *label = new QLabel(tr("Context version: "));
        pHBoxLayout->addWidget(label);
        m_pctrlVersionCbBox = new QComboBox;
        connect(m_pctrlVersionCbBox, SIGNAL(activated(int)), this, SLOT(onVersionChanged()));

        label->setBuddy(m_pctrlVersionCbBox);
        pHBoxLayout->addWidget(m_pctrlVersionCbBox);

        for (size_t i = 0; i < sizeof(versions) / sizeof(Version); ++i)
        {
            m_pctrlVersionCbBox->addItem(QString::fromLatin1(versions[i].str));
            if (versions[i].major == 3 && versions[i].minor == 3)
                m_pctrlVersionCbBox->setCurrentIndex(m_pctrlVersionCbBox->count() - 1);
        }
    }

    pLayout->addItem(pHBoxLayout);
}


void OpenGlDlg::addProfiles(QLayout *pLayout)
{
    QGroupBox *pGroupBox = new QGroupBox(tr("Profile"));
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
    pLayout->addWidget(pGroupBox);
}


void OpenGlDlg::addOptions(QLayout *pLayout)
{
    QGroupBox *pGroupBox = new QGroupBox(tr("Options"));
    {
        QVBoxLayout *pVBoxLayout = new QVBoxLayout;
        {
            for (size_t i=0; i < sizeof(options) / sizeof(Option); ++i)
                pVBoxLayout->addWidget(new QCheckBox(QString::fromLatin1(options[i].str)));
            pGroupBox->setLayout(pVBoxLayout);
        }
        m_pOptionsLayout = pVBoxLayout;
    }
    pLayout->addWidget(pGroupBox);
}


void OpenGlDlg::keyPressEvent(QKeyEvent *pEvent)
{
/*    bool bCtrl = (pEvent->modifiers() & Qt::ControlModifier);
    switch (pEvent->key())
    {
        case Qt::Key_W:
            if(bCtrl) reject();
            break;
        case Qt::Key_Escape:
            reject();
            break;
        default:
            QWidget::keyPressEvent(pEvent);
    }
    pEvent->ignore();*/

    QWidget::keyPressEvent(pEvent);
}



void OpenGlDlg::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        QHBoxLayout *pCurrentDefaultLayout =new QHBoxLayout;
        {
            QLabel *pDefaultLab = new QLabel("Current context version requested by the application:");
            QString strange;
            strange.sprintf("OpenGL %d.%d", gl3dView::oglMajor(), gl3dView::oglMinor());
            QLabel *pCurrentLab = new QLabel(strange);

            pCurrentDefaultLayout->addWidget(pDefaultLab);
            pCurrentDefaultLayout->addWidget(pCurrentLab);
            pCurrentDefaultLayout->addStretch();
        }


        QSplitter *pVSplitter = new QSplitter(Qt::Vertical);
        {
            pVSplitter->setChildrenCollapsible(false);
            QGroupBox *pTestContextBox = new QGroupBox(tr("Test OpenGL context"));
            {
                QHBoxLayout *pTestLayout = new QHBoxLayout;
                {
                    QWidget *pSettings = new QWidget();
                    {
                        QVBoxLayout *pSettingsLayout = new QVBoxLayout;
                        {

                            QFont fnt;
                            QFontMetrics fm(fnt);

                            QPushButton *pBtn = new QPushButton(tr("Test context"));
                            int h = fm.height()*3;
                            pBtn->setMinimumHeight(h);
                            connect(pBtn, SIGNAL(clicked()), SLOT(onCreateContext()));

                            QLabel *pLabApply = new QLabel("Apply to make this version the new default.");

                            addVersions(pSettingsLayout);
                            addProfiles(pSettingsLayout);
                            addOptions(pSettingsLayout);
                            pSettingsLayout->addStretch();
                            pSettingsLayout->addWidget(pBtn);
                            pSettingsLayout->addWidget(pLabApply);

                        }
                        pSettings->setLayout(pSettingsLayout);
                    }

                    m_pctrlglOutput = new QPlainTextEdit;
                    QFont fixedfnt(QFontDatabase::systemFont(QFontDatabase::FixedFont));
                    m_pctrlglOutput->setFont(fixedfnt);
                    m_pctrlglOutput->setReadOnly(true);

                    pTestLayout->addWidget(pSettings);
                    pTestLayout->addWidget(m_pctrlglOutput);
                    pTestLayout->setStretchFactor(pSettings, 1);
                    pTestLayout->setStretchFactor(m_pctrlglOutput, 2);
                }
                pTestContextBox->setLayout(pTestLayout);
            }
            m_pStackWt = new QStackedWidget;
            {
                m_pgl3dTestView = new gl3dTestView;
                m_pStackWt->addWidget(m_pgl3dTestView);
                connect(m_pgl3dTestView, SIGNAL(ready()), this, SLOT(onRenderWindowReady()));
            }
            pVSplitter->addWidget(pTestContextBox);
            pVSplitter->addWidget(m_pStackWt);
        }

        m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close | QDialogButtonBox::Apply, this);
        {
            connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
        }
        pMainLayout->addLayout(pCurrentDefaultLayout);
        pMainLayout->addWidget(pVSplitter);
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);
}


void OpenGlDlg::onButton(QAbstractButton *pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Close) == pButton)    reject();
    else if (m_pButtonBox->button(QDialogButtonBox::Apply) == pButton)    onApply();
}


void OpenGlDlg::onCreateContext()
{
    QSurfaceFormat fmt;

    int idx = m_pctrlVersionCbBox->currentIndex();
    if (idx < 0)  return;
    fmt.setVersion(versions[idx].major, versions[idx].minor);

    for (size_t i=0; i<sizeof(profiles)/sizeof(Profile); ++i)
    {
        if (static_cast<QRadioButton *>(m_profiles->itemAt(int(i))->widget())->isChecked()) {
            fmt.setProfile(profiles[i].profile);
            break;
        }
    }

    for (size_t i=0; i<sizeof(options)/sizeof(Option); ++i)
    {
        if (static_cast<QCheckBox *>(m_pOptionsLayout->itemAt(int(i))->widget())->isChecked()) {
            if (options[i].option)
                fmt.setOption(options[i].option);
        }
    }

    fmt.setRenderableType(QSurfaceFormat::OpenGL);

    m_pctrlglOutput->clear();

    m_pStackWt->removeWidget(m_pgl3dTestView);
    delete m_pgl3dTestView;

    m_pgl3dTestView = new gl3dTestView;
    connect(m_pgl3dTestView, SIGNAL(ready()), SLOT(onRenderWindowReady()));
//    connect(m_pgl3dView, &GLRenderWindow::error, this, &View3dTestDlg::renderWindowError);

    m_pgl3dTestView->setFormat(fmt);

    m_pStackWt->addWidget(m_pgl3dTestView);
    m_pStackWt->setCurrentWidget(m_pgl3dTestView);
    m_pgl3dTestView->repaint(); // force context initialization

    if (!m_pgl3dTestView->context())
    {
        m_pctrlglOutput->appendPlainText(tr("Failed to create context"));
        return;
    }
}


void OpenGlDlg::printFormat(const QSurfaceFormat &format, bool bFull)
{
    m_pctrlglOutput->appendPlainText(QString("   OpenGL version: %1.%2").arg(format.majorVersion()).arg(format.minorVersion()));

    for (size_t i=0; i<sizeof(profiles) / sizeof(Profile); ++i)
    {
        if (profiles[i].profile == format.profile())
        {
            m_pctrlglOutput->appendPlainText(QString("   Profile: %1").arg(QString::fromLatin1(profiles[i].str)));
            break;
        }
    }

    QString opts;
    for (size_t i=0; i<sizeof(options) / sizeof(Option); ++i)
    {
        if (format.testOption(options[i].option))
        {
            opts += QString::fromLatin1(options[i].str) + QStringLiteral(" ");
        }
    }
    m_pctrlglOutput->appendPlainText(QString("   Options: %1").arg(opts));

    if(bFull)
    {
        m_pctrlglOutput->appendPlainText(QString("   Depth buffer size   : %1").arg(QString::number(format.depthBufferSize())));
        m_pctrlglOutput->appendPlainText(QString("   Stencil buffer size : %1").arg(QString::number(format.stencilBufferSize())));
        m_pctrlglOutput->appendPlainText(QString("   Samples             : %1").arg(QString::number(format.samples())));
        m_pctrlglOutput->appendPlainText(QString("   Red buffer size     : %1").arg(QString::number(format.redBufferSize())));
        m_pctrlglOutput->appendPlainText(QString("   Green buffer size   : %1").arg(QString::number(format.greenBufferSize())));
        m_pctrlglOutput->appendPlainText(QString("   Blue buffer size    : %1").arg(QString::number(format.blueBufferSize())));
        m_pctrlglOutput->appendPlainText(QString("   Alpha buffer size   : %1").arg(QString::number(format.alphaBufferSize())));
        m_pctrlglOutput->appendPlainText(QString("   Swap interval       : %1").arg(QString::number(format.swapInterval())));
    }
}


void OpenGlDlg::onRenderWindowReady()
{
    QOpenGLContext *pContext = QOpenGLContext::currentContext();
    pContext = m_pgl3dTestView->context();

    QString vendor, renderer, version, glslVersion;
    const GLubyte *p=nullptr;

    if ((p = glGetString(GL_VENDOR)))
        vendor = QString::fromLatin1(reinterpret_cast<const char *>(p));
    if ((p = glGetString(GL_RENDERER)))
        renderer = QString::fromLatin1(reinterpret_cast<const char *>(p));
    if ((p = glGetString(GL_VERSION)))
        version = QString::fromLatin1(reinterpret_cast<const char *>(p));
    if ((p = glGetString(GL_SHADING_LANGUAGE_VERSION)))
        glslVersion = QString::fromLatin1(reinterpret_cast<const char *>(p));

    m_pctrlglOutput->appendPlainText(QString("*** Context information ***"));
    m_pctrlglOutput->appendPlainText(QString("   Vendor: %1").arg(vendor));
    m_pctrlglOutput->appendPlainText(QString("   Renderer: %1").arg(renderer));
    m_pctrlglOutput->appendPlainText(QString("   OpenGL version: %1").arg(version));
    m_pctrlglOutput->appendPlainText(QString("   GLSL version: %1").arg(glslVersion));

    m_pctrlglOutput->appendPlainText(QString("\n*** QSurfaceFormat::defaultFormat ***"));
    printFormat(QSurfaceFormat::defaultFormat());

    m_pctrlglOutput->appendPlainText(QString("\n*** QSurfaceFormat from context ***"));
    printFormat(pContext->format());

    m_pctrlglOutput->appendPlainText(QString("\n*** QSurfaceFormat from QOpenGLWidget ***"));
    printFormat(m_pgl3dTestView->format());

    m_pctrlglOutput->appendPlainText(QString("\n*** Qt build information ***"));
    const char *gltype[] = { "Desktop", "GLES 2", "GLES 1" };
    m_pctrlglOutput->appendPlainText(QString("   Qt OpenGL configuration  : %1")
                     .arg(QString::fromLatin1(gltype[QOpenGLContext::openGLModuleType()])));
    m_pctrlglOutput->appendPlainText(QString("   Qt OpenGL library handle : %1")
                     .arg(QString::number(qintptr(QOpenGLContext::openGLModuleHandle()), 16)));

    m_pctrlglOutput->moveCursor(QTextCursor::Start);

    m_pctrlglOutput->appendPlainText("\n*** OpenGL support: ***");
    QString strange;
    strange = "   Desktop OpenGL  : ";
    qApp->testAttribute(Qt::AA_UseDesktopOpenGL)? strange += "true" : strange+="false";
    m_pctrlglOutput->appendPlainText(strange);

    strange = "   OpenGL ES       : ";
    qApp->testAttribute(Qt::AA_UseOpenGLES)? strange += "true" : strange+="false";
    m_pctrlglOutput->appendPlainText(strange);

    strange = "   Software OpenGL : ";
    qApp->testAttribute(Qt::AA_UseSoftwareOpenGL)? strange += "true" : strange+="false";
    m_pctrlglOutput->appendPlainText(strange+"\n");

    m_pctrlglOutput->appendPlainText("*** Shaders ***");
    if(m_pgl3dTestView->bUsing120StyleShaders())
        m_pctrlglOutput->appendPlainText("   Using glsl 120 style shaders\n");
    else
        m_pctrlglOutput->appendPlainText("   Using glsl 330 style shaders\n");
}


void OpenGlDlg::onRenderWindowError(const QString &msg)
{
    m_pctrlglOutput->appendPlainText(QString("An error has occurred:\n%1").arg(msg));
}




void OpenGlDlg::hideEvent(QHideEvent *)
{
    s_Geometry = saveGeometry();
}


void OpenGlDlg::showEvent(QShowEvent *)
{
    restoreGeometry(s_Geometry);
}
