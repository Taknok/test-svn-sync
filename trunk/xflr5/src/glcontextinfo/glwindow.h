#ifndef GLWINDOW_H
#define GLWINDOW_H
#include <QDialog>
#include "gltexturewidget.h"

class GLWindow : public QDialog
{
	Q_OBJECT

public:
	GLWindow(QWidget *pParent=NULL);
	GLTextureWidget *m_pCurrentTextureWidget;

private slots:
	void rotateOneStep();
};

#endif // GLWINDOW_H
