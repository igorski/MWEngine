# The MIT License (MIT)
#
# Copyright (c) 2013-2020 Igor Zinken - https://www.igorski.nl
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

# architecture-specific compiler flags

if (${ANDROID_ABI} MATCHES "x86_64")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=x86-64 -msse4.2 -mpopcnt -m64 -mtune=intel")
endif()

# source folders

set(CPP_SRC  ${Project_SOURCE_DIR}/src/main/cpp)
set(JAVA_SRC ${Project_SOURCE_DIR}/src/main/java)

# Include location of the header files
include_directories(${CPP_SRC}/)

# core engine

set(MWENGINE_CORE_SOURCES ${CPP_SRC}/global.cpp
                          ${CPP_SRC}/audioengine.cpp
                          ${CPP_SRC}/audiobuffer.cpp
                          ${CPP_SRC}/audiochannel.cpp
                          ${CPP_SRC}/channelgroup.cpp
                          ${CPP_SRC}/processingchain.cpp
                          ${CPP_SRC}/ringbuffer.cpp
                          ${CPP_SRC}/sequencer.cpp
                          ${CPP_SRC}/sequencercontroller.cpp
                          ${CPP_SRC}/wavetable.cpp
                          ${CPP_SRC}/definitions/libraries.cpp
                          ${CPP_SRC}/drivers/adapter.cpp
                          ${CPP_SRC}/drivers/aaudio_io.cpp
                          ${CPP_SRC}/drivers/opensl_io.c
                          ${CPP_SRC}/events/baseaudioevent.cpp
                          ${CPP_SRC}/events/basecacheableaudioevent.cpp
                          ${CPP_SRC}/events/sampleevent.cpp
                          ${CPP_SRC}/generators/envelopegenerator.cpp
                          ${CPP_SRC}/generators/wavegenerator.cpp
                          ${CPP_SRC}/instruments/baseinstrument.cpp
                          ${CPP_SRC}/instruments/sampledinstrument.cpp
                          ${CPP_SRC}/messaging/notifier.cpp
                          ${CPP_SRC}/messaging/observer.cpp
                          ${CPP_SRC}/modules/envelopefollower.cpp
                          ${CPP_SRC}/modules/lfo.cpp
                          ${CPP_SRC}/modules/routeableoscillator.cpp
                          ${CPP_SRC}/processors/baseprocessor.cpp
                          ${CPP_SRC}/services/library_loader.cpp
                          ${CPP_SRC}/utilities/bufferutility.cpp
                          ${CPP_SRC}/utilities/levelutility.cpp
                          ${CPP_SRC}/utilities/bulkcacher.cpp
                          ${CPP_SRC}/utilities/diskwriter.cpp
                          ${CPP_SRC}/utilities/debug.cpp
                          ${CPP_SRC}/utilities/samplemanager.cpp
                          ${CPP_SRC}/utilities/bufferpool.cpp
                          ${CPP_SRC}/utilities/tablepool.cpp
                          ${CPP_SRC}/utilities/fastmath.cpp
                          ${CPP_SRC}/utilities/wavereader.cpp
                          ${CPP_SRC}/utilities/wavewriter.cpp
                          ${CPP_SRC}/utilities/utils.cpp)

# Mikrowave specific sources (drum machine specific, you can also create drum machines using sampledinstrument)

set(MWENGINE_MIKROWAVE_SOURCES ${CPP_SRC}/drumpattern.cpp
                               ${CPP_SRC}/events/drumevent.cpp
                               ${CPP_SRC}/instruments/druminstrument.cpp)

# synthesis (can be omitted if your use case only concerns sample based playback)

set(MWENGINE_SYNTH_SOURCES ${CPP_SRC}/definitions/pitch.cpp
                           ${CPP_SRC}/events/basesynthevent.cpp
                           ${CPP_SRC}/events/synthevent.cpp
                           ${CPP_SRC}/modules/adsr.cpp
                           ${CPP_SRC}/modules/arpeggiator.cpp
                           ${CPP_SRC}/instruments/oscillatorproperties.cpp
                           ${CPP_SRC}/instruments/synthinstrument.cpp
                           ${CPP_SRC}/generators/synthesizer.cpp)

# effects processors (can be omitted if your use case only concerns raw audio)

set(MWENGINE_PROCESSORS ${CPP_SRC}/processors/bitcrusher.cpp
                        ${CPP_SRC}/processors/dcoffsetfilter.cpp
                        ${CPP_SRC}/processors/decimator.cpp
                        ${CPP_SRC}/processors/delay.cpp
                        ${CPP_SRC}/processors/filter.cpp
                        ${CPP_SRC}/processors/flanger.cpp
                        ${CPP_SRC}/processors/fm.cpp
                        ${CPP_SRC}/processors/formantfilter.cpp
                        ${CPP_SRC}/processors/glitcher.cpp
                        ${CPP_SRC}/processors/limiter.cpp
                        ${CPP_SRC}/processors/lowpassfilter.cpp
                        ${CPP_SRC}/processors/lpfhpfilter.cpp
                        ${CPP_SRC}/processors/phaser.cpp
                        ${CPP_SRC}/processors/pitchshifter.cpp
                        ${CPP_SRC}/processors/reverb.cpp
                        ${CPP_SRC}/processors/reverbsm.cpp
                        ${CPP_SRC}/processors/tremolo.cpp
                        ${CPP_SRC}/processors/waveshaper.cpp)

# when using the library solely from C++ you can omit bundling JNI_SOURCES (or SWIG wrapping)

set(JNI_SOURCES ${CPP_SRC}/jni/javabridge.cpp
                ${CPP_SRC}/jni/javautilities.cpp)

# build the mock driver into the library when unit test execution is enabled

if (${RUN_TESTS} MATCHES "true")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D=MOCK_ENGINE")
    set(MWENGINE_CORE_SOURCES ${MWENGINE_CORE_SOURCES}
                              ${CPP_SRC}/drivers/mock_io.cpp)
endif()

# include the AAudio library into the MWEngine compilation when configured

if (${INCLUDE_AAUDIO_LIBRARY} MATCHES "true")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D=INCLUDE_AAUDIO_LIBRARY")
endif()

add_library(mwengine STATIC
                     ${MWENGINE_CORE_SOURCES}
                     ${MWENGINE_SYNTH_SOURCES}
                     ${MWENGINE_PROCESSORS}
                     ${MWENGINE_MIKROWAVE_SOURCES}
                     ${JNI_SOURCES})

# link all the NDK libraries used by the project

target_link_libraries(mwengine android atomic log OpenSLES)

# if AAudio library inclusion is configured in the build, link it here
# (otherwise AAudio will be resolved by loading libaaudio.so at runtime)

if (${INCLUDE_AAUDIO_LIBRARY} MATCHES "true")
    target_link_libraries(mwengine PRIVATE aaudio)
endif()

#####################################
## SWIG JNI to Java class wrapping ##
#####################################

find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

set_property(SOURCE ${CPP_SRC}/mwengine.i PROPERTY CPLUSPLUS ON)

set(CMAKE_SWIG_FLAGS -DNO_RTIO -c++ -lcarrays.i -package nl.igorski.mwengine.core)
set(CMAKE_SWIG_OUTDIR ${JAVA_SRC}/nl/igorski/mwengine/core)

# Export a wrapper file to Java, and link with the created C++ library

swig_add_module(mwengine_wrapped java ${CPP_SRC}/mwengine.i)
swig_link_libraries(mwengine_wrapped mwengine)

################
## UNIT TESTS ##
################

if (${RUN_TESTS} MATCHES "true")

    # only run for a single architecture, assume x86_64 TODO: specify via local.properties?

    if (${ANDROID_ABI} MATCHES "x86_64")

        set(GTEST_DIR ${ANDROID_NDK}/sources/third_party/googletest)

        add_library(gtest STATIC ${GTEST_DIR}/src/gtest_main.cc ${GTEST_DIR}/src/gtest-all.cc)
        target_include_directories(gtest PRIVATE ${GTEST_DIR})
        target_include_directories(gtest PUBLIC ${GTEST_DIR}/include)

        add_executable(mwengine_unittest ${CPP_SRC}/tests/main.cpp)
        target_link_libraries(mwengine_unittest mwengine gtest)

        # Push and execute tests as post-build event

        set(TARGET_TEST_DIR /data/local/tmp/mwengine_unittest_${ANDROID_ABI}) # Directory on device to push tests.

        #    find_program(ADB NAMES adb PATHS ${ANDROID_SDK_ROOT}/platform-tools)

        add_custom_command(
            TARGET mwengine_unittest POST_BUILD
            COMMAND adb shell mkdir -p ${TARGET_TEST_DIR}
            COMMAND adb push $<TARGET_FILE:mwengine> ${TARGET_TEST_DIR}/
            COMMAND adb push $<TARGET_FILE:mwengine_unittest> ${TARGET_TEST_DIR}/
            COMMAND adb shell \"export LD_LIBRARY_PATH=${TARGET_TEST_DIR}\; ${TARGET_TEST_DIR}/mwengine_unittest\"
        )

    endif()

endif()