// samplv1_programs.h
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

#ifndef __samplv1_programs_h
#define __samplv1_programs_h

#include <QMap>
#include <QString>

#include <stdint.h>


//-------------------------------------------------------------------------
// samplv1_programs - Bank/programs database class.
//

class samplv1_programs
{
public:

	// ctor.
	samplv1_programs();

	// dtor.
	~samplv1_programs();

	// prog. base node
	class Prog
	{
	public:

		Prog(uint16_t id, const QString& name)
			: m_id(id), m_name(name) {}

		uint16_t id() const	{ return m_id; }
		const QString& name() const	{ return m_name; }
		void set_name(const QString& name) { m_name = name; }

	private:

		uint16_t m_id;
		QString  m_name;
	};

	typedef QMap<uint16_t, Prog *> Progs;

	// bank node
	class Bank : public Prog
	{
	public:

		Bank(uint16_t id, const QString& name)
			: Prog(id, name) {}

		~Bank() { clear_progs(); }

		const Progs& progs() const { return m_progs; }

		// prog. managers
		Prog *find_prog(uint16_t prog_id) const;
		Prog *add_prog(uint16_t prog_id, const QString& prog_name);
		void remove_prog(uint16_t prog_id);
		void clear_progs();

	private:

		Progs m_progs;
	};

	typedef QMap<uint16_t, Bank *> Banks;

	const Banks& banks() const { return m_banks; }

	// bank managers
	Bank *find_bank(uint16_t bank_id) const;
	Bank *add_bank(uint16_t bank_id, const QString& bank_name);
	void remove_bank(uint16_t bank_id);

	void clear_banks();

	// current bank/prog. managers
	void set_current_bank_msb(uint8_t bank_msb);
	void set_current_bank_lsb(uint8_t bank_lsb);

	void set_current_bank(uint16_t bank_id);
	void set_current_prog(uint16_t prog_id);

	Bank *current_bank() const { return m_bank; }
	Prog *current_prog() const { return m_prog; }

protected:

	uint16_t current_bank_id() const;

private:

	Banks m_banks;

	uint8_t m_bank_msb;
	uint8_t m_bank_lsb;

	Bank *m_bank;
	Prog *m_prog;
};


#endif	// __samplv1_programs_h

// end of samplv1_programs.h
