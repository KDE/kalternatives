/***************************************************************************
 *   Copyright (C) 2004 by Juanjo Álvarez Martinez <juanjo@juanjoalvarez.net> *
 *   Copyright (C) 2004 by Mario Bensi <nef@ipsquad.net>                   *
 *   Copyright (C) 2008 by Pino Toscano <pino@kde.org>                     *
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

#include <qheaderview.h>
#include <qtimer.h>

#include <kmessagebox.h>
#include <kaboutdata.h>
#include <kdebug.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <kstandardguiitem.h>

#include <unistd.h>
#include <sys/types.h>

#include <config-kalternatives.h>

K_PLUGIN_FACTORY(KalternativesFactory, registerPlugin<Kalternatives>();)
K_EXPORT_PLUGIN(KalternativesFactory(aboutData("kalternatives", I18N_NOOP("Kalternatives"))))

Kalternatives::Kalternatives(QWidget *parent, const QVariantList& args)
:KCModule(KalternativesFactory::componentData(), parent, args)
{
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

#ifdef MANDRAKE
m_mgr = new AltFilesManager("/var/lib/rpm/alternatives");
#else
	#ifdef DISTRO_DEBIAN
	m_mgr = new AltFilesManager("/var/lib/dpkg/alternatives");
	#else
	KMessageBox::sorry(this, i18n("Kalternatives only work on Debian- or Mandrake-based systems"), i18n("Wrong System Type"));
	QTimer::singleShot(0, this, SLOT(die()));
	#endif
#endif
	
	m_ui.setupUi(this);
	
	connect(m_ui.m_altList, SIGNAL(activated(QModelIndex)),
			this, SLOT(slotSelectAlternativesActivated(QModelIndex)));
	
	connect(m_ui.m_bAdd, SIGNAL(clicked()), this,
			SLOT(slotAddClicked()));
	connect(m_ui.m_bDelete, SIGNAL(clicked()), this,
			SLOT(slotDeleteClicked()));
	connect(m_ui.m_bProperties, SIGNAL(clicked()), this,
			SLOT(slotPropertiesClicked()));
	
	m_ui.m_bDelete->setGuiItem(KStandardGuiItem::del());
	m_ui.m_bAdd->setGuiItem(KGuiItem(i18n("&Add"), "list-add"));
	m_ui.m_bProperties->setGuiItem(KGuiItem( i18n( "&Properties" ), "configure"));
	
	if(!m_bisRoot)
	{
		m_ui.m_bDelete->setEnabled(false);
		m_ui.m_bAdd->setEnabled(false);
		m_ui.m_bProperties->setEnabled(false);
		m_ui.m_statusCombo->setEnabled(false);
	}
	
	setAboutData(new KAboutData(*KalternativesFactory::componentData().aboutData()));
	
	m_ui.m_hideAlt->setChecked(true);
}

Kalternatives::~Kalternatives()
{
	if(m_mgr) delete m_mgr;
}

void Kalternatives::die()
{
	delete this;
}

void Kalternatives::load()
{
	m_itemProxyModel = new AlternativeItemProxyModel(m_ui.m_altList);
	slotHideAlternativesClicked();
	AlternativeItemsModel *itemModel = new AlternativeItemsModel(m_mgr, m_itemProxyModel);
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
}

void Kalternatives::slotSelectAlternativesActivated(const QModelIndex &index)
{
	Item *item = index.data(AltItemRole).value<Item *>();
	m_altModel->setItem(item);
	m_ui.m_altTilte->setText(item->getName());
	m_ui.m_statusCombo->setCurrentItem(item->getMode());
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
		// TODO add confirm dialog
		m_altModel->removeAlternative(alt);
	}
}

void Kalternatives::slotPropertiesClicked()
{
	Alternative *a = m_altModel->data(m_ui.m_optionsList->currentIndex(), AltAlternativeRole).value<Alternative *>();
	if (a)
	{
		QString text;
		KDialog *prop = new KDialog(this);
		prop->setCaption(i18n("Alternative Properties"));
		prop->setButtons(KDialog::Close);
		prop->showButtonSeparator(true);
		Ui::PropertiesWindow propUi;
		propUi.setupUi(prop->mainWidget());
		prop->mainWidget()->layout()->setMargin(0);
		connect(prop, SIGNAL(closeClicked()), prop, SLOT(deleteLater()));
		
		text += i18n("Description:\n%1\n", Alternative::prettyDescription(a));
		text += i18n("Path: %1\n", a->getPath());
		text += i18n("Priority: %1\n", a->getPriority());
		
		QStringList* slavesList = a->getSlaves();
		text += i18n("Slaves:");
		
		for ( QStringList::Iterator it = slavesList->begin(); it != slavesList->end(); ++it ) 
		{
			text += "\n\t";
			text += *it;
		}
		propUi.m_text->setText(text);
		prop->show();
	}
}

void Kalternatives::slotUpdateStatusCombo()
{
	Item *item = m_ui.m_altList->currentIndex().data(AltItemRole).value<Item *>();
	if (item)
	{
		m_ui.m_statusCombo->setCurrentItem(item->getMode());
		emit changed(true);
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
	return i18n("<h1>Kalternatives</h1>\n"
	            "A Mandrake/Debian alternatives-system manager.");
}



#include "kalternatives.moc"
