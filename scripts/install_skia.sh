#!/bin/bash

# Set the PKG directory where everything will be installed
export PKG=$(pwd)/install

# Ensure the PKG directory exists
mkdir -pv $PKG

git clone 'https://chromium.googlesource.com/chromium/tools/depot_tools.git'
export PATH="${PWD}/depot_tools:${PATH}"

# Download the packages
wget https://github.com/google/skia/archive/refs/tags/canvaskit/0.34.0.tar.gz

tar -xvf 0.34.0.tar.gz

cd skia-canvaskit-0.34.0
python3 tools/git-sync-deps

# Run configure with the adjusted paths
bin/gn gen out/Shared --args='is_official_build=true is_component_build=true target_os="linux" target_cpu="x64" skia_compile_modules=true skia_enable_tools=true skia_enable_pdf=true skia_enable_sksl=true skia_enable_skshaper=true skia_enable_svg=true skia_enable_gpu=true skia_use_gl=true skia_use_angle=false skia_use_vulkan=false skia_use_dawn=false skia_use_metal=false skia_use_direct3d=false skia_use_expat=true skia_use_harfbuzz=true skia_use_icu=true skia_use_zlib=true skia_use_freetype=true skia_use_ffmpeg=false skia_use_sfml=false skia_use_sfntly=true skia_use_system_expat=false skia_use_system_harfbuzz=false skia_use_system_icu=false skia_use_system_libjpeg_turbo=false skia_use_system_libpng=false skia_use_system_libwebp=false skia_use_system_zlib=false skia_use_system_freetype2=false skia_enable_fontmgr_custom_directory=false text_tests_enabled=false skia_compile_sksl_tests=false'

ninja -C out/Shared

#cd $PKG
#mkdir -p usr/lib64
#mkdir -p usr/include/skia

#cd $PKG/usr/lib64
#cp ../../../skia-canvaskit-0.34.0/out/Shared/*.a .
#cp ../../../skia-canvaskit-0.34.0/out/Shared/*.so .

#cd $PKG/usr/include/skia
#cp -r ../../../../skia-canvaskit-0.34.0/include/* .

cd $PKG