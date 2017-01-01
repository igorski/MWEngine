LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

ifeq ($(MW_BUILD_TYPE),test)

  include $(LOCAL_PATH)/Android_test.mk

else

    LOCAL_MODULE      := mwengine
    LOCAL_C_INCLUDES  := $(LOCAL_PATH)
    LOCAL_CFLAGS      := -O3
    LOCAL_CPPFLAGS    := $(LOCAL_CFLAGS)
    LOCAL_SRC_FILES   := \
    jni/java_interface_wrap.cpp \
    jni/javabridge.cpp \
    jni/javautilities.cpp \
    global.cpp \
    utilities/utils.cpp \
    audioengine.cpp \
    opensl_io.c \
    audiobuffer.cpp \
    audiochannel.cpp \
    instruments/baseinstrument.cpp \
    instruments/druminstrument.cpp \
    instruments/sampledinstrument.cpp \
    instruments/synthinstrument.cpp \
    instruments/oscillatorproperties.cpp \
    drumpattern.cpp \
    events/baseaudioevent.cpp \
    events/basecacheableaudioevent.cpp \
    events/drumevent.cpp \
    events/basesynthevent.cpp \
    events/synthevent.cpp \
    events/sampleevent.cpp \
    processors/baseprocessor.cpp \
    processors/bitcrusher.cpp \
    processors/dcoffsetfilter.cpp \
    processors/decimator.cpp \
    processors/delay.cpp \
    processors/filter.cpp \
    processors/finalizer.cpp \
    processors/flanger.cpp \
    processors/fm.cpp \
    processors/formantfilter.cpp \
    processors/glitcher.cpp \
    processors/limiter.cpp \
    processors/lowpassfilter.cpp \
    processors/lpfhpfilter.cpp \
    processors/phaser.cpp \
    processors/pitchshifter.cpp \
    processors/tremolo.cpp \
    processors/waveshaper.cpp \
    generators/envelopegenerator.cpp \
    generators/wavegenerator.cpp \
    generators/synthesizer.cpp \
    utilities/bufferutility.cpp \
    utilities/levelutility.cpp \
    utilities/bulkcacher.cpp \
    utilities/diskwriter.cpp \
    processingchain.cpp \
    ringbuffer.cpp \
    utilities/debug.cpp \
    utilities/samplemanager.cpp \
    utilities/bufferpool.cpp \
    utilities/tablepool.cpp \
    sequencer.cpp \
    sequencercontroller.cpp \
    wavetable.cpp \
    utilities/fastmath.cpp \
    utilities/wavereader.cpp \
    utilities/wavewriter.cpp \
    messaging/notifier.cpp \
    messaging/observer.cpp \
    modules/adsr.cpp \
    modules/arpeggiator.cpp \
    modules/envelopefollower.cpp \
    modules/lfo.cpp \
    modules/routeableoscillator.cpp \

    LOCAL_LDLIBS := -llog -lOpenSLES -latomic -landroid

    include $(BUILD_SHARED_LIBRARY)

endif
