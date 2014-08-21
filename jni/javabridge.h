/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2014 Igor Zinken - http://www.igorski.nl
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * JavaBridge acts as the mediator between the native C(++) layer
 * audio engine and a Java application, note that the constants
 * inside JavaAPIs must match the method signature present inside
 * JAVA_CLASS make sure obfuscation leaves these intact in
 * production builds!
 */
#ifndef JAVA_BRIDGE_H
#define JAVA_BRIDGE_H

#include <jni.h>
#include "utils.h"

/* this is the classpath to the Java class the MWEngine communicates with */

#define JAVA_CLASS "nl/igorski/lib/audio/renderer/NativeAudioRenderer"

/* method signatures of aforementioned mediator JAVA_CLASS */

typedef struct {
   char const* method;
   char const* signature;
} javaAPI;

namespace JavaAPIs
{
    const javaAPI REGISTRATION_SUCCESS        = { "handleBridgeConnected",         "(I)V" };
    const javaAPI TEMPO_UPDATED               = { "handleTempoUpdated",            "(FIIIII)V" };
    const javaAPI SEQUENCER_STEP_UPDATE       = { "handleSequencerPositionUpdate", "(I)V" };
    const javaAPI RECORDING_UPDATE            = { "handleRecordingUpdate",         "(I)V" };
    const javaAPI BOUNCE_COMPLETED            = { "handleBounceComplete",          "(I)V" };
    const javaAPI OPENSL_INITIALIZATION_ERROR = { "handleOpenSLError",             "()V"  };
}

namespace JavaBridge
{
    void registerVM( JavaVM* aVM );
    void registerInterface( JNIEnv* env, jobject jobj );

    // these methods are used to retrieve references to :

    jclass getJavaInterface();                      // the Java class acting as the JNI message mediator
    JNIEnv* getEnvironment();                       // the Java environment
    JavaVM* getVM();                                // the Java VM
    jmethodID getJavaMethod( javaAPI aAPImethod );  // a method of the Java interface
}

#endif