#!/bin/bash

if [ ! -x "$1" ]; then
    echo "Usage :"
    echo "$0 path_to_executable"
    exit 1
fi

root_dir=$(git rev-parse --show-toplevel)
exec_name="autopanorama"
dpkg_dir=$root_dir/linux
pro_file=$root_dir/AutoPanorama.pro
control_file=$dpkg_dir/control

dpkg_build_dir=$dpkg_dir/pkg
exec_file_dst=$dpkg_build_dir/usr/bin/$exec_name


if test ! -e "$pro_file"; then
    echo "Project file not found at : $pro_file"
    exit 1
fi

if test ! -e "$control_file"; then
    echo "Control file not found at : $control_file"
    exit 1
fi

version=$(cat $pro_file | grep -E "VERSION\s*=" | sed 's/VERSION\s*=\s*\(.*\)$/\1/')
deb_file=$root_dir/autopanorama_${version}.deb
exec_file="$1"

if [ -e $dpkg_build_dir ]; then
    rm -rf $dpkg_build_dir
fi

mkdir $dpkg_build_dir
mkdir $dpkg_build_dir/DEBIAN
mkdir -p $dpkg_build_dir/usr/bin

cp $exec_file $exec_file_dst
cp $control_file $dpkg_build_dir/DEBIAN

cat $control_file | sed "s/^Version:.*$/Version: $version/" > $dpkg_build_dir/DEBIAN/control

echo "This is the content of the archive to be built :"
tree $dpkg_build_dir
dpkg-deb -b $dpkg_build_dir $deb_file

rm -rf $dpkg_build_dir
