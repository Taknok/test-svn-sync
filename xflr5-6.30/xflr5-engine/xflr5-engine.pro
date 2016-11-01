#-------------------------------------------------
#
# Project created by QtCreator 2016-06-07T20:33:33
#
#-------------------------------------------------


TARGET = engine_xflr5
TEMPLATE = lib
CONFIG += qt
CONFIG += staticlib

QT += gui

DEFINES += ENGINE_XFLR5_LIB

INCLUDEPATH += $$PWD/objects
INCLUDEPATH += $$PWD/../XFoil-lib
INCLUDEPATH += $$PWD/plane_analysis


SOURCES += \
    objects/Body.cpp \
    objects/CVector.cpp \
    objects/Frame.cpp \
    objects/NURBSSurface.cpp \
    objects/Panel.cpp \
    objects/Plane.cpp \
    objects/PlaneOpp.cpp \
    objects/Quaternion.cpp \
    objects/Spline.cpp \
    objects/Surface.cpp \
    objects/Wing.cpp \
    objects/WingOpp.cpp \
    objects/WPolar.cpp \
    plane_analysis/LLTAnalysis.cpp \
    plane_analysis/PanelAnalysis.cpp \
    plane_analysis/planeanalysistask.cpp \
    engine_globals.cpp

HEADERS += \
    objects/Body.h \
    objects/CRectangle.h \
    objects/CVector.h \
    objects/Frame.h \
    objects/NURBSSurface.h \
    objects/Panel.h \
    objects/Plane.h \
    objects/PlaneOpp.h \
    objects/PointMass.h \
    objects/Quaternion.h \
    objects/Spline.h \
    objects/Surface.h \
    objects/Wing.h \
    objects/WingOpp.h \
    objects/WingSection.h \
    objects/WPolar.h \
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
