/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2022 Igor Zinken - https://www.igorski.nl
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
#include <utilities/volumeutil.h>

namespace MWEngine {

/* constructor / destructor */

SequencerController::SequencerController()
{
    // by default, function as a sixteen step sequencer

    updateMeasures( 1, 16 );
};

SequencerController::SequencerController( int amountOfMeasures, int stepsPerBar )
{
    updateMeasures( amountOfMeasures, stepsPerBar );
}

SequencerController::~SequencerController()
{
    // nothing allocated here...
};

/* public methods */

void SequencerController::prepare( float aQueuedTempo, int aTimeSigBeatAmount, int aTimeSigBeatUnit )
{
    // calculate buffers and ranges
    if ( aQueuedTempo > 0 )
    {
        setTempo( aQueuedTempo, aTimeSigBeatAmount, aTimeSigBeatUnit );
        AudioEngine::handleTempoUpdate( aQueuedTempo, false );   // just to initialize all buffer sizes
        setLoopRange( 0, ( AudioEngine::amount_of_bars * AudioEngine::samples_per_bar ) - 1, AudioEngine::steps_per_bar );
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
    AudioEngine::handleTempoUpdate( AudioEngine::queuedTempo, true );
}

void SequencerController::setVolume( float aVolume )
{
    AudioEngine::volume = VolumeUtil::toLog( aVolume );
}

void SequencerController::setPlaying( bool aIsPlaying )
{
    Sequencer::playing = aIsPlaying;
}

void SequencerController::setLoopRange( int aStartPosition, int aEndPosition )
{
    setLoopRange( aStartPosition, aEndPosition, AudioEngine::steps_per_bar );
}

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

    AudioEngine::min_step_position = ( int ) round(( aStartPosition / AudioEngine::samples_per_bar ) * aStepsPerBar );
    AudioEngine::max_step_position = ( int ) round(((( float ) aEndPosition / ( float ) AudioEngine::samples_per_bar ) * aStepsPerBar ) - 1 );

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

    if ( aPosition < AudioEngine::min_buffer_position ||
         aPosition > AudioEngine::max_buffer_position )
    {
        aPosition = AudioEngine::min_buffer_position;
    }

    AudioEngine::bufferPosition = aPosition;
    AudioEngine::stepPosition   = ( aPosition / AudioEngine::samples_per_bar ) * AudioEngine::steps_per_bar;

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
    AudioEngine::steps_per_bar     = aStepsPerBar;
    AudioEngine::max_step_position = ( AudioEngine::steps_per_bar * AudioEngine::amount_of_bars ) - 1;
    AudioEngine::handleTempoUpdate( AudioEngine::tempo, false );

    // keep current sequencer step within the new loop range

    if ( AudioEngine::stepPosition > AudioEngine::max_step_position ) {
        AudioEngine::stepPosition = AudioEngine::min_step_position;
    }
}

void SequencerController::updateMeasures( int aValue, int aStepsPerBar )
{
    AudioEngine::amount_of_bars = aValue;
    updateStepsPerBar( aStepsPerBar );
    AudioEngine::max_buffer_position = ( AudioEngine::samples_per_bar * AudioEngine::amount_of_bars ) - 1;
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
    int endBufferPos   = ( startBufferPos + AudioEngine::samples_per_bar ) - 1;

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

void SequencerController::setBounceState( bool aIsBouncing, int aMaxBuffers, char* aOutputFile, int rangeStart, int rangeEnd )
{
    AudioEngine::bouncing = aIsBouncing;

    if ( AudioEngine::bouncing )
    {
        AudioEngine::bounceRangeStart = rangeStart;
        AudioEngine::bounceRangeEnd   = rangeEnd;
        AudioEngine::bufferPosition   = rangeStart;
        AudioEngine::stepPosition     = 0;
    }
    setRecordingState( aIsBouncing, aMaxBuffers, aOutputFile );

    // triggering bounce state should instantly toggle the playback state of the engine

    setPlaying( aIsBouncing );
}

void SequencerController::setRecordingState( bool aRecording, int aMaxBuffers, char* aOutputFile )
{
    // in case Sequencer was recording input from the Android device, halt recording of input
    if ( AudioEngine::recordInputToDisk )
        setRecordingFromDeviceState( false, 0, ( char* ) "\0", false );

    bool wasRecording               = AudioEngine::recordOutputToDisk;
    AudioEngine::recordOutputToDisk = aRecording;

    if ( AudioEngine::recordOutputToDisk )
    {
        DiskWriter::prepare(
            std::string( aOutputFile ), roundTo( aMaxBuffers, AudioEngineProps::BUFFER_SIZE ),
            AudioEngineProps::OUTPUT_CHANNELS
        );
    }
    else if ( wasRecording )
    {
        // recording halted, write currently recording snippet into file
        // and concatenate all recorded snippets into the requested output file name
        // we can do this synchronously as this method is called from outside the
        // rendering thread and thus won't lead to buffer under runs

        if ( DiskWriter::finish() ) {
            Notifier::broadcast( Notifications::RECORDING_COMPLETED );
        }
    }
}

void SequencerController::setRecordingFromDeviceState( bool aRecording, int aMaxBuffers, char* aOutputFile, bool skipProcessing )
{
    // in case Sequencer was recording its output, halt recording of output
    if ( AudioEngine::recordOutputToDisk )
        setRecordingState( false, 0, ( char* ) "\0" );

    bool wasRecording                 = AudioEngine::recordInputToDisk;
    AudioEngine::recordInputToDisk    = aRecording;
    AudioEngine::recordInputWithChain = !skipProcessing;

    if ( AudioEngine::recordInputToDisk )
    {
        DiskWriter::prepare(
            std::string( aOutputFile ), roundTo( aMaxBuffers, AudioEngineProps::BUFFER_SIZE ),
            AudioEngineProps::INPUT_CHANNELS
        );
    }
    else if ( wasRecording )
    {
        // recording halted, write currently recording snippet into file
        // and concatenate all recorded snippets into the requested output file name
        // we can do this synchronously as this method is called from outside the
        // rendering thread and thus won't lead to buffer under runs

        if ( DiskWriter::finish() ) {
            Notifier::broadcast( Notifications::RECORDING_COMPLETED );
        }
    }
}

void SequencerController::saveRecordedSnippet( int snippetBufferIndex )
{
    DiskWriter::writeBufferToFile( snippetBufferIndex, true );
}

} // E.O namespace MWEngine
