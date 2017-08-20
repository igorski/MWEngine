/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2017 Igor Zinken - http://www.igorski.nl
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
#ifndef __GLOBAL_H_INCLUDED__
#define __GLOBAL_H_INCLUDED__

#include <cmath>

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

//#define RECORD_TO_DISK

// if you don't wish to get logcat messages uncomment DEBUG (e.g. release build)
#define DEBUG
#define LOGTAG "MWENGINE" // the logtag used when logging messages to logcat

// if you wish to use the engine without JNI support (for C++ only usage), comment the USE_JNI definition
#define USE_JNI

namespace AudioEngineProps
{
    const int OUTPUT_CHANNELS  = 1;     // valid options are 1 (mono) and 2 (stereo)
    const bool CHANNEL_CACHING = false; // whether to cache AudioChannels and their FX module output in their ProcessingChain

    extern int SAMPLE_RATE;             // initialized on engine start == device specific
    extern int BUFFER_SIZE;             // initialized on engine start == device specific

#ifdef RECORD_DEVICE_INPUT
    const int INPUT_CHANNELS   = 1;
#else
    const int INPUT_CHANNELS   = 0;
#endif
}

// E.O. audio engine configuration

#if PRECISION == 1 // float
 #define SAMPLE_TYPE float
 #define MAX_PHASE 1.0f
#endif

#if PRECISION == 2 // double
 #define SAMPLE_TYPE double
 #define MAX_PHASE 1.0
#endif

// global constants used throughout the engine
const SAMPLE_TYPE PI      = atan( 1 ) * 4;
const SAMPLE_TYPE TWO_PI  = PI * 2.0;
const SAMPLE_TYPE HALF_PI = PI / 2.0;
const int WAVE_TABLE_PRECISION = 128; // the amount of samples in a wave table

extern void *print_message( void* );

#endif
