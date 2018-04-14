#include "../../events/sampleevent.h"
#include "../../instruments/sampledinstrument.h"

TEST( SampleEvent, Constructor )
{
    SampledInstrument* instrument = new SampledInstrument();
    SampleEvent* audioEvent = new SampleEvent( instrument );

    ASSERT_TRUE( instrument == audioEvent->getInstrument() )
        << "expected instrument to equal the value given in constructor";

    delete audioEvent;
    delete instrument;
}

TEST( SampleEvent, GettersSetters )
{
    SampledInstrument* instrument = new SampledInstrument();
    SampleEvent* audioEvent = new SampleEvent( instrument );

    int rangeStart = randomInt( 0, 512 );
    int rangeEnd   = randomInt( 512, 2014 );

    audioEvent->setBufferRangeStart( rangeStart );
    audioEvent->setBufferRangeEnd  ( rangeEnd );

    EXPECT_EQ( rangeStart, audioEvent->getBufferRangeStart() )
        << "expected range start to equal the value given in the setter method";

    EXPECT_EQ( rangeEnd, audioEvent->getBufferRangeEnd() )
        << "expected range end to equal the value given in the setter method";

    EXPECT_EQ(( rangeEnd - rangeStart ) + 1, audioEvent->getBufferRangeLength() )
        << "expected buffer range length to be " << (( rangeEnd - rangeStart ) + 1 );

    delete audioEvent;
    delete instrument;
}

TEST( SampleEvent, BufferRangeStartSanitizing )
{
    SampledInstrument* instrument = new SampledInstrument();
    SampleEvent* audioEvent = new SampleEvent( instrument );

    // set initial sample range/length and buffer range values

    int sampleLength = randomInt( 1024, 2048 );

    audioEvent->setEventLength( sampleLength );

    int rangeStart = randomInt( 0, 256 );
    int rangeEnd   = randomInt( 256, sampleLength / 2 );

    audioEvent->setBufferRangeStart( rangeStart );
    audioEvent->setBufferRangeEnd  ( rangeEnd );

    int rangeLength = audioEvent->getBufferRangeLength();

    // 1. test updating range start value beyond the full range length

    audioEvent->setBufferRangeStart( rangeEnd + 1 );

    EXPECT_EQ( rangeEnd - 1, audioEvent->getBufferRangeStart() )
        << "expected range start to have been sanitized to maximum range length index minus 1";

    EXPECT_EQ( rangeStart + ( rangeLength - 1 ), audioEvent->getBufferRangeEnd() )
        << "expected range end to have been sanitized to range start plus maximum range length index";

    EXPECT_EQ( 2, audioEvent->getBufferRangeLength() )
        << "expected range length to be 2";

    delete audioEvent;
    delete instrument;
}

TEST( SampleEvent, BufferRangeEndSanitizing )
{
    SampledInstrument* instrument = new SampledInstrument();
    SampleEvent* audioEvent = new SampleEvent( instrument );

    int sampleLength = randomInt( 24, 512 );
    AudioBuffer* buffer = new AudioBuffer( 1, sampleLength );
    audioEvent->setSample( buffer );

    int rangeEnd = sampleLength / 2;
    audioEvent->setBufferRangeEnd( rangeEnd );

    EXPECT_EQ( rangeEnd, audioEvent->getBufferRangeEnd() )
        << "expected a range end with a value smaller than the allowed max source sample buffer length";

    audioEvent->setBufferRangeEnd( sampleLength * 2 );

    EXPECT_EQ( sampleLength - 1, audioEvent->getBufferRangeEnd() )
        << "expected buffer range to not exceed the source sample buffer length";

    delete audioEvent;
    delete instrument;
    delete buffer;
}

TEST( SampleEvent, RangeBasedPlayback )
{
    SampleEvent* event = new SampleEvent();
    ASSERT_FALSE( event->getRangeBasedPlayback())
        << "expected SampleEvent to not use range based playback by default";

    int sampleStart  = 500;
    int sampleLength = 1000;
    int sampleEnd    = sampleStart + sampleLength;

    event->setEventStart ( sampleStart );
    event->setEventEnd   ( sampleEnd );
    event->setEventLength( sampleLength );

    int rangeStart   = 200;
    int rangeLength  = sampleLength - rangeStart;
    int rangeEnd     = rangeStart + rangeLength;

    event->setBufferRangeStart( rangeStart );

    ASSERT_TRUE( event->getRangeBasedPlayback())
        << "expected range based playback to be active now SampleEvent has a buffer range different to the sample length";

/*
    event->setBufferRangeStart( 0 );

    ASSERT_FALSE( event->getRangeBasedPlayback())
        << "expected range based playback to be inactive now SampleEvent has no longer got a buffer range different to the sample length";
*/

    delete event;
    event = new SampleEvent();

    event->setEventStart ( sampleStart );
    event->setEventEnd   ( sampleEnd );
    event->setEventLength( sampleLength );

    event->setBufferRangeEnd( rangeEnd - 10 );

    ASSERT_TRUE( event->getRangeBasedPlayback())
        << "expected range based playback to be active now SampleEvent has a buffer range different to the sample length";

    delete event;
}

TEST( SampleEvent, SetSample )
{
    SampledInstrument* instrument = new SampledInstrument();
    SampleEvent* audioEvent = new SampleEvent( instrument );
    int bufferLength = randomInt( 24, 512 );

    EXPECT_EQ( 0, audioEvent->getEventStart() )
        << "expected 0 sample start index after construction";

    EXPECT_EQ( 0, audioEvent->getEventEnd() )
        << "expected 0 sample end index after construction";

    EXPECT_EQ( 0, audioEvent->getEventLength() )
        << "expected 0 sample length index after construction";

    int sampleStart  = bufferLength / 4;
    int sampleLength = bufferLength / 2;

    audioEvent->setEventStart ( sampleStart );
    audioEvent->setEventLength( sampleLength );

    EXPECT_EQ( sampleStart, audioEvent->getEventStart() )
        << "expected sample start to have been updated after setter method";

    EXPECT_EQ( sampleStart + ( sampleLength - 1 ), audioEvent->getEventEnd() )
        << "expected sample length to have been updated after setter method";

    EXPECT_EQ( sampleLength, audioEvent->getEventLength() )
        << "expected sample length to have been updated after setter method";

    int bufferRangeStart = randomInt( 0, sampleStart );
    int bufferRangeEnd   = randomInt( bufferRangeStart + 1, bufferRangeStart + bufferLength / 2 );

    audioEvent->setBufferRangeStart( bufferRangeStart );
    audioEvent->setBufferRangeEnd  ( bufferRangeEnd );

    EXPECT_EQ(( bufferRangeEnd - bufferRangeStart ) + 1, audioEvent->getBufferRangeLength() )
        << "expected buffer range prior to setSample reset to meet the expectation";

    AudioBuffer* buffer = new AudioBuffer( 1, bufferLength );
    audioEvent->setSample( buffer );

    EXPECT_EQ( 0, audioEvent->getBufferRangeStart() )
        << "expected 0 buffer range start index after updating of sample";

    EXPECT_EQ( bufferLength - 1, audioEvent->getBufferRangeEnd() )
        << "expected " << ( bufferLength - 1 ) << " range end index after updating of sample";

    EXPECT_EQ( bufferLength, audioEvent->getBufferRangeLength() )
        << "expected " << bufferLength << " buffer range length after updating of sample";

    delete audioEvent;
    delete instrument;
    delete buffer;
}

TEST( SampleEvent, GetBufferForRange )
{
    SampledInstrument* instrument = new SampledInstrument();
    SampleEvent* audioEvent = new SampleEvent( instrument );

    int sampleLength = randomInt( 8, 24 );
    int sampleStart  = randomInt( 0, ( int )( sampleLength / 2 ));

    audioEvent->setEventStart ( sampleStart );
    audioEvent->setEventLength( sampleLength );

    int sampleEnd = audioEvent->getEventEnd();

    int bufferRangeStart = randomInt( 0, ( int )( sampleLength / 2 ));
    int bufferRangeEnd   = randomInt( bufferRangeStart + 1, bufferRangeStart + sampleLength );
    bool loopeable       = randomBool();

    audioEvent->setBufferRangeStart( bufferRangeStart );
    audioEvent->setBufferRangeEnd  ( bufferRangeEnd );
    audioEvent->setLoopeable( loopeable );

    // generate random audio content

    AudioBuffer* buffer = fillAudioBuffer( new AudioBuffer( randomInt( 1, 1 ), sampleLength ));
    audioEvent->setSample( buffer );

    float volume = randomFloat();
    audioEvent->setVolume( volume );

    // create a temporary buffer to write output in, ensure it is smaller than the event buffer
    AudioBuffer* targetBuffer = new AudioBuffer( buffer->amountOfChannels, randomInt( 2, 4 ));
    int buffersToWrite        = targetBuffer->bufferSize;

    ASSERT_FALSE( bufferHasContent( targetBuffer ))
        << "expected target buffer to be silent after creation, but it has content";

    int readPos        = loopeable ? audioEvent->getReadPointer() : randomInt( 0, sampleEnd );
    int readEnd        = readPos + buffersToWrite - 1;
    bool expectContent = ( readPos >= sampleStart && readPos <= sampleEnd ) ||
                         (( readEnd ) >= sampleStart && ( readEnd ) <= sampleEnd );

    int rangePointer   = bufferRangeStart; // internal range pointer used by SampleEvent

    //std::cout << "ss:" << sampleStart << " sl:" << sampleLength << " se:" << sampleEnd << " range s:" << bufferRangeStart << " range e:" << bufferRangeEnd << " loop:" << loopeable;
    //std::cout << " read: " << readPos << " end: " << readEnd << " expect: " << expectContent << " buffer size: " << targetBuffer->bufferSize << "\n";

    // run the method

    bool gotBuffer = audioEvent->getBufferForRange( targetBuffer, readPos );

    EXPECT_EQ( expectContent, gotBuffer )
        << "result should match the expectation";

    for ( int i = 0, r = readPos; i < targetBuffer->bufferSize; ++i )
    {
        if ( r >= sampleStart && r <= sampleEnd )
        {
            for ( int c = 0; c < targetBuffer->amountOfChannels; ++c )
            {
                SAMPLE_TYPE* buffer       = targetBuffer->getBufferForChannel( c );
                SAMPLE_TYPE* sourceBuffer = audioEvent->getBuffer()->getBufferForChannel( c );

                SAMPLE_TYPE value          = buffer[ i ];
                SAMPLE_TYPE expectedSample = sourceBuffer[ rangePointer ] * volume;

                EXPECT_EQ( expectedSample, value )
                    << "value at " << i << " doesn't match the expected value at read pointer " << rangePointer;
            }
            if ( ++rangePointer > bufferRangeEnd )
                rangePointer = bufferRangeStart;
        }
        if ( ++r > sampleEnd && loopeable )
            r = sampleStart;
    }

    delete audioEvent;
    delete instrument;
    delete targetBuffer;
    delete buffer;
}

// test overridden mix buffer method
TEST( SampleEvent, MixBuffer )
{
    SampledInstrument* instrument = new SampledInstrument();
    SampleEvent* audioEvent = new SampleEvent( instrument );

    int sampleLength = randomInt( 8, 24 );
    int sampleStart  = randomInt( 0, ( int )( sampleLength / 2 ));

    audioEvent->setEventStart ( sampleStart );
    audioEvent->setEventLength( sampleLength );

    int sampleEnd = audioEvent->getEventEnd();

    AudioBuffer* buffer = fillAudioBuffer( new AudioBuffer( randomInt( 1, 4 ), sampleLength ));
    audioEvent->setSample( buffer );

    float volume = randomFloat();
    audioEvent->setVolume( volume );

    //std::cout << " ss: " << sampleStart << " se: " << sampleEnd << " sl: " << sampleLength << " ch: " << buffer->amountOfChannels;

    // create a temporary buffer to write output in, ensure it is smaller than the event buffer
    AudioBuffer* targetBuffer = new AudioBuffer( buffer->amountOfChannels, randomInt( 2, 4 ));
    int buffersToWrite        = targetBuffer->bufferSize;

    ASSERT_FALSE( bufferHasContent( targetBuffer ))
        << "expected target buffer to be silent after creation, but it has content";

    // test 1. mix without loopable range

    int maxBufferPos = sampleLength * 2; // use a "loop range" larger than the size of the events length
    int minBufferPos = randomInt( 0, maxBufferPos / 2 );
    int bufferPos    = randomInt( minBufferPos, maxBufferPos - 1 );
    bool loopStarted = false;
    int loopOffset   = 0;

    // if the random bufferPosition wasn't within the events sampleStart and sampleEnd range, we expect no content

    bool expectContent = ( bufferPos >= sampleStart && bufferPos <= sampleEnd ) ||
                         (( bufferPos + buffersToWrite ) >= sampleStart && ( bufferPos + buffersToWrite ) <= sampleEnd );

    //std::cout << " expected content: " << expectContent << " for buffer size: " << buffersToWrite;
    //std::cout << " min: " << minBufferPos << " max: " << maxBufferPos << " cur: " << bufferPos;

    audioEvent->mixBuffer( targetBuffer, bufferPos, minBufferPos, maxBufferPos, loopStarted, loopOffset, false );

    // validate buffer contents after mixing

    if ( expectContent )
    {
        for ( int c = 0, ca = targetBuffer->amountOfChannels; c < ca; ++c )
        {
            SAMPLE_TYPE* buffer       = targetBuffer->getBufferForChannel( c );
            SAMPLE_TYPE* sourceBuffer = audioEvent->getBuffer()->getBufferForChannel( c );
            SAMPLE_TYPE expectedSample;

            for ( int i = 0; i < buffersToWrite; ++i )
            {
                int r = i + bufferPos; // read pointer for the source buffer

                if ( r >= maxBufferPos && !loopStarted )
                    r -= ( maxBufferPos - minBufferPos );

                if ( r >= sampleStart && r <= sampleEnd )
                {
                    r -= sampleStart; // substract audioEvent start position
                    expectedSample = sourceBuffer[ r ] * volume;
                }
                else {
                    expectedSample = 0.0;
                }
                SAMPLE_TYPE sample = buffer[ i ];

                EXPECT_EQ( expectedSample, sample )
                    << "expected mixed sample at " << i << " to be equal to the calculated expected sample at read offset " << r;
            }
        }

    }
    else {
        ASSERT_FALSE( bufferHasContent( targetBuffer ))
            << "expected target buffer to contain no content after mixing for an out-of-range buffer position";
    }

    // test 2. mixing within a loopable range (implying sequencer is starting a loop)

    targetBuffer->silenceBuffers();

    ASSERT_FALSE( bufferHasContent( targetBuffer ))
        << "expected target buffer to be silent after silencing, but it still has content";

    bufferPos     = randomInt( minBufferPos, maxBufferPos - 1 );
    loopStarted   = true;
    loopOffset    = ( maxBufferPos - bufferPos ) + 1;

    // pre calculate at which buffer iterator the looping will commence
    // loopStartIteratorPosition describes at which sequencer position the loop starts
    // loopStartWritePointer describes at which position in the targetBuffer the loop is written to
    // amountOfLoopedWrites is the amount of samples written in the loop
    // loopStartReadPointer describes at which position the samples from the source audioEvent will be read when loop starts
    // loopStartReadPointerEnd describes the last position the samples from the source audioEvent will be read for the amount of loop writes

    int loopStartIteratorPosition = maxBufferPos + 1;
    int loopStartWritePointer     = loopOffset;
    int loopStartReadPointer      = minBufferPos;
    int amountOfLoopedWrites      = ( bufferPos + buffersToWrite ) - loopStartIteratorPosition;
    int loopStartReadPointerEnd   = ( loopStartReadPointer + amountOfLoopedWrites ) - 1;

    expectContent = ( bufferPos >= sampleStart && bufferPos <= sampleEnd ) ||
                    (( bufferPos + buffersToWrite ) >= sampleStart && ( bufferPos + buffersToWrite ) <= sampleEnd ) ||
                    ( loopStartIteratorPosition > maxBufferPos && (
                        ( loopStartReadPointer >= sampleStart && loopStartReadPointer <= sampleEnd ) ||
                        ( loopStartReadPointerEnd >= sampleStart && loopStartReadPointerEnd <= sampleEnd )));

    audioEvent->mixBuffer( targetBuffer, bufferPos, minBufferPos, maxBufferPos, loopStarted, loopOffset, false );

    //std::cout << " expected content: " << expectContent << " for buffer size: " << buffersToWrite;
    //std::cout << " min: " << minBufferPos << " max: " << maxBufferPos << " cur: " << bufferPos << " loop offset: " << loopOffset;

    if ( expectContent )
    {
        for ( int c = 0, ca = targetBuffer->amountOfChannels; c < ca; ++c )
        {
            SAMPLE_TYPE* buffer       = targetBuffer->getBufferForChannel( c );
            SAMPLE_TYPE* sourceBuffer = audioEvent->getBuffer()->getBufferForChannel( c );

            for ( int i = 0; i < buffersToWrite; ++i )
            {
                SAMPLE_TYPE expectedSample = 0.0;

                int r = i + bufferPos; // read pointer for the source buffer

                if ( i >= loopOffset )
                    r = minBufferPos + ( i - loopOffset );

                if ( r >= sampleStart && r <= sampleEnd )
                {
                    r -= sampleStart; // substract audioEvent start position
                    expectedSample = sourceBuffer[ r ] * volume;
                }
                SAMPLE_TYPE sample = buffer[ i ];

                EXPECT_EQ( expectedSample, sample )
                    << "expected mixed sample at " << i << " to be equal to the calculated expected sample at read "
                    << "offset " << r << " ( sanitized from " << ( i + bufferPos ) << " )";
            }
        }
    }
    else {
        ASSERT_FALSE( bufferHasContent( targetBuffer ))
            << "expected output buffer to contain no content after mixing for an out-of-range buffer position";
    }

    delete audioEvent;
    delete instrument;
    delete targetBuffer;
    delete buffer;
}

TEST( SampleEvent, PlaybackRateDefault )
{
    SampleEvent* audioEvent = new SampleEvent();

    EXPECT_EQ( 1.f, audioEvent->getPlaybackRate() )
        << "expected default playback rate to be 1";

    delete audioEvent;
}

TEST( SampleEvent, PlaybackRateSetter )
{
    SampleEvent* audioEvent = new SampleEvent();

    float expectedRate = randomFloat();
    audioEvent->setPlaybackRate( expectedRate );

    EXPECT_EQ( expectedRate, audioEvent->getPlaybackRate() )
        << "expected default playback rate to equal the set value";

    delete audioEvent;
}

TEST( SampleEvent, PlaybackRateLimit )
{
    SampleEvent* audioEvent = new SampleEvent();

    audioEvent->setPlaybackRate( 0.f );

    EXPECT_EQ( 0.01f, audioEvent->getPlaybackRate()) << "expected minimum rate of 1% of original speed";

    audioEvent->setPlaybackRate( 1000.f );

    EXPECT_EQ( 100.f, audioEvent->getPlaybackRate()) << "expected maximum rate of 100 x original speed";

    delete audioEvent;
}

TEST( SampleEvent, PlaybackRate )
{
    SampleEvent* audioEvent = new SampleEvent();

    // properties for 1.f playback rate
    int eventStart  = 1000;
    int eventLength = 1000;
    int eventEnd    = eventStart + eventLength - 1;

    audioEvent->setEventStart( eventStart );
    audioEvent->setEventLength( eventLength );

    EXPECT_EQ( eventStart, audioEvent->getEventStart() )
        << "expected event start to be unchanged for 1.f playback rate";

    EXPECT_EQ( eventLength, audioEvent->getEventLength() )
        << "expected event length to be unchanged for 1.f playback rate";

    EXPECT_EQ( eventEnd, audioEvent->getEventEnd() )
        << "expected event end to be unchanged for 1.f playback rate";

    // adjust playback rate to 2.f (twice the original speed)

    audioEvent->setPlaybackRate( 2.f );

    EXPECT_EQ( eventStart, audioEvent->getEventStart() )
        << "expected event start at double playback rate to remain unchanged";

    EXPECT_EQ( eventLength / 2, audioEvent->getEventLength() )
        << "expected event length at double playback rate to be at half the original length";

    EXPECT_EQ( eventStart + ( eventLength / 2 ), audioEvent->getEventEnd() )
        << "expected event end at double playback rate to be below the original offset";

    // adjust playback rate to .5f (half the original speed)

    audioEvent->setPlaybackRate( .5f );

    EXPECT_EQ( eventStart, audioEvent->getEventStart() )
        << "expected event start at half playback rate to remain unchanged";

    EXPECT_EQ( eventLength * 2, audioEvent->getEventLength() )
        << "expected event length at half playback rate to be at twice the original length";

    EXPECT_EQ( eventStart + ( eventLength * 2 ), audioEvent->getEventEnd() )
        << "expected event end at half playback rate to be above the original offset";

    delete audioEvent;
}

TEST( SampleEvent, PlaybackRateCustomRange )
{
    SampleEvent* audioEvent = new SampleEvent();

    // properties for 1.f playback rate
    int rangeStart  = 1000;
    int rangeLength = 1000;
    int rangeEnd    = rangeStart + rangeLength - 1;

    audioEvent->setBufferRangeStart( rangeStart );
    audioEvent->setBufferRangeEnd( rangeEnd );

    EXPECT_EQ( rangeStart, audioEvent->getBufferRangeStart() )
        << "expected buffer range start to be unchanged for 1.f playback rate";

    EXPECT_EQ( rangeLength, audioEvent->getBufferRangeLength() )
        << "expected buffer range length to be unchanged for 1.f playback rate";

    EXPECT_EQ( rangeEnd, audioEvent->getBufferRangeEnd() )
        << "expected buffer range end to be unchanged for 1.f playback rate";

    // adjust playback rate to 2.f (twice the original speed)

    audioEvent->setPlaybackRate( 2.f );

    EXPECT_EQ( rangeStart, audioEvent->getBufferRangeStart() )
        << "expected buffer range start at double playback rate to remain unchanged";

    EXPECT_EQ( rangeLength / 2, audioEvent->getBufferRangeLength() )
        << "expected buffer range length at double playback rate to be at half the original length";

    EXPECT_EQ( rangeStart + ( rangeLength / 2 ), audioEvent->getBufferRangeEnd() )
        << "expected buffer range end at double playback rate to be below the original offset";

    // adjust playback rate to .5f (half the original speed)

    audioEvent->setPlaybackRate( .5f );

    EXPECT_EQ( rangeStart, audioEvent->getBufferRangeStart() )
        << "expected buffer range start at half playback rate to remain unchanged";

    EXPECT_EQ( rangeLength * 2, audioEvent->getBufferRangeLength() )
        << "expected buffer range length at half playback rate to be at twice the original length";

    EXPECT_EQ( rangeStart + ( rangeLength * 2 ), audioEvent->getBufferRangeEnd() )
        << "expected buffer range end at half playback rate to be above the original offset";

    delete audioEvent;
}
