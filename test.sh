#!/bin/sh

echo ---------------------------------------
echo Building MWEngine \(might take a moment\)
echo ---------------------------------------

# custom to your machine : Android NDK library location
export ANDROID_NDK_ROOT=/Library/AndroidNDK

# the input folders for the C++ sources
export NDK_PROJECT_PATH=./src/main/cpp
INPUT="-I./src/main/cpp"

# flush/create output folders
rm -rf libs
mkdir libs

# compile Java interface classes

swig -java -package nl.igorski.lib.audio.mwengine -DNO_RTIO -c++ -lcarrays.i -verbose -outdir src/main/java/nl/igorski/lib/audio/mwengine -I/usr/local/include -I/System/Library/Frameworks/JavaVM.framework/Headers $INPUT -o src/main/cpp/jni/java_interface_wrap.cpp src/main/cpp/mwengine.i

# explicitly provide paths to .mk files as default ndk-build expects files in a folder called ./jni
# additionally, we use a different application.mk when testing
$ANDROID_NDK_ROOT/ndk-build -C $NDK_PROJECT_PATH APP_BUILD_SCRIPT=Android.mk NDK_APPLICATION_MK=Application_test.mk NDK_LIBS_OUT=../../../libs V=1 $1 > /dev/null
BUILD_SUCCESS=$?

if [ $BUILD_SUCCESS -eq 0 ]; then

    echo ---------------------------
    echo Running MWEngine unit tests
    echo ---------------------------

    adb push libs/x86/libmwengine_test.so /data/local/tmp/
    adb push libs/x86/mwengine_unittest /data/local/tmp/
    adb shell chmod 775 /data/local/tmp/mwengine_unittest

    # if a numerical argument was provided treat it as the amount of test repeats
    # breaking on the first failure (useful for debugging seemingly random failures)

    if echo $1 | egrep -q '^\-?[0-9]+$'; then
        adb shell "LD_LIBRARY_PATH=/data/local/tmp /data/local/tmp/mwengine_unittest --gtest_repeat=$1 --gtest_break_on_failure"

    # run all the unit tests once

    else
        adb shell "LD_LIBRARY_PATH=/data/local/tmp /data/local/tmp/mwengine_unittest"
    fi

fi
