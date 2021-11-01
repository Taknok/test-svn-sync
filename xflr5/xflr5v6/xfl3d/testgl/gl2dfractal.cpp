/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois
    GNU General Public License v3

*****************************************************************************/

#include <QApplication>
#include <QGridLayout>
#include <QLabel>

#include "gl2dfractal.h"

#include <xflcore/displayoptions.h>
#include <xflcore/trace.h>
#include <xfl3d/views/gl3dview.h> // for the static variables
#include <xflwidgets/customwts/intedit.h>
#include <xflwidgets/customwts/doubleedit.h>
#include <xflwidgets/wt_globals.h>

int gl2dFractal::s_MaxIter = 64;
float gl2dFractal::s_MaxLength = 10;


gl2dFractal::gl2dFractal(QWidget *pParent) : gl2dView(pParent)
{
    setWindowTitle("Mandelbrot");

//    m_rectView = QRectF(-2.5, -1.0, 3.5, 2.0);
    m_rectView = QRectF(-1.0, -1.0, 2.0, 2.0);
    m_Scale = 0.5f;

    m_locIters = m_locLength = -1;
    m_locViewTrans = -1;
    m_locViewScale = -1;
    m_locViewRatio = -1;

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
            QLabel *pLabTitle = new QLabel("Using OpenGL's fragment shader<br>to compute and plot the Mandelbrot set");

            QLabel *plabMaxIter = new QLabel("Max. iterations:");
            m_pieMaxIter = new IntEdit(s_MaxIter);
            m_pieMaxIter->setToolTip("The number of iterations before bailing out.");

            QLabel *plabMaxLength= new QLabel("Max. length:");
            m_pdeMaxLength = new DoubleEdit(s_MaxLength);
            m_pdeMaxLength->setToolTip("The escape amplitude of z.");

            m_plabScale = new QLabel();
            m_plabScale->setFont(DisplayOptions::textFont());

            connect(m_pieMaxIter,   SIGNAL(valueChanged()), SLOT(update()));
            connect(m_pdeMaxLength, SIGNAL(valueChanged()), SLOT(update()));

            pFrameLayout->addWidget(pLabTitle,       1, 1, 1, 2);
            pFrameLayout->addWidget(plabMaxIter,     2, 1);
            pFrameLayout->addWidget(m_pieMaxIter,    2, 2);
            pFrameLayout->addWidget(plabMaxLength,   3, 1);
            pFrameLayout->addWidget(m_pdeMaxLength,  3, 2);
            pFrameLayout->addWidget(m_plabScale,     4, 1, 1, 2);
        }

        pFrame->setLayout(pFrameLayout);
        pFrame->setStyleSheet("QFrame{background-color: transparent;}");
        setWidgetStyle(pFrame, palette);
    }
}


void gl2dFractal::loadSettings(QSettings &settings)
{
    settings.beginGroup("gl2dFractal");
    {
        s_MaxIter   = settings.value("MaxIters", s_MaxIter).toInt();
        s_MaxLength = settings.value("MaxLength", s_MaxLength).toFloat();
    }
    settings.endGroup();
}


void gl2dFractal::saveSettings(QSettings &settings)
{
    settings.beginGroup("gl2dFractal");
    {
        settings.setValue("MaxIters", s_MaxIter);
        settings.setValue("MaxLength", s_MaxLength);
    }
    settings.endGroup();
}


void gl2dFractal::initializeGL()
{
    QString strange, vsrc, fsrc;
    vsrc = ":/resources/shaders/fractal/fractal_VS.glsl";
    m_shadFrac.addShaderFromSourceFile(QOpenGLShader::Vertex, vsrc);
    if(m_shadFrac.log().length())
    {
        strange = QString::asprintf("%s", QString("Frac vertex shader log:"+m_shadFrac.log()).toStdString().c_str());
        trace(strange);
    }

    fsrc = ":/resources/shaders/fractal/fractal_FS.glsl";
    m_shadFrac.addShaderFromSourceFile(QOpenGLShader::Fragment, fsrc);
    if(m_shadFrac.log().length())
    {
        strange = QString::asprintf("%s", QString("Frac fragment shader log:"+m_shadFrac.log()).toStdString().c_str());
        trace(strange);
    }

    m_shadFrac.link();
    m_shadFrac.bind();
    {
        m_attrVertexPosition = m_shadFrac.attributeLocation("VertexPosition");

        m_locLength    = m_shadFrac.uniformLocation("maxlength");
        m_locIters     = m_shadFrac.uniformLocation("maxiters");
        m_locViewTrans = m_shadFrac.uniformLocation("ViewTrans");
        m_locViewScale = m_shadFrac.uniformLocation("ViewScale");
        m_locViewRatio = m_shadFrac.uniformLocation("ViewRatio");
    }
    m_shadFrac.release();

    makeQuad();
}


void gl2dFractal::paintGL()
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
    m_shadFrac.bind();
    {
        int stride = 2;
        s_MaxIter   = m_pieMaxIter->value();
        s_MaxLength = m_pdeMaxLength->value();
        m_shadFrac.setUniformValue(m_locIters,     s_MaxIter);
        m_shadFrac.setUniformValue(m_locLength,    s_MaxLength);
        m_shadFrac.setUniformValue(m_locViewRatio, float(width())/float(height()));

        double w = m_rectView.width();
        QVector2D off(-m_ptOffset.x()/width()*w, m_ptOffset.y()/width()*w);
        m_shadFrac.setUniformValue(m_locViewTrans,  off);
        m_shadFrac.setUniformValue(m_locViewScale,  m_Scale);

        m_vboQuad.bind();
        {
            m_shadFrac.enableAttributeArray(m_attrVertexPosition);
            m_shadFrac.setAttributeBuffer(m_attrVertexPosition, GL_FLOAT, 0, 2, stride*sizeof(GLfloat));

            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glDisable(GL_CULL_FACE);

            int nvtx = m_vboQuad.size()/stride/int(sizeof(float));
            glDrawArrays(GL_TRIANGLE_STRIP, 0, nvtx);

            m_shadFrac.disableAttributeArray(m_attrVertexPosition);
        }
        m_vboQuad.release();
    }
    m_shadFrac.release();
    m_plabScale->setText(QString::asprintf("Scale = %g", m_Scale));

    if (!m_bInitialized)
    {
        m_bInitialized = true;
        emit ready2d();
    }
}


