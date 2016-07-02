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
	m_Style  = 0;
	m_Width  = 1;
	m_Color  = QColor(0,255,0);
	setupLayout();

	m_pStyleDelegate = new LineDelegate(this);//will intercept painting operations
	m_pWidthDelegate = new LineDelegate(this);//will intercept painting operations

	m_pctrlStyle->setItemDelegate(m_pStyleDelegate);
	m_pctrlWidth->setItemDelegate(m_pWidthDelegate);

	connect(m_pctrlColor, SIGNAL(clickedLB()), this, SLOT(onColor()));
	connect(m_pctrlStyle, SIGNAL(activated(int)), this, SLOT(onStyle(int)));
	connect(m_pctrlWidth, SIGNAL(activated(int)), this, SLOT(onWidth(int)));

	connect(OKButton, SIGNAL(clicked()),this, SLOT(accept()));
	connect(CancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}


void LinePickerDlg::fillBoxes()
{
	m_pctrlStyle->setLine(m_Style, m_Width, m_Color, 0);
	m_pctrlWidth->setLine(m_Style, m_Width, m_Color, 0);

	m_pctrlColor->setStyle(m_Style);
	m_pctrlColor->setWidth(m_Width);

	m_pctrlColor->setColor(m_Color);
	m_pStyleDelegate->setLineColor(m_Color);
	m_pWidthDelegate->setLineColor(m_Color);

	m_pctrlStyle->setCurrentIndex(m_Style);
	m_pctrlWidth->setCurrentIndex(m_Width-1);

	int LineWidth[5];
	for (int i=0; i<5;i++) LineWidth[i] = m_Width;
	m_pStyleDelegate->setLineWidth(LineWidth); // the same selected width for all styles


	int LineStyle[5];
	for (int i=0; i<5;i++) LineStyle[i] = m_Style;
	m_pWidthDelegate->setLineStyle(LineStyle); //the same selected style for all widths
}


void LinePickerDlg::initDialog(int style, int width, QColor color)
{
	m_Color = color;
	m_Width = width;
	m_Style = style;
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



void LinePickerDlg::onWidth(int val)
{
	m_Width = val+1;
	fillBoxes();
	repaint();
	OKButton->setFocus();
}


void LinePickerDlg::onStyle(int val)
{
	m_Style = val;
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
    QColor Color = QColorDialog::getColor(m_Color,
                                   this, "Color Selection", dialogOptions);
	if(Color.isValid()) m_Color = Color;

	fillBoxes();
	repaint();
	OKButton->setFocus();
}



int & LinePickerDlg::width()
{
	return m_Width;
}


int & LinePickerDlg::setStyle()
{
	return m_Style;
}


QColor & LinePickerDlg::setColor()
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
	m_Style = style;
	fillBoxes();
	repaint();
}


void LinePickerDlg::setupLayout()
{
	QGridLayout *pStyleLayout = new QGridLayout;
	{
		QLabel *lab1 = new QLabel(tr("Style"));
		QLabel *lab2 = new QLabel(tr("Width"));
		QLabel *lab3 = new QLabel(tr("Color"));
		lab1->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		lab2->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		lab3->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		lab1->setMinimumWidth(60);
		lab2->setMinimumWidth(60);
		lab3->setMinimumWidth(60);
		m_pctrlColor = new LineBtn;
		m_pctrlStyle = new LineCbBox;
		m_pctrlWidth = new LineCbBox;

		QFontMetrics fm(Settings::s_TextFont);

		m_pctrlStyle->setMinimumWidth(17*fm.averageCharWidth());
		m_pctrlWidth->setMinimumWidth(17*fm.averageCharWidth());
		m_pctrlColor->setMinimumWidth(17*fm.averageCharWidth());
		m_pctrlColor->setMinimumHeight(m_pctrlStyle->minimumSizeHint().height());


		pStyleLayout->addWidget(lab1,1,1);
		pStyleLayout->addWidget(lab2,2,1);
		pStyleLayout->addWidget(lab3,3,1);
		pStyleLayout->addWidget(m_pctrlStyle,1,2);
		pStyleLayout->addWidget(m_pctrlWidth,2,2);
		pStyleLayout->addWidget(m_pctrlColor,3,2);
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













