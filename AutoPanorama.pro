QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

LINUX_VERSION = 1.2.0.4
WIN_VERSION = 1.1.0.1
TARGET = autopanorama
TEMPLATE = app
QMAKE_CXXFLAGS += -std=c++11

INCLUDEPATH += $$PWD/install/include/opencv4/

win32 {
    VERSION = $$WIN_VERSION
    OPENCV_VERSION = 320
    contains(QT_ARCH, i386) {
        QMAKE_LFLAGS += -Wl,--large-address-aware
        LIBS += -L$$PWD/install/x86/mingw/staticlib/
        LIBS += -L$$PWD/install/x86/mingw/bin/
        FFMPEG_LIB = opencv_ffmpeg$${OPENCV_VERSION}
        TARGET_PATH = $$PWD/windows/x86
    } else {
        LIBS += -L$$PWD/install/x64/mingw/staticlib/
        LIBS += -L$$PWD/install/x64/mingw/bin/
        FFMPEG_LIB = opencv_ffmpeg$${OPENCV_VERSION}_64
        TARGET_PATH = $$PWD/windows/x64
    }
    target.path = $$TARGET_PATH
    LIBS += \
            -l$${FFMPEG_LIB} \
            -Wl,-Bstatic \
            #-lopencv_shape$${OPENCV_VERSION} \
            -lopencv_stitching$${OPENCV_VERSION} \
            #-lopencv_objdetect$${OPENCV_VERSION} \
            #-lopencv_superres$${OPENCV_VERSION} \
            #-lopencv_videostab$${OPENCV_VERSION} \
            -lopencv_calib3d$${OPENCV_VERSION} \
            -lopencv_features2d$${OPENCV_VERSION} \
            #-lopencv_highgui$${OPENCV_VERSION} \
            -lopencv_videoio$${OPENCV_VERSION} \
            -lopencv_imgcodecs$${OPENCV_VERSION} \
            #-lopencv_video$${OPENCV_VERSION} \
            #-lopencv_photo$${OPENCV_VERSION} \
            #-lopencv_ml$${OPENCV_VERSION} \
            -lopencv_imgproc$${OPENCV_VERSION} \
            -lopencv_flann$${OPENCV_VERSION} \
            -lopencv_core$${OPENCV_VERSION} \
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

    # copy ffmpeg dll
    copyfiles.commands = $$QMAKE_COPY \"$$shell_path($$PWD/install/x64/mingw/bin/$${FFMPEG_LIB}.dll)\" \"$$shell_path($${TARGET_PATH}$$)\" $$escape_expand(\\n\\t)
    QMAKE_EXTRA_TARGETS += copyfiles
    POST_TARGETDEPS += copyfiles

    RC_ICONS = "$$PWD/res/autopanorama.ico"
    QMAKE_TARGET_COPYRIGHT = "GNU GPL"

    CONFIG(release, debug|release) {
        INSTALLS += target
    }
} else {
    VERSION = $$LINUX_VERSION
    target.path = /usr/local/bin/
    QMAKE_RPATHDIR += /usr/share/autopanorama/lib
    LIBS += -L$$PWD/install/lib \
            -lopencv_stitching \
            -lopencv_features2d \
            -lopencv_videoio \
            -lopencv_imgcodecs \
            -lopencv_imgproc \
            -lopencv_core
}

DEFINES += APP_VERSION=\\\"$$VERSION\\\"

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
    linux/autopanorama.desktop \
    linux/control \
    scripts/compile_opencv.sh \
    scripts/linux_deploy.sh \
    README.md \
    scripts/win_deploy.iss

RESOURCES += \
    res/application.qrc

