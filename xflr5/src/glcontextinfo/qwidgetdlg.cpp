#include "qwidgetdlg.h"
#include <QVBoxLayout>

QWidgetDlg::QWidgetDlg()
{
	QVBoxLayout *pMainLayout = new QVBoxLayout;
	pMainLayout->addWidget(&m_gl3Widget);
	setLayout(pMainLayout);
}
