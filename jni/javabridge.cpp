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
#include "javabridge.h"
#include "audioengine.h"
#include <string.h>

static JavaVM*  _vm    = 0;
static jclass   _class = 0; // cached reference to Java mediator class

/**
 * called when the Java VM has finished
 * loading this native library
 */
jint JNI_OnLoad( JavaVM* vm, void* reserved )
{
    JNIEnv* env;

    if ( vm->GetEnv(( void** ) &env, JNI_VERSION_1_6 ) != JNI_OK )
        return -1;

    JavaBridge::registerVM( vm );

    DebugTool::log( "JNI INITED OK" );
    return JNI_VERSION_1_6;
}

/**
 * registers the reference to the JAVA_CLASS (and its host environment)
 * where all subsequent calls will be executed upon, the Java class
 * will only expose static methods for its interface with the native code
 *
 * upon registration the current BUFFER_SIZE will be returned to the JNI
 */
void JavaBridge::registerInterface( JNIEnv* env, jobject jobj )
{
    jclass localRefCls = env->FindClass( JAVA_CLASS );
    if (localRefCls == NULL) {
        return; /* exception thrown */
    }
    /* Create a global reference */
    _class = ( jclass ) env->NewGlobalRef( localRefCls );

    /* The local reference is no longer useful */
    env->DeleteLocalRef( localRefCls );

    /* Is the global reference created successfully? */
    if ( _class == NULL) {
        return; /* out of memory exception thrown */
    }

    jmethodID native_method_id = getJavaMethod( JavaAPIs::REGISTRATION_SUCCESS );

    if ( native_method_id != 0 )
        env->CallStaticVoidMethod( getJavaInterface(), native_method_id, 1 );
}

/**
 * the reference to the JavaVM should be registered
 * when the JNI environment has loaded, the JVM is
 * queried for all subsequent JNI communications with the
 * mediator Java class
 */
void JavaBridge::registerVM( JavaVM* aVM )
{
    _vm = aVM;
}

JavaVM* JavaBridge::getVM()
{
    return _vm;
}

/**
 * retrieve a Java method ID from the registered Java interface class
 * note: all these methods are expected to be static Java methods
 *
 * @param aAPImethod {javaAPI}
 */
jmethodID JavaBridge::getJavaMethod( javaAPI aAPImethod )
{
    jmethodID native_method_id = 0;
    jclass    javaClass        = getJavaInterface();

    if ( javaClass != 0 )
        native_method_id = getEnvironment()->GetStaticMethodID( javaClass, aAPImethod.method, aAPImethod.signature );

    return native_method_id;
}

jclass JavaBridge::getJavaInterface()
{
    JNIEnv *env = getEnvironment();

    // might as well return as subsequent usage of
    // the interface requires a valid environment for it's invocation!
    if ( env == 0 )
        return 0;

    return _class; // we have a cached reference!

    /*
    jclass javaClass = env->FindClass( JAVA_CLASS );

    if ( javaClass != 0 )
        return javaClass;
    else
        return 0;
    */
}

JNIEnv* JavaBridge::getEnvironment()
{
    JNIEnv *env;
    jint rs = _vm->AttachCurrentThread( &env, NULL );

    if ( rs == JNI_OK )
        return env;
    else
        return 0;
    /*
    JNIEnv* env;
    ( getVM() )->GetEnv(( void** ) &env, JNI_VERSION_1_6 );

    return env;
    */
}
