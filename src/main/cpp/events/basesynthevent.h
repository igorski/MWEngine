/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2018 Igor Zinken - http://www.igorski.nl
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
#ifndef __MWENGINE__BASESYNTHEVENT_H_INCLUDED__
#define __MWENGINE__BASESYNTHEVENT_H_INCLUDED__

#include "baseaudioevent.h"
#include "../global.h"

namespace MWEngine {

class SynthInstrument;  // forward declaration, see <instruments/synthinstrument.h>

// will hold references to last known values (see <generators/synthesizer.cpp>)

typedef struct
{
    SAMPLE_TYPE envelope;     // the level of the last applied ADSR envelope
    SAMPLE_TYPE releaseLevel; // the level from which the release envelope will operate (when event is released)
    int envelopeOffset;       // the offset at which the release envelope is (when event is released)

    SAMPLE_TYPE phaseIncr;
    int arpeggioPosition;
    int arpeggioStep;

    std::vector<SAMPLE_TYPE> oscillatorPhases;

} CachedProperties;

/**
 * a SynthEvent describes an AudioEvent that can render
 * its audio when needed, it should basically combine rendering logic
 * into the basic AudioEvent interface
 */
class BaseSynthEvent : public BaseAudioEvent
{
    public:

        BaseSynthEvent();
        BaseSynthEvent( float aFrequency, int aPosition, float aLength, SynthInstrument* aInstrument );
        BaseSynthEvent( float aFrequency, SynthInstrument* aInstrument );

        ~BaseSynthEvent();

        unsigned int instanceId;

        // sequencer related properties
        int position;
        float length;
        bool released;

        void play();
        void stop();

        int getEventEnd();
        bool isQueuedForDeletion();

        // synthesis properties

        float getFrequency();     // return current event frequency (frequency can be modulated by pitch modules, legato, etc.)
        float getBaseFrequency(); // return "root" frequency

        void setFrequency( float aFrequency );
        void setFrequency( float aFrequency, bool storeAsBaseFrequency );

        CachedProperties cachedProps;

        // reference to last phase for a given oscillator render (see synthesizer.h)

        SAMPLE_TYPE getPhaseForOscillator( int aOscillatorNum );
        void setPhaseForOscillator( int aOscillatorNum, SAMPLE_TYPE aPhase );

        int lastWriteIndex;

        // render related
        void invalidateProperties( int aPosition, float aLength, SynthInstrument* aInstrument );
        void calculateBuffers();
        void mixBuffer( AudioBuffer* outputBuffer, int bufferPos,
                        int minBufferPosition, int maxBufferPosition,
                        bool loopStarted, int loopOffset, bool useChannelRange );

        void mixBuffer( AudioBuffer* outputBuffer );

        void unlock();

        virtual void setDeletable( bool value );

    protected:

        static unsigned int INSTANCE_COUNT;

        // used for waveform generation
        SAMPLE_TYPE _frequency, _baseFrequency;

        // properties for non-sequenced synthesis

        int _minLength;
        bool _hasMinLength, _queuedForDeletion;

        SynthInstrument* _synthInstrument;

        // setup related

        void init( SynthInstrument* aInstrument, float aFrequency, int aPosition, float aLength, bool aIsSequenced );

        // render related
        virtual void updateProperties();
        virtual void triggerRelease();
};
} // E.O namespace MWEngine

#endif
