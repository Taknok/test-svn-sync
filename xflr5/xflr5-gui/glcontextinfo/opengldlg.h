/****************************************************************************

    OpenGlDlg Class
    Copyright (C) 2019 Andre Deperrois

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


#include <QWidget>
#include <QDialog>
#include <QComboBox>
#include <QVBoxLayout>
#include <QSurface>
#include <QStackedWidget>
#include <QPlainTextEdit>
#include <QDialogButtonBox>


class gl3dTestView;


class OpenGlDlg : public QDialog
{
    Q_OBJECT
    public:
        OpenGlDlg(QWidget *pParent=nullptr);

    private slots:
        void onCreateContext();
        void onRenderWindowReady();
        void onRenderWindowError(const QString &msg);
        void onVersionChanged() {m_bVersionChanged=true;}
        void onButton(QAbstractButton *pButton);
        void onApply();

    private:
        QSize sizeHint() const override {return QSize(1200,900);}
        void keyPressEvent(QKeyEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;

        void addVersions(QLayout *pLayout);
        void addProfiles(QLayout *pLayout);
        void addOptions(QLayout *pLayout);

        void printFormat(const QSurfaceFormat &format, bool bFull=false);
        void setupLayout();

        QDialogButtonBox *m_pButtonBox;
        QComboBox *m_pctrlVersionCbBox;
        QLayout *m_profiles;
        QLayout *m_pOptionsLayout;
        QPlainTextEdit *m_pctrlglOutput;

        QStackedWidget *m_pStackWt;

        gl3dTestView *m_pgl3dTestView;

        QPair<QSurfaceFormat::OpenGLContextProfile, QString> m_SurfaceProfiles[3];

        bool m_bVersionChanged;
        static QByteArray s_Geometry;
};

