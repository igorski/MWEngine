/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2019 Igor Zinken - http://www.igorski.nl
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
    return _audioEvents->size() > 0;
}

bool BaseInstrument::hasLiveEvents()
{
    return _liveAudioEvents->size() > 0;
}

std::vector<BaseAudioEvent*>* BaseInstrument::getEvents()
{
    return _audioEvents;
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

        float ratio = _oldTempo / AudioEngine::tempo;

        for ( int i = 0, l = _audioEvents->size(); i < l; ++i )
        {
            BaseAudioEvent* event = _audioEvents->at( i );

            float orgStart  = ( float ) event->getEventStart();
            float orgEnd    = ( float ) event->getEventEnd();
            float orgLength = ( float ) event->getEventLength();

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
        while ( _audioEvents->size() > 0 )
            removeEvent( _audioEvents->at( 0 ), false );
    }

    if ( _liveAudioEvents != nullptr )
    {
        while ( _liveAudioEvents->size() > 0 )
            removeEvent( _liveAudioEvents->at( 0 ), true );
    }
}

void BaseInstrument::addEvent( BaseAudioEvent* audioEvent, bool isLiveEvent ) {
    //std::lock_guard<std::mutex> guard( _lock );
    toggleReadLock( true );

    if ( isLiveEvent ) {
        _liveAudioEvents->push_back( audioEvent );
    } else {
        _audioEvents->push_back( audioEvent );
    }
    toggleReadLock( false );
}

bool BaseInstrument::removeEvent( BaseAudioEvent* audioEvent, bool isLiveEvent )
{
    bool removed = false;

    if ( audioEvent == nullptr )
        return removed;

    toggleReadLock( true );

    if ( isLiveEvent )
    {
        if ( std::find( _liveAudioEvents->begin(), _liveAudioEvents->end(), audioEvent ) != _liveAudioEvents->end())
        {
            _liveAudioEvents->erase( std::find( _liveAudioEvents->begin(), _liveAudioEvents->end(), audioEvent ));
            removed = true;
        }
    }
    else if ( std::find( _audioEvents->begin(), _audioEvents->end(), audioEvent ) != _audioEvents->end())
    {
        _audioEvents->erase( std::find( _audioEvents->begin(), _audioEvents->end(), audioEvent ));
        removed = true;
    }
// let's not do the below as management of event allocation isn't the instruments problem.
//#ifndef USE_JNI
//    if ( removed ) {
//
//        // when using JNI, we let SWIG invoke destructors when Java references are finalized
//        // otherwise we delete and dispose the events directly from this instrument
//
//        delete audioEvent;
//        audioEvent = nullptr;
//    }
//#endif
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

} // E.O namespace MWEngine
