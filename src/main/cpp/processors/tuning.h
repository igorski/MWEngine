// Reverb model tuning values
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

#ifndef _tuning_
#define _tuning_
#include "baseprocessor.h"

const int	numcombs		= 8;
const int	numallpasses	= 4;
const SAMPLE_TYPE	muted			= 0;
const SAMPLE_TYPE	fixedgain		= 0.015f;
const SAMPLE_TYPE scalewet		= 3;
const SAMPLE_TYPE scaledry		= 2;
const SAMPLE_TYPE scaledamp		= 0.4f;
const SAMPLE_TYPE scaleroom		= 0.28f;
const SAMPLE_TYPE offsetroom		= 0.7f;
const SAMPLE_TYPE initialroom		= 0.5f;
const SAMPLE_TYPE initialdamp		= 0.5f;
const SAMPLE_TYPE initialwet		= 1/scalewet;
const SAMPLE_TYPE initialdry		= 0.5;
const SAMPLE_TYPE initialwidth	= 1;
const SAMPLE_TYPE initialmode		= 0;
const SAMPLE_TYPE freezemode		= 0.5f;
const int	stereospread	= 23;

// These values assume 44.1KHz sample rate
// they will probably be OK for 48KHz sample rate
// but would need scaling for 96KHz (or other) sample rates.
// The values were obtained by listening tests.
const int combtuningL1		= 1116;
const int combtuningR1		= 1116+stereospread;
const int combtuningL2		= 1188;
const int combtuningR2		= 1188+stereospread;
const int combtuningL3		= 1277;
const int combtuningR3		= 1277+stereospread;
const int combtuningL4		= 1356;
const int combtuningR4		= 1356+stereospread;
const int combtuningL5		= 1422;
const int combtuningR5		= 1422+stereospread;
const int combtuningL6		= 1491;
const int combtuningR6		= 1491+stereospread;
const int combtuningL7		= 1557;
const int combtuningR7		= 1557+stereospread;
const int combtuningL8		= 1617;
const int combtuningR8		= 1617+stereospread;
const int allpasstuningL1	= 556;
const int allpasstuningR1	= 556+stereospread;
const int allpasstuningL2	= 441;
const int allpasstuningR2	= 441+stereospread;
const int allpasstuningL3	= 341;
const int allpasstuningR3	= 341+stereospread;
const int allpasstuningL4	= 225;
const int allpasstuningR4	= 225+stereospread;

#endif//_tuning_

//ends

