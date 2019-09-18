/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois techwinder@flow5.tech
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QWidget>
#include <QLabel>



class PopUp : public QWidget
{
    Q_OBJECT

    public:
        PopUp(QWidget *pParent=nullptr);
        PopUp(QString const &message, QWidget *pParent);
        void appendTextMessage(QString const &text);
        void setTextMessage(QString const &text);

        void setRed();
        void setGreen();
        void setFont(QFont const &fnt);

    protected:
        void showEvent(QShowEvent *);
        void mousePressEvent(QMouseEvent *event);

    private:
        void setupLayout();


    private:
        QLabel *m_pMessage;

};

