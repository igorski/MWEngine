#ifndef __ROUTEABLEOSCILLATOR_H_INCLUDED__
#define __ROUTEABLEOSCILLATOR_H_INCLUDED__

#include "lfo.h"
#include "baseprocessor.h"

class RouteableOscillator
{
    public:
        RouteableOscillator();
        ~RouteableOscillator();
        int destination;
        float speed;
        int wave;

        void linkOscillator();
        void unlinkOscillator();
        bool isLinked();
        LFO *getLinkedOscillator();

    private:
        bool _hasOscillator;
        LFO* _oscillator;
};

#endif
