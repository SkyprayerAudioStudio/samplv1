// samplv1.h
//
/****************************************************************************
   Copyright (C) 2012, rncbc aka Rui Nuno Capela. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*****************************************************************************/

#ifndef __samplv1_h
#define __samplv1_h

#include "config.h"

#include <stdint.h>

// forward declarations
class samplv1_impl;
class samplv1_sample;


//-------------------------------------------------------------------------
// samplv1 - decl.
//

class samplv1
{
public:

	samplv1(uint16_t iChannels = 2, uint32_t iSampleRate = 44100);

	~samplv1();

	void setChannels(uint16_t iChannels);
	uint16_t channels() const;

	void setSampleRate(uint32_t iSampleRate);
	uint32_t sampleRate() const;

	void setSampleFile(const char *pszSampleFile);
	const char *sampleFile() const;

	samplv1_sample *sample() const;

	enum ParamIndex	 {

		GEN1_SAMPLE = 0,
		GEN1_LOOP,
		GEN1_OCTAVE,
		GEN1_TUNING,
		GEN1_GLIDE,
		DCF1_CUTOFF,
		DCF1_RESO,
		DCF1_TYPE,
		DCF1_SLOPE,
		DCF1_ENVELOPE,
		DCF1_ATTACK,
		DCF1_DECAY,
		DCF1_SUSTAIN,
		DCF1_RELEASE,
		LFO1_SHAPE,
		LFO1_WIDTH,
		LFO1_RATE,
		LFO1_SWEEP,
		LFO1_PITCH,
		LFO1_CUTOFF,
		LFO1_RESO,
		LFO1_PANNING,
		LFO1_VOLUME,
		LFO1_ATTACK,
		LFO1_DECAY,
		LFO1_SUSTAIN,
		LFO1_RELEASE,
		DCA1_VOLUME,
		DCA1_ATTACK,
		DCA1_DECAY,
		DCA1_SUSTAIN,
		DCA1_RELEASE,
		OUT1_WIDTH,
		OUT1_PANNING,
		OUT1_VOLUME,

		DEF1_PITCHBEND,
		DEF1_MODWHEEL,
		DEF1_PRESSURE,
		DEF1_VELOCITY,

		CHO1_WET,
		CHO1_DELAY,
		CHO1_FEEDB,
		CHO1_RATE,
		CHO1_MOD,
		FLA1_WET,
		FLA1_DELAY,
		FLA1_FEEDB,
		FLA1_DAFT,
		PHA1_WET,
		PHA1_RATE,
		PHA1_FEEDB,
		PHA1_DEPTH,
		PHA1_DAFT,
		DEL1_WET,
		DEL1_DELAY,
		DEL1_FEEDB,
		DEL1_BPM,
		DYN1_COMPRESS,
		DYN1_LIMITER,

		NUM_PARAMS
	};

	void setParamPort(ParamIndex index, float *pfParam);
	float *paramPort(ParamIndex index) const;

	void reset();

protected:

	void process_midi(uint8_t *data, uint32_t size);
	void process(float **ins, float **outs, uint32_t nframes);

private:

	samplv1_impl *m_pImpl;
};


#endif// __samplv1_h

// end of samplv1.h