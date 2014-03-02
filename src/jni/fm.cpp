#include "fm.h"
#include <math.h>
#include "utils.h"

// constructor

FrequencyModulator::FrequencyModulator( int aWaveForm, float aRate )
{
    _wave          = aWaveForm;
    _phase         = 0.0;
    _phaseIncr     = 0.0;
    _length        = 0;
    _buffer        = BufferUtil::generateSilentBuffer( BufferUtil::calculateBufferLength( MIN_LFO_RATE ));
    modulator      = 0.0;
    carrier        = 0.0;
    fmamp          = 10;
    AMP_MULTIPLIER = 0.15;
    TWO_PI         = 2 * ( atan( 1 ) * 4 );

    setRate( aRate );
}

/* public methods */

void FrequencyModulator::process( float* sampleBuffer, int bufferLength )
{
    for ( int i = 0; i < bufferLength; ++i )
    {
        modulator = modulator + ( TWO_PI_OVER_SR * _rate );
        modulator = modulator < TWO_PI ? modulator : modulator - TWO_PI;

        carrier           = sampleBuffer[ i ];
        sampleBuffer[ i ] = ( carrier * cos( carrier + fmamp * cos( modulator )))/* * AMP_MULTIPLIER*/;
    }
}
