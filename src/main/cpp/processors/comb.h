// Comb filter class declaration
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

#ifndef _comb_
#define _comb_

#include "denormals.h"
#include "baseprocessor.h"
class comb
{
public:
					comb();
			void	setbuffer(SAMPLE_TYPE *buf, int size);
	inline  SAMPLE_TYPE	process(SAMPLE_TYPE inp);
			void	mute();
			void	setdamp(SAMPLE_TYPE val);
			SAMPLE_TYPE	getdamp();
			void	setfeedback(SAMPLE_TYPE val);
			SAMPLE_TYPE	getfeedback();
private:
	SAMPLE_TYPE	feedback;
	SAMPLE_TYPE	filterstore;
	SAMPLE_TYPE	damp1;
	SAMPLE_TYPE	damp2;
	SAMPLE_TYPE	*buffer;
	int		bufsize;
	int		bufidx;
};


// Big to inline - but crucial for speed

inline SAMPLE_TYPE comb::process(SAMPLE_TYPE input)
{
	SAMPLE_TYPE output;

	output = buffer[bufidx];
	undenormalise(output);

	filterstore = (output*damp2) + (filterstore*damp1);
	undenormalise(filterstore);

	buffer[bufidx] = input + (filterstore*feedback);

	if(++bufidx>=bufsize) bufidx = 0;

	return output;
}

#endif //_comb_

//ends
