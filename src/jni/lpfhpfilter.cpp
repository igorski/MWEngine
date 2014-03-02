#include "lpfhpfilter.h"
#include "global.h"
#include <math.h>

// constructor

LPFHPFilter::LPFHPFilter( float aLPCutoff, float aHPCutoff )
{
    setLPF( aLPCutoff, audio_engine::SAMPLE_RATE );
    setHPF( aHPCutoff, audio_engine::SAMPLE_RATE );

    _prevSample            = 0.0;
    _prevUnprocessedSample = _prevSample;
}

/* public methods */

void LPFHPFilter::setLPF( float aCutOffFrequency, int aSampleRate )
{
    float w = 2.0 * aSampleRate;
    float Norm;

    aCutOffFrequency *= 2.0 * ( atan( 1 ) * 4 );
    Norm              = 1.0 / ( aCutOffFrequency + w );
    b1                = ( w - aCutOffFrequency ) * Norm;
    a0                = a1 = aCutOffFrequency * Norm;
}

void LPFHPFilter::setHPF( float aCutOffFrequency, int aSampleRate )
{
    float w = 2.0 * aSampleRate;
    float Norm;

    aCutOffFrequency *= 2.0 *( atan( 1 ) * 4 );
    Norm              = 1.0 / ( aCutOffFrequency + w );
    a0                = w * Norm;
    a1                = -a0;
    b1                = ( w - aCutOffFrequency ) * Norm;
}

void LPFHPFilter::process( float* sampleBuffer, int bufferLength )
{
    int i = 0;

    for ( i; i < bufferLength; ++i )
    {
        float curUnprocessedSample = sampleBuffer[ i ];

        if ( i > 0 )
            _prevSample = sampleBuffer[ i - 1 ];

        sampleBuffer[ i ]      = curUnprocessedSample * a0 + _prevUnprocessedSample * a1 + _prevSample * b1;
        _prevUnprocessedSample = curUnprocessedSample;

//            out[n] = in[n]*a0 + in[n-1]*a1 + out[n-1]*b1;
    }
}
