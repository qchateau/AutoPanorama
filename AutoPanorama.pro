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


INCLUDEPATH += $$PWD/opencv/install/include/
DEPENDPATH += $$PWD/opencv/install/include/

win32 {
    LIBS += -L$$PWD/opencv/install/x86/mingw/bin/ -L$$PWD/opencv/install/x86/mingw/lib/ \
            -lopencv_stitching310.dll \
            -lopencv_features2d310.dll \
            -lopencv_imgcodecs310.dll \
            -lopencv_imgproc310.dll \
            -lopencv_core310.dll

    QMAKE_LFLAGS += -Wl,--large-address-aware
} else {
    LIBS += -Wl,-Bstatic \
            #-L/usr/local/lib -L/usr/local/share/OpenCV/3rdparty/lib \
            -L$$PWD/opencv/install/lib -L$$PWD/opencv/install/share/OpenCV/3rdparty/lib \
            #-lopencv_shape \
            -lopencv_stitching \
            #-lopencv_objdetect \
            #-lopencv_superres \
            #-lopencv_videostab \
            -lopencv_calib3d \
            -lopencv_features2d \
            #-lopencv_highgui \
            #-lopencv_videoio \
            -lopencv_imgcodecs \
            #-lopencv_video \
            #-lopencv_photo \
            #-lopencv_ml \
            -lopencv_imgproc \
            -lopencv_flann \
            -lopencv_core \
            -llibwebp \
            -llibtiff \
            -llibjasper \
            -lIlmImf \
            -ljpeg \
            -lpng \
            -lz \
            #-lgtk-x11-2.0 -lgdk-x11-2.0 -lpangocairo-1.0 \
            #-lcairo -lgio-2.0 -lpangoft2-1.0 -lpango-1.0 -lgobject-2.0 -lfontconfig \
            #-lfreetype -lgthread-2.0 -lglib-2.0 -lavcodec-ffmpeg -lavformat-ffmpeg -lavutil-ffmpeg \
            #-lswscale-ffmpeg -lbz2 -lstdc++ -lm -lpthread -lrt \
            -Wl,-Bdynamic -ldl
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

