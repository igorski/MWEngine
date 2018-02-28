#include "../../processors/reverb.h"

TEST( Reverb, Process )
{
    AudioEngineProps::BUFFER_SIZE = 48000;
    Reverb* reverb = new Reverb();

    AudioBuffer* buffer = new AudioBuffer( 1, 2048 );
    fillAudioBuffer( buffer );

    reverb->process( buffer, buffer->amountOfChannels == 1 );

    delete reverb;
}