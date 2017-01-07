#-------------------------------------------------
#
# Project created by QtCreator 2016-06-07T20:33:33
#
#-------------------------------------------------


TARGET = engine_xflr5
TEMPLATE = lib
CONFIG += qt
CONFIG += staticlib
#CONFIG += create_prl

QT += gui

INCLUDEPATH += $$PWD/../XFoil-lib
INCLUDEPATH += $$PWD/plane_analysis


SOURCES += \
    objects3d/Body.cpp \
    objects3d/CVector.cpp \
    objects3d/Frame.cpp \
    objects3d/NURBSSurface.cpp \
    objects3d/Panel.cpp \
    objects3d/Plane.cpp \
    objects3d/PlaneOpp.cpp \
    objects3d/Quaternion.cpp \
    objects3d/Spline.cpp \
    objects3d/Surface.cpp \
    objects3d/Wing.cpp \
    objects3d/WingOpp.cpp \
    objects3d/WPolar.cpp \
    plane_analysis/LLTAnalysis.cpp \
    plane_analysis/PanelAnalysis.cpp \
    plane_analysis/planeanalysistask.cpp \
    engine_globals.cpp

HEADERS += \
    objects3d/Body.h \
    objects3d/CVector.h \
    objects3d/Frame.h \
    objects3d/NURBSSurface.h \
    objects3d/Panel.h \
    objects3d/Plane.h \
    objects3d/PlaneOpp.h \
    objects3d/PointMass.h \
    objects3d/Quaternion.h \
    objects3d/Spline.h \
    objects3d/Surface.h \
    objects3d/Wing.h \
    objects3d/WingOpp.h \
    objects3d/WingSection.h \
    objects3d/WPolar.h \
    plane_analysis/LLTAnalysis.h \
    plane_analysis/PanelAnalysis.h \
    plane_analysis/planeanalysistask.h \
    engine_globals.h \
    engine_params.h \
    engine_enums.h \
    plane_analysis/planetaskevent.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
