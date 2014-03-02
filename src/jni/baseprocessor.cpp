#include "baseprocessor.h"

/* constructor / destructor */

BaseProcessor::BaseProcessor()
{

}

BaseProcessor::~BaseProcessor()
{

}

/* public methods */

void BaseProcessor::process( AudioBuffer* sampleBuffer, bool isMonoSource )
{
    // override in subclass
}

bool BaseProcessor::isCacheable()
{
    return false;   // override in subclass
}
