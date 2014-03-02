#ifndef GLOBAL_H
#define GLOBAL_H

#include "finalizer.h"
#include "lpfhpfilter.h"
#include "sequencer.h"

// audio engine variables

const float MAX_PHASE = 1.0;

namespace audio_engine
{
    const int INPUT_CHANNELS        = 0;
    const int OUTPUT_CHANNELS       = 1;    // currently mono output
    const int SAMPLES_PER_CHANNEL   = 64;
    const int BYTES_PER_SAMPLE      = 8;
    const int AMOUNT_OF_VOICES      = 1;    // in case we feel adventurous and extend this...

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
extern bool recording;
extern bool haltRecording;
extern bool bouncing;
extern int recordingIterator;
extern int recordingMaxIterations;
extern int recordingFileName;

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
