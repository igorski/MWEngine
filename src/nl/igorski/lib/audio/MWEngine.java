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
package nl.igorski.lib.audio;

import android.content.Context;
import android.os.Build;
import android.util.Log;
import nl.igorski.lib.audio.nativeaudio.BufferUtility;
import nl.igorski.lib.audio.nativeaudio.MWEngineCore;
import nl.igorski.lib.audio.nativeaudio.ProcessingChain;
import nl.igorski.lib.audio.nativeaudio.SequencerController;

/**
 * Created by IntelliJ IDEA.
 * User: igorzinken
 * Date: 10-04-12
 * Time: 20:16
 * To change this template use File | Settings | File Templates.
 */
public final class MWEngine extends Thread
{
    // interface to receive state change messages from the engine

    public interface IObserver
    {
        void handleNotification( int aNotificationId );
        void handleNotification( int aNotificationId, int aNotificationValue );
    }

    private static MWEngine INSTANCE;
    private IObserver       _observer;

    private SequencerController _sequencerController;       // hold reference (prevents garbage collection) to native sequencer providing audio

    /* audio generation related, calculated on platform-specific basis in constructor */

    public static int SAMPLE_RATE = 44100;
    public static int BUFFER_SIZE = 2048;

    /* time related */

    public static int TIME_SIG_BEAT_AMOUNT  = 4; // upper numeral in time signature (i.e. the "3" in 3/4)
    public static int TIME_SIG_BEAT_UNIT    = 4; // lower numeral in time signature (i.e. the "4" in 3/4)
    private float     _tempo;

    // we CAN multiply the output the volume to decrease it, preventing rapidly distorting audio ( especially on filters )
    private static final float VOLUME_MULTIPLIER = 1;
    private static float      _volume            = .85f /* assumed default level */ * VOLUME_MULTIPLIER;

    // we make these available across classes

    public static int BYTES_PER_BEAT;
    public static int BYTES_PER_BAR;
    public static int BYTES_PER_TICK;

    /* recording buffer specific */

    private boolean _recordOutput = false;

    /* native layer connection related */

    private boolean _openSLrunning   = false;
    private boolean _initialCreation = true;
    private int     _openSLRetry     = 0;

    /* threading related */

    protected boolean _isRunning = false;
    protected Object  _pauseLock;
    protected boolean _paused;

    /**
     * The Java-side bridge to manage all native layer components
     * of the MWEngine audio engine
     *
     * @param aContext   {Context} current application context
     */
    public MWEngine( Context aContext, IObserver aObserver )
    {
        INSTANCE   = this;
        _observer  = aObserver;

        _pauseLock = new Object();
        _paused    = false;

        initJNI();

        //setPriority( MAX_PRIORITY ); // no need, all in native layer
    }

    /* public methods */

    // (re-)registers interface to match current/updated JNI environment
    public void initJNI()
    {
        MWEngineCore.init();
    }

    public void createOutput( int aSampleRate, int aBufferSize )
    {
        SAMPLE_RATE = aSampleRate;
        BUFFER_SIZE = aBufferSize;

        // Android emulators can only work at 8 kHz or crash...
        if ( Build.FINGERPRINT.startsWith( "generic" ))
            SAMPLE_RATE = 8000;

        // defaults during initialization
        final float tempo = 120.0f;
        BYTES_PER_BAR = ( int )(( SAMPLE_RATE * 60 ) / tempo * 4 );

        _sequencerController = new SequencerController();
        _sequencerController.prepare( BUFFER_SIZE, SAMPLE_RATE, tempo, TIME_SIG_BEAT_AMOUNT, TIME_SIG_BEAT_UNIT ); // start w/ default of 120 BPM in 4/4 time
    }

    public float getVolume()
    {
        return _volume / VOLUME_MULTIPLIER;
    }

    public void setVolume( float aValue )
    {
        _volume = aValue * VOLUME_MULTIPLIER;
        _sequencerController.setVolume(_volume);
    }

    public SequencerController getSequencerController()
    {
        return _sequencerController;
    }

    public ProcessingChain getMasterBusProcessors()
    {
        return MWEngineCore.getMasterBusProcessors();
    }

    public void setBouncing( boolean value, String outputDirectory )
    {
        _sequencerController.setBounceState( value, calculateMaxBuffers(), outputDirectory );
    }

    /**
     * records the live output of the engine
     *
     * this keeps recording until setRecordingState is invoked with value false
     * given outputDirectory will contain several .WAV files each at the buffer
     * length returned by the "calculateMaxBuffers"-method
     */
    public void setRecordingState( boolean value, String outputDirectory )
    {
        int maxRecordBuffers = 0;

        // create / reset the recorded buffer when
        // hitting the record button
        if ( value )
            maxRecordBuffers = calculateMaxBuffers();

        _recordOutput = value;
        _sequencerController.setRecordingState(_recordOutput, maxRecordBuffers, outputDirectory);
    }

    /**
     * records the input channel of the Android device, note this can be done
     * while the engine is running a sequence / synthesizing audio
     *
     * given outputDirectory will contain a .WAV file at the buffer length
     * representing given maxDurationInMilliSeconds
     */
    public void setRecordFromDeviceInputState( boolean value, String outputDirectory, int maxDurationInMilliSeconds )
    {
        int maxRecordBuffers = 0;

        // create / reset the recorded buffer when
        // hitting the record button

        if ( value )
            maxRecordBuffers = BufferUtility.millisecondsToBuffer( maxDurationInMilliSeconds, SAMPLE_RATE );

        _recordOutput = value;
        _sequencerController.setRecordingFromDeviceState(_recordOutput, maxRecordBuffers, outputDirectory);
    }

    public boolean getRecordingState()
    {
        return _recordOutput;
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
        MWEngineCore.reset();
        _openSLRetry = 0;
    }

    /**
     * queries whether we can try to restart the engine
     * in case an error has occurred, note this will also
     * increment the amount of retries
     *
     * @return {boolean}
     */
    public boolean canRestartEngine()
    {
        return ++_openSLRetry < 5;
    }

    // due to Object pooling we keep the thread alive by just pausing its execution, NOT actual cleanup
    // exiting the application will kill the native library anyways
    public void dispose()
    {
        pause();

        _openSLrunning = false;

        MWEngineCore.stop();   // halt the Native audio thread

        //_isRunning = false;       // nope, as that will actually halt THIS thread
    }

    public void run()
    {
        Log.d( "MWENGINE", "MWEngine::STARTING NATIVE AUDIO RENDER LOOP" );

        while ( _isRunning )
        {
            // start the Native audio thread
            if ( !_openSLrunning )
            {
                // starting native thread
                Log.d( "MWENGINE", "MWEngine::starting engine render thread with " + BUFFER_SIZE +
                                   " sample buffer at " + SAMPLE_RATE + " Hz samplerate" );

                _openSLrunning = true;
                MWEngineCore.start();
            }

            // the remainder of this function body is actually blocked
            // as long as the native thread is running

            _openSLrunning = false;

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
     * requesting data, as such they are invoked from the native layer code and
     * not through any Java call !
     *
     * Java method IDs need to be supplied to C++ in order te make the callbacks, you
     * can discover the IDs by building the Java project and running the following
     * command in the output /bin folder:
     *
     * javap -s -private -classpath classes nl.igorski.lib.audio.MWEngine
     */

    public static void handleBridgeConnected( int aSomething )
    {
        // JNI bridge from native layer connected to this static Java class

        Log.d( "MWENGINE", "MWEngine::connected to JNI bridge" );
    }

    public static void handleNotification( int aNotificationId )
    {
        if ( INSTANCE._observer != null )
            INSTANCE._observer.handleNotification( aNotificationId );
    }

    public static void handleNotificationWithData( int aNotificationId, int aNotificationData )
    {
        if ( INSTANCE._observer != null )
            INSTANCE._observer.handleNotification( aNotificationId, aNotificationData );
    }

    public static void handleTempoUpdated( float aNewTempo, int aBytesPerBeat, int aBytesPerTick,
                                           int aBytesPerBar, int aTimeSigBeatAmount, int aTimeSigBeatUnit )
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
            INSTANCE.getSequencerController().setLoopRange(0, BYTES_PER_BAR);
        }
        Log.d( "MWENGINE", "MWEngine::handleTempoUpdated new tempo > " + aNewTempo + " @ " + aTimeSigBeatAmount + "/" +
                aTimeSigBeatUnit + " time signature ( " + aBytesPerBar + " bytes per bar )" );
    }
}
