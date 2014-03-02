#include "waveshaper.h"
#include <cmath>

// constructor

WaveShaper::WaveShaper( float amount, float level )
{
    _amount = amount;
    _level  = level;
}

/* public methods */

void WaveShaper::process( float* sampleBuffer, int sampleLength )
{
    int i = 0;

    for ( i; i < sampleLength; ++i )
    {
        float input = sampleBuffer[ i ];
        input = ( input * ( std::abs( input ) + _amount ) / (( int ) input ^ 2 ) + ( _amount - 1 ) * std::abs( input ) + 1 );
        sampleBuffer[ i ] = input * _level;
    }
}

/* getters / setters */

float WaveShaper::getAmount()
{
    return _amount;
}

void WaveShaper::setAmount( float value )
{
    _amount = value;
}

float WaveShaper::getLevel()
{
    return _level;
}

void WaveShaper::setLevel( float value )
{
    _level = value;
}
