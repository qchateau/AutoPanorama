#!/bin/bash -e

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
BUILD_DIR="$DIR/../build/autopanorama/"

$DIR/compile_opencv.sh
$DIR/compile_autopanorama.sh
$DIR/linux_deploy.sh $BUILD_DIR/autopanorama
