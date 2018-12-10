/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2018 Igor Zinken - http://www.igorski.nl
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
#include <cstddef>
#include <utilities/utils.h>
#include <events/sampleevent.h>

/* constructor / destructor */

SampledInstrument::SampledInstrument()
{
    // construct() called by BaseInstrument constructor
}

SampledInstrument::~SampledInstrument()
{
    // nowt... see BaseInstrument
}

/* public methods */

void SampledInstrument::updateEvents()
{
    // for a SampledInstrument we don't update the events length range
    // as the length cannot exceed the bufferSize of its sample

    if ( _oldTempo != AudioEngine::tempo ) {

        // when tempo has updated, we update the offsets of all associated events

        float ratio = _oldTempo / AudioEngine::tempo;

        for ( int i = 0, l = _audioEvents->size(); i < l; ++i )
        {
            SampleEvent* event = ( SampleEvent* ) _audioEvents->at( i );
            event->setEventStart(( int )(( float ) event->getEventStart() * ratio ));
            event->setEventEnd( event->getEventStart() + event->getOriginalEventLength() );
        }
        _oldTempo = AudioEngine::tempo;
    }
}
