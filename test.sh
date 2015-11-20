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

    # if a numerical argument was provided treat is as an amount of test repeats
    # breaking on the first failure (useful for debugging seemingly random failures)

    if echo $1 | egrep -q '^\-?[0-9]+$'; then
        adb shell "LD_LIBRARY_PATH=/data/local/tmp /data/local/tmp/mwengine_unittest --gtest_repeat=$1 --gtest_break_on_failure"

    # run all the unit tests once

    else
        adb shell "LD_LIBRARY_PATH=/data/local/tmp /data/local/tmp/mwengine_unittest"
    fi

fi
