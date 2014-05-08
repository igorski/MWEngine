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
package nl.igorski.lib.audio.renderer;

import android.content.Context;
import android.os.Build;
import nl.igorski.lib.audio.nativeaudio.NativeAudioEngine;
import nl.igorski.lib.audio.nativeaudio.SequencerAPI;
import nl.igorski.lib.debug.Logger;

/**
 * Created by IntelliJ IDEA.
 * User: igorzinken
 * Date: 10-04-12
 * Time: 20:16
 * To change this template use File | Settings | File Templates.
 */
public final class NativeAudioRenderer extends Thread
{
    private static NativeAudioRenderer INSTANCE;

    private SequencerAPI _api;       // hold reference (prevents garbage collection) to native sequencer providing audio

    /* audio generation related, calculated on platform-specific basis in constructor */

    public static int SAMPLE_RATE = 44100;
    public static int BUFFER_SIZE = 2048;

    /* time related */

    public static int TIME_SIG_BEAT_AMOUNT  = 4; // upper numeral in time signature (i.e. the "3" in 3/4)
    public static int TIME_SIG_BEAT_UNIT    = 4; // lower numeral in time signature (i.e. the "4" in 3/4)
    private float _tempo;

    // we CAN multiply the output the volume to decrease it, preventing rapidly distorting audio ( especially on filters )
    private static final float VOLUME_MULTIPLIER = 1;
    private static float _volume = .85f /* assumed default level */ * VOLUME_MULTIPLIER;

    // we make these available across classes

    public static int BYTES_PER_SAMPLE = 8;
    public static int BYTES_PER_BEAT;
    public static int BYTES_PER_BAR;
    public static int BYTES_PER_TICK;
    public static int BAR_SUBDIVISIONS = 16;   // amount of steps per bar (defaults to sixteen step sequencing)

    // handle sequencer positions

    private int _stepPosition = 0;

    /* recording buffer specific */

    private boolean _recordOutput = false;

    private boolean _openSLrunning   = false;
    private boolean _initialCreation = true;
    private int _openSLRetry         = 0;

    /* threading related */

    protected boolean _isRunning = false;
    protected Object _pauseLock;
    protected boolean _paused;

    /**
     * AudioRenderer synthesizes all audio
     * created in the sequencer
     *
     * @param aContext   {Context} current application context
     */
    public NativeAudioRenderer( Context aContext )
    {
        INSTANCE   = this;
        _pauseLock = new Object();
        _paused    = false;

        initJNI();

        //setPriority( MAX_PRIORITY ); // no need, all in native layer
    }

    /* public methods */

    // (re-)registers interface to match current/updated JNI environment
    public void initJNI()
    {
        NativeAudioEngine.init();
    }

    public void createOutput( int aSampleRate, int aBufferSize )
    {
        SAMPLE_RATE = aSampleRate;
        BUFFER_SIZE = aBufferSize;

        // Android emulators can only work at 8 kHz or crash...
        if ( Build.FINGERPRINT.startsWith( "generic" ))
            SAMPLE_RATE = 8000;

        _api = new SequencerAPI();

        _api.prepare( BUFFER_SIZE, SAMPLE_RATE, 120.0f, TIME_SIG_BEAT_AMOUNT, TIME_SIG_BEAT_UNIT ); // start w/ default of 120 BPM in 4/4 time
    }

    /**
     * set play / pause state
     * @param value {boolean} whether sequencer is playing (true)
     *                        or paused (false)
     */
    public void setPlaying( boolean value )
    {
        _api.setPlaying( value );
    }

    public void rewind()
    {
        _api.rewind();
    }

    public void setBouncing( boolean value, String outputDirectory )
    {
        _api.setBounceState(value, calculateMaxBuffers(), outputDirectory);
    }

    public float getTempo()
    {
        return _tempo;
    }

    public SequencerAPI getAPI()
    {
        return _api;
    }

    /**
     * tempo changes are executed outside of the
     * render and thus are queued
     *
     * @param aValue {float} new tempo
     * @param aTimeSigBeatAmount {int} time signature beat amount
     * @param aTimeSigBeatUnit   {int} time signature beat unit
     */
    public void setTempo( float aValue, int aTimeSigBeatAmount, int aTimeSigBeatUnit )
    {
        _api.setTempo(aValue, aTimeSigBeatAmount, aTimeSigBeatUnit);
    }

    /**
     * set the amount of subdivisions we use in a bar (in a musical
     * context this can be used to lock time, i.e. 16 allows for
     * fixed sixteen-step sequencing)
     *
     * @param value {int}
     */
    public void setBarSubdivions( int value )
    {
        BAR_SUBDIVISIONS = value;
    }

    /**
     * unless the sequencer isn't running, when
     * this method can be used to set the tempo directly
     *
     * @param aValue {float} new tempo
     * @param aTimeSigBeatAmount {int} time signature beat amount
     * @param aTimeSigBeatUnit   {int} time signature beat amount
     */
    public void setTempoNow( float aValue, int aTimeSigBeatAmount, int aTimeSigBeatUnit )
    {
        _api.setTempoNow( aValue, aTimeSigBeatAmount, aTimeSigBeatUnit );
    }

    public float getVolume()
    {
        return _volume / VOLUME_MULTIPLIER;
    }

    public void setVolume( float aValue )
    {
        _volume = aValue * VOLUME_MULTIPLIER;
        _api.setVolume(_volume);
    }

    /**
     * set the around of measures (bars) the current sequence/song holds
     *
     * @param aValue          {int} amount of measures
     */
    public void updateMeasures( int aValue )
    {
        _api.updateMeasures( aValue, BAR_SUBDIVISIONS );
    }

    public void setRecordingState( boolean value, String outputDirectory )
    {
        int maxRecordBuffers = 0;

        // create / reset the recorded buffer when
        // hitting the record button
        if ( value )
            maxRecordBuffers = calculateMaxBuffers();

        _recordOutput = value;
        _api.setRecordingState( _recordOutput, maxRecordBuffers, outputDirectory );
    }

    public boolean getRecordingState()
    {
        return _recordOutput;
    }

    /**
     * define a range for the sequencer to loop
     *
     * @param aStartPosition  {int} buffer position of the start point
     * @param aEndPosition    {int} buffer position of the end point
     */
    public void setLoopPoint( int aStartPosition, int aEndPosition )
    {
        _api.setLoopPoint( aStartPosition, aEndPosition, BAR_SUBDIVISIONS );
    }

    /**
     * get the sequencers current position (is a step point
     * between the sequencers loop range), we should never
     * return buffer positions as it is the AudioRenderer's
     * job to handle these
     *
     * @return {int}
     */
    public int getPosition()
    {
        return _stepPosition;
    }

    @Override
    public void start()
    {
        if ( !_isRunning )
        {
            super.start();
        }
        else
        {
            initJNI();  // update reference to this Java object in JNI
            unpause();
        }
        _isRunning = true;
    }

    /**
     * invoke when the application suspends, this should
     * halt the execution of the run method and cause the
     * thread to clean up to free CPU resources
     */
    public void pause()
    {
        synchronized ( _pauseLock )
        {
            _paused = true;
        }
    }

    public boolean isPaused()
    {
        return _paused;
    }

    public void unpause()
    {
        initJNI();

        synchronized ( _pauseLock )
        {
            _paused = false;
            _pauseLock.notifyAll();
        }
    }

    public void reset()
    {
        NativeAudioEngine.reset();
    }

    // due to Object pooling we keep the thread alive by just pausing its execution, NOT actual cleanup
    // exiting the application will kill the native library anyways
    public void dispose()
    {
        pause();

        _openSLrunning = false;
        _openSLRetry   = 0;

        NativeAudioEngine.stop();   // halt the Native audio thread

        //_isRunning = false;       // nope, as that will actually halt THIS thread
    }

    public void run()
    {
        Logger.log( "NativeAudioRenderer::STARTING NATIVE AUDIO RENDER LOOP" );

        while ( _isRunning )
        {
            // start the Native audio thread
            if ( !_openSLrunning )
            {
                // starting native thread
                Logger.log( "NativeAudioRenderer::starting engine render thread with " + BUFFER_SIZE + " sample buffer at " + SAMPLE_RATE + " Hz samplerate" );

                _openSLrunning = true;
                NativeAudioEngine.start();
            }

            // the remainder of this function body is actually blocked
            // as long as the native thread is running

            // native thread halted

            // this shouldn't occur, should it !?
            if ( ++_openSLRetry > 5 )
            {
                // ERROR > Java thread remains running while native layer thread has stopped > RESTART native thread
                _openSLrunning = false; // force restart of engine
                _openSLRetry = 0;
            }
            synchronized ( _pauseLock )
            {
                while ( _paused )
                {
                    try
                    {
                        _pauseLock.wait();
                    }
                    catch ( InterruptedException e ) {}
                }
            }
        }
    }

    private int calculateMaxBuffers()
    {
        // we record a maximum of 30 seconds before invoking the "handleRecordingUpdate"-method on the sequencer
        final double amountOfMinutes = .5;

        // convert milliseconds to sample buffer size
        return ( int ) (( amountOfMinutes * 60000 ) * ( SAMPLE_RATE / 1000 ));
    }

    /* native bridge methods */

    /**
     * all these methods are static and provide a bridge from C++ back into Java
     * these methods are used by the native audio engine for updating states and
     * requesting data
     *
     * Java method IDs need to be supplied to C++ in order te make the callbacks, you
     * can discover the IDs by building the Java project and running the following
     * command in the output /bin folder:
     *
     * javap -s -private -classpath classes nl.igorski.lib.audio.renderer.NativeAudioRenderer
     */

    public static void handleBridgeConnected( int aSomething )
    {
        // JNI bridge from native layer connected to this static Java class

        Logger.log( "NativeAudioRenderer::connected to JNI bridge" );
    }

    public static void handleTempoUpdated( float aNewTempo, int aBytesPerBeat, int aBytesPerTick, int aBytesPerBar, int aTimeSigBeatAmount, int aTimeSigBeatUnit )
    {
        INSTANCE._tempo = aNewTempo;

        BYTES_PER_BEAT  = aBytesPerBeat;
        BYTES_PER_TICK  = aBytesPerTick;
        BYTES_PER_BAR   = aBytesPerBar;

        TIME_SIG_BEAT_AMOUNT = aTimeSigBeatAmount;
        TIME_SIG_BEAT_UNIT   = aTimeSigBeatUnit;

        // weird bug where on initial start sequencer would not know the step range...
        if ( INSTANCE._initialCreation )
        {
            INSTANCE._initialCreation = false;
            INSTANCE.setLoopPoint( 0, BYTES_PER_BAR - 1 );
        }

        Logger.log( "NativeAudioRenderer::handleTempoUpdated new tempo > " + aNewTempo + " @ " + aTimeSigBeatAmount + "/" + aTimeSigBeatUnit + " time signature ( " + aBytesPerBar + " bytes per bar )" );
    }

    public static void handleSequencerPositionUpdate( int aStepPosition )
    {
        INSTANCE._stepPosition = aStepPosition;

        //Logger.log( "NativeAudioRenderer::handleSequencerPositionUpdate position > " + aStepPosition );
    }

    public static void handleRecordingUpdate( int aRecordedFileNum )
    {
        // invoked after engine has recorded the given buffer length

        Logger.log( "NativeAudioRenderer::handleRecordingUpdate recorded next fragment with num > " + aRecordedFileNum );
    }

    public static void handleBounceComplete( int aIdentifier )
    {
        // invoked after bouncing of audio has completed

        Logger.log( "NativeAudioRenderer::handleBounceComplete finished bouncing of audio for id > " + aIdentifier );
    }

    public static void handleOpenSLError()
    {
        Logger.log( "NativeAudioRenderer::error occurred during OpenSL initialization" );

        // re-initialize thread
        INSTANCE.dispose();
        INSTANCE._openSLrunning = false;
        INSTANCE._openSLRetry = 0;
        INSTANCE.createOutput( SAMPLE_RATE, BUFFER_SIZE );
        INSTANCE.start();
    }
}
