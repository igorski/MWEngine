#ifndef __FILTER_H_INCLUDED__
#define __FILTER_H_INCLUDED__

#include "baseprocessor.h"
#include "lfo.h"

class Filter : public BaseProcessor
{
    public:
        Filter( float aCutoffFrequency, float aResonance, float aMinFreq, float aMaxFreq, float aLfoRate );
        ~Filter();

        void setCutoff( float frequency );
        float getCutoff();
        void setResonance( float resonance );
        float getResonance();
        bool hasLFO();
        void hasLFO( bool value );
        float getLFO();
        void setLFO( LFO *lfo );
        void setLFORate( float rate );
        void process( float* sampleBuffer, int bufferLength );

    protected:
        float _cutoff;
        float _resonance;
        float _tempCutoff; // used for reading when automating via LFO

        // LFO related

        LFO *_lfo;
        float minFreq;
        float maxFreq;
        float lfoRange;

        float fs;

        // filter specific, used internally

        float a1;
        float a2;
        float a3;
        float b1;
        float b2;
        float c;

        float in1;
        float in2;
        float out1;
        float out2;
        float output;

    private:
        bool _hasLFO;
        void calculateParameters();
};

#endif
