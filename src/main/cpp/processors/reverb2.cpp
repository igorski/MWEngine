// Reverb model implementation
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

#include "reverb2.h"
namespace MWEngine {
//all pass
allpass::allpass()
{
	bufidx = 0;
}

void allpass::setbuffer(SAMPLE_TYPE *buf, int size)
{
	buffer = buf;
	bufsize = size;
}

void allpass::mute()
{
	for (int i=0; i<bufsize; i++)
		buffer[i]=0;
}

void allpass::setfeedback(SAMPLE_TYPE val)
{
	feedback = val;
}

SAMPLE_TYPE allpass::getfeedback()
{
	return feedback;
}

//comb
comb::comb()
{
	filterstore = 0;
	bufidx = 0;
}

void comb::setbuffer(SAMPLE_TYPE *buf, int size)
{
	buffer = buf;
	bufsize = size;
}

void comb::mute()
{
	for (int i=0; i<bufsize; i++)
		buffer[i]=0;
}

void comb::setdamp(SAMPLE_TYPE val)
{
	damp1 = val;
	damp2 = 1-val;
}

SAMPLE_TYPE comb::getdamp()
{
	return damp1;
}

void comb::setfeedback(SAMPLE_TYPE val)
{
	feedback = val;
}

SAMPLE_TYPE comb::getfeedback()
{
	return feedback;
}

//Reverb2
Reverb2::Reverb2()
{
	// Tie the components to their buffers
	combL[0].setbuffer(bufcombL1,combtuningL1);
	combR[0].setbuffer(bufcombR1,combtuningR1);
	combL[1].setbuffer(bufcombL2,combtuningL2);
	combR[1].setbuffer(bufcombR2,combtuningR2);
	combL[2].setbuffer(bufcombL3,combtuningL3);
	combR[2].setbuffer(bufcombR3,combtuningR3);
	combL[3].setbuffer(bufcombL4,combtuningL4);
	combR[3].setbuffer(bufcombR4,combtuningR4);
	combL[4].setbuffer(bufcombL5,combtuningL5);
	combR[4].setbuffer(bufcombR5,combtuningR5);
	combL[5].setbuffer(bufcombL6,combtuningL6);
	combR[5].setbuffer(bufcombR6,combtuningR6);
	combL[6].setbuffer(bufcombL7,combtuningL7);
	combR[6].setbuffer(bufcombR7,combtuningR7);
	combL[7].setbuffer(bufcombL8,combtuningL8);
	combR[7].setbuffer(bufcombR8,combtuningR8);
	allpassL[0].setbuffer(bufallpassL1,allpasstuningL1);
	allpassR[0].setbuffer(bufallpassR1,allpasstuningR1);
	allpassL[1].setbuffer(bufallpassL2,allpasstuningL2);
	allpassR[1].setbuffer(bufallpassR2,allpasstuningR2);
	allpassL[2].setbuffer(bufallpassL3,allpasstuningL3);
	allpassR[2].setbuffer(bufallpassR3,allpasstuningR3);
	allpassL[3].setbuffer(bufallpassL4,allpasstuningL4);
	allpassR[3].setbuffer(bufallpassR4,allpasstuningR4);

	// Set default values
	allpassL[0].setfeedback(0.5f);
	allpassR[0].setfeedback(0.5f);
	allpassL[1].setfeedback(0.5f);
	allpassR[1].setfeedback(0.5f);
	allpassL[2].setfeedback(0.5f);
	allpassR[2].setfeedback(0.5f);
	allpassL[3].setfeedback(0.5f);
	allpassR[3].setfeedback(0.5f);
	setwet(initialwet);
	setroomsize(initialroom);
	setdry(initialdry);
	setdamp(initialdamp);
	setwidth(initialwidth);
	setmode(initialmode);

	// Buffer will be full of rubbish - so we MUST mute them
	mute();
}

void Reverb2::mute()
{
	if (getmode() >= freezemode)
		return;

	for (int i=0;i<numcombs;i++)
	{
		combL[i].mute();
		combR[i].mute();
	}
	for (int i=0;i<numallpasses;i++)
	{
		allpassL[i].mute();
		allpassR[i].mute();
	}
}
void Reverb2::process( AudioBuffer* audioBuffer, bool isMonosource )
{
    int numsamples = audioBuffer->bufferSize;
    int skip = 1;

    SAMPLE_TYPE* inputL  = audioBuffer->getBufferForChannel( 0 );
    SAMPLE_TYPE* inputR  = audioBuffer->getBufferForChannel( 1 );
    SAMPLE_TYPE* outputL = audioBuffer->getBufferForChannel( 0 );
    SAMPLE_TYPE* outputR = audioBuffer->getBufferForChannel( 1 );

    	SAMPLE_TYPE outL,outR,input;

    	while(numsamples-- > 0)
    	{
    		outL = outR = 0;
    		input = (*inputL + *inputR) * gain;

    		// Accumulate comb filters in parallel
    		for(int i=0; i<numcombs; i++)
    		{
    			outL += combL[i].process(input);
    			outR += combR[i].process(input);
    		}

    		// Feed through allpasses in series
    		for(int i=0; i<numallpasses; i++)
    		{
    			outL = allpassL[i].process(outL);
    			outR = allpassR[i].process(outR);
    		}

    		// Calculate output REPLACING anything already there
    		*outputL = outL*wet1 + outR*wet2 + *inputL*dry;
    		*outputR = outR*wet1 + outL*wet2 + *inputR*dry;

    		// Increment sample pointers, allowing for interleave (if any)
    		inputL += skip;
    		inputR += skip;
    		outputL += skip;
    		outputR += skip;
    }

}

void Reverb2::update()
{
// Recalculate internal values after parameter change

	wet1 = wet*(width/2 + 0.5f);
	wet2 = wet*((1-width)/2);

	if (mode >= freezemode)
	{
		roomsize1 = 1;
		damp1 = 0;
		gain = muted;
	}
	else
	{
		roomsize1 = roomsize;
		damp1 = damp;
		gain = fixedgain;
	}

	for(int i=0; i<numcombs; i++)
	{
		combL[i].setfeedback(roomsize1);
		combR[i].setfeedback(roomsize1);
	}

	for(int i=0; i<numcombs; i++)
	{
		combL[i].setdamp(damp1);
		combR[i].setdamp(damp1);
	}
}

// The following get/set functions are not inlined, because
// speed is never an issue when calling them, and also
// because as you develop the reverb model, you may
// wish to take dynamic action when they are called.

void Reverb2::setroomsize(float value)
{
	roomsize = (value*scaleroom) + offsetroom;
	update();
}

float Reverb2::getroomsize()
{
	return (roomsize-offsetroom)/scaleroom;
}

void Reverb2::setdamp(float value)
{
	damp = value*scaledamp;
	update();
}

float Reverb2::getdamp()
{
	return damp/scaledamp;
}

void Reverb2::setwet(float value)
{
	wet = value*scalewet;
	update();
}

float Reverb2::getwet()
{
	return wet/scalewet;
}

void Reverb2::setdry(float value)
{
	dry = value*scaledry;
}

float Reverb2::getdry()
{
	return dry/scaledry;
}

void Reverb2::setwidth(float value)
{
	width = value;
	update();
}

float Reverb2::getwidth()
{
	return width;
}

void Reverb2::setmode(float value)
{
	mode = value;
	update();
}

float Reverb2::getmode()
{
	if (mode >= freezemode)
		return 1;
	else
		return 0;
}
}
//ends
