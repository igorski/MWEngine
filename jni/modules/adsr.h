/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Igor Zinken - http://www.igorski.nl
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
#ifndef __ADSR_H_INCLUDED__
#define __ADSR_H_INCLUDED__

#include "audiobuffer.h"
#include "global.h"

/**
 * ADSR (Attack, Decay, Sustain and Release) provides amplitude
 * envelopes to a given AudioBuffer signal
 * 
 * Attack time is the time taken for initial run-up of level from nil to peak, beginning when the key is first pressed.
 * Decay time is the time taken for the subsequent run down from the attack level to the designated sustain level.
 * Sustain level is the level during the main sequence of the sound's duration, until the key is released.
 * Release time is the time taken for the level to decay from the sustain level to zero after the key is released.
 *
 * TODO :
 *
 * this is VERY rudimentary and actually unfinished as sustain and release
 * envelopes aren't applied yet (release needs some logic elsewhere as it
 * implies a new/large AudioBuffer to extend the sound tail)
 *
 * it might be nice to have non-linear envelope functions too !
 */
class ADSR
{
    static const int DEFAULT_FADE_DURATION = 8; // in samples
  
    public:
    
        ADSR();
        ~ADSR();

        ADSR* clone();
        void cloneEnvelopes( ADSR* source );

        void setBufferLength( int bufferLength );
        int getBufferLength();
        
        float getAttack();
        float getDecay();
        float getSustain();
        float getRelease();

        void setAttack ( float aValue );
        void setDecay  ( float aValue );
        void setSustain( float aValue );
        void setRelease( float aValue );

        SAMPLE_TYPE getLastEnvelope();
        void setLastEnvelope( SAMPLE_TYPE lastEnvelope );

        /**
         * applies all envelopes onto given AudioBuffer* inputBuffer
         * in case the given buffer represents a smaller range of
         * a larger buffer (of an AudioEvent for instance), int eventOffset
         * describes at what offset of the total buffer/event the given
         * buffer starts
         *
         * returns last envelope value
         */
        SAMPLE_TYPE apply( AudioBuffer* inputBuffer );
        SAMPLE_TYPE apply( AudioBuffer* inputBuffer, int eventOffset );

    protected:

        int _bufferLength;
    
        SAMPLE_TYPE _attack;
        SAMPLE_TYPE _decay;
        SAMPLE_TYPE _sustain;
        SAMPLE_TYPE _release;
        SAMPLE_TYPE _lastEnvelope; // last used amp envelope value

        // cached variables for incrementing the envelope
        // durations describe the envelope duration in samples
        // the Increment is the value to increment the envelope
        // by for each written sample

        SAMPLE_TYPE _attackIncrement;
        int _attackDuration;

        SAMPLE_TYPE _decayIncrement;
        SAMPLE_TYPE _lastDecayEnvelope;
        int _decayDuration;

        SAMPLE_TYPE _sustainIncrement;
        int _sustainDuration;

        SAMPLE_TYPE _releaseIncrement;
        int _releaseDuration;

        // attack starts at the beginning of the sound (thus at start
        // AudioBuffer, DSR envelopes have variable start offsets...)

        int _decayStart;
        int _sustainStart;
        int _releaseStart;

        // recalculates the increment values for all envelopes
        void invalidateEnvelopes();
        void setEnvelopesInternal( float attack, float decay, float sustain, float release );
};

#endif
