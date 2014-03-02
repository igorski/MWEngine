#ifndef __BASEBUSPROCESSOR_H_INCLUDED__
#define __BASEBUSPROCESSOR_H_INCLUDED__

#include "audiobuffer.h"

class BaseBusProcessor
{
    public:
        BaseBusProcessor();
        ~BaseBusProcessor();

        /**
         * a bus processor differs from an BaseProcessor
         * as it ADDS the result of the process method
         * to the current sampleBuffer, whereas a common
         * BaseProcessor REPLACES the input with the
         * processed output
         *
         *
         * @param sampleBuffer {AudioBuffer*} the current buffer
         * @param isMonosource {bool} whether the input is mono
         */
        virtual void apply( AudioBuffer* sampleBuffer, bool isMonoSource );
};

#endif
