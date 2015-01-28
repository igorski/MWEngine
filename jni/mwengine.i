/**
 * this file describes which native classes will
 * be made available to the Java using a SWIG wrapper
 * note: the audio engine itself is not directly available
 * but only through javajni.h and sequencer_api.h
 */
%module NativeAudioEngine

%{
#include "jni/javabridge_api.h"
#include "sequencer_api.h"
#include "adsr.h"
#include "arpeggiator.h"
#include "audiochannel.h"
#include "definitions/notifications.h"
#include "events/basesynthevent.h"
#include "events/sampleevent.h"
#include "events/drumevent.h"
#include "events/synthevent.h"
#include "lfo.h"
#include "routeableoscillator.h"
#include "processors/bitcrusher.h"
#include "processors/baseprocessor.h"
#include "processors/decimator.h"
#include "processors/delay.h"
#include "processors/filter.h"
#include "processors/limiter.h"
#include "processors/finalizer.h"
#include "processors/fm.h"
#include "processors/formantfilter.h"
#include "processors/waveshaper.h"
#include "processors/lpfhpfilter.h"
#include "processors/phaser.h"
#include "processingchain.h"
#include "utilities/samplemanager.h"
#include "utilities/bufferutility.h"
#include "utilities/bulkcacher.h"
#include "drumpattern.h"
#include "instruments/baseinstrument.h"
#include "instruments/druminstrument.h"
#include "instruments/sampledinstrument.h"
#include "instruments/synthinstrument.h"
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
%include carrays.i                 // enables passing of arrays via JNI
%array_functions(int, int_array)   // enables int arrays
%include "std_string.i"            // enables using Java Strings as std::string
#include <string>

// MWEngine library contents

%include "jni/javabridge_api.h"
%include "sequencer_api.h"
%include "adsr.h"
%include "arpeggiator.h"
%include "audiochannel.h"
%include "definitions/notifications.h"
%include "events/baseaudioevent.h"
%include "events/basecacheableaudioevent.h"
%include "events/basesynthevent.h"
%include "events/sampleevent.h"
%include "events/drumevent.h"
%include "events/synthevent.h"
%include "lfo.h"
%include "routeableoscillator.h"
%include "processors/baseprocessor.h"
%include "processors/bitcrusher.h"
%include "processors/decimator.h"
%include "processors/delay.h"
%include "processors/filter.h"
%include "processors/limiter.h"
%include "processors/finalizer.h"
%include "processors/lpfhpfilter.h"
%include "processors/fm.h"
%include "processors/formantfilter.h"
%include "processors/phaser.h"
%include "processors/waveshaper.h"
%include "processingchain.h"
%include "utilities/samplemanager.h"
%include "utilities/bufferutility.h"
%include "utilities/bulkcacher.h"
%include "instruments/baseinstrument.h"
%include "instruments/druminstrument.h"
%include "instruments/sampledinstrument.h"
%include "instruments/synthinstrument.h"
%include "drumpattern.h"
