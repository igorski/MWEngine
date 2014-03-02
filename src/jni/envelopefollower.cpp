#include "envelopefollower.h"
#include <cmath>

// constructor

EnvelopeFollower::EnvelopeFollower( float maxGain, float attackMs, float releaseMs, int sampleRate )
{
    envelope = 0.0;

    _attack  = pow( 0.01, maxGain / ( attackMs  * sampleRate * 0.001 ));
    _release = pow( 0.01, maxGain / ( releaseMs * sampleRate * 0.001 ));
}

/* public methods */

void EnvelopeFollower::process( float src, int skip )
{
    float v = std::abs( src );
    src += skip;

    if ( v > envelope )
        envelope = _attack * ( envelope - v ) + v;
    else
        envelope = _release * ( envelope - v ) + v;
}
