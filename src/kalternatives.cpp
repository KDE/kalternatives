/***************************************************************************
 *   Copyright (C) 2004 by Juanjo                                          *
 *   juanjux@yahoo.es                                                      *
 *                                                                         *
 *   Copyright (C) 2004 by Mario Bensi                                     *
 *   nef@ipsquad.net                                                       *
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
#include "altcontroller.h"
#include "altitemelement.h"
#include "treeitemelement.h"
#include "addalternatives.h"
#include "ui_propertieswindow.h"

#include <qtimer.h>

#include <kmessagebox.h>
#include <kaboutdata.h>
#include <ktextedit.h>
#include <kdebug.h>
#include <klocale.h>
#include <qlayout.h>
#include <qfile.h>
#include <qtextstream.h>
#include <kgenericfactory.h>
#include <kstandardguiitem.h>
#include <k3listview.h>

#include <unistd.h>
#include <sys/types.h>

K_PLUGIN_FACTORY(KalternativesFactory, registerPlugin<Kalternatives>();)
K_EXPORT_PLUGIN(KalternativesFactory("kcm_kalternatives"))

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
	#ifdef DEBIAN
	m_mgr = new AltFilesManager("/var/lib/dpkg/alternatives");
	#else
	KMessageBox::sorry(this, i18n("Kalternatives only work on Debian- or Mandrake-based systems"), i18n("Wrong System Type"));
	QTimer::singleShot(0, this, SLOT(die()));
	#endif
#endif
	
	m_ui.setupUi(this);
	
	m_ui.m_altList->setShowToolTips(1);
	
	connect(m_ui.m_hideAlt, SIGNAL(clicked()), this,
			SLOT(slotHideAlternativesClicked()));
	
	connect(m_ui.m_altList, SIGNAL(selectionChanged( Q3ListViewItem* )), this,
			SLOT(slotSelectAlternativesClicked(Q3ListViewItem *)));
	
	connect(m_ui.m_optionsList, SIGNAL(clicked(Q3ListViewItem *)), 
			this, SLOT(slotOptionClicked(Q3ListViewItem *)));
	connect(m_ui.m_bAdd, SIGNAL(clicked()), this,
			SLOT(slotAddClicked()));
	connect(m_ui.m_bDelete, SIGNAL(clicked()), this,
			SLOT(slotDeleteClicked()));
	connect(m_ui.m_bProperties, SIGNAL(clicked()), this,
			SLOT(slotPropertiesClicked()));
	
	m_altList = m_ui.m_altList;
	m_optionsList = m_ui.m_optionsList;
	m_altTilte = m_ui.m_altTilte;
	m_statusCombo = m_ui.m_statusCombo;
	m_hideAlt = m_ui.m_hideAlt;
	
	m_ui.m_bDelete->setGuiItem(KStandardGuiItem::del());
	m_ui.m_bAdd->setGuiItem(KGuiItem(i18n("&Add"), "list-add"));
	m_ui.m_bProperties->setGuiItem(KGuiItem( i18n( "&Properties" ), "configure"));
	
	if(!m_bisRoot)
	{
		m_ui.m_bDelete->setEnabled(false);
		m_ui.m_bAdd->setEnabled(false);
		m_ui.m_bProperties->setEnabled(false);
		m_statusCombo->setEnabled(false);
	}
	
	KAboutData *myAboutData = new KAboutData("kcmkalternatives", 0, ki18n("Kalternatives"),
	KALT_VERSION, ki18n("KDE Mandrake/Debian alternatives-system manager"),
	KAboutData::License_GPL, ki18n("(c) 2004 Juanjo Alvarez Martinez\n"
	                                   "(c) 2004 Mario Bensi"));

	myAboutData->addAuthor(ki18n("Juanjo Alvarez Martinez"), KLocalizedString(), "juanjo@juanjoalvarez.net",
		"http://juanjoalvarez.net");
	myAboutData->addAuthor(ki18n("Mario Bensi"), KLocalizedString(), "nef@ipsquad.net", "http://ipsquad.net");
	myAboutData->addAuthor(ki18n("Pino Toscano"), KLocalizedString(), "toscano.pino@tiscali.it");
	
	setAboutData( myAboutData );
	
	load();
	m_hideAlt->setChecked(true);
	slotHideAlternativesClicked();
}

Kalternatives::~Kalternatives()
{
	if(m_mgr) delete m_mgr;
	if(m_altList) delete m_altList;
	if(m_optionsList) delete m_optionsList;
	if(m_altTilte) delete m_altTilte;
	if(m_statusCombo) delete m_statusCombo;
	if(m_hideAlt) delete m_hideAlt;
}

void Kalternatives::die()
{
	delete this;
}

void Kalternatives::load()
{
	clearList(m_altList);
	Q3PtrList<Item> *itemslist = m_mgr->getGlobalAlternativeList();
	Item *i;
	TreeItemElement *treeit;
	for(i = itemslist->first(); i; i = itemslist->next())
	{
		AltController *altcontroller = new AltController();
		treeit = new TreeItemElement(m_altList, i, altcontroller);
		
		AltsPtrList *altList = i->getAlternatives();
		Alternative *a = altList->first();
		
		AltItemElement *ael;
		
		for(; a; a=altList->next())
		{
			ael = new AltItemElement(treeit, a);
			if(!m_bisRoot) ael->setEnabled(false);
			treeit->getAltController()->addAltItem(ael);
		}
    }
}




void Kalternatives::clearList(K3ListView* list)
{
	Q3ListViewItemIterator it( list );
	Q3ListViewItem *tmp;
	while( (tmp=it.current()) )
	{
		it++;
		list->takeItem(tmp);
	}
}


void Kalternatives::slotSelectAlternativesClicked(Q3ListViewItem *alternative)
{
	clearList(m_optionsList);
	TreeItemElement *treeItem;
	if((treeItem = dynamic_cast<TreeItemElement *>(alternative)))
	{
		Item *item = treeItem->getItem();
		
		m_altTilte->setText(treeItem->getName());
		m_statusCombo->setCurrentItem(item->getMode());
		
		AltItemList *altItemList = treeItem->getAltController()->getAltItemList();
		
		for( AltItemElement *altItem= altItemList->first(); altItem ; altItem = altItemList->next())
		{
			Alternative *a = altItem->getAlternative();
			
			QString priority;
			priority.setNum(a->getPriority());
			
			m_optionsList->insertItem(altItem);
			
			altItem->setText( 1, priority);
			altItem->setText( 2, a->getPath());
			QString m_small_desc = altItem->getDescription();
			
			if (!m_small_desc.isEmpty())
			{
				altItem->setText( 3, m_small_desc);
			}
			else
			{
				altItem->searchDescription();
			}
		}
	}
	m_optionsList->setSelected(m_optionsList->firstChild(), 1);
}



void Kalternatives::slotHideAlternativesClicked()
{
	if (m_hideAlt->isChecked ())
	{
		Q3ListViewItemIterator it( m_altList );
		TreeItemElement *i;
		Q3ListViewItem *tmp;
		while ( it.current() )
		{
			if((i = dynamic_cast<TreeItemElement *>(it.current())))
			{
				if ((i->getItem()->getAlternatives()->count()) <= 1)
				{
					tmp = it.current();
					it++;
					m_altList->takeItem(tmp);
					continue;
				}
			}
			it++;
		}
	}
	else
	{
		load();
	}
	m_altList->setSelected(m_altList->firstChild (), 1);
}


void Kalternatives::slotOptionClicked(Q3ListViewItem *option)
{
	AltItemElement *altItem;
	if((altItem = dynamic_cast<AltItemElement *>(option)))
	{
		if( !altItem->isOn())
			return;
		TreeItemElement *treeItem;
		if((treeItem = dynamic_cast<TreeItemElement *>(m_altList->selectedItem())))
		{
			treeItem->getAltController()->setBoutonOnOff(m_optionsList, altItem);
			treeItem->setChanged(TRUE);
			emit changed(true);
		}
	}
}

void Kalternatives::slotAddClicked()
{
	TreeItemElement *treeItem;
	if((treeItem = dynamic_cast<TreeItemElement *>(m_altList->selectedItem())))
	{
		m_optionsList->setSelected(m_optionsList->firstChild(), 1);
		
		AltItemElement *altItem;
		if((altItem = dynamic_cast<AltItemElement *>(m_optionsList->selectedItem())))
		{
			int countSlave = altItem->getAlternative()->countSlaves();
			AddAlternatives *addAlternatives = new AddAlternatives(treeItem, this, countSlave);
			addAlternatives->show();
		}
	}
}
void Kalternatives::slotDeleteClicked()
{
	TreeItemElement *treeItem;
	if((treeItem = dynamic_cast<TreeItemElement *>(m_altList->selectedItem())))
	{
		AltItemElement *altItem;
		if((altItem = dynamic_cast<AltItemElement *>(m_optionsList->selectedItem())))
		{
			treeItem->getItem()->delAlternativeByPath(altItem->getPath());
			
			AltItemList *altItemList = treeItem->getAltController()->getAltItemList();
			
			altItemList->remove(altItem);
			m_optionsList->takeItem(altItem);
			
			treeItem->setNbrAltChanged(TRUE);
			emit changed( TRUE );
			m_optionsList->setSelected(m_optionsList->firstChild (), 1);
		}
	}
}

void Kalternatives::slotPropertiesClicked()
{
	AltItemElement *altItem;
	
	if((altItem = dynamic_cast<AltItemElement *>(m_optionsList->selectedItem())))
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
		
		Alternative *a = altItem->getAlternative();
		

		text += i18n("Description:\n%1\n", altItem->getDescription());
		text += i18n("Path: %1\n", a->getPath());
		text += i18n("Priority: %1\n", a->getPriority());
		
		QStringList* slavesList = a->getSlaves();
		text += i18np( "Slave :", "Slaves :", slavesList->count() );
		
		for ( QStringList::Iterator it = slavesList->begin(); it != slavesList->end(); ++it ) 
		{
			text += "\n\t";
			text += *it;
		}
		propUi.m_text->setText(text);
		prop->show();
	}
}


void Kalternatives::save()
{
	Q3ListViewItemIterator it( m_altList );
	TreeItemElement *treeItem;
	
	while ( it.current() )
	{
		if((treeItem = dynamic_cast<TreeItemElement *>(it.current())))
		{
			if(treeItem->isNbrAltChanged())
			{
				QString parentPath =  QString("%1/%2")
										.arg(m_mgr->getAltDir())
										.arg(treeItem->getName());
				
				QFile origFile(parentPath);
				if(origFile.exists())
				{
					origFile.remove();
				}
				
				if( origFile.open( QIODevice::WriteOnly ))
				{
					QTextStream stream( &origFile );
				
				
					Item *item = treeItem->getItem();
				
					stream << item->getMode() << endl;
					stream << item->getPath() << endl;
					
					SlaveList *slaveList = item->getSlaves();
					Slave *slave = slaveList->first();
					for(; slave; slave = slaveList->next())
					{
						stream << slave->slname << endl;
						stream << slave->slpath << endl;
					}
					
					stream << endl;
					
					AltItemList *altItemList = treeItem->getAltController()->getAltItemList();
					AltItemElement *altItem= altItemList->first();
					for( ; altItem ; altItem = altItemList->next())
					{
						Alternative *a = altItem->getAlternative();
						
						stream << a->getPath() << endl;
						stream << a->getPriority() << endl;
					
						QStringList *slaveList = a->getSlaves();
						QStringList::Iterator it = slaveList->begin();
						for ( ; it != slaveList->end(); ++it )
						{
							stream << *it << endl;
						}
					}
					origFile.close();
				}
				treeItem->setNbrAltChanged(FALSE);
			}
			if(treeItem->isChanged())
			{
				AltItemList *altItemList = treeItem->getAltController()->getAltItemList();
				AltItemElement *altItem= altItemList->first();
				for( ; altItem ; altItem = altItemList->next())
				{
					if( altItem->isOn() )
					{
						Alternative *a = altItem->getAlternative();
						if(!a->select())
						{
							kDebug() << a->getSelectError() << endl;
						}
					}
				}
				treeItem->setChanged(FALSE);
			}
		}
		it++;
	}
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
