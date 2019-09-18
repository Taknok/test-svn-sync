/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois techwinder@flow5.tech
    All rights reserved.

*****************************************************************************/

#include <QVBoxLayout>
#include <QTimer>

#include "popup.h"


PopUp::PopUp(QWidget *pParent) : QWidget(pParent)
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_DeleteOnClose);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setupLayout();
}


PopUp::PopUp(QString const &message, QWidget *pParent) : QWidget(pParent)
{
    setupLayout();
    m_pMessage->setText(message);
}


void PopUp::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        m_pMessage = new QLabel;
        QPalette palette;
        m_pMessage->setAutoFillBackground(true);
        m_pMessage->setContentsMargins(5,5,5,5);
        m_pMessage->setText("A toast popup\n with useful information");
    }
    pMainLayout->addWidget(m_pMessage);
    setLayout(pMainLayout);
}


void PopUp::setFont(QFont const &fnt)
{
    QFont txtfnt(fnt);
    m_pMessage->setFont(txtfnt);
}


void PopUp::setRed()
{
//    m_pMessage->setStyleSheet("QLabel { color : red; background-color: black; padding: 5px}");
    QPalette palette;
    palette.setColor(QPalette::WindowText, Qt::darkRed);
    palette.setColor(QPalette::Window, QColor(125,125,225,25));
    m_pMessage->setPalette(palette);
    m_pMessage->setAutoFillBackground(true);
}


void PopUp::setGreen()
{
//    m_pMessage->setStyleSheet("QLabel { color : green; background-color: black; padding: 5px}");
    QPalette palette;
    palette.setColor(QPalette::WindowText, Qt::darkGreen);
    palette.setColor(QPalette::Window, QColor(125,125,225,25));
    m_pMessage->setPalette(palette);
    m_pMessage->setAutoFillBackground(true);
}


void PopUp::showEvent(QShowEvent *)
{
    QTimer::singleShot(11000, this, SLOT(close()));
}


void PopUp::mousePressEvent(QMouseEvent *)
{
    close();
}


void PopUp::setTextMessage(const QString &text)
{
    m_pMessage->setText(text);
}


void PopUp::appendTextMessage(const QString &text)
{
    QString strange = m_pMessage->text();
    m_pMessage->setText(strange + "\n"+ text);
}


