#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
BUILD_DIR="$DIR/../build/opencv/"
INSTALL_DIR="$DIR/../install/"
OPENCV_SRC="$DIR/../opencv"
CORES=`nproc --all`

CMAKE_OPTIONS="-DBUILD_DOCS=OFF \
-DBUILD_SHARED_LIBS=OFF \
-DBUILD_opencv_apps=OFF \
-DBUILD_opencv_highgui=OFF \
-DBUILD_opencv_photo=OFF \
-DBUILD_opencv_shape=OFF \
-DBUILD_opencv_superres=OFF \
-DBUILD_opencv_ts=OFF \
-DBUILD_opencv_video=OFF \
-DBUILD_opencv_videoio=OFF \
-DBUILD_opencv_videostab=OFF \
-DWITH_LAPACK=OFF \
-DWITH_EIGEN=ON \
-DCMAKE_BUIL_TYPE=RelWithDebInfo \
-DCMAKE_INSTALL_PREFIX=$INSTALL_DIR" \

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake "$OPENCV_SRC" $CMAKE_OPTIONS
make -j$CORES
make install
