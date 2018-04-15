#include "../../utilities/sampleutility.h"
#include "../../events/sampleevent.h"

TEST( SampleUtility, PitchShift )
{
    SampleEvent* sampleEvent = new SampleEvent();

    EXPECT_EQ( 1.f, floatRounding( sampleEvent->getPlaybackRate(), 0 ))
        << "expected a newly constructed SampleEvent to have a playback rate of 1";

    SampleUtility::pitchShift( sampleEvent, 12 );

    EXPECT_EQ( 2.f, floatRounding( sampleEvent->getPlaybackRate(), 0 ))
        << "expected a pitch shift of a full octave up to give SampleEvent a playback rate of 2";

    SampleUtility::pitchShift( sampleEvent, -12 );

    EXPECT_EQ( .5f, floatRounding( sampleEvent->getPlaybackRate(), 1 ))
        << "expected a pitch shift of a full octave down to give SampleEvent a playback rate of .5";

    delete sampleEvent;
}

TEST( SampleUtility, PitchShiftSampleEventWithAlternateSampleRate )
{
    SampleEvent* sampleEvent = new SampleEvent();
    AudioBuffer* audioBuffer = new AudioBuffer( 1, 24 );

    // ensure SampleEvents sample is at half the sample rate of the engine
    int sampleRate = AudioEngineProps::SAMPLE_RATE / 2;
    sampleEvent->setSample( audioBuffer, sampleRate );

    EXPECT_EQ( .5f, floatRounding( sampleEvent->getPlaybackRate(), 1 ))
        << "expected SampleEvent w/ a sample at half the engine's sample rate to have a playback rate of .5f";

    SampleUtility::pitchShift( sampleEvent, 12 );

    EXPECT_EQ( 1.f, floatRounding( sampleEvent->getPlaybackRate(), 0 ))
        << "expected a pitch shift of a full octave up to give SampleEvent w/ a sample at half "
        << "the engine's sample rate a playback rate of 1";

    SampleUtility::pitchShift( sampleEvent, -12 );

    EXPECT_EQ( .25f, floatRounding( sampleEvent->getPlaybackRate(), 2 ))
        << "expected a pitch shift of a full octave down to give SampleEvent w/ a sample at half "
        << "the engine's sample rate a playback rate of .25";

    delete sampleEvent;
    delete audioBuffer;
}
