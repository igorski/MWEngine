#include "bitcrusher.h"
#include "utils.h"

/* constructor */

BitCrusher::BitCrusher( float amount, float level )
{
    _levelCorrection = 1;

    setAmount( amount );
    setLevel( level );
}

/* public methods */

void BitCrusher::process( float* sampleBuffer, int bufferLength )
{
    int i;

    for ( i = 0; i < bufferLength; ++i )
    {
        short input = ( short ) ( sampleBuffer[ i ] * 32767 );
        short prevent_offset = ( short )( -1 >> _bits + 1 );
        input &= ( -1 << ( 16 - _bits ));
        sampleBuffer[ i ] = ((( input + prevent_offset ) * _levelCorrection ) / 32767 );
    }
}

/* getters / setters */

float BitCrusher::getAmount()
{
    return _amount;
}

void BitCrusher::setAmount( float value )
{
    _amount = value;

    // scale float to 1 - 16 bit range
    _bits = ( int ) floor( scale( value, 1, 15 )) + 1;
    setLevel( _level );
}

float BitCrusher::getLevel()
{
    return _level;
}

void BitCrusher::setLevel( float value )
{
    _level = value;

    // at lower bit resolutions the sound goes through the ceiling, auto-correct the volume
    _levelCorrection = ( _bits < 2 ) ? _level * .35 : _level;
}
