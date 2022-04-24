/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2022 Igor Zinken - https://www.igorski.nl
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
#ifndef __MWENGINE__AUDIORENDERER_H_INCLUDED__
#define __MWENGINE__AUDIORENDERER_H_INCLUDED__

#include <events/baseaudioevent.h>
#include "../processingchain.h"

/**
 * AudioRenderer is a static class that can perform operations
 * on the audio engine's actors to render output to WAV files
 */
namespace MWEngine {
class AudioRenderer {

    public:

        /**
         * Renders the audio provided by given audioEvent when processed by
         * given ProcessingChain into a WAV file of given outputFilename
         */
        static bool renderEvent( const std::string& outputFilename, BaseAudioEvent* audioEvent, ProcessingChain* processingChain );

        /**
         * Renders the audio of given inputFilename when processed by
         * given ProcessingChain into a WAV file of given outputFilename
         */
        static bool renderFile( const std::string& inputFilename, const std::string& outputFilename, ProcessingChain* processingChain );
};

} // E.O namespace MWEngine

#endif