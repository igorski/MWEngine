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
package nl.igorski.mwengine;

import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageManager;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.os.Build;
import android.os.Handler;
import android.os.PowerManager;
import android.os.Process;
import android.util.Log;
import android.view.WindowManager;

import nl.igorski.mwengine.core.*;

public final class MWEngine
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

    public static int SAMPLE_RATE             = 44100;
    public static int BUFFER_SIZE             = 2048;
    public static int OUTPUT_CHANNELS         = 2;
    private static Drivers.types AUDIO_DRIVER = Drivers.types.OPENSL;

    private static float _volume = 1.0f;

    /* engine / thread states */

    private boolean _initialCreation     = true;
    private boolean _disposed            = false;
    private boolean _isRunning           = false;
    private boolean _paused              = false;

    /**
     * The Java-side bridge to manage all native layer components
     * of the MWEngine audio engine
     *
     * @param aContext {Context} current application context
     * @param aObserver {MWEngine.IObserver} observer that will monitor engine states
     */
    public MWEngine( Context aContext, IObserver aObserver ) {
        if ( INSTANCE != null ) throw new Error( "MWEngine already instantiated" );

        INSTANCE   = this;
        _context   = aContext;
        _observer  = aObserver;

        MWEngineCore.init(); // registers the interface between this Java class and the native code
    }

    public static int[] cpuCores;

    /**
     * Call from your Activitity.onCreate()
     */
    public static boolean optimizePerformance( Activity activity ) {
        if ( activity.getWindow() == null ) {
            return false;
        }
        activity.getWindow().addFlags( WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON );

        if ( Build.VERSION.SDK_INT >= Build.VERSION_CODES.N ) {
            // retrieve exclusive cores to run rendering thread on
            cpuCores = Process.getExclusiveCores();
            JavaUtilities.setCpuCores( cpuCores, cpuCores.length );

            // request sustained performance mode when supported
            if ((( PowerManager ) activity.getSystemService( Context.POWER_SERVICE )).isSustainedPerformanceModeSupported()) {
                activity.getWindow().setSustainedPerformanceMode( true );
            }
            return true;
        }
        return false;
    }

    public static boolean supportsLowLatency( Context context ) {
        return context.getPackageManager().hasSystemFeature( PackageManager.FEATURE_AUDIO_LOW_LATENCY );
    }

    public static int getRecommendedSampleRate( Context context ) {
        String SR_CHECK = null;

        // API level 17 available ?  Use the sample rate provided by AudioManager.getProperty(PROPERTY_OUTPUT_SAMPLE_RATE)
        // to prevent the buffers from taking a detour through the system resampler
        if ( android.os.Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1 ) {
            SR_CHECK = ((AudioManager) context.getSystemService( Context.AUDIO_SERVICE )).getProperty( AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE );
        }
        return ( SR_CHECK != null ) ? Integer.parseInt( SR_CHECK ) : 48000;
    }

    /**
     * retrieve the recommended buffer size for the device running the application you can increase / decrease the buffer size
     * for lower latency or higher stability, but note you must use multiples of this recommendation !! Otherwise the
     * buffer callback will occasionally get two calls per timeslice which can cause glitching.
     */
    public static int getRecommendedBufferSize( Context context ) {
        String BS_CHECK = null;

        // API level 17 available ?
        if ( android.os.Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1 ) {
            BS_CHECK = (( AudioManager ) context.getSystemService( Context.AUDIO_SERVICE )).getProperty( AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER );
        }
        return ( BS_CHECK != null ) ? Integer.parseInt( BS_CHECK ) : AudioTrack.getMinBufferSize(
                getRecommendedSampleRate( context ), AudioFormat.CHANNEL_OUT_MONO, AudioFormat.ENCODING_PCM_16BIT
        );
    }

    public void createOutput( int aSampleRate, int aBufferSize, int aOutputChannels, Drivers.types driver ) {
        setup( aSampleRate, aBufferSize, aOutputChannels );
        setAudioDriver( driver );

        // start w/ default of 120 BPM in 4/4 time

        _sequencerController = new SequencerController();
        _sequencerController.prepare( 120.0f, 4, 4 );

        _disposed = false;
    }

    public void setup( int aSampleRate, int aBufferSize, int aOutputChannels ) {
        SAMPLE_RATE     = Build.FINGERPRINT.startsWith( "generic" ) ? 8000 : aSampleRate; // older emulators only work at 8 kHz
        BUFFER_SIZE     = aBufferSize;
        OUTPUT_CHANNELS = aOutputChannels;

        AudioEngine.setup( BUFFER_SIZE, SAMPLE_RATE, OUTPUT_CHANNELS );

        if ( !_isRunning ) return;

        // TODO: synchronize native layer class instances that cached above values??
        pause();   // toggling paused state of the thread will stop the engine
        unpause(); // and upon restart, initialize it with the new settings
    }

    public static MWEngine getInstance() {
        return INSTANCE;
    }

    public float getVolume() {
        return _volume;
    }

    public void setVolume( float aValue ) {
        _volume = aValue;
        _sequencerController.setVolume( _volume );
    }

    public SequencerController getSequencerController() {
        return _sequencerController;
    }

    public ProcessingChain getMasterBusProcessors() {
        return AudioEngine.getMasterBus();
    }

    public void addChannelGroup( ChannelGroup group ) {
        AudioEngine.addChannelGroup( group );
    }

    public void removeChannelGroup( ChannelGroup group ) {
        AudioEngine.removeChannelGroup( group );
    }

    /**
     * Retrieve the AudioChannel the engine allocated to the device input.
     * Note: This returns null if the engine is built with RECORD_DEVICE_INPUT
     * set to false.
     *
     * @return {AudioChannel}
     */
    public AudioChannel getInputChannel() {
        return AudioEngine.getInputChannel();
    }

    public void setBouncing( boolean value, String outputFile ) {
        setBouncing( value, outputFile, 0, AudioEngine.getAmount_of_bars() * AudioEngine.getSamples_per_bar());
    }

    public void setBouncing( boolean value, String outputFile, int rangeStart, int rangeEnd ) {
        _sequencerController.setBounceState( value, calculateRecordingSnippetBufferSize(), outputFile, rangeStart, rangeEnd );
    }

    /**
     * Records the audio coming in from the Android device input.
     * Requires RECORD_DEVICE_INPUT to be enabled in global.h as well as the
     * appropriate permissions defined in the AndroidManifest and granted by the user at runtime.
     *
     * In order to record the input directly to device storage, @see setRecordFromDeviceInputState()
     */
    public void recordInput( boolean recordingActive ) {
        AudioEngine.setRecordDeviceInput( recordingActive );
    }

    /**
     * Records the audio output of the engine and writes it onto the Android
     * device's storage as a WAV file. Note this keeps recording until setRecordingState() is
     * invoked again with value false. Additionally, the sequencer must be running!
     *
     * Requires RECORD_TO_DISK to be enabled in global.h as well as the
     * appropriate permissions defined in the AndroidManifest and granted by the user at runtime.
     *
     * @param recordingActive {boolean} toggle the recording state on/off
     * @param outputFile {string} name of the WAV file to create and write the recording into
     */
    public void setRecordingState( boolean recordingActive, String outputFile ) {
        int maxRecordBuffers = 0;

        // create / reset the recorded buffer when
        // hitting the record button
        if ( recordingActive )
            maxRecordBuffers = calculateRecordingSnippetBufferSize();

        _sequencerController.setRecordingState( recordingActive, maxRecordBuffers, outputFile );
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
     * @param recordingActive {boolean} toggle the recording state on/off
     * @param outputFile {string} name of the WAV file to create and write the recording into
     * @param maxDurationInMilliSeconds {int} the size (in milliseconds) of each individual written buffer
     */
    public void setRecordFromDeviceInputState( boolean recordingActive, String outputFile, int maxDurationInMilliSeconds ) {
        int maxRecordBuffers = 0;

        // create / reset the recorded buffer when
        // hitting the record button

        if ( recordingActive )
            maxRecordBuffers = BufferUtility.millisecondsToBuffer( maxDurationInMilliSeconds, SAMPLE_RATE );

        _sequencerController.setRecordingFromDeviceState( recordingActive, maxRecordBuffers, outputFile );
    }

    /**
     * Invoke when RECORDED_SNIPPET_READY fires. This will write an in-memory audio recording
     * snippet onto device storage. Execute as soon as notification as fired for continuous recording,
     * invoke from a different thread than the audio rendering thread to prevent buffer under runs.
     */
    public void saveRecordedSnippet( int snippetBufferIndex ) {
        _sequencerController.saveRecordedSnippet( snippetBufferIndex );
    }

    public void reset() {
        AudioEngine.reset();
    }

    public void setAudioDriver( Drivers.types audioDriver ) {
        if ( AUDIO_DRIVER == audioDriver ) return;

        AUDIO_DRIVER = audioDriver;

        if ( !_isRunning ) return;

        pause();   // toggling paused state of the thread will stop the engine
        unpause(); // and upon restart, initialize it with the new settings
    }

    /**
     * Invoke whenever you want to destroy MWEngine
     * This halts the audio rendering and stops the Thread
     */
    public void dispose() {
        _disposed  = true;
        _isRunning = false;

        pause();
        reset();

        INSTANCE = null;
    }

    /* threading */

    public void start() {
        if ( _isRunning ) return;

        Log.d( "MWENGINE", "starting native audio rendering thread @ " + SAMPLE_RATE + " Hz using " + BUFFER_SIZE + " samples per buffer" );

        MWEngineCore.init(); // (re-)register JNI interface to match current/updated JNI environment (e.g. after app suspend/focus change)
        AudioEngine.start( AUDIO_DRIVER );

        _isRunning = true;
    }

    public void stop() {
        if ( !_isRunning ) return;

        AudioEngine.stop();
        _isRunning = false;
    }

    /**
     * @deprecated use stop() instead
     */
    @Deprecated
    public void pause() {
        _paused = true;
        stop();
    }

    /**
     * @deprecated use start() instead
     */
    @Deprecated
    public void unpause() {
        _paused = false;
        start();
    }

    /* helper functions */

    private int calculateRecordingSnippetBufferSize() {
        // we divide a recording into 15 second snippets (these are combined when recording finishes)
        final double amountOfMinutes = .25;

        // convert milliseconds to sample buffer size
        return ( int ) (( amountOfMinutes * 60000 ) * ( SAMPLE_RATE / 1000 ));
    }

    /* native bridge methods */

    /**
     * All these methods are static and provide a bridge from the C++ layer code into Java. As such,
     * these methods are used by the native audio engine for updating states and requesting data.
     *
     * Java method IDs need to be supplied to C++ in order te make the callbacks, you
     * can discover the IDs by building the Java project and running the following
     * command in the output /bin folder:
     *
     * javap -s -private -classpath classes nl.igorski.mwengine.MWEngine
     */
    public static void handleBridgeConnected( int aSomething ) {
        if ( INSTANCE != null && INSTANCE._observer != null ) INSTANCE._observer.handleNotification( Notifications.ids.STATUS_BRIDGE_CONNECTED.ordinal() );
    }

    public static void handleNotification( int aNotificationId ) {
        if ( INSTANCE._observer != null ) INSTANCE._observer.handleNotification( aNotificationId );
    }

    public static void handleNotificationWithData( int aNotificationId, int aNotificationData ) {
        if ( INSTANCE._observer != null ) INSTANCE._observer.handleNotification( aNotificationId, aNotificationData );
    }

    public static void handleTempoUpdated( float aNewTempo ) {
        // weird bug where on initial start the sequencer would not know the step range...
        if ( INSTANCE._initialCreation ) {
            INSTANCE._initialCreation = false;
            INSTANCE.getSequencerController().setLoopRange( 0, INSTANCE.getSequencerController().getSamplesPerBar() - 1 );
        }
    }
}
