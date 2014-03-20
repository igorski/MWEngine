package nl.igorski.example;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import nl.igorski.lib.audio.definitions.Pitch;
import nl.igorski.lib.audio.helpers.DevicePropertyCalculator;
import nl.igorski.lib.audio.nativeaudio.*;
import nl.igorski.lib.audio.renderer.NativeAudioRenderer;
import nl.igorski.lib.debug.Logger;

import java.util.Vector;

public class MWEngineActivity extends Activity
{
    public final String LOG_ID  = "MWENGINE";

    /**
     * IMPORTANT : when creating native layer objects through JNI it
     * is important to remember that when the Java references go out of scope
     * (and thus are finalized by the garbage collector), the SWIG interface
     * will invoke the native layer destructors. As such we hold strong
     * references to JNI Objects during the application lifetime
     */
    private SynthInstrument        _synth1;
    private SynthInstrument        _synth2;
    private Phaser                 _phaser;
    private Delay                  _delay;
    private NativeAudioRenderer    _audioRenderer;
    private Vector<BaseAudioEvent> _audioEvents;

    private boolean _sequencerPlaying = false;

    /* public methods */

    /**
     * Called when the activity is first created.
     */
    @Override
    public void onCreate( Bundle savedInstanceState )
    {
        super.onCreate( savedInstanceState );
        setContentView( R.layout.main );

        Logger.setLogTag( "MWENGINE" ); // set the log tag for easy identification in logcat

        init();
    }

    /* protected methods */

    @Override
    public void onWindowFocusChanged( boolean hasFocus )
    {
        Logger.log( "window focus changed for MWEngineActivity, has focus > " + hasFocus );

        if ( !hasFocus )
        {
            // suspending the app - stop threads to save CPU cycles

            if ( _audioRenderer != null )
            {
                _audioRenderer.pause();
                _audioRenderer.dispose();
            }
        }
        else
        {
            // returning to the app
            init();
        }
    }

    /* private methods */

    private void init()
    {
        Logger.log( "initing MWEngineActivity" );

        // STEP 1 : preparing the native audio engine

        // check if it existed as we can pool it (see windowFocusChange when app is suspended)

        final boolean engineExisted = _audioRenderer != null;

        if ( !engineExisted )
            _audioRenderer = new NativeAudioRenderer( getApplicationContext() );

        // get the recommended buffer size for this device (NOTE : lower buffer sizes may
        // provide lower latency, but make sure all buffer sizes are powers of two of
        // the recommended buffer size (overcomes glitching in buffer callbacks )
        // getting the correct sample rate upfront will omit having audio going past the system
        // resampler reducing overall latency

        final int bufferSize = DevicePropertyCalculator.getRecommendedBufferSize( getApplicationContext() );
        final int sampleRate = DevicePropertyCalculator.getRecommendedSampleRate( getApplicationContext() );

        if ( !engineExisted )
            _audioRenderer.createOutput( sampleRate, bufferSize );

        _audioRenderer.start(); // start render thread (NOTE : sequencer is still paused!)

        // STEP 2 : let's create some instruments =D

        _synth1 = new SynthInstrument();
        _synth2 = new SynthInstrument();

        _synth1.setWaveform( 2 );   // sawtooth (see global.h for enumerations)
        _synth2.setWaveform( 5 );   // pulse width modulation

        // add a phaser to synth 1
        _phaser = new Phaser( .5f, .7f, .5f, 440.f, 1600.f );
        _synth1.getProcessingChain().addProcessor( _phaser );

        // add some funky delay to synth 2
        _delay = new Delay( 250f, 2000f, .35f, .5f, 1 );
        _synth2.getProcessingChain().addBusProcessor( _delay );

        // STEP 3 : let's create some music !

        _audioEvents = new Vector<BaseAudioEvent>();    // remember : strong references!
        _audioRenderer.setTempoNow( 130.0f, 4, 4 );     // 130 BPM at 4/4 time

        // bubbly sixteenth note bass line for synth 1

        createSynthEvent( _synth1, Pitch.note( "C", 2 ),  0 );
        createSynthEvent( _synth1, Pitch.note( "C", 2 ),  1 );
        createSynthEvent( _synth1, Pitch.note( "C", 3 ),  2 );
        createSynthEvent( _synth1, Pitch.note( "C", 2 ),  3 );
        createSynthEvent( _synth1, Pitch.note( "A#", 1 ), 4 );
        createSynthEvent( _synth1, Pitch.note( "C", 2 ),  5 );
        createSynthEvent( _synth1, Pitch.note( "C", 3 ),  6 );
        createSynthEvent( _synth1, Pitch.note( "C", 2 ),  7 );
        createSynthEvent( _synth1, Pitch.note( "C", 2 ),  8 );
        createSynthEvent( _synth1, Pitch.note( "C", 2 ),  9 );
        createSynthEvent( _synth1, Pitch.note( "D#", 2 ), 10 );
        createSynthEvent( _synth1, Pitch.note( "C", 2 ),  11 );
        createSynthEvent( _synth1, Pitch.note( "A#", 1 ), 12 );
        createSynthEvent( _synth1, Pitch.note( "A#", 2 ), 13 );
        createSynthEvent( _synth1, Pitch.note( "C", 2 ),  14 );
        createSynthEvent( _synth1, Pitch.note( "C", 2 ),  15 );

        // off-beat minor seventh chord stabs for synth 2

        createSynthEvent( _synth2, Pitch.note( "C", 3 ),  4 );
        createSynthEvent( _synth2, Pitch.note( "G", 3 ),  4 );
        createSynthEvent( _synth2, Pitch.note( "A#", 3 ), 4 );
        createSynthEvent( _synth2, Pitch.note( "D#", 3 ), 4 );

        createSynthEvent( _synth2, Pitch.note( "D", 3 ), 8 );
        createSynthEvent( _synth2, Pitch.note( "A", 3 ), 8 );
        createSynthEvent( _synth2, Pitch.note( "C", 3 ), 8 );
        createSynthEvent( _synth2, Pitch.note( "F", 3 ), 8 );

        // STEP 4 : attach click handler to the play button (see main.xml layout)

        final Button playPauseButton = ( Button ) findViewById( R.id.PlayPauseButton );
        playPauseButton.setOnClickListener( new PlayClickHandler() );
    }

    private class PlayClickHandler implements View.OnClickListener
    {
        public void onClick( View v )
        {
            // start/stop the sequencer so we can toggle hearing actual output! ;)

            _sequencerPlaying = !_sequencerPlaying;
            _audioRenderer.setPlaying( _sequencerPlaying );
        }
    }

    /**
     * convenience method for creating a new SynthEvent (a "musical instruction") for a given
     * SynthInstrument, this defaults to a note of a 16th note duration in this context
     *
     * @param synth     {SynthInstrument} the instrument that is to play the note
     * @param frequency {double} frequency in Hz of the note to play
     * @param position  {int}    position the position of the note within the bar
     */
    private void createSynthEvent( SynthInstrument synth, double frequency, int position )
    {
        final int duration     = 1; // 16th note at a BAR_SUBDIVISION of 16 (see NativeAudioRenderer)
        final SynthEvent event = new SynthEvent(( float ) frequency, position, duration, synth, false );

        event.calculateBuffers();

        _audioEvents.add( event );
    }
}
