// samplv1_jack.h
//
/****************************************************************************
   Copyright (C) 2012-2014, rncbc aka Rui Nuno Capela. All rights reserved.

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

#ifndef __samplv1_jack_h
#define __samplv1_jack_h

#include "samplv1.h"

#include <jack/jack.h>


#ifdef CONFIG_ALSA_MIDI
#include <jack/ringbuffer.h>
#include <alsa/asoundlib.h>
// forward decls.
class samplv1_alsa_thread;
#endif


//-------------------------------------------------------------------------
// samplv1_jack - decl.
//

class samplv1_jack : public samplv1
{
public:

	samplv1_jack();

	~samplv1_jack();

	jack_client_t *client() const;

	void open(const char *client_id);
	void close();

	void activate();
	void deactivate();

	int process(jack_nframes_t nframes);

	void setParamValue(samplv1::ParamIndex index, float fValue);
	float paramValue(samplv1::ParamIndex index) const;

#ifdef CONFIG_ALSA_MIDI
	snd_seq_t *alsa_seq() const;
	void alsa_capture(snd_seq_event_t *ev);
#endif

private:

	jack_client_t *m_client;

	volatile bool m_activated;

	jack_port_t **m_audio_ins;
	jack_port_t **m_audio_outs;

	float **m_ins;
	float **m_outs;

	float m_params[samplv1::NUM_PARAMS];

#ifdef CONFIG_JACK_MIDI
	jack_port_t *m_midi_in;
#endif
#ifdef CONFIG_ALSA_MIDI
	snd_seq_t *m_alsa_seq;
//	int m_alsa_client;
	int m_alsa_port;
	snd_midi_event_t *m_alsa_decoder;
	jack_ringbuffer_t *m_alsa_buffer;
	samplv1_alsa_thread *m_alsa_thread;
#endif

	float m_bpm;
};


#endif// __samplv1_jack_h

// end of samplv1_jack.h
