/****************************************************************************

    gl3dView Class
    Copyright (C) André Deperrois

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

#include <QApplication>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>
#include <QOpenGLPaintDevice>
#include <QMouseEvent>
#include <QFontDatabase>

#include "gl3dview.h"
#include <xfl3d/gl_globals.h>
#include <xfl3d/controls/gllightdlg.h>
#include <xfl3d/controls/w3dprefsdlg.h>
#include <xflcore/displayoptions.h>
#include <xflcore/trace.h>
#include <xflcore/xflcore.h>

//ArcBall parameters
#define NUMANGLES     57
#define NUMCIRCLES     6
#define NUMPERIM     131
#define NUMARCPOINTS  11


FontStruct gl3dView::s_glFontStruct;
Light gl3dView::s_Light;

int gl3dView::s_OpenGLMajor = 2;
int gl3dView::s_OpenGLMinor = 1;

QColor gl3dView::s_TextColor = Qt::white;
QColor gl3dView::s_BackgroundColor = QColor(5,10,17);

bool gl3dView::s_bSpinAnimation = true;
double gl3dView::s_SpinDamping = 0.01;

bool gl3dView::s_bAnimateTransitions = true;
int gl3dView::s_AnimationTime = 500; //ms

gl3dView::gl3dView(QWidget *pParent) : QOpenGLWidget(pParent)
{
    setAutoFillBackground(false);
    setMouseTracking(true);
    setCursor(Qt::CrossCursor);

    m_iTransitionInc = 0;

    m_bShowLight = false;
    m_bArcball = m_bCrossPoint = false;
    m_bAxes = true;
    m_bUse120StyleShaders = true;

    m_glViewportTrans.x  = 0.0;
    m_glViewportTrans.y  = 0.0;
    m_glViewportTrans.z  = 0.0;

    m_glScalef = m_glScalefRef = 1.0f;
    m_glScaleIncrement = 0.0;
    m_ClipPlanePos  = 500.0;

    m_bTrans = false;

    m_LastPoint.setX(0);
    m_LastPoint.setY(0);

    m_PixTextOverlay = QPixmap(107, 97);
    m_PixTextOverlay.fill(Qt::transparent);

    memset(m_MatOut, 0, 16*sizeof(double));

    ANIMATIONFRAMES = 30;
    m_bHasMouseMoved = false;
    m_bDynTranslation = m_bDynRotation = m_bDynScaling = false;
    connect(&m_DynTimer, SIGNAL(timeout()), SLOT(onDynamicIncrement()));

    m_ZoomFactor = 1.0;
    m_iTimerInc=0;
}


void gl3dView::printFormat(QSurfaceFormat const &ctxtFormat, QString &log)
{
    QString strange;
    log.clear();
    log += "-------Testing OpenGL support--------\n";
    strange = QString::asprintf("Loaded version= %d.%d\n", ctxtFormat.majorVersion(),ctxtFormat.minorVersion());
    log +=strange;
    QString vendor, renderer, version, glslVersion;
    const GLubyte *p;
    if ((p = glGetString(GL_VENDOR)))
        vendor = QString::fromLatin1(reinterpret_cast<const char *>(p));
    if ((p = glGetString(GL_RENDERER)))
        renderer = QString::fromLatin1(reinterpret_cast<const char *>(p));
    if ((p = glGetString(GL_VERSION)))
        version = QString::fromLatin1(reinterpret_cast<const char *>(p));
    if ((p = glGetString(GL_SHADING_LANGUAGE_VERSION)))
        glslVersion = QString::fromLatin1(reinterpret_cast<const char *>(p));

    log += "*** Context information ***\n";
    log += QString("   Vendor: %1\n").arg(vendor).toStdString().c_str();
    log += QString("   Renderer: %1\n").arg(renderer).toStdString().c_str();
    log += QString("   OpenGL version: %1\n").arg(version).toStdString().c_str();
    log += QString("   GLSL version: %1\n").arg(glslVersion).toStdString().c_str();

    if(ctxtFormat.testOption(QSurfaceFormat::DeprecatedFunctions))  log += "Using deprecated functions\n";
    if(ctxtFormat.testOption(QSurfaceFormat::DebugContext))         log += "Using debug context\n";
    if(ctxtFormat.testOption(QSurfaceFormat::StereoBuffers))        log += "Using Stereo buffers\n";

    switch (ctxtFormat.profile()) {
    case QSurfaceFormat::NoProfile:
        log += "No Profile\n";
        break;
    case QSurfaceFormat::CoreProfile:
        log += ("Core Profile\n");
        break;
    case QSurfaceFormat::CompatibilityProfile:
        log += ("Compatibility Profile\n");
        break;
    }
    switch (ctxtFormat.renderableType())
    {
    case QSurfaceFormat::DefaultRenderableType:
        log += ("DefaultRenderableType: The default, unspecified rendering method\n");
        break;
    case QSurfaceFormat::OpenGL:
        log += ("OpenGL: Desktop OpenGL rendering\n");
        break;
    case QSurfaceFormat::OpenGLES:
        log += ("OpenGLES: OpenGL ES 2.0 rendering\n");
        break;
    case QSurfaceFormat::OpenVG:
        log += ("OpenVG: Open Vector Graphics rendering\n");
        break;
    }
    log += "-------  Done Testing OpenGL --------";
    log += "\n\n";
}


gl3dView::~gl3dView()
{
    m_vboArcBall.destroy();
    m_vboArcPoint.destroy();
    m_vboSphere.destroy();
}


/**
* The user has modified the position of the clip plane in the 3D view
*@param pos the new z position in viewport coordinates of the clipping plane
*/
void gl3dView::onClipPlane(int pos)
{
    float coef = 4.0;
    float planepos =  pos/100.0f;
    m_ClipPlanePos = 5.0f*sinh(planepos*coef)/sinh(coef);
    update();
}


void gl3dView::on3dIso()
{
    stopDynamicTimer();
    m_QuatStart = m_ArcBall.m_Quat;

    Quaternion qti;
    qti.fromEulerAngles(ROLL, PITCH, YAW);
    Quaternion qtyaw(-30.0, Vector3d(0.0,0.0,1.0));
    m_QuatEnd = qti*qtyaw;
    m_ArcBall.setQuat(m_QuatEnd);

    startRotationTimer();
    emit viewModified();
}


void gl3dView::on3dFlip()
{
    stopDynamicTimer();
    m_QuatStart = m_ArcBall.m_Quat;

    Quaternion qtflip(180.0, Vector3d(1.0,0.0,0.0));
    float ab_flip[16];
    memset(ab_flip, 0, 16*sizeof(float));

    m_QuatEnd = m_QuatStart*qtflip;
    m_ArcBall.setQuat(m_QuatEnd);

//    memcpy(m_ArcBall.m_MatCurrent, ab_new, 16*sizeof(float));

    startRotationTimer();
    emit viewModified();
}


void gl3dView::on3dTop()
{
    stopDynamicTimer();
    m_QuatStart = m_ArcBall.m_Quat;

    m_QuatEnd.set(sqrt(2.0)/2.0, 0.0, 0.0, -sqrt(2.0)/2.0);
    m_ArcBall.setQuat(m_QuatEnd);

    startRotationTimer();
    emit viewModified();
}


void gl3dView::on3dLeft()
{
    stopDynamicTimer();
    m_QuatStart = m_ArcBall.m_Quat;

    m_QuatEnd.set(sqrt(2.0)/2.0, -sqrt(2.0)/2.0, 0.0, 0.0);    // rotate by 90° around x
    m_ArcBall.setQuat(m_QuatEnd);

    startRotationTimer();
    emit viewModified();
}


void gl3dView::on3dFront()
{
    stopDynamicTimer();
    m_QuatStart = m_ArcBall.m_Quat;

    Quaternion Qt1(sqrt(2.0)/2.0, 0.0,           -sqrt(2.0)/2.0, 0.0);// rotate by 90° around y
    Quaternion Qt2(sqrt(2.0)/2.0, -sqrt(2.0)/2.0, 0.0,           0.0);// rotate by 90° around x

    Quaternion qtflip(180.0, Vector3d(0.0,0.0,1.0));
    m_QuatEnd = Qt1*Qt2*qtflip;
    m_ArcBall.setQuat(m_QuatEnd);

    startRotationTimer();
    emit viewModified();
}


void gl3dView::onAxes(bool bChecked)
{
    m_bAxes = bChecked;
    update();
}


void gl3dView::mousePressEvent(QMouseEvent *pEvent)
{
    QPoint point(pEvent->pos().x(), pEvent->pos().y());

    stopDynamicTimer();

    if(m_iTimerInc>0)
    {
        // interrupt animation and return
        m_TransitionTimer.stop();
        m_iTimerInc = 0;
    }

    bool bCtrl = false;
    if(pEvent->modifiers() & Qt::ControlModifier) bCtrl =true;

    m_bHasMouseMoved = false;

    if (pEvent->buttons() & Qt::MiddleButton)
    {
        m_bArcball = true;
        Vector3d real;
        QPoint pt(pEvent->pos().x(), pEvent->pos().y());
        screenToViewport(pt, real);
        m_ArcBall.start(real.x, real.y);
        m_bCrossPoint = true;

        reset3dRotationCenter();
        update();
    }
    else if (pEvent->buttons() & Qt::LeftButton)
    {
        Vector3d real;
        QPoint pt(point.x(), point.y());
        screenToViewport(pt, real);
        m_ArcBall.start(real.x, real.y);
        reset3dRotationCenter();
        if (!bCtrl)
        {
            m_bTrans = true;
            QApplication::setOverrideCursor(Qt::ClosedHandCursor);
        }
        else
        {
            m_bTrans=false;
            m_bArcball = true;
            m_bCrossPoint = true;
        }
        update();
    }

    m_LastPoint = point;
    m_PressedPoint = point;

    m_MoveTime.restart();
}


/**
*Overrides the mouseDoubleClickEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void gl3dView::mouseDoubleClickEvent(QMouseEvent *pEvent)
{
    set3dRotationCenter(pEvent->pos());
}


void gl3dView::mouseMoveEvent(QMouseEvent *pEvent)
{
    QPoint point(pEvent->pos().x(), pEvent->pos().y());
    Vector3d Real;

    QPoint Delta(point.x()-m_LastPoint.x(), point.y()-m_LastPoint.y());
    screenToViewport(point, Real);

    if(std::abs(Delta.x())>10 || std::abs(Delta.y())>10)
        m_bHasMouseMoved = true;

    bool bCtrl = false;

    if (pEvent->modifiers() & Qt::ControlModifier) bCtrl =true;
    if (pEvent->buttons()   & Qt::LeftButton)
    {
        if(bCtrl)
        {
            //rotate
            m_ArcBall.move(Real.x, Real.y);
            update();
        }
        else if(m_bTrans)
        {
            //translate
            int side = std::max(geometry().width(), geometry().height());

            m_glViewportTrans.x += Delta.x()*2.0/double(m_glScalef)/side;
            m_glViewportTrans.y += Delta.y()*2.0/double(m_glScalef)/side;

            m_glRotCenter.x = m_MatOut[0]*(m_glViewportTrans.x) + m_MatOut[1]*(-m_glViewportTrans.y) + m_MatOut[2] *m_glViewportTrans.z;
            m_glRotCenter.y = m_MatOut[4]*(m_glViewportTrans.x) + m_MatOut[5]*(-m_glViewportTrans.y) + m_MatOut[6] *m_glViewportTrans.z;
            m_glRotCenter.z = m_MatOut[8]*(m_glViewportTrans.x) + m_MatOut[9]*(-m_glViewportTrans.y) + m_MatOut[10]*m_glViewportTrans.z;

            update();
        }
    }
    else if (pEvent->buttons() & Qt::MiddleButton)
    {
        m_ArcBall.move(Real.x, Real.y);
        update();
    }
    else if(pEvent->modifiers().testFlag(Qt::AltModifier))
    {
        float zoomFactor=1.0f;

        if(point.y()-m_LastPoint.y()<0) zoomFactor = 1.0f/1.025f;
        else                            zoomFactor = 1.025f;

        m_glScalef *= zoomFactor;
        update();
    }

    m_LastPoint = point;
}


void gl3dView::wheelEvent(QWheelEvent *pEvent)
{
    int dy = pEvent->pixelDelta().y();
    if(dy==0) dy = pEvent->angleDelta().y(); // pixeldelta usabel on macOS and angleDelta on win/linux; depends also on driver and hardware

    if(s_bSpinAnimation && abs(dy)>120)
    {
        m_bDynScaling = true;
        m_ZoomFactor = dy;

        startDynamicTimer();
    }
    else
    {
        if(m_bDynScaling && m_ZoomFactor*dy<=0)
        {
            //user has changed his mind
            m_bDynScaling=false;
            m_DynTimer.stop();
        }
        else
        {
            if(pEvent->angleDelta().y()>0) m_glScalef *= 1.0/(1.0+DisplayOptions::scaleFactor());
            else                           m_glScalef *= 1.0+DisplayOptions::scaleFactor();
        }
    }

    update();
}


void gl3dView::mouseReleaseEvent(QMouseEvent * pEvent )
{
    QApplication::restoreOverrideCursor();

    // reset all flags to default values
    m_bTrans         = false;
    m_bArcball       = false;
    m_bCrossPoint    = false;
    m_bHasMouseMoved = false;

    Vector3d Real;
    screenToViewport(pEvent->pos(), Real);


    //  inverse the rotation matrix and re-calculate the translation vector
    m_ArcBall.getRotationMatrix(m_MatOut, true);
    setViewportTranslation();


    if(s_bSpinAnimation)
    {
        int movetime = m_MoveTime.elapsed();
        if(movetime<300 && !m_PressedPoint.isNull())
        {
            bool bCtrl = false;
            if (pEvent->modifiers() & Qt::ControlModifier) bCtrl =true;

            if((pEvent->button()==Qt::LeftButton && bCtrl) || pEvent->button()==Qt::MiddleButton)
            {
                m_Trans.reset();
                Vector3d m_SpinEnd;
                m_ArcBall.getSpherePoint(Real.x, Real.y, m_SpinEnd);
                Quaternion qt;
                qt.from2UnitVectors(m_ArcBall.m_Start.normalized(), m_SpinEnd.normalized());
                m_SpinInc = Quaternion(qt.angle()/15.0, qt.axis());

                startDynamicTimer();
                m_bDynRotation = true;
            }
            else if(pEvent->button()==Qt::LeftButton)
            {
                Vector3d A, B;
                screenToWorld(m_PressedPoint, 0, A);
                screenToWorld(pEvent->pos(),  0, B);
                m_Trans = B-A;
                startDynamicTimer();
                m_bDynTranslation = true;
            }
        }
    }

    update();
    pEvent->accept();

    emit viewModified();
}


/**
*Overrides the keyPressEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void gl3dView::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {

        case Qt::Key_Control:
        {
            m_bArcball = true;
            update();
            break;
        }
        case Qt::Key_R:
        {
            on3dReset();
            pEvent->accept();
            break;
        }
        case Qt::Key_X:
        {
            break;
        }
        default:
            pEvent->ignore();
    }
}


/**
*Overrides the keyReleaseEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void gl3dView::keyReleaseEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Control:
        {
            m_bArcball = false;
            update();
            break;
        }

        default:
            pEvent->ignore();
    }
}


void gl3dView::reset3dRotationCenter()
{
    m_ArcBall.getRotationMatrix(m_MatOut, false);

    m_glRotCenter.x = m_MatOut[0]*(m_glViewportTrans.x) + m_MatOut[1]*(-m_glViewportTrans.y) + m_MatOut[2] *m_glViewportTrans.z;
    m_glRotCenter.y = m_MatOut[4]*(m_glViewportTrans.x) + m_MatOut[5]*(-m_glViewportTrans.y) + m_MatOut[6] *m_glViewportTrans.z;
    m_glRotCenter.z = m_MatOut[8]*(m_glViewportTrans.x) + m_MatOut[9]*(-m_glViewportTrans.y) + m_MatOut[10]*m_glViewportTrans.z;
}


void gl3dView::glRenderText(double x, double y, double z, const QString & str, const QColor &textcolor, bool bBackground, bool bBold)
{
    QPoint point;
    if(z>double(m_ClipPlanePos)) return;
    QVector4D v4d;
    point = worldToScreen(Vector3d(x,y,z), v4d);
    point *= devicePixelRatio();
    QPainter painter(&m_PixTextOverlay);
    painter.save();

    painter.setPen(QPen(textcolor));
    QFont fixedfont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    fixedfont.setBold(bBold);
    fixedfont.setPointSize(fixedfont.pointSize()*devicePixelRatio());
    painter.setFont(fixedfont);
    if(bBackground)
    {
        QBrush backbrush(s_BackgroundColor);
//		paint.setBrush(backbrush);
        painter.setBackground(backbrush);
        painter.setBackgroundMode(Qt::OpaqueMode);
    }
    painter.drawText(point, str);
    painter.restore();
}


void gl3dView::glRenderText(int x, int y, const QString & str, QColor const &backclr, QColor const &textcolor, bool bBold)
{
    QPainter painter(&m_PixTextOverlay);
    painter.save();
    painter.setPen(QPen(textcolor));
    QFont fixedfont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    fixedfont.setBold(bBold);
    fixedfont.setPointSize(fixedfont.pointSize()*devicePixelRatio());
    painter.setFont(fixedfont);
    QBrush backbrush(backclr);
    painter.setBrush(backbrush);
    painter.drawText(x*devicePixelRatio(),y*devicePixelRatio(), str);
    painter.restore();
}


/**
*Overrides the resizeGL method of the base class.
* Sets the GL viewport to fit in the client area.
* Sets the scaling factors for the objects to be drawn in the viewport.
*@param width the width in pixels of the client area
*@param height the height in pixels of the client area
*/
void gl3dView::resizeGL(int width, int height)
{
    int side = qMin(width, height);
    glViewport((width - side) / 2, (height - side) / 2, side, side);


    double w, h, s;
    w = double(width);
    h = double(height);
    s = 1.0;


    if(w>h) m_GLViewRect.setRect(-s, s*h/w, s, -s*h/w);
    else    m_GLViewRect.setRect(-s*w/h, s, s*w/h, -s);

    if(!m_PixTextOverlay.isNull())    m_PixTextOverlay = m_PixTextOverlay.scaled(rect().size()*devicePixelRatio());
    if(!m_PixTextOverlay.isNull())    m_PixTextOverlay.fill(Qt::transparent);
}



void gl3dView::getGLError()
{
    switch(glGetError())
    {
        case GL_NO_ERROR:
            Trace("No error has been recorded. The value of this symbolic constant is guaranteed to be 0.");
            break;

        case GL_INVALID_ENUM:
            Trace("An unacceptable value is specified for an enumerated argument. "
                  "The offending command is ignored and has no other side effect than to set the error flag.");
            break;

        case GL_INVALID_VALUE:
            Trace("A numeric argument is out of range. The offending command is ignored and has no other "
                  "side effect than to set the error flag.");
            break;

        case GL_INVALID_OPERATION:
            Trace("The specified operation is not allowed in the current state. The offending command is "
                  "ignored and has no other side effect than to set the error flag.");
            break;

        case GL_INVALID_FRAMEBUFFER_OPERATION:
            Trace("The command is trying to render to or read from the framebuffer while the currently "
                  "bound framebuffer is not framebuffer complete (i.e. the return value from glCheckFramebufferStatus "
                  "is not GL_FRAMEBUFFER_COMPLETE). The offending command is ignored and has no other side effect than "
                  "to set the error flag.");
            break;

        case GL_OUT_OF_MEMORY:
            Trace("There is not enough memory left to execute the command. The state of the GL is "
                  "undefined, except for the state of the error flags, after this error is recorded.");
            break;

        case GL_STACK_UNDERFLOW:
            Trace("An attempt has been made to perform an operation that would cause an internal stack to underflow.");
            break;

        case GL_STACK_OVERFLOW:
            Trace("An attempt has been made to perform an operation that would cause an internal stack to overflow.");
            break;
    }
}


void gl3dView::initializeGL()
{
    QSurfaceFormat ctxtFormat = format();
    m_bUse120StyleShaders = (ctxtFormat.majorVersion()*10+ctxtFormat.minorVersion())<33;

    if(g_bTrace)
    {
        QString log;
        printFormat(ctxtFormat, log);
        Trace(log);
    }


    glMakeAxes();
    glMakeUnitSphere(m_vboSphere);
    glMakeCube(Vector3d(), 1.0,1.0,1.0, m_vboCube, m_vboCubeEdges);
    glMakeArcBall(m_ArcBall);
    glMakeArcPoint(m_ArcBall);

    //setup the shader to paint lines
    QString strange;
    QString vsrc, gsrc, fsrc;
    //--------- setup the shader to paint stippled thick lines -----------
    vsrc = m_bUse120StyleShaders ? ":/shaders/line/line_VS_120.glsl"   : ":/shaders/line/line_VS.glsl";
    m_shadLine.addShaderFromSourceFile(QOpenGLShader::Vertex, vsrc);
    if(m_shadLine.log().length())
    {
        strange = QString::asprintf("%s", QString("Line vertex shader log:"+m_shadLine.log()).toStdString().c_str());
        Trace(strange);
    }

    if(!m_bUse120StyleShaders)
    {
        gsrc = ":/shaders/line/line_GS.glsl";
        m_shadLine.addShaderFromSourceFile(QOpenGLShader::Geometry, gsrc);
        if(m_shadLine.log().length())
        {
            strange = QString::asprintf("%s", QString("Line geometry shader log:"+m_shadLine.log()).toStdString().c_str());
            Trace(strange);
        }
    }


    fsrc = m_bUse120StyleShaders? ":/shaders/line/line_FS_120.glsl" : ":/shaders/line/line_FS.glsl";
    m_shadLine.addShaderFromSourceFile(QOpenGLShader::Fragment, fsrc);
    if(m_shadLine.log().length())
    {
        strange = QString::asprintf("%s", QString("Stipple fragment shader log:"+m_shadLine.log()).toStdString().c_str());
        Trace(strange);
    }

    m_shadLine.link();
    m_shadLine.bind();
    {
        m_locLine.m_attrVertex    = m_shadLine.attributeLocation("vertexPosition_modelSpace");
        m_locLine.m_attrColor = m_shadLine.attributeLocation("vertexColor");
        m_locLine.m_vmMatrix     = m_shadLine.uniformLocation("vmMatrix");
        m_locLine.m_pvmMatrix    = m_shadLine.uniformLocation("pvmMatrix");
        m_locLine.m_HasUniColor  = m_shadLine.uniformLocation("HasUniColor");
        m_locLine.m_UniColor     = m_shadLine.uniformLocation("UniformColor");
        m_locLine.m_ClipPlane    = m_shadLine.uniformLocation("clipPlane0");
        m_locLine.m_Thickness    = m_shadLine.uniformLocation("Thickness");
        m_locLine.m_Viewport     = m_shadLine.uniformLocation("Viewport");
        m_locLine.m_Pattern      = m_shadLine.uniformLocation("pattern");
        m_locLine.m_nPatterns    = m_shadLine.uniformLocation("nPatterns");
        GLint nPatterns = 300; // number of patterns per unit projected length (viewport half width = 1)
        m_shadLine.setUniformValue(m_locLine.m_nPatterns, nPatterns);
    }
    m_shadLine.release();


    //setup the shader to paint colored surfaces
    vsrc = m_bUse120StyleShaders ? ":/shaders/surface/surface_VS_120.glsl" : ":/shaders/surface/surface_VS.glsl";
    fsrc = m_bUse120StyleShaders ? ":/shaders/surface/surface_FS_120.glsl" : ":/shaders/surface/surface_FS.glsl";
    m_shadSurf.addShaderFromSourceFile(QOpenGLShader::Vertex, vsrc);
    if(m_shadSurf.log().length()) Trace("Surface vertex shader log:"+m_shadSurf.log());

    m_shadSurf.addShaderFromSourceFile(QOpenGLShader::Fragment, fsrc);
    if(m_shadSurf.log().length()) Trace("Surface fragment shader log:"+m_shadSurf.log());

    m_shadSurf.link();
    m_shadSurf.bind();
    {
        m_locSurf.m_attrVertex = m_shadSurf.attributeLocation("vertexPosition_modelSpace");
        m_locSurf.m_attrNormal = m_shadSurf.attributeLocation("vertexNormal_modelSpace");
        m_locSurf.m_attrUV     = m_shadSurf.attributeLocation("vertexUV");
        m_locSurf.m_attrColor  = m_shadSurf.attributeLocation("vertexColor");
        m_locSurf.m_attrOffset = m_shadSurf.attributeLocation("vertexOffset");

        m_locSurf.m_ClipPlane    = m_shadSurf.uniformLocation("clipPlane0");
        m_locSurf.m_pvmMatrix    = m_shadSurf.uniformLocation("pvmMatrix");
        m_locSurf.m_vmMatrix     = m_shadSurf.uniformLocation("vmMatrix");
        m_locSurf.m_HasUniColor  = m_shadSurf.uniformLocation("HasUniColor");
        m_locSurf.m_UniColor     = m_shadSurf.uniformLocation("UniformColor");
        m_locSurf.m_Light        = m_shadSurf.uniformLocation("LightOn");
        m_locSurf.m_TwoSided     = m_shadSurf.uniformLocation("TwoSided");
        m_locSurf.m_HasTexture   = m_shadSurf.uniformLocation("HasTexture");
        m_locSurf.m_TexSampler   = m_shadSurf.uniformLocation("TheSampler");
        m_locSurf.m_IsInstanced  = m_shadSurf.uniformLocation("Instanced");
        m_locSurf.m_Scale        = m_shadSurf.uniformLocation("uScale");
    }
    m_shadSurf.release();

    glSetupLight();
}


void gl3dView::glSetupLight()
{
    QColor LightColor;
    LightColor.setRedF(  double(s_Light.m_Red));
    LightColor.setGreenF(double(s_Light.m_Green));
    LightColor.setBlueF( double(s_Light.m_Blue));
    GLfloat x = s_Light.m_X;
    GLfloat y = s_Light.m_Y;
    GLfloat z = s_Light.m_Z;

    m_shadSurf.bind();
    {
        if(s_Light.m_bIsLightOn) m_shadSurf.setUniformValue(m_locSurf.m_Light, 1);
        else            m_shadSurf.setUniformValue(m_locSurf.m_Light, 0);

        m_shadSurf.setUniformValue(m_shadSurf.uniformLocation("LightPosition_viewSpace"),  x,y,z);
        m_shadSurf.setUniformValue(m_shadSurf.uniformLocation("EyePosition_viewSpace"),    0,0,s_Light.m_EyeDist);
        m_shadSurf.setUniformValue(m_shadSurf.uniformLocation("LightColor"),               LightColor);
        m_shadSurf.setUniformValue(m_shadSurf.uniformLocation("LightAmbient"),             s_Light.m_Ambient);
        m_shadSurf.setUniformValue(m_shadSurf.uniformLocation("LightDiffuse"),             s_Light.m_Diffuse);
        m_shadSurf.setUniformValue(m_shadSurf.uniformLocation("LightSpecular"),            s_Light.m_Specular);
        m_shadSurf.setUniformValue(m_shadSurf.uniformLocation("MaterialShininess"),        float(s_Light.m_iShininess));
        m_shadSurf.setUniformValue(m_shadSurf.uniformLocation("Kc"),                       s_Light.m_Attenuation.m_Constant);
        m_shadSurf.setUniformValue(m_shadSurf.uniformLocation("Kl"),                       s_Light.m_Attenuation.m_Linear);
        m_shadSurf.setUniformValue(m_shadSurf.uniformLocation("Kq"),                       s_Light.m_Attenuation.m_Quadratic);
    }
    m_shadSurf.release();
}


void gl3dView::paintGL()
{
    glMake3dObjects();

    paintGL3();
    paintOverlay();
}


void gl3dView::paintGL3()
{
    float s = 1.0f;
    double pixelRatio = devicePixelRatio();

    glClearColor(float(s_BackgroundColor.redF()), float(s_BackgroundColor.greenF()), float(s_BackgroundColor.blueF()), 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);

    QVector4D clipplane(0.0,0.0,-1,m_ClipPlanePos);

    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_ClipPlane, m_ClipPlanePos);
    }
    m_shadSurf.release();


    int width  = int(geometry().width()  * pixelRatio);
    int height = int(geometry().height() * pixelRatio);

    m_matProj.setToIdentity();
    m_matProj.ortho(-s,s,-(height*s)/width,(height*s)/width,-500.0f*s,500.0f*s);

    double m[16];
    m_ArcBall.getRotationMatrix(m, true);
    m_matView = QMatrix4x4(float(m[0]),  float(m[1]),  float(m[2]),  float(m[3]),
                              float(m[4]),  float(m[5]),  float(m[6]),  float(m[7]),
                              float(m[8]),  float(m[9]),  float(m[10]), float(m[11]),
                              float(m[12]), float(m[13]), float(m[14]), float(m[15]));

    m_matModel.setToIdentity();//keep identity


    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_ClipPlane, m_ClipPlanePos);
        m_shadLine.setUniformValue(m_locLine.m_Viewport, QVector2D(float(m_GLViewRect.width()), float(m_GLViewRect.height())));
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*m_matModel);
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matProj*m_matView*m_matModel);
    }
    m_shadLine.release();

    if(m_bArcball) paintArcBall();

    if(m_bShowLight)
    {
        Vector3d lightPos(double(s_Light.m_X), double(s_Light.m_Y), double(s_Light.m_Z));
        double radius = double(s_Light.m_Z+2.0f)/73.0;
        QColor lightColor;
        lightColor.setRedF(  double(s_Light.m_Red));
        lightColor.setGreenF(double(s_Light.m_Green));
        lightColor.setBlueF( double(s_Light.m_Blue));
        lightColor.setAlphaF(1.0);

        paintSphere(lightPos, radius, lightColor, false);
    }

    if(m_bAxes)
    {
        // fixed scale axis for the axis
        QMatrix4x4 vm(m_matView);
        m_matView.scale(m_glScalef, m_glScalef, m_glScalef);
        m_matView.translate(m_glRotCenter.xf(), m_glRotCenter.yf(), m_glRotCenter.zf());
        m_matView.scale(0.5f/m_glScalef, 0.5f/m_glScalef, 0.5f/m_glScalef);
        paintAxes();
        m_matView=vm; // leave things as they were
    }

    m_matView.scale(m_glScalef, m_glScalef, m_glScalef);
    m_matView.translate(m_glRotCenter.xf(), m_glRotCenter.yf(), m_glRotCenter.zf());

    glRenderView();

    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
}


void gl3dView::setScale(double refLength)
{
    m_glScalef = 1.5/refLength;
}


void gl3dView::paintArcBall()
{    
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    m_shadLine.bind();
    {
        m_shadLine.enableAttributeArray(m_locLine.m_attrVertex);
        m_vboArcBall.bind();
        {
            m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3, 0);
            m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj*m_matView*m_matModel);
            m_shadLine.setUniformValue(m_locLine.m_UniColor, QColor(43,43,43,175));
            m_shadLine.setUniformValue(m_locLine.m_Pattern, GLStipple(Line::SOLID));
            m_shadLine.setUniformValue(m_locLine.m_Thickness, 2);

            if(m_bUse120StyleShaders) glLineWidth(2);

            int nSegs = m_vboArcBall.size()/2/3/int(sizeof(float)); // 2 vertices and (3 position components)
            glDrawArrays(GL_LINES, 0, nSegs*2);
        }

        m_vboArcBall.release();

        if(m_bCrossPoint)
        {
            QMatrix4x4 pvmCP(m_matProj);
            float angle, xf, yf, zf;
            m_ArcBall.rotateCrossPoint(angle, xf, yf, zf);
            pvmCP.rotate(angle, xf, yf, zf);
            m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, pvmCP);

            m_vboArcPoint.bind();
            {
                m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3, 0);
                m_shadLine.setUniformValue(m_locLine.m_UniColor, QColor(70, 25, 40));
                m_shadLine.setUniformValue(m_locLine.m_Pattern, GLStipple(Line::SOLID));
                m_shadLine.setUniformValue(m_locLine.m_Thickness, 3);
                if(m_bUse120StyleShaders) glLineWidth(5);

                int nSegs = m_vboArcPoint.size()/2/3/int(sizeof(float)); // 2 vertices and (3 position components)
                glDrawArrays(GL_LINES, 0, nSegs*2);
            }
            m_vboArcPoint.release();
        }
        m_shadLine.disableAttributeArray(m_locLine.m_attrVertex);
    }
    m_shadLine.release();
}


void gl3dView::paintAxes()
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*m_matModel);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj*m_matView*m_matModel);
        m_shadLine.setUniformValue(m_locLine.m_HasUniColor, 1);
        m_shadLine.setUniformValue(m_locLine.m_UniColor, W3dPrefsDlg::s_AxisStyle.m_Color);
        m_shadLine.setUniformValue(m_locLine.m_Pattern, GLStipple(W3dPrefsDlg::s_AxisStyle.m_Stipple));
        m_shadLine.setUniformValue(m_locLine.m_Thickness, W3dPrefsDlg::s_AxisStyle.m_Width);
        m_shadLine.setUniformValue(m_locLine.m_Viewport, QVector2D(float(m_GLViewRect.width()), float(m_GLViewRect.height())));
        m_vboAxes.bind();
        {
            m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3);
            m_shadLine.enableAttributeArray(m_locLine.m_attrVertex);

            if(m_bUse120StyleShaders)
            {
                glEnable(GL_LINE_STIPPLE);
                GLLineStipple(W3dPrefsDlg::s_AxisStyle.m_Stipple);
                glLineWidth(float(W3dPrefsDlg::s_AxisStyle.m_Width));
            }

            int nvertices = m_vboAxes.size()/3/int(sizeof(float)); // three components
            glDrawArrays(GL_LINES, 0, nvertices);

            if(m_bUse120StyleShaders)
            {
                glDisable(GL_LINE_STIPPLE);
            }
        }
        m_vboAxes.release();

        m_shadLine.disableAttributeArray(m_locLine.m_attrVertex);
    }
    m_shadLine.release();

    glRenderText(1.0, 0.015, 0.015, "X", s_TextColor);
    glRenderText(0.015, 1.0, 0.015, "Y", s_TextColor);
    glRenderText(0.015, 0.015, 1.0, "Z", s_TextColor);
}


void gl3dView::glMakeArcBall(ArcBall & arcball)
{
    float GLScale = 1.0f;
    int row(0), col(0);
    float lat_incr(0), lon_incr(0), phi(0), phi1(0), theta(0),  theta1(0);

    float Radius = float(arcball.s_sphereRadius);
    lat_incr =  90.0f / NUMANGLES;
    lon_incr = 360.0f / NUMCIRCLES;

    int iv=0;

//    int bufferSize = ((NUMCIRCLES*2)*(NUMANGLES-2) + (NUMPERIM-1)*2)*3;
    int nSegs= NUMCIRCLES * (NUMANGLES-3) *2;
    nSegs += (NUMPERIM-2)*2;

    int buffersize = nSegs;
    buffersize *= 2; // 2 vertices/segment
    buffersize *= 3; // 3 components/vertex

    QVector<float> ArcBallVertexArray(buffersize, 0);

    //ARCBALL
    for (col=0; col<NUMCIRCLES; col++)
    {
        //first
        phi = (col * lon_incr) * PIf/180.0f;
        for (row=1; row<NUMANGLES-2; row++)
        {
            theta  = ( row    * lat_incr) * PIf/180.0f;
            theta1 = ((row+1) * lat_incr) * PIf/180.0f;
            ArcBallVertexArray[iv++] = Radius*cosf(phi)*cosf(theta)*GLScale;
            ArcBallVertexArray[iv++] = Radius*sinf(theta)*GLScale;
            ArcBallVertexArray[iv++] = Radius*sinf(phi)*cosf(theta)*GLScale;
            ArcBallVertexArray[iv++] = Radius*cosf(phi)*cosf(theta1)*GLScale;
            ArcBallVertexArray[iv++] = Radius*sinf(theta1)*GLScale;
            ArcBallVertexArray[iv++] = Radius*sinf(phi)*cosf(theta1)*GLScale;
        }
    }

    for (col=0; col<NUMCIRCLES; col++)
    {
        //Second
        phi = (col * lon_incr ) * PIf/180.0f;
        for (row=1; row<NUMANGLES-2; row++)
        {
            theta  = -( row    * lat_incr) * PIf/180.0f;
            theta1 = -((row+1) * lat_incr) * PIf/180.0f;
            ArcBallVertexArray[iv++] = Radius*cosf(phi)*cosf(theta)*GLScale;
            ArcBallVertexArray[iv++] = Radius*sinf(theta)*GLScale;
            ArcBallVertexArray[iv++] = Radius*sinf(phi)*cosf(theta)*GLScale;
            ArcBallVertexArray[iv++] = Radius*cosf(phi)*cosf(theta1)*GLScale;
            ArcBallVertexArray[iv++] = Radius*sinf(theta1)*GLScale;
            ArcBallVertexArray[iv++] = Radius*sinf(phi)*cosf(theta1)*GLScale;
        }
    }

    theta = 0.;
    for(col=1; col<NUMPERIM-1; col++)
    {
        phi  = (0.0f +  col   *360.0f/72.0f) * PIf/180.0f;
        phi1 = (0.0f + (col+1)*360.0f/72.0f) * PIf/180.0f;
        ArcBallVertexArray[iv++] = Radius * cosf(phi)  * cosf(theta)*GLScale;
        ArcBallVertexArray[iv++] = Radius * sinf(theta)*GLScale;
        ArcBallVertexArray[iv++] = Radius * sinf(phi)  * cosf(theta)*GLScale;
        ArcBallVertexArray[iv++] = Radius * cosf(phi1) * cosf(theta)*GLScale;
        ArcBallVertexArray[iv++] = Radius * sinf(theta)*GLScale;
        ArcBallVertexArray[iv++] = Radius * sinf(phi1) * cosf(theta)*GLScale;
    }

    theta = 0.;
    for(col=1; col<NUMPERIM-1; col++)
    {
        phi =  (0.0f +  col   *360.0f/72.0f) * PIf/180.0f;
        phi1 = (0.0f + (col+1)*360.0f/72.0f) * PIf/180.0f;
        ArcBallVertexArray[iv++] = Radius * cosf(-phi)  * cosf(theta)*GLScale;
        ArcBallVertexArray[iv++] = Radius * sinf(theta)*GLScale;
        ArcBallVertexArray[iv++] = Radius * sinf(-phi)  * cosf(theta)*GLScale;
        ArcBallVertexArray[iv++] = Radius * cosf(-phi1) * cosf(theta)*GLScale;
        ArcBallVertexArray[iv++] = Radius * sinf(theta)*GLScale;
        ArcBallVertexArray[iv++] = Radius * sinf(-phi1) * cosf(theta)*GLScale;
    }
    Q_ASSERT(iv==buffersize);

    m_vboArcBall.destroy();
    m_vboArcBall.create();
    m_vboArcBall.bind();
    m_vboArcBall.allocate(ArcBallVertexArray.data(), buffersize * int(sizeof(GLfloat)));

    m_vboArcBall.release();
}


void gl3dView::glMakeArcPoint(ArcBall const&arcball)
{
    float theta(0), theta1(0), phi(0), phi1(0);
    float Radius = float(arcball.s_sphereRadius);

    float Angle(10.0);

    int iv=0;

    int nsegs = (2*NUMARCPOINTS-1) * 2;
    int buffersize = nsegs * 2 * 3; // 2 vertices and 3 components

    QVector<float> ArcPointVertexArray(buffersize, 0);

    //ARCPOINT
    float lat_incr = Angle / NUMARCPOINTS;

    phi = 0.0;
    for (int row=-NUMARCPOINTS; row<NUMARCPOINTS-1; row++)
    {
        theta  = (0.0f +  row    * lat_incr) * PIf/180.0f;
        theta1 = (0.0f + (row+1) * lat_incr) * PIf/180.0f;
        ArcPointVertexArray[iv++] = Radius*cosf(phi)*cosf(theta);
        ArcPointVertexArray[iv++] = Radius*sinf(theta);
        ArcPointVertexArray[iv++] = Radius*sinf(phi)*cosf(theta);
        ArcPointVertexArray[iv++] = Radius*cosf(phi)*cosf(theta1);
        ArcPointVertexArray[iv++] = Radius*sinf(theta1);
        ArcPointVertexArray[iv++] = Radius*sinf(phi)*cosf(theta1);
    }

    theta = 0.0;
    for(int col=-NUMARCPOINTS; col<NUMARCPOINTS-1; col++)
    {
        phi  = (0.0f +  col   *Angle/NUMARCPOINTS) * PIf/180.0f;
        phi1 = (0.0f + (col+1)*Angle/NUMARCPOINTS) * PIf/180.0f;
        ArcPointVertexArray[iv++] = Radius * cosf(phi) * cosf(theta);
        ArcPointVertexArray[iv++] = Radius * sinf(theta);
        ArcPointVertexArray[iv++] = Radius * sinf(phi) * cosf(theta);
        ArcPointVertexArray[iv++] = Radius * cosf(phi1) * cosf(theta);
        ArcPointVertexArray[iv++] = Radius * sinf(theta);
        ArcPointVertexArray[iv++] = Radius * sinf(phi1) * cosf(theta);
    }

    Q_ASSERT(iv==buffersize);

    m_vboArcPoint.destroy();
    m_vboArcPoint.create();
    m_vboArcPoint.bind();
    m_vboArcPoint.allocate(ArcPointVertexArray.data(), buffersize * int(sizeof(GLfloat)));
    m_vboArcPoint.release();
}


//sphere
#define NUMLONG  31
#define NUMLAT   29
/**
Creates a vbo for a sphere with unit radius by splitting along latitude and longitude arcs.
Wrong way - use icosahedron splits instead
*/
void gl3dView::glMakeUnitSphere(QOpenGLBuffer &vbo)
{
    float phi=0, theta=0;
    float phi1=0, theta1=0;

    float start_lat = -90.0f * PIf/180.0f;
    float start_lon = 0.0f * PIf/180.0f;

    float lat_incr = 180.0f / (NUMLAT-1) * PIf/180.0f;
    float lon_incr = 360.0f / (NUMLONG-1) * PIf/180.0f;

    int nQuads = (NUMLONG-1) * (NUMLAT-1);
    int nTriangles = nQuads*2;
    int bufferSize =  nTriangles * 3 * 6; // 3 vertices *6 components
    QVector<GLfloat> sphereVertexArray(bufferSize);

    int iv=0;
    for (int iLong=0; iLong<NUMLONG-1; iLong++)
    {
        phi  = (start_lon + float(iLong)   * lon_incr) ;
        phi1 = (start_lon + float(iLong+1) * lon_incr) ;
        for (int iLat=0; iLat<NUMLAT-1; iLat++)
        {

            theta  = (start_lat + float(iLat)   * lat_incr);
            theta1 = (start_lat + float(iLat+1) * lat_incr);

            //first triangle
            sphereVertexArray[iv++] = cosf(phi) * cosf(theta);
            sphereVertexArray[iv++] = sinf(phi) * cosf(theta);
            sphereVertexArray[iv++] = sinf(theta);
            sphereVertexArray[iv++] = cosf(phi) * cosf(theta);
            sphereVertexArray[iv++] = sinf(phi) * cosf(theta);
            sphereVertexArray[iv++] = sinf(theta);

            sphereVertexArray[iv++] = cosf(phi1) * cosf(theta);
            sphereVertexArray[iv++] = sinf(phi1) * cosf(theta);
            sphereVertexArray[iv++] = sinf(theta);
            sphereVertexArray[iv++] = cosf(phi1) * cosf(theta);
            sphereVertexArray[iv++] = sinf(phi1) * cosf(theta);
            sphereVertexArray[iv++] = sinf(theta);

            sphereVertexArray[iv++] = cosf(phi) * cosf(theta1);
            sphereVertexArray[iv++] = sinf(phi) * cosf(theta1);
            sphereVertexArray[iv++] = sinf(theta1);
            sphereVertexArray[iv++] = cosf(phi) * cosf(theta1);
            sphereVertexArray[iv++] = sinf(phi) * cosf(theta1);
            sphereVertexArray[iv++] = sinf(theta1);

            //second triangle
            sphereVertexArray[iv++] = cosf(phi1) * cosf(theta);
            sphereVertexArray[iv++] = sinf(phi1) * cosf(theta);
            sphereVertexArray[iv++] = sinf(theta);
            sphereVertexArray[iv++] = cosf(phi1) * cosf(theta);
            sphereVertexArray[iv++] = sinf(phi1) * cosf(theta);
            sphereVertexArray[iv++] = sinf(theta);

            sphereVertexArray[iv++] = cosf(phi1) * cosf(theta1);
            sphereVertexArray[iv++] = sinf(phi1) * cosf(theta1);
            sphereVertexArray[iv++] = sinf(theta1);
            sphereVertexArray[iv++] = cosf(phi1) * cosf(theta1);
            sphereVertexArray[iv++] = sinf(phi1) * cosf(theta1);
            sphereVertexArray[iv++] = sinf(theta1);

            sphereVertexArray[iv++] = cosf(phi) * cosf(theta1);
            sphereVertexArray[iv++] = sinf(phi) * cosf(theta1);
            sphereVertexArray[iv++] = sinf(theta1);
            sphereVertexArray[iv++] = cosf(phi) * cosf(theta1);
            sphereVertexArray[iv++] = sinf(phi) * cosf(theta1);
            sphereVertexArray[iv++] = sinf(theta1);
        }
    }

    Q_ASSERT(iv==bufferSize);

    vbo.create();
    vbo.bind();
    vbo.allocate(sphereVertexArray.constData(), sphereVertexArray.size() * int(sizeof(GLfloat)));
    vbo.release();
}


void gl3dView::paintSphere(Vector3d const &place, double radius, QColor sphereColor, bool bLight)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    QMatrix4x4 mSphere; //is identity
    mSphere.translate(place.xf(), place.yf(), place.zf());
    mSphere.scale(float(radius));

    QMatrix4x4 vmMat(m_matView*m_matModel*mSphere);
    QMatrix4x4 pvmMat(m_matProj*vmMat);

    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix, vmMat);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMat);
    }
    m_shadSurf.release();

    paintTriangles3Vtx(m_vboSphere, sphereColor, false, bLight);
}


void gl3dView::onRotationIncrement()
{
    if(m_iTimerInc>ANIMATIONFRAMES)
    {
        m_TransitionTimer.stop();
        return;
    }
    Quaternion qtrot;
    double t = double(m_iTimerInc)/double(ANIMATIONFRAMES);
    Quaternion::slerp(m_QuatStart, m_QuatEnd, t, qtrot);
    m_ArcBall.setQuat(qtrot);

    reset3dRotationCenter();
    update();
    m_iTimerInc++;
}


void gl3dView::startRotationTimer()
{
    if(s_bAnimateTransitions)
    {
        m_iTimerInc = 0;

        // calculate the number of animation frames for 60Hz refresh rate
        int period = 17; //60 Hz in ms
        ANIMATIONFRAMES = int(double(s_AnimationTime)/double(period));

        disconnect(&m_TransitionTimer, nullptr, nullptr, nullptr);
        connect(&m_TransitionTimer, SIGNAL(timeout()), SLOT(onRotationIncrement()));
        m_TransitionTimer.start(period);
    }
    else
    {
        reset3dRotationCenter();
        update();
    }
}


void gl3dView::startTranslationTimer(Vector3d PP)
{
    int period = 17; //60 Hz in ms
    ANIMATIONFRAMES = int(double(s_AnimationTime)/double(period));

    double inc = double(ANIMATIONFRAMES);
    if(s_bAnimateTransitions)
    {
        m_TransIncrement.x = (-PP.x -m_glRotCenter.x)/inc;
        m_TransIncrement.y = (-PP.y -m_glRotCenter.y)/inc;
        m_TransIncrement.z = (-PP.z -m_glRotCenter.z)/inc;

        m_iTimerInc = 0;

        disconnect(&m_TransitionTimer, nullptr, nullptr, nullptr);
        connect(&m_TransitionTimer, SIGNAL(timeout()), SLOT(onTranslationIncrement()));
        m_TransitionTimer.start(period);
    }
    else
    {
        m_glRotCenter.set(-PP.x, -PP.y, -PP.z);
        setViewportTranslation();

        update();
    }
}


void gl3dView::onTranslationIncrement()
{
    if(m_iTimerInc>=ANIMATIONFRAMES)
    {
        m_TransitionTimer.stop();
        m_iTimerInc = 0;
        return;
    }

    m_glRotCenter += m_TransIncrement;
    setViewportTranslation();

    update();
    m_iTimerInc++;
}


void gl3dView::onResetIncrement()
{
    if(m_iTimerInc>=ANIMATIONFRAMES)
    {
        m_TransitionTimer.stop();
        m_iTimerInc = 0;
        return;
    }

    m_glScalef += m_glScaleIncrement;
    m_glViewportTrans += m_TransIncrement;

    reset3dRotationCenter();
    update();
    m_iTimerInc++;
}


/**
 * Sets an automatic scale for the wing or plane in the 3D view, depending on wing span.
 */
void gl3dView::set3dScale(double length)
{
    if(length>0.0) m_glScalefRef = (4./5.*2.0/length);
    m_glScalef = m_glScalefRef;
    m_glViewportTrans.set(0.0, 0.0, 0.0);
    reset3dRotationCenter();
    update();
}


void gl3dView::startResetTimer(double length)
{
    m_iTimerInc = 0;

    // calculate the number of animation frames for 60Hz refresh rate
    int period = 17; //60 Hz in ms
    ANIMATIONFRAMES = int(double(s_AnimationTime)/double(period));

    m_glScaleIncrement = (1.0/length-m_glScalef)/ANIMATIONFRAMES;
    m_TransIncrement = (Vector3d(0.0,0.0,0.0)-m_glViewportTrans)/ANIMATIONFRAMES;

    disconnect(&m_TransitionTimer, nullptr, nullptr, nullptr);
    connect(&m_TransitionTimer, SIGNAL(timeout()), SLOT(onResetIncrement()));
    m_TransitionTimer.start(7);//7 ms x 50 times
}

/** note: glLineStipple is deprecated since OpenGL 3.1 */
void GLLineStipple(int style)
{
    if     (style == Qt::DashLine)       glLineStipple (1, 0xCFCF);
    else if(style == Qt::DotLine)        glLineStipple (1, 0x6666);
    else if(style == Qt::DashDotLine)    glLineStipple (1, 0xFF18);
    else if(style == Qt::DashDotDotLine) glLineStipple (1, 0x7E66);
    else                                 glLineStipple (1, 0xFFFF);
}


GLushort GLStipple(Line::enumLineStipple stipple)
{
    switch(stipple)
    {
        default:
        case Line::SOLID:       return 0xFFFF;
        case Line::DASH:        return 0x1F1F;
        case Line::DOT:         return 0x6666;
        case Line::DASHDOT:     return 0xFF18;
        case Line::DASHDOTDOT:  return 0x7E66;
    }
}


/**
 * @brief since glLineStipple is deprecated, make an array of simple lines for all 3 axis
 */
void gl3dView::glMakeAxes()
{
    QVector<GLfloat>axisVertexArray(54);

    GLfloat const x_axis[] = {
        -1.0f, 0.0f, 0.0f,
         1.0f, 0.0f, 0.0f ,
         1.0f,   0.0f,   0.0f,
         0.95f,  0.015f, 0.015f,
         1.0f,  0.0f,    0.0f,
         0.95f,-0.015f,-0.015f
    };

    GLfloat const y_axis[] = {
          0.0f,    -1.0f,    0.0f,
          0.0f,     1.0f,    0.0f,
          0.f,      1.0f,    0.0f,
          0.015f,   0.95f,   0.015f,
          0.f,      1.0f,    0.0f,
         -0.015f,   0.95f,  -0.015f
    };

    GLfloat const z_axis[] = {
         0.0f,    0.0f,   -1.0f,
         0.0f,    0.0f,    1.0f,
         0.0f,    0.0f,    1.0f,
         0.015f,  0.015f,  0.95f,
         0.0f,    0.0f,    1.0f,
        -0.015f, -0.015f,  0.95f
    };

    int iv=0;
    for(int i=0; i<18; i++) axisVertexArray[iv++] = x_axis[i]*1.0f;
    for(int i=0; i<18; i++) axisVertexArray[iv++] = y_axis[i]*1.0f;
    for(int i=0; i<18; i++) axisVertexArray[iv++] = z_axis[i]*1.0f;

    Q_ASSERT(iv==54);

    m_vboAxes.destroy();
    m_vboAxes.create();
    m_vboAxes.bind();
    m_vboAxes.allocate(axisVertexArray.constData(), axisVertexArray.size() * int(sizeof(GLfloat)));
    m_vboAxes.release();
}


void gl3dView::paintOverlay()
{
    QOpenGLPaintDevice device(size() * devicePixelRatio());
    QPainter painter(&device);

    if(!m_PixTextOverlay.isNull())
    {
        painter.drawPixmap(0,0, m_PixTextOverlay);
        m_PixTextOverlay.fill(Qt::transparent);
    }
}

void gl3dView::set3dRotationCenter(QPoint const &point)
{
    //adjusts the new rotation center after the user has picked a point on the screen
    //finds the closest panel under the point,
    //and changes the rotation vector and viewport translation
    Vector3d I, A, B, AA, BB, PP;

    screenToViewport(point, B);
    B.z = -1.0;
    A.set(B.x, B.y, +1.0);

    viewportToWorld(A, AA);
    viewportToWorld(B, BB);

    m_TransIncrement.set(BB.x-AA.x, BB.y-AA.y, BB.z-AA.z);
    m_TransIncrement.normalize();

    bool bIntersect = false;

    if(intersectTheObject(AA, BB, I))
    {
        bIntersect = true;
        PP.set(I);
    }

    if(bIntersect)
    {
        startTranslationTimer(PP);
    }
}



void gl3dView::startDynamicTimer()
{
    m_DynTimer.start(17);
    setMouseTracking(false);
}


void gl3dView::stopDynamicTimer()
{
    if(m_DynTimer.isActive())
    {
        m_DynTimer.stop();
//        reset3dRotationCenter();
        //  inverse the rotation matrix and re-calculate the translation vector
        m_ArcBall.getRotationMatrix(m_MatOut, true);
        setViewportTranslation();
    }
    m_bDynTranslation = m_bDynRotation = m_bDynScaling = false;
    setMouseTracking(true);
}



void gl3dView::setViewportTranslation()
{
    m_glViewportTrans.x =  (m_MatOut[0]*m_glRotCenter.x + m_MatOut[1]*m_glRotCenter.y + m_MatOut[2] *m_glRotCenter.z);
    m_glViewportTrans.y = -(m_MatOut[4]*m_glRotCenter.x + m_MatOut[5]*m_glRotCenter.y + m_MatOut[6] *m_glRotCenter.z);
    m_glViewportTrans.z =  (m_MatOut[8]*m_glRotCenter.x + m_MatOut[9]*m_glRotCenter.y + m_MatOut[10]*m_glRotCenter.z);
}


/**
* Converts screen coordinates to OpenGL Viewport coordinates.
* @param point the screen coordinates.
* @param real the viewport coordinates.
*/
void gl3dView::screenToViewport(QPoint const &point, Vector3d &real) const
{
    double h2, w2;
    h2 = double(geometry().height()) /2.0;
    w2 = double(geometry().width())  /2.0;

    real.x =  (double(point.x()) - w2) / w2;
    real.y = -(double(point.y()) - h2) / w2;
}


/**
*Converts screen coordinates to OpenGL Viewport coordinates.
*@param point the screen coordinates.
*@param real the viewport coordinates.
*/
void gl3dView::screenToViewport(QPoint const &point, int z, Vector3d &real) const
{
    double h2, w2;
    h2 = double(geometry().height()) /2.0;
    w2 = double(geometry().width())  /2.0;

    real.x =  (double(point.x()) - w2) / w2;
    real.y = -(double(point.y()) - h2) / w2;

    real.z = double(z);
}


/**
*Converts OpenGL Viewport coordinates to screen coordinates
*@param real the viewport coordinates.
*@param point the screen coordinates.
*/
void gl3dView::viewportToScreen(Vector3d const &real, QPoint &point) const
{
    double dx, dy, h2, w2;

    h2 = m_GLViewRect.height() /2.0;
    w2 = m_GLViewRect.width()  /2.0;

    dx = ( real.x + w2)/2.0;
    dy = (-real.y + h2)/2.0;

    point.setX(int(dx * double(geometry().width())));
    point.setY(int(dy * double(geometry().width())));
}


QVector4D gl3dView::worldToViewport(Vector3d v) const
{
    QVector4D v4(float(v.x), float(v.y), float(v.z), 1.0f);
    return m_matProj*m_matView*m_matModel * v4;
}


QPoint gl3dView::worldToScreen(Vector3d const&v, QVector4D &vScreen) const
{
    QVector4D v4(float(v.x), float(v.y), float(v.z), 1.0f);
    vScreen = m_matProj*m_matView*m_matModel * v4;
    return QPoint(int((vScreen.x()+1.0f)*width()/2), int((1.0f-vScreen.y())*height()/2));
}


QPoint gl3dView::worldToScreen(QVector4D const&v4, QVector4D &vScreen) const
{
    vScreen = m_matProj*m_matView*m_matModel * v4;
    return QPoint(int((vScreen.x()+1.0f)*width()/2), int((1.0f-vScreen.y())*height()/2));
}


void gl3dView::screenToWorld(QPoint const &screenpt, int z, Vector3d &modelpt) const
{
    QMatrix4x4 m;
    QVector4D in, out;
    Vector3d real;

    screenToViewport(screenpt, z, real);
    in.setX(float(real.x));
    in.setY(float(real.y));
    in.setZ(float(real.z));
    in.setW(1.0);

    bool bInverted=false;
    QMatrix4x4 vmMatrix = m_matView*m_matModel;
    m = vmMatrix.inverted(&bInverted);
    out = m * in;

    if(fabs(double(out[3]))>PRECISION)
    {
        modelpt.x = double(out[0]/out[3]);
        modelpt.y = double(out[1]/out[3]);
        modelpt.z = double(out[2]/out[3]);
    }
    else
    {
        modelpt.set(double(out[0]), double(out[1]), double(out[2]));
    }
}


void gl3dView::viewportToWorld(Vector3d vp, Vector3d &w) const
{
    //un-translate
    vp.x += - m_glViewportTrans.x*double(m_glScalef);
    vp.y += + m_glViewportTrans.y*double(m_glScalef);

    //un-scale
    vp.x *= 1.0/double(m_glScalef);
    vp.y *= 1.0/double(m_glScalef);
    vp.z *= 1.0/double(m_glScalef);

    //un-rotate
    double m[16];
    m_ArcBall.getRotationMatrix(m, false);
    w.x = m[0]*vp.x + m[1]*vp.y + m[2] *vp.z;
    w.y = m[4]*vp.x + m[5]*vp.y + m[6] *vp.z;
    w.z = m[8]*vp.x + m[9]*vp.y + m[10]*vp.z;
}


void gl3dView::paintTriangles3Vtx(QOpenGLBuffer &vbo, const QColor &backclr, bool bTwoSided, bool bLight)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_UniColor, backclr);
        if(bLight) m_shadSurf.setUniformValue(m_locSurf.m_Light, 1);
        else       m_shadSurf.setUniformValue(m_locSurf.m_Light, 0);
        m_shadSurf.setUniformValue(m_locSurf.m_HasUniColor, 1);

        if(bTwoSided)
        {
            m_shadSurf.setUniformValue(m_locSurf.m_TwoSided, 1);
            glDisable(GL_CULL_FACE);
        }
        else
        {
            m_shadSurf.setUniformValue(m_locSurf.m_TwoSided, 0);
            glEnable(GL_CULL_FACE);
        }

        m_shadSurf.enableAttributeArray(m_locSurf.m_attrVertex);
        m_shadSurf.enableAttributeArray(m_locSurf.m_attrNormal);

        vbo.bind();
        {
            int nTriangles = vbo.size()/3/6/int(sizeof(float)); // three vertices and (3 position components+3 normal components)

            m_shadSurf.setAttributeBuffer(m_locSurf.m_attrVertex, GL_FLOAT, 0,                 3, 6*sizeof(GLfloat));
            m_shadSurf.setAttributeBuffer(m_locSurf.m_attrNormal, GL_FLOAT, 3*sizeof(GLfloat), 3, 6*sizeof(GLfloat));
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glPolygonOffset(DEPTHFACTOR, DEPTHUNITS);

            glDrawArrays(GL_TRIANGLES, 0, nTriangles*3); // 4 vertices defined but only 3 are used
        }
        vbo.release();
        glDisable(GL_POLYGON_OFFSET_FILL);

        m_shadSurf.disableAttributeArray(m_locSurf.m_attrVertex);
        m_shadSurf.disableAttributeArray(m_locSurf.m_attrNormal);
        m_shadSurf.setUniformValue(m_locSurf.m_TwoSided, 0); // leave things as they were
        glEnable(GL_CULL_FACE);
    }
    m_shadSurf.release();
}


void gl3dView::paintSegments(QOpenGLBuffer &vbo, LineStyle const &ls, bool bHigh)
{
    paintSegments(vbo, ls.m_Color, ls.m_Width, ls.m_Stipple, bHigh);
}


void gl3dView::paintSegments(QOpenGLBuffer &vbo, QColor const &clr, int thickness, Line::enumLineStipple stip, bool bHigh)
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    m_shadLine.bind();
    {
        vbo.bind();
        {
            m_shadLine.enableAttributeArray(m_locLine.m_attrVertex);
            m_shadLine.setAttributeBuffer(m_locLine.m_attrVertex, GL_FLOAT, 0, 3, 3*sizeof(GLfloat));

            int nSegs = vbo.size()/2/3/int(sizeof(float)); // 2 vertices and (3 position components)

            if(bHigh)
            {
                m_shadLine.setUniformValue(m_locLine.m_UniColor, Qt::red);

                if(m_bUse120StyleShaders)glLineWidth(thickness+2);
                else m_shadLine.setUniformValue(m_locLine.m_Thickness, thickness+2);
            }
            else
            {
                m_shadLine.setUniformValue(m_locLine.m_UniColor, clr);
            }
            if(m_bUse120StyleShaders)
            {
                glEnable(GL_LINE_STIPPLE);
                glLineStipple(1, GLStipple(stip));
                glLineWidth(float(thickness));
            }
            else
            {
                m_shadLine.setUniformValue(m_locLine.m_Thickness, thickness);
                m_shadLine.setUniformValue(m_locLine.m_Pattern, GLStipple(stip));
            }

            glDrawArrays(GL_LINES, 0, nSegs*2);// 4 vertices defined but only 3 are used
            glDisable(GL_LINE_STIPPLE);
        }
        vbo.release();

        m_shadLine.disableAttributeArray(m_locLine.m_attrVertex);
    }
    m_shadLine.release();
}



void gl3dView::paintCube(double x, double y, double z, double side, QColor const &clr, bool bLight)
{
    paintBox(x, y, z, side, side, side, clr, bLight);
}


void gl3dView::paintBox(double x, double y, double z, double dx, double dy, double dz, QColor const &clr, bool bLight)
{
    QMatrix4x4 vmMat(m_matView*m_matModel);
    QMatrix4x4 pvmMat(m_matProj*vmMat);

    QMatrix4x4 modelmat;
    modelmat.translate(x, y, z);
    modelmat.scale(dx, dy, dz);

    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix, vmMat*modelmat);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMat*modelmat);
    }
    m_shadSurf.release();
    paintTriangles3Vtx(m_vboCube, clr, false, bLight);

    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, vmMat*modelmat);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, pvmMat*modelmat);
    }
    m_shadLine.release();
    paintSegments(m_vboCubeEdges, W3dPrefsDlg::s_OutlineStyle);

    //leave things as they were
    modelmat.setToIdentity();
    m_shadSurf.bind();
    {
        m_shadSurf.setUniformValue(m_locSurf.m_vmMatrix, vmMat);
        m_shadSurf.setUniformValue(m_locSurf.m_pvmMatrix, pvmMat);
    }
    m_shadSurf.release();

    m_shadLine.bind();
    {
        m_shadLine.setUniformValue(m_locLine.m_vmMatrix, m_matView*m_matModel);
        m_shadLine.setUniformValue(m_locLine.m_pvmMatrix, m_matProj*m_matView*m_matModel);
    }
    m_shadLine.release();
}


void gl3dView::onDynamicIncrement()
{
    if(m_bDynRotation)
    {
        if(fabs(m_SpinInc.angle())<0.01)
        {
            stopDynamicTimer();
            update();
            return;
        }
        m_SpinInc = Quaternion(m_SpinInc.angle()*(1.0-s_SpinDamping), m_SpinInc.axis());
        m_ArcBall.applyRotation(m_SpinInc, false);
    }

    if(m_bDynTranslation)
    {
        double dist = m_Trans.norm()*m_glScalef;
        if(dist<0.01)
        {
            stopDynamicTimer();
            update();
            return;
        }
        m_glRotCenter += m_Trans/10.0;
        setViewportTranslation();

        m_Trans *= (1.0-s_SpinDamping);
    }

    if(m_bDynScaling)
    {
        if(abs(m_ZoomFactor)<10)
        {
            stopDynamicTimer();
            update();
            return;
        }

        double scalefactor(1.0-DisplayOptions::scaleFactor()/3.0 * m_ZoomFactor/120);

        m_glScalef *= scalefactor;
        m_ZoomFactor *= (1.0-s_SpinDamping);
    }

    update();
}



