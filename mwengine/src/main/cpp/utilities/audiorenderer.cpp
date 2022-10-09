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
#include <events/sampleevent.h>
#include "audiorenderer.h"
#include "wavereader.h"
#include "wavewriter.h"
#include "samplemanager.h"
#include "utils.h"

namespace MWEngine {

bool AudioRenderer::renderEvent( const std::string& outputFilename, BaseAudioEvent* audioEvent, ProcessingChain* chain )
{
    AudioBuffer* outputBuffer;
    int bufferSize = audioEvent->getEventLength();

    // 1. calculate the rendered buffer size by taking the event length
    // and adding the tail the processors can optionally add to the signal

    if ( chain != nullptr ) {
        auto processors = chain->getActiveProcessors();
        for ( auto const &processor : processors ) {
            bufferSize += processor->addedDurationInSamples();
        }
    }

    outputBuffer = new AudioBuffer( AudioEngineProps::OUTPUT_CHANNELS, bufferSize );

    // 2. render the audio event for its entire duration into the output buffer

    audioEvent->mixBuffer( outputBuffer, 0, 0, audioEvent->getEventLength(), false, 0, false );

    // 3. apply the processing chain

    if ( chain != nullptr ) {
        auto processors = chain->getActiveProcessors();
        bool isMono = outputBuffer->amountOfChannels == 1;
        for ( auto &processor : processors ) {
            processor->process( outputBuffer, isMono );
        }
    }

    // 4. cap the max amplitude

    capBufferSamplesSafe( outputBuffer );

    // 5. write the output

    size_t writtenSamples = WaveWriter::bufferToWAV( outputFilename, outputBuffer, ( int ) AudioEngineProps::SAMPLE_RATE );

    // 6. clean up and assert success

    delete outputBuffer;

    return writtenSamples == bufferSize;
}

bool AudioRenderer::renderFile( const std::string& inputFilename, const std::string& outputFilename, ProcessingChain* processingChain )
{
    waveFile WAV = WaveReader::fileToBuffer( inputFilename );

    if ( WAV.buffer == nullptr ) {
        return false;
    }

    std::string tempSampleKey = "ar_ts";

    SampleManager::setSample( tempSampleKey, WAV.buffer, WAV.sampleRate );

    auto* event = new SampleEvent();
    event->setSample( SampleManager::getSample( tempSampleKey ));

    bool renderSuccess = renderEvent( outputFilename, event, processingChain );

    // free allocated memory

    delete event;
    SampleManager::removeSample( tempSampleKey, true );

    return renderSuccess;
}

bool AudioRenderer::mergeFiles( const std::string& inputFile1, const std::string& inputFile2, const std::string& outputFilename, bool attenuate )
{
    const int amountOfInputFiles = 2;
    std::string inputFilenames[] = { inputFile1, inputFile2 };

    waveFile inputWaveFiles[ amountOfInputFiles ];
    int bufferSize = 0;
    int amountOfChannels = 1;

    // 1. validate input files and determine maximum channel amount and file size

    for ( size_t i = 0; i < amountOfInputFiles; ++i ) {
        waveFile WAV = WaveReader::fileToBuffer( inputFilenames[ i ] );

        if ( WAV.buffer == nullptr ) {
            // invalid buffer, clean up allocated memory and return
            for ( size_t j = 0; j < i; ++j ) {
                delete inputWaveFiles[ j ].buffer;
            }
            return false;
        }
        bufferSize = std::max( bufferSize, WAV.buffer->bufferSize );
        amountOfChannels = std::max( amountOfChannels, WAV.buffer->amountOfChannels );

        inputWaveFiles[ i ] = WAV;
    }

    // 2. create output buffer and write the audio from all input files

    auto outputBuffer = new AudioBuffer( amountOfChannels, bufferSize );
    SAMPLE_TYPE attenuator = attenuate ? MAX_VOLUME / ( SAMPLE_TYPE ) amountOfInputFiles : MAX_VOLUME;

    for ( auto const WAV : inputWaveFiles ) {
        for ( int c = 0; c < amountOfChannels; ++c ) {
            SAMPLE_TYPE* inputChannel = WAV.buffer->amountOfChannels > c ? WAV.buffer->getBufferForChannel( c ) : WAV.buffer->getBufferForChannel( WAV.buffer->amountOfChannels - 1 );
            SAMPLE_TYPE* outputChannel = outputBuffer->getBufferForChannel( c );

            for ( size_t i = 0, l = std::min( WAV.buffer->bufferSize, bufferSize ); i < l; ++i ) {
                outputChannel[ i ] += ( inputChannel[ i ] * attenuator );
            }
        }
        // free up memory allocated to input wave file
        delete WAV.buffer;
    }

    // 3. write the output

    int writtenSamples = WaveWriter::bufferToWAV( outputFilename, outputBuffer, ( int ) AudioEngineProps::SAMPLE_RATE );

    // 4. clean up and assert success

    delete outputBuffer;

    return writtenSamples == bufferSize;
}

} // E.O namespace MWEngine
