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

#include "main.h"
#include "kalternatives.h"
#include "altparser.h"
#include "propertieswindow.h"

#include <qobject.h>
#include <qlabel.h>
#include <qheader.h>
#include <qfile.h>
#include <qtimer.h>
#include <qiconset.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qlistview.h> 

#include <kmainwindow.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kicontheme.h>
#include <kaboutdialog.h>
#include <kcombobox.h> 

#include <unistd.h>
#include <sys/types.h>
//FIXME: Quitar (debug)
#include <stdio.h>
#include <iostream.h>
#include <kprocess.h> 
#include <qvbuttongroup.h>
#include <kurl.h> 
#include <klineedit.h>  
#include <knuminput.h> 
#include <ktextedit.h> 
#include <klistview.h> 

/******************************* AltController ********************/

AltController::AltController(TreeItemElement *treeItem):
m_treeItem(treeItem)
{
}

AltController::~AltController()
{
}


void AltController::setBoutonOnOff(KListView *list)
{
	QListViewItemIterator it( list );
	AltItemElement *alt;
	while ( it.current() ) 
	{
		if((alt = dynamic_cast<AltItemElement *>(it.current())))
		{
			if((alt!=m_altItem) && alt->isOn())
			{
    			alt->setState(QCheckListItem::Off);
			}
		}
		++it;
	}
	
}

/******************************* AltItemElement ********************/

AltItemElement::AltItemElement(KListView *parent, Alternative *alternative, AltController *altControl )
: QCheckListItem(parent, "", QCheckListItem::RadioButton),
  m_alt(alternative),
  m_parent(parent),
  m_bisBroken(alternative->isBroken()),
  m_path(alternative->getPath()),
  m_altControl(altControl)
{
	m_altControl->setAltItem(this);
    setOn(alternative->isSelected());
    setEnabled(!m_bisBroken);
	m_desc = "";
}

AltItemElement::~AltItemElement()
{
    //Don't delete the alt because it is still being used in the AltFilesManager
    delete m_alt;
}



/******************************* TreeItemElement ********************/

TreeItemElement::TreeItemElement(KListView *parent, Item *itemarg)
: QListViewItem(parent, itemarg->getName()),
  m_item(itemarg),
  m_name(itemarg->getName()),
  m_changed(FALSE)
{
	m_altItemslist = new AltItemList;
}


TreeItemElement::~TreeItemElement()
{
}


/*AltsPtrList* TreeItemElement::getData()
{
    if(m_item->isBroken())
        setEnabled(0);
	return m_item->getAlternatives();
}*/

/********************************* MyThread  *****************************************/
#ifdef DEBIAN
MyThread::MyThread(QString path, AltItemElement *altItem, Kalternatives *kalt):
m_path(path), m_altItem(altItem), m_kalt(kalt)
{
}

MyThread::~MyThread()
{
	if (m_kalt) delete m_kalt;
	if (m_altItem) delete m_altItem;
}

void MyThread::run()
{
	QString tmp = m_kalt->getDescription(m_path);
	m_altItem->setDescription(tmp);
	QString desc = tmp;
	desc.truncate(desc.find("\n"));
	m_altItem->setText( 3, desc);	
}

#endif

/*********************************** AddAlternative *************************************/

AddAlternatives::AddAlternatives(MainWindow *parent, TreeItemElement *treeItem, Kalternatives *kalt):
m_treeItem(treeItem), m_kalt(kalt), m_parent(parent)
{
}

void AddAlternatives::init()
{
	m_addAlternativesUi = new AddAlternativesUi(m_parent,0,TRUE,0);
	connect(m_addAlternativesUi->m_bOk, SIGNAL(clicked()), this, SLOT(slotOkClicked()));
	connect(m_addAlternativesUi->m_bBrowse, SIGNAL(clicked()), this, SLOT(slotBrowseClicked()));
	connect(m_addAlternativesUi->m_bBrowseMan, SIGNAL(clicked()), this, SLOT(slotBrowseManClicked()));
	
	m_fileDialog = new KFileDialog ("", "", m_addAlternativesUi, "Choose ALternative", TRUE);
	connect(m_fileDialog->okButton (), SIGNAL(clicked()), this, SLOT(slotOkFileClicked()));
	
	m_fileDialogMan = new KFileDialog ("", "", m_addAlternativesUi, "Choose ALternative Man Page", TRUE);
	connect(m_fileDialogMan->okButton (), SIGNAL(clicked()), this, SLOT(slotOkFileManClicked()));
	
	m_addAlternativesUi->show();
}

AddAlternatives::~AddAlternatives()
{
	delete m_fileDialog;
	delete m_addAlternativesUi;
}

void AddAlternatives::slotBrowseClicked()
{
	m_fileDialog->show();
}
void AddAlternatives::slotOkFileClicked()
{
	KURL url = m_fileDialog->selectedURL();
	m_addAlternativesUi->m_Path->setText(url.path());
}

void AddAlternatives::slotBrowseManClicked()
{
	m_fileDialogMan->show();
}
void AddAlternatives::slotOkFileManClicked()
{
	KURL url = m_fileDialogMan->selectedURL();
	m_addAlternativesUi->m_PathMan->setText(url.path());
}



void AddAlternatives::slotOkClicked()
{
	if(m_addAlternativesUi->m_Path->text() != "")
	{
		
		Item *item = m_treeItem->getItem();
		Alternative *a = new Alternative(item);
		
		a->setPath(m_addAlternativesUi->m_Path->text());
		a->setPriority(m_addAlternativesUi->m_Priority->value());
		if (m_addAlternativesUi->m_PathMan->text() != "")
		{
			a->addSlave(m_addAlternativesUi->m_PathMan->text());
		}
		
		item->addAlternative(a);
		
		AltController *altController = new AltController(m_treeItem);
		AltItemElement *altItem = new AltItemElement(m_parent->m_optionsList, a, altController);
		
		m_treeItem->addAltItem(altItem);
		
		QString priority;
		priority.setNum(a->getPriority());
		
		altItem->setText( 1, priority);
		altItem->setText( 2, a->getPath());
#ifdef DEBIAN
		QString m_small_desc = altItem->getDescription();
		
		if (m_small_desc == "")
		{
			MyThread *thread = new MyThread(a->getPath(), altItem, m_kalt);
			thread->start();
		}
		else
		{
			m_small_desc.truncate(m_small_desc.find("\n"));
			altItem->setText( 3, m_small_desc);
		}
#endif
		
		/*m_parent->m_altList->setSelected( m_parent->m_altList->firstChild(), TRUE );
		m_parent->m_altList->setSelected( m_parent->m_altList->lastItem(), TRUE );
		m_parent->m_altList->setSelected( m_treeItem, TRUE );;*/
		
		m_treeItem->setChanged(TRUE);
		m_addAlternativesUi->close();
	}
}


/*********************************** Main Window *************************************/

Kalternatives::Kalternatives()
{
    m_mainwindow = new MainWindow();
	
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
	KMessageBox::sorry(m_mainwindow, i18n("Kalternatives only work on Debian- or Mandrake-based systems"), i18n("Wrong System Type"));
	QTimer::singleShot(0, this, SLOT(die()));
	#endif
#endif
	
    if(!m_bisRoot)
    {
        if(KMessageBox::warningContinueCancel(m_mainwindow,
            i18n("You are running this program from a non-privileged user account which usually means that you will be unable to apply any selected changes using the Apply button. If you want to commit your changes to the alternatives system please run the program as the root user."), i18n("Non-Privileged User")) == KMessageBox::Cancel)
            QTimer::singleShot(0, this, SLOT(die()));
    }
	
	init();
}

Kalternatives::~Kalternatives()
{  
    if(m_mgr) delete m_mgr;
	if(m_mainwindow) delete m_mainwindow;
}

void Kalternatives::die()
{
	delete this;
}

void Kalternatives::init()
{
    m_mainwindow->m_altList->setShowToolTips(1);
	
	updateData(m_mgr);
	
	connect(m_mainwindow->m_hideAlt, SIGNAL(clicked()), this, SLOT(slotHideAlternativesClicked()));
	
	connect(m_mainwindow->m_altList, SIGNAL(selectionChanged( QListViewItem* )), this, SLOT(slotSelectAlternativesClicked(QListViewItem *)));
	
	connect(m_mainwindow->m_optionsList, SIGNAL(clicked(QListViewItem *)), this, SLOT(slotOptionClicked(QListViewItem *)));
    connect(m_mainwindow->m_bApply, SIGNAL(clicked()), this, SLOT(slotApplyClicked()));
	connect(m_mainwindow->m_bAdd, SIGNAL(clicked()), this, SLOT(slotAddClicked()));
	connect(m_mainwindow->m_bDelete, SIGNAL(clicked()), this, SLOT(slotDeleteClicked()));
	connect(m_mainwindow->m_bProperties, SIGNAL(clicked()), this, SLOT(slotPropertiesClicked()));
	
	m_mainwindow->m_altList->setSelected( m_mainwindow->m_altList->firstChild(), TRUE );
}

void Kalternatives::updateData(AltFilesManager *mgr)
{
	QPtrList<Item> *itemslist = mgr->getGlobalAlternativeList();
    Item *i;
    TreeItemElement *treeit;
    for(i = itemslist->first(); i; i = itemslist->next())
    {
        treeit = new TreeItemElement(m_mainwindow->m_altList, i);
		
		AltsPtrList *altList = i->getAlternatives();
		Alternative *a = altList->first();
		
    	AltItemElement *ael;
		
		for(; a; a=altList->next())
    	{
			AltController *altcontroller = new AltController(treeit);
        	ael = new AltItemElement(m_mainwindow->m_optionsList, a, altcontroller);
			treeit->addAltItem(ael);
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

#ifdef DEBIAN
void Kalternatives::slotGetDescription(KProcess *proc, char *buffer, int buflen)
{
	if (m_desc != "")
	{
		m_desc += QString::fromLatin1(buffer, buflen);
	}
	else
	{
		m_desc =  QString::fromLatin1(buffer, buflen);
	}
	
	int posDesc = m_desc.findRev("Description:");
	if (posDesc != -1)
	{
		m_desc.remove(0, posDesc+12);
	}
}

void Kalternatives::slotGetExecutable(KProcess *proc, char *buffer, int buflen)
{
	if (m_exec != "")
	{
		m_exec += QString::fromLatin1(buffer, buflen);
	}
	else
	{
		m_exec =  QString::fromLatin1(buffer, buflen);
	}
	
	int pos = m_exec.findRev(":");
	if (pos != -1)
	{
		m_exec.truncate(pos);
		pos = m_exec.findRev(",");
		if (pos !=-1)
		{
			m_exec.remove(0,pos+1);
			m_exec = m_exec.simplifyWhiteSpace();
		}
	}
}

QString Kalternatives::getDescription(QString path)
{
	m_mutex.lock();
	m_exec = "";
	KProcess *proc = new KProcess();
	
	*proc << "dpkg";
	*proc << "-S" << path ;
	
	connect(proc, SIGNAL(receivedStdout(KProcess *, char *, int)), this , SLOT(slotGetExecutable(KProcess *, char *, int)));
	connect(proc, SIGNAL( receivedStderr(KProcess *, char *, int) ), this , SLOT(slotGetExecutable(KProcess *, char *, int)));
	
    proc->start(KProcess::NotifyOnExit, KProcess::AllOutput);
	
	proc->wait();
	
	m_desc = "";

	if (m_exec != "")
	{
		KProcess *procdesc = new KProcess();
		
		*procdesc << "dpkg";
		*procdesc << "-p" << m_exec ;
	
		connect(procdesc, SIGNAL(receivedStdout(KProcess *, char *, int)), this , SLOT(slotGetDescription(KProcess *, char *, int)));
		connect(procdesc, SIGNAL( receivedStderr(KProcess *, char *, int) ), this , SLOT(slotGetDescription(KProcess *, char *, int)));
	
	    procdesc->start(KProcess::NotifyOnExit, KProcess::AllOutput);
	
		procdesc->wait();
	}
	m_mutex.unlock();
	return m_desc;
}

#endif

void Kalternatives::slotSelectAlternativesClicked(QListViewItem *alternative)
{
	clearList(m_mainwindow->m_optionsList);
	TreeItemElement *treeItem;
	if((treeItem = dynamic_cast<TreeItemElement *>(alternative)))
	{
		Item *item = treeItem->getItem();
		
		m_mainwindow->m_altTilte->setText(treeItem->getName());
		m_mainwindow->m_statusCombo->setCurrentItem(item->getMode());
		
    	AltItemList *altItemList = treeItem->getAltItemList();
		
		for( AltItemElement *altItem= altItemList->first(); altItem ; altItem = altItemList->next())
		{
			Alternative *a = altItem->getAlternative();
			
			QString priority;
			priority.setNum(a->getPriority());
			
			m_mainwindow->m_optionsList->insertItem(altItem);
			
			altItem->setText( 1, priority);
			altItem->setText( 2, a->getPath());
#ifdef DEBIAN
			QString m_small_desc = altItem->getDescription();
			
			if (m_small_desc == "")
			{
				MyThread *thread = new MyThread(a->getPath(), altItem, this);
				thread->start();
			}
			else
			{
				m_small_desc.truncate(m_small_desc.find("\n"));
				altItem->setText( 3, m_small_desc);
			}
#endif
    	}
	}
	m_mainwindow->m_optionsList->setSelected(m_mainwindow->m_optionsList->firstChild(), 1);
}



void Kalternatives::slotHideAlternativesClicked()
{
	if (m_mainwindow->m_hideAlt->isChecked ())
	{
		QListViewItemIterator it( m_mainwindow->m_altList );
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
					m_mainwindow->m_altList->takeItem(tmp);
					continue;
				}
			}
			it++;
		}
	}
	else
	{
		clearList(m_mainwindow->m_altList);
		updateData(m_mgr);
	}
	m_mainwindow->m_altList->setSelected(m_mainwindow->m_altList->firstChild (), 1);
}


void Kalternatives::slotOptionClicked(QListViewItem *option)
{
    AltItemElement *it;
	if((it = dynamic_cast<AltItemElement *>(option)))
	{
		if( !it->isOn() )
			return;
    	it->getAltController()->setBoutonOnOff(m_mainwindow->m_optionsList);
		TreeItemElement *i;
		if((i = dynamic_cast<TreeItemElement *>(m_mainwindow->m_altList->selectedItem())))
		{
			i->setChanged(TRUE);
			i->setAltItemChanged(it);
		}
		if(!m_mainwindow->m_bApply->isEnabled() && m_bisRoot)
		{
			m_mainwindow->m_bApply->setEnabled(1);
		}
	}
}

void Kalternatives::slotAddClicked()
{
	TreeItemElement *treeItem;
	if((treeItem = dynamic_cast<TreeItemElement *>(m_mainwindow->m_altList->selectedItem())))
	{
		AddAlternatives *addAlternatives = new AddAlternatives(m_mainwindow, treeItem, this);
		addAlternatives->init();
		treeItem->setChanged(TRUE);
		if(!m_mainwindow->m_bApply->isEnabled() && m_bisRoot)
		{
			m_mainwindow->m_bApply->setEnabled(1);
		}
	}
}
void Kalternatives::slotDeleteClicked()
{
	TreeItemElement *treeItem;
	if((treeItem = dynamic_cast<TreeItemElement *>(m_mainwindow->m_altList->selectedItem())))
	{
		AltItemElement *altItem;
		if((altItem = dynamic_cast<AltItemElement *>(m_mainwindow->m_optionsList->selectedItem())))
		{
			treeItem->getItem()->delAlternativeByPath(altItem->getPath());
			
			AltItemList *altItemList = treeItem->getAltItemList();
			
			altItemList->remove(altItem);
			m_mainwindow->m_optionsList->takeItem(altItem);
			
			treeItem->setChanged(TRUE);
			if(!m_mainwindow->m_bApply->isEnabled() && m_bisRoot)
			{
				m_mainwindow->m_bApply->setEnabled(1);
			}
			m_mainwindow->m_optionsList->setSelected(m_mainwindow->m_optionsList->firstChild (), 1);
		}
	}
}

void Kalternatives::slotPropertiesClicked()
{
	QString text ="";
	PropertiesWindow *prop = new PropertiesWindow(m_mainwindow);
	AltItemElement *altItem;
	
	if((altItem = dynamic_cast<AltItemElement *>(m_mainwindow->m_optionsList->selectedItem())))
	{
		Alternative *a = altItem->getAlternative();
		
#ifdef DEBIAN
		text += "Description : \n";
		
		text += altItem->getDescription();	
		
		if (text == "Description : \n")
		{
			MyThread *thread = new MyThread(a->getPath(), altItem, this);
			thread->start();
			thread->wait();
			text += altItem->getDescription();	
		}
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
	}
	prop->m_text->setText(text);
	prop->show();
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
	QListViewItemIterator it( m_mainwindow->m_altList );
	TreeItemElement *treeItem;
	
   	while ( it.current() )
	{
		if((treeItem = dynamic_cast<TreeItemElement *>(it.current())))
		{
			if(treeItem->isChanged())
			{
				AltItemList *altItemList = treeItem->getAltItemList();
				AltItemElement *altItem= altItemList->first();
				for( ; altItem ; altItem = altItemList->next())
				{
					if( altItem->isOn() )
					{
						Alternative *a = altItem->getAlternative();
						if(!a->select())
						{
							cout << a->getSelectError() <<endl;
						}
					}
				}
			}
		}
		it++;
	}
	m_mainwindow->m_bApply->setEnabled(0);
}

#include "kalternatives.moc"
