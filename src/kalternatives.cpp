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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "kalternatives.h"
#include "altparser.h"
#include "propertieswindow.h"
#include "altcontroller.h"
#include "altitemelement.h"
#include "treeitemelement.h"
#include "addalternatives.h"
#include "mainwindow.h"
#include "altparser.h"

#include <qtimer.h>

#include <kmessagebox.h>
#include <kaboutdialog.h>
#include <ktextedit.h>
#include <kdebug.h>
#include <klocale.h>
#include <qlayout.h>
#include <qfile.h> 
#include <qtextstream.h> 
#include <kgenericfactory.h>


typedef KGenericFactory<Kalternatives, QWidget> KalternativesFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_Kalternatives, KalternativesFactory("kcmkalternatives"))

extern "C"
{
	KCModule *create_kalternatives(QWidget *parent, const char *name)
	{
		return new Kalternatives(parent, name); 
	};
}

Kalternatives::Kalternatives(QWidget *parent, const char *name, const QStringList&)
:KCModule(/*KalternativesFactory::instance(), */parent, name), myAboutData(0)
{
	int user = getuid();
	//FIXME: This won't be needed as kcm
	
	if (user == 0)
	{
		m_bisRoot = true;
		setButtons(KCModule::Help|KCModule::Apply);
		setEnabled( true );
	}
	else 
	{
		m_bisRoot = false;
		setButtons(Help);
		setEnabled( false );
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
	
	MainWindow *mainwindow = new MainWindow(this);
	QBoxLayout *layout = new QVBoxLayout(this, 0, KDialog::spacingHint());
	layout->addWidget( mainwindow );
	
	mainwindow->m_altList->setShowToolTips(1);
	
	connect(mainwindow->m_hideAlt, SIGNAL(clicked()), this,
			SLOT(slotHideAlternativesClicked()));
	
	connect(mainwindow->m_altList, SIGNAL(selectionChanged( QListViewItem* )), this,
			SLOT(slotSelectAlternativesClicked(QListViewItem *)));
	
	connect(mainwindow->m_optionsList, SIGNAL(clicked(QListViewItem *)), 
			this, SLOT(slotOptionClicked(QListViewItem *)));
	connect(mainwindow->m_bAdd, SIGNAL(clicked()), this,
			SLOT(slotAddClicked()));
	connect(mainwindow->m_bDelete, SIGNAL(clicked()), this,
			SLOT(slotDeleteClicked()));
	connect(mainwindow->m_bProperties, SIGNAL(clicked()), this,
			SLOT(slotPropertiesClicked()));
	
	m_altList = mainwindow->m_altList;
	m_optionsList = mainwindow->m_optionsList;
	m_altTilte = mainwindow->m_altTilte;
	m_statusCombo = mainwindow->m_statusCombo;
	m_hideAlt = mainwindow->m_hideAlt;
	
	myAboutData = new KAboutData("KalternativesKCM", "Kalternatives", KALT_VERSION, i18n("KDE Mandrake/Debian alternatives-system manager"),
        KAboutData::License_GPL, "(c) 2004 Juanjo Alvarez Martinez and Mario Bensi", 0, 0 );
		
		
	myAboutData->addAuthor("Juanjo Alvarez Martinez", "\n\nKalternatives -- Mandrake/Debian alternatives-system manager", "juanjo@juanjoalvarez.net",
		"http://juanjoalvarez.net");
	myAboutData->addAuthor("Mario Bensi", "\n\nKalternatives -- Mandrake/Debian alternatives-system manager", "nef@ipsquad.net", "http://ipsquad.net");
	
	setAboutData( myAboutData );
	
	load();
	m_altList->setSelected( m_altList->firstChild(), TRUE );
	resize(615, 490);
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
	QPtrList<Item> *itemslist = m_mgr->getGlobalAlternativeList();
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
			ael = new AltItemElement(m_optionsList, a);
			treeit->getAltController()->addAltItem(ael);
		}
    }
}




void Kalternatives::clearList(KListView* list)
{
	QListViewItemIterator it( list );
	QListViewItem *tmp;
	while( (tmp=it.current()) )
	{
		it++;
		list->takeItem(tmp);
	}
}


void Kalternatives::slotSelectAlternativesClicked(QListViewItem *alternative)
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
			
			if (!(m_small_desc == ""))
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
		QListViewItemIterator it( m_altList );
		TreeItemElement *i;
		QListViewItem *tmp;
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


void Kalternatives::slotOptionClicked(QListViewItem *option)
{
	AltItemElement *altItem;
	if((altItem = dynamic_cast<AltItemElement *>(option)))
	{
		if( !altItem->isOn() )
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
	QString text ="";
	PropertiesWindow *prop = new PropertiesWindow(this);
	AltItemElement *altItem;
	
	if((altItem = dynamic_cast<AltItemElement *>(m_optionsList->selectedItem())))
	{
		Alternative *a = altItem->getAlternative();
		

		text += "Description : \n";
		text += altItem->getDescription();	
		text +="\n Path : ";
		text += a->getPath();
		text +="\n Priority : ";
		QString priority;
		priority.setNum(a->getPriority());
		text += priority;
				
		QStringList* slavesList = a->getSlaves();
		if(slavesList->count() > 1)
		{
			text +="\n Slaves : ";	
		}
		else
		{
			text +="\n Slave : ";
		}
		
		for ( QStringList::Iterator it = slavesList->begin(); it != slavesList->end(); ++it ) 
		{
			text += "\n\t";
			text += *it;
		}
		prop->m_text->setText(text);
		prop->show();
	}
}


void Kalternatives::save()
{
	QListViewItemIterator it( m_altList );
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
				
				if( origFile.open( IO_WriteOnly )) 
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
							kdDebug() << a->getSelectError() << endl;
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
	return i18n("\n\nKalternatives -- Mandrake/Debian alternatives-system manager");
}



#include "kalternatives.moc"
