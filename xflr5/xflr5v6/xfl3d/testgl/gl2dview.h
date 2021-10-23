/****************************************************************************

    Xfl3d
    Copyright (C) André Deperrois
    GNU General Public License v3

*****************************************************************************/

#pragma once

#include <QOpenGLWidget>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QTimer>
#include <QElapsedTimer>


class gl2dView : public QOpenGLWidget
{
    Q_OBJECT
    public:
        gl2dView(QWidget *pParent = nullptr);

        virtual QSize sizeHint() const override {return QSize(1500, 1100);}

        void showEvent(QShowEvent *pEvent) override;
        void wheelEvent(QWheelEvent *pEvent) override;
        void mousePressEvent(QMouseEvent *pEvent) override;
        void mouseReleaseEvent(QMouseEvent *pEvent) override;
        void mouseMoveEvent(QMouseEvent *pEvent) override;
        void keyPressEvent(QKeyEvent *pEvent) override;


        static void setScaleFactor(double f) {s_ScaleFactor=float(f);}

    protected:

        void makeQuad();
        void startDynamicTimer();
        void stopDynamicTimer();

        virtual void setDefaultOffset() = 0;

    protected slots:
        void onDynamicIncrement();

    protected:
        QOpenGLVertexArrayObject m_vao; /** generic vao required for the core profile >3.x*/
        QOpenGLBuffer m_vboQuad;

        QPoint m_LastPoint, m_PressedPoint;
        float m_Scale;
        QPointF m_ptOffset;

        QRectF m_rectView;

        QElapsedTimer m_MoveTime;
        QTimer m_DynTimer;

        QPointF m_Trans;
        bool m_bDynTranslation;

        bool m_bDynScaling;
        float m_ZoomFactor;

        static float s_ScaleFactor;

};


