/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2022 Igor Zinken - https://www.igorski.nl
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
#include <vector>

namespace MWEngine {

// PRECISION defines the floating-point precision used to synthesize the audio samples
// valid options are 1 (32-bit float) and 2 (64-bit double)

#define PRECISION 2

// if you wish to record audio from the Android device input, uncomment the RECORD_DEVICE_INPUT definition
// (note this requires both android.permission.RECORD_AUDIO and android.permission.MODIFY_AUDIO_SETTINGS)

#define RECORD_DEVICE_INPUT

// if you wish to write the engine output to the devices file system, uncomment the RECORD_TO_DISK definition
// (note this requires android.permission.WRITE_EXTERNAL_STORAGE), like RECORD_AUDIO this requires a Runtime Permission
// grant when compiling for target SDK level 23 (Android M)

#define RECORD_TO_DISK

// uncomment to use debugging (e.g. logging to disk/logcat, etc.)
#define DEBUG
#define LOGTAG "MWENGINE" // the logtag used when logging messages to logcat

// if you wish to use the engine without JNI support (e.g. using solely C++/NDK), comment the USE_JNI definition
#define USE_JNI

// Performance improvement suggested at Google I/O 17.
// Prevent CPU frequency scaling by adding a stabilizing load in the callback routine. This load
// essentially keeps invoking a no operation assembly call to keep the CPU busy. By preventing
// scaling the performance should remain consistent throughout. Hopefully this nonsense can
// be removed in a future world of wonderful things.
#define PREVENT_CPU_FREQUENCY_SCALING

namespace AudioEngineProps
{
    const bool CHANNEL_CACHING = false;      // whether to cache AudioChannels and their FX module output in their ProcessingChain

    extern unsigned int     SAMPLE_RATE;     // initialized on engine start == device specific
    extern unsigned int     BUFFER_SIZE;     // initialized on engine start == device specific
    extern unsigned int     OUTPUT_CHANNELS; // initialized on engine start, valid options are 1 (mono) and 2 (stereo)
    extern std::vector<int> CPU_CORES;       // on Android N this can be retrieved from the Activity, see JavaUtilities

#ifdef RECORD_DEVICE_INPUT
    const unsigned int INPUT_CHANNELS = 1;
#else
    const unsigned int INPUT_CHANNELS = 0;
#endif
    extern std::atomic<bool> isRendering;
}

// E.O. audio engine configuration
// --------------------------------------
// DO NOT CHANGE ANYTHING BELOW THIS LINE
// ...unless you know what you are doing of course.
//
// these define constants used throughout the engine
// --------------------------------------

#if PRECISION == 1 // float
    typedef float SAMPLE_TYPE;
    #define SILENCE 0.f
    #define MAX_VOLUME 1.f
    #define undenormalise(sample) ((((*(UINT32 *)&(sample))&0x7f800000)==0)&&((sample)!=0.f))
#endif

#if PRECISION == 2 // double
    typedef double SAMPLE_TYPE;
    #define SILENCE 0.0
    #define MAX_VOLUME 1.0
    #define undenormalise(sample) ((((((UINT32 *)&(sample))[1])&0x7fe00000)==0)&&((sample)!=0.f))
#endif

#define CONV16BIT 32768       // multiplier to convert floating point to signed 16-bit value
#define CONVMYFLT (1./32768.) // multiplier to convert signed 16-bit values to floating point

// maximum volume output of the engine (prevents clipping of extremely hot signals)
#define MAX_OUTPUT 0.98F

// can be used to prevent denormals
// init envelopes to DC_OFFSET prior to processing, add to input before envelope proces
const float DC_OFFSET = 1.0E-25;

// math caches

const SAMPLE_TYPE PI      = atan( 1 ) * 4;
const SAMPLE_TYPE TWO_PI  = PI * 2.0;
const SAMPLE_TYPE HALF_PI = PI / 2.0;

// other

const int WAVE_TABLE_PRECISION = 128; // the amount of samples contained within a wave table

#ifdef PREVENT_CPU_FREQUENCY_SCALING

// noops used to keep the CPU seemingly busy on less intensive render iterations
// this allows us to maintain a consistent thread performance across render iterations (see PerfUtility)

// __i386__ == Intel 32-bit architecture
// __x86_64__ == Intel 64-bit architecture
// __arm__ and __mips__ == both 32-bit architectures
// __aarch64__ == ARM 64-bit

#if defined(__i386__) || defined(__x86_64__)
#define noop() asm volatile("rep; nop" ::: "memory");
#elif defined(__arm__) || defined(__mips__)
#define noop() asm volatile("":::"memory");
#elif defined(__aarch64__)
#define noop() asm volatile("yield" ::: "memory");
#else
#error "no noop assembly instruction can be defined for this architecture"
#endif

// the maximum percentage of CPU usage we deem acceptable for a single render iteration
#define MAX_CPU_PER_RENDER_TIME 0.8F
#endif

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
