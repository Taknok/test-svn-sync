#-------------------------------------------------
#
# Project created by QtCreator 2017-01-18T04:06:28
#
#-------------------------------------------------

QT       -= gui

TARGET = XFoil-lib
TEMPLATE = lib

DEFINES += XFOILLIB_LIBRARY

SOURCES += \
    XFoil.cpp

HEADERS +=\
        xfoil-lib_global.h \
    XFoil.h \
    xfoil_params.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
