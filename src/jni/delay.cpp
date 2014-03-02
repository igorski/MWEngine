#include "delay.h"
#include "global.h"
#include "utils.h"
#include <math.h>

/* constructor / destructor */

/**
 * @param aDelayTime    {float} in milliseconds, time between consecutive repeats
 * @param aMaxDelayTime {float} in milliseconds, the maximum value we're likely to expect
                                 in case delay time will fluctuate in the application
 * @param aMix          {float} 0-1, percentage of dry/wet mix
 * @param aFeedback     {float} 0-1, amount of repeats
 */
Delay::Delay( float aDelayTime, float aMaxDelayTime, float aMix, float aFeedback )
{
    _time        = ( int ) round(( audio_engine::SAMPLE_RATE * .001 ) * aDelayTime );
    _maxTime     = ( int ) round(( audio_engine::SAMPLE_RATE * .001 ) * aMaxDelayTime );
    _delayBuffer = BufferUtil::generateSilentBuffer( _maxTime );
    _mix         = aMix;
    _feedback    = aFeedback;

     // TODO: make stereo ? ( twin buffers )

    _delayIndex = 0;
}

Delay::~Delay()
{
    delete _delayBuffer;
}

/* public methods */

/**
 * run an audio signal through the delay line
 * and add the delay effect
 * @param sampleBuffer {float[]}
 */
void Delay::apply( float* sampleBuffer, int bufferLength )
{
    float delay;
    int readIndex, i;

    for ( i = 0; i < bufferLength; ++i )
    {
        readIndex = _delayIndex - _time + 1;

        if( readIndex < 0 )
            readIndex += _time;

        // read the previously delayed samples from the buffer
        // ( for feedback purposes ) and append the current sample to it

        delay = _delayBuffer[ readIndex ];

        _delayBuffer[ _delayIndex ] = sampleBuffer[ i ] + delay * _feedback;

        if( ++_delayIndex == _time )
            _delayIndex = 0;

        // higher feedback levels can cause a massive noisefest, "limit" them!
        if ( _feedback > .5 )
            sampleBuffer[ i ] += ( delay * _mix * ( 1.5 - _feedback ));
        else
            sampleBuffer[ i ] += ( delay * _mix );
    }
}

/**
 * clears existing buffer contents
 */
void Delay::reset()
{
    if ( _delayBuffer != 0 )
    {
        for ( int i = 0; i < _maxTime; ++i )
            _delayBuffer[ i ] = 0.0;
    }
}

/* getters / setters */

float Delay::getDelayTime()
{
    return _time / ( audio_engine::SAMPLE_RATE * .001 );
}

void Delay::setDelayTime( float aValue )
{
    _time = ( int ) round(( audio_engine::SAMPLE_RATE * .001 ) * aValue );

    if ( _delayIndex > _time )
        _delayIndex = 0;
}

float Delay::getMix()
{
    return _mix;
}

void Delay::setMix( float aValue )
{
    _mix = aValue;
}

float Delay::getFeedback()
{
    return _feedback;
}

void Delay::setFeedback( float aValue )
{
    _feedback = aValue;
}
