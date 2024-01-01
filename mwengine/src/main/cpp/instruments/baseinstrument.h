/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2024 Igor Zinken - https://www.igorski.nl
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
#ifndef __MWENGINE__BASEINSTRUMENT_H_INCLUDED__
#define __MWENGINE__BASEINSTRUMENT_H_INCLUDED__

#include "../audiochannel.h"
#include <events/baseaudioevent.h>

namespace MWEngine {
class BaseInstrument
{
    public:
        BaseInstrument();
        virtual ~BaseInstrument();

        void dispose(); // invoke when removing the Instrument from the engine

        virtual bool hasEvents();     // whether the instrument has events to sequence
        virtual bool hasLiveEvents(); // whether the instruments has events to synthesize on the fly
        virtual void updateEvents( float tempoRatio = 1 );  // updates all associated events after changing instrument properties or tempo change

        virtual std::vector<BaseAudioEvent*>* getEvents();
        virtual std::vector<BaseAudioEvent*>* getEventsForMeasure( int measureNum );
        virtual std::vector<BaseAudioEvent*>* getLiveEvents();

        virtual void clearEvents();

        // internal to the engine
        // addition and removal of events is performed when the Sequencer is collecting audible
        // content for the engine to render. By managing this through the Sequencer we omit the
        // need for thread locks or mutexes when changing event lists during rendering

#ifndef SWIG
        void addEvent( BaseAudioEvent* audioEvent, bool isLiveEvent );
        bool removeEvent( BaseAudioEvent* audioEvent, bool isLiveEvent );
#endif

        void registerInSequencer();
        void unregisterFromSequencer();

        AudioChannel *audioChannel;
        int index;  // index in the Sequencers instrument Vector

    protected:
        virtual void construct();

        std::vector<BaseAudioEvent*>* _audioEvents;
        std::vector<BaseAudioEvent*>* _liveAudioEvents;

        // a vector that indexes all sequenced events by measure for easy lookup by the sequencer
        std::vector<std::vector<BaseAudioEvent*>*> _audioEventsPerMeasure;

        bool _freezeEvents = false;

        void clearMeasureCache();
        void addEventToMeasureCache( BaseAudioEvent* audioEvent );
        void removeEventFromMeasureCache( BaseAudioEvent* audioEvent );
};
} // E.O namespace MWEngine

#endif
