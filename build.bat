%ECHO OFF

REM the input folders for the C++ sources
SET INPUT="-I.\src\main\cpp"

rm -rf src\main\java\nl\igorski\lib\audio\mwengine
mkdir src\main\java\nl\igorski\lib\audio\mwengine

swig -java -package nl.igorski.lib.audio.mwengine -DNO_RTIO -c++ -lcarrays.i -verbose -outdir src\main\java\nl\igorski\lib\audio\mwengine %INPUT% -o src\main\cpp\jni\java_interface_wrap.cpp src\main\cpp\mwengine.i

ndk-build V=1
