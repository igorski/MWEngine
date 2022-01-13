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
#include "druminstrument.h"
#include "../audioengine.h"
#include "../sequencer.h"
#include <algorithm>
#include <cstddef>
#include <vector>

namespace MWEngine {

/* constructor / destructor */

DrumInstrument::DrumInstrument()
{
    // construct() overridden here, must invoke explicitly
    construct();
}

DrumInstrument::~DrumInstrument()
{
    clearEvents();

    delete rOsc;
    delete drumPatterns;

    rOsc         = nullptr;
    drumPatterns = nullptr;
}

/* public methods */

bool DrumInstrument::hasEvents()
{
    if ( !drumPatterns->empty() ) {
        auto activePatternEvents = getEventsForActivePattern();
        return activePatternEvents != nullptr ? !activePatternEvents->empty() : false;
    }
    return false;
}

void DrumInstrument::updateEvents( float tempoRatio )
{
    for ( int i = 0, l = drumPatterns->size(); i < l; ++i ) {
        drumPatterns->at( i )->cacheEvents( drumTimbre );
    }
}

void DrumInstrument::clearEvents()
{
    if ( drumPatterns != nullptr )
    {
        for ( int i = 0, l = drumPatterns->size(); i < l; ++i )
        {
            DrumPattern* pattern = drumPatterns->at( i );
            pattern->clear();   // pool the pattern but lose the events
            //pattern->removeFromInstrument();
            //delete pattern;
        }
//        drumPatterns->clear();
    }
    activeDrumPattern = 0;
}

bool DrumInstrument::removeEvent( BaseAudioEvent* audioEvent, bool isLiveEvent )
{
    bool removed = false;

    if ( audioEvent != nullptr )
    {
        if ( !isLiveEvent )
        {
            // drum pattern store their AudioEvents in their own vectors (allows instant switching of patterns)

            std::vector<BaseAudioEvent*>* audioEvents = getEventsForActivePattern();

            if ( audioEvents == nullptr ) {
                return removed;
            }
            auto it = std::find( audioEvents->begin(), audioEvents->end(), audioEvent );

            if ( it != audioEvents->end())
            {
                audioEvents->erase( it );
                removed = true;
            }
        }
        else {
            removed = BaseInstrument::removeEvent( audioEvent, isLiveEvent );
        }

        if ( removed ) {
            delete audioEvent;
            audioEvent = nullptr;
        }
    }
    return removed;
}

std::vector<BaseAudioEvent*>* DrumInstrument::getEvents()
{
    return getEventsForActivePattern();
}

std::vector<BaseAudioEvent*>* DrumInstrument::getEventsForPattern( int patternNum )
{
    DrumPattern* pattern = getDrumPattern( patternNum );
    return pattern != nullptr ? pattern->audioEvents : nullptr;
}

std::vector<BaseAudioEvent*>* DrumInstrument::getEventsForActivePattern()
{
    return getEventsForPattern( activeDrumPattern );
}

DrumPattern* DrumInstrument::getDrumPattern( int patternNum )
{
    if ( patternNum >= drumPatterns->size() ) {
        return nullptr;
    }
    return drumPatterns->at( patternNum );
}

int DrumInstrument::setDrumPattern( DrumPattern* pattern )
{
    // make sure we don't allow double additions of the same pattern
    for ( int i = 0; i < drumPatterns->size(); i++ )
    {
        if ( drumPatterns->at( i ) == pattern )
            return i;
    }
    drumPatterns->push_back( pattern );
    return ( int ) ( drumPatterns->size() ) - 1; // return index of pattern
}

/* protected methods */

void DrumInstrument::construct()
{
    drumTimbre        = DrumTimbres::LIGHT;
    rOsc              = nullptr;// new RouteableOscillator();  // currently unused...
    audioChannel      = new AudioChannel( 1.0, AudioEngine::samples_per_bar );

    activeDrumPattern = 0;
    drumPatterns      = new std::vector<DrumPattern*>();

    // unlike the base instruments, drum instrument events come from the registered patterns

    _audioEvents      = nullptr;
    _liveAudioEvents  = new std::vector<BaseAudioEvent*>();

    registerInSequencer(); // auto-register instrument inside the sequencer
}

} // E.O namespace MWEngine
