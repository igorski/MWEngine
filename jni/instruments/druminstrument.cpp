/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2014 Igor Zinken - http://www.igorski.nl
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
#include "../utils.h"
#include <cstddef>
#include <vector>

/* constructor / destructor */

DrumInstrument::DrumInstrument()
{
    volume            = .5;
    drumTimbre        = DrumTimbres::LIGHT;
    rOsc              = 0;// new RouteableOscillator();  // currently unused...
    audioChannel      = new AudioChannel( volume, AudioEngine::bytes_per_bar );

    activeDrumPattern = 0;
    drumPatterns      = new std::vector<DrumPattern*>();

    registerInSequencer(); // auto-register instrument inside the sequencer
}

DrumInstrument::~DrumInstrument()
{
    DebugTool::log( "DrumInstrument::DESTRUCT" );

    delete rOsc;
    delete[] drumPatterns;
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

bool DrumInstrument::removeEvent( BaseAudioEvent* audioEvent )
{
    if ( audioEvent != 0 )
    {
        delete audioEvent;
        audioEvent = 0;
        return true;
    }
    return false;
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
