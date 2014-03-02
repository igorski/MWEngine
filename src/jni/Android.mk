LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE      := native_audio
LOCAL_C_INCLUDES  := $(LOCAL_PATH)
LOCAL_CFLAGS      := -O3
LOCAL_CPPFLAGS    := $(LOCAL_CFLAGS)
LOCAL_SRC_FILES   := \
chunkware/SimpleEnvelope.cpp \
chunkware/SimpleGate.cpp \
chunkware/SimpleComp.cpp \
chunkware/SimpleLimit.cpp \
java_interface_wrap.cpp \
global.cpp \
utils.cpp \
native_audio_engine.cpp \
java_bridge.cpp \
opensl_io.c \
audiochannel.cpp \
baseaudioevent.cpp \
basecacheableaudioevent.cpp \
baseprocessor.cpp \
basebusprocessor.cpp \
bitcrusher.cpp \
bulkcacher.cpp \
compressor.cpp \
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
formant_filter.cpp \
lfo.cpp \
limiter.cpp \
lpfhpfilter.cpp \
phaser.cpp \
pitchshifter.cpp \
processingchain.cpp \
ringbuffer.cpp \
samplemanager.cpp \
sequencer.cpp \
sequencer_api.cpp \
samplebuffer.cpp \
sampleevent.cpp \
synthevent.cpp \
synthinstrument.cpp \
waveshaper.cpp \
wavewriter.cpp \
routeable_oscillator.cpp \

LOCAL_LDLIBS := -llog -lOpenSLES

include $(BUILD_SHARED_LIBRARY)
