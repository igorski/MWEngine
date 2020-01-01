/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2016-2020 Igor Zinken - https://www.igorski.nl
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
#ifndef __MWENGINE__MOCK_ENGINE_INCLUDED__
#define __MWENGINE__MOCK_ENGINE_INCLUDED__

/**
 * The mock engine is used during unit testing
 * to verify whether audio processing (buffer read / writes)
 * behaves appropriately.
 */
namespace MWEngine {

    class Mock_IO {
        public:
            Mock_IO();
            ~Mock_IO();

            int writeOutput( float *buffer, int size );
    };

    /**
     * Variables exposed through a static class
     * this can be read by the audioengine_test to
     * assert results
     */

    class MockData {
        public:
            static bool engine_started;
            static int test_program;
            static bool test_successful;
            static int render_iterations;
    };
}
#endif
