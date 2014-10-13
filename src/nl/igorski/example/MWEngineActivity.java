package nl.igorski.example;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.SeekBar;
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
    private Finalizer              _finalizer;
    private LPFHPFilter            _lpfhpf;
    private SynthInstrument        _synth1;
    private SynthInstrument        _synth2;
    private Filter                 _filter;
    private Phaser                 _phaser;
    private Delay                  _delay;
    private NativeAudioRenderer    _audioRenderer;
    private Vector<SynthEvent>     _synth1Events;
    private Vector<SynthEvent>     _synth2Events;

    private boolean _sequencerPlaying = false;
    private boolean _inited           = false;

    private float minFilterCutoff = 50.0f;
    private float maxFilterCutoff;

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
            if ( !_inited )
                init();
            else
                _audioRenderer.start(); // resumes audio render thread
        }
    }

    /* event handlers */

    /**
     *  invoked when user presses the play / pause button
     */
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
     *  invoked when user interacts with the filter cutoff slider
     */
    private class FilterCutOffChangeHandler implements SeekBar.OnSeekBarChangeListener
    {
        public void onProgressChanged( SeekBar seekBar, int progress, boolean fromUser )
        {
            _filter.setCutoff(( progress / 100f ) * ( maxFilterCutoff - minFilterCutoff ) + minFilterCutoff );
        }

        public void onStartTrackingTouch( SeekBar seekBar ) {

        }

        public void onStopTrackingTouch( SeekBar seekBar ) {

        }
    }

    /**
     *  invoked when user interacts with the delay mix slider
     */
    private class DelayMixChangeHandler implements SeekBar.OnSeekBarChangeListener
    {
        public void onProgressChanged( SeekBar seekBar, int progress, boolean fromUser )
        {
            _delay.setFeedback( progress / 100f );
        }

        public void onStartTrackingTouch( SeekBar seekBar ) {

        }

        public void onStopTrackingTouch( SeekBar seekBar ) {

        }
    }

    /**
     *  invoked when user interacts with the tempo slider
     */
    private class TempoChangeHandler implements SeekBar.OnSeekBarChangeListener
    {
        public void onProgressChanged( SeekBar seekBar, int progress, boolean fromUser )
        {
            final float minTempo = 40f;     // minimum allowed tempo is 40 BPM
            final float maxTempo = 260f;    // maximum allowed tempo is 260 BPM

            final float newTempo = ( progress / 100f ) * ( maxTempo - minTempo ) + minTempo;

            _audioRenderer.setTempo( newTempo, 4, 4 ); // update to match new tempo in 4/4 time

            // update all audio events (re-renders their contents to match the new tempo)

            for ( final SynthEvent audioEvent : _synth1Events )
                audioEvent.invalidateProperties( audioEvent.getPosition(), audioEvent.getLength(), _synth1 );

            for ( final SynthEvent audioEvent : _synth2Events )
                audioEvent.invalidateProperties( audioEvent.getPosition(), audioEvent.getLength(), _synth2 );
        }

        public void onStartTrackingTouch( SeekBar seekBar ) {

        }

        public void onStopTrackingTouch( SeekBar seekBar ) {

        }
    }

    /* private methods */

    private void init()
    {
        if ( _inited )
            return;

        Logger.log( "initing MWEngineActivity" );

        // STEP 1 : preparing the native audio engine

        _audioRenderer = new NativeAudioRenderer( getApplicationContext() );

        // get the recommended buffer size for this device (NOTE : lower buffer sizes may
        // provide lower latency, but make sure all buffer sizes are powers of two of
        // the recommended buffer size (overcomes glitching in buffer callbacks )
        // getting the correct sample rate upfront will omit having audio going past the system
        // resampler reducing overall latency

        final int bufferSize = DevicePropertyCalculator.getRecommendedBufferSize( getApplicationContext() );
        final int sampleRate = DevicePropertyCalculator.getRecommendedSampleRate( getApplicationContext() );

        _audioRenderer.createOutput( sampleRate, bufferSize );
        _audioRenderer.updateMeasures( 1 ); // we'll loop just a single measure
        _audioRenderer.start(); // start render thread (NOTE : sequencer is still paused!)

        final int outputChannels = 1;   // see global.h

        // create a lowpass filter to catch all low rumbling and a Finalizer (limiter) to prevent clipping of output :)
        _lpfhpf    = new LPFHPFilter(( float )  NativeAudioRenderer.SAMPLE_RATE, 55, outputChannels );
        _finalizer = new Finalizer  ( 2f, 500f, NativeAudioRenderer.SAMPLE_RATE,     outputChannels );

        final ProcessingChain masterBus = _audioRenderer.getMasterBusProcessors();
        masterBus.addProcessor( _finalizer );
        masterBus.addProcessor( _lpfhpf );

        // STEP 2 : let's create some instruments =D

        _synth1 = new SynthInstrument();
        _synth2 = new SynthInstrument();

        _synth1.setWaveform( 2 );   // sawtooth (see global.h for enumerations)
        _synth2.setWaveform( 5 );   // pulse width modulation

        // a high decay for synth 1 (bubblier effect)
        _synth1.getAdsr().setDecay( .9f );

        // add a filter to synth 1
        maxFilterCutoff = ( float ) sampleRate / 8;

        _filter = new Filter( maxFilterCutoff / 2, ( float ) ( Math.sqrt( 1 ) / 2 ), minFilterCutoff, maxFilterCutoff, 0f, 1 );
        _synth1.getProcessingChain().addProcessor( _filter );

        // add a phaser to synth 1
        _phaser = new Phaser( .5f, .7f, .5f, 440.f, 1600.f );
        _synth1.getProcessingChain().addProcessor( _phaser );

        // add some funky delay to synth 2
        _delay = new Delay( 250f, 2000f, .35f, .5f, 1 );
        _synth2.getProcessingChain().addProcessor( _delay );

        // STEP 3 : let's create some music !

        _synth1Events = new Vector<SynthEvent>();   // remember : strong references!
        _synth2Events = new Vector<SynthEvent>();   // remember : strong references!
        _audioRenderer.setTempoNow( 130.0f, 4, 4 ); // 130 BPM at 4/4 time

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

        // STEP 4 : attach event handler to the UI elements (see main.xml layout)

        final Button playPauseButton = ( Button ) findViewById( R.id.PlayPauseButton );
        playPauseButton.setOnClickListener( new PlayClickHandler() );

        final SeekBar filterSlider = ( SeekBar ) findViewById( R.id.FilterCutoffSlider );
        filterSlider.setOnSeekBarChangeListener( new FilterCutOffChangeHandler() );

        final SeekBar feedbackSlider = ( SeekBar ) findViewById( R.id.MixSlider );
        feedbackSlider.setOnSeekBarChangeListener( new DelayMixChangeHandler() );

        final SeekBar tempoSlider = ( SeekBar ) findViewById( R.id.TempoSlider );
        tempoSlider.setOnSeekBarChangeListener( new TempoChangeHandler() );

        _inited = true;
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

        if ( synth == _synth1 )
            _synth1Events.add( event );
        else
            _synth2Events.add( event );
    }
}
