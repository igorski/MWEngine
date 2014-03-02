#ifndef __PHASER_H_INCLUDED__
#define __PHASER_H_INCLUDED__

#include "baseprocessor.h"

class AllPassDelay
{
    public:
        AllPassDelay();
        void delay( float aDelay );
        float update( float aSample );

    private:
        float _a1;
        float _zm1;
};

class Phaser : public BaseProcessor
{
    public:
        Phaser( float aRate, float aFeedback, float aDepth, float aMinFreq, float aMaxFreq );
        ~Phaser();

        void setDepth( float depth );
        float getDepth();
        void setFeedback( float fb );
        float getFeedback();
        void setRate( float aRate );
        float getRate();
        void setRange( float aMin, float aMax );
        void process( float* sampleBuffer, int bufferLength );

    private:
        float _dmin;
        float _dmax;
        float _fb;
        float _depth;
        float _zm1;
        float _lfoPhase;
        float _lfoInc;
        float _rate;

        AllPassDelay *_alps[ 6 ]; // six-stage phaser
};

#endif
