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

    chain->removeProcessor( processor1 );

    // ensure chain still holds secondary processor

    EXPECT_EQ( chain->getActiveProcessors().size(), 1 )
        << "expected ProcessingChain to contain one active processors after removal of a single processor";

    ASSERT_TRUE( chain->getActiveProcessors().at( 0 ) == processor2 )
        << "expected ProcessingChain to hold non-removed processor at first position in its chain";

    // remove secondary processor

    chain->removeProcessor( processor2 );

    // ensure chain is now empty

    EXPECT_EQ( chain->getActiveProcessors().size(), 0 )
        << "expected ProcessingChain to contain no active processors after removal of a all processors";

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