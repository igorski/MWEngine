#ifndef __BASEPROCESSOR_H_INCLUDED__
#define __BASEPROCESSOR_H_INCLUDED__

#include "global.h"
#include "audiobuffer.h"

class BaseProcessor
{
    public:
        BaseProcessor();
        ~BaseProcessor();

        /**
         * @param {audioBuffer*} sampleBuffer the buffer to write into
         * @param {bool} isMonoSource whether the source signal is mono (save CPU cycles
         *               by solely processing a single channel and writing its values into the
         *               remaining channels
         */
        virtual void process( AudioBuffer* sampleBuffer, bool isMonoSource );

        /**
         * if this processors effect is non-dynamic its output
         * can be cached to omit unnecessary repeated calculations
         * consuming unnecessary CPU cycles
         */
        virtual bool isCacheable();
};

#endif
