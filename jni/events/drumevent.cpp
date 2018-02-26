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
#include "drumevent.h"
#include "../audioengine.h"
#include "../global.h"
#include "../utilities/samplemanager.h"
#include <cstdlib>

/* constructor / destructor */

/**
 * initializes an DrumEvent with very definitive properties to be pre-cached
 * for use in a sequencer context
 *
 * @param aPosition    {int} the step position in the sequencer
 * @param aDrumType    {int} the PercussionType to synthesize
 * @param aDrumTimbre  {int} the drum machine's timbre
 * @param aInstrument  {BaseInstrument*} the DrumInstrument the event corresponds to
 */
DrumEvent::DrumEvent( int aPosition, int aDrumType, int aDrumTimbre, BaseInstrument* aInstrument )
{
    construct();
    init( aInstrument );

    position    = aPosition;
    _eventStart = position * ( int ) AudioEngine::samples_per_step;

    setType  ( aDrumType );
    setTimbre( aDrumTimbre );

    updateSample();
}

DrumEvent::~DrumEvent()
{

}

/* public methods */

int DrumEvent::getTimbre()
{
    return _timbre;
}

void DrumEvent::setTimbre( int aTimbre )
{
    _timbre = aTimbre;

    if ( !_locked )
        updateSample();
    else
        _updateAfterUnlock = true;
}

int DrumEvent::getType()
{
    return _type;
}

void DrumEvent::setType( int aType )
{
    _type = aType;

    if ( !_locked )
        updateSample();
    else
        _updateAfterUnlock = true;
}

void DrumEvent::unlock()
{
    _locked = false;

    if ( _updateAfterUnlock )
        updateSample();

    _updateAfterUnlock = false;
}

/* private methods */

void DrumEvent::updateSample()
{
    std::string smp;

    switch ( _type )
    {
        case PercussionTypes::KICK_808:

            if ( _timbre == DrumTimbres::GRAVEL )
                smp = "kdg";
            else
                smp = "kd";
            break;

        case PercussionTypes::STICK:

            if ( _timbre == DrumTimbres::GRAVEL )
                smp = "stg";
            else
                smp = "st";
            break;

        case PercussionTypes::SNARE:

            if ( _timbre == DrumTimbres::GRAVEL )
                smp = "sng";
            else
                smp = "sn";
            break;

        case PercussionTypes::HI_HAT:

            if ( _timbre == DrumTimbres::GRAVEL )
                smp = "hhg";
            else
                smp = "hh";
            break;
    }
    setSample( SampleManager::getSample( smp ));
}
