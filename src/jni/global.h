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

// audio engine variables

const float MAX_PHASE = 1.0;

namespace audio_engine
{
    const int INPUT_CHANNELS  = 1;
    const int OUTPUT_CHANNELS = 1;
    const bool USE_CACHING    = false; // whether to cache audio channels and their processing chains

    extern int SAMPLE_RATE;         // initialized on engine start == device specific
    extern int BUFFER_SIZE;         // initialized on engine start == device specific
}

// global sequencer variables

extern int bytes_per_beat;
extern int bytes_per_bar;
extern int bytes_per_tick;

extern int amount_of_bars;
extern int beat_subdivision;    // the amount of sub divisions the engine recognises for a beat (for instance a value of 4 equals sixteenth notes in 4/4 time)
extern int min_buffer_position; // initially 0, but can differ when looping specific measures
extern int max_buffer_position;
extern int min_step_position;
extern int max_step_position;
extern bool playing;
extern bool recordOutput;
extern bool haltRecording;
extern bool bouncing;
extern int recordingIterator;
extern int recordingMaxIterations;
extern int recordingFileName;
extern bool recordFromDevice;
extern bool monitorRecording;

/* buffer read/write pointers */

extern int bufferPosition;
extern int stepPosition;
extern int playbackPos;

/* tempo related */

extern float tempo;
extern float queuedTempo;
extern int time_sig_beat_amount;
extern int time_sig_beat_unit;
extern int queuedTime_sig_beat_amount;
extern int queuedTime_sig_beat_unit;

/* audio output related */

extern float volume;

extern void *print_message( void* );

// enumerations

class WaveForms
{
    public:
        enum WaveFormTypes { SINE_WAVE, TRIANGLE, SAWTOOTH, SQUARE_WAVE, NOISE, PWM, KARPLUS_STRONG };
};

class PercussionTypes
{
    public:
        enum Types { KICK_808, STICK, SNARE, HI_HAT };
};

class DrumSynthTimbres
{
    public:
        enum Timbres { LIGHT, GRAVEL };
};

#endif
