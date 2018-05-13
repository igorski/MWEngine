#!/bin/sh

# custom to your machine : Android NDK library location
export ANDROID_NDK_ROOT=/Library/AndroidNDK

# the input folders for the C++ sources
export NDK_PROJECT_PATH=./src/main/cpp
INPUT="-I./src/main/cpp"

# flush/create output folders
rm -rf libs
mkdir libs
rm -rf src/main/java/nl/igorski/lib/audio/mwengine
mkdir -p src/main/java/nl/igorski/lib/audio/mwengine

# compile Java interface classes

swig -java -package nl.igorski.lib.audio.mwengine -DNO_RTIO -c++ -lcarrays.i -verbose -outdir src/main/java/nl/igorski/lib/audio/mwengine -I/usr/local/include -I/System/Library/Frameworks/JavaVM.framework/Headers $INPUT -o src/main/cpp/jni/java_interface_wrap.cpp src/main/cpp/mwengine.i

# explicitly provide paths to .mk files as default ndk-build expects files in a folder called ./jni
$ANDROID_NDK_ROOT/ndk-build -C $NDK_PROJECT_PATH APP_BUILD_SCRIPT=Android.mk NDK_APPLICATION_MK=Application.mk NDK_LIBS_OUT=../../../libs V=1 $1
