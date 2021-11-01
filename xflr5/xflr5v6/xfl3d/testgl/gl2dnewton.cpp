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
#include <xflwidgets/wt_globals.h>

int gl2dNewton::s_MaxIter = 32;
float gl2dNewton::s_Tolerance = 1.0e-6f;
QColor gl2dNewton::s_Colors[5] = {QColor(67,17,11), QColor(55,101,107), QColor(0,37,67), QColor(67,67,67), QColor(121,110,97)};


gl2dNewton::gl2dNewton(QWidget *pParent) : gl2dView(pParent)
{
    setWindowTitle("Newton");

    m_rectView = QRectF(-1.0, -1.0, 2.0, 2.0);

    m_bResetRoots = true;

    m_iHoveredRoot = m_iSelectedRoot = -1;

    m_Time = 0;
    for(int i=0; i<2*MAXROOTS; i++)
    {
        m_omega[i] = 2.0*QRandomGenerator::global()->bounded(1.0)-1.0;
    }

    m_locIters = m_locTolerance = -1;
    m_locViewTrans = -1;
    m_locViewScale = -1;
    m_locViewRatio = -1;
    m_locNRoots    = -1;
    for(int i=0; i<MAXROOTS; i++) m_locColor[i] = m_locRoot[i] = -1;

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
            QLabel *pLabTitle = new QLabel("Using OpenGL's fragment shader to compute<br> and plot a polynomial Newton fractal.");

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

            m_prb3roots = new QRadioButton("3 roots");
            m_prb5roots = new QRadioButton("5 roots");
            m_prb3roots->setChecked(true);
            connect(m_prb3roots, SIGNAL(clicked(bool)), SLOT(onNRoots()));
            connect(m_prb5roots, SIGNAL(clicked(bool)), SLOT(onNRoots()));

            m_pchShowRoots = new QCheckBox("Show roots");
            connect(m_pchShowRoots, SIGNAL(clicked(bool)), SLOT(update()));
            m_pchAnimateRoots = new QCheckBox("Animate roots");
            connect(m_pchAnimateRoots, SIGNAL(clicked(bool)), SLOT(onAnimate(bool)));

            pFrameLayout->addWidget(pLabTitle,         1, 1, 1, 2);
            pFrameLayout->addWidget(plabMaxIter,       2, 1);
            pFrameLayout->addWidget(m_pieMaxIter,      2, 2);
            pFrameLayout->addWidget(plabMaxLength,     3, 1);
            pFrameLayout->addWidget(m_pdeTolerance,    3, 2);
            pFrameLayout->addWidget(m_prb3roots,       4, 1);
            pFrameLayout->addWidget(m_prb5roots,       4, 2);
            pFrameLayout->addWidget(m_pchShowRoots,    5, 1);
            pFrameLayout->addWidget(m_pchAnimateRoots, 6, 1);

            pFrameLayout->addWidget(m_plabScale,       7, 1, 1, 2);
        }

        pFrame->setLayout(pFrameLayout);
        pFrame->setStyleSheet("QFrame{background-color: transparent;}");
        setWidgetStyle(pFrame, palette);
    }

    connect(&m_Timer, SIGNAL(timeout()), SLOT(onMoveRoots()));

    onNRoots();
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
    QString strange, vsrc, gsrc, fsrc;
    vsrc = ":/resources/shaders/newton/newton_VS.glsl";
    m_shadNewton.addShaderFromSourceFile(QOpenGLShader::Vertex, vsrc);
    if(m_shadNewton.log().length())
    {
        strange = QString::asprintf("%s", QString("Newton vertex shader log:"+m_shadNewton.log()).toStdString().c_str());
        trace(strange);
    }

    fsrc = ":/resources/shaders/newton/newton_FS.glsl";
    m_shadNewton.addShaderFromSourceFile(QOpenGLShader::Fragment, fsrc);
    if(m_shadNewton.log().length())
    {
        strange = QString::asprintf("%s", QString("Newton fragment shader log:"+m_shadNewton.log()).toStdString().c_str());
        trace(strange);
    }

    m_shadNewton.link();

    m_shadNewton.bind();
    {
        m_attrVertexPosition = m_shadNewton.attributeLocation("VertexPosition");
        m_locViewTrans = m_shadNewton.uniformLocation("ViewTrans");
        m_locViewScale = m_shadNewton.uniformLocation("ViewScale");
        m_locViewRatio = m_shadNewton.uniformLocation("ViewRatio");
        m_locNRoots    = m_shadNewton.uniformLocation("nroots");
        m_locTolerance = m_shadNewton.uniformLocation("tolerance");
        m_locIters     = m_shadNewton.uniformLocation("maxiters");
        for(int i=0; i<MAXROOTS; i++)
            m_locColor[i] = m_shadNewton.uniformLocation(QString::asprintf("color%d",i));
        for(int i=0; i<MAXROOTS; i++)
            m_locRoot[i]  = m_shadNewton.uniformLocation(QString::asprintf("root%d",i));
    }
    m_shadNewton.release();

    vsrc = ":/resources/shaders/point/point_VS.glsl";
    m_shadPoint.addShaderFromSourceFile(QOpenGLShader::Vertex, vsrc);
    if(m_shadPoint.log().length())
    {
        strange = QString::asprintf("%s", QString("Point vertex shader log:"+m_shadPoint.log()).toStdString().c_str());
        trace(strange);
    }

    gsrc = ":/resources/shaders/point/point_GS.glsl";
    m_shadPoint.addShaderFromSourceFile(QOpenGLShader::Geometry, gsrc);
    if(m_shadPoint.log().length())
    {
        strange = QString::asprintf("%s", QString("Point geometry shader log:"+m_shadPoint.log()).toStdString().c_str());
        trace(strange);
    }

    fsrc = ":/resources/shaders/point/point_FS.glsl";
    m_shadPoint.addShaderFromSourceFile(QOpenGLShader::Fragment, fsrc);
    if(m_shadPoint.log().length())
    {
        strange = QString::asprintf("%s", QString("Point fragment shader log:"+m_shadPoint.log()).toStdString().c_str());
        trace(strange);
    }

    m_shadPoint.link();
    m_shadPoint.bind();
    {
        m_locPoint.m_attrVertex = m_shadPoint.attributeLocation("vertexPosition_modelSpace");
        m_locPoint.m_State  = m_shadPoint.attributeLocation("PointState");
        m_locPoint.m_vmMatrix   = m_shadPoint.uniformLocation("vmMatrix");
        m_locPoint.m_pvmMatrix  = m_shadPoint.uniformLocation("pvmMatrix");
        m_locPoint.m_ClipPlane  = m_shadPoint.uniformLocation("clipPlane0");
        m_locPoint.m_UniColor   = m_shadPoint.uniformLocation("Color");
        m_locPoint.m_Thickness  = m_shadPoint.uniformLocation("Thickness");
        m_locPoint.m_Shape      = m_shadPoint.uniformLocation("Shape");
        m_locPoint.m_Viewport   = m_shadPoint.uniformLocation("Viewport");
        m_locPoint.m_Light      = m_shadPoint.uniformLocation("LightOn");
        m_locPoint.m_TwoSided   = m_shadPoint.uniformLocation("TwoSided");
    }
    m_shadPoint.release();

    makeQuad();
}


void gl2dNewton::paintGL()
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
    float ratio = float(width())/float(height());
    double w = m_rectView.width();
    QVector2D off((-m_ptOffset.x())/width()*w, m_ptOffset.y()/width()*w);

    m_shadNewton.bind();
    {
        int stride = 2;
        s_MaxIter   = m_pieMaxIter->value();
        s_Tolerance = m_pdeTolerance->value();
        m_shadNewton.setUniformValue(m_locIters,     s_MaxIter);
        m_shadNewton.setUniformValue(m_locTolerance, s_Tolerance);
        m_shadNewton.setUniformValue(m_locViewRatio, ratio);

        int nroots = m_prb3roots->isChecked() ? 3 : 5;
        m_shadNewton.setUniformValue(m_locNRoots, nroots);
        for(int i=0; i<nroots; i++)
        {
            m_shadNewton.setUniformValue(m_locColor[i], s_Colors[i]);
            m_shadNewton.setUniformValue(m_locRoot[i], m_Root[i]);
        }

        m_shadNewton.setUniformValue(m_locViewTrans, off);
        m_shadNewton.setUniformValue(m_locViewScale, m_Scale);

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

    if(m_pchShowRoots->isChecked())
    {

        if(m_bResetRoots)
        {
            int nroots = m_prb3roots->isChecked() ? 3 : 5;
            int buffersize = nroots*4;
            QVector<float> pts(buffersize);
            int iv = 0;
            for(int is=0; is<nroots; is++)
            {
                pts[iv++] = m_Root[is].x();
                pts[iv++] = m_Root[is].y();
                pts[iv++] = 0.0f;
                if (is==m_iSelectedRoot || is==m_iHoveredRoot)
                    pts[iv++] = 1.0f;
                else
                    pts[iv++] = -1.0f; // invalid state, use uniform
            }

            if(m_vboRoots.isCreated()) m_vboRoots.destroy();
            m_vboRoots.create();
            m_vboRoots.bind();
            m_vboRoots.allocate(pts.data(), buffersize * int(sizeof(GLfloat)));
            m_vboRoots.release();

            m_bResetRoots = false;
        }

        QMatrix4x4 m_matModel;
        QMatrix4x4 m_matView;
        QMatrix4x4 m_matProj;
        float s = 1.0;
        int width  = geometry().width();
        int height = geometry().height();
        m_matProj.ortho(-s,s,-(height*s)/width,(height*s)/width,-1.0e3*s,1.0e3*s);

        m_matView.scale(m_Scale, m_Scale, m_Scale);
        m_matView.translate(-off.x(), -off.y(), 0.0f);

        QMatrix4x4 vmMat(m_matView*m_matModel);
        QMatrix4x4 pvmMat(m_matProj*vmMat);


        m_shadPoint.bind();
        {
            float m_ClipPlanePos(500.0);
            m_shadPoint.setUniformValue(m_locPoint.m_ClipPlane, m_ClipPlanePos);
            m_shadPoint.setUniformValue(m_locPoint.m_Viewport, QVector2D(float(m_GLViewRect.width()), float(m_GLViewRect.height())));
    //        m_shadPoint.setUniformValue(m_locPoint.m_Viewport, QVector2D(float(m_rectView.width()), float(m_rectView.height())));
            m_shadPoint.setUniformValue(m_locPoint.m_vmMatrix,  vmMat);
            m_shadPoint.setUniformValue(m_locPoint.m_pvmMatrix, pvmMat);
        }
        m_shadPoint.release();

        paintPoints(m_vboRoots, 1.0, 0, false, Qt::white, 4);
    }

    m_plabScale->setText(QString::asprintf("Scale = %g", m_Scale));

    if (!m_bInitialized)
    {
        m_bInitialized = true;
        emit ready2d();
    }
}


void gl2dNewton::paintPoints(QOpenGLBuffer &vbo, float width, int iShape, bool bLight, QColor const &clr, int stride)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
    m_shadPoint.bind();
    {
        // iShape 0: Pentagon, 1: Icosahedron, 2: Cube
        m_shadPoint.setUniformValue(m_locPoint.m_Shape, iShape);
        m_shadPoint.setUniformValue(m_locPoint.m_UniColor, clr); // only used if vertex state is invalid 0< or >1
        m_shadPoint.setUniformValue(m_locPoint.m_Thickness, width);
        if(bLight)m_shadPoint.setUniformValue(m_locPoint.m_Light, 1);
        else      m_shadPoint.setUniformValue(m_locPoint.m_Light, 0);

        if(vbo.bind())
        {
            m_shadPoint.enableAttributeArray(m_locPoint.m_attrVertex);
            m_shadPoint.setAttributeBuffer(m_locPoint.m_attrVertex, GL_FLOAT, 0, 3, stride*sizeof(float));
            m_shadPoint.enableAttributeArray(m_locPoint.m_State);
            m_shadPoint.setAttributeBuffer(m_locPoint.m_State, GL_FLOAT, 3*sizeof(float), 1, stride*sizeof(float));
            int npts = vbo.size()/stride/int(sizeof(float));
            glDrawArrays(GL_POINTS, 0, npts);// 4 vertices defined but only 3 are used
            m_shadPoint.disableAttributeArray(m_locPoint.m_attrVertex);
            m_shadPoint.disableAttributeArray(m_locPoint.m_State);
        }
        vbo.release();
    }
    m_shadPoint.release();
}


void gl2dNewton::onNRoots()
{
    if(m_prb3roots->isChecked())
    {
        for(int i=0; i<3; i++)
        {
            m_amp0[i] = 1.0f;
            m_phi0[i] = 2.0f*PIf/3.0f * float(i);
            m_Root[i].setX(m_amp0[i]*cos(m_phi0[i]));
            m_Root[i].setY(m_amp0[i]*sin(m_phi0[i]));
        }
    }
    else if(m_prb5roots->isChecked())
    {
        for(int i=0; i<5; i++)
        {
            m_amp0[i] = 1.0f;
            m_phi0[i] = 2.0f*PIf/5.0f * float(i);
            m_Root[i].setX(m_amp0[i]*cos(m_phi0[i]));
            m_Root[i].setY(m_amp0[i]*sin(m_phi0[i]));
        }
    }

    m_Time = 0;
    m_bResetRoots = true;
    update();
}


void gl2dNewton::onAnimate(bool bAnimate)
{
    if(bAnimate)
    {
        if(!m_Timer.isActive())
        {
            m_Timer.start(17);
        }
    }
    else
        m_Timer.stop();
}


void gl2dNewton::mousePressEvent(QMouseEvent *pEvent)
{
    QVector2D pt;
    screenToWorld(pEvent->pos(), pt);

    int nroots = m_prb3roots->isChecked() ? 3 : 5;
    for(int i=0; i<nroots; i++)
    {
        if(pt.distanceToPoint(m_Root[i])<0.025/m_Scale)
        {
            m_Timer.stop();
//            m_pchAnimateRoots->setChecked(false);
            m_iSelectedRoot  = i;
            return;
        }
    }

    gl2dView::mousePressEvent(pEvent);
}


void gl2dNewton::mouseMoveEvent(QMouseEvent *pEvent)
{
    QVector2D pt;
    screenToWorld(pEvent->pos(), pt);
    int nroots = m_prb3roots->isChecked() ? 3 : 5;

    if(m_iSelectedRoot>=0 && m_iSelectedRoot<nroots)
    {
        m_Root[m_iSelectedRoot] = pt;
        for(int i=0; i<nroots; i++)
        {
            m_amp0[i] = sqrt(m_Root[i].x()*m_Root[i].x()+m_Root[i].y()*m_Root[i].y());
            m_phi0[i] = atan2f(m_Root[i].y(), m_Root[i].x());
        }
        m_Time = 0;
        m_bResetRoots = true;
        update();
        return;
    }
    else
    {
        for(int i=0; i<nroots; i++)
        {
            if(pt.distanceToPoint(m_Root[i])<0.025/m_Scale)
            {
                m_iHoveredRoot = i;
                m_bResetRoots = true;
                update();
                return;
            }
        }
        m_iHoveredRoot = -1;
        m_bResetRoots = true;
        if(!m_pchAnimateRoots->isChecked()) update();
    }
    gl2dView::mouseMoveEvent(pEvent);
}


void gl2dNewton::mouseReleaseEvent(QMouseEvent *pEvent)
{
    if(m_iSelectedRoot>=0 || m_iHoveredRoot>=0)
    {
        m_iSelectedRoot = m_iHoveredRoot = -1;
        m_bResetRoots = true;
        m_Time = 0;
        update();
        QApplication::restoreOverrideCursor();

        if(m_pchAnimateRoots->isChecked()) m_Timer.start(17);
        return;
    }
    gl2dView::mouseReleaseEvent(pEvent);
}


void gl2dNewton::onMoveRoots()
{
    float t = float(m_Time)/1000.0f;

    int nroots = m_prb3roots->isChecked() ? 3 : 5;
    for(int i=0; i<nroots; i++)
    {
        float amp = m_amp0[i] - (1.0f - cosf(m_omega[2*i]*t*6.0*PIf))/2.0f;
        float phi = m_phi0[i] + 2.0f*PIf*sinf(m_omega[2*i+1]*t);

        m_Root[i].setX(amp*cos(phi));
        m_Root[i].setY(amp*sin(phi));
    }

    m_Time++;

    m_bResetRoots = true;
    update();
}








