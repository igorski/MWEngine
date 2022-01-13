/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2022 Igor Zinken - https://www.igorski.nl
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
#include <mutex>

namespace MWEngine {
class BaseInstrument
{
    public:
        BaseInstrument();
        virtual ~BaseInstrument();

        virtual bool hasEvents();     // whether the instrument has events to sequence
        virtual bool hasLiveEvents(); // whether the instruments has events to synthesize on the fly
        virtual void updateEvents( float tempoRatio = 1 );  // updates all associated events after changing instrument properties or tempo change

        virtual std::vector<BaseAudioEvent*>* getEvents();
        virtual std::vector<BaseAudioEvent*>* getEventsForMeasure( int measureNum );
        virtual std::vector<BaseAudioEvent*>* getLiveEvents();

        virtual void clearEvents();
        virtual void addEvent( BaseAudioEvent* audioEvent, bool isLiveEvent );
        virtual bool removeEvent( BaseAudioEvent* audioEvent, bool isLiveEvent );

        void toggleReadLock( bool lock );
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

        // mutex to lock event vector mutations
        std::mutex* _lock;
        bool _locked       = false;
        bool _freezeEvents = false;

        void clearMeasureCache();
        void addEventToMeasureCache( BaseAudioEvent* audioEvent );
        void removeEventFromMeasureCache( BaseAudioEvent* audioEvent );
};
} // E.O namespace MWEngine

#endif
