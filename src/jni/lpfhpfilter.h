#ifndef __LPFHPFILTER_H_INCLUDED__
#define __LPFHPFILTER_H_INCLUDED__

#include "baseprocessor.h"

class LPFHPFilter : public BaseProcessor
{
    public:
        LPFHPFilter( float aLPCutoff, float aHPCutoff );
        void setLPF( float aCutOffFrequency, int aSampleRate );
        void setHPF( float aCutOffFrequency, int aSampleRate );
        void process( float* sampleBuffer, int bufferLength );

    private:
        float a0;
        float a1;
        float b0;
        float b1;
        float _prevSample;
        float _prevUnprocessedSample;
};

#endif
