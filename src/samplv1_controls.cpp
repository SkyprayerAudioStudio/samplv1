// samplv1_controls.cpp
//
/****************************************************************************
   Copyright (C) 2012-2015, rncbc aka Rui Nuno Capela. All rights reserved.

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

#include "samplv1_controls.h"

#include <QHash>


#define RPN_MSB   0x65
#define RPN_LSB   0x64

#define NRPN_MSB  0x63
#define NRPN_LSB  0x62

#define DATA_MSB  0x06
#define DATA_LSB  0x26


#define CC14_MSB_MIN    0x00
#define CC14_MSB_MAX    0x20

#define CC14_LSB_MIN    CC14_MSB_MAX
#define CC14_LSB_MAX   (CC14_MSB_MAX << 1)


//---------------------------------------------------------------------
// xrpn_data14 - decl.
//
class xrpn_data14
{
public:

	xrpn_data14()
		: m_msb(0), m_lsb(0) {}

	xrpn_data14 ( const xrpn_data14& data )
		: m_msb(data.m_msb), m_lsb(data.m_lsb) {}

	void clear() { m_msb = m_lsb = 0; }

	bool is_msb () const
		{ return (m_msb & 0x80); }
	void set_msb ( unsigned char msb )
		{ m_msb = (msb & 0x7f) | 0x80; }
	unsigned msb() const
		{ return (m_msb & 0x7f); }

	bool is_lsb () const
		{ return (m_lsb & 0x80); }
	void set_lsb ( unsigned char lsb )
		{ m_lsb = (lsb & 0x7f) | 0x80; }
	unsigned lsb() const
		{ return (m_lsb & 0x7f); }

	unsigned short data() const
	{
		unsigned short val = 0;

		if (is_lsb()) {
			val += (m_lsb & 0x7f);
			if (is_msb())
				val += (m_msb & 0x7f) << 7;
		}
		else
		if (is_msb())
			val += (m_msb & 0x7f);

		return val;
	}

	bool is_any() const
		{ return is_msb() || is_lsb(); }
	bool is_7bit() const
		{ return is_any() && !is_14bit(); }
	bool is_14bit() const
		{ return is_msb() && is_lsb(); }

private:

	unsigned char m_msb;
	unsigned char m_lsb;
};


//---------------------------------------------------------------------
// xrpn_item - decl.
//
class xrpn_item
{
public:

	xrpn_item() : m_time(0), m_port(0), m_status(0) {}

	xrpn_item ( const xrpn_item& item ) : m_time(item.m_time),
		m_port(item.m_port), m_status(item.m_status),
		m_param(item.m_param), m_value(item.m_value) {}

	void clear()
	{
		m_time = 0;
		m_port = 0;
		m_status = 0;
		m_param.clear();
		m_value.clear();
	}

	void set_time(unsigned long time)
		{ m_time = time; }
	unsigned long time() const
		{ return m_time; }

	void set_port(int port)
		{ m_port = port; }
	int port() const
		{ return m_port; }

	void set_status(unsigned char status)
		{ m_status = status; }
	unsigned char status() const
		{ return m_status; }

	samplv1_controls::Type type() const
		{ return samplv1_controls::Type(m_status & 0xf0); }
	unsigned short channel() const
		{ return (m_status & 0x0f); }

	bool is_param_msb() const
		{ return m_param.is_msb(); }
	bool is_param_lsb() const
		{ return m_param.is_lsb(); }

	void set_param_msb(unsigned char msb)
		{ m_param.set_msb(msb); }
	void set_param_lsb(unsigned char lsb)
		{ m_param.set_lsb(lsb); }

	unsigned char param_msb() const
		{ return m_param.msb(); }
	unsigned char param_lsb() const
		{ return m_param.lsb(); }

	unsigned short param() const
		{ return m_param.data(); }

	bool is_value_msb() const
		{ return m_value.is_msb(); }
	bool is_value_lsb() const
		{ return m_value.is_lsb(); }

	void set_value_msb(unsigned char msb)
		{ m_value.set_msb(msb); }
	void set_value_lsb(unsigned char lsb)
		{ m_value.set_lsb(lsb); }

	unsigned char value_msb() const
		{ return m_value.msb(); }
	unsigned char value_lsb() const
		{ return m_value.lsb(); }

	unsigned short value() const
		{ return m_value.data(); }

	bool is_any() const
		{ return m_param.is_any() || m_value.is_any(); }
	bool is_ready() const
		{ return m_param.is_any() && m_value.is_any(); }
	bool is_7bit() const
		{ return m_param.is_any() && m_value.is_7bit(); }
	bool is_14bit() const
		{ return m_param.is_any() && m_value.is_14bit(); }

private:

	unsigned long m_time;
	int           m_port;
	unsigned char m_status;
	xrpn_data14   m_param;
	xrpn_data14   m_value;
};

typedef QHash<unsigned int, xrpn_item> xrpn_cache;


//---------------------------------------------------------------------
// xrpn_queue - decl.
//
class xrpn_queue
{
public:

	xrpn_queue ( unsigned int size = 0 )
		: m_size(0), m_mask(0), m_read(0), m_write(0), m_events(0)
		{ resize(size); }

	~xrpn_queue () { if (m_events) delete [] m_events; }

	void resize ( unsigned int size )
	{
		unsigned int new_size = 4; // must be a power-of-2...
		while (new_size < size)
			new_size <<= 1;
		if (new_size > m_size) {
			const unsigned int old_size = m_size;
			samplv1_controls::Event *new_events
				= new samplv1_controls::Event [new_size];
			samplv1_controls::Event *old_events = m_events;
			if (old_events) {
				if (m_write > m_read) {
					::memcpy(new_events + m_read, old_events + m_read,
						(m_write - m_read) * sizeof(samplv1_controls::Event));
				}
				else
				if (m_write < m_read) {
					::memcpy(new_events + m_read, old_events + m_read,
						(old_size - m_read) * sizeof(samplv1_controls::Event));
					if (m_write > 0) {
						::memcpy(new_events + old_size, old_events,
							m_write * sizeof(samplv1_controls::Event));
					}
					m_write += old_size;
				}
			}
			m_size = new_size;
			m_mask = new_size - 1;
			m_events = new_events;
			if (old_events)
				delete old_events;
		}
	}

	void clear() { m_read = m_write = 0; }

	bool push (
		unsigned char status,
		unsigned short param,
		unsigned short value )
	{
		samplv1_controls::Event event;

		event.status = status;
		event.param  = param;
		event.value  = value;

		return push(event);
	}

	bool push ( const samplv1_controls::Event& event )
	{
		if (count() >= m_mask)
			resize(m_size + 4);
		const unsigned int w = (m_write + 1) & m_mask;
		if (w == m_read)
			return false;
		m_events[m_write] = event;
		m_write = w;
		return true;
	}

	bool pop ( samplv1_controls::Event& event )
	{
		const unsigned int r = m_read;
		if (r == m_write)
			return false;
		event = m_events[r];
		m_read = (r + 1) & m_mask;
		return true;
	}

	bool is_pending () const
		{ return (m_read != m_write); }

	unsigned int count() const
	{
		if (m_write < m_read)
			return (m_write + m_size - m_read) & m_mask;
		else
			return (m_write - m_read);
	}

private:

	unsigned int m_size;
	unsigned int m_mask;
	unsigned int m_read;
	unsigned int m_write;

	samplv1_controls::Event *m_events;
};


//---------------------------------------------------------------------
// samplv1_controls:Impl - decl.
//
class samplv1_controls::Impl
{
public:

	Impl() : m_count(0) {}

	bool is_pending () const
		{ return m_queue.is_pending(); }

	bool dequeue ( samplv1_controls::Event& event )
		{ return m_queue.pop(event); }

	void flush()
	{
		if (m_count > 0) {
			xrpn_cache::Iterator iter = m_cache.begin();
			const xrpn_cache::Iterator& iter_end = m_cache.end();
			for ( ; iter != iter_end; ++iter)
				enqueue(iter.value());
			m_cache.clear();
		//	m_count = 0;
		}
	}

	bool process ( const samplv1_controls::Event& event )
	{
		const unsigned short channel = (event.status & 0x0f);

		if (event.param == RPN_MSB) {
			xrpn_item& item = get_item(channel);
			if (item.is_param_msb()
				|| (item.is_any() && item.type() != samplv1_controls::RPN))
				enqueue(item);
			if (item.type() == samplv1_controls::None) {
				item.set_status(samplv1_controls::RPN | channel);
				++m_count;
			}
			item.set_param_msb(event.value);
			return true;
		}
		else
		if (event.param == RPN_LSB) {
			xrpn_item& item = get_item(channel);
			if (item.is_param_lsb()
				|| (item.is_any() && item.type() != samplv1_controls::RPN))
				enqueue(item);
			if (item.type() == samplv1_controls::None) {
				item.set_status(samplv1_controls::RPN | channel);
				++m_count;
			}
			item.set_param_lsb(event.value);
			return true;
		}
		else
		if (event.param == NRPN_MSB) {
			xrpn_item& item = get_item(channel);
			if (item.is_param_msb()
				|| (item.is_any() && item.type() != samplv1_controls::NRPN))
				enqueue(item);
			if (item.type() == samplv1_controls::None) {
				item.set_status(samplv1_controls::NRPN | channel);
				++m_count;
			}
			item.set_param_msb(event.value);
			return true;
		}
		else
		if (event.param == NRPN_LSB) {
			xrpn_item& item = get_item(channel);
			if (item.is_param_lsb()
				|| (item.is_any() && item.type() != samplv1_controls::NRPN))
				enqueue(item);
			if (item.type() == samplv1_controls::None) {
				item.set_status(samplv1_controls::NRPN | channel);
				++m_count;
			}
			item.set_param_lsb(event.value);
			return true;
		}
		else
		if (event.param == DATA_MSB) {
			xrpn_item& item = get_item(channel);
			if (item.type() == samplv1_controls::None)
				return false;
			if (item.is_value_msb()
				|| (item.type() != samplv1_controls::RPN
				 && item.type() != samplv1_controls::NRPN)) {
				enqueue(item);
				return false;
			}
			item.set_value_msb(event.value);
			if (item.is_14bit())
				enqueue(item);
			return true;
		}
		else
		if (event.param == DATA_LSB) {
			xrpn_item& item = get_item(channel);
			if (item.type() == samplv1_controls::None)
				return false;
			if (item.is_value_lsb()
				|| (item.type() != samplv1_controls::RPN
				 && item.type() != samplv1_controls::NRPN)) {
				enqueue(item);
				return false;
			}
			item.set_value_lsb(event.value);
			if (item.is_14bit())
				enqueue(item);
			return true;
		}
		else
		if (event.param > CC14_MSB_MIN && event.param < CC14_MSB_MAX) {
			xrpn_item& item = get_item(channel);
			if (item.is_param_msb() || item.is_value_msb()
				|| (item.is_any() && item.type() != samplv1_controls::CC14)
				|| (item.type() == samplv1_controls::CC14
					&& item.param_lsb() != event.param + CC14_LSB_MIN))
				enqueue(item);
			if (item.type() == samplv1_controls::None) {
				item.set_status(samplv1_controls::CC14 | channel);
				++m_count;
			}
			item.set_param_msb(event.param);
			item.set_value_msb(event.value);
			if (item.is_14bit())
				enqueue(item);
			return true;
		}
		else
		if (event.param > CC14_LSB_MIN && event.param < CC14_LSB_MAX) {
			xrpn_item& item = get_item(channel);
			if (item.is_param_lsb() || item.is_value_lsb()
				|| (item.is_any() && item.type() != samplv1_controls::CC14)
				|| (item.type() == samplv1_controls::CC14
					&& item.param_msb() != event.param - CC14_LSB_MIN))
				enqueue(item);
			if (item.type() == samplv1_controls::None) {
				item.set_status(samplv1_controls::CC14 | channel);
				++m_count;
			}
			item.set_param_lsb(event.param);
			item.set_value_lsb(event.value);
			if (item.is_14bit())
				enqueue(item);
			return true;
		}

		return false;
	}

protected:

	xrpn_item& get_item ( unsigned short channel )
		{ return m_cache[channel]; }

	void enqueue ( xrpn_item& item )
	{
		if (item.type() == samplv1_controls::None)
			return;

		if (item.type() == samplv1_controls::CC14) {
			if (item.is_14bit()) {
				m_queue.push(item.status(), item.param_msb(), item.value());
			} else  {
				const unsigned char status = samplv1_controls::CC | item.channel();
				if (item.is_value_msb())
					m_queue.push(status, item.param_msb(), item.value_msb());
				if (item.is_value_lsb())
					m_queue.push(status, item.param_lsb(), item.value_lsb());
			}
		}
		else
		if (item.is_ready()) {
			m_queue.push(item.status(), item.param(), item.value());
		} else {
			const unsigned char status = samplv1_controls::CC | item.channel();
			if (item.type() == samplv1_controls::RPN) {
				if (item.is_param_msb())
					m_queue.push(status, RPN_MSB, item.param_msb());
				if (item.is_param_lsb())
					m_queue.push(status, RPN_LSB, item.param_lsb());
			}
			else
			if (item.type() == samplv1_controls::NRPN) {
				if (item.is_param_msb())
					m_queue.push(status, NRPN_MSB, item.param_msb());
				if (item.is_param_lsb())
					m_queue.push(status, NRPN_LSB, item.param_lsb());
			}
			if (item.is_value_msb())
				m_queue.push(status, DATA_MSB, item.value_msb());
			if (item.is_value_lsb())
				m_queue.push(status, DATA_LSB, item.value_lsb());
		}

		item.clear();
		--m_count;
	}

private:

	unsigned int m_count;

	xrpn_cache m_cache;
	xrpn_queue m_queue;
};


//---------------------------------------------------------------------
// samplv1_controls - impl.
//

samplv1_controls::samplv1_controls ( samplv1 *pSynth )
	: m_pImpl(new samplv1_controls::Impl()), m_pSynth(pSynth)
{
}


samplv1_controls::~samplv1_controls (void)
{
	delete m_pImpl;
}


// controller queue methods.
void samplv1_controls::process_enqueue (
	unsigned short channel, unsigned short param, unsigned short value )
{
	Event event;

	event.status = CC | (channel & 0x0f);
	event.param  = param;
	event.value  = value;

	if (m_pImpl->process(event))
		return;

	process_event(event);
}


void samplv1_controls::process_dequeue (void)
{
	Event event;

	while (m_pImpl->is_pending()) {
		if (m_pImpl->dequeue(event))
			process_event(event);
	}
}


void samplv1_controls::process_event ( const Event& event )
{
	const Key key(event);
	const int index = find_control(key);
	if (index < 0)
		return;

	// TODO: process controller event...
	float fValue = float(event.value) / 127.0f;
	if (Type(key.status & 0xf0) != CC)
		fValue /= 127.0f;

	m_pSynth->setParamValue(samplv1::ParamIndex(index), fValue);
}


void samplv1_controls::flush (void)
{
	m_pImpl->flush();
}


// text utilities.
samplv1_controls::Type samplv1_controls::typeFromText ( const QString& sText )
{
	if (sText == "CC")
		return CC;
	else
	if (sText == "RPN")
		return RPN;
	else
	if (sText == "NRPN")
		return NRPN;
	else
	if (sText == "CC14")
		return CC14;
	else
		return None;
}


QString samplv1_controls::textFromType ( Type ctype )
{
	QString sText;

	switch (ctype) {
	case CC:
		sText = "CC";
		break;
	case RPN:
		sText = "RPN";
		break;
	case NRPN:
		sText = "NRPN";
		break;
	case CC14:
		sText = "CC14";
		break;
	default:
		break;
	}

	return sText;
}


// end of samplv1_controls.cpp