#ifndef __BASEAUDIOEVENT_H_INCLUDED__
#define __BASEAUDIOEVENT_H_INCLUDED__

class BaseAudioEvent
{
    public:
        BaseAudioEvent();
        ~BaseAudioEvent();

        /**
         * a BaseAudioEvent contains a buffer holding the samples, this
         * is used by the AudioRenderer to write the samples into the output buffer
         * @return {float*} the buffer containing this events samples
         */
        virtual float* getBuffer();

        /**
         * an AudioEvent can also synthesize audio live, this
         * method should be called during the write cycle on
         * each buffer update of the AudioRenderer
         * @param aBufferLength {int} desired buffer length ( usually buffer size )
         * @return {float*} the buffer containing the live generated samples
         */
        virtual float* synthesize( int aBufferLength );

        virtual int getSampleLength();
        virtual int getSampleStart();
        virtual int getSampleEnd();

        virtual bool deletable();   // query whether this event is queued for deletion
        virtual void setDeletable( bool value );

        virtual void lock();        // "lock" mutations within this events buffer during reading
        virtual void unlock();      // unlock
        virtual bool isLocked();

        virtual void destroy();

    protected:

        // buffer regions
        int _sampleStart;
        int _sampleEnd;
        int _sampleLength;

        // cached buffer
        float* _buffer;
        void destroyBuffer();

        bool _deleteMe;
        bool _locked;
        bool _updateAfterUnlock;    // use in update-methods when checking for lock
};

#endif
