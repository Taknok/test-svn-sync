/****************************************************************************

	LinePicker Class
	Copyright (C) 2009 Andre Deperrois adeperrois@xflr5.com

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

#include <QPushButton>
#include <QKeyEvent>
#include <QColorDialog>
#include <QLabel>
#include <QGridLayout>

#include "LinePickerDlg.h"
#include <globals.h>
#include "./Settings.h"


LinePickerDlg::LinePickerDlg(QWidget *pParent): QDialog(pParent)
{
	setWindowTitle(tr("Line Picker"));
	m_pParent = pParent;
	m_PointStyle = 0;
	m_LineStyle  = 0;
	m_Width  = 1;
	m_Color  = QColor(0,255,0);
	setupLayout();

	m_pPointStyleDelegate = new LineDelegate(this);//will intercept painting operations
	m_pLineStyleDelegate = new LineDelegate(this);//will intercept painting operations
	m_pWidthDelegate = new LineDelegate(this);//will intercept painting operations

	m_pctrlPoints->setItemDelegate(m_pPointStyleDelegate);
	m_pctrlStyle->setItemDelegate(m_pLineStyleDelegate);
	m_pctrlWidth->setItemDelegate(m_pWidthDelegate);

	connect(m_pctrlPoints, SIGNAL(activated(int)), this, SLOT(onPoints(int)));
	connect(m_pctrlStyle, SIGNAL(activated(int)), this, SLOT(onStyle(int)));
	connect(m_pctrlWidth, SIGNAL(activated(int)), this, SLOT(onWidth(int)));
	connect(m_pctrlColor, SIGNAL(clickedLB()), this, SLOT(onColor()));

	connect(OKButton, SIGNAL(clicked()),this, SLOT(accept()));
	connect(CancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}


void LinePickerDlg::fillBoxes()
{
	m_pctrlColor->setStyle(m_LineStyle);
	m_pctrlColor->setWidth(m_Width);
	m_pctrlColor->setColor(m_Color);


	int LineStyle[5];
	int LineWidth[5];
	int LinePoints[5];
	for (int i=0; i<5;i++)
	{
		LineWidth[i] = m_Width;
		LineStyle[i] = m_LineStyle;
		LinePoints[i] = 0;
	}
	m_pLineStyleDelegate->setPointStyle(LinePoints);
	m_pLineStyleDelegate->setLineWidth(LineWidth); // the same selected width for all styles
	m_pLineStyleDelegate->setLineColor(m_Color);

	m_pWidthDelegate->setPointStyle(LinePoints);
	m_pWidthDelegate->setLineStyle(LineStyle); //the same selected style for all widths
	m_pWidthDelegate->setLineColor(m_Color);

	for (int i=0; i<5;i++) LinePoints[i]=i;
	m_pPointStyleDelegate->setPointStyle(LinePoints);
	m_pPointStyleDelegate->setLineStyle(LineStyle);
	m_pPointStyleDelegate->setLineWidth(LineWidth);
	m_pPointStyleDelegate->setLineColor(m_Color);

	m_pctrlPoints->setLine(m_LineStyle, m_Width, m_Color, m_PointStyle);
	m_pctrlStyle->setLine(m_LineStyle, m_Width, m_Color, 0);
	m_pctrlWidth->setLine(m_LineStyle, m_Width, m_Color, 0);

	m_pctrlPoints->setCurrentIndex(m_PointStyle);
	m_pctrlStyle->setCurrentIndex(m_LineStyle);
	m_pctrlWidth->setCurrentIndex(m_Width-1);
}


void LinePickerDlg::initDialog(int pointStyle, int lineStyle, int width, QColor color, bool bAcceptPointStyle)
{
	m_PointStyle = pointStyle;
	m_LineStyle = lineStyle;
	m_Width = width;
	m_Color = color;

	QString str;
	for (int i=0; i<5; i++)
	{
		str = QString("%1").arg(i);
		m_pctrlWidth->addItem(str);
		m_pctrlPoints->addItem(str);
	}

	m_pctrlStyle->addItem("solid");//string doesn't matter
	m_pctrlStyle->addItem("dash");
	m_pctrlStyle->addItem("dot");
	m_pctrlStyle->addItem("dashdot");
	m_pctrlStyle->addItem("dashdotdot");

	fillBoxes();
}



void LinePickerDlg::initDialog()
{
	QString str;
	for (int i=0; i<5; i++)
	{
		str = QString("%1").arg(i);
		m_pctrlWidth->addItem(str);
	}
	m_pctrlStyle->addItem("solid");//string doesn't matter
	m_pctrlStyle->addItem("dash");
	m_pctrlStyle->addItem("dot");
	m_pctrlStyle->addItem("dashdot");
	m_pctrlStyle->addItem("dashdotdot");

	fillBoxes();
}



void LinePickerDlg::keyPressEvent(QKeyEvent *event)
{
	// Prevent Return Key from closing App
	switch (event->key())
	{
		case Qt::Key_Return:
		case Qt::Key_Enter:
		{
			if(!OKButton->hasFocus())
			{
				OKButton->setFocus();
				return;
			}
			else
			{
				accept();
				return;
			}
			break;
		}
		case Qt::Key_Escape:
		{
			reject();
			return;
		}
	}
}


void LinePickerDlg::onPoints(int val)
{
	m_PointStyle = val;
	fillBoxes();
	repaint();
	OKButton->setFocus();
}


void LinePickerDlg::onStyle(int val)
{
	m_LineStyle = val;
	fillBoxes();
	repaint();
	OKButton->setFocus();
}


void LinePickerDlg::onWidth(int val)
{
	m_Width = val+1;
	fillBoxes();
	repaint();
	OKButton->setFocus();
}


void LinePickerDlg::onColor()
{
    QColorDialog::ColorDialogOptions dialogOptions = QColorDialog::ShowAlphaChannel;
#ifdef Q_OS_MAC
#if QT_VERSION >= 0x040700
    dialogOptions |= QColorDialog::DontUseNativeDialog;
#endif
#endif
	QColor Color = QColorDialog::getColor(m_Color, this, "Color Selection", dialogOptions);
	if(Color.isValid()) m_Color = Color;

	fillBoxes();
	repaint();
	OKButton->setFocus();
}


int & LinePickerDlg::pointStyle()
{
	return m_PointStyle;
}


int & LinePickerDlg::lineStyle()
{
	return m_LineStyle;
}


int & LinePickerDlg::lineWidth()
{
	return m_Width;
}


QColor & LinePickerDlg::lineColor()
{
	return m_Color;
}

void LinePickerDlg::setColor(QColor color)
{
	m_Color = color;
	fillBoxes();
	repaint();
}


void LinePickerDlg::setStyle(int style)
{
	m_LineStyle = style;
	fillBoxes();
	repaint();
}


void LinePickerDlg::setupLayout()
{
	QGridLayout *pStyleLayout = new QGridLayout;
	{
		QLabel *lab0 = new QLabel(tr("Points"));
		QLabel *lab1 = new QLabel(tr("Style"));
		QLabel *lab2 = new QLabel(tr("Width"));
		QLabel *lab3 = new QLabel(tr("Color"));
		lab0->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		lab1->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		lab2->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		lab3->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		lab0->setMinimumWidth(60);
		lab1->setMinimumWidth(60);
		lab2->setMinimumWidth(60);
		lab3->setMinimumWidth(60);
		m_pctrlPoints = new LineCbBox(this);
		m_pctrlPoints->showPoints(true);
		m_pctrlStyle = new LineCbBox;
		m_pctrlWidth = new LineCbBox;
		m_pctrlColor = new LineBtn;


		QFontMetrics fm(Settings::s_TextFont);

		m_pctrlPoints->setMinimumWidth(17*fm.averageCharWidth());
		m_pctrlStyle->setMinimumWidth(17*fm.averageCharWidth());
		m_pctrlWidth->setMinimumWidth(17*fm.averageCharWidth());
		m_pctrlColor->setMinimumWidth(17*fm.averageCharWidth());
		m_pctrlColor->setMinimumHeight(m_pctrlStyle->minimumSizeHint().height());

		pStyleLayout->addWidget(lab0,1,1);
		pStyleLayout->addWidget(lab1,2,1);
		pStyleLayout->addWidget(lab2,3,1);
		pStyleLayout->addWidget(lab3,4,1);
		pStyleLayout->addWidget(m_pctrlPoints,1,2);
		pStyleLayout->addWidget(m_pctrlStyle, 2,2);
		pStyleLayout->addWidget(m_pctrlWidth, 3,2);
		pStyleLayout->addWidget(m_pctrlColor, 4,2);
	}

	QHBoxLayout *pCommandButtons = new QHBoxLayout;
	{
		OKButton = new QPushButton(tr("OK"));
		CancelButton = new QPushButton(tr("Cancel"));
		pCommandButtons->addStretch(1);
		pCommandButtons->addWidget(OKButton);
		pCommandButtons->addStretch(1);
		pCommandButtons->addWidget(CancelButton);
		pCommandButtons->addStretch(1);
	}

	QVBoxLayout *pMainLayout = new QVBoxLayout;
	{
		pMainLayout->addStretch(1);
		pMainLayout->addLayout(pStyleLayout);
		pMainLayout->addStretch(1);
		pMainLayout->addLayout(pCommandButtons);
		pMainLayout->addStretch(1);
	}

	setMinimumHeight(170);
	setLayout(pMainLayout);
}



void LinePickerDlg::setWidth(int width)
{
	m_Width = width;
	fillBoxes();
	repaint();
}













