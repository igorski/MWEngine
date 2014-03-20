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
#include "sequencer_api.h"
#include "global.h"
#include "utils.h"
#include "native_audio_engine.h"
#include "diskwriter.h"

/* constructor / destructor */

SequencerAPI::SequencerAPI()
{

};

SequencerAPI::~SequencerAPI()
{
    // should never be removed and maintained throughout due to Object pooling...
};

/* public methods */

void SequencerAPI::prepare( int aBufferSize, int aSampleRate, float aQueuedTempo, int aTimeSigBeatAmount, int aTimeSigBeatUnit )
{
    // set renderer output variables
    audio_engine::BUFFER_SIZE = aBufferSize;
    audio_engine::SAMPLE_RATE = aSampleRate;

    // calculate buffers and ranges
    if ( aQueuedTempo > 0 )
    {
        queuedTempo = aQueuedTempo;
        handleTempoUpdate( aQueuedTempo, false );   // just to initialize all buffer sizes
        setLoopPoint( 0, bytes_per_bar, aTimeSigBeatAmount * aTimeSigBeatUnit );
    }
};

/**
 * make the sequencer loop between two given points
 *
 * @param aStartPosition {int} buffer offset of the loops startpoint (starts at 0 !)
 * @param aEndPosition   {int} buffer offset of the loops endpoint
 * @param aStepsPerBar   {int} the amount of individual segments the sequencer subdivides a single bar into
 *                             this is used for periodic notifications when the sequencer switches step
 */
void SequencerAPI::setLoopPoint( int aStartPosition, int aEndPosition, int aStepsPerBar )
{
    min_buffer_position = aStartPosition;
    max_buffer_position = aEndPosition;

    // keep current buffer read pointer within the new loop range
    if ( bufferPosition < min_buffer_position || bufferPosition > max_buffer_position )
        bufferPosition = min_buffer_position;

    min_step_position = ( aStartPosition / bytes_per_bar ) * aStepsPerBar;
    max_step_position = (( aEndPosition + 1 ) / bytes_per_bar ) * aStepsPerBar;

    // keep current sequencer step within the new loop range
    if ( stepPosition < min_step_position || stepPosition > max_step_position )
        stepPosition =  min_step_position;
}

void SequencerAPI::updateMeasures( int aValue, int aStepsPerBar )
{
    amount_of_bars      = aValue;
    max_step_position   = aStepsPerBar * amount_of_bars;
    max_buffer_position = ( bytes_per_bar * amount_of_bars ) - 1; // -1 as we use array lookups and start at 0
}

void SequencerAPI::setTempo( float aTempo, int aTimeSigBeatAmount, int aTimeSigBeatUnit )
{
    queuedTempo = aTempo;

    queuedTime_sig_beat_amount = aTimeSigBeatAmount;
    queuedTime_sig_beat_unit   = aTimeSigBeatUnit;
}

void SequencerAPI::setTempoNow( float aTempo, int aTimeSigBeatAmount, int aTimeSigBeatUnit )
{
    setTempo( aTempo, aTimeSigBeatAmount, aTimeSigBeatUnit );
    handleTempoUpdate( queuedTempo, true );
}

void SequencerAPI::setVolume( float aVolume )
{
    volume = aVolume;
}

void SequencerAPI::setPlaying( bool aIsPlaying )
{
    playing = aIsPlaying;
}

void SequencerAPI::setActiveDrumPattern( int activePattern )
{
    sequencer::activeDrumPattern = activePattern;
}

void SequencerAPI::rewind()
{
    bufferPosition = min_buffer_position;
    stepPosition   = min_step_position;

    broadcastStepPosition();
}

/**
 * used for intelligent pre-caching, get the BaseCacheableAudioEvents
 * belonging to a specific measure for on-demand caching
 *
 * @param aMeasure {int} the measure containing the events we'd like to precache
 */
void SequencerAPI::cacheAudioEventsForMeasure( int aMeasure )
{
    int startBufferPos = bytes_per_bar * aMeasure;
    int endBufferPos   = startBufferPos + bytes_per_bar;

    std::vector<BaseCacheableAudioEvent*>* list = collectCacheableSequencerEvents( startBufferPos, endBufferPos );
    getBulkCacher()->addToQueue( list );

    delete list; // free memory

    if ( getBulkCacher()->hasQueue())
        getBulkCacher()->cacheQueue();
}

BulkCacher* SequencerAPI::getBulkCacher()
{
    return sequencer::bulkCacher;
}

/**
 * when bouncing, the writing of buffers into the hardware is omitted
 * for an increase in bouncing speed (otherwise its real time)
 */
void SequencerAPI::setBounceState( bool aIsBouncing, int aMaxBuffers, char* aOutputDirectory )
{
    bouncing = aIsBouncing;

    if ( bouncing )
    {
        bufferPosition = 0;
        stepPosition   = 0;
    }
    setRecordingState( aIsBouncing, aMaxBuffers, aOutputDirectory );
}

/**
 * record the output of the sequencer
 *
 * aRecording        {bool} toggles the recording state
 * aMaxBuffers        {int} the maximum amount of buffers (each will hold BUFFER_SIZE in length) to store
 *                          before broadcasting the RECORDING_UPDATE message back via the JNI
 * aOutputDirectory {char*} name of the folder to write int
 */
void SequencerAPI::setRecordingState( bool aRecording, int aMaxBuffers, char* aOutputDirectory )
{
    bool wasRecording = recordOutput;
    recordOutput      = aRecording;

    if ( recordOutput )
    {
        DiskWriter::prepareOutput( std::string( aOutputDirectory ), aMaxBuffers );
    }
    else
    {
        if ( wasRecording )
        {
            if ( !playing )
            {
                DiskWriter::writeBufferToFile( audio_engine::SAMPLE_RATE, audio_engine::OUTPUT_CHANNELS, true );
            }
            else {
                // apparently renderer is stopped before cycle completes next Disk Writing query... =/
                DiskWriter::writeBufferToFile( audio_engine::SAMPLE_RATE, audio_engine::OUTPUT_CHANNELS, true );
                haltRecording = true;
            }
        }
    }
    recordingFileName = 0;  // write existing buffers using previous iteration before resetting this counter!!
}

/**
 * record audio from the Androids input
 *
 * aRecording        {bool} toggles the recording state
 * aMaxBuffers        {int} the maximum amount of buffers (each will hold BUFFER_SIZE in length) to store
 *                          before broadcasting the RECORDING_UPDATE message back via the JNI
 * aOutputDirectory {char*} name of the folder to write int
 */
void SequencerAPI::setRecordingFromDeviceState( bool aRecording, int aMaxBuffers, char* aOutputDirectory )
{
    bool wasRecording = recordFromDevice;
    recordFromDevice  = aRecording;

    if ( recordFromDevice )
    {
        // we have only one diskwriter currently (see Java CacheWriter/Readers), so we
        // must halt recording of live output when recording from the Android device
        if ( recordOutput )
        {
            setRecordingState( false, 0, "" );
        }
        DiskWriter::prepareOutput( std::string( aOutputDirectory ), aMaxBuffers );
    }
    else
    {
        if ( wasRecording )
        {
            if ( !playing )
            {
                DiskWriter::writeBufferToFile( audio_engine::SAMPLE_RATE, audio_engine::OUTPUT_CHANNELS, true );
            }
            else {
                // apparently renderer is stopped before cycle completes next Disk Writing query... =/
                DiskWriter::writeBufferToFile( audio_engine::SAMPLE_RATE, audio_engine::OUTPUT_CHANNELS, true );
                haltRecording = true;
            }
        }
    }
    recordingFileName = 0;  // write existing buffers using previous iteration before resetting this counter!!
}
