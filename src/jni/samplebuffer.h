#ifndef __SAMPLEBUFFER_H_INCLUDED__
#define __SAMPLEBUFFER_H_INCLUDED__

class SampleBuffer
{
    public:
        float* buffer;

        SampleBuffer();
        SampleBuffer( float* aBuffer );
        ~SampleBuffer();
};

#endif