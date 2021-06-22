/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Igor Zinken - https://www.igorski.nl
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
#ifndef __MWENGINE__PERF_UTILITY_H_INCLUDED__
#define __MWENGINE__PERF_UTILITY_H_INCLUDED__

#include <ctime>
#include <utilities/debug.h>
#ifdef MOCK_ENGINE
#include <drivers/adapter.h>
#endif

const int64_t NANOS_PER_SECOND      = 1000000000;
const int64_t NANOS_PER_MILLISECOND = 1000000;

namespace MWEngine {
namespace PerfUtility
{
    /**
     * Retrieves the current timestamp with nanosecond precision
     */
    inline int64_t now( clockid_t clockId = CLOCK_MONOTONIC )
    {
        struct timespec time;
        int result = clock_gettime( clockId, &time );
        return ( result < 0 ) ? result : ( time.tv_sec * NANOS_PER_SECOND ) + time.tv_nsec;
    }

    /**
     * Optimizes the performance of the calling thread by setting the thread affinity.
     * On Android N the exclusive CPU cores will be used.
     */
    inline void optimizeThreadPerformance( const std::vector<int>& cpuIds )
    {
        cpu_set_t mask;
        pid_t current_thread_id = gettid();
        cpu_set_t cpu_set;
        CPU_ZERO( &cpu_set );

        // If the callback cpu ids aren't specified then bind to the current cpu
        if ( cpuIds.empty() ) {
            int current_cpu_id = sched_getcpu();
            Debug::log( "Binding to current CPU ID %d for thread %d", current_cpu_id, current_thread_id );
            CPU_SET( current_cpu_id, &cpu_set );
        } else {
            Debug::log( "Binding to %d CPU IDs", static_cast<int>( cpuIds.size()) );
            for ( size_t i = 0; i < cpuIds.size(); i++ ) {
                int cpu_id = cpuIds.at( i );
                Debug::log( "CPU ID %d added to cores set for thread %d", cpu_id, current_thread_id );
                CPU_SET( cpu_id, &cpu_set );
            }
        }

        if ( sched_setaffinity( current_thread_id, sizeof( cpu_set_t ), &cpu_set ) != 0 ) {
            int err = errno;
            Debug::log( "Error in the syscall setaffinity: mask=%d=0x%x err=%d=0x%x", mask, mask, err, err );
        }
    }

#ifdef PREVENT_CPU_FREQUENCY_SCALING

    #define OPERATIONS_PER_STEP 20000
    
    /**
     * Apply a load (via repeated assembly noop calls) until given deadline
     * to prevent the CPU from scaling its frequency, which can impact performance
     *
     * Adapted from The Android Open Source Project
     */
    inline double applyCPUStabilizingLoad( int64_t stabilizationEndTime, double noopsPerTick )
    {
#ifdef MOCK_ENGINE
        if ( DriverAdapter::isMocked() ) {
            return noopsPerTick;
        }
#endif
        int64_t now = PerfUtility::now();

        int totalNoopsToExecute   = ( int ) noopsPerTick * OPERATIONS_PER_STEP;
        int64_t iterationDuration = 0;
        int64_t lastIterationTime = 0;

        while ( now <= stabilizationEndTime )
        {
            for ( size_t i = 0; i < totalNoopsToExecute; i++ ) {
                noop();
            }
            lastIterationTime = now;
            now               = PerfUtility::now();
            iterationDuration = now - lastIterationTime;

            // smoothing values by calculating exponential moving average and applying as a low pass filter
            double totalOperationsTick = ( double ) totalNoopsToExecute / iterationDuration;
            noopsPerTick               = 0.1F * totalOperationsTick + ( 1.0 - 0.1F ) * noopsPerTick;
            totalNoopsToExecute        = ( int ) ( noopsPerTick * OPERATIONS_PER_STEP );
        }
        return noopsPerTick;
    }

#endif
}
};
#endif
