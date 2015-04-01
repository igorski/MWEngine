LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE      := mwengine
LOCAL_C_INCLUDES  := $(LOCAL_PATH)
LOCAL_CFLAGS      := -O3
LOCAL_CPPFLAGS    := $(LOCAL_CFLAGS)
LOCAL_SRC_FILES   := \
jni/java_interface_wrap.cpp \
jni/javabridge.cpp \
jni/javautilities.cpp \
audioengine.cpp \
global.cpp \
utilities/utils.cpp \
opensl_io.c \
audiobuffer.cpp \
audiochannel.cpp \
events/baseaudioevent.cpp \
events/basecacheableaudioevent.cpp \
events/basesynthevent.cpp \
events/drumevent.cpp \
events/sampleevent.cpp \
events/synthevent.cpp \
modules/adsr.cpp \
modules/arpeggiator.cpp \
modules/envelopefollower.cpp \
modules/lfo.cpp \
modules/routeableoscillator.cpp \
processors/baseprocessor.cpp \
processors/bitcrusher.cpp \
processors/dcoffsetfilter.cpp \
processors/decimator.cpp \
processors/delay.cpp \
processors/filter.cpp \
processors/finalizer.cpp \
processors/fm.cpp \
processors/formantfilter.cpp \
processors/limiter.cpp \
processors/lpfhpfilter.cpp \
processors/phaser.cpp \
processors/waveshaper.cpp \
generators/wavegenerator.cpp \
utilities/bufferutility.cpp \
utilities/bulkcacher.cpp \
utilities/diskwriter.cpp \
utilities/samplemanager.cpp \
utilities/tablepool.cpp \
utilities/wavewriter.cpp \
messaging/notifier.cpp \
messaging/observer.cpp \
drumpattern.cpp \
processingchain.cpp \
ringbuffer.cpp \
sequencer.cpp \
sequencer_api.cpp \
wavetable.cpp \
instruments/baseinstrument.cpp \
instruments/druminstrument.cpp \
instruments/sampledinstrument.cpp \
instruments/synthinstrument.cpp \

LOCAL_LDLIBS := -llog -lOpenSLES

include $(BUILD_SHARED_LIBRARY)
