#ifndef __DECIMATOR_H_INCLUDED__
#define __DECIMATOR_H_INCLUDED__

#include "baseprocessor.h"

class Decimator : public BaseProcessor
{
    public:
        Decimator( int bits, float rate );

        int getBits();
        void setBits( int value );
        float getRate();
        void setRate( float value );
        void process( float* sampleBuffer, int bufferLength );

    private:
        int _bits;
        float _rate;
        long _m;
        float _count;
};

#endif
