/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2014 Igor Zinken - http://www.igorski.nl
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
#include "compressor.h"
#include "global.h"
#include "utils.h"
#include <algorithm>
#include <cstdlib>
#include <math.h>

/**
 * @param aThreshold  {float} threshold in dB before the compression kicks in, the range is
 *                             -THRESHOLD_MAX_NEGATIVE_VALUE - THRESHOLD_MAX_POSITIVE_VALUE
 * @param aAttack     {float} attack time, in milliseconds, 0.1 - 100 ms
 * @param aRelease    {float} release time, in milliseconds, 200 - 2000 ms
 * @param aRatio      {float} ratio > 0 ratio (compression: < 1 ; expansion: > 1)
 */
Compressor::Compressor( float aThreshold, float aAttack, float aRelease, float aRatio )
{
    _sc = new chunkware_simple::SimpleComp();

    setSampleRate( AudioEngineProps::SAMPLE_RATE );
    setThreshold ( aThreshold );
    setAttack    ( aAttack );
    setRelease   ( aRelease );
    setRatio     ( aRatio );

    _sc->initRuntime(); // TODO : is this right ?
}

Compressor::~Compressor()
{
    delete _sc;
}

/* public methods */

void Compressor::process( AudioBuffer* sampleBuffer, bool isMonoSource )
{
    // only stereo...
    int bufferSize = sampleBuffer->bufferSize;

    SAMPLE_TYPE* leftChannel  = sampleBuffer->getBufferForChannel( 0 );
    SAMPLE_TYPE* rightChannel = sampleBuffer->amountOfChannels > 1 ? sampleBuffer->getBufferForChannel( 1 ) : leftChannel;

    for ( int i = 0; i < bufferSize; ++i )
        _sc->process( leftChannel[ i ], rightChannel[ i ] );
}

bool Compressor::isCacheable()
{
    return true;
}

/* getters / setters */

float Compressor::getAttack()
{
    return _sc->getAttack();
}

void Compressor::setAttack( float value )
{
    _sc->setAttack( value );
}

float Compressor::getRelease()
{
    return _sc->getRelease();
}

void Compressor::setRelease( float value )
{
    _sc->setRelease( value );
}

float Compressor::getRatio()
{
    return _sc->getRatio();
}

void Compressor::setRatio( float value )
{
    _sc->setRatio( value );
}

float Compressor::getThreshold()
{
    return _sc->getThresh();
}

void Compressor::setThreshold( float value )
{
    _sc->setThresh( value );
}

void Compressor::setSampleRate( int aSampleRate )
{
    _sc->setSampleRate(( float ) aSampleRate );
}
