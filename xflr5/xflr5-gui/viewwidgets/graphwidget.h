/****************************************************************************

    GraphWidget Class
        Copyright (C) 2008-2017 Andre Deperrois 

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


#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <QWidget>

class Graph;

class GraphWidget : public QWidget
{
    Q_OBJECT

    friend class XFoilAnalysisDlg;
    friend class LLTAnalysisDlg;
    friend class BatchDlg;
    friend class XDirectTileWidget;
    friend class MiarexTileWidget;

public:
    GraphWidget(QWidget *pParent=nullptr);
    Graph *graph(){return m_pGraph;}

    void setGraph(Graph *pGraph);
    void setTitles(QString &Title, QPoint &Place);

protected:
    void paintEvent(QPaintEvent *pEvent);
    void resizeEvent (QResizeEvent *pEvent);
    void contextMenuEvent (QContextMenuEvent *pEvent);
    void keyPressEvent(QKeyEvent *pEvent);
    void mouseDoubleClickEvent (QMouseEvent *pEvent);
    void mouseMoveEvent(QMouseEvent *pEvent);
    void mousePressEvent(QMouseEvent *pEvent);
    void mouseReleaseEvent(QMouseEvent *pEvent);
    void wheelEvent (QWheelEvent *pEvent);


signals:
    void graphChanged(Graph *);
    void graphResized(Graph *);

public slots:
    void onGraphSettings();
    void onResetGraphScales();


private:
    QPoint m_TitlePosition;
    QString m_GraphTitle;
    Graph *m_pGraph;

    QPoint m_LegendOrigin;
    bool m_bDrawLegend;
    void showLegend(bool bShow);
    void setLegendPosition(QPoint pos);

public:
    static void *s_pMainFrame;
    void *m_pParent;

private:
    QPoint m_LastPoint;           /**< The client position of the previous mousepress event */
    bool m_bTransGraph;
    bool m_bXPressed;                  /**< true if the X key is pressed */
    bool m_bYPressed;                  /**< true if the Y key is pressed */
};

#endif // GRAPHWIDGET_H
