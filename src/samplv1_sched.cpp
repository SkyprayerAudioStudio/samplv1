// samplv1_sched.cpp
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

#include "samplv1_sched.h"

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

#include <stdint.h>


//-------------------------------------------------------------------------
// samplv1_sched_thread - worker/schedule thread decl.
//

class samplv1_sched_thread : public QThread
{
public:

	// ctor.
	samplv1_sched_thread(uint32_t nsize = 8);

	// dtor.
	~samplv1_sched_thread();

	// schedule processing and wake from wait condition.
	void schedule(samplv1_sched *sched);

protected:

	// main thread executive.
	void run();

private:

	// sync queue instance reference.
	uint32_t m_nsize;
	uint32_t m_nmask;

	samplv1_sched **m_items;

	volatile uint32_t m_iread;
	volatile uint32_t m_iwrite;

	// whether the thread is logically running.
	volatile bool m_running;

	// thread synchronization objects.
	QMutex m_mutex;
	QWaitCondition m_cond;
};


static samplv1_sched_thread *g_sched_thread = NULL;
static uint32_t g_sched_refcount = 0;

static QList<samplv1_sched_notifier *> g_sched_notifiers;


//-------------------------------------------------------------------------
// samplv1_sched_thread - worker/schedule thread impl.
//

// ctor.
samplv1_sched_thread::samplv1_sched_thread ( uint32_t nsize ) : QThread()
{
	m_nsize = (4 << 1);
	while (m_nsize < nsize)
		m_nsize <<= 1;
	m_nmask = (m_nsize - 1);
	m_items = new samplv1_sched * [m_nsize];

	m_iread  = 0;
	m_iwrite = 0;

	::memset(m_items, 0, m_nsize * sizeof(samplv1_sched *));

	m_running = false;
}


// dtor.
samplv1_sched_thread::~samplv1_sched_thread (void)
{
	// fake sync and wait
	if (m_running && isRunning()) do {
		if (m_mutex.tryLock()) {
			m_running = false;
			m_cond.wakeAll();
			m_mutex.unlock();
		}
	} while (!wait(100));

	delete [] m_items;
}


// schedule processing and wake from wait condition.
void samplv1_sched_thread::schedule ( samplv1_sched *sched )
{
	if (!sched->sync_wait()) {
		const uint32_t w = (m_iwrite + 1) & m_nmask;
		if (w != m_iread) {
			m_items[m_iwrite] = sched;
			m_iwrite = w;
		}
	}

	if (m_mutex.tryLock()) {
		m_cond.wakeAll();
		m_mutex.unlock();
	}
}


// main thread executive.
void samplv1_sched_thread::run (void)
{
	m_mutex.lock();

	m_running = true;

	while (m_running) {
		// do whatever we must...
		uint32_t r = m_iread;
		while (r != m_iwrite) {
			samplv1_sched *sched = m_items[r];
			if (sched) {
				sched->sync_process();
				m_items[r] = NULL;
			}
			++r &= m_nmask;
		}
		m_iread = r;
		// wait for sync...
		m_cond.wait(&m_mutex);
	}

	m_mutex.unlock();
}


//-------------------------------------------------------------------------
// samplv1_sched - worker/scheduled stuff (pure virtual).
//

// ctor.
samplv1_sched::samplv1_sched ( Type stype)
	: m_stype(stype), m_sync_wait(false)
{
	if (++g_sched_refcount == 1 && g_sched_thread == NULL) {
		g_sched_thread = new samplv1_sched_thread();
		g_sched_thread->start();
	}
}


// dtor (virtual).
samplv1_sched::~samplv1_sched (void)
{
	if (--g_sched_refcount == 0) {
		if (g_sched_thread) {
			delete g_sched_thread;
			g_sched_thread = NULL;
		}
	}
}


// schedule process.
void samplv1_sched::schedule (void)
{
	if (g_sched_thread)
		g_sched_thread->schedule(this);
}


// test-and-set.
bool samplv1_sched::sync_wait (void)
{
	const bool sync_wait = m_sync_wait;

	if (!sync_wait)
		m_sync_wait = true;

	return sync_wait;
}


// scheduled processor.
void samplv1_sched::sync_process (void)
{
	process();

	m_sync_wait = false;

	sync_notify(m_stype);
}


// signal broadcast (static).
void samplv1_sched::sync_notify ( Type stype )
{
	QListIterator<samplv1_sched_notifier *> iter(g_sched_notifiers);
	while (iter.hasNext())
		iter.next()->sync_notify(stype);
}


//-------------------------------------------------------------------------
// samplv1_sched_notifier - worker/schedule proxy decl.
//

// ctor.
samplv1_sched_notifier::samplv1_sched_notifier ( QObject *parent )
	: QObject(parent)
{
	g_sched_notifiers.append(this);
}


// dtor.
samplv1_sched_notifier::~samplv1_sched_notifier (void)
{
	g_sched_notifiers.removeAll(this);
}


void samplv1_sched_notifier::sync_notify ( samplv1_sched::Type stype )
{
	emit notify(int(stype));
}


// end of samplv1_sched.cpp
