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
#ifndef JAVABRIDGE_API_H_INCLUDED
#define JAVABRIDGE_API_H_INCLUDED

#include "javabridge.h"
#include "processingchain.h"

/**
 * javabridge_api.h is used to establish a two-way communication (together with javabridge.h)
 * to allow the AudioEngine to send messages to the Java VM, if you do not need
 * to send messages TO Java and are using the engine only in a native environment,
 * simple omit adding this header file in the "native_audio_lib.i"-file which
 * describes the SWIG-enabled classes for the JNI environment
 */

/**
 * these are the same methods as declared in the AudioEngine namespace, but
 * re-declared so we can call them from Java without having to resort to
 * go through a lot of trouble declaring the JNIEnv* en jobject arguments
 * which otherwise wouldn't survive the SWIG wrapping...
 */
extern "C"
{
    void init();
    void start();
    void stop();
    void reset();
    ProcessingChain* getMasterBusProcessors();
}

#endif