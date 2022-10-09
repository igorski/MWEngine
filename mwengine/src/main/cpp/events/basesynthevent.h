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
#ifndef __MWENGINE__BASESYNTHEVENT_H_INCLUDED__
#define __MWENGINE__BASESYNTHEVENT_H_INCLUDED__

#include <global.h>
#include <resizable_audiobuffer.h>
#include "baseaudioevent.h"

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
 * a SynthEvent describes an AudioEvent that renders its audio at runtime, by
 * hooking into the associated (Synth)Instruments Synthesizer
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
        void setSequencePosition( int positionInSequencerSteps );
        int getSequencePosition();
        void setSequenceDuration( float durationInSequencerSteps );
        float getSequenceDuration();

#ifndef SWIG
        // internal to the engine
        bool released;
#endif
        void play();
        void stop();

        int getEventEnd();
        // events with a positive release phase should not directly be removed upon stop() / sequencer removal
        bool shouldEnqueueRemoval();

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
        void invalidateProperties();

#ifndef SWIG
        // internal to the engine
        void mixBuffer( AudioBuffer* outputBuffer, int bufferPos,
                        int minBufferPosition, int maxBufferPosition,
                        bool loopStarted, int loopOffset, bool useChannelRange );

        void mixBuffer( AudioBuffer* outputBuffer );
#endif

        void unlock();
        void repositionToTempoChange( float ratio );

        virtual void enqueueRemoval( bool value );

    protected:

        static unsigned int INSTANCE_COUNT;

        // used for waveform generation
        SAMPLE_TYPE _frequency, _baseFrequency;

        // properties for non-sequenced synthesis

        int _minLength;
        bool _hasMinLength;
        bool _shouldEnqueueRemoval;

        // properties for sequenced synthesis

        int _position;
        float _duration;

        // gets the instrument rendering this event, typecast to its subclass
        SynthInstrument* getSynthInstrument();

        // setup related

        void init( SynthInstrument* aInstrument, float aFrequency, int aPosition, float aLength, bool aIsSequenced );

        // render related
        virtual void updateProperties();
        virtual void triggerRelease();

        // grabs a temporary, empty buffer to render the audio into (this intermediate buffer
        // is necessary as each event can trigger its own ADSR envelopes over its lifetime,
        // affecting the amplitude of the existing buffers contents) the resulting synthesized audio
        // in this buffer can be merged into the actual output buffer provided to the mixBuffer() methods
        ResizableAudioBuffer* getEmptyTempBuffer( int bufferSize );
};
} // E.O namespace MWEngine

#endif
