#ifndef __ENVELOPEFOLLOWER_H_INCLUDED__
#define __ENVELOPEFOLLOWER_H_INCLUDED__

class EnvelopeFollower
{
    public:
        EnvelopeFollower( float maxGain, float attackMs, float releaseMs, int sampleRate );
        float envelope;
        void process( float src, int skip );

    protected:
        float _attack;
        float _release;
};

#endif
