#include "drumevent.h"
#include "global.h"
#include "samplemanager.h"
#include <cstdlib>

/* constructor / destructor */

/**
 * initializes an DrumEvent with very definitive properties to be pre-cached
 * for use in a sequencer context
 *
 * @param aPosition    {int} the step position in the sequencer
 * @param aDrumType    {int} the PercussionType to synthesize
 * @param aInstrument  {DrumInstrument*} the instruments properties
 */
DrumEvent::DrumEvent( int aPosition, int aDrumType, DrumInstrument* aInstrument )
{
    _inited = false;

    position = aPosition;

    setType  ( aDrumType );
    setTimbre( aInstrument->drumTimbre );

    init();

    _inited = true;
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

    if ( _inited )
    {
        if ( !_locked )
            updateSample();
        else
            _updateAfterUnlock = true;
    }
}

int DrumEvent::getType()
{
    return _type;
}

void DrumEvent::setType( int aType )
{
    _type = aType;

    if ( _inited )
    {
        if ( !_locked )
            updateSample();
        else
            _updateAfterUnlock = true;
    }
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

            if ( _timbre == DrumSynthTimbres::GRAVEL )
                smp = "kdg";
            else
                smp = "kd";
            break;

        case PercussionTypes::STICK:

            if ( _timbre == DrumSynthTimbres::GRAVEL )
                smp = "stg";
            else
                smp = "st";
            break;

        case PercussionTypes::SNARE:

            if ( _timbre == DrumSynthTimbres::GRAVEL )
                smp = "sng";
            else
                smp = "sn";
            break;

        case PercussionTypes::HI_HAT:

            if ( _timbre == DrumSynthTimbres::GRAVEL )
                smp = "hhg";
            else
                smp = "hh";
            break;
    }
    setSample( SampleManager::getSample( smp ), SampleManager::getSampleLength( smp ));
}
