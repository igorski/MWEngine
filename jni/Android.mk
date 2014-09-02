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
baseaudioevent.cpp \
basecacheableaudioevent.cpp \
baseprocessor.cpp \
bitcrusher.cpp \
bufferutility.cpp \
bulkcacher.cpp \
decimator.cpp \
diskwriter.cpp \
drumevent.cpp \
druminstrument.cpp \
drumpattern.cpp \
delay.cpp \
envelopefollower.cpp \
filter.cpp \
finalizer.cpp \
fm.cpp \
formantfilter.cpp \
lfo.cpp \
limiter.cpp \
lpfhpfilter.cpp \
observer.cpp \
phaser.cpp \
processingchain.cpp \
ringbuffer.cpp \
samplemanager.cpp \
sequencer.cpp \
sequencer_api.cpp \
sampleevent.cpp \
synthevent.cpp \
synthinstrument.cpp \
waveshaper.cpp \
wavewriter.cpp \
routeableoscillator.cpp \

LOCAL_LDLIBS := -llog -lOpenSLES

include $(BUILD_SHARED_LIBRARY)
