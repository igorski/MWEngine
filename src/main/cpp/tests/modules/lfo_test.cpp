#include "../../modules/lfo.h"
#include "../../definitions/waveforms.h"
#include "../../utilities/tablepool.h"
#include "../../utilities/waveutil.h"

TEST( LFO, Constructor )
{
    LFO* lfo = new LFO();

    EXPECT_FLOAT_EQ( lfo->getRate(), .1f )
        << "expected default LFO rate to equal its minimum allowed rate";

    EXPECT_EQ( lfo->getWave(), WaveForms::SILENCE )
        << "expected default LFO wave to be silent";

    EXPECT_FLOAT_EQ( lfo->getDepth(), 1.f )
        << "expected default LFO depth to be full depth";

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

    EXPECT_EQ( lfo->getWave(), waveform )
        << "expected waveform to have been changed";

    delete lfo;

    // verify waveform still exists in the table (setting a waveform
    // should clone the table inside the LFO instance)

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

    ASSERT_FALSE( TablePool::hasTable( tableId ))
        << "expected TablePool not to contain a table for this id";

    // setting a new waveform will trigger a fetch from the TablePool, but as there
    // is no table registered for given waveform, it will be generated and registered inline
    lfo->setWave( waveform );

    EXPECT_EQ( lfo->getWave(), waveform )
        << "expected waveform to have been changed";

    ASSERT_FALSE( TablePool::hasTable( tableId ))
        << "expected TablePool not to contain a table for this id";

    delete lfo;
}

TEST( LFO, UniPolarConversion )
{
    LFO* lfo = new LFO();

    // bipolar waveform
    SAMPLE_TYPE buffer[ 5 ] = { -1.0, -0.5, 0, 0.5, 1.0 };
    WaveTable* table = new WaveTable( 5, 10.f );
    table->setBuffer( buffer );

    int waveform = WaveForms::SQUARE;
    std::string tableId = SSTR( waveform );

    // pre-allocate Table for given waveform in TablePool
    TablePool::setTable( table, tableId );

    // setting a new waveform will trigger a fetch from the TablePool
    // and should convert it to be a unipolar waveform
    lfo->setWave( waveform );

    ASSERT_TRUE( WaveUtil::isUnipolar( lfo->getTable()->getBuffer(), lfo->getTable()->tableLength ))
        << "expected table to have been cloned and converted to unipolar waveform";

    delete lfo;

    // free memory allocated to the Table
    //TODO: segmentation fault?
    //TablePool::removeTable( tableId, true );
}

TEST( LFO, SetRate )
{
    LFO* lfo = new LFO();

    float rate = randomFloat( LFO::MIN_RATE(), LFO::MAX_RATE() );

    lfo->setRate( rate );

    EXPECT_FLOAT_EQ( rate, lfo->getRate() )
        << "expected rate to have been updated";

    delete lfo;
}

TEST( LFO, SetRateMin )
{
    LFO* lfo = new LFO();

    float rate = LFO::MIN_RATE() - 10;

    lfo->setRate( rate );

    EXPECT_FLOAT_EQ( LFO::MIN_RATE(), lfo->getRate() )
        << "expected rate to have been sanitized to the defined minimum";

    delete lfo;
}

TEST( LFO, SetRateMax )
{
    LFO* lfo = new LFO();

    float rate = LFO::MAX_RATE() + 10;

    lfo->setRate( rate );

    EXPECT_FLOAT_EQ( LFO::MAX_RATE(), lfo->getRate() )
        << "expected rate to have been sanitized to the defined maximum";

    delete lfo;
}

TEST( LFO, SetWave )
{
    LFO* lfo = new LFO();

    int waveform = WaveForms::PWM;

    lfo->setWave( waveform );

    EXPECT_EQ( waveform, lfo->getWave() )
        << "expected wave to have been updated";

    delete lfo;
}

TEST( LFO, SetWaveCustom )
{
    LFO* lfo = new LFO();

    int waveform = WaveForms::PWM;

    lfo->setWave( waveform );
    lfo->setWave( WaveForms::SILENCE );

    EXPECT_EQ( waveform, lfo->getWave() )
        << "expected wave not to have been updated to silence";

    delete lfo;
}

TEST( LFO, SetDepth )
{
    LFO* lfo = new LFO();

    float depth = randomFloat();

    lfo->setDepth( depth );

    EXPECT_EQ( depth, lfo->getDepth() )
        << "expected depth to have been updated to match set value";

    delete lfo;
}
