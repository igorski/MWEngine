/**
 * The MIT License (MIT)
 * Copyright (c) 2013-2022 Igor Zinken - https://www.igorski.nl
 *
 * dynamics processing based off work by Chunkware
 *
 * (c) 2006, ChunkWare Music Software, OPEN-SOURCE
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#ifndef __MWENGINE_GATE_H__
#define __MWENGINE_GATE_H__

#include "basedynamicsprocessor.h"

namespace MWEngine {
class Gate : public BaseDynamicsProcessor
{
    public:
        Gate();
        ~Gate();

        std::string getType() {
            return std::string( "Gate" );
        }

        float getThreshold() const {
            return _thresholdDb;
        }
        void setThreshold( float dB );

        void init();

#ifndef SWIG
        // internal to the engine
        void process( AudioBuffer* audioBuffer, bool isMonoSource );
        bool isCacheable();
#endif

    private:
        float _thresholdDb;
        float _thresholdLinear;
        float _overThreshEnvLinear;

        inline void processInternal( SAMPLE_TYPE &in1, SAMPLE_TYPE &in2 )
        {
            // create sidechain
            SAMPLE_TYPE rect1 = fabs( in1 );
            SAMPLE_TYPE rect2 = fabs( in2 );

            SAMPLE_TYPE keyLinked = fabs( std::max( rect1, rect2 )); // link channels with greater of 2, and rectify

            SAMPLE_TYPE keyOverThresh = SAMPLE_TYPE( keyLinked > _thresholdLinear );

            keyOverThresh += DC_OFFSET;
            BaseDynamicsProcessor::run( keyOverThresh, _overThreshEnvLinear );
            keyOverThresh = _overThreshEnvLinear - DC_OFFSET;

            // apply gain reduction
            in1 *= keyOverThresh;
            in2 *= keyOverThresh;
        }
};
} // E.O. namespace MWEngine

#endif