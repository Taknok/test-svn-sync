#-------------------------------------------------
#
# Project created by QtCreator 2017-01-18T20:08:35
#
#-------------------------------------------------

#QT       -= gui

TARGET = graph-lib
TEMPLATE = lib
CONFIG += staticlib

DEFINES += GRAPHLIB_LIBRARY

SOURCES += \
    Curve.cpp \
    Graph.cpp \
    QGraph.cpp \
    graph_globals.cpp

HEADERS +=\
        graph-lib_global.h \
    Curve.h \
    Graph.h \
    QGraph.h \
    graph_globals.h \
    linestyle.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
