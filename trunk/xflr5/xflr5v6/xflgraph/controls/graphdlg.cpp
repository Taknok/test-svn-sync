/****************************************************************************

    GraphDlg  Classes
    Copyright (C) 2009-2019 Andre Deperrois

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


#include <QFontDialog>
#include <QColorDialog>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QDebug>

#include <xflgraph/graph.h>
#include <xflgraph/controls/graphdlg.h>
#include <miarex/miarex.h>
#include <xflwidgets/color/colorbutton.h>
#include <xflwidgets/line/linebtn.h>
#include <xflwidgets/line/linepickerdlg.h>
#include <xflwidgets/text/doubleedit.h>
#include <xflwidgets/text/intedit.h>
#include <xflwidgets/text/textclrbtn.h>
#include <xflobjects/objects2d/polar.h>
#include <xflobjects/objects3d/wpolar.h>


int GraphDlg::s_iActivePage = 0;


GraphDlg::GraphDlg(QWidget *pParent): QDialog(pParent)
{
    setWindowTitle(tr("Graph Settings"));

    m_pParent = pParent;

    m_pGraph    = nullptr;
    m_bApplied = true;
    m_bVariableChanged = false;

    m_XSel = 0;
    m_YSel = 1;

    m_pTitleFont = m_pLabelFont = nullptr;

    setupLayout();
    connectSignals();
}


void GraphDlg::connectSignals()
{
    connect(m_ptcbTitleClr, SIGNAL(clickedTB()),  this, SLOT(onTitleColor()));
    connect(m_ptcbLabelClr, SIGNAL(clickedTB()),  this, SLOT(onLabelColor()));

    connect(m_ppbTitleButton, SIGNAL(clicked()),  this, SLOT(onTitleFont()));
    connect(m_ppbLabelButton, SIGNAL(clicked()),  this, SLOT(onLabelFont()));

    connect(m_pchXAuto, SIGNAL(clicked()), this, SLOT(onAutoX()));
    connect(m_pchYAuto, SIGNAL(clicked()), this, SLOT(onAutoY()));
    connect(m_pchYInverted, SIGNAL(clicked()), this, SLOT(onYInverted()));

    connect(m_pchXMajGridShow, SIGNAL(stateChanged(int)), this, SLOT(onXMajGridShow(int)));
    connect(m_pchYMajGridShow, SIGNAL(stateChanged(int)), this, SLOT(onYMajGridShow(int)));
    connect(m_pchXMinGridShow, SIGNAL(stateChanged(int)), this, SLOT(onXMinGridShow(int)));
    connect(m_pchYMinGridShow, SIGNAL(stateChanged(int)), this, SLOT(onYMinGridShow(int)));

    connect(m_plbAxisStyle, SIGNAL(clickedLB()), this, SLOT(onAxisStyle()));
    connect(m_plbXMajGridStyle, SIGNAL(clickedLB()), this, SLOT(onXMajGridStyle()));
    connect(m_plbYMajGridStyle, SIGNAL(clickedLB()), this, SLOT(onYMajGridStyle()));
    connect(m_plbXMinGridStyle, SIGNAL(clickedLB()), this, SLOT(onXMinGridStyle()));
    connect(m_plbYMinGridStyle, SIGNAL(clickedLB()), this, SLOT(onYMinGridStyle()));

    connect(m_pchAutoXMinUnit, SIGNAL(clicked()), this, SLOT(onAutoMinGrid()));
    connect(m_pchAutoYMinUnit, SIGNAL(clicked()), this, SLOT(onAutoMinGrid()));

    connect(m_pchGraphBorder, SIGNAL(stateChanged(int)), this, SLOT(onGraphBorder(int)));
    connect(m_pcbGraphBack, SIGNAL(clicked()), this, SLOT(onGraphBackColor()));
    connect(m_plbBorderStyle, SIGNAL(clicked()), this, SLOT(onBorderStyle()));

    /*    connect(m_pctrlXSel, SIGNAL(itemActivated ( QListWidgetItem*)), SLOT(OnVariableChanged()));
    connect(m_pctrlYSel, SIGNAL(itemActivated ( QListWidgetItem*)), SLOT(OnVariableChanged()));
    connect(m_pctrlXSel, SIGNAL(itemClicked ( QListWidgetItem*)), SLOT(OnVariableChanged()));
    connect(m_pctrlYSel, SIGNAL(itemClicked ( QListWidgetItem*)), SLOT(OnVariableChanged()));*/
    connect(m_plwXSel, SIGNAL(itemSelectionChanged()), SLOT(onVariableChanged()));
    connect(m_plwYSel, SIGNAL(itemSelectionChanged()), SLOT(onVariableChanged()));

    connect(m_plwXSel, SIGNAL(itemDoubleClicked (QListWidgetItem *)), SLOT(onOK()));
    connect(m_plwYSel, SIGNAL(itemDoubleClicked (QListWidgetItem *)), SLOT(onOK()));
}


void GraphDlg::fillVariableList()
{
    m_plwXSel->clear();
    m_plwYSel->clear();

    switch(m_pGraph->graphType())
    {
        case  GRAPH::INVERSEGRAPH:
        {
            m_plwXSel->addItem("X - Chord");
            m_plwYSel->addItem("Q - Velocity");
            break;
        }
        case GRAPH::OPPGRAPH:
        {
            //foil oppoint graph variables
            m_plwXSel->addItem("X - chord");
            m_plwYSel->addItem("Cp");
            m_plwYSel->addItem("Q - Velocity");
            m_plwYSel->addItem("sqrt(Max. Shear Coefficient)");
            m_plwYSel->addItem("Top Side D* and Theta");
            m_plwYSel->addItem("Bottom Side D* and Theta");
            m_plwYSel->addItem("Log(Re_Theta)");
            m_plwYSel->addItem("Re_Theta");
            m_plwYSel->addItem("Amplification Ratio");
            m_plwYSel->addItem("Dissipation Coefficient");
            m_plwYSel->addItem("Wall shear stress");
            m_plwYSel->addItem("Edge Velocity");
            m_plwYSel->addItem("Kinematic Shape Parameter");
            break;
        }
        case GRAPH::POLARGRAPH:
        {
            //foil polar graph variables
            for(int iVar=0; iVar<15; iVar++)
            {
                m_plwXSel->addItem(Polar::variableName(iVar));
                m_plwYSel->addItem(Polar::variableName(iVar));
            }
            break;
        }
        case GRAPH::POPPGRAPH:
        {
            //wing graph variable
            m_plwXSel->addItem(tr("Y - span"));

            m_plwYSel->addItem(tr("Induced Angle"));                        //0
            m_plwYSel->addItem(tr("Total Angle"));                        //1
            m_plwYSel->addItem(tr("Local lift coef."));                    //2
            m_plwYSel->addItem(tr("Local Lift C.Cl/M.A.C."));                //3
            m_plwYSel->addItem(tr("Airfoil viscous drag coef."));            //4
            m_plwYSel->addItem(tr("Induced drag coef."));                    //5
            m_plwYSel->addItem(tr("Total drag coef."));                    //6
            m_plwYSel->addItem(tr("Local Drag C.Cd/M.A.C."));              //7
            m_plwYSel->addItem(tr("Airfoil Pitching moment coef."));       //8
            m_plwYSel->addItem(tr("Total Pitching moment coef."));         //9
            m_plwYSel->addItem(tr("Reynolds"));                            //10
            m_plwYSel->addItem(tr("Top Transition x-pos%"));               //11
            m_plwYSel->addItem(tr("Bottom Transition x-pos%"));            //12
            m_plwYSel->addItem(tr("Centre of Pressure x-pos%"));           //13
            m_plwYSel->addItem(tr("Bending moment"));                      //14
            break;
        }
        case GRAPH::WPOLARGRAPH:
        {
            //WingPolar Graph Variables
            for(int iVar=0; iVar<50; iVar++)
            {
                m_plwXSel->addItem(Miarex::WPolarVariableName(iVar));
                m_plwYSel->addItem(Miarex::WPolarVariableName(iVar));
            }
            break;
        }
        case GRAPH::CPGRAPH:
        {
            m_plwXSel->addItem(tr("X - Chord"));
            m_plwYSel->addItem(tr("Cp"));
            break;
        }
        case GRAPH::STABTIMEGRAPH:
        {
            m_plwXSel->addItem(tr("X - Chord"));
            m_plwYSel->addItem(tr("Cp"));
            break;
        }
        case GRAPH::OTHERGRAPH:
            return;
    }
    m_plwXSel->adjustSize();
    m_plwYSel->adjustSize();
}


void GraphDlg::reject()
{
    m_pGraph->copySettings(&m_SaveGraph);
    done(QDialog::Rejected);
}


void GraphDlg::showEvent(QShowEvent *pEvent)
{
    m_pTabWidget->setCurrentIndex(s_iActivePage);
    pEvent->ignore();
}


void GraphDlg::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_pButtonBox->hasFocus())
            {
                m_pButtonBox->setFocus();
                return;
            }
            else
            {
                onOK();
                return;
            }
        }
        case Qt::Key_Escape:
        {
            reject();
            break;
        }
        default:
            pEvent->ignore();
    }
}


void GraphDlg::onActivePage(int index)
{
    s_iActivePage = index;
}


void GraphDlg::onAutoMinGrid()
{
    bool bAuto;
    bAuto = m_pchAutoXMinUnit->isChecked();
    m_pGraph->setAutoXMinUnit(bAuto);
    m_pdeXMinorUnit->setEnabled(!bAuto);

    bAuto = m_pchAutoYMinUnit->isChecked();
    m_pGraph->setAutoYMinUnit(bAuto);
    m_pdeYMinorUnit->setEnabled(!bAuto);
}


void GraphDlg::onAutoX()
{
    bool bAuto = m_pchXAuto->checkState() == Qt::Checked;
    m_pdeXMin->setEnabled(!bAuto);
    m_pdeXMax->setEnabled(!bAuto);
    m_pdeXUnit->setEnabled(!bAuto);
    m_pdeXOrigin->setEnabled(!bAuto);
    setApplied(false);
}


void GraphDlg::onAutoY()
{
    bool bAuto = m_pchYAuto->checkState() == Qt::Checked;
    m_pdeYMin->setEnabled(!bAuto);
    m_pdeYMax->setEnabled(!bAuto);
    m_pdeYUnit->setEnabled(!bAuto);
    m_pdeYOrigin->setEnabled(!bAuto);
    setApplied(false);
}


void GraphDlg::onAxisStyle()
{
    LinePickerDlg dlg(this);
    dlg.initDialog(0, m_pGraph->axisStyle(), m_pGraph->axisWidth(), m_pGraph->axisColor(), false, false);

    if(QDialog::Accepted==dlg.exec())
    {
        m_pGraph->setAxisStyle(dlg.theStyle());
        m_plbAxisStyle->setStipple(dlg.lineStipple());
        m_plbAxisStyle->setWidth(dlg.lineWidth());
        m_plbAxisStyle->setColor(dlg.lineColor());
        setApplied(false);
    }
}


void GraphDlg::onBorderStyle()
{
    LinePickerDlg dlg(this);

    QColor color;
    int s = m_pGraph->borderStyle();
    int w = m_pGraph->borderWidth();
    color = m_pGraph->borderColor();
    dlg.initDialog(0,s,w,color, false, false);

    if(QDialog::Accepted==dlg.exec())
    {
        m_pGraph->setBorderStyle(dlg.theStyle());
        m_plbBorderStyle->setStipple(dlg.lineStipple());
        m_plbBorderStyle->setWidth(dlg.lineWidth());
        m_plbBorderStyle->setColor(dlg.lineColor());
        setApplied(false);
    }
}

void GraphDlg::onGraphBorder(int state)
{
    bool bShow = (state==Qt::Checked);
    m_pGraph->setBorder(bShow);
    setApplied(false);
}


void GraphDlg::onGraphBackColor()
{
    QColor BkColor = m_pGraph->backgroundColor();
    BkColor = QColorDialog::getColor(BkColor);
    if(BkColor.isValid()) m_pGraph->setBkColor(BkColor);

    m_pcbGraphBack->setColor(m_pGraph->backgroundColor());
    setButtonColors();
    setApplied(false);
}


void GraphDlg::onLabelColor()
{
    QColor color = m_pGraph->labelColor();
    color = QColorDialog::getColor(color);

    m_pGraph->setLabelColor(color);
    m_ptcbLabelClr->setTextColor(color);

    setApplied(false);
    update();

}


void GraphDlg::onLabelFont()
{
    bool ok=false;
    QFont LabelFont("Courier");
    m_pGraph->getLabelFont(LabelFont);
    QFont font = QFontDialog::getFont(&ok, LabelFont, this);

    if (ok)
    {
        m_ppbLabelButton->setFont(font);
        m_ppbLabelButton->setText(font.family()+QString(" %1").arg(font.pointSize()));
        m_ptcbLabelClr->setFont(font);
        m_pGraph->setLabelFont(font);
        setApplied(false);
    }
}


void GraphDlg::onButton(QAbstractButton *pButton)
{
    if (     m_pButtonBox->button(QDialogButtonBox::Ok)      == pButton)  onOK();
    else if (m_pButtonBox->button(QDialogButtonBox::Discard) == pButton)  reject();
    else if (m_pButtonBox->button(QDialogButtonBox::Reset)   == pButton)  onRestoreParams();
}


void GraphDlg::onOK()
{
    applyChanges();

    m_XSel = m_plwXSel->currentRow();
    m_YSel = m_plwYSel->currentRow();
    m_pGraph->setVariables(m_plwXSel->currentRow(), m_plwYSel->currentRow());

    accept();
}


void GraphDlg::applyChanges()
{
    m_pGraph->setAutoX(m_pchXAuto->isChecked());
    m_pGraph->setXMin(m_pdeXMin->value());
    m_pGraph->setXMax(m_pdeXMax->value());
    m_pGraph->setX0(m_pdeXOrigin->value());
    m_pGraph->setXUnit(m_pdeXUnit->value());

    m_pGraph->setAutoY(m_pchYAuto->isChecked());
    m_pGraph->setYMin(m_pdeYMin->value());
    m_pGraph->setYMax(m_pdeYMax->value());
    m_pGraph->setY0(m_pdeYOrigin->value());
    m_pGraph->setYUnit(m_pdeYUnit->value());

    double MinUnit;
    if(!m_pchAutoXMinUnit->isChecked())
    {
        MinUnit = m_pdeXMinorUnit->value();
        m_pGraph->setXMinorUnit(MinUnit);
        m_pGraph->setAutoXMinUnit(false);
    }
    else
        m_pGraph->setAutoXMinUnit(true);

    if(!m_pchAutoYMinUnit->isChecked())
    {
        MinUnit = m_pdeYMinorUnit->value();
        m_pGraph->setYMinorUnit(MinUnit);
        m_pGraph->setAutoYMinUnit(false);
    }
    else
        m_pGraph->setAutoYMinUnit(true);

    m_pGraph->setMargin(m_pieMargin->value());

}


void GraphDlg::onRestoreParams()
{
    m_pGraph->copySettings(&m_SaveGraph);

    setControls();
    setApplied(true);

    if(m_pParent) m_pParent->update();
}


void GraphDlg::onTitleColor()
{
    QColor color = m_pGraph->titleColor();
    color = QColorDialog::getColor(color);

    m_pGraph->setTitleColor(color);
    m_ptcbTitleClr->setTextColor(color);

    setApplied(false);
    update();
}


void GraphDlg::onTitleFont()
{
    bool bOk=false;
    QFont TitleFont("Arial");
    m_pGraph->getTitleFont(TitleFont);

    QFont font = QFontDialog::getFont(&bOk, TitleFont, this);

    if (bOk)
    {
        m_ppbTitleButton->setFont(font);
        m_ppbTitleButton->setText(font.family()+QString(" %1").arg(font.pointSize()));
        m_ptcbTitleClr->setFont(font);
        m_pGraph->setTitleFont(font);
        setApplied(false);
    }
}


void GraphDlg::onVariableChanged()
{
    m_bVariableChanged = true;
}


void GraphDlg::onMargin()
{
    m_pGraph->setMargin(m_pieMargin->value());
}


void GraphDlg::onXMajGridStyle()
{
    LinePickerDlg dlg(this);
    int s,w;
    QColor color;
    bool bShow;
    m_pGraph->bXMajGrid(bShow,color,s,w);
    dlg.initDialog(0,s,w,color, false, false);

    if(QDialog::Accepted==dlg.exec())
    {
        m_pGraph->setXMajGrid(bShow, dlg.lineColor(), dlg.lineStipple(), dlg.lineWidth());
        m_plbXMajGridStyle->setStipple(dlg.lineStipple());
        m_plbXMajGridStyle->setWidth(dlg.lineWidth());
        m_plbXMajGridStyle->setColor(dlg.lineColor());
        setApplied(false);
    }
}

void GraphDlg::onXMinGridStyle()
{
    LinePickerDlg dlg(this);
    int s,w;
    QColor color;
    bool bShow, bAuto;
    double unit;
    m_pGraph->bXMinGrid(bShow, bAuto,color,s,w,unit);
    dlg.initDialog(0,s,w,color, false, false);

    if(QDialog::Accepted==dlg.exec())
    {
        m_pGraph->setXMinGrid(bShow, bAuto, dlg.lineColor(), dlg.lineStipple(), dlg.lineWidth(),unit);
        m_plbXMinGridStyle->setStipple(dlg.lineStipple());
        m_plbXMinGridStyle->setWidth(dlg.lineWidth());
        m_plbXMinGridStyle->setColor(dlg.lineColor());
        setApplied(false);
    }
}


void GraphDlg::onXMajGridShow(int state)
{
    bool bShow = (state==Qt::Checked);
    m_pGraph->setXMajGrid(bShow);
    m_plbXMajGridStyle->setEnabled(bShow);
    setApplied(false);
}


void GraphDlg::onXMinGridShow(int state)
{
    bool bShow = (state==Qt::Checked);
    m_pGraph->setXMinGrid(bShow);
    m_plbXMinGridStyle->setEnabled(bShow);
    m_pchAutoXMinUnit->setEnabled(bShow);
    m_pdeXMinorUnit->setEnabled(bShow && !m_pGraph->bAutoXMin());

    setApplied(false);
}


void GraphDlg::onYInverted()
{
    m_pGraph->setInverted(m_pchYInverted->checkState() == Qt::Checked);
    setApplied(false);
}


void GraphDlg::onYMajGridShow(int state)
{
    bool bShow = (state==Qt::Checked);
    m_pGraph->setYMajGrid(bShow);
    m_plbYMajGridStyle->setEnabled(bShow);
    setApplied(false);
}

void GraphDlg::onYMajGridStyle()
{
    LinePickerDlg dlg(this);
    int s,w;
    QColor color;
    bool bShow;
    m_pGraph->yMajGrid(bShow,color,s,w);
    dlg.initDialog(0,s,w,color, false, false);

    if(QDialog::Accepted==dlg.exec())
    {
        m_pGraph->setYMajGrid(bShow, dlg.lineColor(), dlg.lineStipple(), dlg.lineWidth());
        m_plbYMajGridStyle->setStipple(dlg.lineStipple());
        m_plbYMajGridStyle->setWidth(dlg.lineWidth());
        m_plbYMajGridStyle->setColor(dlg.lineColor());
    }
}


void GraphDlg::onYMinGridShow(int state)
{
    bool bShow = (state==Qt::Checked);
    m_pGraph->setYMinGrid(bShow);
    m_plbYMinGridStyle->setEnabled(bShow);
    m_pchAutoYMinUnit->setEnabled(bShow);
    m_pdeYMinorUnit->setEnabled(bShow && !m_pGraph->bAutoYMin());

    setApplied(false);
}


void GraphDlg::onYMinGridStyle()
{
    LinePickerDlg dlg(this);
    int s,w;
    QColor color;
    bool bShow, bAuto;
    double unit;
    m_pGraph->bYMinGrid(bShow, bAuto,color,s,w,unit);
    dlg.initDialog(0,s,w,color, false, false);

    if(QDialog::Accepted==dlg.exec())
    {
        m_pGraph->setYMinGrid(bShow, bAuto, dlg.lineColor(), dlg.lineStipple(), dlg.lineWidth(),unit);
        m_plbYMinGridStyle->setStipple(dlg.lineStipple());
        m_plbYMinGridStyle->setWidth(dlg.lineWidth());
        m_plbYMinGridStyle->setColor(dlg.lineColor());
        setApplied(false);
    }
}


void GraphDlg::setApplied(bool bApplied)
{
    m_bApplied = bApplied;
    //    ApplyButton->setEnabled(!bApplied);
}


void GraphDlg::setButtonColors()
{
    m_ptcbTitleClr->setTextColor(m_pGraph->titleColor());
    m_ptcbTitleClr->setBackgroundColor(m_pGraph->backgroundColor());

    m_ptcbLabelClr->setTextColor(m_pGraph->labelColor());
    m_ptcbLabelClr->setBackgroundColor(m_pGraph->backgroundColor());
}


void GraphDlg::setControls()
{
    m_pchXAuto->setChecked(m_pGraph->bAutoX());
    m_pchYAuto->setChecked(m_pGraph->bAutoY());

    m_pdeXMin->setValue(m_pGraph->xMin());
    m_pdeXMax->setValue(m_pGraph->xMax());
    m_pdeXOrigin->setValue(m_pGraph->xOrigin());
    m_pdeXUnit->setValue(m_pGraph->xUnit());
    m_pdeYMin->setValue(m_pGraph->yMin());
    m_pdeYMax->setValue(m_pGraph->yMax());
    m_pdeYOrigin->setValue(m_pGraph->yOrigin());
    m_pdeYUnit->setValue(m_pGraph->yUnit());

    onAutoX();
    onAutoY();

    setButtonColors();

    QFont font;
    m_pGraph->getLabelFont(font);
    m_ppbLabelButton->setText(font.family()+QString(" %1").arg(font.pointSize()));
    m_ppbLabelButton->setFont(font);

    m_pGraph->getTitleFont(font);
    m_ppbTitleButton->setText(font.family()+QString(" %1").arg(font.pointSize()));
    m_ppbTitleButton->setFont(font);

    bool bState, bAuto;
    QColor color;
    int style, width;
    double unit;

    m_pGraph->bXMajGrid(bState, color, style, width);
    m_pchXMajGridShow->setChecked(bState);
    m_plbXMajGridStyle->setColor(color);
    m_plbXMajGridStyle->setStipple(style);
    m_plbXMajGridStyle->setWidth(width);
    m_plbXMajGridStyle->setEnabled(bState);

    m_pGraph->bXMinGrid(bState, bAuto,color, style, width, unit);
    m_pchXMinGridShow->setChecked(bState);
    m_plbXMinGridStyle->setColor(color);
    m_plbXMinGridStyle->setStipple(style);
    m_plbXMinGridStyle->setWidth(width);
    m_plbXMinGridStyle->setEnabled(bState);
    m_pdeXMinorUnit->setValue(unit);
    m_pchAutoXMinUnit->setChecked(bAuto);
    m_pchAutoXMinUnit->setEnabled(bState);
    m_pdeXMinorUnit->setEnabled(!bAuto && bState);

    m_pGraph->yMajGrid(bState, color, style, width);
    m_pchYMajGridShow->setChecked(bState);
    m_plbYMajGridStyle->setColor(color);
    m_plbYMajGridStyle->setStipple(style);
    m_plbYMajGridStyle->setWidth(width);
    m_plbYMajGridStyle->setEnabled(bState);

    m_pGraph->bYMinGrid(bState, bAuto,color, style, width, unit);
    m_pchYMinGridShow->setChecked(bState);
    m_plbYMinGridStyle->setColor(color);
    m_plbYMinGridStyle->setStipple(style);
    m_plbYMinGridStyle->setWidth(width);
    m_plbYMinGridStyle->setEnabled(bState);
    m_pdeYMinorUnit->setValue(unit);
    m_pchAutoYMinUnit->setChecked(bAuto);
    m_pchAutoYMinUnit->setEnabled(bState);
    m_pdeYMinorUnit->setEnabled(!bAuto && bState);

    m_plbAxisStyle->setColor(m_pGraph->axisColor());
    m_plbAxisStyle->setStipple(m_pGraph->axisStyle());
    m_plbAxisStyle->setWidth(m_pGraph->axisWidth());

    m_pchGraphBorder->setChecked(m_pGraph->hasBorder());
    m_plbBorderStyle->setColor(m_pGraph->borderColor());
    m_plbBorderStyle->setStipple(m_pGraph->borderStyle());
    m_plbBorderStyle->setWidth(m_pGraph->borderWidth());

    m_pcbGraphBack->setColor(m_pGraph->backgroundColor());

    m_pieMargin->setValue(m_pGraph->margin());

    m_pchYInverted->setChecked(m_pGraph->bInverted());

    fillVariableList();

    m_plwXSel->setCurrentRow(m_pGraph->xVariable());
    m_plwYSel->setCurrentRow(m_pGraph->yVariable());
    m_bVariableChanged = false;

    setApplied(true);
}


void GraphDlg::setupLayout()
{
    QFontMetrics fm(font());

    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Discard | QDialogButtonBox::Reset);
    {
        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
    }

    m_pTabWidget = new QTabWidget(this);
    m_pScalePage    = new QWidget(this);
    m_pGridPage     = new QWidget(this);
    m_pFontPage     = new QWidget(this);
    m_pVariablePage = new QWidget(this);

    //________Variable Page______________________
    QVBoxLayout *pVariablePageLayout = new QVBoxLayout(this);
    {
        QHBoxLayout *pAxisNamesLayout = new QHBoxLayout;
        {
            QLabel *YAxis = new QLabel(tr("YAxis"));
            QLabel *vs = new QLabel(tr("vs."));
            QLabel *XAxis = new QLabel(tr("XAxis"));
            pAxisNamesLayout->addStretch(1);
            pAxisNamesLayout->addWidget(YAxis);
            pAxisNamesLayout->addStretch(1);
            pAxisNamesLayout->addWidget(vs);
            pAxisNamesLayout->addStretch(1);
            pAxisNamesLayout->addWidget(XAxis);
            pAxisNamesLayout->addStretch(1);
        }

        QHBoxLayout *pVariableBoxLayout = new QHBoxLayout;
        {
            m_plwXSel = new QListWidget;
            m_plwYSel = new QListWidget;
            pVariableBoxLayout->addWidget(m_plwYSel);
            pVariableBoxLayout->addWidget(m_plwXSel);
        }

        pVariablePageLayout->addLayout(pAxisNamesLayout);
        pVariablePageLayout->addLayout(pVariableBoxLayout);
    }
    m_pVariablePage->setLayout(pVariablePageLayout);

    //________Font Page______________________

    QVBoxLayout *pFontPageLayout = new QVBoxLayout;
    {
        QGroupBox *pFontBox = new QGroupBox(tr("Fonts"));
        {
            QGridLayout *pFontButtonsLayout = new QGridLayout;
            {
                QLabel *lab1  = new QLabel(tr("Title"));
                QLabel *lab2  = new QLabel(tr("Label"));
                QLabel *lab402  = new QLabel(tr("Font"));
                QLabel *lab403  = new QLabel(tr("Color"));
                lab1->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
                lab2->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
                lab402->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
                lab403->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
                pFontButtonsLayout->addWidget(lab402,1,2);
                pFontButtonsLayout->addWidget(lab403,1,3);
                pFontButtonsLayout->addWidget(lab1,2,1);
                pFontButtonsLayout->addWidget(lab2,3,1);

                m_ppbTitleButton  = new QPushButton();
                m_ppbLabelButton  = new QPushButton();

                pFontButtonsLayout->addWidget(m_ppbTitleButton,2,2);
                pFontButtonsLayout->addWidget(m_ppbLabelButton,3,2);

                m_ptcbTitleClr  = new TextClrBtn(this);
                m_ptcbTitleClr->setText(tr("Title Color"));
                m_ptcbLabelClr  = new TextClrBtn(this);
                m_ptcbLabelClr->setText(tr("Label Color"));

                pFontButtonsLayout->addWidget(m_ptcbTitleClr,2,3);
                pFontButtonsLayout->addWidget(m_ptcbLabelClr,3,3);
            }
            pFontBox->setLayout(pFontButtonsLayout);
        }

        QGroupBox *pBackBox = new QGroupBox(tr("BackGround"));
        {
            QGridLayout *pBackDataLayout = new QGridLayout;
            {
                QLabel *GraphBackLabel = new QLabel(tr("Graph Background"));
                GraphBackLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                m_pchGraphBorder = new QCheckBox(tr("Graph Border"));

                m_pcbGraphBack = new ColorButton;
                m_pcbGraphBack->setMinimumWidth(100);
                m_plbBorderStyle = new LineBtn(this);
                m_plbBorderStyle->setMinimumWidth(100);

                pBackDataLayout->addWidget(GraphBackLabel,1,1);
                pBackDataLayout->addWidget(m_pchGraphBorder,2,1,1,1,Qt::AlignRight | Qt::AlignVCenter);

                pBackDataLayout->addWidget(m_pcbGraphBack,1,2);
                pBackDataLayout->addWidget(m_plbBorderStyle,2,2);

                pBackDataLayout->setColumnStretch(0,1);
            }
            pBackBox->setLayout(pBackDataLayout);
        }
        QGroupBox *pPaddingBox = new QGroupBox(tr("Padding"));
        {
            QHBoxLayout *pPaddingLayout = new QHBoxLayout;
            {
                QLabel *pMarginLabel = new QLabel(tr("Margin"));
                QLabel *pMarginUnit = new QLabel(tr("pixels"));
                m_pieMargin = new IntEdit(31, this);
                pPaddingLayout->addStretch(1);
                pPaddingLayout->addWidget(pMarginLabel);
                pPaddingLayout->addWidget(m_pieMargin);
                pPaddingLayout->addWidget(pMarginUnit);
            }
            pPaddingBox->setLayout(pPaddingLayout);
        }

        pFontPageLayout->addWidget(pFontBox);
        pFontPageLayout->addStretch(1);
        pFontPageLayout->addWidget(pBackBox);
        pFontPageLayout->addStretch(1);
        pFontPageLayout->addWidget(pPaddingBox);
        pFontPageLayout->addStretch(1);
    }
    m_pFontPage->setLayout(pFontPageLayout);
    //________End Font Page______________________

    //________Scale Page______________________

    QGridLayout *pScalePageLayout = new QGridLayout;
    {
        QLabel *XAxis2 = new QLabel(tr("X Axis"));
        QLabel *YAxis2 = new QLabel(tr("Y Axis"));
        XAxis2->setAlignment(Qt::AlignCenter);
        YAxis2->setAlignment(Qt::AlignCenter);

        QLabel *MinLabel = new QLabel(tr("Min"));
        QLabel *MaxLabel = new QLabel(tr("Max"));
        QLabel *OriginLabel = new QLabel(tr("Origin"));
        QLabel *UnitLabel = new QLabel(tr("Unit"));
        MinLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        MaxLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        OriginLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        UnitLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        //    ScaleData->addStretch(1);
        pScalePageLayout->addWidget(MinLabel,5,1);
        pScalePageLayout->addWidget(MaxLabel,6,1);
        pScalePageLayout->addWidget(OriginLabel,7,1);
        pScalePageLayout->addWidget(UnitLabel,8,1);

        m_pchXAuto    = new QCheckBox(tr("Auto Scale"));
        m_pdeXMin     = new DoubleEdit;
        m_pdeXMax     = new DoubleEdit;
        m_pdeXOrigin  = new DoubleEdit;
        m_pdeXUnit    = new DoubleEdit;

        pScalePageLayout->addWidget(XAxis2,2,2);
        pScalePageLayout->addWidget(m_pchXAuto,4,2);
        pScalePageLayout->addWidget(m_pdeXMin,5,2);
        pScalePageLayout->addWidget(m_pdeXMax,6,2);
        pScalePageLayout->addWidget(m_pdeXOrigin,7,2);
        pScalePageLayout->addWidget(m_pdeXUnit,8,2);

        m_pchYInverted = new QCheckBox(tr("Inverted Axis"));
        m_pchYAuto     = new QCheckBox(tr("Auto Scale"));
        m_pdeYMin      = new DoubleEdit;
        m_pdeYMax      = new DoubleEdit;
        m_pdeYOrigin   = new DoubleEdit;
        m_pdeYUnit     = new DoubleEdit;

        pScalePageLayout->addWidget(YAxis2,2,3);
        pScalePageLayout->addWidget(m_pchYInverted,3,3);
        pScalePageLayout->addWidget(m_pchYAuto,4,3);
        pScalePageLayout->addWidget(m_pdeYMin,5,3);
        pScalePageLayout->addWidget(m_pdeYMax,6,3);
        pScalePageLayout->addWidget(m_pdeYOrigin,7,3);
        pScalePageLayout->addWidget(m_pdeYUnit,8,3);
        pScalePageLayout->setRowStretch(9,1);
    }
    m_pScalePage->setLayout(pScalePageLayout);
    //________End Scale Page______________________

    //________Axis Page______________________
    QGridLayout *pAxisDataLayout = new QGridLayout;
    {
        pAxisDataLayout->setRowStretch(0,1);
        QLabel *AxisStyleLabel = new QLabel(tr("Axis Style"));

        m_pchXMajGridShow = new QCheckBox(tr("X Major Grid"));
        m_pchYMajGridShow = new QCheckBox(tr("Y Major Grid"));
        m_pchXMinGridShow = new QCheckBox(tr("X Minor Grid"));
        m_pchYMinGridShow = new QCheckBox(tr("Y Minor Grid"));
        m_pchAutoXMinUnit = new QCheckBox(tr("Auto Unit"));
        m_pchAutoYMinUnit = new QCheckBox(tr("Auto Unit"));

        m_plbAxisStyle = new LineBtn(this);

        m_plbXMajGridStyle = new LineBtn(this);
        m_plbYMajGridStyle = new LineBtn(this);
        m_plbXMinGridStyle = new LineBtn(this);
        m_plbYMinGridStyle = new LineBtn(this);

        m_pdeXMinorUnit = new DoubleEdit;
        m_pdeYMinorUnit = new DoubleEdit;

        pAxisDataLayout->addWidget(AxisStyleLabel,1,1);
        pAxisDataLayout->addWidget(m_pchXMajGridShow,2,1);
        pAxisDataLayout->addWidget(m_pchYMajGridShow,3,1);
        pAxisDataLayout->addWidget(m_pchXMinGridShow,4,1);
        pAxisDataLayout->addWidget(m_pchYMinGridShow,5,1);

        pAxisDataLayout->addWidget(m_plbAxisStyle,1,2);
        pAxisDataLayout->addWidget(m_plbXMajGridStyle,2,2);
        pAxisDataLayout->addWidget(m_plbYMajGridStyle,3,2);
        pAxisDataLayout->addWidget(m_plbXMinGridStyle,4,2);
        pAxisDataLayout->addWidget(m_plbYMinGridStyle,5,2);

        pAxisDataLayout->addWidget(m_pchAutoXMinUnit,4,3);
        pAxisDataLayout->addWidget(m_pchAutoYMinUnit,5,3);
        pAxisDataLayout->addWidget(m_pdeXMinorUnit,4,4);
        pAxisDataLayout->addWidget(m_pdeYMinorUnit,5,4);
    }
    m_pGridPage->setLayout(pAxisDataLayout);
    //________End Axis Page______________________


    m_pTabWidget->addTab(m_pVariablePage, tr("Variables"));
    m_pTabWidget->addTab(m_pScalePage, tr("Scales"));
    m_pTabWidget->addTab(m_pGridPage, tr("Axis and Grids"));
    m_pTabWidget->addTab(m_pFontPage, tr("Fonts and BackGround"));

    m_pTabWidget->setCurrentIndex(s_iActivePage);
    connect(m_pTabWidget, SIGNAL(currentChanged (int)), this, SLOT(onActivePage(int)));

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(m_pTabWidget);
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);
}


void GraphDlg::setGraph(Graph *pGraph)
{
    m_pGraph = pGraph;
    m_SaveGraph.copySettings(m_pGraph);
    setControls();
}



void GraphDlg::setActivePage(int iPage)
{
    s_iActivePage =iPage;
}
