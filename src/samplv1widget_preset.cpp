// samplv1widget_preset.cpp
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

#include "samplv1widget_preset.h"

#include "samplv1widget_config.h"

#include <QHBoxLayout>

#include <QToolButton>
#include <QComboBox>

#include <QMessageBox>
#include <QFileDialog>
#include <QUrl>


//-------------------------------------------------------------------------
// samplv1widget_preset - Custom edit-box widget.
//

// Constructor.
samplv1widget_preset::samplv1widget_preset ( QWidget *pParent )
	: QWidget (pParent)
{
	m_pNewButton    = new QToolButton();
	m_pOpenButton   = new QToolButton();
	m_pComboBox     = new QComboBox();
	m_pSaveButton   = new QToolButton();
	m_pDeleteButton = new QToolButton();
	m_pResetButton  = new QToolButton();

	m_pNewButton->setIcon(QIcon(":/images/presetNew.png"));
	m_pOpenButton->setIcon(QIcon(":/images/presetOpen.png"));
	m_pComboBox->setEditable(true);
	m_pComboBox->setMinimumWidth(240);
#if QT_VERSION >= 0x040200
	m_pComboBox->setCompleter(NULL);
#endif
	m_pComboBox->setInsertPolicy(QComboBox::NoInsert);
	m_pSaveButton->setIcon(QIcon(":/images/presetSave.png"));
	m_pDeleteButton->setIcon(QIcon(":/images/presetDelete.png"));
	m_pResetButton->setText("&Reset");

	m_pNewButton->setToolTip(tr("New Preset"));
	m_pOpenButton->setToolTip(tr("Open Preset"));
	m_pSaveButton->setToolTip(tr("Save Preset"));
	m_pDeleteButton->setToolTip(tr("Delete Preset"));
	m_pResetButton->setToolTip(tr("Reset Preset"));

	QHBoxLayout *pHBoxLayout = new QHBoxLayout();
	pHBoxLayout->setMargin(2);
	pHBoxLayout->setSpacing(4);
	pHBoxLayout->addWidget(m_pNewButton);
	pHBoxLayout->addWidget(m_pOpenButton);
	pHBoxLayout->addWidget(m_pComboBox);
	pHBoxLayout->addWidget(m_pSaveButton);
	pHBoxLayout->addWidget(m_pDeleteButton);
	pHBoxLayout->addSpacing(4);
	pHBoxLayout->addWidget(m_pResetButton);
	QWidget::setLayout(pHBoxLayout);

	m_iInitPreset  = 0;
	m_iDirtyPreset = 0;

	// UI signal/slot connections...
	QObject::connect(m_pNewButton,
		SIGNAL(clicked()),
		SLOT(newPreset()));
	QObject::connect(m_pOpenButton,
		SIGNAL(clicked()),
		SLOT(openPreset()));
	QObject::connect(m_pComboBox,
		SIGNAL(editTextChanged(const QString&)),
		SLOT(changePreset(const QString&)));
	QObject::connect(m_pSaveButton,
		SIGNAL(clicked()),
		SLOT(savePreset()));
	QObject::connect(m_pDeleteButton,
		SIGNAL(clicked()),
		SLOT(deletePreset()));
	QObject::connect(m_pResetButton,
		SIGNAL(clicked()),
		SLOT(resetPreset()));

	refreshPreset();
	stabilizePreset();
}


// Preset group path name.
QString samplv1widget_preset::presetGroup (void) const
{
	return "/Presets/";
}


// Preset name/text accessors.
void samplv1widget_preset::clearPreset (void)
{
	m_iInitPreset = 0;

	samplv1widget_config *pConfig = samplv1widget_config::getInstance();
	if (pConfig)
		pConfig->sPreset.clear();

	bool bBlockSignals = m_pComboBox->blockSignals(true);
	m_pComboBox->clearEditText();
	m_pComboBox->blockSignals(bBlockSignals);
}


void samplv1widget_preset::setPreset ( const QString& sPreset )
{
	bool bBlockSignals = m_pComboBox->blockSignals(true);
	m_pComboBox->setEditText(sPreset);
	m_pComboBox->blockSignals(bBlockSignals);
}

QString samplv1widget_preset::preset (void) const
{
	return m_pComboBox->currentText();
}


// Check whether current preset may be reset.
bool samplv1widget_preset::queryPreset (void)
{
	if (m_iInitPreset == 0)
		return true;

	samplv1widget_config *pConfig = samplv1widget_config::getInstance();
	if (pConfig == NULL)
		return false;

	const QString& sPreset = pConfig->sPreset;
	if (!sPreset.isEmpty() && m_iDirtyPreset > 0) {
		switch (QMessageBox::warning(this,
			tr("Warning") + " - " SAMPLV1_TITLE,
			tr("Some preset parameters have been changed:\n\n"
			"\"%1\".\n\nDo you want to save the changes?")
			.arg(sPreset),
			QMessageBox::Save |
			QMessageBox::Discard |
			QMessageBox::Cancel)) {
		case QMessageBox::Save:
			savePreset(sPreset);
			// Fall thru...
		case QMessageBox::Discard:
			break;
		default: // Cancel...
			setPreset(sPreset);
			return false;
		}
	}

	return true;
}


// Preset management slots...
void samplv1widget_preset::changePreset ( const QString& sPreset )
{
	bool bLoadPreset = (!sPreset.isEmpty()
		&& m_pComboBox->findText(sPreset) >= 0);
	if (bLoadPreset && !queryPreset())
		return;

	if (bLoadPreset)
		loadPreset(sPreset);
	else
		stabilizePreset();
}


void samplv1widget_preset::loadPreset ( const QString& sPreset )
{
	if (sPreset.isEmpty())
		return;

	samplv1widget_config *pConfig = samplv1widget_config::getInstance();
	if (pConfig) {
		pConfig->beginGroup(presetGroup());
		emit loadPresetFile(pConfig->value(sPreset).toString());
		pConfig->endGroup();
		++m_iInitPreset;
		pConfig->sPreset = sPreset;
	//	setPreset(sPreset);
		refreshPreset();
	}

	stabilizePreset();
}


void samplv1widget_preset::newPreset (void)
{
	if (queryPreset()) {
		emit newPresetFile();
		clearPreset();
		refreshPreset();
	}

	stabilizePreset();
}


void samplv1widget_preset::openPreset (void)
{
	samplv1widget_config *pConfig = samplv1widget_config::getInstance();
	if (pConfig == NULL)
		return;

	QString sFilename;

	const QString  sExt(SAMPLV1_TITLE);
	const QString& sTitle  = tr("Open Preset") + " - " SAMPLV1_TITLE;
	const QString& sFilter = tr("Preset files (*.%1)").arg(sExt);
#if 1//QT_VERSION < 0x040400
	sFilename = QFileDialog::getOpenFileName(parentWidget(),
		sTitle, pConfig->sPresetDir, sFilter);
#else
	QFileDialog fileDialog(nativeParentWidget(),
		sTitle, pConfig->sPresetDir, sFilter);
	fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
	fileDialog.setFileMode(QFileDialog::ExistingFile);
	fileDialog.setDefaultSuffix(sExt);
	QList<QUrl> urls(fileDialog.sidebarUrls());
	urls.append(QUrl::fromLocalFile(pConfig->sPresetDir));
	fileDialog.setSidebarUrls(urls);
	if (fileDialog.exec())
		sFilename = fileDialog.selectedFiles().first();
#endif
	if (!sFilename.isEmpty()) {
		QFileInfo fi(sFilename);
		if (fi.exists() && queryPreset()) {
			const QString& sPreset = fi.completeBaseName();
			pConfig->beginGroup(presetGroup());
			pConfig->setValue(sPreset, sFilename);
			pConfig->endGroup();
			emit loadPresetFile(sFilename);
			++m_iInitPreset;
			pConfig->sPreset = sPreset;
			pConfig->sPresetDir = fi.absolutePath();
			setPreset(sPreset);
			refreshPreset();
		}
	}

	stabilizePreset();
}


void samplv1widget_preset::savePreset (void)
{
	savePreset(m_pComboBox->currentText());
}

void samplv1widget_preset::savePreset ( const QString& sPreset )
{
	if (sPreset.isEmpty())
		return;

	samplv1widget_config *pConfig = samplv1widget_config::getInstance();
	if (pConfig == NULL)
		return;

	const QString sExt(SAMPLV1_TITLE);
	QFileInfo fi(QDir(pConfig->sPresetDir), sPreset + '.' + sExt);
	QString sFilename = fi.absoluteFilePath();
	if (!fi.exists()) {
		const QString& sTitle  = tr("Save Preset") + " - " SAMPLV1_TITLE;
		const QString& sFilter = tr("Preset files (*.%1)").arg(sExt);
	#if 1//QT_VERSION < 0x040400
		sFilename = QFileDialog::getSaveFileName(parentWidget(),
			sTitle, sFilename, sFilter);
	#else
		QFileDialog fileDialog(nativeParentWidget(),
			sTitle, sFilename, sFilter);
		fileDialog.setAcceptMode(QFileDialog::AcceptSave);
		fileDialog.setFileMode(QFileDialog::AnyFile);
		fileDialog.setDefaultSuffix(sExt);
		QList<QUrl> urls(fileDialog.sidebarUrls());
		urls.append(QUrl::fromLocalFile(pConfig->sPresetDir));
		fileDialog.setSidebarUrls(urls);
		if (fileDialog.exec())
			sFilename = fileDialog.selectedFiles().first();
	#endif
	} else {
		if (QMessageBox::warning(parentWidget(),
			tr("Warning") + " - " SAMPLV1_TITLE,
			tr("About to replace preset:\n\n"
			"\"%1\"\n\n"
			"Are you sure?")
			.arg(sPreset),
			QMessageBox::Ok | QMessageBox::Cancel)
			== QMessageBox::Cancel) {
			sFilename.clear();
		}
	}

	if (!sFilename.isEmpty()) {
		if (QFileInfo(sFilename).suffix() != sExt)
			sFilename += '.' + sExt;
		emit savePresetFile(sFilename);
		pConfig->beginGroup(presetGroup());
		pConfig->setValue(sPreset, sFilename);
		pConfig->endGroup();
		++m_iInitPreset;
		pConfig->sPreset = sPreset;
		pConfig->sPresetDir = QFileInfo(sFilename).absolutePath();
		refreshPreset();
	}

	stabilizePreset();
}


void samplv1widget_preset::deletePreset (void)
{
	const QString& sPreset = m_pComboBox->currentText();
	if (sPreset.isEmpty())
		return;

	samplv1widget_config *pConfig = samplv1widget_config::getInstance();
	if (pConfig == NULL)
		return;

	if (QMessageBox::warning(parentWidget(),
		tr("Warning") + " - " SAMPLV1_TITLE,
		tr("About to remove preset:\n\n"
		"\"%1\"\n\n"
		"Are you sure?")
		.arg(sPreset),
		QMessageBox::Ok | QMessageBox::Cancel)
		== QMessageBox::Cancel)
		return;

	pConfig->beginGroup(presetGroup());
	const QString& sFilename = pConfig->value(sPreset).toString();
	if (QFileInfo(sFilename).exists())
		QFile(sFilename).remove();
	pConfig->remove(sPreset);
	pConfig->endGroup();

	clearPreset();
	refreshPreset();
	stabilizePreset();
}


void samplv1widget_preset::resetPreset (void)
{
	const QString& sPreset = m_pComboBox->currentText();

	bool bLoadPreset = (!sPreset.isEmpty()
		&& m_pComboBox->findText(sPreset) >= 0);
	if (bLoadPreset && !queryPreset())
		return;

	if (bLoadPreset) {
		loadPreset(sPreset);
	} else {
		emit resetPresetFile();
		m_iDirtyPreset = 0;
		stabilizePreset();
	}
}


// Widget refreshner-loader.
void samplv1widget_preset::refreshPreset (void)
{
	bool bBlockSignals = m_pComboBox->blockSignals(true);

	const QString sOldPreset = m_pComboBox->currentText();
	m_pComboBox->clear();
	samplv1widget_config *pConfig = samplv1widget_config::getInstance();
	if (pConfig) {
		pConfig->beginGroup(presetGroup());
		m_pComboBox->insertItems(0, pConfig->childKeys());
		m_pComboBox->model()->sort(0);
		pConfig->endGroup();
	}

	int iIndex = m_pComboBox->findText(sOldPreset);
	if (iIndex >= 0)
		m_pComboBox->setCurrentIndex(iIndex);
	else
		m_pComboBox->setEditText(sOldPreset);

	m_iDirtyPreset = 0;

	m_pComboBox->blockSignals(bBlockSignals);
}


// Preset control.
void samplv1widget_preset::initPreset (void)
{
	samplv1widget_config *pConfig = samplv1widget_config::getInstance();
	if (pConfig && !pConfig->sPreset.isEmpty())
		loadPreset(pConfig->sPreset);
	else
		newPreset();
}


void samplv1widget_preset::dirtyPreset (void)
{
	++m_iDirtyPreset;

	stabilizePreset();
}


void samplv1widget_preset::stabilizePreset (void)
{
	const QString& sPreset = m_pComboBox->currentText();

	bool bEnabled = (!sPreset.isEmpty());
	bool bExists  = (m_pComboBox->findText(sPreset) >= 0);
	bool bDirty   = (m_iDirtyPreset > 0);

	m_pSaveButton->setEnabled(bEnabled && (!bExists || bDirty));
	m_pDeleteButton->setEnabled(bEnabled && bExists);
	m_pResetButton->setEnabled(bDirty);
}


// end of samplv1widget_preset.cpp
