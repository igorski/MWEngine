/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2021 Igor Zinken - https://www.igorski.nl
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef __MWENGINE__LEVEL_UTILITY_H_INCLUDED__
#define __MWENGINE__LEVEL_UTILITY_H_INCLUDED__

#include "global.h"
#include "utils.h"
#include "../audiochannel.h"

/**
 * LevelUtility provides methods to determine the mean level
 * of an AudioBuffer, which can be used for UI representations (e.g. level meters)
 */
namespace MWEngine {
class LevelUtility
{
    public:
        // maximum abs value of the given AudioChannels signal
        static SAMPLE_TYPE max( AudioChannel* audioChannel, int channelNum );

        // root mean square of the given AudioChannels signal
        static float RMS( AudioChannel* audioChannel, int channelNum );

        // dBSPL for the given AudioChannels signal
        static float dBSPL( AudioChannel* audioChannel, int channelNum );

        // linear energy of the given AudioChannels signal
        inline static float linear( AudioChannel* audioChannel, int channelNum ) {
            AudioBuffer* audioBuffer = audioChannel->getOutputBuffer();
            SAMPLE_TYPE* buffer      = audioBuffer->getBufferForChannel( channelNum );
            SAMPLE_TYPE sum          = 0.0;
            SAMPLE_TYPE sample;

            for ( int i = 0, l = audioBuffer->bufferSize; i < l; ++i ) {
                sample = capSample( buffer[ i ]);
                sum   += ( sample * sample );
            }
            return ( float ) sum;
        }
};
} // E.O namespace MWEngine

#endif
