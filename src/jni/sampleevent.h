#ifndef __SAMPLEEVENT_H_INCLUDED__
#define __SAMPLEEVENT_H_INCLUDED__

#include "basecacheableaudioevent.h"

class SampleEvent : public BaseCacheableAudioEvent
{
    public:
        SampleEvent();
        SampleEvent( int aPosition );
        virtual ~SampleEvent();

        // used by view representation
        int position;

        virtual int getSampleLength();
        virtual int getSampleStart();
        virtual int getSampleEnd();
        virtual bool deletable();
        virtual void setDeletable( bool value );

        virtual float* getBuffer();
        virtual float* synthesize( int aBufferLength );
        virtual bool isCached();
        virtual void setAutoCache( bool aValue );
        virtual void cache( bool doCallback );
        virtual void setSample( float* sampleBuffer, int sampleLength );

    protected:
        // buffer regions
        int _sampleStart;
        int _sampleEnd;
        int _sampleLength;

        // removal of AudioEvents must occur outside of the
        // cache loop, by activating this boolean we're queuing
        // the AudioEvent for removal

        bool _deleteMe;
        bool _cancel;            // whether we should cancel caching
        bool _cachingCompleted;  // whether we're done caching

        void init();
};

#endif
