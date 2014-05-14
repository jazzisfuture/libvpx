#-------------------------------------------------
#
# Project created by QtCreator 2014-04-22T15:35:25
#
#-------------------------------------------------

QT += core
QT += gui
QT += widgets
QT += multimedia

TARGET = vp9bat

TEMPLATE = app

SOURCES += main.cpp \
    mainwindow.cpp \
    videoinfowidget.cpp \
    txwidget.cpp \
    bitstream.cpp \
    statspiewidget.cpp \
    pixelvalueswidget.cpp \
    filmstripwidget.cpp \
    sampledetailwidget.cpp \
    vp9bat.pb.cc

HEADERS += \
    mainwindow.h \
    styles.h \
    videoinfowidget.h \
    txwidget.h \
    bitstream.h \
    statspiewidget.h \
    pixelvalueswidget.h \
    filmstripwidget.h \
    sampledetailwidget.h \
    vp9bat.h \
    vp9bat.pb.h

INCLUDEPATH += \
    .. \
    /usr/local/include


unix|win32: LIBS += -lprotobuf
