#ifndef __DRUMPATTERN_H_INCLUDED__
#define __DRUMPATTERN_H_INCLUDED__

#include "drumevent.h"
#include "druminstrument.h"
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
