#ifndef __DCOFFSETFILTER_H_INCLUDED__
#define __DCOFFSETFILTER_H_INCLUDED__

#include "baseprocessor.h"

class DCOffsetFilter : public BaseProcessor
{
    public:
        DCOffsetFilter();

    private:
        float lastSample;
        float R;
}

#endif
