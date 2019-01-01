/**
 * The MIT License (MIT)
 *
 * based on public source code by Dennis Cronin, adapted
 * by Igor Zinken - http://www.igorski.nl
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
#include "flanger.h"
#include "../global.h"
#include <utilities/utils.h>
#include <utilities/bufferutility.h>
#include <algorithm>

namespace MWEngine {

/* constructors / destructor */

Flanger::Flanger( float rate, float width, float feedback, float delay, float mix )
{
    init( rate, width, feedback, delay, mix );
}

Flanger::Flanger()
{
    init( 0.1f, 0.5f, 0.75f, .1f, 1.f );
}

Flanger::~Flanger()
{
    while ( _buffers.size() > 0 ) {
        delete _buffers.back();
        _buffers.pop_back();
    }
    while ( _caches.size() > 0 ) {
        delete _caches.back();
        _caches.pop_back();
    }
    delete _delayFilter;
    delete _mixFilter;
}

/* public methods */

float Flanger::getRate()
{
    return _rate;
}

void Flanger::setRate( float value )
{
    _rate = value;

    // map into param onto 0.05Hz - 10hz with log curve
    _sweepRate  = pow( 10.0, ( SAMPLE_TYPE ) _rate );
    _sweepRate -= 1.0;
    _sweepRate *= 1.05556f;
    _sweepRate += 0.05f;

    setSweep();
}

float Flanger::getWidth()
{
    return _width;
}

void Flanger::setWidth( float value )
{
    _width = value;

    if ( value <= 0.05)
        _sweepSamples = 0.0;
    else
        _sweepSamples = value * SAMPLE_MULTIPLIER;

    setSweep();
}

float Flanger::getDelay()
{
    return _delay;
}

void Flanger::setDelay( float value )
{
    _delay = value;
}

float Flanger::getFeedback()
{
    return _feedback;
}

void Flanger::setFeedback( float value )
{
    _feedback = value;
}

float Flanger::getMix()
{
    return _mix;
}

void Flanger::setMix( float value )
{
    _mix = value;
}

void Flanger::setChannelMix( int channel, float wet )
{
    wet       = capParam( wet );
    float dry = 1.0f - wet;

    _caches.at( channel )->mixWet = wet;
    _caches.at( channel )->mixDry = dry;
}

void Flanger::process( AudioBuffer* sampleBuffer, bool isMonoSource )
{
    int maxWriteIndex = FLANGER_BUFFER_SIZE - 1;

    SAMPLE_TYPE delay, mix, delaySamples, sample, w1, w2, ep;
    int ep1, ep2;

    int amountOfChannels = std::min( sampleBuffer->amountOfChannels, ( int ) _buffers.size() );
    int bufferSize       = sampleBuffer->bufferSize;

    // store/restore the processor properties
    // this ensures that multi channel processing for a
    // single buffer uses all properties across all channels
    // store() before processing channel 0, restore() every
    // channel afterwards

    int writePointerStored  = _writePointer;
    SAMPLE_TYPE sweepStored = _sweep;

    _delayFilter->store();
    _mixFilter->store();

    for ( int c = 0; c < amountOfChannels; ++c  ) {

        SAMPLE_TYPE* channelBuffer = sampleBuffer->getBufferForChannel( c );
        SAMPLE_TYPE* delayBuffer   = _buffers.at( c );
        ChannelCache* channelCache = _caches.at( c );

        // when first channel has been processed, restore the stored values
        // for the next channel iteration

        if ( c > 0 ) {
            _writePointer = writePointerStored;
            _sweep        = sweepStored;
            _delayFilter->restore();
            _mixFilter->restore();
        }

        for ( int i = 0; i < bufferSize; i++ ) {

            // filter delay and mix output

            delay = _delayFilter->processSingle( _delay );
            mix   = _mixFilter->processSingle( _mix );

            if ( ++_writePointer > maxWriteIndex )
                _writePointer = 0;

            // delay 0.0-1.0 maps to 0.02ms to 10ms (always have at least 1 sample of delay)
            delaySamples = ( delay * SAMPLE_MULTIPLIER ) + 1.f;
            delaySamples += _sweep;

            // build the two emptying pointers and do linear interpolation
            ep = ( SAMPLE_TYPE ) _writePointer - delaySamples;

            if ( ep < 0.0 )
                ep += ( SAMPLE_TYPE ) FLANGER_BUFFER_SIZE;

            MODF( ep, ep1, w2 );
            w1 = 1.0 - w2;

            if ( ++ep1 > maxWriteIndex )
                ep1 = 0;

            ep2 = ep1 + 1;

            if ( ep2 > maxWriteIndex )
                ep2 = 0;

            // process input channels and write caches

            sample = channelBuffer[ i ];
            delayBuffer[ _writePointer ] =
                sample + _feedback * _feedbackPhase * channelCache->lastSample;

            channelCache->lastSample = delayBuffer[ ep1 ] * w1 + delayBuffer[ ep2 ] * w2;

            // write effected sample into the output buffer

            channelBuffer[ i ] = capSample(
                channelCache->mixDry * sample + channelCache->mixWet * mix * channelCache->lastSample
            );

            // process sweep

            if ( _step != 0.0 ) {
                _sweep += _step;

                if ( _sweep <= 0.0 ) {
                    _sweep = 0.0;
                    _step = -_step;
                }
                else if ( _sweep >= _maxSweepSamples )
                    _step = -_step;
            }
        }
    }
}

/* protected methods */

void Flanger::setSweep()
{
    // translate sweep rate to samples per second
    _step = ( _sweepSamples * 2.0 * _sweepRate ) / ( SAMPLE_TYPE ) AudioEngineProps::SAMPLE_RATE;
    _maxSweepSamples = _sweepSamples;
    _sweep = 0.0;
}

void Flanger::init( float rate, float width, float feedback, float delay, float mix )
{
    FLANGER_BUFFER_SIZE = ( int ) (( SAMPLE_TYPE ) AudioEngineProps::SAMPLE_RATE / 5.0f );
    SAMPLE_MULTIPLIER   = ( SAMPLE_TYPE ) AudioEngineProps::SAMPLE_RATE * 0.01f;

    _writePointer    = 0;
    _feedbackPhase   = 1.f;
    _sweepSamples    = 0.f;

    // create sample buffers and caches for each channel

    for ( int i = 0; i < AudioEngineProps::OUTPUT_CHANNELS; ++i ) {
        _buffers.push_back( BufferUtility::generateSilentBuffer( FLANGER_BUFFER_SIZE ));
        _caches.push_back( new ChannelCache());
    }

    _delayFilter = new LowPassFilter( 20.0f );
    _mixFilter   = new LowPassFilter( 20.0f );

    setRate( rate );
    setWidth( width );
    setFeedback( feedback );
    setDelay( delay );
    setMix( mix );
}

} // E.O namespace MWEngine
