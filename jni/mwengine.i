/**
 * this file describes which native classes will
 * be made available to the Java using a SWIG wrapper
 * note the audio engine itself is not directly available
 * but only through javajni.h and sequencer_api.h
 */
%module NativeAudioEngine

%{
#include "javabridge_api.h"
#include "sequencer_api.h"
#include "arpeggiator.h"
#include "audiochannel.h"
#include "bitcrusher.h"
#include "baseprocessor.h"
#include "bufferutility.h"
#include "bulkcacher.h"
#include "decimator.h"
#include "sampleevent.h"
#include "delay.h"
#include "drumevent.h"
#include "drumpattern.h"
#include "druminstrument.h"
#include "filter.h"
#include "fm.h"
#include "formantfilter.h"
#include "lfo.h"
#include "phaser.h"
#include "processingchain.h"
#include "routeableoscillator.h"
#include "samplemanager.h"
#include "synthinstrument.h"
#include "synthevent.h"
#include "waveshaper.h"
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
%include carrays.i                 // enable passing of arrays via JNI
%array_functions(int, int_array)   // int arrays
%include "javabridge_api.h"
%include "sequencer_api.h"
%include "arpeggiator.h"
%include "audiochannel.h"
%include "baseaudioevent.h"
%include "basecacheableaudioevent.h"
%include "baseprocessor.h"
%include "bitcrusher.h"
%include "bufferutility.h"
%include "bulkcacher.h"
%include "decimator.h"
%include "sampleevent.h"
%include "drumevent.h"
%include "druminstrument.h"
%include "drumpattern.h"
%include "delay.h"
%include "filter.h"
%include "lfo.h"
%include "fm.h"
%include "formantfilter.h"
%include "phaser.h"
%include "processingchain.h"
%include "routeableoscillator.h"
%include "samplemanager.h"
%include "synthinstrument.h"
%include "synthevent.h"
%include "waveshaper.h"
