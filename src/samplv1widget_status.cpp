// samplv1widget_status.cpp
//
/****************************************************************************
   Copyright (C) 2012-2017, rncbc aka Rui Nuno Capela. All rights reserved.

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

#include "samplv1widget_status.h"

#include <QLabel>
#include <QIcon>
#include <QPixmap>
#include <QHBoxLayout>


//-------------------------------------------------------------------------
// samplv1widget_status - Custom status-bar widget.
//

// Constructor.
samplv1widget_status::samplv1widget_status ( QWidget *pParent )
	: QStatusBar (pParent)
{
	QIcon icon;

	icon.addPixmap(
		QPixmap(":/images/ledOff.png"), QIcon::Normal, QIcon::Off);
	icon.addPixmap(
		QPixmap(":/images/ledOn.png"), QIcon::Normal, QIcon::On);

	m_midiInLed[0] = new QPixmap(
		icon.pixmap(16, 16, QIcon::Normal, QIcon::Off));
	m_midiInLed[1] = new QPixmap(
		icon.pixmap(16, 16, QIcon::Normal, QIcon::On));

	const QString sMidiIn(tr("MIDI In"));

	QWidget *pMidiInWidget = new QWidget();
	pMidiInWidget->setToolTip(tr("%1 status").arg(sMidiIn));

	QHBoxLayout *pMidiInLayout = new QHBoxLayout();
	pMidiInLayout->setMargin(0);
	pMidiInLayout->setSpacing(0);

	m_pMidiInLedLabel = new QLabel();
	m_pMidiInLedLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	m_pMidiInLedLabel->setPixmap(*m_midiInLed[0]);
	m_pMidiInLedLabel->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
	m_pMidiInLedLabel->setAutoFillBackground(true);
	pMidiInLayout->addWidget(m_pMidiInLedLabel);

	QLabel *pMidiInTextLabel = new QLabel(sMidiIn);
	pMidiInTextLabel->setMargin(2);
	pMidiInTextLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	pMidiInTextLabel->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
	pMidiInTextLabel->setAutoFillBackground(true);
	pMidiInLayout->addWidget(pMidiInTextLabel);

	pMidiInWidget->setLayout(pMidiInLayout);
	QStatusBar::addWidget(pMidiInWidget);

	const QFontMetrics fm(QStatusBar::font());
	m_pModifiedLabel = new QLabel();
	m_pModifiedLabel->setAlignment(Qt::AlignHCenter);
	m_pModifiedLabel->setMinimumSize(QSize(fm.width("MOD") + 4, fm.height()));
	m_pModifiedLabel->setToolTip(tr("Modification status"));
	m_pModifiedLabel->setAutoFillBackground(true);
	QStatusBar::addPermanentWidget(m_pModifiedLabel);
}


// Destructor.
samplv1widget_status::~samplv1widget_status (void)
{
	delete m_midiInLed[1];
	delete m_midiInLed[0];
}


// Permanent widgets accessors.
void samplv1widget_status::midiInLed ( bool bMidiInLed )
{
	m_pMidiInLedLabel->setPixmap(*m_midiInLed[bMidiInLed ? 1 : 0]);
}


void samplv1widget_status::modified ( bool bModified )
{
	if (bModified)
		m_pModifiedLabel->setText(tr("MOD"));
	else
		m_pModifiedLabel->clear();
}


// end of samplv1widget_status.cpp
