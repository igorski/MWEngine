/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2015 Igor Zinken - http://www.igorski.nl
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
#include "sampledinstrument.h"
#include "../audioengine.h"
#include "../global.h"
#include "../sequencer.h"
#include "../utilities/utils.h"
#include <cstddef>
#include <events/sampleevent.h>

/* constructor / destructor */

SampledInstrument::SampledInstrument()
{
    construct();
}

SampledInstrument::~SampledInstrument()
{
    // when using JNI, we let SWIG invoke destructors when Java references are finalized
    // otherwise we delete and dispose the events directly from this instrument
#ifndef USE_JNI
    while ( _audioEvents->size() > 0 )
        delete _audioEvents->back();

    while ( _liveAudioEvents->size() > 0 )
        delete _liveAudioEvents->back();
#endif
}

/* public methods */

bool SampledInstrument::removeEvent( BaseAudioEvent* audioEvent, bool isLiveEvent )
{
    bool removed = false;

    if ( audioEvent != 0 )
    {
        removed = BaseInstrument::removeEvent( audioEvent, isLiveEvent );
#ifndef USE_JNI
        delete audioEvent;
        audioEvent = 0;
#endif
    }
    return removed;
}
