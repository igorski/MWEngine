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
#ifndef __DRUMINSTRUMENT_H_INCLUDED__
#define __DRUMINSTRUMENT_H_INCLUDED__

#include "baseinstrument.h"
#include "../audiochannel.h"
#include "../drumpattern.h"
#include <events/baseaudioevent.h>
#include <modules/routeableoscillator.h>
#include <vector>

class DrumInstrument : public BaseInstrument
{
    public:
        DrumInstrument();
        ~DrumInstrument();

        std::vector<BaseAudioEvent*>* getEvents();
        std::vector<BaseAudioEvent*>* getLiveEvents();
        std::vector<BaseAudioEvent*>* getEventsForPattern( int patternNum );
        std::vector<BaseAudioEvent*>* getEventsForActivePattern();

        int drumTimbre;

        std::vector<DrumPattern*>*    drumPatterns;
        std::vector<BaseAudioEvent*>* liveAudioEvents;
        int activeDrumPattern;

        RouteableOscillator *rOsc;

        // base class overrides
        // note we don't override 'removeEvents' as DrumPatterns manage the events

        bool hasEvents();
        bool hasLiveEvents();
        void updateEvents();
        void clearEvents();
        bool removeEvent( BaseAudioEvent* aEvent );
        DrumPattern* getDrumPattern( int patternNum );
        int setDrumPattern( DrumPattern* pattern );

};

#endif
