QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

VERSION = 1.1.0.0
TARGET = autopanorama
TEMPLATE = app

INCLUDEPATH += $$PWD/install/include/

win32 {
    contains(QT_ARCH, i386) {
        QMAKE_LFLAGS += -Wl,--large-address-aware
        LIBS += -L$$PWD/install/x86/mingw/staticlib/
        target.path = $$PWD/windows/x86
    } else {
        LIBS += -L$$PWD/install/x64/mingw/staticlib/
        target.path = $$PWD/windows/x64
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
            -lopencv_videoio310 \
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
            -lz \
            -lavifil32 \ # video support
            -lavicap32 \ # video support
            -lmsvfw32 \ # video support
            -lole32 \ # video support
            -loleaut32 \ # video support
            -luuid # video support

    RC_ICONS = "$$PWD/res/autopanorama.ico"
    QMAKE_TARGET_COPYRIGHT = "GNU GPL"
} else {
    target.path = /usr/local/bin/
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
            -lopencv_videoio \
            -lopencv_imgcodecs \
            #-lopencv_video \
            #-lopencv_photo \
            #-lopencv_ml \
            -lopencv_imgproc \
            -lopencv_flann \
            -lopencv_core \
            -lIlmImf \
            -lippicv \
            -llibjasper \
            -Wl,-Bdynamic \
            -lgstreamer-1.0 \ # videos support
            -lgobject-2.0 \ # videos support
            -lglib-2.0 \ # videos support
            -ldc1394 \ # videos support
            -lgstapp-1.0 \ # videos support
            -lgstpbutils-1.0 \ # videos support
            -lgstriff-1.0 \ # videos support
            -lavcodec \ # videos support
            -lavformat \ # videos support
            -lavutil \ # videos support
            -lswscale \ # videos support
            -lgphoto2 \ # videos support
            -lgphoto2_port \ # videos support
            -ldl \
            -lwebp \
            -ltiff \
            -ljpeg \
            -lpng \
            -lz
}

CONFIG(release, debug|release) {
    INSTALLS += target
}

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/panoramamaker.cpp \
    src/qfilewidget.cpp \
    src/innercutfinder.cpp \
    src/videopreprocessor.cpp

HEADERS  += \
    src/mainwindow.h \
    src/panoramamaker.h \
    src/qfilewidget.h \
    src/innercutfinder.h \
    src/env.h \
    src/videopreprocessor.h

FORMS    += \
    src/mainwindow.ui

DISTFILES += \
    res/autopanorama.rc \
    scripts/compile_opencv.sh \
    README.md \
    scripts/win_deploy.iss

RESOURCES += \
    res/application.qrc

