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
#include "audiorenderer.h"
#include "wavewriter.h"

namespace MWEngine {

bool AudioRenderer::renderEvent( const std::string& outputFilename, BaseAudioEvent* audioEvent, ProcessingChain* chain )
{
    AudioBuffer* outputBuffer;
    int bufferSize = audioEvent->getEventLength();

    // 1. calculate the rendered buffer size by taking the event length
    // and adding the tail the processors can optionally add to the signal

    if ( chain != nullptr ) {
        auto processors = chain->getActiveProcessors();
        for ( auto &processor : processors ) {
            bufferSize += processor->addedDurationInSamples();
        }
    }

    outputBuffer = new AudioBuffer( AudioEngineProps::OUTPUT_CHANNELS, bufferSize );

    // 2. render the audio event for its entire duration into the output buffer

    audioEvent->mixBuffer( outputBuffer, 0, 0, audioEvent->getEventLength(), false, 0, false );

    // 3. apply the processing chain

    if ( chain != nullptr ) {
        auto processors = chain->getActiveProcessors();
        for ( auto &processor : processors ) {
            processor->process( outputBuffer, outputBuffer->amountOfChannels == 1 );
        }
    }

    // 4. write the output

    int writtenSamples = WaveWriter::bufferToWAV( outputFilename, outputBuffer, ( int ) AudioEngineProps::SAMPLE_RATE );

    // 5. clean up and assert success

    delete outputBuffer;

    return writtenSamples == bufferSize;
}

} // E.O namespace MWEngine
