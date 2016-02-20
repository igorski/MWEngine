/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2015 Igor Zinken - http://www.igorski.nl
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
import android.util.Log;
import nl.igorski.lib.audio.nativeaudio.*;

/**
 * Created by IntelliJ IDEA.
 * User: igorzinken
 * Date: 10-04-12
 * Time: 20:16
 * To change this template use File | Settings | File Templates.
 */
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
         * supported notification identifiers :
         *
         * ERROR_HARDWARE_UNAVAILABLE fired when MWEngine cannot connect to audio hardware (fatal)
         * ERROR_THREAD_START         fired when MWEngine cannot start the rendering thread (fatal)
         * STATUS_BRIDGE_CONNECTED    fired when MWEngine connects to the native layer code through JNI
         * MARKER_POSITION_REACHED    fired when request Sequencer marker position has been reached
         */
        void handleNotification( int aNotificationId );

        /**
         * invoked whenever the engine broadcasts a notification
         *
         * @param aNotificationId {int} unique identifier for the notification
         * @param aNotificationValue {int} payload for the notification
         *
         * supported notiifcations identifiers :
         *
         * SEQUENCER_POSITION_UPDATED fired when Sequencer has advanced a step, payload describes
         *                            the precise buffer offset of the Sequencer when the notification fired
         *                            (as a value in the range of 0 - BUFFER_SIZE)
         * RECORDING_STATE_UPDATED    fired when a recording snippet of request size has been written
         *                            to the output folder, payload contains snippet number
         * BOUNCE_COMPLETE            fired when the offline bouncing of the Sequencer range has completed
         */
        void handleNotification( int aNotificationId, int aNotificationValue );
    }

    private static MWEngine INSTANCE;
    private IObserver       _observer;
    private Context         _context;

    private SequencerController _sequencerController;

    /* audio generation related, should be overridden to match device-specific values */

    public static int SAMPLE_RATE = 44100;
    public static int BUFFER_SIZE = 2048;

    // we CAN multiply the output the volume to decrease it, preventing rapidly distorting audio ( especially on filters )
    private static final float VOLUME_MULTIPLIER = .85f;
    private static float       _volume           = .85f /* assumed default level */ * VOLUME_MULTIPLIER;

    /* recording buffer specific */

    private boolean _recordOutput = false;

    /* engine / thread states */

    private boolean _openSLrunning     = false;
    private int     _openSLRetry       = 0;
    private boolean _initialCreation   = true;
    private boolean _disposed          = false;
    private boolean _threadStarted     = false;
    private boolean _isRunning         = false;
    private boolean _paused            = false;
    private final Object _pauseLock;

    /**
     * The Java-side bridge to manage all native layer components
     * of the MWEngine audio engine
     *
     * @param aContext {Context} current application context
     * @param aObserver {MWEngine.IObserver} observer that will monitor engine states
     */
    public MWEngine( Context aContext, IObserver aObserver )
    {
        INSTANCE   = this;
        _context   = aContext;
        _observer  = aObserver;
        _pauseLock = new Object();

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

        // start w/ default of 120 BPM in 4/4 time

        _sequencerController = new SequencerController();
        _sequencerController.prepare( BUFFER_SIZE, SAMPLE_RATE, 120.0f, 4, 4 );

        _disposed = false;
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
        _sequencerController.setBounceState(value, calculateMaxBuffers(), outputDirectory);
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

        _disposed      = true;
        _openSLrunning = false;

        MWEngineCore.stop();       // halt the Native audio thread
        //_isRunning     = false;  // nope, we won't halt this thread (pooled)
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
        else
        {
            initJNI();  // update reference to this Java object in JNI
            unpause();
        }
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

    /**
     * invoke when the application regains focus
     */
    public void unpause()
    {
        initJNI();

        synchronized ( _pauseLock )
        {
            _paused = false;
            _pauseLock.notifyAll();
        }
    }

    public boolean isPaused()
    {
        return _paused;
    }

    public void run()
    {
        //DebugTool.log( "MWEngine STARTING NATIVE AUDIO RENDER THREAD" );

        handleThreadStartTimeout();

        while ( _isRunning )
        {
            // start the Native audio thread
            if ( !_openSLrunning )
            {
                Log.d( "MWENGINE", "STARTING NATIVE THREAD @ " + SAMPLE_RATE + " Hz using " + BUFFER_SIZE + " samples per buffer" );

                _openSLrunning = true;
                MWEngineCore.start();
            }

            // the remainder of this function body is actually blocked
            // as long as the native thread is running

            Log.d( "MWENGINE", "MWEngine THREAD STOPPED");

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

    /* helper functions */

    private int calculateMaxBuffers()
    {
        // we record a maximum of 30 seconds before invoking the "handleRecordingUpdate"-method on the sequencer
        final double amountOfMinutes = .5;

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
        if ( !_openSLrunning )
        {
            final Handler handler = new Handler( _context.getMainLooper() );
            handler.postDelayed( new Runnable()
            {
                public void run()
                {
                    if ( !_disposed && !_openSLrunning )
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
