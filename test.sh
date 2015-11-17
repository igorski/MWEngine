#!/bin/sh

echo ---------------------------------------
echo Building MWEngine \(might take a moment\)
echo ---------------------------------------

( exec "./build.sh" ) > /dev/null
BUILD_SUCCESS=$?

if [ $BUILD_SUCCESS -eq 0 ]; then

    echo ---------------------------
    echo Running MWEngine unit tests
    echo ---------------------------

    adb push libs/armeabi/libmwengine.so /data/local/tmp/
    adb push libs/armeabi/mwengine_unittest /data/local/tmp/
    adb shell chmod 775 /data/local/tmp/mwengine_unittest

    # run all the unit tests once

#    adb shell "LD_LIBRARY_PATH=/data/local/tmp /data/local/tmp/mwengine_unittest"

    # ALTERNATIVE: run all the unit tests with a defined amount of repeats (useful for debugging seemingly random failures)

    adb shell "LD_LIBRARY_PATH=/data/local/tmp /data/local/tmp/mwengine_unittest --gtest_repeat=100"

fi
