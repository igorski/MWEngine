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
int max_step_position   = 16; // initially we assume a sixteen-step sequencer for a single bar

bool playing          = false;
bool recording        = false;
bool haltRecording    = false;
bool bouncing         = false;
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
