#!/bin/bash -e

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT="$DIR/.."
BUILD_DIR="$DIR/../build/autopanorama/"
CORES=`nproc --all`

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
qmake $ROOT/AutoPanorama.pro
make -j$CORES
