#include "../processingchain.h"
#include "../processors/baseprocessor.h"

TEST( ProcessingChain, ProcessorAddition )
{
    BaseProcessor* processor1 = new BaseProcessor();
    BaseProcessor* processor2 = new BaseProcessor();

    ProcessingChain* chain = new ProcessingChain();

    // ensure chain is empty upon construction

    EXPECT_EQ( chain->getActiveProcessors().size(), 0 )
        << "expected ProcessingChain to contain no active processors upon construction";

    // add a processor

    chain->addProcessor( processor1 );

    EXPECT_EQ( chain->getActiveProcessors().size(), 1 )
        << "expected ProcessingChain to contain one active processor upon addition of a processor";

    ASSERT_TRUE( chain->getActiveProcessors().at( 0 ) == processor1 )
        << "expected ProcessingChain to hold the added processor in its chain";

    // ensure no double addition of the same processor is possible

    chain->addProcessor( processor1 );

    EXPECT_EQ( chain->getActiveProcessors().size(), 1 )
        << "expected ProcessingChain to still contain one active processor upon attempted addition of previously added processor";

    // add another processor

    chain->addProcessor( processor2 );

    EXPECT_EQ( chain->getActiveProcessors().size(), 2 )
            << "expected ProcessingChain to contain two active processor upon addition of another processor";

    ASSERT_TRUE( chain->getActiveProcessors().at( 0 ) == processor1 )
        << "expected ProcessingChain to hold the added processor in its chain";

    ASSERT_TRUE( chain->getActiveProcessors().at( 1 ) == processor2 )
        << "expected ProcessingChain to hold the added processor in its chain";

    delete processor1;
    delete processor2;
    delete chain;
}

TEST( ProcessingChain, ProcessorRemoval )
{
    BaseProcessor* processor1 = new BaseProcessor();
    BaseProcessor* processor2 = new BaseProcessor();

    ProcessingChain* chain = new ProcessingChain();

    // add processors

    chain->addProcessor( processor1 );
    chain->addProcessor( processor2 );

    // ensure they have been added

    EXPECT_EQ( chain->getActiveProcessors().size(), 2 )
        << "expected ProcessingChain to contain two active processors after addition";

    // remove first processor

    bool removed = chain->removeProcessor( processor1 );

    ASSERT_TRUE( removed ) << "expected removeProcessor to have returned success upon removal of processor1";

    // ensure chain still holds secondary processor

    EXPECT_EQ( chain->getActiveProcessors().size(), 1 )
        << "expected ProcessingChain to contain one active processors after removal of a single processor";

    ASSERT_TRUE( chain->getActiveProcessors().at( 0 ) == processor2 )
        << "expected ProcessingChain to hold non-removed processor at first position in its chain";

    // remove secondary processor

    removed = chain->removeProcessor( processor2 );

    ASSERT_TRUE( removed ) << "expected removeProcessor to have returned success upon removal of processor2";

    // ensure chain is now empty

    EXPECT_EQ( chain->getActiveProcessors().size(), 0 )
        << "expected ProcessingChain to contain no active processors after removal of a all processors";

    // validate success is false when removing a processor that is not/no longer part of the chain

    removed = chain->removeProcessor( processor1 );

    ASSERT_FALSE( removed ) << "expected removeProcessor to have returned false upon removal of not-added processor1";

    delete processor1;
    delete processor2;
    delete chain;
}

TEST( ProcessingChain, Reset )
{
    BaseProcessor* processor1 = new BaseProcessor();
    BaseProcessor* processor2 = new BaseProcessor();

    ProcessingChain* chain = new ProcessingChain();

    chain->addProcessor( processor1 );
    chain->addProcessor( processor2 );

    EXPECT_EQ( chain->getActiveProcessors().size(), 2 )
        << "expected chain to hold 2 active processors after addition";

    chain->reset();

    EXPECT_EQ( chain->getActiveProcessors().size(), 0 )
        << "expected chain to hold no active processors after reset";

    delete processor1;
    delete processor2;
    delete chain;
}

TEST( ProcessingChain, hasProcessors )
{
    BaseProcessor* processor = new BaseProcessor();

    ProcessingChain* chain = new ProcessingChain();

    ASSERT_FALSE( chain->hasProcessors() ) << "expected chain to have no processor upon construction";

    chain->addProcessor( processor );

    ASSERT_TRUE( chain->hasProcessors() ) << "expected chain to have processors after addition";

    chain->reset();

    ASSERT_FALSE( chain->hasProcessors() ) << "expected chain to have no processors after reset";

    delete processor;
    delete chain;
}

TEST( ProcessingChain, amountOfProcesors )
{
    BaseProcessor* processor1 = new BaseProcessor();
    BaseProcessor* processor2 = new BaseProcessor();

    ProcessingChain* chain = new ProcessingChain();
    EXPECT_EQ( 0, chain->amountOfProcessors()) << "expected no processors upon construction";

    chain->addProcessor( processor1 );
    EXPECT_EQ( 1, chain->amountOfProcessors()) << "expected 1 processor after addition";

    chain->addProcessor( processor2 );
    EXPECT_EQ( 2, chain->amountOfProcessors()) << "expected 2 processors after addition";

    chain->removeProcessor( processor1 );
    EXPECT_EQ( 1, chain->amountOfProcessors()) << "expected 1 processor after removal";

    chain->reset();
    EXPECT_EQ( 0, chain->amountOfProcessors()) << "expected no processors after reset";

    delete processor1;
    delete processor2;
    delete chain;
}

TEST( ProcessingChain, getProcessorAt )
{
    BaseProcessor* processor1 = new BaseProcessor();
    BaseProcessor* processor2 = new BaseProcessor();

    ProcessingChain* chain = new ProcessingChain();
    ASSERT_TRUE( nullptr == chain->getProcessorAt( 0 )) << "expected null pointer when index is out of range";

    chain->addProcessor( processor1 );
    chain->addProcessor( processor2 );

    ASSERT_TRUE( processor1 = chain->getProcessorAt( 0 ));
    ASSERT_TRUE( processor2 = chain->getProcessorAt( 1 ));

    chain->reset();

    delete processor1;
    delete processor2;
    delete chain;
}