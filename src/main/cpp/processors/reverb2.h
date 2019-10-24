// Reverb model declaration
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

#ifndef _revmodel_
#define _revmodel_

#include "baseprocessor.h"
#include "../audiobuffer.h"
#include "tuning.h"
namespace MWEngine {
//all pass
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

// comb
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

// Reverb2
class Reverb2 : public BaseProcessor {

    public:
					Reverb2();
			void	mute();
			void	setroomsize(float value);
			float	getroomsize();
			void	setdamp(float value);
			float	getdamp();
			void	setwet(float value);
			float	getwet();
			void	setdry(float value);
			float	getdry();
			void	setwidth(float value);
			float	getwidth();
			void	setmode(float value);
			float	getmode();

			void process( AudioBuffer* audioBuffer, bool isMonoSource );
private:
			void	update();
private:
	float	gain;
	float	roomsize,roomsize1;
	float	damp,damp1;
	float	wet,wet1,wet2;
	float	dry;
	float	width;
	float	mode;

	// The following are all declared inline 
	// to remove the need for dynamic allocation
	// with its subsequent error-checking messiness

	// Comb filters
	comb	combL[numcombs];
	comb	combR[numcombs];

	// Allpass filters
	allpass	allpassL[numallpasses];
	allpass	allpassR[numallpasses];

	// Buffers for the combs
	SAMPLE_TYPE	bufcombL1[combtuningL1];
	SAMPLE_TYPE	bufcombR1[combtuningR1];
	SAMPLE_TYPE	bufcombL2[combtuningL2];
	SAMPLE_TYPE	bufcombR2[combtuningR2];
	SAMPLE_TYPE	bufcombL3[combtuningL3];
	SAMPLE_TYPE	bufcombR3[combtuningR3];
	SAMPLE_TYPE	bufcombL4[combtuningL4];
	SAMPLE_TYPE	bufcombR4[combtuningR4];
	SAMPLE_TYPE	bufcombL5[combtuningL5];
	SAMPLE_TYPE	bufcombR5[combtuningR5];
	SAMPLE_TYPE	bufcombL6[combtuningL6];
	SAMPLE_TYPE	bufcombR6[combtuningR6];
	SAMPLE_TYPE	bufcombL7[combtuningL7];
	SAMPLE_TYPE	bufcombR7[combtuningR7];
	SAMPLE_TYPE	bufcombL8[combtuningL8];
	SAMPLE_TYPE	bufcombR8[combtuningR8];

	// Buffers for the allpasses
	SAMPLE_TYPE	bufallpassL1[allpasstuningL1];
	SAMPLE_TYPE	bufallpassR1[allpasstuningR1];
	SAMPLE_TYPE	bufallpassL2[allpasstuningL2];
	SAMPLE_TYPE	bufallpassR2[allpasstuningR2];
	SAMPLE_TYPE	bufallpassL3[allpasstuningL3];
	SAMPLE_TYPE	bufallpassR3[allpasstuningR3];
	SAMPLE_TYPE	bufallpassL4[allpasstuningL4];
	SAMPLE_TYPE	bufallpassR4[allpasstuningR4];
};
}
#endif//_revmodel_

//ends
