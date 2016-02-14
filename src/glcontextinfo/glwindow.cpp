#include "glwindow.h"

#include <QtWidgets>
GLWindow::GLWindow(QWidget *pParent) : QDialog(pParent)
{
	setWindowTitle("OpenGL textures");

	m_pCurrentTextureWidget = new GLTextureWidget(this);
	QHBoxLayout *pMainLayout = new QHBoxLayout;
	pMainLayout->addWidget(m_pCurrentTextureWidget);
	setLayout(pMainLayout);

	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(rotateOneStep()));
	timer->start(31);
}



void GLWindow::rotateOneStep()
{
	if (m_pCurrentTextureWidget)
	{
//		int xRot = (int)((double)qrand()/(double)RAND_MAX *16);
//		int yRot = (int)((double)qrand()/(double)RAND_MAX *16);
//		int zRot = (int)((double)qrand()/(double)RAND_MAX *16);
		int xRot = 13, yRot = 15, zRot = 17;
		m_pCurrentTextureWidget->rotateBy(xRot, yRot, zRot);
	}
}
