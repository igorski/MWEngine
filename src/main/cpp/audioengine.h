/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2020 Igor Zinken - https://www.igorski.nl
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
#ifndef __MWENGINE__AUDIOENGINE_H_INCLUDED__
#define __MWENGINE__AUDIOENGINE_H_INCLUDED__

#include "audiobuffer.h"
#include "audiochannel.h"
#include "global.h"
#include "processingchain.h"
#include "channelgroup.h"
#include <definitions/drivers.h>

namespace MWEngine {
class AudioEngine
{
    /**
     * this is the API exposed to an external application
     * for interacting with the AudioEngine (it does nothing
     * more than prepare and start / stop / reset the render thread
     */

    /* public API */

    public:

        static void setup( unsigned int bufferSize, unsigned int sampleRate, unsigned int amountOfChannels );
        static void start( Drivers::types audioDriver );
        static void stop();
        static void reset();

        static AudioChannel* getInputChannel();

        // renders the audio. this should not be called directly (is called
        // by the audio drivers). Use start() instead (triggers driver activity)

        static bool render( int amountOfSamples );

        /* engine properties */

        static int samples_per_beat;      // the amount of samples necessary for a single beat at the current tempo and sample rate
        static int samples_per_bar;       // the amount of samples for a full bar at the current tempo and sample rate
        static int samples_per_step;       // the amount of samples within a single status update subdivision

        static int amount_of_bars;         // the amount of measures in the current sequencer
        static int steps_per_bar;          // the amount of subdivisions in a single measure the engine broadcast a status update for
        static int min_buffer_position;    // the lowest sample offset in the current loop range
        static int max_buffer_position;    // the maximum sample offset in the current loop range
        static int marked_buffer_position; // the buffer position that should launch a notification when playback exceeds this position
        static int min_step_position;      // the lowest step in the current sequence
        static int max_step_position;      // the maximum step in the current sequence (e.g. 15 for single measure using a 16 step sequencer - step starts at 0.)
        static bool recordOutputToDisk;    // whether to record rendered output
        static bool bouncing;              // whether bouncing audio (i.e. rendering in inaudible offline mode without thread lock)
        static bool recordInputToDisk;     // whether to record audio from the Android device input to disk
        static bool recordDeviceInput;     // whether audio from the Android device input should be audible

        /* buffer read/write pointers */

        static int bufferPosition;      // the current sequence position in samples ("playback head" offset)
        static int stepPosition;        // the current sequence bar subdivided position (e.g. 16th note of a bar)
        static int bounceRangeStart;    // when bouncing, this defines the starting point of the bounce range
        static int bounceRangeEnd;      // when bouncing, this defines the end point of the bounce range

        /* tempo related */

        static float tempo;                     // the tempo of the sequencer
        static float queuedTempo;               // the tempo the sequencer will move to once current render cycle completes
        static int time_sig_beat_amount;        // time signature upper numeral (i.e. the "3" in 3/4)
        static int time_sig_beat_unit;          // time signature lower numeral (i.e. the "4" in 3/4)
        static int queuedTime_sig_beat_amount;  // the time signature beat amount the sequencer moves to on next cycle
        static int queuedTime_sig_beat_unit;    // the time signature beat unit the sequencer moves to on next cycle

        /* output related */

        static float volume;                // master volume
        static ProcessingChain* masterBus;  // processing chain for the master bus

        static void addChannelGroup( ChannelGroup* group );
        static void removeChannelGroup( ChannelGroup* group );

        /* internal methods */

        static void handleTempoUpdate( float aQueuedTempo, bool broadcastUpdate );

    private:

        /* render properties */

        static bool loopStarted; // whether the current buffer will exceed the end offset of the loop (read remaining samples from the start)
        static int  loopOffset;   // the offset within the current buffer where we exceed max_buf_pos and start reading from min_buf_pos
        static int  loopAmount;   // amount of samples we must read from the current loop ranges start offset (== min_buffer_position)
        static int  outputChannels;
        static int  thread;
        static bool isMono;
        static std::vector<AudioChannel*>* channels;
        static std::vector<ChannelGroup*> groups;
        static AudioBuffer* inBuffer;
        static float*       outBuffer;

#ifdef PREVENT_CPU_FREQUENCY_SCALING

        /* time */

        static double  mOpsPerNano;
        static int64_t mFrameCount;
        static int64_t mEpochTimeNanos;

#if defined(__i386__) || defined(__x86_64__)
#define noop() asm volatile("rep; nop" ::: "memory");
#elif defined(__arm__) || defined(__mips__)
#define noop() asm volatile("":::"memory");
#elif defined(__aarch64__)
#define noop() asm volatile("yield" ::: "memory");
#else
#error "noop cannot be defined for this architecture"
#endif

        #define MAX_CPU_PER_RENDER_ITERATION 0.8F
#endif

        /* recording */

#ifdef RECORD_DEVICE_INPUT
        static float* recbufferIn;
        static AudioChannel* inputChannel;
#endif

        /* internal render methods */

        static void handleSequencerPositionUpdate( int bufferOffset );
        static bool writeChannelCache            ( AudioChannel* channel, AudioBuffer* channelBuffer, int cacheReadPos );
};
} // E.O namespace MWEngine

#endif
