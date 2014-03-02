#include "drumpattern.h"

#include "global.h"
#include "utils.h"

/* constructor / destructor */

DrumPattern::DrumPattern( int aNum )
{
    num         = aNum;
    eventAmount = 0;

    audioEvents  = new std::vector<DrumEvent*>();

    kickPattern  = new int[ 0 ];
    snarePattern = new int[ 0 ];
    stickPattern = new int[ 0 ];
    hatPattern   = new int[ 0 ];

    kickPatternLength  = 0;
    snarePatternLength = 0;
    stickPatternLength = 0;
    hatPatternLength   = 0;

    sequencer::drumPatterns.push_back( this );  // add the pattern to the sequencer so it can be heard
}

DrumPattern::~DrumPattern()
{
    destroy();
}

/* public methods */

int* DrumPattern::getKickPattern()
{
    return kickPattern;
}

void DrumPattern::setKickPattern( int* aDrumPattern, int arrayLength )
{
    kickPatternLength = arrayLength;

    delete[] kickPattern;

    kickPattern = new int[ arrayLength ];

    for ( unsigned int i = 0; i < arrayLength; ++i )
        kickPattern[ i ] = aDrumPattern[ i ];
}

int* DrumPattern::getSnarePattern()
{
    return snarePattern;
}

void DrumPattern::setSnarePattern( int* aDrumPattern, int arrayLength )
{
    snarePatternLength = arrayLength;
    delete[] snarePattern;

    snarePattern = new int[ arrayLength ];

    for ( unsigned int i = 0; i < arrayLength; ++i )
        snarePattern[ i ] = aDrumPattern[ i ];
}

int* DrumPattern::getStickPattern()
{
    return stickPattern;
}

void DrumPattern::setStickPattern( int* aDrumPattern, int arrayLength )
{
    stickPatternLength = arrayLength;

    delete[] stickPattern;

    stickPattern = new int[ arrayLength ];

    for ( unsigned int i = 0; i < arrayLength; ++i )
        stickPattern[ i ] = aDrumPattern[ i ];
}

int* DrumPattern::getHatPattern()
{
    return hatPattern;
}

void DrumPattern::setHatPattern( int* aDrumPattern, int arrayLength )
{
    hatPatternLength = arrayLength;

    delete[] hatPattern;

    hatPattern = new int[ arrayLength ];

    for ( unsigned int i = 0; i < arrayLength; ++i )
        hatPattern[ i ] = aDrumPattern[ i ];
}

void DrumPattern::updateTimbre( int newTimbre )
{
    int i = 0;
    for ( i; i < audioEvents->size(); i++ )
    {
        DrumEvent* vo = audioEvents->at( i );
        vo->setTimbre( newTimbre );
    }
}

/**
 * pre-caches all patterns stored in
 * the int* Arrays into the DrumEvent vector
 *
 * @param aInstrument DrumInstrument} the drum instrument
 */
void DrumPattern::cacheEvents( DrumInstrument *aInstrument )
{
    // clear previous vector contents
    destroyAudioEvents();

    int i;

    for ( i = 0; i < kickPatternLength; ++i )
        addDrumEvent( kickPattern[ i ], PercussionTypes::KICK_808, aInstrument );

    for ( i = 0; i < snarePatternLength; ++i )
        addDrumEvent( snarePattern[ i ], PercussionTypes::SNARE, aInstrument );

    for ( i = 0; i < stickPatternLength; ++i )
        addDrumEvent( stickPattern[ i ], PercussionTypes::STICK, aInstrument );

    for ( i = 0; i < hatPatternLength; ++i )
        addDrumEvent( hatPattern[ i ], PercussionTypes::HI_HAT, aInstrument );
}

void DrumPattern::addDrumEvent( int aPosition, int aDrumType, DrumInstrument* aInstrument )
{
    audioEvents->push_back( new DrumEvent( aPosition, aDrumType, aInstrument ));
    eventAmount = audioEvents->size();
}

void DrumPattern::removeDrumEvent( int aPosition, int aType )
{
    int i = 0;
    for ( i; i < audioEvents->size(); i++ )
    {
        DrumEvent* vo = audioEvents->at( i );

        if ( vo->position == aPosition && vo->getType() == aType )
        {
            vo->setDeletable( true );
            audioEvents->erase( audioEvents->begin() + i );
            delete vo;
            break;
        }
    }
    eventAmount = audioEvents->size();
}

void DrumPattern::destroy()
{
    DebugTool::log( "DrumPattern::destroy num %d", num );

    // remove pattern from sequencer
    int i = 0;
    for ( i; i < sequencer::drumPatterns.size(); i++ )
    {
        if ( sequencer::drumPatterns[ i ] == this )
        {
            sequencer::drumPatterns.erase( sequencer::drumPatterns.begin() + i );
            break;
        }
    }
    destroyAudioEvents();
    delete audioEvents;

    delete[] kickPattern;
    delete[] snarePattern;
    delete[] stickPattern;
    delete[] hatPattern;
}

void DrumPattern::destroyAudioEvents()
{
    int i = 0;
    for ( i; i < audioEvents->size(); i++ )
    {
        DrumEvent* vo = audioEvents->at( i );
        delete vo;
    }
    audioEvents->clear();
    eventAmount = 0;
}