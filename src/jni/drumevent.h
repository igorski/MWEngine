#ifndef __DRUMEVENT_H_INCLUDED__
#define __DRUMEVENT_H_INCLUDED__

#include "sampleevent.h"
#include "druminstrument.h"
#include <string>

class DrumEvent : public SampleEvent
{
    public:
        DrumEvent( int aPosition, int aDrumType, DrumInstrument* aInstrument );
        ~DrumEvent();

        int getTimbre();
        void setTimbre( int aTimbre );
        int getType();
        void setType( int aType );
        void unlock();

    private:
        int _timbre;
        int _type;
        bool _inited;
        void updateSample();
};

#endif
