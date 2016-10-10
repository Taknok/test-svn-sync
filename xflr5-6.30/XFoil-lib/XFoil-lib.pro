#-------------------------------------------------
#
# Project created by QtCreator 2016-10-08T16:47:55
#
#-------------------------------------------------

QT       -= gui

CONFIG += qt
CONFIG += staticlib

TARGET = XFoil-lib
TEMPLATE = lib

DEFINES += XFOILLIB_LIBRARY


SOURCES += xfoil_lib.cpp \
    XFoil.cpp \
    XFoilTask.cpp \
    objects/CVector.cpp \
    objects/Foil.cpp \
    objects/OpPoint.cpp \
    objects/Polar.cpp \
    objects/Spline.cpp \
    xfoil_globals.cpp

HEADERS += xfoil_lib.h\
    XFoil.h \
    XFoilTask.h \
    xfoiltaskevent.h \
    xfoil_params.h \
    objects/CRectangle.h \
    objects/CVector.h \
    objects/Foil.h \
    objects/OpPoint.h \
    objects/Polar.h \
    objects/Spline.h \
    xfoil_globals.h \
    xfoil_enums.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
