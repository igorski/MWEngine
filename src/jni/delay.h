#ifndef __DELAY_H_INCLUDED__
#define __DELAY_H_INCLUDED__

#include "basebusprocessor.h"

class Delay : public BaseBusProcessor
{
    public:
        Delay( float aDelayTime, float aMaxDelayTime, float aMix, float aFeedback );
        ~Delay();

        float getDelayTime();
        void setDelayTime( float aValue );
        float getMix();
        void setMix( float aValue );
        float getFeedback();
        void setFeedback( float aValue );
        void apply( float* sampleBuffer, int bufferLength );
        void reset();

    protected:
        float* _delayBuffer;
        int _delayIndex;
        int _time;
        int _maxTime;
        float _mix;
        float _feedback;
};

#endif
