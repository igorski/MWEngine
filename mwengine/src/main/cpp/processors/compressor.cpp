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
#include "compressor.h"

namespace MWEngine {

/* constructor / destructor */

Compressor::Compressor() : BaseDynamicsProcessor( 10.0, 100.0 )
{
    setThreshold( 0.0 );
    setRatio( 1.0 );
    setSampleRate( AudioEngineProps::SAMPLE_RATE );
    init();
}

Compressor::~Compressor()
{

}

/* getters / setters */

void Compressor::setThreshold( float dB )
{
    _thresholdDb = dB;
}

void Compressor::setRatio( float ratio )
{
    _ratio = ratio;
}

/* public methods */

void Compressor::init()
{
    _overThreshEnvDb = DC_OFFSET;
}

void Compressor::process( AudioBuffer* sampleBuffer, bool isMonoSource )
{
    int bufferSize = sampleBuffer->bufferSize;
    bool isMonoBuffer = sampleBuffer->amountOfChannels == 1;

    SAMPLE_TYPE* leftChannel  = sampleBuffer->getBufferForChannel( 0 );
    SAMPLE_TYPE* rightChannel = isMonoBuffer ? leftChannel : sampleBuffer->getBufferForChannel( 1 );

    for ( int i = 0; i < bufferSize; ++i ) {
        processInternal( leftChannel[ i ], rightChannel[ i ] );
    }
}

bool Compressor::isCacheable()
{
    return true;
}

} // E.O. namespace MWEngine