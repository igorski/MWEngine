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
#include "basedynamicsprocessor.h"

namespace MWEngine {

EnvelopeDetector::EnvelopeDetector( float ms, float sampleRate )
{
    _sampleRate = sampleRate;
    setTimeConstant( ms );
}

void EnvelopeDetector::setTimeConstant( float ms )
{
    _timeConstant = std::max( 0.f, ms );
    cacheCoefficient();
}

void EnvelopeDetector::setSampleRate( float sampleRate )
{
    _sampleRate = sampleRate;
    cacheCoefficient();
}

void EnvelopeDetector::cacheCoefficient()
{
    _coefficient = exp( -1000.f / ( _timeConstant * _sampleRate ) );
}

BaseDynamicsProcessor::BaseDynamicsProcessor( float attackInMs, float releaseInMs, int sampleRate )
    : _attackEnvelope( attackInMs, ( float ) sampleRate )
    , _releaseEnvelope( releaseInMs, ( float ) sampleRate )
{

}

void BaseDynamicsProcessor::setAttack( float ms )
{
    _attackEnvelope.setTimeConstant( ms );
}

void BaseDynamicsProcessor::setRelease( float ms )
{
    _releaseEnvelope.setTimeConstant( ms );
}

void BaseDynamicsProcessor::setSampleRate( int sampleRate )
{
    _attackEnvelope.setSampleRate(( float ) sampleRate );
    _releaseEnvelope.setSampleRate(( float ) sampleRate );
}

} // E.O. namespace MWEngine