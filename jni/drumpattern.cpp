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
#include "drumpattern.h"

#include "global.h"
#include "utils.h"
#include <instruments/druminstrument.h>

/* constructor / destructor */

DrumPattern::DrumPattern( int aNum, BaseInstrument* aInstrument )
{
    num         = aNum;
    eventAmount = 0;

    audioEvents  = new std::vector<BaseAudioEvent*>();

    kickPattern  = new int[ 0 ];
    snarePattern = new int[ 0 ];
    stickPattern = new int[ 0 ];
    hatPattern   = new int[ 0 ];

    kickPatternLength  = 0;
    snarePatternLength = 0;
    stickPatternLength = 0;
    hatPatternLength   = 0;
    _instrument        = aInstrument;

    (( DrumInstrument* ) _instrument )->drumPatterns->push_back( this );  // add the pattern to the sequencer so it can be heard
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
        (( DrumEvent* ) audioEvents->at( i ))->setTimbre( newTimbre );
    }
}

/**
 * pre-caches all patterns stored in
 * the int* Arrays into the DrumEvent vector
 *
 * @param {int} aDrumTimbre the drum instrument's timbre
 */
void DrumPattern::cacheEvents( int aDrumTimbre )
{
    // clear previous vector contents
    destroyAudioEvents();

    int i;

    for ( i = 0; i < kickPatternLength; ++i )
        addDrumEvent( kickPattern[ i ], PercussionTypes::KICK_808, aDrumTimbre );

    for ( i = 0; i < snarePatternLength; ++i )
        addDrumEvent( snarePattern[ i ], PercussionTypes::SNARE, aDrumTimbre );

    for ( i = 0; i < stickPatternLength; ++i )
        addDrumEvent( stickPattern[ i ], PercussionTypes::STICK, aDrumTimbre );

    for ( i = 0; i < hatPatternLength; ++i )
        addDrumEvent( hatPattern[ i ], PercussionTypes::HI_HAT, aDrumTimbre );
}

void DrumPattern::addDrumEvent( int aPosition, int aDrumType, int aDrumTimbre )
{
    audioEvents->push_back( new DrumEvent( aPosition, aDrumType, aDrumTimbre, _instrument ));
    eventAmount = audioEvents->size();
}

void DrumPattern::removeDrumEvent( int aPosition, int aType )
{
    int i = 0;
    for ( i; i < audioEvents->size(); i++ )
    {
        DrumEvent* vo = (( DrumEvent* ) audioEvents->at( i ));

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

    // remove pattern from sequence

    DrumInstrument* instrument = (( DrumInstrument* ) _instrument );

    int i = 0;
    for ( i; i < instrument->drumPatterns->size(); i++ )
    {
        if ( instrument->drumPatterns->at( i ) == this )
        {
            instrument->drumPatterns->erase( instrument->drumPatterns->begin() + i );
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
        BaseAudioEvent* vo = audioEvents->at( i );
        delete vo;
    }
    audioEvents->clear();
    eventAmount = 0;
}