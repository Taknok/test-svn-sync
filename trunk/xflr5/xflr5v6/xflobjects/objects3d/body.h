/****************************************************************************

    Body Class
    Copyright (C) 2007-2016 Andre Deperrois

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

/** @file
 * This file implements the definition of the Body class.
 */



#ifndef BODY_H
#define BODY_H


#include <QColor>
#include <QTextStream>
#include <QVarLengthArray>

#include <xflobjects/objects3d/panel.h>
#include <xflobjects/objects3d/nurbssurface.h>
#include <xflobjects/objects3d/pointmass.h>


#include <xflcore/core_enums.h>

#define NHOOPPOINTS 67  //used for display and to export the geometry
#define NXPOINTS 97     //used for display and to export the geometry


/**
 * This class :
 *     - defines the body object,
 *      - provides the methods for the calculation of the plane's geometric properties,
 *   - porvides methods for the panel calculations.
 * The data is stored in International Standard Units, i.e. meters, kg, and seconds.
 * Angular data is stored in degrees.
 */
class Body
{
public:
    Body();
    ~Body();

    bool isInNURBSBody(double x, double z) const;
    bool isInNURBSBodyOld(Vector3d Pt);
    bool intersect(Vector3d A, Vector3d B, Vector3d &I, bool bRight) const;
    bool intersectFlatPanels(const Vector3d &A, const Vector3d &B, Vector3d &I) const;
    bool intersectNURBS(Vector3d A, Vector3d B, Vector3d &I, bool bRight) const;

    bool importDefinition(QTextStream &inStream, double mtoUnit, QString &errorMessage);

    int insertFrame(Vector3d Real);
    int insertFrameBefore(int iFrame);
    int insertFrameAfter(int iFrame);
    int insertPoint(Vector3d Real);
    int isFramePos(Vector3d Real, double ZoomFactor);
    int removeFrame(int n);
    int readFrame(QTextStream &in, int &Line, Frame *pFrame, double const &Unit);

    double length() const;

    double getu(double x) const;
    double getv(double u, Vector3d r, bool bRight) const;
    double getSectionArcLength(double x) const;

    Vector3d centerPoint(double u) const;
    Vector3d leadingPoint() const;

    void clearPointMasses();
    void computeAero(double *Cp, double &XCP, double &YCP, double &ZCP,
                     double &GCm, double &GRm, double &GYm, double &Alpha, Vector3d &CoG) const;
    void duplicate(const Body *pBody);
    void getPoint(double u, double v, bool bRight, Vector3d &Pt) const;
    Vector3d Point(double u, double v, bool bRight) const;
    void removeActiveFrame();
    void removeSideLine(int SideLine);
    void scale(double XFactor, double YFactor, double ZFactor, bool bFrameOnly=false, int FrameID=0);
    void translate(double XTrans, double, double ZTrans, bool bFrameOnly=false, int FrameID=0);
    void translate(Vector3d T, bool bFrameOnly=false, int FrameID=0);
    void setNURBSKnots();
    void setPanelPos();
    void setEdgeWeight(double uw, double vw);

    Frame *frame(int iFrame);
    Frame const *frameAt(int iFrame) const;
    Frame *activeFrame() const;

    int setActiveFrame(Frame const*pFrame);
    Frame *setActiveFrame(int iFrame);


    double framePosition(int iFrame);
    int frameCount()      const {return m_SplineSurface.frameCount();}
    int framePointCount() const {return m_SplineSurface.framePointCount();}
    int sideLineCount()   const {return m_SplineSurface.framePointCount();}// same as FramePointCount();

    void computeBodyAxisInertia();
    void computeVolumeInertia(Vector3d &CoG, double &CoGIxx, double &CoGIyy, double &CoGIzz, double &CoGIxz);
    double totalMass() const;
    double const &volumeMass() const {return m_VolumeMass;}
    void setVolumeMass(double m) {m_VolumeMass=m;}

    Vector3d CoG() {return m_CoG;}


    QString &bodyName(){return m_BodyName;}
    QString &bodyDescription() {return m_BodyDescription;}

    bool hasTextures() const {return m_bTextures;}
    void setTextures(bool bTextures) {m_bTextures = bTextures;}

    QColor const &bodyColor() const {return m_BodyColor;}
    QColor &bodyColor() {return m_BodyColor;}
    void setBodyColor(QColor const&color) {m_BodyColor=color;}

    NURBSSurface& nurbs() {return m_SplineSurface;}

    Xfl::enumBodyLineType &bodyType(){return m_LineType;}
    bool isFlatPanelType() const {return m_LineType==Xfl::BODYPANELTYPE;}
    bool isSplineType()    const {return m_LineType==Xfl::BODYSPLINETYPE;}

    NURBSSurface *splineSurface() {return &m_SplineSurface;}

    void setNXPanels(int nx) {m_nxPanels=nx;}
    void setNHPanels(int nh) {m_nhPanels=nh;}
    int nxPanels() const {return m_nxPanels;}
    int nhPanels() const {return m_nhPanels;}

    void exportGeometry(QTextStream &outStream, int type, double mtoUnit, int nx, int nh) const;
    void exportSTLBinary(QDataStream &outStream, int nXPanels, int nHoopPanels, double unit) const;
    void exportSTLBinarySplines(QDataStream &outStream, int nXPanels, int nHoopPanels, double unit) const;
    void exportSTLBinaryFlatPanels(QDataStream &outStream, double unitd) const;

    bool exportBodyDefinition(QTextStream &outStream, double mtoUnit) const;

    bool serializeBodyWPA(QDataStream &ar, bool bIsStoring);
    bool serializeBodyXFL(QDataStream &ar, bool bIsStoring);

    int readValues(QString line, double &x, double &y, double &z) const;
    bool Rewind1Line(QTextStream &in, int &Line, QString &strong) const;


    //____________________VARIABLES_____________________________________________

    QString m_BodyName;                       /**< the Body's name, used as its reference */
    QString m_BodyDescription;                /**< a free description for the Body */

    NURBSSurface m_SplineSurface;             /**< the spline surface which defines the left (port) side of the body */

    Xfl::enumBodyLineType m_LineType;              /**< the type of body surfaces 1=PANELS  2=NURBS */

    int m_iActiveFrame;                          /**< the currently selected Frame for display */
    int m_iHighlightFrame;                    /**< the currently selected Frame to highlight */
    int m_NElements;                          /**< the number of mesh elements for this Body object = m_nxPanels * m_nhPanels *2 */
    int m_nxPanels;                           /**< For a NURBS body, the number of mesh elements in the direction of the x-axis */
    int m_nhPanels;                           /**< For a NURBS body, the number of mesh elements in the hoop direction */

    int m_BodyStyle;                          /**< the index of the spline's style */
    int m_BodyWidth;                          /**< the width of the spline */
    QColor m_BodyColor;                       /**< the Body's display color */

    double m_Bunch;                            /**< a bunch parameter to set the density of the points of the NURBS surface; unused */

    double m_VolumeMass;                       /**< the mass of the Body's structure, excluding point masses */
    QVector<PointMass*> m_PointMass;             /**< the array of PointMass objects */

    double m_CoGIxx;                           /**< the Ixx component of the inertia tensor, calculated at the CoG */
    double m_CoGIyy;                           /**< the Ixx component of the inertia tensor, calculated at the CoG */
    double m_CoGIzz;                           /**< the Ixx component of the inertia tensor, calculated at the CoG */
    double m_CoGIxz;                           /**< the Ixx component of the inertia tensor, calculated at the CoG */
    Vector3d m_CoG;                             /**< the position of the CoG */


    QVector<int> m_xPanels;              /**< the number of mesh panels between two frames */
    QVector<int> m_hPanels;              /**< the number of mesh panels in the hoop direction between two sidelines */
    QVector<double> m_XPanelPos;


    Panel *m_pBodyPanel;                       /** A pointer to the first body panel in the array */

    //allocate temporary variables to
    //avoid lengthy memory allocation times on the stack
    mutable double value, bs, cs;
    mutable Vector3d t_R, t_Prod, t_Q, t_r, t_N;
    //    mutable Vector3d P0, P1, P2, PI;

    bool m_bTextures;

};
#endif

