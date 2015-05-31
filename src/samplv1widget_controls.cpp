// samplv1widget_controls.cpp
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

#include "samplv1widget_controls.h"

#include "samplv1_controls.h"
#include "samplv1_config.h"

#include <QHeaderView>

#include <QSpinBox>
#include <QLineEdit>
#include <QComboBox>


//----------------------------------------------------------------------------
// samplv1widget_controls_item_delegate -- Custom (tree) list item delegate.

// ctor.
samplv1widget_controls_item_delegate::samplv1widget_controls_item_delegate (
	QObject *pParent ) : QItemDelegate(pParent)
{
}


// QItemDelegate interface...
QSize samplv1widget_controls_item_delegate::sizeHint (
	const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
	return QItemDelegate::sizeHint(option, index) + QSize(4, 4);
}


QWidget *samplv1widget_controls_item_delegate::createEditor ( QWidget *pParent,
	const QStyleOptionViewItem& /*option*/, const QModelIndex& index ) const
{
	QWidget *pEditor = NULL;

	switch (index.column()) {
	case 0: // Channel.
	{
		QSpinBox *pSpinBox = new QSpinBox(pParent);
		pSpinBox->setMinimum(1);
		pSpinBox->setMaximum(16);
		pEditor = pSpinBox;
		break;
	}

	case 1: // Type.
	{
		QComboBox *pComboBox = new QComboBox(pParent);
		pComboBox->setEditable(false);
		pComboBox->addItem(
			samplv1_controls::textFromType(samplv1_controls::CC));
		pComboBox->addItem(
			samplv1_controls::textFromType(samplv1_controls::RPN));
		pComboBox->addItem(
			samplv1_controls::textFromType(samplv1_controls::NRPN));
		pComboBox->addItem(
			samplv1_controls::textFromType(samplv1_controls::CC14));
		pEditor = pComboBox;
		break;
	}

	case 2: // Param.
	{
		QComboBox *pComboBox = new QComboBox(pParent);
		pComboBox->setEditable(true);
		pEditor = pComboBox;
		break;
	}

	case 3: // Index.
	{
		QComboBox *pComboBox = new QComboBox(pParent);
		pComboBox->setEditable(false);
		for (uint32_t i = 0; i < samplv1::NUM_PARAMS; ++i)
			pComboBox->addItem(samplv1_param::paramName(samplv1::ParamIndex(i)));
		pEditor = pComboBox;
		break;
	}

	default:
		break;
	}

#ifdef CONFIG_DEBUG_0
	qDebug("samplv1widget_controls_item_delegate::createEditor(%p, %d, %d) = %p",
		pParent, index.row(), index.column(), pEditor);
#endif

	return pEditor;
}


void samplv1widget_controls_item_delegate::setEditorData (
	QWidget *pEditor, const QModelIndex& index ) const
{
#ifdef CONFIG_DEBUG_0
	qDebug("samplv1widget_controls_item_delegate::setEditorData(%p, %d, %d)",
		pEditor, index.row(), index.column());
#endif

	switch (index.column()) {
	case 0: // Channel.
	{
		const int iChannel = index.data().toInt();
		//	= index.model()->data(index, Qt::DisplayRole).toInt();
		QSpinBox *pSpinBox = qobject_cast<QSpinBox *> (pEditor);
		if (pSpinBox) pSpinBox->setValue(iChannel);
		break;
	}

	case 1: // Type.
	{
		const QString& sText = index.data().toString();
		//	= index.model()->data(index, Qt::DisplayRole).toString();
		QComboBox *pComboBox = qobject_cast<QComboBox *> (pEditor);
		if (pComboBox) pComboBox->setEditText(sText);
		break;
	}

	case 2: // Param.
	{
		const QString& sText = index.data().toString();
		//	= index.model()->data(index, Qt::DisplayRole).toString();
		QComboBox *pComboBox = qobject_cast<QComboBox *> (pEditor);
		if (pComboBox) pComboBox->setEditText(sText);
		break;
	}

	case 3: // Index.
	{
		const int iIndex = index.data().toInt();
		//	= index.model()->data(index, Qt::DisplayRole).toInt();
		QComboBox *pComboBox = qobject_cast<QComboBox *> (pEditor);
		if (pComboBox) pComboBox->setCurrentIndex(iIndex);
		break;
	}

	default:
		break;
	}
}


void samplv1widget_controls_item_delegate::setModelData ( QWidget *pEditor,
	QAbstractItemModel *pModel,	const QModelIndex& index ) const
{
#ifdef CONFIG_DEBUG_0
	qDebug("samplv1widget_controls_item_delegate::setModelData(%p, %d, %d)",
		pEditor, index.row(), index.column());
#endif

	switch (index.column()) {
	case 0: // Channel.
	{
		QSpinBox *pSpinBox = qobject_cast<QSpinBox *> (pEditor);
		if (pSpinBox) {
			const int iChannel = pSpinBox->value();
			pModel->setData(index, QString::number(iChannel));
		}
		break;
	}

	case 1: // Type.
	{
		QComboBox *pComboBox = qobject_cast<QComboBox *> (pEditor);
		if (pComboBox) {
			const QString& sType = pComboBox->currentText();
			pModel->setData(index, sType);
		}
		break;
	}

	case 2: // Param.
	{
		QComboBox *pComboBox = qobject_cast<QComboBox *> (pEditor);
		if (pComboBox) {
			const QString& sParam = pComboBox->currentText();
			pModel->setData(index, sParam);
		}
		break;
	}

	case 3: // Index.
	{
		QComboBox *pComboBox = qobject_cast<QComboBox *> (pEditor);
		if (pComboBox) {
			const int iIndex = pComboBox->currentIndex();
			pModel->setData(index, QString::number(iIndex));
		}
		break;
	}

	default:
		break;
	}

	// Done.
}


//----------------------------------------------------------------------------
// samplv1widget_controls -- UI wrapper form.

// ctor.
samplv1widget_controls::samplv1widget_controls ( QWidget *pParent )
	: QTreeWidget(pParent)
{
	QTreeWidget::setColumnCount(4);

	QTreeWidget::setAlternatingRowColors(true);
	QTreeWidget::setUniformRowHeights(true);
	QTreeWidget::setAllColumnsShowFocus(true);

	QTreeWidget::setSelectionBehavior(QAbstractItemView::SelectRows);
	QTreeWidget::setSelectionMode(QAbstractItemView::SingleSelection);

	QHeaderView *pHeaderView = QTreeWidget::header();
#if QT_VERSION < 0x050000
	pHeaderView->setResizeMode(QHeaderView::ResizeToContents);
#else
	pHeaderView->setSectionResizeMode(QHeaderView::ResizeToContents);
#endif
	pHeaderView->hide();

	QTreeWidget::setItemDelegate(new samplv1widget_controls_item_delegate(this));
}


// dtor.
samplv1widget_controls::~samplv1widget_controls (void)
{
}


// utilities.
void samplv1widget_controls::loadControls ( samplv1_controls *pControls )
{
	QTreeWidget::clear();

	QList<QTreeWidgetItem *> items;
	const samplv1_controls::Map& map = pControls->map();
	samplv1_controls::Map::ConstIterator iter = map.constBegin();
	const samplv1_controls::Map::ConstIterator& iter_end = map.constEnd();
	for ( ; iter != iter_end; ++iter) {
		const samplv1_controls::Key& key = iter.key();
		const samplv1::ParamIndex index = samplv1::ParamIndex(iter.value());
		QTreeWidgetItem *pItem = new QTreeWidgetItem(this);
		pItem->setText(0, QString::number(
			(key.status & 0x0f) + 1));
		pItem->setText(1, samplv1_controls::textFromType(
			samplv1_controls::Type(key.status & 0xf0)));
		pItem->setText(2, QString::number(key.param));
		pItem->setText(3, samplv1_param::paramName(index));
		pItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
		items.append(pItem);
	}
	QTreeWidget::addTopLevelItems(items);
	QTreeWidget::expandAll();
}


void samplv1widget_controls::saveControls ( samplv1_controls *pControls )
{
	pControls->clear();

	const int iItemCount = QTreeWidget::topLevelItemCount();
	for (int iItem = 0 ; iItem < iItemCount; ++iItem) {
		QTreeWidgetItem *pItem = QTreeWidget::topLevelItem(iItem);
		const unsigned short channel
			= pItem->text(0).toInt() - 1;
		const unsigned char ctype
			= samplv1_controls::typeFromText(pItem->text(1));
		samplv1_controls::Key key;
		key.status = ctype | (channel & 0x0f);
		key.param = pItem->text(2).toInt();
		pControls->add_control(key, pItem->text(3).toInt());
	}
}


// slots.
void samplv1widget_controls::addControlItem (void)
{
	QTreeWidget::setFocus();

	QTreeWidgetItem *pItem = newControlItem();
	if (pItem) {
		QTreeWidget::setCurrentItem(pItem);
		QTreeWidget::editItem(pItem, 0);
	}
}


// factory methods.
QTreeWidgetItem *samplv1widget_controls::newControlItem (void)
{
	QTreeWidgetItem *pItem = new QTreeWidgetItem();
	pItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
	QTreeWidget::addTopLevelItem(pItem);

	return pItem;
}


// end of samplv1widget_controls.cpp
