#ifndef __AUDIOBUFFER_H_INCLUDED__
#define __AUDIOBUFFER_H_INCLUDED__

#include <vector>

class AudioBuffer
{
    public:
        AudioBuffer( int aAmountOfChannels, int aBufferSize );
        ~AudioBuffer();

        int amountOfChannels;
        int bufferSize;
        bool loopeable;

        float* getBufferForChannel( int aChannelNum );
        int mergeBuffers( AudioBuffer* aBuffer, int aReadOffset, int aWriteOffset );
        void silenceBuffers();
        void applyMonoSource();
        AudioBuffer* clone();

    protected:
        std::vector<float*>* _buffers;
};

#endif
