#!/bin/bash

if [ ! -x "$1" ]; then
    echo "Usage :"
    echo "$0 path_to_executable"
    exit 1
fi

exec_file="$1"
root_dir=$(git rev-parse --show-toplevel)
exec_name="autopanorama"
dpkg_dir=$root_dir/linux
res_dir=$root_dir/res
pro_file=$root_dir/AutoPanorama.pro
control_file=$dpkg_dir/control
desktop_file=$dpkg_dir/autopanorama.desktop
icon_file=$res_dir/autopanorama.png

needed_libs=$(ldd $exec_file)
libs_files=$(ls $root_dir/build/opencv/lib/*.so)
used_libs_files=""
for file in $libs_files; do
    basename=$(basename $file)
    if echo $needed_libs | grep -q $basename; then
        used_libs_files="$used_libs_files $(ls $file*)"
    fi
done

dpkg_build_dir=$dpkg_dir/pkg
exec_file_dst=$dpkg_build_dir/usr/bin/$exec_name
desktop_file_dst=$dpkg_build_dir/usr/share/applications/${exec_name}.desktop
icon_file_dst=$dpkg_build_dir/usr/share/autopanorama/autopanorama.png
libs_dst=$dpkg_build_dir/usr/share/autopanorama/lib/


if test ! -e "$pro_file"; then
    echo "Project file not found at : $pro_file"
    exit 1
fi

if test ! -e "$control_file"; then
    echo "Control file not found at : $control_file"
    exit 1
fi

version=$(cat $pro_file | grep -E "^\s*VERSION\s*=" | sed 's/VERSION\s*=\s*\(.*\)$/\1/')
deb_file=$root_dir/autopanorama_${version}.deb

if [ -e $dpkg_build_dir ]; then
    rm -rf $dpkg_build_dir
fi

mkdir $dpkg_build_dir
mkdir $dpkg_build_dir/DEBIAN
mkdir -p $(dirname $exec_file_dst)
mkdir -p $(dirname $desktop_file_dst)
mkdir -p $(dirname $icon_file_dst)
mkdir -p $libs_dst

cp $exec_file $exec_file_dst
cp $desktop_file $desktop_file_dst
cp $icon_file $icon_file_dst
cp -a $used_libs_files $libs_dst
cp $control_file $dpkg_build_dir/DEBIAN

cat $control_file | sed "s/^Version:.*$/Version: $version/" > $dpkg_build_dir/DEBIAN/control

echo "This is the content of the archive to be built :"
tree $dpkg_build_dir
dpkg-deb -b $dpkg_build_dir $deb_file

rm -rf $dpkg_build_dir
