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
#ifndef __MWENGINE__JAVABRIDGE_API_H_INCLUDED
#define __MWENGINE__JAVABRIDGE_API_H_INCLUDED

#include <jni/javabridge.h>

// javabridge_api.h is used to establish a two-way communication (together with javabridge.h)
// to allow the AudioEngine to send messages to the Java VM, if you do not need
// to send messages to the Java environment or are using the engine only within a native
// environment, simply omit adding this header file in the "mwengine.i"-file.
//
// the methods declared here are implemented within the AudioEngine class, but
// re-declared to contain JNIEnv* en jobject arguments. This intermediate header file
// fools the SWIG wrapper to not remove these.

extern "C"
{
    void init();
}

#endif