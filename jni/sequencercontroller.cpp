/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2016 Igor Zinken - http://www.igorski.nl
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
#include "sequencercontroller.h"
#include "sequencer.h"
#include "audioengine.h"
#include <definitions/notifications.h>
#include <messaging/notifier.h>
#include <utilities/utils.h>
#include <utilities/diskwriter.h>

/* constructor / destructor */

SequencerController::SequencerController()
{
    // by default, function as a sixteen step sequencer

    stepsPerBar = 16;
    AudioEngine::max_step_position = stepsPerBar - 1;
};

SequencerController::~SequencerController()
{

};

/* public methods */

void SequencerController::prepare( int aBufferSize, int aSampleRate, float aQueuedTempo,
                                   int aTimeSigBeatAmount, int aTimeSigBeatUnit )
{
    // set renderer output variables
    AudioEngineProps::BUFFER_SIZE = aBufferSize;
    AudioEngineProps::SAMPLE_RATE = aSampleRate;

    // calculate buffers and ranges
    if ( aQueuedTempo > 0 )
    {
        AudioEngine::queuedTempo = aQueuedTempo;
        AudioEngine::handleTempoUpdate( aQueuedTempo, false );   // just to initialize all buffer sizes
        setLoopRange( 0, AudioEngine::samples_per_bar, aTimeSigBeatAmount * aTimeSigBeatUnit );
    }
};

float SequencerController::getTempo()
{
    return AudioEngine::tempo;
}

void SequencerController::setTempo( float aTempo, int aTimeSigBeatAmount, int aTimeSigBeatUnit )
{
    AudioEngine::queuedTempo = aTempo;

    AudioEngine::queuedTime_sig_beat_amount = aTimeSigBeatAmount;
    AudioEngine::queuedTime_sig_beat_unit   = aTimeSigBeatUnit;
}

void SequencerController::setTempoNow( float aTempo, int aTimeSigBeatAmount, int aTimeSigBeatUnit )
{
    setTempo( aTempo, aTimeSigBeatAmount, aTimeSigBeatUnit );
    updateStepsPerBar( stepsPerBar );
    AudioEngine::handleTempoUpdate( AudioEngine::queuedTempo, true );
}

void SequencerController::setVolume( float aVolume )
{
    AudioEngine::volume = aVolume;
}

void SequencerController::setPlaying( bool aIsPlaying )
{
    Sequencer::playing = aIsPlaying;
}

void SequencerController::setLoopRange( int aStartPosition, int aEndPosition )
{
    setLoopRange( aStartPosition, aEndPosition, stepsPerBar );
}

/**
 * make the sequencer loop between two given points
 *
 * @param aStartPosition {int} buffer offset of the loops startpoint (starts at 0 !)
 * @param aEndPosition   {int} buffer offset of the loops endpoint
 * @param aStepsPerBar   {int} the amount of individual segments the sequencer subdivides a single bar into
 *                             this is used for periodic notifications when the sequencer switches step
 */
void SequencerController::setLoopRange( int aStartPosition, int aEndPosition, int aStepsPerBar )
{
    AudioEngine::min_buffer_position = aStartPosition;
    AudioEngine::max_buffer_position = aEndPosition;

    // keep current buffer read pointer within the new loop range
    if ( AudioEngine::bufferPosition < AudioEngine::min_buffer_position ||
         AudioEngine::bufferPosition > AudioEngine::max_buffer_position )
    {
        AudioEngine::bufferPosition = AudioEngine::min_buffer_position;
    }
    AudioEngine::min_step_position = round(( aStartPosition / AudioEngine::samples_per_bar ) * aStepsPerBar );
    AudioEngine::max_step_position = round(((( float ) aEndPosition / ( float )AudioEngine::samples_per_bar ) * aStepsPerBar ) - 1 );

    // keep current sequencer step within the new loop range
    if ( AudioEngine::stepPosition < AudioEngine::min_step_position ||
         AudioEngine::stepPosition > AudioEngine::max_step_position )
    {
        AudioEngine::stepPosition = AudioEngine::min_step_position;
    }
    updateStepsPerBar( aStepsPerBar );
}

int SequencerController::getStepPosition()
{
    return AudioEngine::stepPosition;
}

int SequencerController::getBufferPosition()
{
    return AudioEngine::bufferPosition;
}

void SequencerController::setBufferPosition( int aPosition )
{
    // keep position within the sequences range (see "setLoopRange")

    if ( aPosition < AudioEngine::min_buffer_position )
        aPosition = AudioEngine::min_buffer_position;

    else if ( aPosition > AudioEngine::max_buffer_position )
        aPosition = AudioEngine::max_buffer_position;

    AudioEngine::bufferPosition = aPosition;
    AudioEngine::stepPosition   = ( aPosition / AudioEngine::samples_per_bar ) * stepsPerBar;

    Notifier::broadcast( Notifications::SEQUENCER_POSITION_UPDATED );
}

int SequencerController::getSamplesPerBeat()
{
    return AudioEngine::samples_per_beat;
}

int SequencerController::getSamplesPerStep()
{
    return ( int ) AudioEngine::samples_per_step;
}

int SequencerController::getSamplesPerBar()
{
    return AudioEngine::samples_per_bar;
}

int SequencerController::getTimeSigBeatAmount()
{
    return AudioEngine::time_sig_beat_amount;
}

int SequencerController::getTimeSigBeatUnit()
{
    return AudioEngine::time_sig_beat_unit;
}

void SequencerController::updateStepsPerBar( int aStepsPerBar )
{
    stepsPerBar = aStepsPerBar;
    AudioEngine::beat_subdivision = aStepsPerBar / AudioEngine::time_sig_beat_amount;
}

void SequencerController::updateMeasures( int aValue, int aStepsPerBar )
{
    AudioEngine::amount_of_bars      = aValue;
    AudioEngine::max_step_position   = ( aStepsPerBar * AudioEngine::amount_of_bars ) - 1;
    AudioEngine::max_buffer_position = ( AudioEngine::samples_per_bar * AudioEngine::amount_of_bars ) - 1;

    updateStepsPerBar( aStepsPerBar );
}

void SequencerController::rewind()
{
    setBufferPosition( AudioEngine::min_buffer_position );
}

void SequencerController::setNotificationMarker( int aPosition )
{
    AudioEngine::marked_buffer_position = aPosition;
}

/**
 * used for intelligent pre-caching, get the BaseCacheableAudioEvents
 * belonging to a specific measure for on-demand caching
 *
 * @param aMeasure {int} the measure containing the events we'd like to precache
 */
void SequencerController::cacheAudioEventsForMeasure( int aMeasure )
{
    int startBufferPos = AudioEngine::samples_per_bar * aMeasure;
    int endBufferPos   = startBufferPos + AudioEngine::samples_per_bar;

    std::vector<BaseCacheableAudioEvent*>* list = Sequencer::collectCacheableSequencerEvents( startBufferPos, endBufferPos );
    getBulkCacher()->addToQueue( list );

    delete list; // free memory

    if ( getBulkCacher()->hasQueue())
        getBulkCacher()->cacheQueue();
}

BulkCacher* SequencerController::getBulkCacher()
{
    return Sequencer::bulkCacher;
}

/**
 * when bouncing, the writing of buffers into the hardware is omitted
 * for an increase in bouncing speed (otherwise its real time)
 */
void SequencerController::setBounceState( bool aIsBouncing, int aMaxBuffers, char* aOutputDirectory )
{
    AudioEngine::bouncing = aIsBouncing;

    if ( AudioEngine::bouncing )
    {
        AudioEngine::bufferPosition = 0;
        AudioEngine::stepPosition   = 0;
    }
    setRecordingState( aIsBouncing, aMaxBuffers, aOutputDirectory );
}

/**
 * record the output of the sequencer
 *
 * aRecording        {bool} toggles the recording state
 * aMaxBuffers        {int} the total recorded buffer size to store in memory
 *                          before writing the recorded contents as .WAV file into
 *                          the given output directory.
 * aOutputDirectory {char*} name of the folder to write each snippet into
 */
void SequencerController::setRecordingState( bool aRecording, int aMaxBuffers, char* aOutputDirectory )
{
    // in case Sequencer was recording input from the Android device, halt recording of input
    if ( AudioEngine::recordFromDevice )
        setRecordingFromDeviceState( false, 0, '\0' );

    bool wasRecording         = AudioEngine::recordOutput;
    AudioEngine::recordOutput = aRecording;

    if ( AudioEngine::recordOutput )
    {
        DiskWriter::prepare( std::string( aOutputDirectory ), aMaxBuffers, AudioEngineProps::OUTPUT_CHANNELS );
    }
    else if ( wasRecording )
    {
        if ( !Sequencer::playing )
        {
            DiskWriter::writeBufferToFile( AudioEngineProps::SAMPLE_RATE, AudioEngineProps::OUTPUT_CHANNELS, true );
        }
        else {
            // apparently renderer is stopped before cycle completes next Disk Writing query... =/
            DiskWriter::writeBufferToFile( AudioEngineProps::SAMPLE_RATE, AudioEngineProps::OUTPUT_CHANNELS, true );
            AudioEngine::haltRecording = true;
        }
    }
    AudioEngine::recordingFileId = 0;  // write existing buffers using previous iteration before resetting this counter!!
}

/**
 * record audio from the Androids input channel, this stores only the incoming audio
 * not the remaining audio processed / generated by the engine
 *
 * aRecording        {bool} toggles the recording state
 * aMaxBuffers        {int} the total recorded buffer size to store in memory
 *                          before writing the recorded contents as .WAV file into
 *                          the given output directory.
 * aOutputDirectory {char*} name of the folder to write each snippet into
 */
void SequencerController::setRecordingFromDeviceState( bool aRecording, int aMaxBuffers, char* aOutputDirectory )
{
    // in case Sequencer was recording its output, halt recording of output
    if ( AudioEngine::recordOutput )
        setRecordingState( false, 0, '\0' );

    bool wasRecording             = AudioEngine::recordFromDevice;
    AudioEngine::recordFromDevice = aRecording;

    if ( AudioEngine::recordFromDevice )
    {
        DiskWriter::prepare( std::string( aOutputDirectory ), aMaxBuffers, AudioEngineProps::INPUT_CHANNELS );
    }
    else if ( wasRecording )
    {
        if ( !Sequencer::playing )
        {
            DiskWriter::writeBufferToFile( AudioEngineProps::SAMPLE_RATE, AudioEngineProps::INPUT_CHANNELS, true );
        }
        else {
            // apparently renderer is stopped before cycle completes next Disk Writing query... =/
            DiskWriter::writeBufferToFile( AudioEngineProps::SAMPLE_RATE, AudioEngineProps::INPUT_CHANNELS, true );
            AudioEngine::haltRecording = true;
        }
    }
    AudioEngine::recordingFileId = 0;  // write existing buffers using previous iteration before resetting this counter!!
}
