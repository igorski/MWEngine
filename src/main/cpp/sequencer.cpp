/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2020 Igor Zinken - https://www.igorski.nl
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
#include "sequencer.h"
#include "audioengine.h"
#include <utilities/utils.h>
#include <vector>

namespace MWEngine {

/* static member intialization */

bool Sequencer::playing           = false;
BulkCacher* Sequencer::bulkCacher = new BulkCacher( true );
std::vector<BaseInstrument*> Sequencer::instruments;

/* public methods */

int Sequencer::registerInstrument( BaseInstrument* instrument )
{
    int index       = -1;
    bool wasPresent = false; // prevent double addition

    for ( int i = 0; i < instruments.size(); i++ )
    {
        if ( instruments.at( i ) == instrument )
            wasPresent = true;
    }

    if ( !wasPresent ) {
        instruments.push_back( instrument );
        index = instruments.size() - 1;
    }
    return index; // the index this instrument is registered at
}

bool Sequencer::unregisterInstrument( BaseInstrument* instrument )
{
    for ( int i = 0; i < instruments.size(); i++ )
    {
        if ( instruments.at( i ) == instrument )
        {
            instruments.erase( instruments.begin() + i );
            return true;
        }
    }
    return false;
}

bool Sequencer::getAudioEvents( std::vector<AudioChannel*>* channels, int bufferPosition,
                                int bufferSize, bool addLiveInstruments, bool flushChannels )
{
    channels->clear();

    int bufferEnd    = bufferPosition + ( bufferSize - 1 );          // the highest SampleEnd value we'll query
    bool loopStarted = bufferEnd > AudioEngine::max_buffer_position; // whether this request exceeds the min_buffer_position - max_buffer_position range

    size_t total = instruments.size();

    // note we update the channels mix properties here as they might change during playback

    for ( size_t i = 0; i < total; ++i )
    {
        BaseInstrument* instrument      = instruments.at( i );
        AudioChannel* instrumentChannel = instrument->audioChannel;

        // clear previous channel contents when requested
        if ( flushChannels ) {
            instrumentChannel->reset();
        }

        if ( !instrumentChannel->muted )
        {
            if ( playing )
            {
                int firstMeasure = ( int ) floor(( float ) bufferPosition / ( float ) AudioEngine::samples_per_bar );
                int lastMeasure  = ( int ) floor(( float ) bufferEnd / ( float ) AudioEngine::samples_per_bar );

                collectSequencedEvents( instrument, bufferPosition, bufferEnd, firstMeasure );

                if ( lastMeasure != firstMeasure ) {
                    // TODO ensure we don't add duplicates (see audiochannel.cpp)
                    collectSequencedEvents( instrument, bufferPosition, bufferEnd, lastMeasure );
                }
            }

            if ( addLiveInstruments && instrument->hasLiveEvents() )
                collectLiveEvents( instrument );

            channels->push_back( instrumentChannel );
        }
    }
    return loopStarted;
}

void Sequencer::updateEvents()
{
    for ( int i = 0, l = ( int ) instruments.size(); i < l; ++i ) {
        instruments.at( i )->updateEvents();
    }
}

void Sequencer::clearEvents()
{
    for ( int i = 0, l = ( int ) instruments.size(); i < l; ++i ) {
        instruments.at( i )->clearEvents();
    }
}

/**
 * used by the getAudioEvents-method of the sequencer, this validates
 * the present AudioEvents against the requested position
 * and updates and flushes the removal queue
 *
 * @param instrument     {BaseInstrument*} instrument to gather events from
 * @param bufferPosition {int} the current buffers start pointer
 * @param bufferEnd      {int} the current buffers end pointer
 * @param measure        {int} the current measure to look up events for
 */
void Sequencer::collectSequencedEvents( BaseInstrument* instrument, int bufferPosition, int bufferEnd, int measure )
{
    if ( !instrument->hasEvents() ) {
        return;
    }

    AudioChannel* channel = instrument->audioChannel;

    instrument->toggleReadLock( true ); // lock the events vector while sequencing

    auto audioEvents = instrument->getEventsForMeasure( measure );

    if ( audioEvents == nullptr ) {
        return;
    }

    // removal queue
    std::vector<BaseAudioEvent*> removes;

    // channel has an internal loop (e.g. drum machine) ? recalculate requested
    // buffer position by subtracting all measures above the first

    if ( channel->maxBufferPosition > 0 )
    {
        int samplesPerBar = AudioEngine::samples_per_bar;

        while ( bufferPosition >= channel->maxBufferPosition )
        {
            bufferPosition -= samplesPerBar;
            bufferEnd      -= samplesPerBar;
        }
    }

    size_t i = 0;
    size_t total = audioEvents->size();

    for ( ; i < total; i++ )
    {
        BaseAudioEvent* audioEvent = audioEvents->at( i );

        if ( audioEvent->isEnabled() )
        {
            int eventStart = audioEvent->getEventStart();
            int eventEnd   = audioEvent->getEventEnd();

            if (( eventStart >= bufferPosition && eventStart <= bufferEnd ) ||
                ( eventStart <  bufferPosition && eventEnd >= bufferPosition ))
            {
                if ( !audioEvent->isDeletable())
                    channel->addEvent( audioEvent );
                else
                    removes.push_back( audioEvent );
            }
        }
    }

    instrument->toggleReadLock( false ); // release mutex

    // removal queue filled ? process it so we can safely
    // remove "deleted" AudioEvents without errors occurring
    if ( !removes.empty() )
    {
        total = removes.size();

        for ( i = 0; i < total; i++ )
        {
            BaseAudioEvent* audioEvent = removes[ i ];
            instrument->removeEvent( audioEvent, false );
        }
    }
}

void Sequencer::collectLiveEvents( BaseInstrument* instrument )
{
    AudioChannel* channel = instrument->audioChannel;

    instrument->toggleReadLock( true ); // lock the events vector while sequencing
    std::vector<BaseAudioEvent*>* liveEvents = instrument->getLiveEvents();

    // removal queue
    std::vector<BaseAudioEvent*> removes;

    size_t i = 0;
    size_t total = liveEvents->size();

    for ( ; i < total; i++ )
    {
        BaseAudioEvent* audioEvent = liveEvents->at( i );

        if ( !audioEvent->isDeletable())
            channel->addLiveEvent( audioEvent );
        else
            removes.push_back( audioEvent );
    }

    instrument->toggleReadLock( false ); // release mutex

    // removal queue filled ? process it so we can safely
    // remove "deleted" AudioEvents without errors occurring
    if ( !removes.empty() )
    {
        total = removes.size();

        for ( i = 0; i < total; i++ )
        {
            BaseAudioEvent* audioEvent = removes[ i ];
            instrument->removeEvent( audioEvent, true );
        }
    }
}

/**
 * used by the cacheAudioEventsForMeasure-method, this collects
 * all AudioEvents in the requested measure for entry into the BulkCacher
 *
 * @param bufferPosition {int} the desired measures buffers start pointer
 * @param bufferEnd      {int} the desired measures buffers end pointer
 *
 * @return {std::vector<BaseCacheableAudioEvent*>}
 */
std::vector<BaseCacheableAudioEvent*>* Sequencer::collectCacheableSequencerEvents( int bufferPosition, int bufferEnd )
{
    auto* events = new std::vector<BaseCacheableAudioEvent*>();

    for ( int i = 0, l = ( int ) instruments.size(); i < l; ++i )
    {
        std::vector<BaseAudioEvent*>* audioEvents = instruments.at( i )->getEvents();
        for ( int j = 0; j < audioEvents->size(); j++ )
        {
            BaseAudioEvent* audioEvent = audioEvents->at( j );

            // if event is an instance of BaseCacheableAudioEvent add it to the list
            if ( dynamic_cast<BaseCacheableAudioEvent*>( audioEvent ) != nullptr )
            {
                int eventStart = audioEvent->getEventStart();
                int eventEnd   = audioEvent->getEventEnd();

                if (( eventStart >= bufferPosition && eventStart <= bufferEnd ) ||
                    ( eventStart <  bufferPosition && eventEnd >= bufferPosition ))
                {
                    if ( !audioEvent->isDeletable())
                        events->push_back(( BaseCacheableAudioEvent* ) audioEvent );
                }
            }
        }
    }
    return events;
}

} // E.O namespace MWEngine
