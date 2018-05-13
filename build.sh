#!/bin/sh

# custom to your machine : Android NDK library location
export ANDROID_NDK_ROOT=/Library/AndroidSDK/ndk-bundle

# the input folders for the C++ sources
export APP_BUILD_SCRIPT=./src/main/cpp/Android.mk
export NDK_PROJECT_PATH=./src/main/cpp
INPUT="-I./src/main/cpp"

rm -rf src/main/java/nl/igorski/lib/audio/mwengine
mkdir -p src/main/java/nl/igorski/lib/audio/mwengine

swig -java -package nl.igorski.lib.audio.mwengine -DNO_RTIO -c++ -lcarrays.i -verbose -outdir src/main/java/nl/igorski/lib/audio/mwengine -I/usr/local/include -I/System/Library/Frameworks/JavaVM.framework/Headers $INPUT -o src/main/cpp/jni/java_interface_wrap.cpp src/main/cpp/mwengine.i

$ANDROID_NDK_ROOT/ndk-build V=1 $1
