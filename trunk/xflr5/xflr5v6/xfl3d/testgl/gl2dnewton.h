/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois
    GNU General Public License v3

*****************************************************************************/

#pragma once

#include <QOpenGLShaderProgram>
#include <QSettings>
#include <QCheckBox>

#include <xfl3d/testgl/gl2dview.h>

#include <QLabel>

class ColorMenuBtn;
class IntEdit;
class DoubleEdit;

class gl2dNewton : public gl2dView
{
    Q_OBJECT
    public:
        gl2dNewton(QWidget *pParent = nullptr);

        void setDefaultOffset() override {m_ptOffset = QPointF(0.25*width(),0);}
        void initializeGL() override;
        void paintGL()  override;

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);


    private slots:
        void onAnimate(bool bAnimate);
        void onMoveRoots();

    private:

        QOpenGLShaderProgram m_shadNewton;

        // shader uniforms
        int m_locIters;
        int m_locTolerance;
        int m_locColor[3];
        int m_locRoot[3];
        int m_locViewTrans;
        int m_locViewScale;
        int m_locViewRatio;

        //shader attributes
        int m_attrVertexPosition;

        QVector2D m_Root[3];
        int m_Time;
        double m_omega[6];

        IntEdit *m_pieMaxIter;
        DoubleEdit *m_pdeTolerance;
        QLabel *m_plabScale;
        QCheckBox *m_pchAnimate;

        QTimer m_Timer;

        static int s_MaxIter;
        static float s_Tolerance;
        static QColor s_Colors[3];

};
