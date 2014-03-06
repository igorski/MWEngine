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
#include "processingchain.h"
#include "global.h"
#include <math.h>
#include "utils.h"

// constructor

/**
 * TODO: remove all references to specific processors
 * and allow check-in of BaseProcessor Objects (allowing
 * for multiple instances of the same effect)
 *
 * TODO: add a convenient method for altering the order
 * of these effects
 */
ProcessingChain::ProcessingChain()
{
    filterCutoff    = audio_engine::SAMPLE_RATE / 4;
    filterResonance = sqrt( 1 ) * .5;
    filterFormant   = 0;

    phaserDepth     = 0.5;
    phaserFeedback  = 0.7;
    phaserRate      = 0.5;

    delayTime       = 250;
    delayMix        = .25;
    delayFeedback   = .5;

    distortion               = .5;
    distortionLevel          = .5;
    decimatorDistortion      = 16;
    decimatorDistortionLevel = .25;

    cAttack         = 20;
    cRelease        = 500;
    cThreshold      = 8;
    cGain           = 15.0;
    cRatio          = 1.2;

    lpfCutoff = audio_engine::SAMPLE_RATE;
    hpfCutoff = 5;

    // start off with all effects deactivated
    reset();

    fm           = 0;
    filter       = 0;
    compressor   = 0;
    delay        = 0;
    lpfhpf       = 0;
    formant      = 0;
    phaser       = 0;
    pitchShifter = 0;
    bitCrusher   = 0;
    decimator    = 0;
    waveShaper   = 0;
}

ProcessingChain::~ProcessingChain()
{
    DebugTool::log( "ProcessingChain::DESTRUCT" );

    _activeProcessors.clear();
    _activeBusProcessors.clear();

    delete fm;
    delete filter;
    delete compressor;
    delete delay;
    delete lpfhpf;
    delete formant;
    delete phaser;
    delete pitchShifter;
    delete bitCrusher;
    delete decimator;
    delete waveShaper;
}

/* public methods */

void ProcessingChain::reset()
{
    fmActive         = false;
    filterActive     = false;
    formantActive    = false;
    phaserActive     = false;
    waveshaperActive = false;
    bitCrusherActive = false;
    decimatorActive  = false;
    delayActive      = false;
    compressorActive = false;
    lpfHpfActive     = false;

    cacheActiveProcessors();
}

void ProcessingChain::cacheActiveProcessors()
{
    // note no delete / destructors should be invoked when clearing the previously active processors!
    _activeProcessors.clear();
    _activeBusProcessors.clear();

    /* processors */

    if ( fmActive )
        _activeProcessors.push_back( fm );

    if ( waveshaperActive )
        _activeProcessors.push_back( waveShaper );

    if ( bitCrusherActive )
        _activeProcessors.push_back( bitCrusher );

    // always active (but mostly idle sparing CPU sources)
    if ( pitchShifter != 0 )
        _activeProcessors.push_back( pitchShifter );

    if ( decimatorActive )
        _activeProcessors.push_back( decimator );

    if ( phaserActive )
        _activeProcessors.push_back( phaser );

    if ( filterActive )
        _activeProcessors.push_back( filter );

    if ( formantActive )
        _activeProcessors.push_back( formant );

    if ( compressorActive )
        _activeProcessors.push_back( compressor );

    if ( lpfHpfActive )
        _activeProcessors.push_back( lpfhpf );

    /* bus processors */

    if ( delayActive )
        _activeBusProcessors.push_back( delay );
}

std::vector<BaseProcessor*> ProcessingChain::getActiveProcessors()
{
    return _activeProcessors;
}

std::vector<BaseBusProcessor*> ProcessingChain::getActiveBusProcessors()
{
    return _activeBusProcessors;
}
