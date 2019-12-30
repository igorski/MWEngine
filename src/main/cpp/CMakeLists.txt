# The MIT License (MIT)
#
# Copyright (c) 2013-2019 Igor Zinken - https://www.igorski.nl
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the "Software"), to deal in
# the Software without restriction, including without limitation the rights to
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
# the Software, and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
# IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
cmake_minimum_required(VERSION 3.4.1)

# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Include a location to the header files
include_directories(.)

# core engine

set(MWENGINE_CORE_SOURCES global.cpp
                          audioengine.cpp
                          audiobuffer.cpp
                          audiochannel.cpp
                          processingchain.cpp
                          ringbuffer.cpp
                          sequencer.cpp
                          sequencercontroller.cpp
                          wavetable.cpp
                          drumpattern.cpp
                          drivers/adapter.cpp
                          drivers/opensl_io.c
                          drivers/aaudio_io.cpp
                          events/baseaudioevent.cpp
                          events/basecacheableaudioevent.cpp
                          events/drumevent.cpp
                          events/sampleevent.cpp
                          generators/envelopegenerator.cpp
                          generators/wavegenerator.cpp
                          instruments/baseinstrument.cpp
                          instruments/druminstrument.cpp
                          instruments/sampledinstrument.cpp
                          messaging/notifier.cpp
                          messaging/observer.cpp
                          modules/envelopefollower.cpp
                          modules/lfo.cpp
                          modules/routeableoscillator.cpp
                          processors/baseprocessor.cpp
                          utilities/bufferutility.cpp
                          utilities/levelutility.cpp
                          utilities/bulkcacher.cpp
                          utilities/diskwriter.cpp
                          utilities/debug.cpp
                          utilities/samplemanager.cpp
                          utilities/bufferpool.cpp
                          utilities/tablepool.cpp
                          utilities/fastmath.cpp
                          utilities/wavereader.cpp
                          utilities/wavewriter.cpp
                          utilities/utils.cpp)

# synthesis (can be omitted if your use case only concerns sample based playback)

set(MWENGINE_SYNTH_SOURCES events/basesynthevent.cpp
                           events/synthevent.cpp
                           modules/adsr.cpp
                           modules/arpeggiator.cpp
                           instruments/oscillatorproperties.cpp
                           instruments/synthinstrument.cpp
                           generators/synthesizer.cpp)

# effects processors (can be omitted if your use case only concerns raw audio)

set(MWENGINE_PROCESSORS processors/bitcrusher.cpp
                        processors/dcoffsetfilter.cpp
                        processors/decimator.cpp
                        processors/delay.cpp
                        processors/filter.cpp
                        processors/flanger.cpp
                        processors/fm.cpp
                        processors/formantfilter.cpp
                        processors/glitcher.cpp
                        processors/limiter.cpp
                        processors/lowpassfilter.cpp
                        processors/lpfhpfilter.cpp
                        processors/phaser.cpp
                        processors/pitchshifter.cpp
                        processors/reverb.cpp
                        processors/reverbsm.cpp
                        processors/tremolo.cpp
                        processors/waveshaper.cpp)

# when using the library solely from C++ you can omit bundling JNI_SOURCES (or SWIG wrapping)

set(JNI_SOURCES jni/javabridge.cpp
                jni/javautilities.cpp)

add_library(mwengine SHARED
            ${MWENGINE_CORE_SOURCES}
            ${MWENGINE_SYNTH_SOURCES}
            ${MWENGINE_PROCESSORS}
            ${JNI_SOURCES})

# link all the NDK libraries used by the project

target_link_libraries(mwengine android atomic log OpenSLES aaudio)

#####################################
## SWIG JNI to Java class wrapping ##
#####################################

find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

# Ensure file recognized as C++ (otherwise, exported as C file)

set_property(SOURCE mwengine.i PROPERTY CPLUSPLUS ON)

# Setup SWIG flags and locations

set(CMAKE_SWIG_FLAGS -DNO_RTIO -c++ -lcarrays.i -package nl.igorski.mwengine.core)
set(CMAKE_SWIG_OUTDIR ${Project_SOURCE_DIR}/swig_wrapper)

# Export a wrapper file to Java, and link with the created C++ library

swig_add_module(mwengine_wrapper java mwengine.i)
swig_link_libraries(mwengine_wrapper mwengine)

# Take all generated .java files (ignoring the temporary cxx wrappers) and move to Java source folder

file(GLOB JAVA_FILES ${Project_SOURCE_DIR}/swig_wrapper/*.java)
if (${JAVA_FILES})
    file(COPY ${JAVA_FILES}
        DESTINATION ${Project_SOURCE_DIR}/../java/nl/igorski/mwengine/core)
    file(remove ${JAVA_FILES})
endif()
file(remove ${CMAKE_SWIG_OUTDIR})

################
## UNIT TESTS ##
################

#set(GOOGLETEST_ROOT ${ANDROID_NDK}/sources/third_party/googletest/googletest)
#add_library(gtest STATIC ${GOOGLETEST_ROOT}/src/gtest_main.cc ${GOOGLETEST_ROOT}/src/gtest-all.cc)
#target_include_directories(gtest PRIVATE ${GOOGLETEST_ROOT})
#target_include_directories(gtest PUBLIC ${GOOGLETEST_ROOT}/include)
#
#add_executable(mwengine_unittest tests/main.cpp)
#target_link_libraries(mwengine_unittest gtest)

# Automatically launch the unit tests onto an attached device

#find_program(ADB adb)
#add_custom_command(TARGET mwengine_unittest POST_BUILD
#        COMMAND ${ADB} shell mkdir -p /data/local/tmp/${ANDROID_ABI}
#        COMMAND ${ADB} push $<TARGET_FILE:native-lib> /data/local/tmp/${ANDROID_ABI}/
#        COMMAND ${ADB} push $<TARGET_FILE:footest> /data/local/tmp/${ANDROID_ABI}/
#        COMMAND ${ADB} shell \"export LD_LIBRARY_PATH=/data/local/tmp/${ANDROID_ABI}\; /data/local/tmp/${ANDROID_ABI}/mwengine_unittest\")