#include "routeable_oscillator.h"
#include "global.h"
#include "utils.h"

// constructor

RouteableOscillator::RouteableOscillator()
{
    wave           = WaveForms::SINE_WAVE;
    speed          = 5;  // in Hz
    destination    = -1; // is enumeration

    _oscillator    = new LFO(); // pre-create > Object pooling
    _hasOscillator = false;
}

RouteableOscillator::~RouteableOscillator()
{
    delete _oscillator;
}

/* public methods */

/**
 * as we're now using Object pooing and the Routeable Oscillator
 * (currently) has no switchable targets, we can use the link- and
 * unlink methods to toggle the oscillated effect on and off
 */
void RouteableOscillator::linkOscillator()
{
    _hasOscillator = true;
}

void RouteableOscillator::unlinkOscillator()
{
    _hasOscillator = false;
}

bool RouteableOscillator::isLinked()
{
    return _hasOscillator;
}

LFO* RouteableOscillator::getLinkedOscillator()
{
    return _oscillator;
}
