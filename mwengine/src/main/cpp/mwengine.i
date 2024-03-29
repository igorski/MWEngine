/**
 * this file describes which native classes will
 * be made available to the Java using a SWIG wrapper
 *
 * optional Java-specific utilities can be included by adding
 * the respective files from the jni/-folder (be sure to add
 * these to Android.mk to ensure they are built into the library)
 */
%module MWEngineCore

%{

// first up include and declare use of MWEngine namespace inside generated wrapper file
#include "global.h"
using namespace MWEngine;

#include "jni/javabridge_api.h"
#include "jni/javautilities.h"
#include "definitions/drivers.h"
#include "definitions/notifications.h"
#include "definitions/pitch.h"
#include "definitions/waveforms.h"
#include "audiochannel.h"
#include "channelgroup.h"
#include "processingchain.h"
#include "processors/baseprocessor.h"
#include "processors/basedynamicsprocessor.h"
#include "processors/bitcrusher.h"
#include "processors/compressor.h"
#include "processors/dcoffsetfilter.h"
#include "processors/decimator.h"
#include "processors/delay.h"
#include "processors/filter.h"
#include "processors/flanger.h"
#include "processors/limiter.h"
#include "processors/fm.h"
#include "processors/formantfilter.h"
#include "processors/gain.h"
#include "processors/gate.h"
#include "processors/glitcher.h"
#include "processors/lowpassfilter.h"
#include "processors/lpfhpfilter.h"
#include "processors/phaser.h"
#include "processors/pitchshifter.h"
#include "processors/reverb.h"
#include "processors/reverbsm.h"
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
#include "utilities/audiorenderer.h"
#include "utilities/samplemanager.h"
#include "utilities/sampleutility.h"
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
#include "audioengine.h"
#include "sequencercontroller.h"
%}

// declare the value for the SAMPLE_TYPE typedef (defined in global.h)
// omitting this will create a custom SWIG wrapper instead of a float/double primitive
typedef double SAMPLE_TYPE;

// Enable the JNI class to load the required native library.
%pragma(java) jniclasscode=%{
  static {
    try {
        java.lang.System.loadLibrary( "mwengine_wrapped" );
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
%include "definitions/drivers.h"
%include "definitions/notifications.h"
%include "definitions/pitch.h"
%include "definitions/waveforms.h"
%include "audiochannel.h"
%include "channelgroup.h"
%include "modules/adsr.h"
%include "modules/arpeggiator.h"
%include "modules/lfo.h"
%include "modules/routeableoscillator.h"
%include "processingchain.h"
%include "processors/baseprocessor.h"
%include "processors/basedynamicsprocessor.h"
%include "processors/bitcrusher.h"
%include "processors/compressor.h"
%include "processors/dcoffsetfilter.h"
%include "processors/decimator.h"
%include "processors/delay.h"
%include "processors/filter.h"
%include "processors/flanger.h"
%include "processors/gate.h"
%include "processors/limiter.h"
%include "processors/lowpassfilter.h"
%include "processors/lpfhpfilter.h"
%include "processors/fm.h"
%include "processors/formantfilter.h"
%include "processors/gain.h"
%include "processors/glitcher.h"
%include "processors/phaser.h"
%include "processors/pitchshifter.h"
%include "processors/reverb.h"
%include "processors/reverbsm.h"
%include "processors/tremolo.h"
%include "processors/waveshaper.h"
%include "utilities/bufferutility.h"
%include "utilities/bulkcacher.h"
%include "utilities/levelutility.h"
%include "utilities/sampleutility.h"
%include "drumpattern.h"
%include "utilities/audiorenderer.h"
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
%include "audioengine.h"
%include "sequencercontroller.h"
