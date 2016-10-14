QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = AutoPanorama
TEMPLATE = app

#CONFIG += dynamic

INCLUDEPATH += $$PWD/opencv/install/include/
DEPENDPATH += $$PWD/opencv/install/include/

CONFIG(dynamic) {
    message(Dynamic linking of OpenCV)
    win32 {
        LIBS +=  -L$$PWD/opencv/install/x86/mingw/bin/ -L$$PWD/opencv/install/x86/mingw/lib/ \
                 -lopencv_stitching310.dll \
                 -lopencv_features2d310.dll \
                 -lopencv_imgcodecs310.dll \
                 -lopencv_imgproc310.dll \
                 -lopencv_core310.dll

        QMAKE_LFLAGS += -Wl,--large-address-aware
    } else {
        LIBS += -L/usr/local/lib \
                -lopencv_core \
                -lopencv_imgcodecs \
                -lopencv_stitching \
                -lopencv_imgproc \
                -lopencv_features2d
    }
} else {
    message(Static linking of OpenCV)
    win32 {
        LIBS += -Wl,-Bstatic \
                -L$$PWD/opencv/install/x86/mingw/staticlib/ \
                #-lopencv_shape310 \
                -lopencv_stitching310 \
                #-lopencv_objdetect310 \
                #-lopencv_superres310 \
                #-lopencv_videostab310 \
                -lopencv_calib3d310 \
                -lopencv_features2d310 \
                #-lopencv_highgui310 \
                #-lopencv_videoio310 \
                -lopencv_imgcodecs310 \
                #-lopencv_video310 \
                #-lopencv_photo310 \
                #-lopencv_ml310 \
                -lopencv_imgproc310 \
                -lopencv_flann310 \
                -lopencv_core310 \
                -llibwebp \
                -llibtiff \
                -llibjasper \
                -lIlmImf \
                -llibjpeg \
                -llibpng \
                -lz

        QMAKE_LFLAGS += -Wl,--large-address-aware
    } else {
        LIBS += -Wl,-Bstatic \
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
                -lippicv \
                -Wl,-Bdynamic -ldl
    }
}

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/panoramamaker.cpp \
    src/qfilewidget.cpp \
    src/innercutfinder.cpp

HEADERS  += \
    src/mainwindow.h \
    src/panoramamaker.h \
    src/qfilewidget.h \
    src/innercutfinder.h

FORMS    += \
    src/mainwindow.ui

DISTFILES += \
    res/autopanorama.rc \
    scripts/compile_opencv.sh \
    README.md

RESOURCES += \
    res/application.qrc

RC_FILE = \
    res/autopanorama.rc
