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
#include "lfo.h"
#include "native_audio_engine.h"
#include "utils.h"
#include <math.h>

// constructor / destructor

LFO::LFO()
{
    _wave               = WaveForms::SINE_WAVE;
    _phase              = 0.0;
    _phaseIncr          = 0.0;
    _length             = 0;
    _readOffset         = 0;

    int bufferSize      = BufferUtil::calculateBufferLength( MIN_LFO_RATE );
    _buffer             = new float[ bufferSize ];

    for ( int i = 0; i < bufferSize; ++i )
        _buffer[ i ] = 0.0f;

    TWO_PI = 2 * ( atan( 1 ) * 4 );

    setRate( MIN_LFO_RATE );
}

LFO::~LFO()
{
    if ( _buffer != 0 )
    {
        delete _buffer;
        _buffer = 0;
    }
}

/* public methods */

float LFO::getRate()
{
    return _rate;
}

void LFO::setRate( float value )
{
    _rate          = value;
    _phase         = 0.0;
    _phaseIncr     = value / audio_engine::SAMPLE_RATE;
    TWO_PI_OVER_SR = TWO_PI / audio_engine::SAMPLE_RATE;

    setLength( BufferUtil::calculateBufferLength( value )); // will invoke generation of cycle
}

int LFO::getWave()
{
    return _wave;
}

void LFO::setWave( int value )
{
    _wave = value;

    // re-cache buffer if a length was set
    if ( _length > 0 )
        generate( _length );
}

int LFO::getLength()
{
    return _length;
}

void LFO::setLength( int value )
{
    if ( _length != value )
    {
        generate( value );

        // if new rate requires a smaller buffer range
        // reset the current read offset
        if ( value < _length )
            _readOffset = 0;
    }
    _length = value;
}

void LFO::generate( int aLength )
{
    float output = 0.0;
    float tmp;

    for ( int i = 0; i < aLength; ++i )
    {
        switch( _wave )
        {
            case WaveForms::SINE_WAVE:

                if ( _phase < .5 )
                {
                    tmp = ( _phase * 4.0 - 1.0 );
                    output = ( 1.0 - tmp * tmp );
                }
                else {
                    tmp = ( _phase * 4.0 - 3.0 );
                    output = ( tmp * tmp - 1.0 );
                }

                break;

            case WaveForms::TRIANGLE:

                output = ( _phase - ( int )( _phase )) * 4;

                if ( output < 2 )
                    output -= 1;
                else
                    output = 3 - output;
                break;

            case WaveForms::SQUARE_WAVE:

                if ( _phase < .5 )
                {
                    tmp = TWO_PI * ( _phase * 4.0 - 1.0 );
                    output = ( 1.0 - tmp * tmp );
                }
                else {
                    tmp = TWO_PI * ( _phase * 4.0 - 3.0 );
                    output = ( tmp * tmp - 1.0 );
                }
                break;

            case WaveForms::SAWTOOTH:

                output = ( _phase < 0.0f ) ? _phase - ( int )( _phase - 1.0f ) : _phase - ( int )( _phase );
                break;
        }
        _phase += _phaseIncr;

        // restore _phase, max range is 0 - 1 ( float )
        // note this should be the end of the generate-range...
        if ( _phase > 1.0f )
            _phase -= 1.0f;

        _buffer[ i ] = output;
    }
}

int LFO::getReadOffset()
{
    return _readOffset;
}

void LFO::setReadOffset( int value )
{
    _readOffset = value;
}

/**
 * return a value at the current read pointer
 * from the cached buffer
 */
float LFO::peek()
{
    float value = _buffer[ _readOffset ];

    if ( ++_readOffset == _length )
        _readOffset = 0;

    return value;
}
