/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2018 Igor Zinken - http://www.igorski.nl
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef __MWENGINE__GLOBAL_H_INCLUDED__
#define __MWENGINE__GLOBAL_H_INCLUDED__

#include <cmath>
#include <limits.h>
#include <stdint.h>

namespace MWEngine {

// DRIVER defines which driver to use
// valid options are 0 (OpenSL, works from Android 4.1 up) and 1 (AAudio, Android 8 up)

#define DRIVER 0

// PRECISION defines the floating-point precision used to synthesize the audio samples
// valid options are 1 (32-bit float) and 2 (64-bit double)

#define PRECISION 2

// if you wish to record audio from the Android device input, uncomment the RECORD_DEVICE_INPUT definition
// (note this requires both android.permission.RECORD_AUDIO and android.permission.MODIFY_AUDIO_SETTINGS with a
// positive value for AudioEngineProps::INPUT_CHANNELS)

//#define RECORD_DEVICE_INPUT

// if you wish to write the engine output to the devices file system, uncomment the ALLOW_WRITING definition
// (note this requires android.permission.WRITE_EXTERNAL_STORAGE), like RECORD_AUDIO this requires a Runtime Permission
// grant when compiling for target SDK level 23 (Android M)

#define RECORD_TO_DISK

// uncomment to use debugging (e.g. logging to disk/logcat, etc.)
#define DEBUG
#define LOGTAG "MWENGINE" // the logtag used when logging messages to logcat

// if you wish to use the engine without JNI support (e.g. using solely C++/NDK), comment the USE_JNI definition
#define USE_JNI

namespace AudioEngineProps
{
    const bool CHANNEL_CACHING = false; // whether to cache AudioChannels and their FX module output in their ProcessingChain

    extern int SAMPLE_RATE;     // initialized on engine start == device specific
    extern int BUFFER_SIZE;     // initialized on engine start == device specific
    extern int OUTPUT_CHANNELS; // initialized on engine start, valid options are 1 (mono) and 2 (stereo)

#ifdef RECORD_DEVICE_INPUT
    const int INPUT_CHANNELS = 1;
#else
    const int INPUT_CHANNELS = 0;
#endif
}

// E.O. audio engine configuration
// --------------------------------------
// DO NOT CHANGE ANYTHING BELOW THIS LINE
// ...unless you know what you are doing of course.
//
// these define constants used throughout the engine
// --------------------------------------

#if PRECISION == 1 // float
    #define SAMPLE_TYPE float
#endif

#if PRECISION == 2 // double
    #define SAMPLE_TYPE double
#endif

// math

const SAMPLE_TYPE PI     = atan( 1 ) * 4;
const SAMPLE_TYPE TWO_PI = PI * 2.0;

// other

const int WAVE_TABLE_PRECISION = 128; // the amount of samples contained within a wave table
extern void *print_message( void* );

} // E.O namespace MWEngine

// data types
// here we solve a problem that occurs when reading bytes from
// external files (e.g. loading audio files) where different
// CPU architectures have different sizes for each of the types
// we define integer types by their size in bits in the places where it matters

#if INT_MAX == INT16_MAX
    // 16-bit Android architectures ? WTF...
    #define INT16 int
    #define INT32 long
    #define INT64 long long
    #define UINT16 unsigned int
    #define UINT32 unsigned long
    #define UINT64 unsigned long long
#elseif INT_MAX == INT64_MAX
    // not the case for ARMv8/ARM64 but let's be safe
    #define INT16 short
    #define INT32 float
    #define INT64 int
    #define UINT16 unsigned short
    #define UINT32 unsigned float
    #define UINT64 unsigned int
#else
    // expected across most Android CPU architectures
    #define INT16 short
    #define INT32 int
    #define INT64 long
    #define UINT16 unsigned short
    #define UINT32 unsigned int
    #define UINT64 unsigned long
#endif

#endif // E.O. __MWENGINE__GLOBAL_H_INCLUDED__
