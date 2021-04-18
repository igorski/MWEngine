/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2014-2018 Igor Zinken - http://www.igorski.nl
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
#ifndef __MWENGINE__ADSR_H_INCLUDED__
#define __MWENGINE__ADSR_H_INCLUDED__

#include "audiobuffer.h"
#include "global.h"
#include <events/basesynthevent.h>

/**
 * ADSR (Attack, Decay, Sustain and Release) provides amplitude
 * envelopes to a BaseSynthEvents signal
 *
 * Attack time is the time taken for initial run-up of level from nil to peak, beginning when the key is first pressed.
 * Decay time is the time taken for the subsequent run down from the attack level to the designated sustain level.
 * Sustain level is the level during the main sequence of the sound's duration, until the key is released.
 * Release time is the time taken for the level to decay from the sustain level to zero after the key is released.
 * All time values are in seconds, sustain level is in 0 - 1 range
 *
 * TODO:
 * it might be nice to have non-linear envelope functions too !
 */
namespace MWEngine {
class ADSR
{
    public:
    
        ADSR();
        ADSR( float attackTime, float decayTime, float sustainLevel, float releaseTime );
        ~ADSR();

        ADSR* clone();
        void cloneEnvelopes( ADSR* source );
        
        float getAttackTime();
        float getDecayTime();
        float getSustainLevel();
        float getReleaseTime();

        void setAttackTime  ( float aValue );
        void setDecayTime   ( float aValue );
        void setSustainLevel( float aValue );
        void setReleaseTime ( float aValue );

        // get release offset and duration in buffer samples

        int getReleaseStartOffset();
        int getReleaseDuration();

        /**
         * applies all envelopes onto given AudioBuffer* inputBuffer
         *
         * in case the given buffer represents a smaller range of
         * a larger buffer (the full AudioEvent length for instance), int writeOffset
         * describes the current offset (in samples) relative to to the events duration
         * at which the effect should start its application, the application lasts for
         * the bufferSize of given inputBuffer in length
         */
        void apply( AudioBuffer* inputBuffer, BaseSynthEvent* synthEvent, int writeOffset );

        // set envelope durations (in buffer samples) directly
        // this is more useful for unit testing rather than direct use
        void setDurations( int attackDuration, int decayDuration, int releaseDuration, int bufferLength );

    protected:

        float _attackTime;
        float _decayTime;
        float _sustainLevel;
        float _releaseTime;

        // cached variables for incrementing the envelope
        // durations describe the envelope duration in samples
        // the Increment is the value to increment the envelope
        // by for each written sample

        SAMPLE_TYPE _attackIncrement;
        int _attackDuration;

        SAMPLE_TYPE _decayDecrement;
        SAMPLE_TYPE _lastDecayEnvelope;
        int _decayDuration;

        int _sustainDuration;

        SAMPLE_TYPE _releaseDecrement;
        int _releaseDuration;

        // attack starts at the beginning of the sound (thus at start
        // AudioBuffer, DSR envelopes have variable start offsets...)
        // these are in buffer samples relative to the events buffer duration

        int _decayStart;
        int _sustainStart;
        int _releaseStart;

        // used by apply method and are derived from the processed BaseSynthEvent

        int _bufferLength;

        // recalculates the increment values for all envelopes

        void invalidateEnvelopes();
        void setEnvelopesInternal( float attackTime, float decayTime, float sustainLevel, float releaseTime );
        void construct();
};
} // E.O namespace MWEngine

#endif
