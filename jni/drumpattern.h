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

#include "druminstrument.h"
#include <events/drumevent.h>
#include <vector>

class DrumPattern
{
    public:
        DrumPattern( int aNum );
        ~DrumPattern();

        int num;
        int eventAmount;
        std::vector<DrumEvent*> *audioEvents;

        void updateTimbre( int newTimbre );
        void cacheEvents( DrumInstrument* aInstrument );
        void addDrumEvent( int aPosition, int aDrumType, DrumInstrument* aInstrument );
        void removeDrumEvent( int aPosition, int aType );
        void destroy();

        int kickPatternLength;
        int snarePatternLength;
        int stickPatternLength;
        int hatPatternLength;

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
        void destroyAudioEvents();
};

#endif
