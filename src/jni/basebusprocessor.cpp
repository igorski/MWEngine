#include "basebusprocessor.h"

/* constructor / destructor */

BaseBusProcessor::BaseBusProcessor()
{

}

BaseBusProcessor::~BaseBusProcessor()
{

}

/* public methods */

void BaseBusProcessor::apply( AudioBuffer* sampleBuffer, bool isMonoSource )
{
    // override in subclass
}