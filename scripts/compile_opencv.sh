#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
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
-DCMAKE_INSTALL_PREFIX=$OPENCV_SRC/install" \

cd "$OPENCV_SRC"
cmake . $CMAKE_OPTIONS
make -j$CORES
make install
