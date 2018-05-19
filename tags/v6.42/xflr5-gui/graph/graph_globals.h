#ifndef GRAPH_GLOBALS_H
#define GRAPH_GLOBALS_H

#include <QPainter>


void drawPoint(QPainter &painter, int pointStyle, QPoint pt, QColor backColor);
Qt::PenStyle getStyle(int s);


#endif // GRAPH_GLOBALS_H
