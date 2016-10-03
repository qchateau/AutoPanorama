#-------------------------------------------------
#
# Project created by QtCreator 2016-10-01T11:44:37
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = AutoPanorama
TEMPLATE = app
RC_FILE = autopanorama.rc

win32 {
    LIBS += -L$$PWD/lib/ -lopencv_core310.dll -lopencv_imgcodecs310.dll \
            -lopencv_stitching310.dll -lopencv_imgproc310.dll

    INCLUDEPATH += $$PWD/lib/include
    DEPENDPATH += $$PWD/lib/include
} else {
    INCLUDEPATH += /usr/local/include/opencv2
    LIBS += -L/usr/local/lib -lopencv_core -lopencv_imgcodecs \
            -lopencv_stitching -lopencv_imgproc
}

SOURCES += main.cpp\
        mainwindow.cpp \
    panoramamaker.cpp

HEADERS  += mainwindow.h \
    panoramamaker.h

FORMS    += mainwindow.ui

DISTFILES += \
    autopanorama.rc \
    autopanorama.ico
