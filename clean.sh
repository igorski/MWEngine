#!/bin/sh

# custom to your machine : Android NDK library location
export ANDROID_NDK_ROOT=/Library/AndroidNDK

# the input folders for the C++ sources
export NDK_PROJECT_PATH=./src/main/cpp
INPUT="-I./src/main/cpp"

# flush output folders
rm -rf libs
rm -rf src/main/java/nl/igorski/lib/audio/mwengine

# remove generated files
rm src/main/cpp/jni/java_interface_wrap.cpp

# explicitly provide paths to .mk files as default ndk-build expects files in a folder called ./jni
$ANDROID_NDK_ROOT/ndk-build clean -C $NDK_PROJECT_PATH APP_BUILD_SCRIPT=Android.mk NDK_APPLICATION_MK=Application.mk NDK_LIBS_OUT=../../../libs V=1 $1
