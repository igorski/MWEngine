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
#include "observer.h"
#include "audioengine.h"
#include "utils.h"

#ifndef USE_JNI

/* methods that must be overridden to use in a C++ only environment */

void Observer::handleBounceComplete( int aIdentifier )
{
    DebugTool::log( "Observer handleBounceComplete" );
};

void Observer::broadcastStepPosition()
{
    DebugTool::log( "Observer broadcastStepPosition" );
};

void Observer::broadcastTempoUpdate()
{
    DebugTool::log( "Observer broadcastTempoUpdate" );
};

void Observer::handleHardwareUnavailable()
{
    DebugTool::log( "Observer handleHardwareUnavailable" );
};

void Observer::broadcastRecordingUpdate( int aRecordingIdentifier )
{
    DebugTool::log( "Obvserver broadcastRecordingUpdate" );
};

#else

/* JNI ready callbacks to use with the Java Bridge */

#include "javabridge.h"

void Observer::broadcastStepPosition()
{
    jmethodID native_method_id = JavaBridge::getJavaMethod( JavaAPIs::SEQUENCER_STEP_UPDATE );

    if ( native_method_id != 0 )
    {
        JNIEnv* env = JavaBridge::getEnvironment();

        if ( env != 0 )
            env->CallStaticVoidMethod( JavaBridge::getJavaInterface(), native_method_id, AudioEngine::stepPosition );
    }
}

void Observer::broadcastTempoUpdate()
{
    jmethodID native_method_id = JavaBridge::getJavaMethod( JavaAPIs::TEMPO_UPDATED );

    if ( native_method_id != 0 )
    {
        JNIEnv* env = JavaBridge::getEnvironment();

        if ( env != 0 )
        {
            env->CallStaticVoidMethod( JavaBridge::getJavaInterface(), native_method_id,
                                       AudioEngine::tempo, AudioEngine::bytes_per_beat,
                                       AudioEngine::bytes_per_tick, AudioEngine::bytes_per_bar,
                                       AudioEngine::time_sig_beat_amount, AudioEngine::time_sig_beat_unit );
        }
    }
}

void Observer::handleHardwareUnavailable()
{
    jmethodID native_method_id = JavaBridge::getJavaMethod( JavaAPIs::OPENSL_INITIALIZATION_ERROR );

    if ( native_method_id != 0 )
    {
        JNIEnv* env = JavaBridge::getEnvironment();

        if ( env != 0 )
        env->CallStaticVoidMethod( JavaBridge::getJavaInterface(), native_method_id );
    }
}

void Observer::handleBounceComplete( int aIdentifier )
{
    jmethodID native_method_id = JavaBridge::getJavaMethod( JavaAPIs::BOUNCE_COMPLETED );

    if ( native_method_id != 0 )
    {
        JNIEnv* env = JavaBridge::getEnvironment();

        if ( env != 0 )
            env->CallStaticVoidMethod( JavaBridge::getJavaInterface(), native_method_id, aIdentifier );
    }
}

void Observer::broadcastRecordingUpdate( int aRecordingIdentifier )
{
    jmethodID native_method_id = JavaBridge::getJavaMethod( JavaAPIs::RECORDING_UPDATE );

    if ( native_method_id != 0 )
    {
        JNIEnv* env = JavaBridge::getEnvironment();

        if ( env != 0 )
            env->CallStaticVoidMethod( JavaBridge::getJavaInterface(), native_method_id, aRecordingIdentifier );
    }
}

#endif
