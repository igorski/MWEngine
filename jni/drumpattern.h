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
#ifndef __DRUMPATTERN_H_INCLUDED__
#define __DRUMPATTERN_H_INCLUDED__

#include <events/baseaudioevent.h>
#include <events/drumevent.h>
#include <instruments/baseinstrument.h>
#include <vector>

class DrumPattern
{
    public:
        DrumPattern( int aNum, BaseInstrument* aInstrument );
        ~DrumPattern();

        static const int AMOUNT_OF_STEPS = 16; // work as sixteen step sequencer
        static const int EVENT_OFF       = 0;
        static const int EVENT_ON        = 1;

        int num;
        int eventAmount;
        std::vector<BaseAudioEvent*>* audioEvents;

        void addToInstrument();
        void removeFromInstrument();

        void updateTimbre( int newTimbre );
        void cacheEvents( int aDrumTimbre );
        void addDrumEvent( int aPosition, int aDrumType, int aDrumTimbre );
        void removeDrumEvent( int aPosition, int aType );
        bool hasContent();
        void clear();

        int* getKickPattern();
        int* getSnarePattern();
        int* getStickPattern();
        int* getHatPattern();
        void setKickPattern ( int* aPattern, int arrayLength );
        void setSnarePattern( int* aPattern, int arrayLength );
        void setStickPattern( int* aPattern, int arrayLength );
        void setHatPattern  ( int* aPattern, int arrayLength );

    protected:
        int* kickPattern;
        int* snarePattern;
        int* stickPattern;
        int* hatPattern;
        BaseInstrument* _instrument;
        void destroyAudioEvents();
};

#endif
