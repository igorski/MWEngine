#include "../../events/sampleevent.h"
#include "../../instruments/sampledinstrument.h"

TEST( SampleEvent, Constructor )
{
    SampledInstrument* instrument = new SampledInstrument();
    SampleEvent* audioEvent = new SampleEvent( instrument );

    ASSERT_TRUE( instrument == audioEvent->getInstrument() )
        << "expected instrument to equal the value given in constructor";

    deleteAudioEvent( audioEvent );
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

    deleteAudioEvent( audioEvent );
}

TEST( SampleEvent, PlayStop )
{
    SampledInstrument* instrument = new SampledInstrument();
    SampleEvent* audioEvent = new SampleEvent( instrument );

    // expect AudioEvent not be in any of the event queues of the instrument after construction

    bool found = false;
    for ( int i = 0; i < instrument->getEvents()->size(); ++i )
    {
        if ( instrument->getEvents()->at( i ) == audioEvent )
            found = true;
    }

    ASSERT_FALSE( found )
        << "expected event not to be present in the event list after construction";

    found = false;
    for ( int i = 0; i < instrument->getLiveEvents()->size(); ++i )
    {
        if ( instrument->getLiveEvents()->at( i ) == audioEvent )
            found = true;
    }

    ASSERT_FALSE( found )
        << "expected event not to be present in the live event list after construction";

    // 1. activate play-state

    audioEvent->play();

    // expect event to be present in live events list

    found = false;
    for ( int i = 0; i < instrument->getEvents()->size(); ++i )
    {
        if ( instrument->getEvents()->at( i ) == audioEvent )
            found = true;
    }

    ASSERT_FALSE( found )
        << "expected event not to be present in the sequenced event list after invocation of play()";

    found = false;
    for ( int i = 0; i < instrument->getLiveEvents()->size(); ++i )
    {
        if ( instrument->getLiveEvents()->at( i ) == audioEvent )
            found = true;
    }

    ASSERT_TRUE( found )
        << "expected event to be present in the live event list after invocation of play()";

    ASSERT_TRUE( audioEvent->isEnabled() )
        << "expected SampleEvent to be enabled after invocation of play()";

    // 2. deactive play-state

    audioEvent->stop();

    // expect event not be in the event lists anymore

    found = false;
    for ( int i = 0; i < instrument->getEvents()->size(); ++i )
    {
        if ( instrument->getEvents()->at( i ) == audioEvent )
            found = true;
    }

    ASSERT_FALSE( found )
        << "expected event not to be present in the event list after invocation of stop()";

    found = false;
    for ( int i = 0; i < instrument->getLiveEvents()->size(); ++i )
    {
        if ( instrument->getLiveEvents()->at( i ) == audioEvent )
            found = true;
    }

    ASSERT_FALSE( found )
        << "expected event not to be present in the live event list after invocation of stop()";

    ASSERT_FALSE( audioEvent->isEnabled() )
        << "expected SampleEvent not to be enabled after invocation of stop()";

    deleteAudioEvent( audioEvent );
}

// test overridden mix buffer method

TEST( SampleEvent, MixBuffer )
{
    SampleEvent* audioEvent = new SampleEvent( new SampledInstrument() );

    int sampleLength = randomInt( 8, 24 );
    int sampleStart  = randomInt( 0, ( int )( sampleLength / 2 ));

    audioEvent->setSampleStart ( sampleStart );
    audioEvent->setSampleLength( sampleLength );

    int sampleEnd = audioEvent->getSampleEnd();

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
                    << "expected mixed sample at " << i << " to be equal the calculated expected sample at read offset " << r;
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
                        ( loopStartReadPointer >= sampleStart && loopStartReadPointer <= sampleEnd ) |
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
    deleteAudioEvent( audioEvent );
    delete targetBuffer;
    delete buffer;
}
