/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2014 Igor Zinken - http://www.igorski.nl
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
#ifndef GLOBAL_H
#define GLOBAL_H

#include <cmath>

// audio engine configuration

// PRECISION defines the floating-point precision used to synthesize the audio samples
// valid values are 1 (32-bit float) and 2 (64-bit double)
// if you wish to use the engine without JNI support (for C++ only usage), comment the USE_JNI definition

#define PRECISION 2
#define USE_JNI

namespace AudioEngineProps
{
    const int INPUT_CHANNELS   = 1;
    const int OUTPUT_CHANNELS  = 1;      // min 1 (mono)
    const bool CHANNEL_CACHING = false;  // whether to cache AudioChannels and their compatible modules in their ProcessingChain
    const bool EVENT_CACHING   = false;  // whether to cache AudioBuffers within non-sample based AudioEvents (consumes memory!)

    extern int SAMPLE_RATE;            // initialized on engine start == device specific
    extern int BUFFER_SIZE;            // initialized on engine start == device specific
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
const SAMPLE_TYPE PI     = atan( 1 ) * 4;
const SAMPLE_TYPE TWO_PI = PI * 2.0;

extern void *print_message( void* );

// enumerations

class WaveForms
{
    public:
        enum WaveFormTypes { SINE,
                             TRIANGLE,
                             SAWTOOTH,
                             SQUARE,
                             NOISE,
                             PWM,
                             KARPLUS_STRONG };
};

class PercussionTypes
{
    public:
        enum Types { KICK_808,
                     STICK,
                     SNARE,
                     HI_HAT };
};

class DrumSynthTimbres
{
    public:
        enum Timbres { LIGHT,
                       GRAVEL };
};

#endif
