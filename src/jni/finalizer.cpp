#include "finalizer.h"

// constructor

/**
 * Finalizer should be seen as the "final" processor in a typical setting, it is essentially
 * a limiter making sure no clipping occurs, as well as a preventive measure to omit
 * nasty pops and clicks caused by DC offsets
 *
 * @param attackMs   {float} attack time in milliseconds
 * @param releaseMs  {float} attack decay time in milliseconds
 * @param sampleRate {int} the current samplerate
 */
Finalizer::Finalizer( float attackMs, float releaseMs, int sampleRate )
{
    init( attackMs, releaseMs, sampleRate );

    lastSample  = 0;
    lastDSample = 0.0;
}

/* public methods */

short* Finalizer::process( float* sampleBuffer, int bufferLength )
{
    _output = Limiter::process( sampleBuffer, bufferLength );

    int i = 0;

    for ( i; i < bufferLength; ++i )
    {
        short theSample = ( short ) ( 0.996 * ( lastSample + _output[ i ] - lastSample ));
        lastSample = theSample;

        _output[ i ] = theSample;
    }
    return _output;
}

/**
 * used by the AudioBouncer, which doesn't need shorts!
 * @param sampleBuffer {float*} buffer containing samples to limit
 * @return {float*} limited buffer
 */
void Finalizer::limitfloats( float* sampleBuffer, int bufferLength )
{
    Limiter::limitfloats( sampleBuffer, bufferLength );

    int i = 0;

    for ( i; i < bufferLength; ++i )
    {
        float theSample = 0.996 * ( lastDSample + sampleBuffer[ i ] - lastDSample );
        lastDSample = theSample;

        // extreme limiting (still above the thresholds?)
        if ( theSample < -1.0 )
            theSample = -1.0;

        else if ( theSample > +1.0 )
            theSample = +1.0;

        sampleBuffer[ i ] = theSample;
    }
}
