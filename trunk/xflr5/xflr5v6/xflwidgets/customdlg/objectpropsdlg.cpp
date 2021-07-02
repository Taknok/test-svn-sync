/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>

#include <QShowEvent>
#include <QHideEvent>

#include "objectpropsdlg.h"


#include <xflwidgets/customwts/plaintextoutput.h>

QByteArray ObjectPropsDlg::s_Geometry;


ObjectPropsDlg::ObjectPropsDlg(QWidget *pParent) : QDialog(pParent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setupLayout();
}


void ObjectPropsDlg::setupLayout()
{
    QDialogButtonBox *pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    {
        connect(pButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    }
    QVBoxLayout * pMainLayout = new QVBoxLayout(this);
    {
        m_pctrlOutput = new PlainTextOutput;
        pMainLayout->addWidget(m_pctrlOutput);
        pMainLayout->addSpacing(20);
        pMainLayout->addWidget(pButtonBox);
    }

    setLayout(pMainLayout);
}


void ObjectPropsDlg::initDialog(QString title, QString props)
{
    setWindowTitle(title);
    m_pctrlOutput->insertPlainText(props);
    m_pctrlOutput->moveCursor(QTextCursor::Start);
}


void ObjectPropsDlg::showEvent(QShowEvent *)
{
    restoreGeometry(s_Geometry);
}


void ObjectPropsDlg::hideEvent(QHideEvent*)
{
    s_Geometry = saveGeometry();
}







