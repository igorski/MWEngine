/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2018 Igor Zinken - http://www.igorski.nl
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
#include "phaser.h"
#include "../global.h"
#include <math.h>

namespace MWEngine {

/* constructor / destructor */

/**
 * @param aRate     {float} desired LFO rate in Hz
 * @param aFeedback {float} 0 - 1
 * @param aDepth    {float} 0 - 1
 * @param aMinFreq  {float} minimum frequency value in Hz allowed for the filters range
 * @param aMaxFreq  {float} maxiumum frequency value in Hz allowed for the filters range
 */
Phaser::Phaser( float aRate, float aFeedback, float aDepth, float aMinFreq, float aMaxFreq )
{
    init( aRate, aFeedback, aDepth, aMinFreq, aMaxFreq, AudioEngineProps::OUTPUT_CHANNELS );
}

/**
 * @param aRate     {float} desired LFO rate in Hz
 * @param aFeedback {float} 0 - 1
 * @param aDepth    {float} 0 - 1
 * @param aMinFreq  {float} minimum frequency value in Hz allowed for the filters range
 * @param aMaxFreq  {float} maxiumum frequency value in Hz allowed for the filters range
 * @param amountOfChannels {int} amount of channels
 */
Phaser::Phaser( float aRate, float aFeedback, float aDepth, float aMinFreq, float aMaxFreq, int amountOfChannels )
{
    init( aRate, aFeedback, aDepth, aMinFreq, aMaxFreq, amountOfChannels );
}

Phaser::~Phaser()
{
    for ( int i = 0; i < _amountOfChannels; ++i ) {
        for ( int j = 0; j < STAGES; ++j )
            delete _alps->at( i ).at( j );

        _alps->erase( _alps->begin() + i );
    }
    delete _alps;
    _alps = nullptr;
}

/* public methods */

/**
 * set the filter range
 * @param aMin {float} lowest allowed value in Hz
 * @param aMax {float} highest allowed value in Hz
 */
void Phaser::setRange( float aMin, float aMax )
{
    _dmin = aMin / ( AudioEngineProps::SAMPLE_RATE / 2.0 );
    _dmax = aMax / ( AudioEngineProps::SAMPLE_RATE / 2.0 );
}

float Phaser::getRate()
{
    return _rate;
}

/**
 * @param aRate {float} in Hz
 */
void Phaser::setRate( float aRate )
{
    _rate   = aRate;
    _lfoInc = 2.0 * 3.14159f * ( _rate / AudioEngineProps::SAMPLE_RATE );
}

float Phaser::getFeedback()
{
    return _fb;
}

void Phaser::setFeedback( float fb )
{
    _fb = fb;
}

float Phaser::getDepth()
{
    return _depth;
}

void Phaser::setDepth( float depth )
{
    _depth = depth;
}

void Phaser::process( AudioBuffer* sampleBuffer, bool isMonoSource )
{
    int bufferSize = sampleBuffer->bufferSize;
    int amountOfChannels = std::min( _amountOfChannels, sampleBuffer->amountOfChannels );

    for ( int c = 0; c < amountOfChannels; ++c )
    {
        SAMPLE_TYPE* channelBuffer = sampleBuffer->getBufferForChannel( c );

        int i, j;
        std::vector<AllPassDelay*> delay = _alps->at( c );

        for ( i = 0; i < bufferSize; ++i )
        {
            // calculate and update phaser sweep LFO...
            SAMPLE_TYPE d  = _dmin + ( _dmax - _dmin ) * (( sin( _lfoPhase ) + 1.0 ) / 2.0 );
            _lfoPhase     += _lfoInc;

            if ( _lfoPhase >= TWO_PI )
                _lfoPhase -= TWO_PI;

            // update filter coefficients
            for ( j = 0; j < STAGES; ++j )
                delay.at( j )->delay( d );

            // calculate output
            float y = delay[ 0 ]->update(
                      delay[ 1 ]->update(
                      delay[ 2 ]->update(
                      delay[ 3 ]->update(
                      delay[ 4 ]->update(
                      delay[ 5 ]->update( channelBuffer[ i ] + _zm1 * _fb ))))));

            _zm1 = y;

            channelBuffer[ i ] += ( y * _depth );
        }

        // save CPU cycles when working on a mono source
        if ( isMonoSource )
        {
            sampleBuffer->applyMonoSource();
            break;
        }
    }
}

void Phaser::init( float aRate, float aFeedback, float aDepth, float aMinFreq, float aMaxFreq, int amountOfChannels )
{
    _lfoPhase         = 0.0;
    _zm1              = 0.0;
    _amountOfChannels = amountOfChannels;

    setRange( aMinFreq, aMaxFreq );
    setRate( aRate );

    _fb       = aFeedback;
    _depth    = aDepth;

    _alps = new std::vector<std::vector<AllPassDelay*>>( amountOfChannels );

    for ( int i = 0; i < _amountOfChannels; ++i ) {
        _alps->at( i ) = std::vector<AllPassDelay*>( STAGES );

        for ( int j = 0; j < STAGES; ++j )
            _alps->at( i ).at( j ) = new AllPassDelay();
    }
}

/* "private" class */

AllPassDelay::AllPassDelay()
{
    _a1  = 0.0;
    _zm1 = 0.0;
}

/* public methods */

// sample delay time
void AllPassDelay::delay( float aDelay )
{
    _a1 = ( 1.0 - aDelay ) / ( 1.0 + aDelay );
}

float AllPassDelay::update( float aSample )
{
    float y = aSample * - _a1 + _zm1;
    _zm1 = y * _a1 + aSample;

    return y;
}

} // E.O namespace MWEngine
