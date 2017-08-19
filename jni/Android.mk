# Experimental AAudio support, set to true when building for AAudio (requires NDK target 26)
BUILD_AAUDIO = false

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

# unit test-specific or library creation specific builds

ifeq ($(MW_BUILD_TYPE),test)
    LOCAL_MODULE      := mwengine_test
    LOCAL_CPPFLAGS    := $(LOCAL_CFLAGS) -D=MOCK_ENGINE
else
    LOCAL_MODULE      := mwengine
endif

# shared configurations

LOCAL_C_INCLUDES  := $(LOCAL_PATH)
LOCAL_CFLAGS      := -O3
LOCAL_CPPFLAGS    := $(LOCAL_CFLAGS)

# source files

LOCAL_SRC_FILES   := \
jni/java_interface_wrap.cpp \
jni/javabridge.cpp \
jni/javautilities.cpp \
global.cpp \
drivers/adapter.cpp \
drivers/opensl_io.c \
utilities/utils.cpp \
audioengine.cpp \
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

ifeq ($(BUILD_AAUDIO),true)
    LOCAL_SRC_FILES   += \
    drivers/aaudio_io.cpp \

    LOCAL_LDLIBS := -laaudio
endif

LOCAL_LDLIBS += -lOpenSLES -landroid -latomic -llog

include $(BUILD_SHARED_LIBRARY)

# when creating a unit test build, create test runner as instantly executable file

ifeq ($(MW_BUILD_TYPE),test)
    include $(CLEAR_VARS)
    LOCAL_MODULE           := mwengine_unittest
    LOCAL_SRC_FILES        := tests/main.cpp
    LOCAL_SHARED_LIBRARIES := mwengine_test
    LOCAL_STATIC_LIBRARIES := googletest_main
    include $(BUILD_EXECUTABLE)

    $(call import-module,third_party/googletest)
endif
