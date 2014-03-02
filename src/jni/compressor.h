#ifndef __COMPRESSOR_H_INCLUDED__
#define __COMPRESSOR_H_INCLUDED__

#include "baseprocessor.h"
#include <chunkware/SimpleComp.h>

class Compressor : public BaseProcessor
{
    public:
        Compressor( float aThreshold, float aAttack, float aRelease, float aRatio );
        ~Compressor();

        static const int THRESHOLD_MAX_NEGATIVE_VALUE = 40;
        static const int THRESHOLD_MAX_POSITIVE_VALUE = 20;

        void process( float* sampleBuffer, int bufferLength );

        float getAttack();
        void setAttack( float value );

        float getRelease();
        void setRelease( float value );

        float getThreshold();
        void setThreshold( float value );

        float getRatio();
        void setRatio( float value );

        void setSampleRate( int aSampleRate );

    protected:

    private:
        chunkware_simple::SimpleComp* _sc;
};

#endif
