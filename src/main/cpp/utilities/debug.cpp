/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2017 Igor Zinken - http://www.igorski.nl
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
#include "debug.h"
#include "../global.h"
#include <string>

#ifdef DEBUG
#include <android/log.h>
#include <stdio.h>
#endif

/* logging */

namespace Debug
{
    /**
     * log a message into the logcat, messages can be concatenated
     * with optional extra arguments to log variable output, e.g. :
     *
     * "this is an (un)signed integer %d, this is a float/double %f"
     */
    void log( const char* aMessage, ... )
    {
#ifdef DEBUG
        /**
         * TODO can we refactor to use systrace?? Tracing is preferable to logging
         * inside the callback since tracing does not block.
         *
         * See https://developer.android.com/studio/profile/systrace-commandline.html
         */
        va_list args;
        va_start( args, aMessage );
        __android_log_vprint( ANDROID_LOG_VERBOSE, LOGTAG, aMessage, args );
        va_end( args );
#endif
    }

    void logToFile( const char* aFileName, const char* aMessage, ... )
    {
#ifdef DEBUG
        FILE* file = fopen( aFileName, "a" );

        aMessage = ( std::string( aMessage ) + std::string( "\n" )).c_str();

        va_list args;
        va_start( args, aMessage );
        vfprintf( file, aMessage, args );
        va_end( args );

        fclose( file );
#endif
    }
}
