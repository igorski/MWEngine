#ifndef __WAVESHAPER_H_INCLUDED__
#define __WAVESHAPER_H_INCLUDED__

#include "baseprocessor.h"

class WaveShaper : public BaseProcessor
{
    public:
        WaveShaper( float amount, float level );

        float getAmount();
        void setAmount( float value );
        float getLevel();
        void setLevel( float value );
        void process( float* sampleBuffer, int sampleLength );

    private:
        float _amount;
        float _level;
};

#endif
