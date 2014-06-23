// samplv1widget_jack.cpp
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

#include "samplv1widget_jack.h"

#include "samplv1_jack.h"

#ifdef CONFIG_NSM
#include "samplv1_nsm.h"
#endif

#include <QFileInfo>
#include <QDir>

#include <QCloseEvent>


//-------------------------------------------------------------------------
// samplv1widget_jack - impl.
//

// Constructor.
samplv1widget_jack::samplv1widget_jack ( samplv1_jack *pSampl )
	: samplv1widget(), m_pSampl(pSampl)
	#ifdef CONFIG_NSM
		, m_pNsmClient(NULL), m_bNsmDirty(false)
	#endif
{
#ifdef CONFIG_NSM
	// Check whether to participate into a NSM session...
	const QString& nsm_url
		= QString::fromLatin1(::getenv("NSM_URL"));
	if (!nsm_url.isEmpty()) {
		m_pNsmClient = new samplv1_nsm(nsm_url);
		QObject::connect(m_pNsmClient,
			SIGNAL(open()),
			SLOT(openSession()));
		QObject::connect(m_pNsmClient,
			SIGNAL(save()),
			SLOT(saveSession()));
		QObject::connect(m_pNsmClient,
			SIGNAL(show()),
			SLOT(showSession()));
		QObject::connect(m_pNsmClient,
			SIGNAL(hide()),
			SLOT(hideSession()));
		m_pNsmClient->announce(SAMPLV1_TITLE, ":switch:dirty:optional-gui:");
		samplv1widget_config *pConfig = samplv1widget_config::getInstance();
		if (pConfig)
			pConfig->bDontUseNativeDialogs = true;
		return;
	}
#endif	// CONFIG_NSM

	// Initialize preset stuff...
	// initPreset();
	updateSample(m_pSampl->sample());
	updateParamValues();
}


// Synth engine accessor.
samplv1 *samplv1widget_jack::instance (void) const
{
	return m_pSampl;
}


#ifdef CONFIG_NSM

void samplv1widget_jack::openSession (void)
{
	if (m_pNsmClient == NULL)
		return;

	if (!m_pNsmClient->is_active())
		return;

#ifdef CONFIG_DEBUG
	qDebug("samplv1widget_jack::openSession()");
#endif

	m_pSampl->deactivate();
	m_pSampl->close();

	const QString& client_id = m_pNsmClient->client_id();
	const QString& path_name = m_pNsmClient->path_name();
	const QString& display_name = m_pNsmClient->display_name();

	m_pSampl->open(client_id.toUtf8().constData());
	m_pSampl->activate();

	const QDir dir(path_name);
	if (!dir.exists())
		dir.mkpath(path_name);

	const QFileInfo fi(path_name, display_name + '.' + SAMPLV1_TITLE);
	if (fi.exists())
		loadPreset(fi.absoluteFilePath());

	m_bNsmDirty = false;

	m_pNsmClient->open_reply();
	m_pNsmClient->dirty(false);

	m_pNsmClient->visible(QWidget::isVisible());
}

void samplv1widget_jack::saveSession (void)
{
	if (m_pNsmClient == NULL)
		return;

	if (!m_pNsmClient->is_active())
		return;

#ifdef CONFIG_DEBUG
	qDebug("samplv1widget_jack::saveSession()");
#endif

	if (m_bNsmDirty) {
		const QString& path_name = m_pNsmClient->path_name();
		const QString& display_name = m_pNsmClient->display_name();
	//	const QString& client_id = m_pNsmClient->client_id();
		const QFileInfo fi(path_name, display_name + '.' + SAMPLV1_TITLE);
		savePreset(fi.absoluteFilePath());
		m_bNsmDirty = false;
	}

	m_pNsmClient->save_reply();
	m_pNsmClient->dirty(false);
}


void samplv1widget_jack::showSession (void)
{
	if (m_pNsmClient == NULL)
		return;

	if (!m_pNsmClient->is_active())
		return;

#ifdef CONFIG_DEBUG
	qDebug("samplv1widget_jack::showSession()");
#endif

	QWidget::show();
	QWidget::raise();
	QWidget::activateWindow();
}

void samplv1widget_jack::hideSession (void)
{
	if (m_pNsmClient == NULL)
		return;

	if (!m_pNsmClient->is_active())
		return;

#ifdef CONFIG_DEBUG
	qDebug("samplv1widget_jack::hideSession()");
#endif

	QWidget::hide();
}


#endif	// CONFIG_NSM


// Param port method.
void samplv1widget_jack::updateParam (
	samplv1::ParamIndex index, float fValue ) const
{
	float *pParamPort = m_pSampl->paramPort(index);
	if (pParamPort)
		*pParamPort = fValue;
}


// Dirty flag method.
void samplv1widget_jack::updateDirtyPreset ( bool bDirtyPreset )
{
	samplv1widget::updateDirtyPreset(bDirtyPreset);

#ifdef CONFIG_NSM
	if (m_pNsmClient && m_pNsmClient->is_active()) {
		if (!m_bNsmDirty/* && bDirtyPreset*/) {
			m_pNsmClient->dirty(true);
			m_bNsmDirty = true;
		}
	}
#endif
}


// Application close.
void samplv1widget_jack::closeEvent ( QCloseEvent *pCloseEvent )
{
#ifdef CONFIG_NSM
	if (m_pNsmClient && m_pNsmClient->is_active())
		samplv1widget::updateDirtyPreset(false);
#endif

	// Let's be sure about that...
	if (queryClose()) {
		pCloseEvent->accept();
	//	QApplication::quit();
	} else {
		pCloseEvent->ignore();
	}
}


#ifdef CONFIG_NSM

// Optional GUI handlers.
void samplv1widget_jack::showEvent ( QShowEvent *pShowEvent )
{
	QWidget::showEvent(pShowEvent);

	if (m_pNsmClient)
		m_pNsmClient->visible(true);
}

void samplv1widget_jack::hideEvent ( QHideEvent *pHideEvent )
{
	if (m_pNsmClient)
		m_pNsmClient->visible(false);

	QWidget::hideEvent(pHideEvent);
}

#endif	// CONFIG_NSM


// end of samplv1widget_jack.cpp
