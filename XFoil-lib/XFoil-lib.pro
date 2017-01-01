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

SOURCES += \
    XFoil.cpp \
    XFoilTask.cpp \
    objects2d/CVector.cpp \
    objects2d/Foil.cpp \
    objects2d/OpPoint.cpp \
    objects2d/Polar.cpp \
    objects2d/Spline.cpp \
    xfoil_globals.cpp

HEADERS +=\
    XFoil.h \
    XFoilTask.h \
    xfoiltaskevent.h \
    xfoil_params.h \
    objects2d/CVector.h \
    objects2d/Foil.h \
    objects2d/OpPoint.h \
    objects2d/Polar.h \
    objects2d/Spline.h \
    xfoil_globals.h \
    xfoil_enums.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
