#ifndef __FM_H_INCLUDED__
#define __FM_H_INCLUDED__

#include "baseprocessor.h"
#include "lfo.h"

class FrequencyModulator : public BaseProcessor, public LFO
{
    public:
        FrequencyModulator( int aWaveForm, float aRate );
        void process( float* sampleBuffer, int bufferLength );

        // these are here only for SWIG purposes so we can "multiple inherit" from LFO, bit fugly... but hey
        #ifdef SWIG
        float getRate();
        void setRate( float value );
        int getWave();
        void setWave( int value );
        void generate( int aLength );
        int getLength();
        void setLength( int value );

        #endif

    private:
        float* _buffer; // cached buffer
        float modulator;
        float carrier;
        float fmamp;
        float AMP_MULTIPLIER;
};

#endif
