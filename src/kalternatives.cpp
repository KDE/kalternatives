/***************************************************************************
 *   Copyright (C) 2004 by Juanjo                                          *
 *   juanjux@yahoo.es                                                      *
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



Kalternatives::Kalternatives()
{
	int user = getuid();
	//FIXME: This won't be needed as kcm
	if (user == 0) m_bisRoot = true;
	else m_bisRoot = false;

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
	
	if(!m_bisRoot)
	{
	if(KMessageBox::warningContinueCancel(this,
		i18n("You are running this program from a non-privileged user account which usually means that you will be unable to apply any selected changes using the Apply button. If you want to commit your changes to the alternatives system please run the program as the root user."), i18n("Non-Privileged User")) == KMessageBox::Cancel)
		QTimer::singleShot(0, this, SLOT(die()));
	}
	
	MainWindow *mainwindow = new MainWindow(this);
	QGridLayout *KalternativesLayout = new QGridLayout( this, 1, 1, 11, 6, "KalternativesLayout"); 
	KalternativesLayout->addWidget(mainwindow, 0, 0);
	
	mainwindow->m_altList->setShowToolTips(1);
	
	connect(mainwindow->m_hideAlt, SIGNAL(clicked()), this,
			SLOT(slotHideAlternativesClicked()));
	
	connect(mainwindow->m_altList, SIGNAL(selectionChanged( QListViewItem* )), this,
			SLOT(slotSelectAlternativesClicked(QListViewItem *)));
	
	connect(mainwindow->m_optionsList, SIGNAL(clicked(QListViewItem *)), 
			this, SLOT(slotOptionClicked(QListViewItem *)));
	connect(mainwindow->m_bApply, SIGNAL(clicked()), this,
			SLOT(slotApplyClicked()));
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
	m_bApply = mainwindow->m_bApply;
	
	updateData(m_mgr);
	m_altList->setSelected( m_altList->firstChild(), TRUE );
	resize(615, 490);
}

Kalternatives::~Kalternatives()
{  
	if(m_mgr) delete m_mgr;
}

void Kalternatives::die()
{
	delete this;
}

void Kalternatives::updateData(AltFilesManager *mgr)
{
	QPtrList<Item> *itemslist = mgr->getGlobalAlternativeList();
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
#ifdef DEBIAN
			QString m_small_desc = altItem->getDescription();
			
			if (!(m_small_desc == ""))
			{
				m_small_desc.truncate(m_small_desc.find("\n"));
				altItem->setText( 3, m_small_desc);
			}
			else
			{
				altItem->searchDescription();
			}
#endif
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
		clearList(m_altList);
		updateData(m_mgr);
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
		}
		if(!m_bApply->isEnabled() && m_bisRoot)
		{
			m_bApply->setEnabled(1);
		}
	}
}

void Kalternatives::slotAddClicked()
{
	TreeItemElement *treeItem;
	if((treeItem = dynamic_cast<TreeItemElement *>(m_altList->selectedItem())))
	{
		AddAlternatives *addAlternatives = new AddAlternatives(treeItem, this);
		addAlternatives->show();
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
			if(!m_bApply->isEnabled() && m_bisRoot)
			{
				m_bApply->setEnabled(1);
			}
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
		
#ifdef DEBIAN
		text += "Description : \n";
		
		text += altItem->getDescription();	
		
		/*if (text == "Description : \n")
		{
			FindDescriptionThread *thread = new FindDescriptionThread(a->getPath(), altItem, this);
			thread->start();
			thread->wait();
			text += altItem->getDescription();	
		}*/
#endif
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

void Kalternatives::slotAboutClicked()
{
	/*KAboutDialog *dlg = new KAboutDialog;
	dlg->setTitle(i18n("KDE Mandrake/Debian alternatives-system manager"));
	dlg->setAuthor("Juanjo Alvarez Martinez", "juanjo@juanjoalvarez.net",
		"http://juanjoalvarez.net", "\n\nKalternatives -- Mandrake/Debian alternatives-system manager");
	dlg->setVersion(KALT_VERSION);
	dlg->show();*/
}

void Kalternatives::slotApplyClicked()
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
	m_bApply->setEnabled(0);
}

#include "kalternatives.moc"
