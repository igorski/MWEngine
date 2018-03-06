#include "../../modules/lfo.h"
#include "../../definitions/waveforms.h"
#include "../../utilities/tablepool.h"

TEST( LFO, Constructor )
{
    LFO* lfo = new LFO();

    EXPECT_EQ( lfo->getRate(), .1 ) << "expected default LFO rate to equal its minimum allowed rate";
    EXPECT_EQ( lfo->getWave(), WaveForms::SINE ) << "expected default LFO wave to be sine";

    delete lfo;
}

TEST( LFO, GenerateWaveformOnDemand )
{
    int waveform = WaveForms::SINE;
    std::string tableId = SSTR( waveform );

    ASSERT_FALSE( TablePool::hasTable( tableId )) << "expected TablePool not to contain a table for this id";

    LFO* lfo = new LFO();

    ASSERT_TRUE( TablePool::hasTable( tableId ))
        << "expected TablePool to contain a table for this id as LFO has generated it";

    delete lfo;

    ASSERT_FALSE( TablePool::hasTable( tableId ))
        << "expected TablePool to have removed the table for this id as LFO had generated it";
}

TEST( LFO, UseWaveformFromPool )
{
    int waveform = WaveForms::TRIANGLE;
    std::string tableId = SSTR( waveform );

    // pre-allocate Table for given waveform in TablePool
    TablePool::setTable( new WaveTable( 100, 10.f ), tableId );

    LFO* lfo = new LFO();

    // LFO constructs by default for SINE (see constructor test), explicitly set new waveform
    // this will trigger a fetch from the TablePool
    lfo->setWave( waveform );

    EXPECT_EQ( lfo->getWave(), waveform ) << "expected waveform to have been changed";

    delete lfo;

    ASSERT_TRUE( TablePool::hasTable( tableId ))
        << "expected TablePool to still contain the table as it was allocated from outside the LFO";

    // free memory allocated to the Table
    TablePool::removeTable( tableId, true );
}
