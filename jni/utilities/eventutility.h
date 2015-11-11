/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Igor Zinken - http://www.igorski.nl
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
#ifndef __EVENT_UTILITY_H_INCLUDED__
#define __EVENT_UTILITY_H_INCLUDED__

#include "global.h"
#include <events/baseaudioevent.h>

/**
 * EventUtility provides a convenient hook to
 * position AudioEvents at the sample level but
 * by using musical concepts for timing
 */
class EventUtility
{
    public:

        // position an AudioEvent by using musical timing concepts
        // NOTE : this results in strict "on the grid" timing, using buffer samples instead allows for more
        // accurate positioning for drifting / swing / early / late events
        //
        // audioEvent is a pointer to the AudioEvent to mutate
        // startMeasure describes which measure the event belongs to (NOTE : first measure starts at 0)
        // subdivisions describes the amount of "steps" in each measure (e.g. 16 for 16 steps within a single measure)
        // offset describes the offset at which to position the event, this is a subset of the amount of subdivisons
        //
        // examples :
        // ( audioEvent, 0, 16, 4 ) positions audioEvent at 4 / 16 = start of the 2nd quaver in the first measure
        // ( audioEvent, 1, 32, 4 ) positions audioEvent at 4 / 32 = 1/8th note in the second measure
        static void positionEvent( BaseAudioEvent* audioEvent, int startMeasure, int subdivisions, int offset );
};

#endif