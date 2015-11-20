#include "../../events/drumevent.h"
#include "../../instruments/druminstrument.h"
#include "../../utilities/samplemanager.h"
#include "../../utilities/utils.h"

TEST( DrumEvent, Constructor )
{
    int MAX_TYPE = 3;

    // prepare samplemanager

    for ( int i = 0; i < MAX_TYPE; ++i )
        SampleManager::setSample( SSTR( i ), randomAudioBuffer() );

    int position               = randomInt( 0, 15 );
    int type                   = randomInt( 0, MAX_TYPE );
    int timbre                 = randomInt( 0, 1 );
    DrumInstrument* instrument = new DrumInstrument();

    DrumEvent* audioEvent = new DrumEvent( position, type, timbre, instrument );

    EXPECT_EQ( position, audioEvent->position )
        << "expected position to equal the value given in constructor";

    EXPECT_EQ( type, audioEvent->getType() )
        << "expected type to equal the value given in constructor";

    EXPECT_EQ( timbre, audioEvent->getTimbre() )
        << "expected timbre to equal the value given in constructor";

    // clean samplemanager

    for ( int i = 0; i < MAX_TYPE; ++i )
        SampleManager::removeSample( SSTR( i ) );

    deleteAudioEvent( audioEvent );
}

TEST( DrumEvent, GettersSetters )
{
    int MAX_TYPE = 3;

    // prepare samplemanager

    for ( int i = 0; i < MAX_TYPE; ++i )
        SampleManager::setSample( SSTR( i ), randomAudioBuffer() );

    int position               = randomInt( 0, 15 );
    int type                   = randomInt( 0, MAX_TYPE );
    int timbre                 = randomInt( 0, 1 );
    DrumInstrument* instrument = new DrumInstrument();

    DrumEvent* audioEvent = new DrumEvent( position, type, timbre, instrument );

    // randomize type values
    type   = randomInt( 0, 3 );
    timbre = randomInt( 0, 1 );

    audioEvent->setType( type );
    audioEvent->setTimbre( timbre );

    EXPECT_EQ( type, audioEvent->getType() )
        << "expected type to equal the value given in the setter";

    EXPECT_EQ( timbre, audioEvent->getTimbre() )
        << "expected timbre to equal the value given in the setter";

    // clean samplemanager

        for ( int i = 0; i < MAX_TYPE; ++i )
            SampleManager::removeSample( SSTR( i ) );

    deleteAudioEvent( audioEvent );
}

// test overridden lock method

TEST( DrumEvent, LockedState )
{
    int MAX_TYPE = 3;

    // prepare samplemanager

    for ( int i = 0; i < MAX_TYPE; ++i )
        SampleManager::setSample( SSTR( i ), randomAudioBuffer() );

    int position               = randomInt( 0, 15 );
    int type                   = randomInt( 0, MAX_TYPE );
    int timbre                 = randomInt( 0, 1 );
    DrumInstrument* instrument = new DrumInstrument();

    DrumEvent* audioEvent = new DrumEvent( position, type, timbre, instrument );

    ASSERT_FALSE( audioEvent->isLocked() )
        << "expected audio event to be unlocked after construction";

    audioEvent->lock();

    ASSERT_TRUE( audioEvent->isLocked() )
        << "expected audio event to be locked after locking";

    audioEvent->unlock();

    ASSERT_FALSE( audioEvent->isLocked() )
        << "expected audio event to be unlocked after unlocking";

    // clean samplemanager

    for ( int i = 0; i < MAX_TYPE; ++i )
        SampleManager::removeSample( SSTR( i ) );

    deleteAudioEvent( audioEvent );
}
