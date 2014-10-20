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
#include "sequencer.h"
#include "audioengine.h"
#include "utils.h"
#include <algorithm>
#include <vector>

namespace sequencer
{
    std::vector<BaseInstrument*> instruments;
    BulkCacher* bulkCacher = new BulkCacher( true );

    /* public methods */

    std::vector<AudioChannel*> getAudioEvents( std::vector<AudioChannel*> channels, int bufferPosition,
                                               int bufferEnd, bool addLiveInstruments )
    {
        // clear previous channel contents (note we don't delete the channels anywhere as we re-use them)
        channels.clear();

        int i, l;

        // 1. the sequenced synthesizers, note we update their mix properties here as they might change during playback

        for ( i = 0, l = instruments.size(); i < l; ++i )
        {
            BaseInstrument* instrument      = instruments.at( i );
            AudioChannel* instrumentChannel = instrument->audioChannel;

            instrumentChannel->reset();
            instrumentChannel->mixVolume = instrument->volume;

            if ( !instrumentChannel->muted )
            {
                collectSequencedEvents( instrument, bufferPosition, bufferEnd );

                if ( addLiveInstruments && instrument->hasLiveEvents() )
                    collectLiveEvents( instrument );

                channels.push_back( instrumentChannel );
            }
        }
        return channels;
    }

    void updateEvents()
    {
        for ( int i = 0, l = instruments.size(); i < l; ++i )
            instruments.at( i )->updateEvents();
    }

    void clearEvents()
    {
        for ( int i = 0, l = instruments.size(); i < l; ++i )
            instruments.at( i )->clearEvents();
    }

    /**
     * used by the getAudioEvents-method of the sequencer, this validates
     * the present AudioEvents against the requested position
     * and updates and flushes the removal queue
     *
     * @param instrument     {BaseInstrument*} instrument to gather events from
     * @param bufferPosition {int} the current buffers start pointer
     * @param bufferEnd      {int} the current buffers end pointer
     */
    void collectSequencedEvents( BaseInstrument* instrument, int bufferPosition, int bufferEnd )
    {
        if ( !instrument->hasEvents() )
            return;

        AudioChannel* channel                     = instrument->audioChannel;
        std::vector<BaseAudioEvent*>* audioEvents = instrument->getEvents();

        // removal queue
        std::vector<BaseAudioEvent*> removes;

        // channel has an internal loop (e.g. drum machine) ? recalculate requested
        // buffer position by subtracting all measures above the first
        if ( channel->maxBufferPosition > 0 )
        {
            int bytesPerBar = AudioEngine::bytes_per_bar;

            while ( bufferPosition >= channel->maxBufferPosition )
            {
                bufferPosition -= bytesPerBar;
                bufferEnd      -= bytesPerBar;
            }
        }

        int i = 0, amount = audioEvents->size();
        for ( i; i < amount; i++ )
        {
            BaseAudioEvent* audioEvent = audioEvents->at( i );

            if ( audioEvent->isEnabled() )
            {
                int sampleStart = audioEvent->getSampleStart();
                int sampleEnd   = audioEvent->getSampleEnd();

                if ( audioEvent->getLoopeable() ||
                   ( sampleStart >= bufferPosition && sampleStart <= bufferEnd ) ||
                   ( sampleStart <  bufferPosition && sampleEnd >= bufferPosition ))
                {
                    if ( !audioEvent->deletable())
                        channel->addEvent( audioEvent );
                    else
                        removes.push_back( audioEvent );
                }
            }
        }
        // removal queue filled ? process it so we can safely
        // remove "deleted" AudioEvents without errors occurring
        if ( removes.size() > 0 )
        {
            int i = 0;
            for ( i; i < removes.size(); i++ )
            {
                BaseAudioEvent* audioEvent = removes[ i ];

                // remove audio event from the list
                if ( std::find( audioEvents->begin(), audioEvents->end(), audioEvent ) != audioEvents->end())
                {
                    audioEvents->erase( std::find( audioEvents->begin(), audioEvents->end(), audioEvent ));
                }
                instrument->removeEvent( audioEvent );
            }
        }
    }

    void collectLiveEvents( BaseInstrument* instrument )
    {
        AudioChannel* channel                    = instrument->audioChannel;
        std::vector<BaseAudioEvent*>* liveEvents = instrument->getLiveEvents();

        // removal queue
        std::vector<BaseAudioEvent*> removes;

        int i = 0;
        for ( i; i < liveEvents->size(); i++ )
        {
            BaseAudioEvent* audioEvent = liveEvents->at( i );

            if ( !audioEvent->deletable())
                channel->addLiveEvent( audioEvent );
            else
                removes.push_back( audioEvent );
        }
        // removal queue filled ? process it so we can safely
        // remove "deleted" AudioEvents without errors occurring
        if ( removes.size() > 0 )
        {
            int i = 0;
            for ( i; i < removes.size(); i++ )
            {
                BaseAudioEvent* audioEvent = removes[ i ];

                // remove audio event from the list
                if ( std::find( liveEvents->begin(), liveEvents->end(), audioEvent ) != liveEvents->end())
                {
                    liveEvents->erase( std::find( liveEvents->begin(), liveEvents->end(), audioEvent ));
                }
                instrument->removeEvent( audioEvent );
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
    std::vector<BaseCacheableAudioEvent*>* collectCacheableSequencerEvents( int bufferPosition, int bufferEnd )
    {
        std::vector<BaseCacheableAudioEvent*>* events = new std::vector<BaseCacheableAudioEvent*>();

        for ( int i = 0, l = instruments.size(); i < l; ++i )
        {
            std::vector<BaseAudioEvent*>* audioEvents = instruments.at( i )->getEvents();
            int amount = audioEvents->size();

            for ( int j = 0; j < amount; j++ )
            {
                BaseAudioEvent* audioEvent = audioEvents->at( j );

                // if event is an instance of BaseCacheableAudioEvent add it to the list
                if ( dynamic_cast<BaseCacheableAudioEvent*>( audioEvent ) != NULL )
                {
                    int sampleStart = audioEvent->getSampleStart();
                    int sampleEnd   = audioEvent->getSampleEnd();

                    if (( sampleStart >= bufferPosition && sampleStart <= bufferEnd ) ||
                        ( sampleStart <  bufferPosition && sampleEnd >= bufferPosition ))
                    {
                        if ( !audioEvent->deletable())
                            events->push_back(( BaseCacheableAudioEvent* ) audioEvent );
                    }
                }
            }
        }
        return events;
    }
}
