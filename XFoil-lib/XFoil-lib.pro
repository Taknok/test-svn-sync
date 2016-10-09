#-------------------------------------------------
#
# Project created by QtCreator 2016-10-08T16:47:55
#
#-------------------------------------------------

QT       -= gui

TARGET = XFoil-lib
TEMPLATE = lib


CONFIG += qt
CONFIG += staticlib


SOURCES += \
    XFoil.cpp \
    XFoilTask.cpp \
    objects/CVector.cpp \
    objects/Foil.cpp \
    objects/OpPoint.cpp \
    objects/Polar.cpp \
    objects/Spline.cpp \
    xfoil_globals.cpp

HEADERS +=\
    XFoil.h \
    XFoilTask.h \
    xfoiltaskevent.h \
    xfoil_params.h \
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
