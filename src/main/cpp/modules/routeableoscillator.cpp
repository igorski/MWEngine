/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2014 Igor Zinken - http://www.igorski.nl
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "global.h"
#include <definitions/waveforms.h>
#include <modules/routeableoscillator.h>
#include <utilities/utils.h>

// constructor

RouteableOscillator::RouteableOscillator()
{
    wave           = WaveForms::SINE;
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
