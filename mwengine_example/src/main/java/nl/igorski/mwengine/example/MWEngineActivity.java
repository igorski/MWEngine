package nl.igorski.mwengine.example;

import android.Manifest;
import android.annotation.TargetApi;
import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import android.content.Context;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.Spinner;

import nl.igorski.mwengine.MWEngine;
import nl.igorski.mwengine.core.*;
import java.util.Vector;

public final class MWEngineActivity extends AppCompatActivity {
    /**
     * IMPORTANT : when creating native layer objects through JNI it
     * is important to remember that when the Java references go out of scope
     * (and thus are finalized by the garbage collector), the SWIG interface
     * will invoke the native layer destructors. As such we hold strong
     * references to JNI Objects during the application lifetime
     */
    private Limiter             _limiter;
    private LPFHPFilter         _lpfhpf;
    private SynthInstrument     _synth1;
    private SynthInstrument     _synth2;
    private SampledInstrument   _sampler;
    private Filter              _filter;
    private Phaser              _phaser;
    private Delay               _delay;
    private MWEngine            _engine;
    private SequencerController _sequencerController;
    private Vector<SynthEvent>  _synth1Events;
    private Vector<SynthEvent>  _synth2Events;
    private Vector<SampleEvent> _drumEvents;
    private SynthEvent          _liveEvent;
    private SampleEvent         _liveClapEvent;

    private boolean _sequencerPlaying = false;
    private boolean _isRecording      = false;
    private boolean _inited           = false;

    // AAudio is only supported from Android 8/Oreo onwards.
    private final boolean _supportsAAudio = android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O;
    private Drivers.types  _audioDriver   = _supportsAAudio ? Drivers.types.AAUDIO : Drivers.types.OPENSL;

    private int SAMPLE_RATE;
    private int BUFFER_SIZE;
    private final int OUTPUT_CHANNELS = 2; // 1 = mono, 2 = stereo

    private final float minFilterCutoff = 50.0f;
    private float maxFilterCutoff;

    private static final String LOG_TAG = "MWENGINE"; // logcat identifier
    private static final int PERMISSIONS_CODE = 8081981;

    // we manage two different bass patterns to demonstrate changing sequenced events at runtime

    private int _patternIndex = 0;

    private final double[] _pattern1 = new double[]{
        Pitch.note( "C", 2 ), Pitch.note( "C", 2 ), Pitch.note( "C", 3 ), Pitch.note( "C", 2 ),
        Pitch.note( "A#", 1 ), Pitch.note( "C", 2 ), Pitch.note( "C", 3 ), Pitch.note( "C", 2 ),
        Pitch.note( "C", 2 ), Pitch.note( "C", 2 ), Pitch.note( "D#", 3 ), Pitch.note( "C", 2 ),
        Pitch.note( "A#", 1 ), Pitch.note( "A#", 2 ), Pitch.note( "C", 2 ), Pitch.note( "C", 2 )
    };

    private final double[] _pattern2 = new double[]{
        Pitch.note( "C", 2 ), Pitch.note( "C", 2 ), Pitch.note( "C", 3 ), Pitch.note( "C", 2 ),
        Pitch.note( "C", 2 ), Pitch.note( "A#", 2 ), Pitch.note( "C", 2 ), Pitch.note( "C", 2 ),
        Pitch.note( "G#", 2 ), Pitch.note( "C", 2 ), Pitch.note( "C", 2 ), Pitch.note( "G", 2 ),
        Pitch.note( "C", 2 ), Pitch.note( "C", 2 ), Pitch.note( "F#", 2 ), Pitch.note( "G", 2 )
    };

    /* public methods */

    /**
     * Called when the activity is created. This also fires
     * on screen orientation changes.
     */
    @Override
    public void onCreate( Bundle savedInstanceState ) {
        super.onCreate( savedInstanceState );
        setContentView( R.layout.activity_main );

        // these may not necessarily all be required for your use case (e.g. if you're not recording
        // from device audio inputs or reading/writing files) but are here for self-documentation

        if ( android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.M ) {
            String[] PERMISSIONS = {
                Manifest.permission.RECORD_AUDIO, // RECORD_AUDIO must be granted prior to engine.start()
                Manifest.permission.READ_EXTERNAL_STORAGE,
                Manifest.permission.WRITE_EXTERNAL_STORAGE
            };
            // Check if we have all the necessary permissions, if not: prompt user
            int permission = checkSelfPermission( Manifest.permission.RECORD_AUDIO );
            if ( permission == PackageManager.PERMISSION_GRANTED )
                init();
            else
                requestPermissions( PERMISSIONS, PERMISSIONS_CODE );
        }
    }

    @TargetApi( Build.VERSION_CODES.M )
    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult( requestCode, permissions, grantResults );
        if ( requestCode != PERMISSIONS_CODE ) return;
        for ( int i = 0; i < permissions.length; i++ ) {
            String permission = permissions[ i ];
            int grantResult   = grantResults[ i ];
            if ( permission.equals( Manifest.permission.RECORD_AUDIO ) && grantResult == PackageManager.PERMISSION_GRANTED ) {
                init();
            } else {
                requestPermissions( new String[]{ Manifest.permission.RECORD_AUDIO }, PERMISSIONS_CODE );
            }
        }
    }

    /**
     * Called when screen resizes / orientation changes. We handle this manually as we do not want onDestroy()
     * to fire for this occasion (the audio engine would otherwise be disposed, which is not what we want
     * for this scenario (see :configChanges directive in AndroidManifest)
     */
    @Override
    public void onConfigurationChanged( @NonNull Configuration newConfig ) {
        super.onConfigurationChanged( newConfig );
        Log.d( LOG_TAG, "MWEngineActivity::onConfigurationChanged, new orientation: " + newConfig.orientation );
    }

    /**
     * Called when the activity is destroyed. This should not fire on screen resize/orientation
     * changes. See AndroidManifest.xml for the appropriate :configChanges directive on the activity.
     * On actual destroy, we clean up the engine's thread and memory allocated outside of the Java environment.
     */
    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.d( LOG_TAG, "MWEngineActivity::onDestroy" );
        flushSong();        // free memory allocated by song
        _engine.dispose();  // dispose the engine
    }

    private void init() {
        Log.d( LOG_TAG, "MWEngineActivity::init, had existing _inited state: " + _inited );

        if ( _inited )
            return;

        // STEP 1 : preparing the native audio engine

        _engine = new MWEngine( new StateObserver() );
        MWEngine.optimizePerformance( this );

        // get the recommended buffer size for this device (NOTE : lower buffer sizes may
        // provide lower latency, but make sure all buffer sizes are powers of two of
        // the recommended buffer size (overcomes glitching in buffer callbacks )
        // getting the correct sample rate upfront will omit having audio going past the system
        // resampler reducing overall latency

        BUFFER_SIZE = MWEngine.getRecommendedBufferSize( getApplicationContext() );
        SAMPLE_RATE = MWEngine.getRecommendedSampleRate( getApplicationContext() );

        _engine.createOutput( SAMPLE_RATE, BUFFER_SIZE, OUTPUT_CHANNELS, 1, _audioDriver );

        // STEP 2 : let's create some music !

        _synth1Events = new Vector<SynthEvent>();
        _synth2Events = new Vector<SynthEvent>();
        _drumEvents   = new Vector<SampleEvent>();

        setupSong();

        // STEP 3 : start your engine!
        // Starts engines render thread (NOTE: sequencer is still paused)
        // this ensures that audio will be output as appropriate (e.g. when
        // playing live events / starting sequencer and playing the song)

        _engine.start();

        // STEP 4 : attach event handlers to the UI elements (see main.xml layout)

        findViewById( R.id.PlayPauseButton ).setOnClickListener(( View v ) -> {
            _sequencerPlaying = !_sequencerPlaying;
            _engine.getSequencerController().setPlaying( _sequencerPlaying );
            (( Button ) v ).setText( _sequencerPlaying ? R.string.pause_btn : R.string.play_btn );
        });
        findViewById( R.id.RecordButton ).setOnClickListener(( View v ) -> {
            _isRecording = !_isRecording;
            if ( _isRecording )
                _engine.startOutputRecording( Environment.getExternalStorageDirectory().getAbsolutePath() + "/Download/mwengine_output.wav" );
            else
                _engine.stopOutputRecording();
            (( Button ) v ).setText( _isRecording ? R.string.rec_btn_off : R.string.rec_btn_on );
        });
        findViewById( R.id.LiveNoteButton ).setOnTouchListener(( View v, MotionEvent event ) -> {
            switch( event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    _liveEvent.play();
                    return true;
                case MotionEvent.ACTION_UP:
                    _liveEvent.stop(); // event will start release phase of the synthesized note
                    return true;
            }
            return false;
        });
        findViewById( R.id.LiveSampleButton ).setOnTouchListener(( View v, MotionEvent event ) -> {
            if ( event.getAction() == MotionEvent.ACTION_DOWN ) {
                _liveClapEvent.play(); // event will .stop() after playing sample in full
                return true;
            }
            return false;
        });
        findViewById( R.id.PatternSwitchButton ).setOnClickListener(( View v ) -> {
            _patternIndex = _patternIndex == 0 ? 1 : 0;
            // we allow users to dispose events while the Sequencer is running, as such we delay execution
            // until the engine has notified that it has entered its idle phase between renders (meaning:
            // it is safe to perform operations that potentially deallocate buffer memory)
            _engine.executeWhenIdle(() -> {
                createDrumPattern();
                createBassPattern();
            });
        });
        findViewById( R.id.RecordInputButton ).setOnTouchListener(( View v, MotionEvent event ) -> {
            switch( event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    _engine.recordInput( true );
                    return true;
                case MotionEvent.ACTION_UP:
                    _engine.recordInput( false );
                    return true;
            }
            return false;
        });

        (( SeekBar ) findViewById( R.id.FilterCutoffSlider )).setOnSeekBarChangeListener( new FilterCutOffChangeHandler() );
        (( SeekBar ) findViewById( R.id.SynthDecaySlider )).setOnSeekBarChangeListener( new SynthDecayChangeHandler() );
        (( SeekBar ) findViewById( R.id.MixSlider )).setOnSeekBarChangeListener( new DelayMixChangeHandler() );
        (( SeekBar ) findViewById( R.id.PitchSlider )).setOnSeekBarChangeListener( new PitchChangeHandler() );
        (( SeekBar ) findViewById( R.id.TempoSlider )).setOnSeekBarChangeListener( new TempoChangeHandler() );
        (( SeekBar ) findViewById( R.id.VolumeSlider )).setOnSeekBarChangeListener( new VolumeChangeHandler() );

        if ( !_supportsAAudio ) {
            findViewById( R.id.DriverSelection ).setVisibility( View.GONE );
        } else {
            (( Spinner ) findViewById( R.id.DriverSpinner )).setOnItemSelectedListener( new DriverChangeHandler() );
        }
        _inited = true;
    }

    /* protected methods */

    protected void setupSong() {
        _sequencerController = _engine.getSequencerController();
        _sequencerController.setTempoNow( 130.0f, 4, 4 ); // 130 BPM in 4/4 time

        int STEPS_PER_MEASURE = 16; // amount of subdivisions within a single measure
        _sequencerController.updateMeasures( 1, STEPS_PER_MEASURE); // we'll loop just a single measure with given subdivisions

        // cache some of the engines properties

        final ProcessingChain masterBus = _engine.getMasterBusProcessors();

        // Load some samples from the packaged assets folder into the SampleManager

        loadWAVAsset( "hat.wav",  "hat" );
        loadWAVAsset( "clap.wav", "clap" );

        // create a lowpass filter to catch all low rumbling and a limiter to prevent clipping of output :)

        _lpfhpf  = new LPFHPFilter(( float )  MWEngine.SAMPLE_RATE, 55, OUTPUT_CHANNELS );
        _limiter = new Limiter();

        masterBus.addProcessor( _lpfhpf );
        masterBus.addProcessor( _limiter );

        // STEP 2 : let's create some instruments =D

        _synth1  = new SynthInstrument();
        _synth2  = new SynthInstrument();
        _sampler = new SampledInstrument();

        _synth1.getOscillatorProperties( 0 ).setWaveform( 2 ); // sawtooth (see waveforms.h for enumerations)
        _synth2.getOscillatorProperties( 0 ).setWaveform( 5 ); // pulse width modulation

        // a short decay for synth 1 with a 0 sustain level (provides a bubbly effect)
        _synth1.getAdsr().setDecayTime( .1f );
        _synth1.getAdsr().setSustainLevel( 0.0f );
        // a short release for synth 2 (smooth fade out)
        _synth2.getAdsr().setReleaseTime( 0.15f );

        // add a filter to synth 1
        maxFilterCutoff = ( float ) SAMPLE_RATE / 8;

        _filter = new Filter(
            maxFilterCutoff / 2, ( float ) ( Math.sqrt( 1 ) / 2 ),
            minFilterCutoff, maxFilterCutoff, OUTPUT_CHANNELS
        );
        _synth1.getAudioChannel().getProcessingChain().addProcessor( _filter );

        // add a phaser to synth 1
        _phaser = new Phaser( .5f, .7f, .5f, 440.f, 1600.f );
        _synth1.getAudioChannel().getProcessingChain().addProcessor( _phaser );

        // add some funky delay to synth 2
        _delay = new Delay( 250, 2000, .35f, .5f, OUTPUT_CHANNELS );
        _synth2.getAudioChannel().getProcessingChain().addProcessor( _delay );

        // adjust synthesizer volumes
        _synth2.getAudioChannel().setVolume( .7f );

        // STEP 2 : Sample events to play back a drum beat

        createDrumPattern();

        // Real-time synthesis events

        // bubbly sixteenth note bass line for synth 1
        createBassPattern();

        // Off-beat minor seventh chord stabs for synth 2

        createSynthEvent( _synth2, Pitch.note( "C", 3 ),  4 );
        createSynthEvent( _synth2, Pitch.note( "G", 3 ),  4 );
        createSynthEvent( _synth2, Pitch.note( "A#", 3 ), 4 );
        createSynthEvent( _synth2, Pitch.note( "D#", 3 ), 4 );

        createSynthEvent( _synth2, Pitch.note( "D", 3 ), 8 );
        createSynthEvent( _synth2, Pitch.note( "A", 3 ), 8 );
        createSynthEvent( _synth2, Pitch.note( "C", 3 ), 8 );
        createSynthEvent( _synth2, Pitch.note( "F", 3 ), 8 );

        // a C note to be synthesized live when holding down the corresponding button

        _liveEvent     = new SynthEvent(( float ) Pitch.note( "C", 3 ), _synth2 );
        _liveClapEvent = new SampleEvent( _sampler );
        _liveClapEvent.setSample( SampleManager.getSample( "clap" ));
    }

    protected void flushSong() {
        // this ensures that Song resources currently in use by the engine are released

        Log.d( LOG_TAG, "MWEngineActivity::flushSong" );

        _engine.stop();

        // remove events from Sequencer

        for ( final BaseAudioEvent event : _synth1Events )
            event.dispose();
        for ( final BaseAudioEvent event : _synth2Events )
            event.dispose();
        for ( final BaseAudioEvent event : _drumEvents )
            event.dispose();

        // clear Vectors so all references to the events are broken (and can be garbage collected)

        _synth1Events.clear();
        _synth2Events.clear();
        _drumEvents.clear();

        // detach all processors from engine's master bus

        _engine.getMasterBusProcessors().reset();

        // unregister the instruments from the song

        _synth1.dispose();
        _synth2.dispose();
        _sampler.dispose();

        // allow these to be garbage collected (will call the native layer destructor to free
        // memory allocated to their resources (e.g. AudioChannels, Processors)

        _synth1  = null;
        _synth2  = null;
        _sampler = null;

        // and these (garbage collection invokes native layer destructors, so we'll let
        // these processors be cleared lazily)

        _filter = null;
        _phaser = null;
        _delay  = null;
        _lpfhpf = null;

        // flush sample memory allocated in the SampleManager
        SampleManager.flushSamples();
    }

    @Override
    public void onWindowFocusChanged( boolean hasFocus ) {
        Log.d( LOG_TAG, "MWEngineActivity::onWindowFocusChanged, has focus: " + hasFocus );

        if ( !hasFocus ) {
            // suspending the app - halt audio rendering in MWEngine Thread to save CPU cycles
            if ( _engine != null )
                _engine.stop();
        }
        else {
            // returning to the app
            if ( !_inited )
                init();          // initialize this example application
            else
                _engine.start(); // resumes audio rendering
        }
    }

    /* event handlers */

    private class DriverChangeHandler implements AdapterView.OnItemSelectedListener {
        public void onItemSelected( AdapterView<?> parent, View view, int pos, long id ) {
            String selectedValue = parent.getItemAtPosition( pos ).toString();
            _audioDriver = selectedValue.equalsIgnoreCase( "aaudio" ) ? Drivers.types.AAUDIO : Drivers.types.OPENSL;
            _engine.setAudioDriver( _audioDriver );
        }
        @Override
        public void onNothingSelected(AdapterView<?> arg0) {}
    }

    private class FilterCutOffChangeHandler implements SeekBar.OnSeekBarChangeListener {
        public void onProgressChanged( SeekBar seekBar, int progress, boolean fromUser ) {
            _filter.setCutoff(( progress / 100f ) * ( maxFilterCutoff - minFilterCutoff ) + minFilterCutoff );
        }
        public void onStartTrackingTouch( SeekBar seekBar ) {}
        public void onStopTrackingTouch ( SeekBar seekBar ) {}
    }

    private class SynthDecayChangeHandler implements SeekBar.OnSeekBarChangeListener {
        public void onProgressChanged( SeekBar seekBar, int progress, boolean fromUser ) {
            _synth1.getAdsr().setDecayTime( progress / 100f );
            _synth1.updateEvents(); // update all synth events to match new ADSR properties
        }
        public void onStartTrackingTouch( SeekBar seekBar ) {}
        public void onStopTrackingTouch ( SeekBar seekBar ) {}
    }

    private class DelayMixChangeHandler implements SeekBar.OnSeekBarChangeListener {
        public void onProgressChanged( SeekBar seekBar, int progress, boolean fromUser ) {
            _delay.setFeedback( progress / 100f );
        }
        public void onStartTrackingTouch( SeekBar seekBar ) {}
        public void onStopTrackingTouch ( SeekBar seekBar ) {}
    }

    private class PitchChangeHandler implements SeekBar.OnSeekBarChangeListener {
        public void onProgressChanged( SeekBar seekBar, int progress, boolean fromUser ) {
            for ( final SampleEvent drumEvent : _drumEvents )
                drumEvent.setPlaybackRate(( progress / 50f ));
        }
        public void onStartTrackingTouch( SeekBar seekBar ) {}
        public void onStopTrackingTouch ( SeekBar seekBar ) {}
    }

    private class TempoChangeHandler implements SeekBar.OnSeekBarChangeListener {
        public void onProgressChanged( SeekBar seekBar, int progress, boolean fromUser ) {
            final float minTempo = 40f;     // minimum allowed tempo is 40 BPM
            final float maxTempo = 260f;    // maximum allowed tempo is 260 BPM
            final float newTempo = ( progress / 100f ) * ( maxTempo - minTempo ) + minTempo;
            _engine.getSequencerController().setTempo( newTempo, 4, 4 ); // update to match new tempo in 4/4 time
        }
        public void onStartTrackingTouch( SeekBar seekBar ) {}
        public void onStopTrackingTouch ( SeekBar seekBar ) {}
    }

    private class VolumeChangeHandler implements SeekBar.OnSeekBarChangeListener {
        public void onProgressChanged( SeekBar seekBar, int progress, boolean fromUser ) {
            _engine.setVolume( progress / 100f );
        }
        public void onStartTrackingTouch( SeekBar seekBar ) {}
        public void onStopTrackingTouch ( SeekBar seekBar ) {}
    }

    /* state change message listener */

    private class StateObserver implements MWEngine.IObserver {
        private final Notifications.ids[] _notificationEnums = Notifications.ids.values(); // cache the enumerations (from native layer) as int Array
        public void handleNotification( final int aNotificationId ) {
            switch ( _notificationEnums[ aNotificationId ]) {
                case ERROR_HARDWARE_UNAVAILABLE:
                    Log.d( LOG_TAG, "ERROR : received driver error callback from native layer" );
                    _engine.dispose();
                    break;
                case MARKER_POSITION_REACHED:
                    Log.d( LOG_TAG, "Marker position has been reached" );
                    break;
                case RECORDING_COMPLETED:
                    Log.d( LOG_TAG, "Recording has completed" );
                    break;
            }
        }

        public void handleNotification( final int aNotificationId, final int aNotificationValue ) {
            switch ( _notificationEnums[ aNotificationId ]) {
                case SEQUENCER_POSITION_UPDATED:

                    // for this notification id, the notification value describes the precise buffer offset of the
                    // engine when the notification fired (as a value in the range of 0 - BUFFER_SIZE). using this value
                    // we can calculate the amount of samples pending until the next step position is reached
                    // which in turn allows us to calculate the engine latency

                    int sequencerPosition = _sequencerController.getStepPosition();
                    int elapsedSamples    = _sequencerController.getBufferPosition();

                    // example snippet to show how to track sequencer position. Using logcat output in a
                    // frequently firing engine callback is not recommended for performance reasons
                    // Log.d( LOG_TAG, "seq. position: " + sequencerPosition + ", buffer offset: " + aNotificationValue + ", elapsed samples: " + elapsedSamples );
                    break;
                case RECORDED_SNIPPET_READY:
                    runOnUiThread(() -> {
                        // we run the saving (I/O operations) on a different thread to prevent buffer under runs while rendering audio
                        _engine.saveRecordedSnippet( aNotificationValue ); // notification value == snippet buffer index
                    });
                    break;
                case RECORDED_SNIPPET_SAVED:
                    Log.d( LOG_TAG, "Recorded snippet " + aNotificationValue + " saved to storage" );
                    break;
            }
        }
    }

    /* private methods */

    /**
     * convenience method for creating a new SynthEvent (a "musical instruction") for a given
     * SynthInstrument, this defaults to a note of a 16th note duration in this context
     *
     * @param synth     {SynthInstrument} the instrument that is to play the note
     * @param frequency {double} frequency in Hz of the note to play
     * @param position  {int}    position the position of the note within the bar
     */
    private void createSynthEvent( SynthInstrument synth, double frequency, int position ) {
        // duration in measure subdivisions, essentially a 16th note for the current STEPS_PER_MEASURE (16)

        final int duration = 1;
        final SynthEvent event = new SynthEvent(( float ) frequency, position, duration, synth );

        if ( synth == _synth1 )
            _synth1Events.add( event );
        else
            _synth2Events.add( event );
    }

    private void createDrumPattern() {
        // clear any existing patterns (when switching)
        for ( final SampleEvent event : _drumEvents ) {
            event.dispose();
        }
        _drumEvents.clear(); // clear the old events so they can be garbage collected
        // depending on the pattern hi-hats occur on the off beats or every 16th note
        int start = _patternIndex == 0 ? 2 : 0;
        int incr  = _patternIndex == 0 ? 4 : 1;
        for ( int i = start; i < 16; i += incr ) {
            createDrumEvent( "hat",  i );
        }
        createDrumEvent( "clap", 4 );  // clap sound on the second beat of the bar
        createDrumEvent( "clap", 12 ); // clap sound on the third beat of the bar
    }

    private void createBassPattern() {
        // clear any existing patterns (when switching)
        for ( final SynthEvent event : _synth1Events ) {
            event.dispose();
        }
        _synth1Events.clear(); // clear the old events so they can be garbage collected
        // create notes for the active pattern
        double[] notes = _patternIndex == 0 ? _pattern1 : _pattern2;
        for ( int i = 0; i < notes.length; ++i ) {
            createSynthEvent( _synth1, notes[ i ], i );
        }
    }

    /**
     * convenience method for creating a new SampleEvent
     *
     * @param sampleName {String} identifier (inside the SampleManager) of the sample to use
     * @param position {int} position within the composition to place the event at
     */
    private void createDrumEvent( String sampleName, int position ) {
        final SampleEvent drumEvent = new SampleEvent( _sampler );
        drumEvent.setSample( SampleManager.getSample( sampleName ));
        drumEvent.positionEvent( 0, 16, position );
        drumEvent.addToSequencer(); // samples have to be explicitly added for playback

        _drumEvents.add( drumEvent );
    }

    /**
     * convenience method to load WAV files packaged in the APK
     * and read their audio content into MWEngine's SampleManager
     *
     * @param assetName {String} assetName filename for the resource in the /assets folder
     * @param sampleName {String} identifier for the files WAV content inside the SampleManager
     */
    private void loadWAVAsset( String assetName, String sampleName ) {
        final Context ctx = getApplicationContext();
        JavaUtilities.createSampleFromAsset(
            sampleName, ctx.getAssets(), ctx.getCacheDir().getAbsolutePath(), assetName
        );
    }
}
