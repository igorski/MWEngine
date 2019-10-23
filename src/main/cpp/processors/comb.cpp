// Comb filter implementation
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

#include "comb.h"

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

// ends
