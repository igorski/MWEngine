/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2020 Igor Zinken - https://www.igorski.nl
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
#include "../audioengine.h"
#include "../sequencer.h"
#include <drivers/adapter.h>
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
    unregisterFromSequencer();
    clearEvents();
    clearMeasureCache();

    delete audioChannel;
    delete _audioEvents;
    delete _liveAudioEvents;

    audioChannel     = nullptr;
    _audioEvents     = nullptr;
    _liveAudioEvents = nullptr;
}

/* public methods */

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

void BaseInstrument::updateEvents()
{
    // when updating to reflect changes in the instruments propertes
    // or to update event properties responding to tempo changes
    // override this function in your derived class for custom implementations

    if ( _oldTempo != AudioEngine::tempo ) {

        //std::lock_guard<std::mutex> guard( _lock );
        toggleReadLock( true );

        // when tempo has updated, we update the offsets of all associated events
        // note the measure cache remains untouched (nothing changes with regards to
        // measure separation)

        float ratio = _oldTempo / AudioEngine::tempo;

        for ( int i = 0, l = _audioEvents->size(); i < l; ++i )
        {
            BaseAudioEvent* event = _audioEvents->at( i );

            auto orgStart  = ( float ) event->getEventStart();
            auto orgEnd    = ( float ) event->getEventEnd();
            auto orgLength = ( float ) event->getEventLength();

            event->setEventStart ( orgStart  * ratio );
            event->setEventLength( orgLength * ratio );
            event->setEventEnd   (( orgEnd + 1 ) * ratio ); // add 1 to correct for rounding of float
        }
        _oldTempo = AudioEngine::tempo;

        toggleReadLock( false );
    }
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
    //std::lock_guard<std::mutex> guard( _lock );
    toggleReadLock( true );

    if ( isLiveEvent ) {
        _liveAudioEvents->push_back( audioEvent );
    } else {
        _audioEvents->push_back( audioEvent );
        addEventToMeasureCache( audioEvent );
    }
    toggleReadLock( false );
}

bool BaseInstrument::removeEvent( BaseAudioEvent* audioEvent, bool isLiveEvent )
{
    bool removed = false;

    if ( audioEvent == nullptr || _liveAudioEvents == nullptr || _audioEvents == nullptr ) {
        return removed;
    }

    //std::lock_guard<std::mutex> guard( _lock );
    toggleReadLock( true );

    if ( isLiveEvent )
    {
        auto it = std::find( _liveAudioEvents->begin(), _liveAudioEvents->end(), audioEvent );
        if ( it != _liveAudioEvents->end() )
        {
            _liveAudioEvents->erase( it );
            audioEvent->resetPlayState();
            removed = true;
        }
    }
    else
    {
        auto it = std::find( _audioEvents->begin(), _audioEvents->end(), audioEvent );
        if ( it != _audioEvents->end()) {
            _audioEvents->erase( it );
        }
        removeEventFromMeasureCache( audioEvent );
        removed = true;
    }
    toggleReadLock( false );

    return removed;
}

void BaseInstrument::registerInSequencer()
{
    index     = Sequencer::registerInstrument( this );
    _oldTempo = AudioEngine::tempo;
}

void BaseInstrument::unregisterFromSequencer()
{
    Sequencer::unregisterInstrument( this );
    index = -1;
}

void BaseInstrument::toggleReadLock( bool locked )
{
    // when unit testing, GoogleTest deadlocks on this attempted locking operation. We don't
    // need to test for mutex behaviour, but it would be nice not to have to wrap this code

    if ( DriverAdapter::isMocked() ) {
        return;
    }

    if ( locked ) {
        _lock.lock();
    } else {
        _lock.unlock();
    }
}

/* protected methods */

void BaseInstrument::construct()
{
    audioChannel = new AudioChannel( 1.0 );

    // events

    _audioEvents     = new std::vector<BaseAudioEvent*>();
    _liveAudioEvents = new std::vector<BaseAudioEvent*>();

    // register instrument inside the sequencer

    registerInSequencer();
}

void BaseInstrument::addEventToMeasureCache( BaseAudioEvent* audioEvent )
{
    int startMeasureForEvent = EventUtility::getStartMeasureForEvent( audioEvent );
    int endMeasureForEvent   = EventUtility::getEndMeasureForEvent( audioEvent );

    for ( size_t i = startMeasureForEvent; i <= endMeasureForEvent; ++i ) {
        while ( _audioEventsPerMeasure.size() <= i ) {
            _audioEventsPerMeasure.push_back( new std::vector<BaseAudioEvent*>() );
        }
        _audioEventsPerMeasure.at( i )->push_back( audioEvent );
    }
}

void BaseInstrument::removeEventFromMeasureCache( BaseAudioEvent* audioEvent )
{
    int startMeasureForEvent = EventUtility::getStartMeasureForEvent( audioEvent );
    int endMeasureForEvent   = EventUtility::getEndMeasureForEvent( audioEvent );

    for ( size_t i = startMeasureForEvent; i < endMeasureForEvent; ++i ) {
        auto eventVector = _audioEventsPerMeasure.at( i );
        auto it = std::find( eventVector->begin(), eventVector->end(), audioEvent );
        if ( it != eventVector->end()) {
            eventVector->erase( it );
        }
    }
}

void BaseInstrument::clearMeasureCache()
{
    for ( size_t i = 0; i < _audioEventsPerMeasure.size(); ++i ) {
        auto eventVector = _audioEventsPerMeasure.at( i );
        eventVector->clear();
        delete eventVector;
    }
    _audioEventsPerMeasure.clear();
}

} // E.O namespace MWEngine
