/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois
    GNU General Public License v3

*****************************************************************************/

#include <QApplication>
#include <QRandomGenerator>
#include <QGridLayout>
#include <QLabel>

#include "gl2dnewton.h"

#include <xflcore/displayoptions.h>
#include <xflcore/trace.h>
#include <xfl3d/views/gl3dview.h> // for the static variables

#include <xflwidgets/customwts/intedit.h>
#include <xflwidgets/customwts/doubleedit.h>
#include <xflwidgets/color/colormenubtn.h>
#include <xflwidgets/wt_globals.h>

int gl2dNewton::s_MaxIter = 32;
float gl2dNewton::s_Tolerance = 1.e-6;
QColor gl2dNewton::s_Colors[3] = {QColor(67,17,11), QColor(55,101,107), QColor(0,37,67)};

gl2dNewton::gl2dNewton(QWidget *pParent) : gl2dView(pParent)
{
    setWindowTitle("Newton");

    m_rectView = QRectF(-1.0, -1.0, 2.0, 2.0);

    for(int i=0; i<3; i++)
    {
        double amp = 1.0;
        double phase = 2.0*PI/3 * double(i);
        m_Root[i].setX(amp*cos(phase));
        m_Root[i].setY(amp*sin(phase));
    }

    m_Time = 0;
    for(int i=0; i<6; i++)
    {
        m_omega[i] = 2.0*QRandomGenerator::global()->bounded(1.0)-1.0;
    }

    m_locIters = m_locTolerance = -1;
    m_locViewTrans = -1;
    m_locViewScale = -1;
    m_locViewRatio = -1;
    m_locColor[0] = m_locColor[1] = m_locColor[2] = -1;

    QFrame *pFrame = new QFrame(this);
    {
        QPalette palette;
        palette.setColor(QPalette::WindowText, gl3dView::textColor());
        palette.setColor(QPalette::Text,  gl3dView::textColor());
        QColor clr = gl3dView::backColor();
        clr.setAlpha(0);
        palette.setColor(QPalette::Window, clr);
        palette.setColor(QPalette::Base, clr);
        pFrame->setCursor(Qt::ArrowCursor);

        pFrame->setFrameShape(QFrame::NoFrame);
        pFrame->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        QGridLayout*pFrameLayout = new QGridLayout;
        {
            QLabel *pLabTitle = new QLabel("Using OpenGL's fragment shader<br>to compute and plot a Newton fractal.");

            QLabel *plabMaxIter = new QLabel("Max. iterations:");
            m_pieMaxIter = new IntEdit(s_MaxIter);
            m_pieMaxIter->setToolTip("The number of iterations before bailing out.");

            QLabel *plabMaxLength= new QLabel("Tolerance:");
            m_pdeTolerance = new DoubleEdit(s_Tolerance);
            m_pdeTolerance->setToolTip("The escape amplitude of z.");
            connect(m_pieMaxIter,   SIGNAL(valueChanged()), SLOT(update()));
            connect(m_pdeTolerance, SIGNAL(valueChanged()), SLOT(update()));

            m_plabScale = new QLabel();
            m_plabScale->setFont(DisplayOptions::textFont());

            m_pchAnimate = new QCheckBox("Animate  roots");
            connect(m_pchAnimate, SIGNAL(clicked(bool)), SLOT(onAnimate(bool)));

            pFrameLayout->addWidget(pLabTitle,       1, 1, 1, 2);
            pFrameLayout->addWidget(plabMaxIter,     2, 1);
            pFrameLayout->addWidget(m_pieMaxIter,    2, 2);
            pFrameLayout->addWidget(plabMaxLength,   3, 1);
            pFrameLayout->addWidget(m_pdeTolerance,  3, 2);
            pFrameLayout->addWidget(m_pchAnimate,    4, 1);

            pFrameLayout->addWidget(m_plabScale,     5, 1, 1, 2);
        }

        pFrame->setLayout(pFrameLayout);
        pFrame->setStyleSheet("QFrame{background-color: transparent;}");
        setWidgetStyle(pFrame, palette);
    }

    connect(&m_Timer, SIGNAL(timeout()), SLOT(onMoveRoots()));
}


void gl2dNewton::onMoveRoots()
{
    double t = double(m_Time)/2000.0;

    for(int i=0; i<3; i++)
    {
        double amp = 1.0                  - (0.5 + 0.5 * sin(m_omega[2*i]*t - PI/2.0));
        double phi = 2.0*PI/3 * double(i) + 2.0*PI*sin(m_omega[2*i+1]*t);
        m_Root[i].setX(amp*cos(phi));
        m_Root[i].setY(amp*sin(phi));
    }

//qDebug("%4d  %7f  %7f  %7f  %7f  %7f  %7f", m_Time, s_Root[0].x(), s_Root[0].y(), s_Root[1].x(), s_Root[1].y(), s_Root[2].x(), s_Root[2].y());
    m_Time++;

    update();
}


void gl2dNewton::onAnimate(bool bAnimate)
{
    if(bAnimate)
    {
        if(!m_Timer.isActive())
        {
            m_Timer.start(16);
        }
    }
    else
        m_Timer.stop();
}


void gl2dNewton::loadSettings(QSettings &settings)
{
    settings.beginGroup("gl2dNewton");
    {
        s_MaxIter   = settings.value("MaxIters", s_MaxIter).toInt();
        s_Tolerance = settings.value("MaxLength", s_Tolerance).toFloat();
    }
    settings.endGroup();
}


void gl2dNewton::saveSettings(QSettings &settings)
{
    settings.beginGroup("gl2dNewton");
    {
        settings.setValue("MaxIters", s_MaxIter);
        settings.setValue("MaxLength", s_Tolerance);
    }
    settings.endGroup();
}


void gl2dNewton::initializeGL()
{
    QString strange, vsrc, fsrc;
    vsrc = ":/resources/shaders/newton/newton_VS.glsl";
    m_shadNewton.addShaderFromSourceFile(QOpenGLShader::Vertex, vsrc);
    if(m_shadNewton.log().length())
    {
        strange = QString::asprintf("%s", QString("Newton vertex shader log:"+m_shadNewton.log()).toStdString().c_str());
        Trace(strange);
    }

    fsrc = ":/resources/shaders/newton/newton_FS.glsl";
    m_shadNewton.addShaderFromSourceFile(QOpenGLShader::Fragment, fsrc);
    if(m_shadNewton.log().length())
    {
        strange = QString::asprintf("%s", QString("Newton fragment shader log:"+m_shadNewton.log()).toStdString().c_str());
        Trace(strange);
    }

    m_shadNewton.link();

    m_shadNewton.bind();
    {
        m_attrVertexPosition = m_shadNewton.attributeLocation("VertexPosition");
        m_locViewTrans = m_shadNewton.uniformLocation("ViewTrans");
        m_locViewScale = m_shadNewton.uniformLocation("ViewScale");
        m_locViewRatio = m_shadNewton.uniformLocation("ViewRatio");

        m_locTolerance = m_shadNewton.uniformLocation("tolerance");
        m_locIters     = m_shadNewton.uniformLocation("maxiters");
        for(int i=0; i<3; i++)
            m_locColor[i] = m_shadNewton.uniformLocation(QString::asprintf("color%d",i));
        for(int i=0; i<3; i++)
            m_locRoot[i]  = m_shadNewton.uniformLocation(QString::asprintf("root%d",i));
    }
    m_shadNewton.release();

    makeQuad();
}


void gl2dNewton::paintGL()
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
    m_shadNewton.bind();
    {
        int stride = 2;
        s_MaxIter   = m_pieMaxIter->value();
        s_Tolerance = m_pdeTolerance->value();
        m_shadNewton.setUniformValue(m_locIters,     s_MaxIter);
        m_shadNewton.setUniformValue(m_locTolerance, s_Tolerance);
        m_shadNewton.setUniformValue(m_locViewRatio, float(width())/float(height()));
        for(int i=0; i<3; i++)
            m_shadNewton.setUniformValue(m_locColor[i], s_Colors[i]);

        m_shadNewton.setUniformValue(m_locRoot[0], m_Root[0]);
        m_shadNewton.setUniformValue(m_locRoot[1], m_Root[1]);
        m_shadNewton.setUniformValue(m_locRoot[2], m_Root[2]);

        double w = m_rectView.width();
        QVector2D off(-m_ptOffset.x()/width()*w, m_ptOffset.y()/width()*w);
        m_shadNewton.setUniformValue(m_locViewTrans,  off);
        m_shadNewton.setUniformValue(m_locViewScale,  m_Scale);

        m_vboQuad.bind();
        {
            m_shadNewton.enableAttributeArray(m_attrVertexPosition);
            m_shadNewton.setAttributeBuffer(m_attrVertexPosition, GL_FLOAT, 0, 2, stride*sizeof(GLfloat));

            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glDisable(GL_CULL_FACE);

            int nvtx = m_vboQuad.size()/stride/int(sizeof(float));
            glDrawArrays(GL_TRIANGLE_STRIP, 0, nvtx);

            m_shadNewton.disableAttributeArray(m_attrVertexPosition);
        }
        m_vboQuad.release();
    }
    m_shadNewton.release();

    m_plabScale->setText(QString::asprintf("Scale = %g", m_Scale));

    if (!m_bInitialized)
    {
        m_bInitialized = true;
        emit ready2d();
    }
}




