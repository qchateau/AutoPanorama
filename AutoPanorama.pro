#-------------------------------------------------
#
# Project created by QtCreator 2016-10-01T11:44:37
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = AutoPanorama
TEMPLATE = app

INCLUDEPATH += /usr/local/include/opencv2
LIBS += -L/usr/local/lib -lopencv_core -lopencv_imgcodecs -lopencv_stitching

SOURCES += main.cpp\
        mainwindow.cpp \
    panoramamaker.cpp

HEADERS  += mainwindow.h \
    panoramamaker.h

FORMS    += mainwindow.ui
