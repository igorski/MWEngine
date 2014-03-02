#ifndef __LIMITER_H_INCLUDED__
#define __LIMITER_H_INCLUDED__

#include "envelopefollower.h"

class Limiter
{
    public:
        Limiter();
        Limiter( float attackMs, float releaseMs, int sampleRate );
        ~Limiter();
        float getLinearGR();
        short* process( float* sampleBuffer, int bufferLength );
        void limitfloats( float* sampleBuffer, int bufferLength );

    protected:
        void init( float attackMs, float releaseMs, int sampleRate );
        short* _output;

    private:
        int skip;
        float maxGain;
        EnvelopeFollower* _follower;
};

#endif
