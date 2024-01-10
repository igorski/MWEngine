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
#include "baseinstrument.h"
#include <audioengine.h>
#include <sequencer.h>
#include <utilities/eventutility.h>
#include <algorithm>

namespace MWEngine {

/* constructor / destructor */

BaseInstrument::BaseInstrument()
{
    construct();
}

BaseInstrument::~BaseInstrument()
{
    dispose();

    delete audioChannel;
    delete _audioEvents;
    delete _liveAudioEvents;

    audioChannel     = nullptr;
    _audioEvents     = nullptr;
    _liveAudioEvents = nullptr;
}

/* public methods */

void BaseInstrument::dispose()
{
    unregisterFromSequencer();
    clearEvents();
    clearMeasureCache();
}

bool BaseInstrument::hasEvents()
{
    return !_audioEvents->empty();
}

bool BaseInstrument::hasLiveEvents()
{
    return !_liveAudioEvents->empty();
}

std::vector<BaseAudioEvent*>* BaseInstrument::getEvents()
{
    return _audioEvents;
}

std::vector<BaseAudioEvent*>* BaseInstrument::getEventsForMeasure( int measureNum )
{
    return _audioEventsPerMeasure.size() <= measureNum ? nullptr : _audioEventsPerMeasure.at( measureNum );
}

std::vector<BaseAudioEvent*>* BaseInstrument::getLiveEvents()
{
    return _liveAudioEvents;
}

void BaseInstrument::updateEvents( float tempoRatio )
{
    // when updating to reflect changes in the instruments properties
    // or to update event properties responding to tempo changes
    // override this function in your derived class for custom implementations

    if ( tempoRatio == 1 ) {
        return;
    }

    // when tempo has updated, we update the offsets of all associated events
    // note the measure cache remains untouched (nothing changes with regards to
    // measure separation)

    // TODO: there is an issue that creeps in with regards to maintaining an accurate measure
    // cache, get(Start|End)MeasureForEvent relies on AudioEngine::samples_per_bar so the removal is
    // calculating for the new start/end measure range, therefore possibly missing the old (prior to
    // tempo change) range values. Additionally, we might risk undefined behaviour on the read locks
    // for an already locked mutex. We set a flag to prevent event add/remove changes (triggered by their
    // repositioning) and invoke a manual flush and recache after all sequenced events have been repositioned.

    _freezeEvents = true;

    size_t i = 0, total = _audioEvents->size();
    for ( ; i < total; ++i ) {
        _audioEvents->at( i )->repositionToTempoChange( tempoRatio );
    }

    _freezeEvents = false;
    clearMeasureCache();

    for ( i = 0; i < total; ++i ) {
        addEventToMeasureCache( _audioEvents->at( i ));
    };
}

void BaseInstrument::clearEvents()
{
    if ( _audioEvents != nullptr )
    {
        while ( !_audioEvents->empty() ) {
            removeEvent( _audioEvents->at( 0 ), false );
        }
    }

    if ( _liveAudioEvents != nullptr )
    {
        while ( !_liveAudioEvents->empty() ) {
            removeEvent( _liveAudioEvents->at( 0 ), true );
        }
    }
}

void BaseInstrument::addEvent( BaseAudioEvent* audioEvent, bool isLiveEvent )
{
    if ( _freezeEvents ) {
        return;
    }

    if ( isLiveEvent ) {
        _liveAudioEvents->push_back( audioEvent );
    } else {
        _audioEvents->push_back( audioEvent );
        addEventToMeasureCache( audioEvent );
    }
}

bool BaseInstrument::removeEvent( BaseAudioEvent* audioEvent, bool isLiveEvent )
{
    if ( _freezeEvents ) {
        return false;
    }

    bool removed = false;

    if ( audioEvent == nullptr || _liveAudioEvents == nullptr || _audioEvents == nullptr ) {
        return removed;
    }

    if ( isLiveEvent )
    {
        removed = EventUtility::removeEventFromVector( _liveAudioEvents, audioEvent );
        if ( removed ) {
            audioEvent->resetPlayState();
        }
    }
    else
    {
        removed = EventUtility::removeEventFromVector( _audioEvents, audioEvent );
        if ( removed ) {
            removeEventFromMeasureCache( audioEvent );
        }
    }
    if ( removed ) {
        audioEvent->onRemove();
    }
    return removed;
}

void BaseInstrument::registerInSequencer()
{
    index = Sequencer::registerInstrument( this );
}

void BaseInstrument::unregisterFromSequencer()
{
    Sequencer::unregisterInstrument( this );
    index = -1;
}

/* protected methods */

void BaseInstrument::construct()
{
    audioChannel = new AudioChannel( 1.F );

    // events

    _audioEvents     = new std::vector<BaseAudioEvent*>();
    _liveAudioEvents = new std::vector<BaseAudioEvent*>();

    // register instrument inside the sequencer

    registerInSequencer();
}

void BaseInstrument::addEventToMeasureCache( BaseAudioEvent* audioEvent )
{
    unsigned long startMeasureForEvent = EventUtility::getStartMeasureForEvent( audioEvent );
    unsigned long endMeasureForEvent   = EventUtility::getEndMeasureForEvent( audioEvent );

    for ( unsigned long i = startMeasureForEvent; i <= endMeasureForEvent; ++i ) {
        while ( _audioEventsPerMeasure.size() <= i ) {
            _audioEventsPerMeasure.push_back( new std::vector<BaseAudioEvent*>() );
        }
        _audioEventsPerMeasure.at( i )->push_back( audioEvent );
    }
}

void BaseInstrument::removeEventFromMeasureCache( BaseAudioEvent* audioEvent )
{
    unsigned long startMeasureForEvent     = EventUtility::getStartMeasureForEvent( audioEvent );
    unsigned long endMeasureForEvent       = EventUtility::getEndMeasureForEvent( audioEvent );
    unsigned long audioEventPerMeasureSize = _audioEventsPerMeasure.size();

    for ( size_t i = startMeasureForEvent; i <= endMeasureForEvent; ++i ) {
        if ( i >= audioEventPerMeasureSize ) {
            return;
        }
        auto eventVector = _audioEventsPerMeasure.at( i );
        EventUtility::removeEventFromVector( eventVector, audioEvent );
    }
}

void BaseInstrument::clearMeasureCache()
{
    size_t i = _audioEventsPerMeasure.size();
    while ( i-- > 0 ) {
        auto eventVector = _audioEventsPerMeasure.at( i );
        eventVector->clear();
        delete eventVector;
    }
    _audioEventsPerMeasure.clear();
}

} // E.O namespace MWEngine
