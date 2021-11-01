/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois
    GNU General Public License v3

*****************************************************************************/

#pragma once

#include <QOpenGLShaderProgram>
#include <QSettings>

#include <xfl3d/testgl/gl2dview.h>

#include <QLabel>


class IntEdit;
class DoubleEdit;

class gl2dFractal : public gl2dView
{
    Q_OBJECT
    public:
        gl2dFractal(QWidget *pParent = nullptr);

        QPointF defaultOffset() override {return QPointF(+0.5*float(width()),0.0f);}

        void initializeGL() override;
        void paintGL()  override;

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:

        QOpenGLShaderProgram m_shadFrac;

        // shader uniforms
        int m_locIters;
        int m_locLength;
        int m_locViewTrans;
        int m_locViewScale;
        int m_locViewRatio;

        //shader attributes
        int m_attrVertexPosition;


        IntEdit *m_pieMaxIter;
        DoubleEdit *m_pdeMaxLength;
        QLabel *m_plabScale;

        static int s_MaxIter;
        static float s_MaxLength;

};


