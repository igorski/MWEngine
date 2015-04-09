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
#include "synthevent.h"
#include "../audioengine.h"
#include "../sequencer.h"
#include "../global.h"
#include <utilities/bufferpool.h>
#include <utilities/bufferutility.h>
#include <cmath>

/* constructor / destructor */

/**
 * initializes an SynthEvent for use in a sequenced context
 *
 * @param aFrequency   {int}    the frequency of the note to synthesize in Hz
 * @param aPosition    {int}    the step position in the sequencer
 * @param aLength      {float} the length in steps the note lasts
 * @param aInstrument  {SynthInstrument} the instruments properties
 */
SynthEvent::SynthEvent( float aFrequency, int aPosition, float aLength,
                        SynthInstrument* aInstrument )
{
    init( aInstrument, aFrequency, aPosition, aLength, true );
}

/**
 * initializes an SynthEvent to be synthesized at once, for a live instrument context
 *
 * @param aFrequency  {int} the frequency of the note to synthesize in Hz
 * @param aInstrument {SynthInstrument}
 */
SynthEvent::SynthEvent( float aFrequency, SynthInstrument* aInstrument )
{
    init( aInstrument, aFrequency, 0, 1, false );
}

SynthEvent::~SynthEvent()
{
    BufferPool::destroyRingBuffersForEvent( this ); // see synthesizer.h
}
