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
#include <utilities/debug.h>
#include <instruments/druminstrument.h>

/* constructor / destructor */

DrumPattern::DrumPattern( int aNum, BaseInstrument* aInstrument )
{
    num         = aNum;
    eventAmount = 0;

    audioEvents  = new std::vector<BaseAudioEvent*>();

    kickPattern  = new int[ AMOUNT_OF_STEPS ];
    snarePattern = new int[ AMOUNT_OF_STEPS ];
    stickPattern = new int[ AMOUNT_OF_STEPS ];
    hatPattern   = new int[ AMOUNT_OF_STEPS ];

    _instrument  = aInstrument;

    clear();    // acts as an init
}

DrumPattern::~DrumPattern()
{
    Debug::log( "DrumPattern::destroy num %d", num );

    destroyAudioEvents();
    delete audioEvents;

    removeFromInstrument();

    delete[] kickPattern;
    delete[] snarePattern;
    delete[] stickPattern;
    delete[] hatPattern;
}

/* public methods */

void DrumPattern::addToInstrument()
{
    // adds the pattern to the sequencer so its contents can be heard

    (( DrumInstrument* ) _instrument )->setDrumPattern( this );
}

void DrumPattern::removeFromInstrument()
{
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
}

int* DrumPattern::getKickPattern()
{
    return kickPattern;
}

void DrumPattern::setKickPattern( int* aDrumPattern, int arrayLength )
{
    for ( unsigned int i = 0; i < arrayLength; ++i )
        kickPattern[ i ] = aDrumPattern[ i ];
}

int* DrumPattern::getSnarePattern()
{
    return snarePattern;
}

void DrumPattern::setSnarePattern( int* aDrumPattern, int arrayLength )
{
    for ( unsigned int i = 0; i < arrayLength; ++i )
        snarePattern[ i ] = aDrumPattern[ i ];
}

int* DrumPattern::getStickPattern()
{
    return stickPattern;
}

void DrumPattern::setStickPattern( int* aDrumPattern, int arrayLength )
{
    for ( unsigned int i = 0; i < arrayLength; ++i )
        stickPattern[ i ] = aDrumPattern[ i ];
}

int* DrumPattern::getHatPattern()
{
    return hatPattern;
}

void DrumPattern::setHatPattern( int* aDrumPattern, int arrayLength )
{
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

    for ( unsigned int i = 0; i < AMOUNT_OF_STEPS; ++i )
    {
        if ( kickPattern[ i ] == EVENT_ON )
            addDrumEvent( i, PercussionTypes::KICK_808, aDrumTimbre );

        if ( snarePattern[ i ] == EVENT_ON )
            addDrumEvent( i, PercussionTypes::SNARE, aDrumTimbre );

        if ( stickPattern[ i ] == EVENT_ON )
            addDrumEvent( i, PercussionTypes::STICK, aDrumTimbre );

        if ( hatPattern[ i ] == EVENT_ON )
            addDrumEvent( i, PercussionTypes::HI_HAT, aDrumTimbre );
    }
}

void DrumPattern::addDrumEvent( int aPosition, int aDrumType, int aDrumTimbre )
{
    audioEvents->push_back( new DrumEvent( aPosition, aDrumType, aDrumTimbre, _instrument ));
    eventAmount = audioEvents->size();
}

void DrumPattern::removeDrumEvent( int aPosition, int aType )
{
    int removed = 0;

    for ( int i = 0; i < audioEvents->size(); i++ )
    {
        DrumEvent* vo = (( DrumEvent* ) audioEvents->at( i ));

        if ( vo->position == aPosition && vo->getType() == aType )
        {
            vo->setDeletable( true ); // actual removal is invoked by the sequencer
            removed = 1;
            break;
        }
    }
    eventAmount = audioEvents->size() - removed;
}

bool DrumPattern::hasContent()
{
    for ( int i = 0; i < AMOUNT_OF_STEPS; ++i )
    {
        if ( kickPattern [ i ] == EVENT_ON ||
             snarePattern[ i ] == EVENT_ON ||
             stickPattern[ i ] == EVENT_ON ||
             hatPattern  [ i ] == EVENT_ON )
         {
             return true;
         }
    }
    return false;
}

void DrumPattern::clear()
{
    destroyAudioEvents();

    // XOX-style... 0 = off, 1 = on

    for ( int i = 0; i < AMOUNT_OF_STEPS; ++i )
    {
        kickPattern [ i ] = EVENT_OFF;
        snarePattern[ i ] = EVENT_OFF;
        stickPattern[ i ] = EVENT_OFF;
        hatPattern  [ i ] = EVENT_OFF;
    }
}

/* private methods */

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