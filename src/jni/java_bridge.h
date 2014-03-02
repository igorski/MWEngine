#include <jni.h>
#include "utils.h"

typedef struct {
   char const* method;
   char const* signature;
} javaAPI;

// constants relating to the Java class and its exposed methods
#define JAVA_CLASS "nl/igorski/lib/audio/renderer/NativeAudioRenderer"

namespace JavaAPIs
{
    const javaAPI REGISTRATION_SUCCESS        = { "handleBridgeConnected",         "(I)V" };
    const javaAPI TEMPO_UPDATED               = { "handleTempoUpdated",            "(DIIIII)V" };
    const javaAPI SEQUENCER_STEP_UPDATE       = { "handleSequencerPositionUpdate", "(I)V" };
    const javaAPI RECORDING_UPDATE            = { "handleRecordingUpdate",         "(I)V" };
    const javaAPI BOUNCE_COMPLETED            = { "handleBounceComplete",          "(I)V" };
    const javaAPI OPENSL_INITIALIZATION_ERROR = { "handleOpenSLError",             "()V"  };
}

/* public methods */

void registerVM( JavaVM* aVM );
void registerInterface( JNIEnv* env, jobject jobj );
jclass getJavaInterface();                      // retrieve reference to Java class acting as the JNI message mediator
JNIEnv* getEnvironment();                       // retrieve reference to Java environment
JavaVM* getVM();                                // retrieve reference to the Java VM
jmethodID getJavaMethod( javaAPI aAPImethod );  // retrieve a method of the Java interface