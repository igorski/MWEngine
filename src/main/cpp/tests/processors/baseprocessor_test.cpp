#include "../../processors/baseprocessor.h"
#include "../../processingchain.h"

TEST( BaseProcessor, ChainRemovalOnDestruction )
{
    BaseProcessor* processor = new BaseProcessor();
    ProcessingChain* chain   = new ProcessingChain();

    chain->addProcessor( processor );

    EXPECT_EQ( chain->getActiveProcessors().size(), 1 )
        << "expected processor to have been added to the chain";

    delete processor;

    EXPECT_EQ( chain->getActiveProcessors().size(), 0 )
        << "expected processor to have been removed from the chain after destruction";

    delete chain;
}
