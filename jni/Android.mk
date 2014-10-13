LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE      := mwengine
LOCAL_C_INCLUDES  := $(LOCAL_PATH)
LOCAL_CFLAGS      := -O3
LOCAL_CPPFLAGS    := $(LOCAL_CFLAGS)
LOCAL_SRC_FILES   := \
audioengine.cpp \
java_interface_wrap.cpp \
javabridge.cpp \
global.cpp \
utils.cpp \
opensl_io.c \
adsr.cpp \
audiobuffer.cpp \
audiochannel.cpp \
arpeggiator.cpp \
events/baseaudioevent.cpp \
events/basecacheableaudioevent.cpp \
events/basesynthevent.cpp \
events/drumevent.cpp \
events/sampleevent.cpp \
events/synthevent.cpp \
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
bufferutility.cpp \
bulkcacher.cpp \
diskwriter.cpp \
drumpattern.cpp \
envelopefollower.cpp \
lfo.cpp \
observer.cpp \
processingchain.cpp \
ringbuffer.cpp \
samplemanager.cpp \
sequencer.cpp \
sequencer_api.cpp \
baseinstrument.cpp \
druminstrument.cpp \
synthinstrument.cpp \
wavewriter.cpp \
routeableoscillator.cpp \

LOCAL_LDLIBS := -llog -lOpenSLES

include $(BUILD_SHARED_LIBRARY)
