#ifndef __RINGBUFFER_H_INCLUDED__
#define __RINGBUFFER_H_INCLUDED__

class RingBuffer
{
    public:
        RingBuffer( int capacity );
        ~RingBuffer();
        int getSize();
        bool isEmpty();
        bool isFull();
        void flush();
        void enqueue( float aSample );
        float dequeue();
        float peek();

    protected:
        float* _buffer;
        int bufferLength;
        int first;
        int last;
};

#endif
