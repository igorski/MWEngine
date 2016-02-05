/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2016 Igor Zinken - http://www.igorski.nl
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

/* constructor / destructor */

DrumInstrument::DrumInstrument()
{
    construct();
}

DrumInstrument::~DrumInstrument()
{
    delete rOsc;
    delete[] drumPatterns;

    while ( _liveAudioEvents->size() > 0 )
        delete _liveAudioEvents->back();
}

/* public methods */

bool DrumInstrument::hasEvents()
{
    if ( drumPatterns->size() > 0 )
        return getEventsForActivePattern()->size() > 0;

    return false;
}

void DrumInstrument::updateEvents()
{
    for ( int i = 0, l = drumPatterns->size(); i < l; ++i )
        drumPatterns->at( i )->cacheEvents( drumTimbre );
}

void DrumInstrument::clearEvents()
{
    if ( drumPatterns != 0 )
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

    if ( audioEvent != 0 )
    {
        if ( !isLiveEvent )
        {
            // drum pattern store their AudioEvents in their own vectors (allows instant switching of patterns)

            std::vector<BaseAudioEvent*>* audioEvents = getEventsForActivePattern();

            if ( std::find( audioEvents->begin(), audioEvents->end(), audioEvent ) != audioEvents->end())
            {
                audioEvents->erase( std::find( audioEvents->begin(), audioEvents->end(), audioEvent ));
                audioEvent->removeFromSequencer(); // updates event state to not-added-to-sequencer
                removed = true;
            }
        }
        else {
            removed = BaseInstrument::removeEvent( audioEvent, isLiveEvent );
        }

        if ( removed ) {
            delete audioEvent;
            audioEvent = 0;
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
    DrumPattern* pattern = drumPatterns->at( patternNum );
    return pattern->audioEvents;
}

std::vector<BaseAudioEvent*>* DrumInstrument::getEventsForActivePattern()
{
    return getEventsForPattern( activeDrumPattern );
}

DrumPattern* DrumInstrument::getDrumPattern( int patternNum )
{
    return drumPatterns->at( patternNum );
}

int DrumInstrument::setDrumPattern( DrumPattern* pattern )
{
    // make sure we don't allow double additions of the same pattern
    for ( int i; i < drumPatterns->size(); i++ )
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
    volume            = MAX_PHASE / 2;
    drumTimbre        = DrumTimbres::LIGHT;
    rOsc              = 0;// new RouteableOscillator();  // currently unused...
    audioChannel      = new AudioChannel( volume, AudioEngine::samples_per_bar );

    activeDrumPattern = 0;
    drumPatterns      = new std::vector<DrumPattern*>();
    _liveAudioEvents  = new std::vector<BaseAudioEvent*>();

    registerInSequencer(); // auto-register instrument inside the sequencer
}
