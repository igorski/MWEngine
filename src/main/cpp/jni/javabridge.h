/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2018 Igor Zinken - http://www.igorski.nl
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
 * audio engine and a Java application
 */
#ifndef __MWENGINE__JAVA_BRIDGE_H
#define __MWENGINE__JAVA_BRIDGE_H

#include <jni.h>
#include <string>

namespace MWEngine {

/* this is the classpath to the Java class the MWEngine communicates with */

#define MWENGINE_JAVA_CLASS "nl/igorski/lib/audio/MWEngine"

/* method signatures of aforementioned mediator MWENGINE_JAVA_CLASS */

typedef struct {
   char const* method;
   char const* signature;
} javaAPI;

/**
 * The method signatures used to handle broadcast messages from the native
 * layer from within the MWENGINE_JAVA_CLASS.
 *
 * Make sure code obfuscation leaves these intact in production builds!
 */
namespace JavaAPIs
{
    const javaAPI REGISTRATION_SUCCESS     = { "handleBridgeConnected",         "(I)V"  };
    const javaAPI HANDLE_NOTIFICATION      = { "handleNotification",            "(I)V"  };
    const javaAPI HANDLE_NOTIFICATION_DATA = { "handleNotificationWithData",    "(II)V" };
    const javaAPI TEMPO_UPDATED            = { "handleTempoUpdated",            "(F)V"  };
}

namespace JavaBridge
{
    void registerVM( JavaVM* aVM );
    void registerInterface( JNIEnv* env, jobject jobj );

    // these methods are used to retrieve references to :

    jclass getJavaInterface();                      // the Java class acting as the JNI message mediator
    JNIEnv* getEnvironment();                       // get a pointer to the Java environment (for C to Java communication)
    JavaVM* getVM();                                // the Java VM
    jmethodID getJavaMethod( javaAPI aAPImethod );  // retrieve the identifier of a Java method

    /* convenience methods to convert Java data types to C types */

    std::string getString( jstring aString );
}

} // E.O namespace MWEngine

#endif
