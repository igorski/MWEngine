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

#include "basecacheableaudioevent.h"
#include "arpeggiator.h"
#include "synthinstrument.h"
#include "lfo.h"
#include "ringbuffer.h"
#include "global.h"

class SynthEvent : public BaseCacheableAudioEvent
{
    public:
        // construct as cacheable sequencer event
        SynthEvent( float aFrequency, int aPosition, float aLength, SynthInstrument *aInstrument, bool aAutoCache );
        SynthEvent( float aFrequency, int aPosition, float aLength, SynthInstrument *aInstrument, bool aAutoCache, bool hasParent );
        // construct as live event
        SynthEvent( float aFrequency, SynthInstrument *aInstrument );
        SynthEvent( float aFrequency, SynthInstrument *aInstrument, bool hasParent );

        ~SynthEvent();

        // envelopes

        static const float VOLUME_CORRECTION = .65; // we downscale the volume of the instrument as multi-timbral pops occur easily
        bool liveSynthesis;

        // used by view representation
        int position;
        float length;

        // properties
        float getFrequency();
        void setFrequency( float aFrequency );
        void setFrequency( float aFrequency, bool allOscillators, bool storeAsBaseFrequency );
        void updateProperties( int aPosition, float aLength, SynthInstrument *aInstrument, int aState );
        void calculateBuffers();
        void cache( bool doCallback );
        float getAttack();
        void setAttack( float aValue );
        int getDecay();
        void setDecay( int aValue );
        float getRelease();
        void setRelease( float aValue );
        float getVolume();
        void setVolume( float aValue );

        AudioBuffer* getBuffer();
        AudioBuffer* synthesize( int aBufferLength );

        void unlock();

    private:

        // reference to instrument
        SynthInstrument* _instrument;

        // used for waveform generation
        SAMPLE_TYPE _phase;
        SAMPLE_TYPE _phaseIncr;
        SAMPLE_TYPE _frequency;
        SAMPLE_TYPE _baseFrequency;
        int   _type;
        float PI;
        float TWO_PI;
        float TWO_PI_OVER_SR;
        float _decay;
        float _release;
        float _attack;
        float _volume;

        // specific to Pulse Width Modulation
        float pwr;
        float pwAmp;

        // specific to Karplus-Strong synthesis

        RingBuffer* _ringBuffer;
        int _ringBufferSize;
        float EnergyDecayFactor;

        int decayStart;
        int decayIncr;
        float attackEnv;
        float attackIncr;
        int releaseStart;
        float releaseIncr;
        float releaseEnv;
        static const int DEFAULT_FADE_DURATION = 8; // in samples
        static const int DECAY_MULTIPLIER      = 200;

        // oscillators

        LFO *_rOsc;         // routeable oscillator
        SynthEvent *_osc2;  // secondary oscillator
        bool hasParent;
        float _pwmValue;

        // modules

        void applyModules( SynthInstrument* instrument );
        Arpeggiator* _arpeggiator;

        // live synthesis

        int _minLength;
        bool _hasMinLength;
        bool _queuedForDeletion;

        AudioBuffer* _liveBuffer;

        void init( SynthInstrument *aInstrument, float aFrequency, int aPosition, int aLength, bool aHasParent, bool aLiveSynthesis );
        void createOSC2( int aPosition, int aLength, SynthInstrument *aInstrument );
        void setDeletable( bool value );
        void render( AudioBuffer* outputBuffer );
        void initKarplusStrong();
        void destroyLiveBuffer();
        void destroyOSC2();

        // caching
        void doCache();
        void resetCache();
        void resetEnvelopes();
};

#endif
