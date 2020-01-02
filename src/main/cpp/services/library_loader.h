/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Igor Zinken - https://www.igorski.nl
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software" ), to deal in
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
#ifndef __MWENGINE__LIBRARY_LOADER_H_INCLUDED__
#define __MWENGINE__LIBRARY_LOADER_H_INCLUDED__

#include <dlfcn.h>
#include <unistd.h>
#include <definitions/libraries.h>

// convenience macro to map given function name (fn) from given library handle (lh)
// to a definition in the libraries.h file by reinterpreting the result to given signature (s)

#define MAP_AND_CAST(lh,s,fn) reinterpret_cast<s>(mapMethod(lh, fn))

namespace MWEngine {

/**
 * LibraryLoader provides a singleton class that
 * can resolve native libraries at runtime
 */
class LibraryLoader {
    public:
        static LibraryLoader* getInstance();

#ifndef INCLUDE_AAUDIO_LIBRARY
        bool loadAAudioLibrary();
#endif

    private:

        // singleton usage, hence private constructor/destructor

        LibraryLoader() {}
        ~LibraryLoader();

#ifndef INCLUDE_AAUDIO_LIBRARY
        void* _aaudioLib = nullptr;
#endif

        /**
         * Retrieve a pointer to a method described by given functionName
         * from a library at given libraryHandle
         */
        void* mapMethod( void* libraryHandle, const char* functionName ) {
            return dlsym( libraryHandle, functionName );
        }
};
} // E.O namespace MWEngine

#endif