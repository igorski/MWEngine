%ECHO OFF

REM the input folders for the C++ sources
SET INPUT="-I.\jni"

rm -rf src\nl\igorski\lib\audio\nativeaudio
mkdir src\nl\igorski\lib\audio\nativeaudio

swig -java -package nl.igorski.lib.audio.nativeaudio -DNO_RTIO -c++ -lcarrays.i -verbose -outdir src\nl\igorski\lib\audio\nativeaudio %INPUT% -o jni\java_interface_wrap_test.cpp jni\mwengine.i

ndk-build TARGET_PLATFORM=android-14 V=1
