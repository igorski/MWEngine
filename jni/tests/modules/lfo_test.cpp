#include "../../modules/lfo.h"
#include "../../definitions/waveforms.h"
#include "../../utilities/tablepool.h"

TEST( LFO, Constructor )
{
    LFO* lfo = new LFO();

    EXPECT_FLOAT_EQ( lfo->getRate(), .1f ) << "expected default LFO rate to equal its minimum allowed rate";
    EXPECT_EQ( lfo->getWave(), WaveForms::SILENCE ) << "expected default LFO wave to be silent";

    delete lfo;
}

TEST( LFO, UseWaveformFromPool )
{
    LFO* lfo = new LFO();

    int waveform = WaveForms::TRIANGLE;
    std::string tableId = SSTR( waveform );

    // pre-allocate Table for given waveform in TablePool
    TablePool::setTable( new WaveTable( 100, 10.f ), tableId );

    // setting a new waveform will trigger a fetch from the TablePool
    lfo->setWave( waveform );

    EXPECT_EQ( lfo->getWave(), waveform ) << "expected waveform to have been changed";

    delete lfo;

    ASSERT_TRUE( TablePool::hasTable( tableId ))
        << "expected TablePool to still contain the table as it was allocated from outside the LFO";

    // free memory allocated to the Table
    TablePool::removeTable( tableId, true );
}

TEST( LFO, GenerateWaveformOnDemand )
{
    LFO* lfo = new LFO();

    int waveform = WaveForms::SINE;
    std::string tableId = SSTR( waveform );

    ASSERT_FALSE( TablePool::hasTable( tableId )) << "expected TablePool not to contain a table for this id";

    // setting a new waveform will trigger a fetch from the TablePool, but as there
    // is no table registered for given waveform, it will be generated and registered inline
    lfo->setWave( waveform );

    ASSERT_TRUE( TablePool::hasTable( tableId ))
        << "expected TablePool to contain a table for this id as LFO has generated it";

    delete lfo;

    ASSERT_FALSE( TablePool::hasTable( tableId ))
        << "expected TablePool to have removed the table for this id as LFO had generated it";
}

TEST( LFO, SetRate )
{
    LFO* lfo = new LFO();

    float rate = randomFloat();

    lfo->setRate( rate );

    EXPECT_FLOAT_EQ( rate, lfo->getRate() ) << "expected rate to have been updated";

    delete lfo;
}

TEST( LFO, SetWave )
{
    LFO* lfo = new LFO();

    int waveform = WaveForms::PWM;

    lfo->setWave( waveform );

    EXPECT_EQ( waveform, lfo->getWave() ) << "expected wave to have been updated";

    delete lfo;
}
