/****************************************************************************

    Xfl3d
    Copyright (C) André Deperrois
    GNU General Public License v3

*****************************************************************************/

#include <cmath>

#include <QApplication>
#include <QOpenGLPaintDevice>

#include "gl2dview.h"
#include <xfl3d/controls/w3dprefs.h>
#include <xfl3d/views/gl3dview.h> // for the static variables
#include <xflcore/displayoptions.h>
#include <xflcore/trace.h>

#include <xflwidgets/customwts/intedit.h>
#include <xflwidgets/customwts/doubleedit.h>
#include <xflwidgets/wt_globals.h>



gl2dView::gl2dView(QWidget *pParent) : QOpenGLWidget(pParent)
{
    setFocusPolicy(Qt::WheelFocus);
    setCursor(Qt::CrossCursor);

    m_bInitialized = false;
    m_bDynTranslation = false;
    m_bDynScaling     = false;
    connect(&m_DynTimer, SIGNAL(timeout()), SLOT(onDynamicIncrement()));

    m_Scale = 1.0f;
}


void gl2dView::onDynamicIncrement()
{
    if(m_bDynTranslation)
    {
        double dist = sqrt(m_Trans.x()*m_Trans.x()+m_Trans.y()*m_Trans.y())*m_Scale;
        if(dist<0.01)
        {
            stopDynamicTimer();
            update();
            return;
        }
        m_ptOffset += m_Trans/10.0;

        m_Trans *= (1.0-gl3dView::spinDamping());
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

        m_Scale *= scalefactor;
        m_ZoomFactor *= (1.0-gl3dView::spinDamping());
    }

    update();
}


void gl2dView::startDynamicTimer()
{
    m_DynTimer.start(17);
    setMouseTracking(false);
}


void gl2dView::stopDynamicTimer()
{
    if(m_DynTimer.isActive())
    {
        m_DynTimer.stop();
    }
    m_bDynTranslation  = m_bDynScaling = false;
    setMouseTracking(true);
}


/* triangle strip */
void gl2dView::makeQuad()
{
    QVector<GLfloat> QuadVertexArray(12, 0);

    int iv = 0;
    QuadVertexArray[iv++] = 0.0f;
    QuadVertexArray[iv++] = 0.0f;

    QuadVertexArray[iv++] = m_rectView.left();
    QuadVertexArray[iv++] = m_rectView.top();

    QuadVertexArray[iv++] = m_rectView.right();
    QuadVertexArray[iv++] = m_rectView.top();

    QuadVertexArray[iv++] = m_rectView.right();
    QuadVertexArray[iv++] = m_rectView.bottom();

    QuadVertexArray[iv++] = m_rectView.left();
    QuadVertexArray[iv++] = m_rectView.bottom();

    QuadVertexArray[iv++] = m_rectView.left();
    QuadVertexArray[iv++] = m_rectView.top();

    Q_ASSERT(iv==QuadVertexArray.size());

    m_vboQuad.destroy();
    m_vboQuad.create();
    m_vboQuad.bind();
    m_vboQuad.allocate(QuadVertexArray.data(), QuadVertexArray.size() * sizeof(GLfloat));
    m_vboQuad.release();
}


void gl2dView::keyPressEvent(QKeyEvent *pEvent)
{
//    bool bCtrl = (pEvent->modifiers() & Qt::ControlModifier);
    switch (pEvent->key())
    {
        case Qt::Key_Escape:
        {
            if(windowFlags()&Qt::FramelessWindowHint)
            {
                setWindowFlags(Qt::Window);
                show(); //Note: This function calls setParent() when changing the flags for a window, causing the widget to be hidden. You must call show() to make the widget visible again..
                return;
            }
            break;
        }
        case Qt::Key_R:
        {
            stopDynamicTimer();
            m_ptOffset = defaultOffset();
            m_Scale = 1.0f;
            update();
            break;
        }
    }

    QOpenGLWidget::keyPressEvent(pEvent);
}


void gl2dView::showEvent(QShowEvent *pEvent)
{
    setFocus();
    m_ptOffset = defaultOffset();
    QOpenGLWidget::showEvent(pEvent);
}


void gl2dView::wheelEvent(QWheelEvent *pEvent)
{
    int dy = pEvent->pixelDelta().y();
    if(dy==0) dy = pEvent->angleDelta().y(); // pixeldelta usable on macOS and angleDelta on win/linux; depends also on driver and hardware

    if(gl3dView::bSpinAnimation() && abs(dy)>120)
    {
        m_bDynScaling = true;
        m_ZoomFactor = dy;

        startDynamicTimer();
    }
    else
    {
        float zoomfactor(1.0f);
        if(pEvent->angleDelta().y()>0) zoomfactor = 1.0/(1.0+DisplayOptions::scaleFactor());
        else                           zoomfactor = 1.0+DisplayOptions::scaleFactor();

        m_Scale *= zoomfactor;

        int a = rect().center().x();
        int b = rect().center().y();
        m_ptOffset.rx() = a + (m_ptOffset.x()-a);
        m_ptOffset.ry() = b + (m_ptOffset.y()-b);
        update();
    }

    pEvent->accept();
}


void gl2dView::resizeGL(int width, int height)
{
    QOpenGLWidget::resizeGL(width, height);

//    int side = std::min(width, height);
//    glViewport((width - side) / 2, (height - side) / 2, side, side);

    double w = double(width);
    double h = double(height);
    double s = 1.0;

    if(w>h)	m_GLViewRect.setRect(-s, s*h/w, s, -s*h/w);
    else    m_GLViewRect.setRect(-s*w/h, s, s*w/h, -s);
}



void gl2dView::mouseReleaseEvent(QMouseEvent *pEvent)
{
    if(gl3dView::bSpinAnimation())
    {
        int movetime = m_MoveTime.elapsed();
        if(movetime<300 && !m_PressedPoint.isNull())
        {
            if(pEvent->button()==Qt::LeftButton)
            {
                m_Trans = (pEvent->pos() - m_PressedPoint)/m_Scale;
                startDynamicTimer();
                m_bDynTranslation = true;
            }
        }
    }
    QApplication::restoreOverrideCursor();
}


void gl2dView::mousePressEvent(QMouseEvent *pEvent)
{
    m_LastPoint = pEvent->pos();
    m_PressedPoint = m_LastPoint;

    stopDynamicTimer();
    m_MoveTime.restart();
    QApplication::setOverrideCursor(Qt::ClosedHandCursor);
}


void gl2dView::mouseMoveEvent(QMouseEvent *pEvent)
{
    if(!hasFocus()) setFocus();

    QPoint point = pEvent->pos();

    if(pEvent->buttons() & Qt::LeftButton)
    {
        //translate the view

        QPoint delta = point - m_LastPoint;
        m_ptOffset += delta/m_Scale;

        m_LastPoint.rx() = point.x();
        m_LastPoint.ry() = point.y();

        update();
        return;
    }

    pEvent->accept();
}


void gl2dView::screenToViewport(QPoint const &point, QVector2D &real) const
{
    double h2 = double(geometry().height()) /2.0;
    double w2 = double(geometry().width())  /2.0;

    real.setX( (double(point.x()) - w2) / w2);
    real.setY(-(double(point.y()) - h2) / w2);
}


void gl2dView::screenToWorld(QPoint const &screenpt, QVector2D &pt)
{
    QMatrix4x4 m_matView, m;
    QVector4D in, out;
    QVector2D real;
    double w = m_rectView.width();
    QVector2D off((-m_ptOffset.x())/width()*w, m_ptOffset.y()/width()*w);

    screenToViewport(screenpt, real);
    in.setX(float(real.x()));
    in.setY(float(real.y()));
    in.setZ(0.0f);
    in.setW(1.0f);

    m_matView.scale(m_Scale, m_Scale, m_Scale);
    m_matView.translate(-off.x(), -off.y(), 0.0f);

    bool bInverted=false;
    QMatrix4x4 vmMatrix = m_matView;
    m = vmMatrix.inverted(&bInverted);
    out = m * in;

    if(fabs(double(out[3]))>PRECISION)
    {
        pt.setX(double(out[0]/out[3]));
        pt.setY(double(out[1]/out[3]));
    }
    else
    {
        pt.setX(double(out[0]));
        pt.setY(double(out[0]));
    }
}




