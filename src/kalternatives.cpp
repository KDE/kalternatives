/***************************************************************************
 *   Copyright (C) 2004 by Juanjo Álvarez Martinez <juanjo@juanjoalvarez.net> *
 *   Copyright (C) 2004 by Mario Bensi <nef@ipsquad.net>                   *
 *   Copyright (C) 2004 by Kevin Ottens <ervin@kde.org>                    *
 *   Copyright (C) 2004, 2008-2009 by Pino Toscano <pino@kde.org>          *
 *   Copyright (C) 2008 by Armin Berres <armin@space-based.de>             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02111-1307, USA.          *
 ***************************************************************************/

#include "kalternatives.h"
#include "altparser.h"
#include "addalternatives.h"
#include "alternativemodels.h"
#include "ui_propertieswindow.h"
#include "aboutdata.h"
#include "slavemodel.h"

#include <qdialog.h>
#include <qdialogbuttonbox.h>
#include <qheaderview.h>
#include <qitemselectionmodel.h>
#include <qtimer.h>

#include <kmessagebox.h>
#include <kaboutdata.h>
#include <klazylocalizedstring.h>
#include <kpluginfactory.h>
#include <kstandardguiitem.h>

#include <unistd.h>
#include <sys/types.h>

K_PLUGIN_FACTORY(KalternativesFactory, registerPlugin<Kalternatives>();)

static inline QString componentName()
{
    return QStringLiteral("kalternatives");
}

Kalternatives::Kalternatives(QWidget *parent, const QVariantList& args)
    : KCModule(parent, args)
{
	auto *aboutData = new KAboutData(::aboutData(componentName(), kli18n("Kalternatives").untranslatedText()));
	setAboutData(aboutData);

	setUseRootOnlyMessage(false);

	int user = getuid();
	
	if (user == 0)
	{
		m_bisRoot = true;
		setButtons(KCModule::Help|KCModule::Apply);
	}
	else 
	{
		m_bisRoot = false;
		setButtons(Help);
	}
	
	m_ui.setupUi(this);
	m_ui.m_mainSplitter->setStretchFactor(1, 5);
	
	connect(m_ui.m_altList, SIGNAL(clicked(QModelIndex)),
			this, SLOT(slotSelectAlternativesActivated(QModelIndex)));
	
	connect(m_ui.m_bAdd, SIGNAL(clicked()), this,
			SLOT(slotAddClicked()));
	connect(m_ui.m_bDelete, SIGNAL(clicked()), this,
			SLOT(slotDeleteClicked()));
	connect(m_ui.m_bProperties, SIGNAL(clicked()), this,
			SLOT(slotPropertiesClicked()));
	
	KStandardGuiItem::assign(m_ui.m_bDelete, KStandardGuiItem::Delete);
	m_ui.m_bDelete->setWhatsThis(i18n("Removes the selected alternative from the current group."));
	KGuiItem::assign(m_ui.m_bAdd, KGuiItem(i18n("&Add"), "list-add",
	                                       i18n("Adds a new alternative for the selected group.")));
	KGuiItem::assign(m_ui.m_bProperties, KGuiItem(i18n("&Properties"), "configure",
	                                              i18n("Shows the properties (path, priority, and slaves) of the selected alternative.")));
	
	m_ui.m_statusCombo->addItem(i18nc("Automatic alternative choice", "Automatic"), Item::AutoMode);
	m_ui.m_statusCombo->addItem(i18nc("Manual alternative choice", "Manual"), Item::ManualMode);
	
	m_ui.m_bDelete->setEnabled(false);
	m_ui.m_bAdd->setEnabled(false);
	m_ui.m_bProperties->setEnabled(false);
	if(!m_bisRoot)
	{
		m_ui.m_statusCombo->setEnabled(false);
	}
}

Kalternatives::~Kalternatives()
{
}

void Kalternatives::load()
{
	m_itemProxyModel = new AlternativeItemProxyModel(m_ui.m_altList);
	slotHideAlternativesClicked();
	AlternativeItemsModel *itemModel = new AlternativeItemsModel(componentName(), m_itemProxyModel);
	m_itemProxyModel->setSourceModel(itemModel);
	m_ui.m_altList->setModel(m_itemProxyModel);
	connect(itemModel, SIGNAL(dataChanged(QModelIndex, QModelIndex)),
	        this, SLOT(slotUpdateStatusCombo()));
	
	QHeaderView *h = m_ui.m_altList->header();
	h->resizeSections(QHeaderView::Stretch);
	h->resizeSection(0, h->sectionSizeHint(0));
	connect(m_ui.m_hideAlt, SIGNAL(clicked()), this,
			SLOT(slotHideAlternativesClicked()));
	
	QSortFilterProxyModel *altListSorter = new QSortFilterProxyModel(m_ui.m_optionsList);
	altListSorter->setDynamicSortFilter(true);
	m_altModel = new AlternativeAltModel(itemModel, !m_bisRoot, altListSorter);
	altListSorter->setSourceModel(m_altModel);
	m_ui.m_optionsList->setModel(altListSorter);
	m_ui.m_optionsList->header()->setSortIndicator(0, Qt::AscendingOrder);
	connect(m_altModel, SIGNAL(dataChanged(QModelIndex, QModelIndex)),
	        this, SLOT(configChanged()));
	connect(m_altModel, SIGNAL(rowsInserted(QModelIndex, int, int)),
	        this, SLOT(configChanged()));
	connect(m_ui.m_statusCombo, SIGNAL(activated(int)),
	        m_altModel, SLOT(statusChanged(int)));

	connect(m_ui.m_altList->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
	        this, SLOT(slotUpdateButtons()));
	connect(m_ui.m_optionsList->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
	        this, SLOT(slotUpdateButtons()));
}

void Kalternatives::slotSelectAlternativesActivated(const QModelIndex &index)
{
	Item *item = index.data(AltItemRole).value<Item *>();
	m_altModel->setItem(item);
	m_ui.m_altTilte->setText(item->getName());
	const int statusIndex = m_ui.m_statusCombo->findData(item->getMode());
	Q_ASSERT(statusIndex != -1);
	m_ui.m_statusCombo->setCurrentIndex(statusIndex);
	slotUpdateButtons();
}

void Kalternatives::slotHideAlternativesClicked()
{
	m_itemProxyModel->setShowSingleAlternative(!m_ui.m_hideAlt->isChecked());
}

void Kalternatives::slotAddClicked()
{
	Item *item = m_ui.m_altList->currentIndex().data(AltItemRole).value<Item *>();
	if (item)
	{
		AddAlternatives addAlternatives(item, this);
		addAlternatives.exec();
		if (Alternative *a = addAlternatives.alternative())
		{
			m_altModel->addAlternative(a);
		}
	}
}

void Kalternatives::slotDeleteClicked()
{
	Alternative *alt = m_ui.m_optionsList->currentIndex().data(AltAlternativeRole).value<Alternative *>();
	if (alt)
	{
		const QString messageText = i18n("Are you really sure you want to delete the alternative '%1'?", alt->getPath());
		if (KMessageBox::warningYesNo(this, messageText, i18n("Delete Alternative")) == KMessageBox::Yes)
		{
			m_altModel->removeAlternative(alt);
		}
	}
}

void Kalternatives::slotPropertiesClicked()
{
	Alternative *a = m_ui.m_optionsList->currentIndex().data(AltAlternativeRole).value<Alternative *>();
	if (a)
	{
		QString text;
		QDialog *prop = new QDialog(this);
		prop->setWindowTitle(i18n("Alternative Properties"));
		QVBoxLayout *lay = new QVBoxLayout(prop);
		QWidget *main = new QWidget(prop);
		Ui::PropertiesWindow propUi;
		propUi.setupUi(main);
		main->layout()->setContentsMargins(0, 0, 0, 0);
		lay->addWidget(main);
		QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Close, prop);
		lay->addWidget(buttons);
		connect(buttons, SIGNAL(rejected()), prop, SLOT(deleteLater()));
		
		propUi.labelPath->setText(a->getPath());
		propUi.labelDescription->setText(Alternative::prettyDescription(a));
		propUi.labelPriority->setText(QString::number(a->getPriority()));
		
		if (a->slavesCount() > 0)
		{
			SlaveModel *sm = new SlaveModel(propUi.slaveView);
			sm->setItem(a->getParent());
			sm->setAlternative(a);
			propUi.slaveView->setModel(sm);
		}
		else
		{
			propUi.slavesGroup->hide();
		}
		
		prop->show();
	}
}

void Kalternatives::slotUpdateStatusCombo()
{
	Item *item = m_ui.m_altList->currentIndex().data(AltItemRole).value<Item *>();
	if (item)
	{
		const int statusIndex = m_ui.m_statusCombo->findData(item->getMode());
		Q_ASSERT(statusIndex != -1);
		m_ui.m_statusCombo->setCurrentIndex(statusIndex);
		emit changed(true);
	}
}

void Kalternatives::slotUpdateButtons()
{
	if (m_bisRoot)
	{
		const bool altSelected = m_ui.m_altList->selectionModel()->hasSelection();
		m_ui.m_bAdd->setEnabled(altSelected);
	}
	const bool altChoiceSelected = m_ui.m_optionsList->selectionModel()->currentIndex().isValid();
	m_ui.m_bProperties->setEnabled(altChoiceSelected);
	if (m_bisRoot)
	{
		m_ui.m_bDelete->setEnabled(altChoiceSelected);
	}
}

void Kalternatives::save()
{
	AlternativeItemsModel *model = qobject_cast<AlternativeItemsModel *>(m_itemProxyModel->sourceModel());
	model->save();
	emit changed( false );
}

void Kalternatives::configChanged()
{
	emit changed(true);
}

QString Kalternatives::quickHelp() const
{
	return i18n("<h1>Alternatives Configuration</h1>\n"
	            "This module allows you to configure the system alternatives in "
	            "Debian/Fedora/Mandriva/openSUSE/Ubuntu distributions.");

}

#include <kalternatives.moc>
