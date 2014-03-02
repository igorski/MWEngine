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
#include "global.h"
#include <pthread.h>

// defaults (overridden by audio engine initializer for platform-specific values)

int audio_engine::SAMPLE_RATE = 44100;
int audio_engine::BUFFER_SIZE = 1024;

// global sequencer variables

int bytes_per_beat;
int bytes_per_bar;
int bytes_per_tick;

int amount_of_bars      = 1;
int beat_subdivision    = 4;
int min_buffer_position = 0; // initially 0, but can differ when looping specific measures
int max_buffer_position = 0; // calculated when sequencer API creates output
int min_step_position   = 0;
int max_step_position   = 16;

bool playing          = false;
bool recordOutput     = false;
bool haltRecording    = false;
bool bouncing         = false;
bool recordFromDevice = false;
bool monitorRecording = false; // might introduce feedback on microphone ;)
int recordingFileName = 0;

/* buffer read/write pointers */

int bufferPosition = 0;
int stepPosition   = 0;

/* tempo related */

float tempo                   = 90.0;
float queuedTempo             = 120.0;
int time_sig_beat_amount       = 4;
int time_sig_beat_unit         = 4;
int queuedTime_sig_beat_amount = time_sig_beat_amount;
int queuedTime_sig_beat_unit   = time_sig_beat_unit;

/* audio output related */

float volume = .85;

/* used for threading */

void *print_message( void* )
{

}
