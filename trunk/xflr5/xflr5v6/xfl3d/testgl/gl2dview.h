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

        void resizeGL(int width, int height)  override;

    protected:

        void makeQuad();
        void startDynamicTimer();
        void stopDynamicTimer();

        void screenToViewport(QPoint const &point, QVector2D &real) const;
        void screenToWorld(QPoint const &screenpt, QVector2D &pt);

        virtual QPointF defaultOffset() = 0;

    protected slots:
        void onDynamicIncrement();

    signals:
        void ready2d() const;

    protected:
        QOpenGLVertexArrayObject m_vao; /** generic vao required for the core profile >3.x*/
        QOpenGLBuffer m_vboQuad;

        QPoint m_LastPoint, m_PressedPoint;
        float m_Scale;
        QPointF m_ptOffset;


        QElapsedTimer m_MoveTime;
        QTimer m_DynTimer;

        QPointF m_Trans;
        bool m_bDynTranslation;

        bool m_bDynScaling;
        float m_ZoomFactor;

        QRectF m_rectView;
        QRectF m_GLViewRect;    /**< The OpenGl viewport.*/

        bool m_bInitialized;
};

