/****************************************************************************

    GLLightDlg class
    Copyright (C) 2009 Andre Deperrois xflr5@yahoo.com

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


#pragma once

#include <QDialog>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QSettings>


class gl3dView;
class ExponentialSlider;
class DoubleEdit;


struct Light
{
    float m_Ambient=0.3f, m_Diffuse=1.2f, m_Specular=0.5f; // the light intensities
    float m_Red=1.0f, m_Green=1.0f, m_Blue=1.0f; // the color of light
    float m_X=0.1f, m_Y=0.3f, m_Z=0.5f; // coordinates in camera space
    bool m_bIsLightOn=true;
};

struct Attenuation
{
        float m_Constant=1.0, m_Linear=0.5, m_Quadratic=0.0;
};

class GLLightDlg : public QDialog
{
    Q_OBJECT
    friend class Miarex;
    friend class gl3dView;

    public:
        GLLightDlg(QWidget *pParent=nullptr);
        void apply();
        void readParams(void);
        void setDefaults();
        void setModelSize(double span);
        void setParams(void);
        void setgl3dView(gl3dView*pglView) {m_pglView = pglView;}


        static bool loadSettings(QSettings &settings);
        static bool saveSettings(QSettings &settings);

        static bool isLightOn() {return s_Light.m_bIsLightOn;}
        static void setLightOn(bool bLight) {s_Light.m_bIsLightOn = bLight;}

    private:
        void connectSignals();
        void setupLayout();
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        QSize minimumSizeHint() const override {return QSize(300, 350);}
        QSize sizeHint() const override{return QSize(400, 400);}
        void setEnabled();
        void setLabels();

    private slots:
        void onChanged();
        void onDefaults();
        void onLight();

    private:
        QSlider *m_pslRed, *m_pslGreen, *m_pslBlue;
        QSlider  *m_pslMatShininess;
        ExponentialSlider *m_peslLightAmbient, *m_peslLightDiffuse, *m_peslLightSpecular;
        ExponentialSlider *m_prslXLight, *m_peslYLight, *m_peslZLight;

        QCheckBox *m_plabLight;
        QLabel *m_plabLightAmbient, *m_plabLightDiffuse, *m_plabLightSpecular;
        QLabel *m_plabPosXValue, *m_plabPosYValue, *m_plabPosZValue;
        QLabel *m_pctrlLightRed, *m_plabLightGreen, *m_plabLightBlue;
        QLabel *m_plabMatShininess;

        QPushButton *m_ppbDefaults, *m_ppbClose;

        DoubleEdit *m_pdeConstantAttenuation , *m_pdeLinearAttenuation , *m_pdeQuadAttenuation;


    private:
        gl3dView *m_pglView;

        static Light s_Light;
        static Attenuation s_Attenuation;
        static int s_iShininess;
        float m_ModelSize;

};



