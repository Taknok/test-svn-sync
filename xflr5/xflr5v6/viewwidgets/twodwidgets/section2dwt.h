/****************************************************************************

    Section2dWidget Class
    Copyright (C) 2015-2019 Andre Deperrois

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

#pragma once

#include <QWidget>
#include <QMenu>
#include <QPixmap>

#include <xflobjects/objects3d/vector3d.h>
#include <xflcore/ls2.h>

/**
* @file
* @brief This file contains the declaration of the class Section2dWidget, used for 2d drawing of object line-type sections.
*/

/**
*@class Section2dWidget
* @brief This class is used for 2d drawing of object line-type sections such as Foils, body Frames, and body axial lines.
*
* This class is abstract.
* The pure virtual methods highlightPoint(), selectPoint(), dragSelectedPoint(), onInsertPt(), and onRemovePt() are dependent on the object type
* and must therefore be implemented in the derived class;
*/


class Section2dWidget : public QWidget
{
    friend class MainFrame;
    friend class BodyLineWidget;
    friend class BodyFrameWidget;
    friend class Direct2dDesign;
    friend class AFoil;

    Q_OBJECT
    public:
        Section2dWidget(QWidget *parent = nullptr);

        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;

        void contextMenuEvent (QContextMenuEvent *pEvent) override;
        void keyPressEvent(QKeyEvent *pEvent) override;
        void keyReleaseEvent(QKeyEvent *pEvent) override;
        void mouseDoubleClickEvent (QMouseEvent *pEvent) override;
        void mouseMoveEvent(QMouseEvent *pEvent) override;
        void mousePressEvent(QMouseEvent *pEvent) override;
        void mouseReleaseEvent(QMouseEvent *pEvent) override;
        virtual void paintEvent(QPaintEvent *pEvent) override;
        void resizeEvent (QResizeEvent *pEvent) override;
        void wheelEvent (QWheelEvent *pEvent) override;

        void setContextMenu(QMenu *pMenu);

        void setNeutralLineColor(QColor clr){m_NeutralStyle.m_Color = clr;}
        QColor const& neutralLineColor() const {return m_NeutralStyle.m_Color;}

    protected:
        Vector3d mousetoReal(QPoint &point);
        virtual void setScale();
        virtual void createActions();
        virtual void createContextMenu();
        void drawScale(QPainter &painter, double scalex);
        void drawScaleLegend(QPainter &painter);
        void drawBackImage(QPainter &painter);
        void drawXGrid(QPainter &painter, double scalex, QPointF Offset);
        void drawYGrid(QPainter &painter, double scaley, QPointF Offset);
        void drawXMinGrid(QPainter &painter, double scalex, QPointF Offset);
        void drawYMinGrid(QPainter &painter, double scaley, QPointF Offset);

        void releaseZoom();
        void zoomView(double zoomFactor);

        void paintGrids(QPainter &painter);

        virtual int highlightPoint(Vector3d real) = 0;
        virtual int selectPoint(Vector3d real) = 0;
        virtual void dragSelectedPoint(double x, double y) =0;


    public slots:
        void onResetXScale();
        void onResetYScale();
        void onResetScales();
        void onClearBackImage();
        void onLoadBackImage();
        void onGridSettings();
        void onZoomLess();
        void onZoomIn();
        void onZoomYOnly();
        virtual void onInsertPt() {}
        virtual void onRemovePt() {}

    signals:
        void objectModified();


    public:
        Vector3d m_objectOffset;
        double m_objectScale;


    protected:
        QCursor m_hcMove;           /**< the cursor to display when moving the viewport */
        QCursor m_hcCross;          /**< the cursor to display in the client area, when not dragging or zooming */

        QMenu *m_pSection2dContextMenu;
        QVector<QAction*> m_ActionList;

        bool m_bScale;              /**< true if the scale should be displayed */
        bool m_bZoomPlus;           /**< true if the user is in the process of zooming in by drawing a rectangle */
        bool m_bZoomYOnly;          /**< true if only the y-axis should be scaled */
        bool m_bTrans;              /**< true if the view is being dragged by the user */
        bool m_bDrag;               /**< true if a point is being dragged by the user */
        bool m_bShowLegend;         /**< true if the legend should be shown */
        bool m_bXDown;              /**< true if the 'X' key is pressed */
        bool m_bYDown;              /**< true if the 'Y' key is pressed */

        LS2 m_XStyle, m_YStyle;
        LS2 m_XMinStyle;            /**< the style of the minor X-grid */
        LS2 m_YMinStyle;            /**< the style of the minor Y-grid */
        LS2 m_NeutralStyle;         /**< the style of the neutral line y=0 */

        double m_XGridUnit;         /**< the unit of the main X-grid */
        double m_YGridUnit;         /**< the unit of the main Y-grid */
        double m_XMinUnit;          /**< the unit of the minor X-grid */
        double m_YMinUnit;          /**< the unit of the minor Y-grid */


        double m_fScale;            /**< the current scale of the display */
        double m_fScaleY;           /**< the ratio between the  y and x scales */
        double m_fRefScale;         /**< the reference scale of the display */

        QPointF m_ptOffset;          /**< the foil's leading edge position in screen coordinates */
        QPointF m_ViewportTrans;     /**< the translation of the viewport */
        QPoint m_PointDown;         /**< the screen point where the last left-click occured */

        QRect m_ZoomRect;           /**< the user-defined rectangle for zooming in */

        Vector3d m_MousePos;         /**< the mouse position */


        bool m_bIsImageLoaded;      /**< true if a backgruond image is loaded */
        QPixmap m_BackImage;        /**< the QPixmap object with the background image */

        //    QAction *m_pInsertPt, *m_pRemovePt, *m_pResetScaleAction, *m_pGridSettingsAction;
        //    QAction *m_pLoadBackImage, *m_pClearBackImage;

};




