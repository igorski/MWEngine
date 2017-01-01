/**
 * this file describes which native classes will
 * be made available to the Java using a SWIG wrapper
 * NOTE : the audio engine itself is not directly available
 * but only through javajni.h and sequencercontroller.h which are
 * wrapped in the nl.igorski.lib.audio.MWEngine class
 *
 * optional Java-specific utilities can be included by adding
 * the respective files from the jni/-folder (be sure to add
 * these to Android.mk to ensure they are built into the library)
 */
%module MWEngineCore

%{
#include "jni/javabridge_api.h"
#include "jni/javautilities.h"
#include "definitions/notifications.h"
#include "definitions/waveforms.h"
#include "audiochannel.h"
#include "processingchain.h"
#include "processors/bitcrusher.h"
#include "processors/baseprocessor.h"
#include "processors/decimator.h"
#include "processors/delay.h"
#include "processors/filter.h"
#include "processors/flanger.h"
#include "processors/limiter.h"
#include "processors/finalizer.h"
#include "processors/fm.h"
#include "processors/formantfilter.h"
#include "processors/glitcher.h"
#include "processors/lowpassfilter.h"
#include "processors/lpfhpfilter.h"
#include "processors/phaser.h"
#include "processors/pitchshifter.h"
#include "processors/tremolo.h"
#include "processors/waveshaper.h"
#include "utilities/bufferutility.h"
#include "utilities/bulkcacher.h"
#include "utilities/levelutility.h"
#include "drumpattern.h"
#include "modules/adsr.h"
#include "modules/arpeggiator.h"
#include "modules/lfo.h"
#include "modules/routeableoscillator.h"
#include "utilities/samplemanager.h"
#include "instruments/baseinstrument.h"
#include "instruments/druminstrument.h"
#include "instruments/sampledinstrument.h"
#include "instruments/synthinstrument.h"
#include "instruments/oscillatorproperties.h"
#include "events/sampleevent.h"
#include "events/drumevent.h"
#include "events/basecacheableaudioevent.h"
#include "events/basesynthevent.h"
#include "events/synthevent.h"
#include "sequencercontroller.h"
%}

// Enable the JNI class to load the required native library.
%pragma(java) jniclasscode=%{
  static {
    try {
        java.lang.System.loadLibrary( "mwengine" );
    }
    catch ( UnsatisfiedLinkError e ) {
        java.lang.System.err.println( "mwengine native code library failed to load.\n" + e );
        java.lang.System.exit(1);
    }
  }
%}

// JNI specific wrappers for passing data types from Java to C++
%include "enums.swg"               // will convert enumerations to Java enums
%include carrays.i                 // enable passing of arrays via JNI
%array_functions(int, int_array)   // int arrays
%include "std_string.i"            // enables using Java Strings as std::string

%include "jni/javabridge_api.h"
%include "jni/javautilities.h"
%include "definitions/notifications.h"
%include "definitions/waveforms.h"
%include "audiochannel.h"
%include "modules/adsr.h"
%include "modules/arpeggiator.h"
%include "modules/lfo.h"
%include "modules/routeableoscillator.h"
%include "audiochannel.h"
%include "processingchain.h"
%include "processors/baseprocessor.h"
%include "processors/bitcrusher.h"
%include "processors/decimator.h"
%include "processors/delay.h"
%include "processors/filter.h"
%include "processors/flanger.h"
%include "processors/limiter.h"
%include "processors/finalizer.h"
%include "processors/lowpassfilter.h"
%include "processors/lpfhpfilter.h"
%include "processors/fm.h"
%include "processors/formantfilter.h"
%include "processors/glitcher.h"
%include "processors/phaser.h"
%include "processors/pitchshifter.h"
%include "processors/tremolo.h"
%include "processors/waveshaper.h"
%include "utilities/bufferutility.h"
%include "utilities/bulkcacher.h"
%include "utilities/levelutility.h"
%include "drumpattern.h"
%include "utilities/samplemanager.h"
%include "instruments/baseinstrument.h"
%include "instruments/druminstrument.h"
%include "instruments/sampledinstrument.h"
%include "instruments/synthinstrument.h"
%include "instruments/oscillatorproperties.h"
%include "events/baseaudioevent.h"
%include "events/basecacheableaudioevent.h"
%include "events/basesynthevent.h"
%include "events/sampleevent.h"
%include "events/drumevent.h"
%include "events/synthevent.h"
%include "sequencercontroller.h"
