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
    _buffer             = BufferUtil::generateSilentBuffer( BufferUtil::calculateBufferLength( MIN_LFO_RATE ));

    TWO_PI              = 2 * ( atan( 1 ) * 4 );

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

                output = ( _phase < 0 ) ? _phase - ( int )( _phase - 1 ) : _phase - ( int )( _phase );
                break;
        }
        _phase += _phaseIncr;

        // restore _phase, max range is 0 - 1 ( float )
        // note this should be the end of the generate-range...
        if ( _phase > 1.0 )
            _phase -= 1.0;

        _buffer[ i ] = output;
    }
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
