/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Igor Zinken - http://www.igorski.nl
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
#ifndef __NOTIFICATIONS_H_INCLUDED__
#define __NOTIFICATIONS_H_INCLUDED__

class Notifications
{
    public:

        /**
         * all the messages the audio engine broadcasts
         * to indicate state changes
         *
         * see observer.h and notifier.h for integration
         */
        enum ids {
            SEQUENCER_POSITION_UPDATED,
            MARKER_POSITION_REACHED,
            SEQUENCER_TEMPO_UPDATED,
            RECORDING_STATE_UPDATED,
            BOUNCE_COMPLETE,

            STATUS_BRIDGE_CONNECTED,

            ERROR_HARDWARE_UNAVAILABLE,
            ERROR_THREAD_START
        };
};

#endif