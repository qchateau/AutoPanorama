QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

VERSION = 1.0.1.1
TARGET = AutoPanorama
TEMPLATE = app

INCLUDEPATH += $$PWD/install/include/
DEPENDPATH += $$PWD/install/include/

win32 {
    contains(QT_ARCH, i386) {
        QMAKE_LFLAGS += -Wl,--large-address-aware
        LIBS += -L$$PWD/install/x86/mingw/staticlib/
            target.path = $$PWD/windows/x86
    } else {
        LIBS += -L$$PWD/install/x64/mingw/staticlib/
            target.path = $$PWD/windows/x64
    }
    CONFIG(release, debug|release) {
        INSTALLS += target
    }
    LIBS += -Wl,-Bstatic \
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

    RC_ICONS = "$$PWD/res/autopanorama.ico"
    QMAKE_TARGET_COPYRIGHT = "GNU GPL"
} else {
    LIBS += -Wl,-Bstatic \
            -L$$PWD/install/lib \
            -L$$PWD/install/share/OpenCV/3rdparty/lib \
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
            -Wl,-Bdynamic \
            -ldl
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
    src/innercutfinder.h \
    src/env.h

FORMS    += \
    src/mainwindow.ui

DISTFILES += \
    res/autopanorama.rc \
    scripts/compile_opencv.sh \
    README.md \
    scripts/win_deploy.iss

RESOURCES += \
    res/application.qrc

