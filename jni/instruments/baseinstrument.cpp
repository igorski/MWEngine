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
#include "baseinstrument.h"
#include "../sequencer.h"

/* constructor / destructor */

BaseInstrument::BaseInstrument()
{

}

BaseInstrument::~BaseInstrument()
{
    unregisterFromSequencer();
    delete audioChannel;
}

/* public methods */

bool BaseInstrument::hasEvents()
{
    return false;   // override in derived class
}

bool BaseInstrument::hasLiveEvents()
{
    return false;   // override in derived class
}

void BaseInstrument::updateEvents()
{
    // override in derived class
}

void BaseInstrument::clearEvents()
{
    // override in derived class
}

bool BaseInstrument::removeEvent( BaseAudioEvent* aEvent )
{
    return false;   // override in derived class
}

void BaseInstrument::registerInSequencer()
{
    bool wasPresent = false; // prevent double addition

    for ( int i = 0; i < sequencer::instruments.size(); i++ )
    {
        if ( sequencer::instruments.at( i ) == this )
            wasPresent = true;
    }

    if ( !wasPresent )
        sequencer::instruments.push_back( this );
}

void BaseInstrument::unregisterFromSequencer()
{
    for ( int i = 0; i < sequencer::instruments.size(); i++ )
    {
        if ( sequencer::instruments.at( i ) == this )
        {
            sequencer::instruments.erase( sequencer::instruments.begin() + i );
            break;
        }
    }
}

std::vector<BaseAudioEvent*>* BaseInstrument::getEvents()
{
    return 0; // override in derived class
}

std::vector<BaseAudioEvent*>* BaseInstrument::getLiveEvents()
{
    return 0; // override in derived class
}
