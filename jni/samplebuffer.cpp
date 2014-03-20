#include "samplebuffer.h"

/* constructor / destructor */

SampleBuffer::SampleBuffer( float* aBuffer )
{
    buffer = aBuffer;
};

SampleBuffer::~SampleBuffer()
{
    delete buffer;
};
