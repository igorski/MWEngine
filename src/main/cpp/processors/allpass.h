// Allpass filter declaration
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

#ifndef _allpass_
#define _allpass_

#include "baseprocessor.h"
#include "denormals.h"

class allpass
{
public:
					allpass();
			void	setbuffer(SAMPLE_TYPE *buf, int size);
	inline  SAMPLE_TYPE	process(SAMPLE_TYPE inp);
			void	mute();
			void	setfeedback(SAMPLE_TYPE val);
			SAMPLE_TYPE	getfeedback();
// private:
	SAMPLE_TYPE	feedback;
	SAMPLE_TYPE	*buffer;
	int		bufsize;
	int		bufidx;
};


// Big to inline - but crucial for speed

inline SAMPLE_TYPE allpass::process(SAMPLE_TYPE input)
{
	SAMPLE_TYPE output;
	SAMPLE_TYPE bufout;
	
	bufout = buffer[bufidx];
	undenormalise(bufout);
	
	output = -input + bufout;
	buffer[bufidx] = input + (bufout*feedback);

	if(++bufidx>=bufsize) bufidx = 0;

	return output;
}

#endif//_allpass

//ends