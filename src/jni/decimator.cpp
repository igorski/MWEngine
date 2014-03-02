#include "decimator.h"
#include <math.h>

// constructor

/**
 * @param bits {int} 1 - 32
 * @param rate {float} sample rate 0 - 1 ( 1 being equal to the original sample rate )
 */
Decimator::Decimator( int bits, float rate )
{
    setBits( bits );
    setRate( rate );

    _count = 0.0;
}

/* public methods */

void Decimator::process( float* sampleBuffer, int bufferLength )
{
    int i = 0;

    for ( i; i < bufferLength; ++i )
    {
        float sample = sampleBuffer[ i ];

        _count += _rate;

        if ( _count >= 1 )
        {
            _count -= 1;
            //sample  = ( long ) ( sample * _m ) / _m;
            sample = _m * floor( sample / _m + 0.5 );
        }
        sampleBuffer[ i ] = sample;
    }
}

/* getters / setters */

int Decimator::getBits()
{
    return _bits;
}

void Decimator::setBits( int value )
{
    _bits = value;
    _m    = 1 << ( _bits - 1 );
}

float Decimator::getRate()
{
    return _rate;
}

void Decimator::setRate( float value )
{
    _rate = value;
}
