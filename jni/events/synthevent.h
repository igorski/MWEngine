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
#ifndef __SYNTHEVENT_H_INCLUDED__
#define __SYNTHEVENT_H_INCLUDED__

#include "basesynthevent.h"
#include "../arpeggiator.h"
#include "../ringbuffer.h"

/**
 * SynthEvent is an example of a BaseSynthEvent that uses
 * one or more oscillators to render output. It is completely equal
 * to the rendering used in MikroWave
 *
 * https://play.google.com/store/apps/details?id=nl.igorski.mikrowave&hl=en
 */
class SynthEvent : public BaseSynthEvent
{
    public:
        // construct as a sequenced event
        SynthEvent( float aFrequency, int aPosition, float aLength, SynthInstrument *aInstrument, bool aAutoCache );
        SynthEvent( float aFrequency, int aPosition, float aLength, SynthInstrument *aInstrument, bool aAutoCache, bool hasParent );

        // construct as live event (for instance: a keyboard noteOn)
        SynthEvent( float aFrequency, SynthInstrument *aInstrument );
        SynthEvent( float aFrequency, SynthInstrument *aInstrument, bool hasParent );

        ~SynthEvent();

        // properties
        void setFrequency( float aFrequency );
        void setFrequency( float aFrequency, bool allOscillators, bool storeAsBaseFrequency );
        float getBaseFrequency();   // return "root" frequency (frequency can be modulated by pitch modules)

        // render related
        void invalidateProperties( int aPosition, float aLength, SynthInstrument *aInstrument );
        void calculateBuffers();

        // cache related
        void cache( bool doCallback );

    protected:

        // setup related
        void init( SynthInstrument *aInstrument, float aFrequency, int aPosition,
                   int aLength, bool aIsSequenced, bool aHasParent );

        void addToSequencer();
        void setDeletable( bool value );

        // render related
        void render( AudioBuffer* outputBuffer );
        void updateProperties();

        // caching
        void resetCache();

    private:

        // used for waveform generation
        SAMPLE_TYPE _phase;
        SAMPLE_TYPE _phaseIncr;
        SAMPLE_TYPE _baseFrequency;
        SAMPLE_TYPE TWO_PI_OVER_SR;
        int _type;
        int _fadeInDuration;
        int _fadeOutDuration;

        // specific to Pulse Width Modulation
        float pwr;
        float pwAmp;

        // specific to Karplus-Strong synthesis

        RingBuffer* _ringBuffer;
        int _ringBufferSize;
        void initKarplusStrong();

        // oscillators

        SynthEvent *_osc2;  // secondary oscillator
        bool hasParent;
        float _pwmValue;
        void createOSC2( int aPosition, int aLength, SynthInstrument *aInstrument );
        void destroyOSC2();

        // modules

        void applyModules( SynthInstrument* instrument );
        Arpeggiator* _arpeggiator;
};

#endif
