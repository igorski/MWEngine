/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Igor Zinken - http://www.igorski.nl
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
#ifndef __MWENGINE__WAVETABLES_H_INCLUDED__
#define __MWENGINE__WAVETABLES_H_INCLUDED__

#include <global.h>

namespace MWEngine {
class WaveTables
{
    public:

        // a pre cached sine waveform (can be used directly
        // by LFO). Further tables can be generated at runtime
        // using WaveGenerator. Tables can be pooled in TablePool
        // to avoid repeated CPU strain by generation.

        static constexpr SAMPLE_TYPE SINE_TABLE[ WAVE_TABLE_PRECISION ] =
        {
            0, 0.0490677, 0.0980171, 0.14673, 0.19509, 0.24298, 0.290285, 0.33689, 0.382683,
            0.427555, 0.471397, 0.514103, 0.55557, 0.595699, 0.634393, 0.671559, 0.707107,
            0.740951, 0.77301, 0.803208, 0.83147, 0.857729, 0.881921, 0.903989, 0.92388,
            0.941544, 0.95694, 0.970031, 0.980785, 0.989177, 0.995185, 0.998795, 1, 0.998795,
            0.995185, 0.989177, 0.980785, 0.970031, 0.95694, 0.941544, 0.92388, 0.903989,
            0.881921, 0.857729, 0.83147, 0.803208, 0.77301, 0.740951, 0.707107, 0.671559,
            0.634393, 0.595699, 0.55557, 0.514103, 0.471397, 0.427555, 0.382683, 0.33689,
            0.290285, 0.24298, 0.19509, 0.14673, 0.0980171, 0.0490677, 1.22465e-16, -0.0490677,
            -0.0980171, -0.14673, -0.19509, -0.24298, -0.290285, -0.33689, -0.382683, -0.427555,
            -0.471397, -0.514103, -0.55557, -0.595699, -0.634393, -0.671559, -0.707107,
            -0.740951, -0.77301, -0.803208, -0.83147, -0.857729, -0.881921, -0.903989, -0.92388,
            -0.941544, -0.95694, -0.970031, -0.980785, -0.989177, -0.995185, -0.998795, -1,
            -0.998795, -0.995185, -0.989177, -0.980785, -0.970031, -0.95694, -0.941544, -0.92388,
            -0.903989, -0.881921, -0.857729, -0.83147, -0.803208, -0.77301, -0.740951, -0.707107,
            -0.671559, -0.634393, -0.595699, -0.55557, -0.514103, -0.471397, -0.427555, -0.382683,
            -0.33689, -0.290285, -0.24298, -0.19509, -0.14673, -0.0980171, -0.0490677
        };
};

// declare the table so it the above definition can be linked by the compiler

constexpr SAMPLE_TYPE WaveTables::SINE_TABLE[ WAVE_TABLE_PRECISION ];

} // E.O namespace MWEngine

#endif
