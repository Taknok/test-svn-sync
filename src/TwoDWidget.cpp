

#include <QtGui>
#include "MainFrame.h"
#include <math.h>
#include "Graph/QGraph.h"
#include "Graph/Curve.h"
#include "Miarex/Miarex.h"
#include "XDirect/XDirect.h"
#include "Design/AFoil.h"
#include "XInverse/XInverse.h"
#include "TwoDWidget.h"



TwoDWidget::TwoDWidget(QWidget *parent)
	: QWidget(parent)
{
	m_pMainFrame = NULL;
	m_pXDirect   = NULL;
	m_pMiarex    = NULL;
	m_pAFoil     = NULL;

	setMouseTracking(true);
	setCursor(Qt::CrossCursor);
}



void TwoDWidget::keyPressEvent(QKeyEvent *event)
{
	MainFrame *pMainFrame = (MainFrame*)m_pMainFrame;
	if(pMainFrame->m_iApp == XFOILANALYSIS && m_pXDirect)
	{
		QXDirect* pXDirect = (QXDirect*)m_pXDirect;
		pXDirect->keyPressEvent(event);
	}
	else if(pMainFrame->m_iApp == MIAREX && m_pMiarex)
	{
		QMiarex* pMiarex = (QMiarex*)m_pMiarex;
		pMiarex->keyPressEvent(event);
	}
	else if(pMainFrame->m_iApp == DIRECTDESIGN && m_pAFoil)
	{
		QAFoil *pAFoil= (QAFoil*)m_pAFoil;
		pAFoil->keyPressEvent(event);
	}
	else if(pMainFrame->m_iApp == INVERSEDESIGN && m_pXInverse)
	{
		QXInverse *pXInverse= (QXInverse*)m_pXInverse;
		pXInverse->keyPressEvent(event);
	}
}

void TwoDWidget::keyReleaseEvent(QKeyEvent *event)
{

	MainFrame *pMainFrame = (MainFrame*)m_pMainFrame;
	if(pMainFrame->m_iApp == XFOILANALYSIS && m_pXDirect)
	{
		QXDirect* pXDirect = (QXDirect*)m_pXDirect;
		pXDirect->keyReleaseEvent(event);
	}
	else if(pMainFrame->m_iApp == MIAREX && m_pMiarex)
	{
		QMiarex* pMiarex = (QMiarex*)m_pMiarex;
		pMiarex->keyReleaseEvent(event);
	}
	else if(pMainFrame->m_iApp == DIRECTDESIGN && m_pAFoil)
	{
		QAFoil *pAFoil= (QAFoil*)m_pAFoil;
		pAFoil->keyReleaseEvent(event);
	}
	else if(pMainFrame->m_iApp == INVERSEDESIGN && m_pXInverse)
	{
		QXInverse *pXInverse= (QXInverse*)m_pXInverse;
		pXInverse->keyReleaseEvent(event);
	}
}



void TwoDWidget::mousePressEvent(QMouseEvent *event)
{
	MainFrame *pMainFrame = (MainFrame*)m_pMainFrame;
	if(pMainFrame->m_iApp == XFOILANALYSIS && m_pXDirect)
	{
		QXDirect* pXDirect = (QXDirect*)m_pXDirect;
		pXDirect->mousePressEvent(event);
	}
	else if(pMainFrame->m_iApp == MIAREX && m_pMiarex)
	{
		QMiarex* pMiarex = (QMiarex*)m_pMiarex;
		pMiarex->mousePressEvent(event);
	}
	else if(pMainFrame->m_iApp == DIRECTDESIGN && m_pAFoil)
	{
		QAFoil *pAFoil= (QAFoil*)m_pAFoil;
		pAFoil->mousePressEvent(event);
	}
	else if(pMainFrame->m_iApp == INVERSEDESIGN && m_pXInverse)
	{
		QXInverse *pXInverse= (QXInverse*)m_pXInverse;
		pXInverse->mousePressEvent(event);
	}
}


void TwoDWidget::mouseReleaseEvent(QMouseEvent *event)
{
	MainFrame *pMainFrame = (MainFrame*)m_pMainFrame;
	if(pMainFrame->m_iApp == XFOILANALYSIS && m_pXDirect)
	{
		QXDirect* pXDirect = (QXDirect*)m_pXDirect;
		pXDirect->mouseReleaseEvent(event);
	}
	else if(pMainFrame->m_iApp == MIAREX && m_pMiarex)
	{
		QMiarex* pMiarex = (QMiarex*)m_pMiarex;
		pMiarex->mouseReleaseEvent(event);
	}
	else if(pMainFrame->m_iApp == DIRECTDESIGN && m_pAFoil)
	{
		QAFoil *pAFoil= (QAFoil*)m_pAFoil;
		pAFoil->mouseReleaseEvent(event);
	}
	else if(pMainFrame->m_iApp == INVERSEDESIGN && m_pXInverse)
	{
		QXInverse *pXInverse= (QXInverse*)m_pXInverse;
		pXInverse->mouseReleaseEvent(event);
	}
}


void TwoDWidget::mouseMoveEvent(QMouseEvent *event)
{
	MainFrame *pMainFrame = (MainFrame*)m_pMainFrame;
	if(pMainFrame->m_iApp == XFOILANALYSIS && m_pXDirect)
	{
		QXDirect* pXDirect = (QXDirect*)m_pXDirect;
		pXDirect->mouseMoveEvent(event);
	}
	else if(pMainFrame->m_iApp == MIAREX && m_pMiarex)
	{
		QMiarex* pMiarex = (QMiarex*)m_pMiarex;
		pMiarex->mouseMoveEvent(event);
	}
	else if(pMainFrame->m_iApp == DIRECTDESIGN && m_pAFoil)
	{
		QAFoil *pAFoil= (QAFoil*)m_pAFoil;
		pAFoil->mouseMoveEvent(event);
	}
	else if(pMainFrame->m_iApp == INVERSEDESIGN && m_pXInverse)
	{
		QXInverse *pXInverse= (QXInverse*)m_pXInverse;
		pXInverse->mouseMoveEvent(event);
	}
}

void TwoDWidget::mouseDoubleClickEvent ( QMouseEvent * event )
{
	MainFrame *pMainFrame = (MainFrame*)m_pMainFrame;

	if(pMainFrame->m_iApp == XFOILANALYSIS && m_pXDirect)
	{
		QXDirect *pXDirect = (QXDirect*)m_pXDirect;
		pXDirect->mouseDoubleClickEvent(event );
	}
	else if(pMainFrame->m_iApp == DIRECTDESIGN && m_pAFoil)
	{
		QAFoil *pAFoil= (QAFoil*)m_pAFoil;
		pAFoil->mouseDoubleClickEvent(event);
	}
	else if(pMainFrame->m_iApp == MIAREX && m_pMiarex)
	{
		QMiarex* pMiarex = (QMiarex*)m_pMiarex;
		pMiarex->mouseDoubleClickEvent(event );
	}
	else if(pMainFrame->m_iApp == DIRECTDESIGN && m_pAFoil)
	{
//		QAFoil *pAFoil= (QAFoil*)m_pAFoil;
//		pAFoil->SetScale(rect());
	}
	else if(pMainFrame->m_iApp == INVERSEDESIGN && m_pXInverse)
	{
		QXInverse *pXInverse= (QXInverse*)m_pXInverse;
		pXInverse->mouseDoubleClickEvent(event );
	}
}


void TwoDWidget::resizeEvent(QResizeEvent *event)
{
//	MainFrame *pMainFrame = (MainFrame*)m_pMainFrame;
	QXDirect  *pXDirect  = (QXDirect*)m_pXDirect;
	QMiarex   *pMiarex   = (QMiarex*)m_pMiarex;
	QAFoil    *pAFoil    = (QAFoil*)m_pAFoil;
	QXInverse *pXInverse = (QXInverse*)m_pXInverse;

//	m_rCltRect = rect();

	if(m_pXDirect)
	{
		pXDirect->SetFoilScale(rect());
		pXDirect->SetPolarLegendPos();
	}
	if(m_pMiarex)
	{
		pMiarex->m_bIs2DScaleSet = false;
		pMiarex->Set2DScale();
		if(pMiarex->m_iView==WOPPVIEW)   pMiarex->SetWingLegendPos();
		if(pMiarex->m_iView==WPOLARVIEW) pMiarex->SetWPlrLegendPos();
	}
	if(m_pAFoil)
	{
		pAFoil->SetScale(rect());
	}
	if(m_pXInverse)
	{
		pXInverse->SetScale(rect());
	}
}


void TwoDWidget::wheelEvent(QWheelEvent *event)
{
	MainFrame *pMainFrame = (MainFrame*)m_pMainFrame;
	if(pMainFrame->m_iApp == XFOILANALYSIS && m_pXDirect)
	{
		QXDirect* pXDirect = (QXDirect*)m_pXDirect;
		pXDirect->wheelEvent(event);
	}
	else if(pMainFrame->m_iApp == MIAREX && m_pMiarex)
	{
		QMiarex* pMiarex = (QMiarex*)m_pMiarex;
		pMiarex->wheelEvent(event);
	}
	else if(pMainFrame->m_iApp == DIRECTDESIGN && m_pAFoil)
	{
		QAFoil *pAFoil= (QAFoil*)m_pAFoil;
		pAFoil->wheelEvent(event);
	}
	else if(pMainFrame->m_iApp == INVERSEDESIGN && m_pXInverse)
	{
		QXInverse *pXInverse= (QXInverse*)m_pXInverse;
		pXInverse->wheelEvent(event);
	}
}


void TwoDWidget::paintEvent(QPaintEvent *event)
{
	MainFrame *pMainFrame = (MainFrame*)m_pMainFrame;
	if(pMainFrame->m_iApp == XFOILANALYSIS && m_pXDirect)
	{	
		QXDirect* pXDirect = (QXDirect*)m_pXDirect;
		QPainter painter(this);
		pXDirect->PaintView(painter);
	}
	else if(pMainFrame->m_iApp == MIAREX && m_pMiarex)
	{
		QMiarex* pMiarex = (QMiarex*)m_pMiarex;
		QPainter painter(this);
		pMiarex->PaintView(painter);
	}
	else if(pMainFrame->m_iApp == DIRECTDESIGN && m_pAFoil)
	{
		QAFoil *pAFoil= (QAFoil*)m_pAFoil;
		QPainter painter(this);
		pAFoil->PaintView(painter);
	}
	else if(pMainFrame->m_iApp == INVERSEDESIGN && m_pXInverse)
	{
		QXInverse *pXInverse= (QXInverse*)m_pXInverse;
		QPainter painter(this);
		pXInverse->PaintView(painter);
	}
	else
	{
		QPainter painter(this);
		painter.fillRect(rect(), pMainFrame->m_BackgroundColor);
	}
}


void TwoDWidget::contextMenuEvent (QContextMenuEvent * event)
{
	MainFrame *pMainFrame = (MainFrame*)m_pMainFrame;
	QPoint ScreenPt = event->globalPos();
	QPoint CltPt = event->pos();

	switch(pMainFrame->m_iApp)
	{
		case MIAREX:
		{
			QMiarex *pMiarex = (QMiarex*)m_pMiarex;
			if(event->reason()==QContextMenuEvent::Keyboard)
			{
				ScreenPt.rx() = pMiarex->m_LastPoint.x()+pMainFrame->pos().x()+geometry().x();
				ScreenPt.ry() = pMiarex->m_LastPoint.y()+pMainFrame->pos().y()+geometry().y();
			}

			pMiarex->m_pCurGraph = pMiarex->GetGraph(CltPt);
			if(pMiarex->m_iView==WOPPVIEW)         pMainFrame->WOppCtxMenu->exec(ScreenPt);
			else if (pMiarex->m_iView==WPOLARVIEW) pMainFrame->WPlrCtxMenu->exec(ScreenPt);
			else if (pMiarex->m_iView==WCPVIEW)    pMainFrame->WCpCtxMenu->exec(ScreenPt);
			else if(pMiarex->m_iView==WSTABVIEW)
			{
				if(pMiarex->m_iStabilityView==STABTIMEVIEW)       pMainFrame->WTimeCtxMenu->exec(ScreenPt);
				else if(pMiarex->m_iStabilityView==STABPOLARVIEW) pMainFrame->WPlrCtxMenu->exec(ScreenPt);
			}
			break;
		}
		case XFOILANALYSIS:
		{
			QXDirect *pXDirect = (QXDirect*)m_pXDirect;
			pXDirect->m_pCurGraph = pXDirect->GetGraph(CltPt);
			if(pXDirect->m_bPolar) pMainFrame->OperPolarCtxMenu->exec(ScreenPt);
			else                   pMainFrame->OperFoilCtxMenu->exec(ScreenPt);
			break;
		}
		case DIRECTDESIGN:
		{
			pMainFrame->AFoilCtxMenu->exec(ScreenPt);
			break;
		}
		case INVERSEDESIGN:
		{
			pMainFrame->InverseContextMenu->exec(ScreenPt);
			break;
		}
	}
}
