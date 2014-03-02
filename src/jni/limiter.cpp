#include "limiter.h"
#include "global.h"

// constructor / destructor

Limiter::Limiter()
{
    init( 10, 500, audio_engine::SAMPLE_RATE );
}

Limiter::~Limiter()
{
    delete _follower;
    delete _output;
}

/**
 * creates a limiter, attack 10 ms works well with decay 500 ms for average use
 *
 * @param attackMs   {float} attack time in milliseconds
 * @param releaseMs  {float} attack decay time in milliseconds
 * @param sampleRate {int} the current samplerate
 */
Limiter::Limiter( float attackMs, float releaseMs, int sampleRate )
{
    init( attackMs, releaseMs, sampleRate );
}

/* public methods */

float Limiter::getLinearGR()
{
    return _follower->envelope > 1. ? 1 / _follower->envelope : 1.;
}

short* Limiter::process( float* sampleBuffer, int bufferLength )
{
    // I don't want the left and right sides to get scaled differently, that would be silly.
    // So I call Process() once on my interleaved stereo data with twice the number of
    // frames, and skip = 1.

    _output = new short[ bufferLength ];

    // 32767 being the max value for a short
    int i = bufferLength;

    while ( i-- > 0 )
    {
        float dest = sampleBuffer[ i ];

        _follower->process( dest, skip );

        if ( _follower->envelope > maxGain )
            dest = dest / _follower->envelope;

        // extreme limiting (still above the thresholds?)
        if ( dest < -1.0 )
            dest = -1.0;

        else if ( dest  > +1.0 )
            dest = +1.0;

        _output[ i ] = ( short ) (( dest + skip ) * 32767 );
    }
    return _output;
}

/**
 * used by the AudioBouncer, which doesn't need shorts!
 * @param sampleBuffer {float[]} buffer containing samples to limit
 * @return {float[]} limited buffer
 */
void Limiter::limitfloats( float* sampleBuffer, int bufferLength )
{
    int i = bufferLength;

    while ( i-- > 0 )
    {
        float dest = sampleBuffer[ i ];

        _follower->process( dest, skip );

        if ( _follower->envelope > maxGain )
            dest = dest / _follower->envelope;

        sampleBuffer[ i ] = ( dest + skip );
    }
}

/* protected methods */

void Limiter::init( float attackMs, float releaseMs, int sampleRate )
{
    maxGain   = .85;
    _follower = new EnvelopeFollower( maxGain, attackMs, releaseMs, sampleRate );
    _output   = new short[ audio_engine::BUFFER_SIZE ];
    skip      = 0; // channel amount minus 1 apparently
}
