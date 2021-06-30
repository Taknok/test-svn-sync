/****************************************************************************

    xflwidgets Library
    Copyright (C) Andre Deperrois 
    GNU General Public License v3

*****************************************************************************/

#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>

#include "gridcontrol.h"
#include <xflwidgets/view/grid.h>
#include <xflwidgets/line/linemenu.h>


GridControl::GridControl(QWidget *pParent) : QWidget(pParent)
{
    m_pGrid = nullptr;
    setupLayout();
    connectSignals();
}


void GridControl::connectSignals()
{
    connect(m_pchYAxisShow,   SIGNAL(clicked(bool)), SLOT(onYAxisShow(bool)));
    connect(m_pchXAxisShow,   SIGNAL(clicked(bool)), SLOT(onXAxisShow(bool)));
    connect(m_pchXMajShow,    SIGNAL(clicked(bool)), SLOT(onXMajShow(bool)));
    connect(m_pchYMajShow,    SIGNAL(clicked(bool)), SLOT(onYMajShow(bool)));
    connect(m_pchXMinShow,    SIGNAL(clicked(bool)), SLOT(onXMinShow(bool)));
    connect(m_pchYMinShow,    SIGNAL(clicked(bool)), SLOT(onYMinShow(bool)));

    connect(m_plbYAxisStyle,   SIGNAL(clickedLB(LineStyle)), SLOT(onYAxisStyle()));
    connect(m_plbXAxisStyle,   SIGNAL(clickedLB(LineStyle)), SLOT(onXAxisStyle()));
    connect(m_plbXMajStyle,    SIGNAL(clickedLB(LineStyle)), SLOT(onXMajStyle()));
    connect(m_pclbYMajStyle,   SIGNAL(clickedLB(LineStyle)), SLOT(onYMajStyle()));
    connect(m_pctrlXMinStyle,  SIGNAL(clickedLB(LineStyle)), SLOT(onXMinStyle()));
    connect(m_plbYMinStyle,    SIGNAL(clickedLB(LineStyle)), SLOT(onYMinStyle()));
}


void GridControl::initControls(Grid *pGrid)
{
    m_pGrid = pGrid;

    m_plbXAxisStyle->setTheStyle(m_pGrid->xAxisStyle());
    m_plbYAxisStyle->setTheStyle(m_pGrid->yAxisStyle());

    m_plbXMajStyle->setTheStyle(m_pGrid->xMajStyle());
    m_pctrlXMinStyle->setTheStyle(m_pGrid->xMinStyle());

    m_pclbYMajStyle->setTheStyle(m_pGrid->yMajStyle(0));
    m_plbYMinStyle->setTheStyle(m_pGrid->yMinStyle(0));

    m_plbXAxisStyle->setEnabled(m_pGrid->bXAxis());
    m_plbYAxisStyle->setEnabled(m_pGrid->bYAxis());
    m_plbXMajStyle->setEnabled(m_pGrid->bXMajGrid());
    m_pclbYMajStyle->setEnabled(m_pGrid->bYMajGrid(0));
    m_pctrlXMinStyle->setEnabled(m_pGrid->bXMinGrid());
    m_plbYMinStyle->setEnabled(m_pGrid->bYMinGrid(0));

    m_pchXAxisShow->setChecked(m_pGrid->bXAxis());
    m_pchYAxisShow->setChecked(m_pGrid->bYAxis());
    m_pchXMajShow->setChecked(m_pGrid->bXMajGrid());
    m_pchYMajShow->setChecked(m_pGrid->bYMajGrid(0));
    m_pchXMinShow->setChecked(m_pGrid->bXMinGrid());
    m_pchYMinShow->setChecked(m_pGrid->bYMinGrid(0));
}


void GridControl::setupLayout()
{
    QGridLayout *pMainLayout = new QGridLayout;
    {
        QLabel *pLabX = new QLabel("X");
        QLabel *pLabY = new QLabel("Y");

        QLabel *pLabAxes = new QLabel(tr("Axes"));
        QLabel *pLabMajGrid = new QLabel(tr("Maj. grid"));
        QLabel *pLabMinGrid = new QLabel(tr("Min. grid"));

        m_pchXAxisShow = new QCheckBox(QString());
        m_pchYAxisShow = new QCheckBox(QString());
        m_pchXMajShow  = new QCheckBox(QString());
        m_pchYMajShow  = new QCheckBox(QString());
        m_pchXMinShow  = new QCheckBox(QString());
        m_pchYMinShow  = new QCheckBox(QString());

        m_plbYAxisStyle  = new LineBtn(this);
        m_plbXAxisStyle  = new LineBtn(this);
        m_plbXMajStyle   = new LineBtn(this);
        m_pclbYMajStyle  = new LineBtn(this);
        m_pctrlXMinStyle = new LineBtn(this);
        m_plbYMinStyle   = new LineBtn(this);


        pMainLayout->addWidget(pLabX, 1, 2, 1, 2, Qt::AlignCenter);
        pMainLayout->addWidget(pLabY, 1, 4, 1, 2, Qt::AlignCenter);

        pMainLayout->addWidget(pLabAxes,         2,1);
        pMainLayout->addWidget(m_pchXAxisShow,   2,2);
        pMainLayout->addWidget(m_plbXAxisStyle,  2,3);
        pMainLayout->addWidget(m_pchYAxisShow,   2,4);
        pMainLayout->addWidget(m_plbYAxisStyle,  2,5);
        pMainLayout->addWidget(pLabMajGrid,      3,1);
        pMainLayout->addWidget(m_pchXMajShow,    3,2);
        pMainLayout->addWidget(m_plbXMajStyle,   3,3);
        pMainLayout->addWidget(m_pchYMajShow,    3,4);
        pMainLayout->addWidget(m_pclbYMajStyle,  3,5);
        pMainLayout->addWidget(pLabMinGrid,      4,1);
        pMainLayout->addWidget(m_pchXMinShow,    4,2);
        pMainLayout->addWidget(m_pctrlXMinStyle, 4,3);
        pMainLayout->addWidget(m_pchYMinShow,    4,4);
        pMainLayout->addWidget(m_plbYMinStyle,   4,5);
    }

    setLayout(pMainLayout);
}


void GridControl::onXAxisStyle()
{
    LineMenu lineMenu;
    lineMenu.initMenu(m_pGrid->xAxisStyle());
    lineMenu.exec(QCursor::pos());
    m_pGrid->setXAxisStyle(lineMenu.theStyle());
    m_plbXAxisStyle->setTheStyle(m_pGrid->xAxisStyle());
    emit gridModified(true);
}


void GridControl::onYAxisStyle()
{
    LineMenu lineMenu;
    lineMenu.initMenu(m_pGrid->yAxisStyle());
    lineMenu.exec(QCursor::pos());
    m_pGrid->setYAxisStyle(lineMenu.theStyle());
    m_plbYAxisStyle->setTheStyle(m_pGrid->yAxisStyle());
    emit gridModified(true);
}


void GridControl::onXMajStyle()
{
    LineMenu lineMenu;
    lineMenu.initMenu(m_pGrid->xMajStyle());
    lineMenu.exec(QCursor::pos());
    m_pGrid->setXMajStyle(lineMenu.theStyle());
    m_plbXMajStyle->setTheStyle(m_pGrid->xMajStyle());
    emit gridModified(true);
}


void GridControl::onXMinStyle()
{
    LineMenu lineMenu;
    lineMenu.initMenu(m_pGrid->xMinStyle());
    lineMenu.exec(QCursor::pos());
    m_pGrid->setXMinStyle(lineMenu.theStyle());
    m_pctrlXMinStyle->setTheStyle(m_pGrid->xMinStyle());
    emit gridModified(true);
}


void GridControl::onYMajStyle()
{
    LineMenu lineMenu;
    lineMenu.initMenu(m_pGrid->yMajStyle(0));
    lineMenu.exec(QCursor::pos());
    m_pGrid->setYMajStyle(0, lineMenu.theStyle());
    m_pclbYMajStyle->setTheStyle(m_pGrid->yMajStyle(0));
    emit gridModified(true);
}


void GridControl::onYMinStyle()
{
    LineMenu lineMenu;
    lineMenu.initMenu(m_pGrid->yMinStyle(0));
    lineMenu.exec(QCursor::pos());
    m_pGrid->setYMinStyle(0, lineMenu.theStyle());
    m_plbYMinStyle->setTheStyle(m_pGrid->yMinStyle(0));
    emit gridModified(true);
}


void GridControl::onXAxisShow(bool bShow)
{
    m_pGrid->showXAxis(bShow);
    m_plbXAxisStyle->setEnabled(m_pGrid->bXAxis());
    emit gridModified(true);
}


void GridControl::onYAxisShow(bool bShow)
{
    m_pGrid->showYAxis(bShow);
    m_plbYAxisStyle->setEnabled(m_pGrid->bYAxis());
    emit gridModified(true);
}


void GridControl::onXMajShow(bool bShow)
{
    m_pGrid->showXMajGrid(bShow);
    m_plbXMajStyle->setEnabled(m_pGrid->bXMajGrid());
    emit gridModified(true);
}


void GridControl::onYMajShow(bool bShow)
{
    m_pGrid->showYMajGrid(0, bShow);
    m_pclbYMajStyle->setEnabled(m_pGrid->bYMajGrid(0));
    emit gridModified(true);
}


void GridControl::onXMinShow(bool bShow)
{
    m_pGrid->showXMinGrid(bShow);
    m_pctrlXMinStyle->setEnabled(m_pGrid->bXMinGrid());
    emit gridModified(true);
}


void GridControl::onYMinShow(bool bShow)
{
    m_pGrid->showYMinGrid(0, bShow);
    m_plbYMinStyle->setEnabled(m_pGrid->bYMinGrid(0));
    emit gridModified(true);
}


