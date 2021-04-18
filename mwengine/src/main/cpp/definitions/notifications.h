/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2018 Igor Zinken - http://www.igorski.nl
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
#ifndef __MWENGINE__NOTIFICATIONS_H_INCLUDED__
#define __MWENGINE__NOTIFICATIONS_H_INCLUDED__

namespace MWEngine {
class Notifications
{
    public:

        /**
         * all the messages the audio engine broadcasts
         * to indicate state changes
         *
         * see observer.h and notifier.h for integration
         */
        enum ids
        {
            /* sequencer actions */

            SEQUENCER_POSITION_UPDATED, // sequencer has progressed to the next step within the measure
            MARKER_POSITION_REACHED,    // sequencer has reached the requested marker position
            SEQUENCER_TEMPO_UPDATED,    // sequencer has updated its tempo to the requested tempo

            /* recording actions */

            RECORDED_SNIPPET_READY,     // single snippet is ready for writing to storage
            RECORDED_SNIPPET_SAVED,     // single snippet has been written to storage and flushed from memory
            RECORDING_COMPLETED,        // recording has completed in full all snippets have been saved into requested output file, memory and temp files flushed
            BOUNCE_COMPLETE,            // bouncing has completed, see RECORDING_COMPLETED

            /* system messages */

            STATUS_BRIDGE_CONNECTED,    // JNI bridge connected

            /* fatal errors */

            ERROR_HARDWARE_UNAVAILABLE, // audio hardware unavailable
            ERROR_THREAD_START          // error occurred during starting of render thread
        };
};
} // E.O namespace MWEngine

#endif
