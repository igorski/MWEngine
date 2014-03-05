/**
 * describes the Open SL bridge interface
 * available to the Java application
 */
%module NativeAudioEngine

%{
#include "native_audio_engine_api.h"
#include "sequencer_api.h"
#include "arpeggiator.h"
#include "audiochannel.h"
#include "bitcrusher.h"
#include "baseprocessor.h"
#include "basebusprocessor.h"
#include "bulkcacher.h"
#include "decimator.h"
#include "sampleevent.h"
#include "delay.h"
#include "drumevent.h"
#include "drumpattern.h"
#include "druminstrument.h"
#include "filter.h"
#include "fm.h"
#include "formant_filter.h"
#include "lfo.h"
#include "phaser.h"
#include "processingchain.h"
#include "routeable_oscillator.h"
#include "samplemanager.h"
#include "synthinstrument.h"
#include "synthevent.h"
#include "waveshaper.h"
%}

// Enable the JNI class to load the required native library.
%pragma(java) jniclasscode=%{
  static {
    try {
        java.lang.System.loadLibrary("native_audio");
    } catch (UnsatisfiedLinkError e) {
        java.lang.System.err.println("native_audio native code library failed to load.\n" + e);
        java.lang.System.exit(1);
    }
  }
%}
%include carrays.i                 // enable passing arrays via JNI
%array_functions(int, int_array)   // int arrays
%include "native_audio_engine_api.h"
%include "sequencer_api.h"
%include "arpeggiator.h"
%include "audiochannel.h"
%include "baseaudioevent.h"
%include "basecacheableaudioevent.h"
%include "baseprocessor.h"
%include "basebusprocessor.h"
%include "bitcrusher.h"
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
%include "formant_filter.h"
%include "phaser.h"
%include "processingchain.h"
%include "routeable_oscillator.h"
%include "samplemanager.h"
%include "synthinstrument.h"
%include "synthevent.h"
%include "waveshaper.h"
