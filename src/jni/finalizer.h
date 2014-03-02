#ifndef __FINALIZER_H_INCLUDED__
#define __FINALIZER_H_INCLUDED__

#include "limiter.h"

class Finalizer : public Limiter
{
    public:
        Finalizer( float attackMs, float releaseMs, int sampleRate );
        short* process( float* sampleBuffer, int bufferLength );
        void limitfloats( float* sampleBuffer, int bufferLength );

    private:
        short lastSample;
        float lastDSample;
};

#endif
