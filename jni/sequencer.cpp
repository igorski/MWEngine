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
    std::vector<SynthInstrument*> synthesizers;
    std::vector<SampledInstrument*> samplers;
    DrumInstrument* drummachine;

    std::vector<DrumPattern*> drumPatterns;

    int activeDrumPattern  = 0;
    BulkCacher* bulkCacher = new BulkCacher( true ); // sequential to spare CPU sources

    std::vector<AudioChannel*> getAudioEvents( std::vector<AudioChannel*> channels, int bufferPosition,
                                               int bufferEnd, bool addLiveInstruments )
    {
        // clear previous channel contents (note we don't delete the channels anywhere as we re-use them)
        channels.clear();

        int i, l;

        // 1. the sequenced synthesizers, note we update their mix properties here as they might change during playback

        for ( i = 0, l = sequencer::synthesizers.size(); i < l; ++i )
        {
            SynthInstrument* synthesizer = sequencer::synthesizers.at( i );
            AudioChannel* synthChannel   = synthesizer->audioChannel;

            synthChannel->reset();
            synthChannel->mixVolume = synthesizer->volume;
            collectSequencedEvents( synthChannel, synthesizer->audioEvents, bufferPosition, bufferEnd );

            // the live keyboard
            if ( addLiveInstruments )
                collectLiveEvents( synthChannel, synthesizer->liveEvents );

            channels.push_back( synthChannel );
        }

        // 2. the samplers

        for ( i = 0, l = sequencer::samplers.size(); i < l; ++i )
        {
            SampledInstrument* sampler   = sequencer::samplers.at( i );
            AudioChannel* samplerChannel = sampler->audioChannel;

            samplerChannel->reset();

            if ( !samplerChannel->muted )
            {
                collectSequencerSamplerEvents( samplerChannel, sampler->audioEvents, bufferPosition, bufferEnd );
                channels.push_back( samplerChannel );
            }
        }

        // 3. drum machine, note we update its properties here as they might change during playback

        if ( sequencer::drummachine != 0 )
        {
            AudioChannel* drumChannel = sequencer::drummachine->audioChannel;

            drumChannel->reset();
            drumChannel->mixVolume         = sequencer::drummachine->volume;
            drumChannel->maxBufferPosition = AudioEngine::bytes_per_bar;
            collectDrumEvents( drumChannel, bufferPosition, bufferEnd );

            channels.push_back( drumChannel );
        }
        return channels;
    }

    void clearEvents()
    {
        for ( int i = 0, l = sequencer::synthesizers.size(); i < l; ++i )
        {
            SynthInstrument* synthesizer = sequencer::synthesizers.at( i );

            if ( synthesizer->audioEvents != 0 )
                synthesizer->audioEvents->clear();

            if ( synthesizer->liveEvents != 0 )
                synthesizer->liveEvents->clear();
        }
        sequencer::drumPatterns.clear();
        sequencer::activeDrumPattern = 0;
    }

    /**
     * used by the getAudioEvents-method of the sequencer, this validates
     * the present AudioEvents against the requested position
     * and updates and flushes the removal queue
     *
     * @param channel        {AudioChannel} AudioChannel to append events to
     * @param audioEvents    {std::vector<BaseCacheableAudioEvent*>*} audioEvents to query
     * @param bufferPosition {int} the current buffers start pointer
     * @param bufferEnd      {int} the current buffers end pointer
     */
    void collectSequencedEvents( AudioChannel *channel, std::vector<BaseCacheableAudioEvent*>* audioEvents, int bufferPosition, int bufferEnd )
    {
        // removal queue
        std::vector<BaseCacheableAudioEvent*> removes;

        int i = 0;
        int amount = audioEvents->size();
        for ( i; i < amount; i++ )
        {
            BaseCacheableAudioEvent* audioEvent = audioEvents->at( i );

            int sampleStart = audioEvent->getSampleStart();
            int sampleEnd   = audioEvent->getSampleEnd();

            if (( sampleStart >= bufferPosition && sampleStart <= bufferEnd ) ||
                ( sampleStart < bufferPosition  && sampleEnd >= bufferPosition ))
            {
                if ( !audioEvent->deletable())
                    channel->addEvent( audioEvent );
                else
                    removes.push_back( audioEvent );
            }
        }
        // removal queue filled ? process it so we can safely
        // remove "deleted" AudioEvents without errors occurring
        if ( removes.size() > 0 )
        {
            int i = 0;
            for ( i; i < removes.size(); i++ )
            {
                BaseCacheableAudioEvent* audioEvent = removes[ i ];

                // remove audio event from the list
                if ( std::find( audioEvents->begin(), audioEvents->end(), audioEvent ) != audioEvents->end())
                {
                    audioEvents->erase( std::find( audioEvents->begin(), audioEvents->end(), audioEvent ));
                }
#ifndef USE_JNI
                // when using JNI, we let SWIG invoke destructors when Java references are finalized
                delete audioEvent;
                audioEvent = 0;
#endif
            }
        }
    }

    void collectLiveEvents( AudioChannel *channel, std::vector<BaseAudioEvent*>* liveEvents )
    {
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
#ifndef USE_JNI
                // when using JNI, we let SWIG invoke destructors when Java references are finalized
                delete audioEvent;
                audioEvent = 0;
#endif
            }
        }
    }

    void collectSequencerSamplerEvents( AudioChannel *channel, std::vector<SampleEvent*> *audioEvents,
                                        int bufferPosition, int bufferEnd )
    {
        // removal queue
        std::vector<BaseCacheableAudioEvent*> removes;

        int i = 0;
        int amount = audioEvents->size();
        for ( i; i < amount; i++ )
        {
            SampleEvent* audioEvent = audioEvents->at( i );

            if ( audioEvent->isEnabled() )
            {
                int sampleStart = audioEvent->getSampleStart();
                int sampleEnd   = audioEvent->getSampleEnd();

                if ( audioEvent->getLoopeable() ||
                   ( sampleStart >= bufferPosition && sampleStart <= bufferEnd ) ||
                   ( sampleStart < bufferPosition  && sampleEnd >= bufferPosition ))
                {
                    if ( !audioEvent->deletable())
                        channel->addEvent( audioEvent );
                    else
                        removes.push_back( audioEvent );
                }
            }
        }
        // removal queue filled ? process it so we can safely
        // remove "deleted" AudioEvents without read errors occurring

        if ( removes.size() > 0 )
        {
            int i = 0;
            for ( i; i < removes.size(); i++ )
            {
                BaseCacheableAudioEvent* audioEvent = removes[ i ];

                // remove audio event from sequencer (if it was present)
                if ( std::find( audioEvents->begin(), audioEvents->end(), audioEvent ) != audioEvents->end())
                {
                    audioEvents->erase( std::find( audioEvents->begin(), audioEvents->end(), audioEvent ));
                }
#ifndef USE_JNI
                // when using JNI, we let SWIG invoke destructors when Java references are finalized
                delete audioEvent;
                audioEvent = 0;
#endif
            }
        }
    }

    void collectDrumEvents( AudioChannel *channel, int bufferPosition, int bufferEnd )
    {
        if ( sequencer::drumPatterns.size() > 0 )
        {
            // drums loop by pattern, recalculate buffer position by subtracting
            // all measures above the first
            int bytesPerBar = AudioEngine::bytes_per_bar;

            while ( bufferPosition >= bytesPerBar )
            {
                bufferPosition -= bytesPerBar;
                bufferEnd      -= bytesPerBar;
            }

            DrumPattern* pattern = sequencer::drumPatterns[ sequencer::activeDrumPattern ];

            int i = 0;
            for ( i; i < pattern->audioEvents->size(); i++ )
            {
                BaseAudioEvent* audioEvent = pattern->audioEvents->at( i );

                int sampleStart = audioEvent->getSampleStart();
                int sampleEnd   = audioEvent->getSampleEnd();

                if (( sampleStart >= bufferPosition && sampleStart <= bufferEnd )
                        || ( sampleStart < bufferPosition && sampleEnd >= bufferPosition ))
                {
                    if ( !audioEvent->deletable())
                        channel->addEvent( audioEvent );
                    //else
                    //    audioEvent->destroy(); // should've been destroyed by the 'remove event' in DrumPattern...
                }
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

        //DebugTool::log("check for events at start range %d", bufferPosition);
        //DebugTool::log("until %d", bufferEnd );

        for ( int i = 0, l = sequencer::synthesizers.size(); i < l; ++i )
        {
            std::vector<BaseCacheableAudioEvent*>* audioEvents = sequencer::synthesizers.at( i )->audioEvents;
            int amount = audioEvents->size();

            for ( int j = 0; j < amount; j++ )
            {
                BaseCacheableAudioEvent* audioEvent = audioEvents->at( j );

                int sampleStart = audioEvent->getSampleStart();
                int sampleEnd   = audioEvent->getSampleEnd();

                if (( sampleStart >= bufferPosition && sampleStart <= bufferEnd ) ||
                    ( sampleStart < bufferPosition && sampleEnd >= bufferPosition ))
                {
                    if ( !audioEvent->deletable())
                        events->push_back( audioEvent );
                }
            }
        }
        return events;
    }
}
