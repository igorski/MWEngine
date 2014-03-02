#ifndef __SYNTHEVENT_H_INCLUDED__
#define __SYNTHEVENT_H_INCLUDED__

#include "basecacheableaudioevent.h"
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

        float getFrequency();
        void setFrequency( float aFrequency );
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

        float* getBuffer();
        float* synthesize( int aBufferLength );
        void unlock();

    private:

        // reference to instrument
        SynthInstrument* _instrument;

        // used for waveform generation
        float _phase;
        float _phaseIncr;
        float _frequency;
        int    _type;
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

        // live synthesis

        int _minLength;
        bool _hasMinLength;
        bool _queuedForDeletion;

        float* _liveBuffer;

        void init( SynthInstrument *aInstrument, float aFrequency, int aPosition, int aLength, bool aHasParent, bool aLiveSynthesis );
        void createOSC2( int aPosition, int aLength, SynthInstrument *aInstrument );
        void setDeletable( bool value );
        void render( float* aOutputBuffer, int bufferLength );
        void initKarplusStrong();
        void destroyLiveBuffer();
        void destroyOSC2();

        // caching
        void doCache( int length );
        void resetCache();
        void resetEnvelopes();
};

#endif
