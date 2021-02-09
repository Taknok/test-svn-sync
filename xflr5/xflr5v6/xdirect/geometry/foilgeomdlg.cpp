/****************************************************************************

    FoilGeomDlg Class
    Copyright (C) 2008-2016 Andre Deperrois 

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
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>

#include <xfoil.h>
#include "foilgeomdlg.h"


#include <misc/text/doubleedit.h>
#include <xflobjects/objects2d/foil.h>


XFoil *FoilGeomDlg::s_pXFoil;


FoilGeomDlg::FoilGeomDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("Foil Geometry"));

    m_pParent = pParent;

    setupLayout();

    connect(m_pRestoreButton, SIGNAL(clicked()),this, SLOT(onRestore()));
    connect(m_OKButton, SIGNAL(clicked()),this, SLOT(onOK()));
    connect(m_CancelButton, SIGNAL(clicked()), this, SLOT(reject()));

    connect(m_pctrlCamber, SIGNAL(editingFinished()), this, SLOT(onCamber()));
    connect(m_pctrlXCamber, SIGNAL(editingFinished()), this, SLOT(onXCamber()));
    connect(m_pctrlThickness, SIGNAL(editingFinished()), this, SLOT(onThickness()));
    connect(m_pctrlXThickness, SIGNAL(editingFinished()), this, SLOT(onXThickness()));

    connect(m_pctrlCamberSlide, SIGNAL(sliderMoved(int)), this, SLOT(onCamberSlide(int)));
    connect(m_pctrlXCamberSlide, SIGNAL(sliderMoved(int)), this, SLOT(onXCamberSlide(int)));
    connect(m_pctrlThickSlide, SIGNAL(sliderMoved(int)), this, SLOT(onThickSlide(int)));
    connect(m_pctrlXThickSlide, SIGNAL(sliderMoved(int)), this, SLOT(onXThickSlide(int)));

}


void FoilGeomDlg::setupLayout()
{
    QGroupBox *pCamberGroup = new QGroupBox(tr("Camber"));
    {
        QVBoxLayout *pCamberData = new QVBoxLayout;
        {
            m_pctrlCamberSlide = new QSlider;
            m_pctrlCamberSlide->setOrientation(Qt::Horizontal);
            m_pctrlCamberSlide->setTickPosition(QSlider::TicksBelow);
            m_pctrlCamberSlide->setMinimumWidth(200);
            m_pctrlCamber =new DoubleEdit;
            m_pctrlXCamberSlide = new QSlider;
            m_pctrlXCamberSlide->setOrientation(Qt::Horizontal);
            m_pctrlXCamberSlide->setTickPosition(QSlider::TicksBelow);
            m_pctrlXCamberSlide->setMinimumWidth(200);
            m_pctrlXCamber = new DoubleEdit;
            QLabel *lab1 = new QLabel(tr("Value"));
            QLabel *lab2 = new QLabel(tr("%Chord"));
            QLabel *lab3 = new QLabel(tr("0%"));
            QLabel *lab4 = new QLabel(tr("10%"));
            QLabel *lab5 = new QLabel(tr("Max x-pos"));
            QLabel *lab6 = new QLabel(tr("%Chord"));
            QLabel *lab7 = new QLabel(tr("0%"));
            QLabel *lab8 = new QLabel(tr("100%"));
            lab1->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            lab5->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            lab3->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            lab7->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            lab1->setMinimumWidth(70);
            lab2->setMinimumWidth(70);
            lab3->setMinimumWidth(50);
            lab4->setMinimumWidth(50);
            lab5->setMinimumWidth(70);
            lab6->setMinimumWidth(70);
            lab7->setMinimumWidth(50);
            lab8->setMinimumWidth(50);

            QHBoxLayout *pCambValLayout = new QHBoxLayout;
            {
                pCambValLayout->addWidget(lab1);
                pCambValLayout->addWidget(m_pctrlCamber);
                pCambValLayout->addWidget(lab2);
                pCambValLayout->addStretch(1);
                pCambValLayout->addWidget(lab3);
                pCambValLayout->addWidget(m_pctrlCamberSlide);
                pCambValLayout->addWidget(lab4);
            }

            QHBoxLayout *XCambVal = new QHBoxLayout;
            {
                XCambVal->addWidget(lab5);
                XCambVal->addWidget(m_pctrlXCamber);
                XCambVal->addWidget(lab6);
                XCambVal->addStretch(1);
                XCambVal->addWidget(lab7);
                XCambVal->addWidget(m_pctrlXCamberSlide);
                XCambVal->addWidget(lab8);
            }
            pCamberData->addLayout(pCambValLayout);
            pCamberData->addLayout(XCambVal);
        }
        pCamberGroup->setLayout(pCamberData);
    }


    QGroupBox *pThicknessGroup = new QGroupBox(tr("Thickness"));
    {
        QVBoxLayout *pThicknessData = new QVBoxLayout;
        {
            m_pctrlThickSlide = new QSlider;
            m_pctrlThickSlide->setOrientation(Qt::Horizontal);
            m_pctrlThickSlide->setTickPosition(QSlider::TicksBelow);
            m_pctrlThickSlide->setMinimumWidth(200);
            m_pctrlThickness =new DoubleEdit;
            m_pctrlXThickSlide = new QSlider;
            m_pctrlXThickSlide->setOrientation(Qt::Horizontal);
            m_pctrlXThickSlide->setTickPosition(QSlider::TicksBelow);
            m_pctrlXThickSlide->setMinimumWidth(200);
            m_pctrlXThickness = new DoubleEdit;
            QLabel *lab11 = new QLabel(tr("Value"));
            QLabel *lab12 = new QLabel(tr("%Chord"));
            QLabel *lab13 = new QLabel("0%");
            QLabel *lab14 = new QLabel("20%");
            QLabel *lab15 = new QLabel(tr("Max x-pos"));
            QLabel *lab16 = new QLabel(tr("%Chord"));
            QLabel *lab17 = new QLabel("0%");
            QLabel *lab18 = new QLabel("100%");

            lab11->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            lab15->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            lab13->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            lab17->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            lab11->setMinimumWidth(70);
            lab12->setMinimumWidth(70);
            lab13->setMinimumWidth(50);
            lab14->setMinimumWidth(50);
            lab15->setMinimumWidth(70);
            lab16->setMinimumWidth(70);
            lab17->setMinimumWidth(50);
            lab18->setMinimumWidth(50);

            QHBoxLayout *ThickVal = new QHBoxLayout;
            {
                ThickVal->addWidget(lab11);
                ThickVal->addWidget(m_pctrlThickness);
                ThickVal->addWidget(lab12);
                ThickVal->addStretch(1);
                ThickVal->addWidget(lab13);
                ThickVal->addWidget(m_pctrlThickSlide);
                ThickVal->addWidget(lab14);
            }

            QHBoxLayout *pXThickVal = new QHBoxLayout;
            {
                pXThickVal->addWidget(lab15);
                pXThickVal->addWidget(m_pctrlXThickness);
                pXThickVal->addWidget(lab16);
                pXThickVal->addStretch(1);
                pXThickVal->addWidget(lab17);
                pXThickVal->addWidget(m_pctrlXThickSlide);
                pXThickVal->addWidget(lab18);
            }

            pThicknessData->addLayout(ThickVal);
            pThicknessData->addLayout(pXThickVal);
        }
        pThicknessGroup->setLayout(pThicknessData);
    }

    QHBoxLayout *pCommandButtons = new QHBoxLayout;
    {
        m_OKButton      = new QPushButton(tr("OK"));
        m_CancelButton  = new QPushButton(tr("Cancel"));
        m_pRestoreButton  = new QPushButton(tr("Restore"));
        pCommandButtons->addStretch(1);
        pCommandButtons->addWidget(m_pRestoreButton);
        pCommandButtons->addStretch(1);
        pCommandButtons->addWidget(m_OKButton);
        pCommandButtons->addStretch(1);
        pCommandButtons->addWidget(m_CancelButton);
        pCommandButtons->addStretch(1);
    }


    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(pCamberGroup);
        pMainLayout->addStretch(1);
        pMainLayout->addWidget(pThicknessGroup);
        pMainLayout->addStretch(1);
        pMainLayout->addLayout(pCommandButtons);
        setLayout(pMainLayout);
    }

    setMinimumWidth(500);
    setMinimumHeight(300);

    m_pctrlCamber->setPrecision(2);
    m_pctrlXCamber->setPrecision(2);
    m_pctrlThickness->setPrecision(2);
    m_pctrlXThickness->setPrecision(2);

    m_pctrlCamberSlide->setRange(0,100);
    m_pctrlCamberSlide->setTickInterval(5);
    m_pctrlXCamberSlide->setRange(0,1000);
    m_pctrlXCamberSlide->setTickInterval(100);
    m_pctrlThickSlide->setRange(0,200);
    m_pctrlThickSlide->setTickInterval(5);
    m_pctrlXThickSlide->setRange(0,1000);
    m_pctrlXThickSlide->setTickInterval(100);
}



/*
    Modification of the airfoil with xfoils tcset and hipnt

    There are several airfoils involved:

       memFoil      holds the original airfoil - read only
       baseFoil     either a copy of memFoil or a smoothed version of it
       bufferFoil   the foil which is displayed during modification (slave)
       XFoil        buffer airfoil on which XFoil is doing it's modification
                    attention: pangen output is in current not in buffer airfoil.

    XFoil->hipnt to set max. thickness or camber position is quite sensible to not
    well defined airfoils especially at leading edge. This may result in garbage results.
    If this is detected a re-paneling of the airfoil is made to smooth the outline.

    Modification of geometry is always based on the original airfoil data
         (repeated modification on the same xfoil foil leads to strange artefacts)

*/
void FoilGeomDlg::apply()
{

    if (m_modifying) return;                        // avoid multi threaded xfoil modification of geometry
                                                    // if user is playing around with the slider...

    m_modifying = true;

    // prepare xfoil foil for modification
    s_pXFoil->initialize();
    s_pXFoil->initXFoilGeometry(m_pBaseFoil->n, m_pBaseFoil->x, m_pBaseFoil->y, m_pBaseFoil->nx, m_pBaseFoil->ny);

    // do it
    s_pXFoil->hipnt(m_fXCamber, m_fXThickness);     // xfoil hipnt is the most sensitive routine - better do it first
    s_pXFoil->tcset(m_fCamber, m_fThickness);       // xfoil tcset to change camber or thickness


    // output sanity

    // this never should happen
    if(s_pXFoil->nb != m_pMemFoil->n)
    {
        QMessageBox::information(window(), tr("Error"), tr("Panel number changed during modification"));
    }

    // is output corrupted? if yes re-panel base airfoil for the next try
    else if (!isXFoilOk())
    {
        s_pXFoil->initialize();
        s_pXFoil->initXFoilGeometry(m_pMemFoil->n, m_pMemFoil->x, m_pMemFoil->y, m_pBufferFoil->nx, m_pBufferFoil->ny);
        s_pXFoil->npan = s_pXFoil->nb;
        s_pXFoil->pangen();
        qDebug ("FoilGeomDlg: pangen with nb =%3d due to corrupted output airfoil", s_pXFoil->nb);

        for (int j=0; j< s_pXFoil->n; j++)
        {
            m_pBaseFoil->x[j] = s_pXFoil->x[j+1];
            m_pBaseFoil->y[j] = s_pXFoil->y[j+1];
        }
    }
    // everything ok - update bufferFoil to display for user
    else
    {
        for (int j=0; j< s_pXFoil->nb; j++)
        {
            m_pBufferFoil->xb[j] = s_pXFoil->xb[j+1];
            m_pBufferFoil->yb[j] = s_pXFoil->yb[j+1];
        }
        m_pBufferFoil->nb = s_pXFoil->nb;
        m_pBufferFoil->initFoil();
        m_pBufferFoil->setFlap();
    }

    m_bModified = true;
    m_pParent->update();
    m_modifying = false;
}



bool FoilGeomDlg::isXFoilOk ()
{
    for (int j=0; j< s_pXFoil->nb; j++)
    {
        if (isnan(s_pXFoil->xb[j+1]) || isnan(s_pXFoil->xb[j+1]))
        {
            return false;
        }
    }
    return true;
}



void FoilGeomDlg::keyPressEvent(QKeyEvent *event)
{
    // Prevent Return Key from closing App
    switch (event->key())
    {
        case Qt::Key_Escape:
        {
            done(0);
            return;
        }
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_OKButton->hasFocus() && !m_CancelButton->hasFocus())
            {
                apply();
                m_OKButton->setFocus();
            }
            else
            {
                QDialog::accept();
            }
            break;
        }
        default:
            event->ignore();
            break;
    }
}


void FoilGeomDlg::initDialog()
{
    // round values to be consistend to entry field value decimals
    m_fCamber     = round (m_pMemFoil->camber()     * 10000) / 10000 ;
    m_fThickness  = round (m_pMemFoil->thickness()  * 10000) / 10000 ;
    m_fXCamber    = round (m_pMemFoil->xCamber()    * 10000) / 10000 ;
    m_fXThickness = round (m_pMemFoil->xThickness() * 10000) / 10000 ;

    if(qAbs(m_fCamber) <0.0001)
    {
        m_pctrlCamberSlide->setEnabled(false);
        m_pctrlCamber->setEnabled(false);
    }

    // set values into entry fields and sliders
    m_pctrlCamber->setValue(m_fCamber*100.0);
    m_pctrlThickness->setValue(m_fThickness*100.0);
    m_pctrlXCamber->setValue(m_fXCamber*100.0);
    m_pctrlXThickness->setValue(m_fXThickness*100.0);

    m_pctrlCamberSlide->setSliderPosition(int(m_fCamber*1000.0));
    m_pctrlThickSlide->setSliderPosition(int(m_fThickness*1000.0));
    m_pctrlXCamberSlide->setSliderPosition(int(m_fXCamber*1000.0));
    m_pctrlXThickSlide->setSliderPosition(int(m_fXThickness*1000.0));

    m_modifying = false;
    m_pBaseFoil = new Foil();
    m_pBaseFoil->copyFoil(m_pMemFoil);
}


void FoilGeomDlg::onRestore()
{
    m_pBufferFoil->copyFoil(m_pMemFoil);
    m_pBufferFoil->setEditStyle();

    initDialog ();

    m_bModified = false;
    m_pParent->update();
}

void FoilGeomDlg::onOK()
{
    apply();
    if(!m_bModified) done(0);
    else done(1);
}



void FoilGeomDlg::onCamber()
{
    m_fCamber = m_pctrlCamber->value() / 100.0;
    m_pctrlCamberSlide->blockSignals( true );
    m_pctrlCamberSlide->setValue(int(m_fCamber*1000.0));
    m_pctrlCamberSlide->blockSignals( false );
    apply();
}

void FoilGeomDlg::onCamberSlide(int pos)
{
    m_fCamber = double(pos)/1000.0;
    m_pctrlCamber->blockSignals( true );
    m_pctrlCamber->setValue(m_fCamber * 100.0);
    m_pctrlCamber->blockSignals( false );
    apply();
}

void FoilGeomDlg::onThickness()
{
    m_fThickness = m_pctrlThickness->value() / 100.0;
    m_pctrlThickSlide->blockSignals( true );
    m_pctrlThickSlide->setValue(int(m_fThickness*1000.0));
    m_pctrlThickSlide->blockSignals( false );
    apply();
}

void FoilGeomDlg::onThickSlide(int pos)
{
    m_fThickness = double(pos)/1000.0;
    m_pctrlThickness->blockSignals( true );
    m_pctrlThickness->setValue(m_fThickness *100.0);
    m_pctrlThickness->blockSignals( false );
    apply();
}

void FoilGeomDlg::onXCamberSlide(int pos)
{
    m_fXCamber = double(pos)/1000.0;
    m_pctrlXCamber->blockSignals( true );
    m_pctrlXCamber->setValue(m_fXCamber * 100.0);
    m_pctrlXCamber->blockSignals( false );
    apply();
}

void FoilGeomDlg::onXCamber()
{
    m_fXCamber = m_pctrlXCamber->value() / 100.0;
    m_pctrlXCamberSlide->blockSignals( true );
    m_pctrlXCamberSlide->setValue(int(m_fXCamber*1000.0));
    m_pctrlXCamberSlide->blockSignals( false );
    apply();
}

void FoilGeomDlg::onXThickSlide(int pos)
{
    m_fXThickness = double(pos)/1000.0;
    m_pctrlXThickness->blockSignals( true );
    m_pctrlXThickness->setValue(m_fXThickness * 100.0);
    m_pctrlXThickness->blockSignals( false );
    apply();
}

void FoilGeomDlg::onXThickness()
{
    m_fXThickness = m_pctrlXThickness->value() / 100.0;
    m_pctrlXThickSlide->blockSignals( true );
    m_pctrlXThickSlide->setValue(int(m_fXThickness*1000.0));
    m_pctrlXThickSlide->blockSignals( false );
    apply();
}


