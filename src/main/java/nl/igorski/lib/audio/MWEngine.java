/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2018 Igor Zinken - http://www.igorski.nl
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
import android.os.Handler;
import android.os.Process;
import android.util.Log;
import nl.igorski.lib.audio.mwengine.*;

public final class MWEngine extends Thread
{
    /**
     * interface to receive state change messages from the engine
     * these messages are constants defined inside Notifications.ids.values()
     */
    public interface IObserver
    {
        /**
         * invoked whenever the engine broadcasts a notification
         * @param aNotificationId {int} unique identifier for the notification
         *
         * supported notification identifiers (see notifications.h):
         *
         * ERROR_HARDWARE_UNAVAILABLE fired when MWEngine cannot connect to audio hardware (fatal)
         * ERROR_THREAD_START         fired when MWEngine cannot start the rendering thread (fatal)
         * STATUS_BRIDGE_CONNECTED    fired when MWEngine connects to the native layer code through JNI
         * MARKER_POSITION_REACHED    fired when request Sequencer marker position has been reached
         * RECORDING_COMPLETED        fired when recording has completed and requested output file is saved
         */
        void handleNotification( int aNotificationId );

        /**
         * invoked whenever the engine broadcasts a notification
         *
         * @param aNotificationId {int} unique identifier for the notification
         * @param aNotificationValue {int} payload for the notification
         *
         * supported notification identifiers (see notifications.h):
         *
         * SEQUENCER_POSITION_UPDATED fired when Sequencer has advanced a step, payload describes
         *                            the precise buffer offset of the Sequencer when the notification fired
         *                            (as a value in the range of 0 - BUFFER_SIZE)
         * RECORDED_SNIPPET_READY     fired when a recording snippet of the requested size is ready for
         *                            writing onto storage, payload describes snippets buffer index (see DiskWriter)
         * RECORDED_SNIPPET_SAVED     fired when snippet has been saved onto storage, payload describes snippets number
         * BOUNCE_COMPLETE            fired when the offline bouncing of the Sequencer range has completed
         */
        void handleNotification( int aNotificationId, int aNotificationValue );
    }

    private static MWEngine INSTANCE;
    private IObserver       _observer;
    private Context         _context;

    private SequencerController _sequencerController;

    /* audio generation related, should be overridden to match device-specific values */

    public static int SAMPLE_RATE     = 44100;
    public static int BUFFER_SIZE     = 2048;
    public static int OUTPUT_CHANNELS = 1; // 1 = mono, 2 = stereo

    private static float _volume = 1.0f;

    /* engine / thread states */

    private boolean _nativeEngineRunning = false;
    private int     _nativeEngineRetries = 0;
    private boolean _initialCreation     = true;
    private boolean _disposed            = false;
    private boolean _threadStarted       = false;
    private boolean _isRunning           = false;
    private boolean _paused              = false;
    private final Object _pauseLock      = new Object();

    /**
     * The Java-side bridge to manage all native layer components
     * of the MWEngine audio engine
     *
     * @param aContext {Context} current application context
     * @param aObserver {MWEngine.IObserver} observer that will monitor engine states
     */
    public MWEngine( Context aContext, IObserver aObserver )
    {
        if ( INSTANCE != null )
            throw new Error( "MWEngine already instantiated" );

        INSTANCE   = this;
        _context   = aContext;
        _observer  = aObserver;

        initJNI();
    }

    /* public methods */

    // (re-)registers interface to match current/updated JNI environment

    /**
     * (re)-register the interface between this Java class and the
     * native layer audio engine. This is recommended to synchronise
     * changes in the JNI environments (for instance: when suspending
     * the application or starting new Threads)
     */
    public void initJNI()
    {
        MWEngineCore.init();
    }

    public void createOutput( int aSampleRate, int aBufferSize, int aOutputChannels )
    {
        SAMPLE_RATE     = aSampleRate;
        BUFFER_SIZE     = aBufferSize;
        OUTPUT_CHANNELS = aOutputChannels;

        // older Android emulators can only work at 8 kHz or crash violently...
        if ( Build.FINGERPRINT.startsWith( "generic" ))
            SAMPLE_RATE = 8000;

        MWEngineCore.setup( BUFFER_SIZE, SAMPLE_RATE, OUTPUT_CHANNELS );

        // start w/ default of 120 BPM in 4/4 time

        _sequencerController = new SequencerController();
        _sequencerController.prepare( 120.0f, 4, 4 );

        _disposed = false;
    }

    public static MWEngine getInstance()
    {
        return INSTANCE;
    }

    public float getVolume()
    {
        return _volume;
    }

    public void setVolume( float aValue )
    {
        _volume = aValue;
        _sequencerController.setVolume( _volume );
    }

    public SequencerController getSequencerController()
    {
        return _sequencerController;
    }

    public ProcessingChain getMasterBusProcessors()
    {
        return MWEngineCore.getMasterBusProcessors();
    }

    /**
     * Retrieve the AudioChannel the engine allocated to the device input.
     * Note: This returns null if the engine is built with RECORD_DEVICE_INPUT
     * set to false.
     *
     * @return {AudioChannel}
     */
    public AudioChannel getInputChannel()
    {
        return MWEngineCore.getInputChannel();
    }

    public void setBouncing( boolean value, String outputDirectory )
    {
        setBouncing(value, outputDirectory, calculateRecordingSnippetBufferSize());
    }

    public void setBouncing( boolean value, String outputFile, int maxRecordBuffers )
    {
        _sequencerController.setBounceState( value, maxRecordBuffers, outputFile );
    }

    /**
     * Records the audio coming in from the Android device input.
     * Requires RECORD_DEVICE_INPUT to be enabled in global.h as well as the
     * appropriate permissions defined in the AndroidManifest and granted by the user at runtime.
     *
     * In order to record the input directly to device storage, @see setRecordFromDeviceInputState()
     *
     * @param record {boolean}
     */
    public void recordInput( boolean record )
    {
        MWEngineCore.recordInput( record );
    }

    /**
     * Records the audio output of the engine and writes it onto the Android
     * device's storage as a WAV file. Note this keeps recording until setRecordingState() is
     * invoked again with value false. Additionally, the sequencer must be running!
     *
     * Requires RECORD_TO_DISK to be enabled in global.h as well as the
     * appropriate permissions defined in the AndroidManifest and granted by the user at runtime.
     *
     * @param value {boolean} toggle the recording state on/off
     * @param outputFile {string} name of the WAV file to create and write the recording into
     */
    public void setRecordingState( boolean value, String outputFile )
    {
        int maxRecordBuffers = 0;

        // create / reset the recorded buffer when
        // hitting the record button
        if ( value )
            maxRecordBuffers = calculateRecordingSnippetBufferSize();

        _sequencerController.setRecordingState( value, maxRecordBuffers, outputFile );
    }

    /**
     * Records the audio coming in from the Android device input onto the Android
     * device's storage. Note this can also be done while the engine is running a sequence /
     * synthesizing live events. Given outputFile will contain recorded .WAV data with a buffer length
     * represented by given maxDurationInMilliSeconds
     *
     * Requires RECORD_DEVICE_INPUT to be enabled in global.h as well as the
     * appropriate permissions defined in the AndroidManifest and granted by the user at runtime.
     *
     * @param value {boolean} toggle the recording state on/off
     * @param outputFile {string} name of the WAV file to create and write the recording into
     * @param maxDurationInMilliSeconds {int} the size (in milliseconds) of each individual written buffer
     */
    public void setRecordFromDeviceInputState( boolean value, String outputFile, int maxDurationInMilliSeconds )
    {
        int maxRecordBuffers = 0;

        // create / reset the recorded buffer when
        // hitting the record button

        if ( value )
            maxRecordBuffers = BufferUtility.millisecondsToBuffer( maxDurationInMilliSeconds, SAMPLE_RATE );

        _sequencerController.setRecordingFromDeviceState( value, maxRecordBuffers, outputFile );
    }

    /**
     * Invoke when RECORDED_SNIPPET_READY fires. This will write an in-memory audio recording
     * snippet onto device storage. Execute as soon as notification as fired for continuous recording,
     * invoke from a different thread than the audio rendering thread to prevent buffer under runs.
     *
     * @param snippetBufferIndex {int}
     */
    public void saveRecordedSnippet( int snippetBufferIndex )
    {
        _sequencerController.saveRecordedSnippet( snippetBufferIndex );
    }

    public void reset()
    {
        MWEngineCore.reset();
        _nativeEngineRetries = 0;
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
        return ++_nativeEngineRetries < 5;
    }

    /**
     * Invoke whenever you want to destroy MWEngine
     * This halts the audio rendering and stops the Thread
     */
    public void dispose()
    {
        _disposed  = true;
        _isRunning = false;

        pause();
        reset();

        INSTANCE = null;
    }

    /* threading */

    @Override
    public void start()
    {
        if ( !_isRunning )
        {
            if ( !_threadStarted )
                super.start();

            _threadStarted = true;
            _isRunning     = true;
        }
        else {
            unpause();
        }
    }

    /**
     * invoke when the application suspends, this
     * halts the execution of the audio rendering and causes the
     * Thread to free CPU resources
     */
    public void pause()
    {
        _paused = true;

        // halt the audio rendering in the native layer of the engine
        if ( _nativeEngineRunning )
            MWEngineCore.stop();
    }

    /**
     * invoke when the application regains focus
     */
    public void unpause()
    {
        _paused = false;

        synchronized ( _pauseLock ) {
            _pauseLock.notify();
        }
    }

    public boolean isPaused()
    {
        return _paused;
    }

    public void run()
    {
        Log.d( "MWENGINE", "starting MWEngine render thread" );

        android.os.Process.setThreadPriority( Process.THREAD_PRIORITY_URGENT_AUDIO );
        handleThreadStartTimeout();

        while ( _isRunning )
        {
            // start the native rendering thread
            if ( !_paused && !_nativeEngineRunning )
            {
                Log.d( "MWENGINE", "starting native audio rendering thread @ " + SAMPLE_RATE + " Hz using " + BUFFER_SIZE + " samples per buffer" );

                _nativeEngineRunning = true;
                MWEngineCore.start();
            }

            // the remainder of this function body is blocked
            // as long as the native thread is running, getting here
            // implies engine has been stopped (see pause()) or an
            // error occurred

            _nativeEngineRunning = false;

            Log.d( "MWENGINE", "native audio rendering thread halted" );

            if ( _paused && !_disposed ) {

                // slight timeout to avoid deadlocks when attempting to lock

                try {
                    Thread.sleep(50L);
                }
                catch ( Exception e ) {}

                // obtain lock and wait until it is released by unpause()

                synchronized ( _pauseLock ) {
                    while ( _paused ) {
                        try {
                            _pauseLock.wait();
                        }
                        catch ( InterruptedException e ) {}
                    }
                }
            }
        }
    }

    /* helper functions */

    private int calculateRecordingSnippetBufferSize()
    {
        // we divide a recording into 15 second snippets (these are combined when recording finishes)
        final double amountOfMinutes = .25;

        // convert milliseconds to sample buffer size
        return ( int ) (( amountOfMinutes * 60000 ) * ( SAMPLE_RATE / 1000 ));
    }

    /**
     * rare bug : occasionally the audio engine won't start, closing / reopening
     * the application tends to work....
     *
     * this poor man's check checks whether the bridge has submitted its connection
     * message from the native layer after a short timeout
     */
    private void handleThreadStartTimeout()
    {
        if ( !_nativeEngineRunning )
        {
            final Handler handler = new Handler( _context.getMainLooper() );
            handler.postDelayed( new Runnable()
            {
                public void run()
                {
                    if ( !_disposed && !_nativeEngineRunning)
                        _observer.handleNotification( Notifications.ids.ERROR_THREAD_START.ordinal() );
                }

            }, 2000 );
        }
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
     * javap -s -private -classpath classes nl.igorski.lib.audio.MWEngine
     */
    public static void handleBridgeConnected( int aSomething )
    {
        if ( INSTANCE._observer != null )
            INSTANCE._observer.handleNotification( Notifications.ids.STATUS_BRIDGE_CONNECTED.ordinal() );
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

    public static void handleTempoUpdated( float aNewTempo )
    {
        // weird bug where on initial start the sequencer would not know the step range...

        if ( INSTANCE._initialCreation )
        {
            INSTANCE._initialCreation = false;
            INSTANCE.getSequencerController().setLoopRange(
                    0, INSTANCE.getSequencerController().getSamplesPerBar() - 1
            );
        }
    }
}
