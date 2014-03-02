#ifndef __BITCRUSHER_H_INCLUDED__
#define __BITCRUSHER_H_INCLUDED__

#include "baseprocessor.h"

class BitCrusher : public BaseProcessor
{
    public:
        BitCrusher( float amount, float level );

        float getAmount();
        void setAmount( float value );
        float getLevel();
        void setLevel( float value );
        void process( float* sampleBuffer, int bufferLength );

    private:
        int _bits; // we scale the amount to integers in the 1-16 range
        float _amount;
        float _level;
        float _levelCorrection;
};

#endif
