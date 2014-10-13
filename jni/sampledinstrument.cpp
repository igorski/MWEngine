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
#include "sampledinstrument.h"
#include "global.h"
#include "sequencer.h"
#include "utils.h"
#include <cstddef>

/* constructor / destructor */

SampledInstrument::SampledInstrument()
{
    audioChannel    = new AudioChannel( .75, bytes_per_bar );
    audioEvents     = new std::vector<SampleEvent*>();

    sequencer::samplers.push_back( this );  // register instrument inside the sequencer
    index = sequencer::samplers.size() - 1; // the index this instrument is registered at in the sequencer
}

SampledInstrument::~SampledInstrument()
{
    DebugTool::log( "SampledInstrument::DESTRUCT" );

    for ( int i; i < sequencer::samplers.size(); i++ )
    {
        if ( sequencer::samplers[ i ] == this )
        {
            sequencer::samplers.erase( sequencer::samplers.begin() + i );
            break;
        }
    }
    audioEvents->clear();
    delete audioEvents;

    delete processingChain;
}

/* public methods */

bool SampledInstrument::hasEvents()
{
    return audioEvents->size() > 0;
}
