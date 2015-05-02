/****************************************************************************

	ViewObjectDelegate Class
	Copyright (C) 2015 Andre Deperrois adeperrois@xflr5.com

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
#include <QPainter>
#include <QComboBox>
#include <QtDebug>
#include "ViewObjectDelegate.h"
#include "../../objects/Foil.h"

#include "../../misc/IntEdit.h"
#include "../../misc/DoubleEdit.h"
#include "../../xflr5.h"


QList <void*> *ViewObjectDelegate::s_poaFoil;


ViewObjectDelegate::ViewObjectDelegate(QWidget *pParent) : QStyledItemDelegate(pParent)
{
}


QWidget *ViewObjectDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex & index ) const
{
	int dataType = index.model()->data(index, Qt::UserRole).toInt();

	if(dataType==XFLR5::BOOL)
	{
			QComboBox *editor = new QComboBox(parent);
			editor->setSizeAdjustPolicy(QComboBox::AdjustToContents);
			//fill comboboxes here
			editor->addItem("true");
			editor->addItem("false");
			return editor;
	}
	else if(dataType==XFLR5::POLARTYPE)
	{
			QComboBox *editor = new QComboBox(parent);
			editor->setSizeAdjustPolicy(QComboBox::AdjustToContents);
			//fill comboboxes here
			editor->addItem("FIXEDSPEEDPOLAR");
			editor->addItem("FIXEDLIFTPOLAR");
			editor->addItem("FIXEDAOAPOLAR");
			editor->addItem("STABILITYPOLAR");
			editor->addItem("BETAPOLAR");
			return editor;
	}
	else if(dataType==XFLR5::ANALYSISMETHOD)
	{
			QComboBox *editor = new QComboBox(parent);
			editor->setSizeAdjustPolicy(QComboBox::AdjustToContents);
			//fill comboboxes here
			editor->addItem("LLTMETHOD");
			editor->addItem("VLMMETHOD");
			editor->addItem("PANELMETHOD");
			return editor;
	}
	else if(dataType==XFLR5::PANELDISTRIBUTION)
	{
			QComboBox *editor = new QComboBox(parent);
			editor->setSizeAdjustPolicy(QComboBox::AdjustToContents);
			//fill comboboxes here
			editor->addItem("INVERSESINE");
			editor->addItem("COSINE");
			editor->addItem("SINE");
			editor->addItem("UNIFORM");
			return editor;
	}
	else if(dataType==XFLR5::REFDIMENSIONS)
	{
			QComboBox *editor = new QComboBox(parent);
			editor->setSizeAdjustPolicy(QComboBox::AdjustToContents);
			//fill comboboxes here
			editor->addItem("PLANFORMREFDIM");
			editor->addItem("PROJECTEDREFDIM");
			editor->addItem("MANUALREFDIM");

			return editor;
	}
	else if(dataType==XFLR5::BODYTYPE)
	{
			QComboBox *editor = new QComboBox(parent);
			editor->setSizeAdjustPolicy(QComboBox::AdjustToContents);
			//fill comboboxes here
			editor->addItem("FLATPANELS");
			editor->addItem("NURBS");
			return editor;
	}
	else if(dataType==XFLR5::FOILNAME)
	{
		QComboBox *editor = new QComboBox(parent);
		editor->setSizeAdjustPolicy(QComboBox::AdjustToContents);
		//fill comboboxes here
		for(int i=0; i<s_poaFoil->size(); i++)
		{
			Foil *pFoil = (Foil*)s_poaFoil->at(i);
			editor->addItem(pFoil->foilName());
		}
		return editor;
	}
	else if (dataType==XFLR5::DOUBLE)
	{
		DoubleEdit *editor = new DoubleEdit(parent);
		editor->SetPrecision(3);
		return editor;
	}
	else if (dataType==XFLR5::INTEGER)
	{
		IntEdit *editor = new IntEdit(parent);
		return editor;
	}
	else
	{
		//String case, only edit the first line with the polar name
		if(index.row()==0 && index.column()==2)  return new QLineEdit(parent);
		else                                     return NULL;
	}
	return NULL;
}


void ViewObjectDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
	int dataType = index.model()->data(index, Qt::UserRole).toInt();
	if(dataType==XFLR5::INTEGER)
	{
		int value = index.model()->data(index, Qt::EditRole).toInt();
		IntEdit *pIE = static_cast<IntEdit*>(editor);
		pIE->SetValue(value);
	}
	else if(dataType==XFLR5::DOUBLE)
	{
		double value = index.model()->data(index, Qt::EditRole).toDouble();
		DoubleEdit *pDE = static_cast<DoubleEdit*>(editor);
		pDE->SetValue(value);
	}
	else if(dataType==XFLR5::STRING)
	{
		QString strong = index.model()->data(index, Qt::EditRole).toString();
		QLineEdit *pLineEdit = (QLineEdit*)editor;
		pLineEdit->setText(strong);
	}
//	else if(dataType==XFLR5::BOOL || dataType==XFLR5::PANELDISTRIBUTION || dataType==XFLR5::FOILNAME || dataType==XFLR5::BODYTYPE)
	else
	{
		QString strong = index.model()->data(index, Qt::EditRole).toString();
		QComboBox *pCbBox = static_cast<QComboBox*>(editor);
		int pos = pCbBox->findText(strong);
		if (pos>=0) pCbBox->setCurrentIndex(pos);
		else        pCbBox->setCurrentIndex(0);
	}
}


void ViewObjectDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
	int dataType = index.model()->data(index, Qt::UserRole).toInt();
	if(dataType==XFLR5::INTEGER)
	{
		IntEdit *pIE = static_cast<IntEdit*>(editor);
		int value = pIE->Value();
		model->setData(index, value, Qt::EditRole);
	}
	else if(dataType==XFLR5::DOUBLE)
	{
		DoubleEdit *pDE = static_cast<DoubleEdit*>(editor);
		double value = pDE->Value();
		model->setData(index, value, Qt::EditRole);
	}
	else if(dataType==XFLR5::STRING)
	{
		QLineEdit *pLineEdit = (QLineEdit*)editor;
		model->setData(index, pLineEdit->text(), Qt::EditRole);
	}
//	else if(dataType==XFLR5::BOOL || dataType==XFLR5::PANELDISTRIBUTION || dataType==XFLR5::FOILNAME ||
//			dataType==XFLR5::BODYTYPE || dataType==XFLR5::POLARTYPE || dataType==XFLR5::ANALYSISMETHOD || dataType==XFLR5::REFDIMENSIONS)
	else
	{
		QString strong;
		QComboBox *pCbBox = static_cast<QComboBox*>(editor);
		int sel = pCbBox->currentIndex();
		if (sel >=0) strong = pCbBox->itemText(sel);
		model->setData(index, strong, Qt::EditRole);
	}
}


void ViewObjectDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	int dataType = index.model()->data(index, Qt::UserRole).toInt();
	QString strong;
	QStyleOptionViewItem myOption = option;
	if(dataType==XFLR5::INTEGER)
	{
		myOption.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
		strong = QString("%L1").arg(index.model()->data(index, Qt::DisplayRole).toInt());
	}
	else if(dataType==XFLR5::DOUBLE)
	{
		myOption.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
		strong = QString("%L1").arg(index.model()->data(index, Qt::DisplayRole).toDouble(), 0,'g', 3);
	}
	else if(dataType==XFLR5::STRING)
	{
		myOption.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
		strong = index.model()->data(index, Qt::DisplayRole).toString();
	}
	else if( dataType==XFLR5::REFDIMENSIONS)
	{
		myOption.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
		strong = index.model()->data(index, Qt::DisplayRole).toString();
	}
//	else if(dataType==XFLR5::BOOL || dataType==XFLR5::PANELDISTRIBUTION || dataType==XFLR5::FOILNAME ||
//			dataType==XFLR5::BODYTYPE || dataType==XFLR5::POLARTYPE || dataType==XFLR5::ANALYSISMETHOD || dataType==XFLR5::REFDIMENSIONS)
	else
	{
		myOption.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
		strong = index.model()->data(index, Qt::DisplayRole).toString();
	}

//	QBrush backBrush(Qt::transparent);
//	painter->setBrush(backBrush);
//	painter->setBackground(backBrush);
//	painter->setBackgroundMode(Qt::OpaqueMode);

	QPen textPen(Qt::darkBlue);
	painter->setPen(textPen);

	QFont fnt;
	QFontMetrics fm(fnt);
	int w = (int)((double)fm.height()/2);//pixels

	QRect sR3 = myOption.rect;
	sR3.setLeft(sR3.left()+w);
	sR3.setRight(sR3.right()-w);
	painter->drawText(sR3, myOption.displayAlignment , strong);
}


void ViewObjectDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
	editor->setGeometry(option.rect);
}












