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
#ifndef __NATIVEAUDIOENGINE_H_INCLUDED__
#define __NATIVEAUDIOENGINE_H_INCLUDED__

/**
 * this is the root of AudioEngine which will
 * handle all output and rendering duties
 */
#include "audiobuffer.h"
#include "global.h"
#include "finalizer.h"
#include "lpfhpfilter.h"
#include "sequencer.h"
#include "utils.h"

// internal handlers triggering JNI callbacks
void handleTempoUpdate            ( float aQueuedTempo, bool broadcastUpdate );
void handleSequencerPositionUpdate( float streamTimeStamp );
void handleBounceComplete         ( int aIdentifier );
bool writeChannelCache( AudioChannel* channel, AudioBuffer* channelBuffer, int cacheReadPos );
void broadcastStepPosition();
void handleHardwareUnavailable();

#endif
