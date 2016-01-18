#include "../wavetable.h"

TEST( WaveTable, Constructor )
{
    int length       = randomInt( 2, 256 );
    float frequency  = randomFloat( 20, 880 );
    WaveTable* table = new WaveTable( length, frequency );

    EXPECT_EQ( length, table->tableLength )
        << "expected WaveTable length to equal the value given in the constructor";

    EXPECT_EQ( frequency, table->getFrequency() )
        << "expected WaveTable frequency to equal the value given in the constructor";

    EXPECT_EQ( 0.0, table->getAccumulator() )
        << "expected WaveTable accumulator to be 0.0 upon construction";

    ASSERT_FALSE( table->hasContent() )
        << "expected WaveTable to have no content upon construction, but it did";

    delete table;
}

TEST( WaveTable, Frequency )
{
    int length       = randomInt( 2, 256 );
    float frequency  = randomFloat( 20, 880 );
    WaveTable* table = new WaveTable( length, frequency );

    EXPECT_EQ( frequency, table->getFrequency() )
        << "expected WaveTable frequency to equal the value given in the constructor";

    float frequency2 = frequency;

    while ( frequency2 == frequency )
        frequency2 = randomFloat( 20, 880 );

    table->setFrequency( frequency2 );

    EXPECT_EQ( frequency2, table->getFrequency() )
        << "expected WaveTable frequency to equal to last set value";

    delete table;
}

TEST( WaveTable, HasContent )
{
    int length       = randomInt( 2, 256 );
    float frequency  = randomFloat( 20, 880 );
    WaveTable* table = new WaveTable( length, frequency );

    ASSERT_FALSE( table->hasContent() )
        << "expected WaveTable to have no content upon construction";

    SAMPLE_TYPE* buffer = table->getBuffer();

    for ( int i = 0; i < length; ++i )
        buffer[ i ] = randomSample( -1.0, 1.0 );

    ASSERT_TRUE( table->hasContent() )
        << "expected WaveTable to report to have content after assignment of sample values in buffer";

    delete table;
}

TEST( WaveTable, Accumulator )
{
    int length       = randomInt( 2, 256 );
    float frequency  = randomFloat( 20, 880 );
    WaveTable* table = new WaveTable( length, frequency );

    EXPECT_EQ( 0.0, table->getAccumulator() )
        << "expected WaveTable accumulator to be 0.0 upon construction";

    for ( int i = 0; i < randomInt( 1, 100 ); ++i )
    {
        SAMPLE_TYPE value = randomSample( 0.0, ( SAMPLE_TYPE ) length );
        table->setAccumulator( value );

        EXPECT_EQ( value, table->getAccumulator() )
            << "expected WaveTable accumulator to be " << value << " after setter, got " << table->getAccumulator() << " instead";
    }
    delete table;
}

TEST( WaveTable, CloneTable )
{
    int length1      = randomInt( 2, 256 );
    float frequency1  = randomFloat( 20, 880 );
    WaveTable* table1 = new WaveTable( length1, frequency1 );

    int length2       = length1;
    float frequency2  = frequency1;

    // ensure secondary table has a different frequency / length

    while ( length2 == length1 )
        length2 = randomInt( 2, 256 );

    while ( frequency2 == frequency1 )
        frequency2 = randomFloat( 20, 880 );

    WaveTable* table2 = new WaveTable( length2, frequency2 );

    ASSERT_FALSE( table2->tableLength == table1->tableLength )
        << "expected WaveTables to be of unequal length during clone test";

    table1->cloneTable( table2 );

    ASSERT_TRUE( table2->tableLength == table1->tableLength )
        << "expected WaveTables to be of equal length after clone";

    for ( int i = 0; i < length2; ++i )
    {
        EXPECT_EQ( table1->getBuffer()[ i ], table2->getBuffer()[ i ])
            << "expected WaveTables cloned buffer to have sample contents equal to its source, but it didn't";
    }

    delete table1;
    delete table2;
}

TEST( WaveTable, Clone )
{
    int length       = randomInt( 2, 256 );
    float frequency  = randomFloat( 20, 880 );
    WaveTable* table = new WaveTable( length, frequency );

    table->setAccumulator( randomSample( 0.0, ( SAMPLE_TYPE ) length ));

    // randomly create content

    if ( randomBool() )
    {
        for ( int i = 0; i < length; ++i )
            table->getBuffer()[ i ] = randomSample( -1.0, 1.0 );
    }

    // create clone and validate its properties against the source table

    WaveTable* clone = table->clone();

    EXPECT_EQ( table->tableLength, clone->tableLength )
        << "expected cloned WaveTables length to equal the source WaveTables";

    EXPECT_EQ( table->getFrequency(), clone->getFrequency() )
        << "expected cloned WaveTable frequency to equal the source WaveTables";

    EXPECT_EQ( table->getAccumulator(), clone->getAccumulator() )
        << "expected cloned WaveTable accumulator to equal the source WaveTables";

    EXPECT_EQ( table->hasContent(), table->hasContent() )
        << "expected cloned WaveTable hasContent value to equal the source WaveTables";

    if ( table->hasContent() )
    {
        for ( int i = 0; i < length; ++i ) {
            EXPECT_EQ( table->getBuffer()[ i ], clone->getBuffer()[ i ])
                << "expected cloned WaveTable buffer to have equal buffer contents, but it didn't";
        }
    }
    delete table;
    delete clone;
}
