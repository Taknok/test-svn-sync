/****************************************************************************

    xflr5 v6
    Copyright (C) Andre Deperrois 
    GNU General Public License v3

*****************************************************************************/

#pragma once



#include <QDialog>



class PlainTextOutput;

class ObjectPropsDlg : public QDialog
{
    Q_OBJECT

    public:
        ObjectPropsDlg(QWidget *pParent);
        void initDialog(QString title, QString props);

        QSize sizeHint() const override {return QSize(700,500);}

        static void setWindowGeometry(QByteArray geom) {s_Geometry=geom;}
        static QByteArray windowGeometry() {return s_Geometry;}

    protected:
        void showEvent(QShowEvent *event) override;
        void hideEvent(QHideEvent *event) override;

    private:
        void setupLayout();

        PlainTextOutput *m_pctrlOutput;

        static QByteArray s_Geometry;
};




