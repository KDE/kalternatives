/***************************************************************************
 *   Copyright (C) 2004 by Mario BENSI                                     *
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
 
 #include "addalternatives.h"
 #include "treeitemelement.h"
 #include "kalternatives.h"
 #include "addalternativesui.h"
 #include "altcontroller.h"
 #include "altitemelement.h"
 #include "altparser.h"
 
 #include <qlayout.h>
 #include <iostream>
 #include <kurl.h>
 
 using namespace std;

AddAlternatives::AddAlternatives(TreeItemElement *treeItem, Kalternatives *kalt):
QWidget(kalt,0, WDestructiveClose | WType_Modal),m_treeItem(treeItem), m_kalt(kalt)
{
	AddAlternativesUi *addAlternativesUi = new AddAlternativesUi(this);
	QGridLayout *AddAlternativesLayout = new QGridLayout( this, 1, 1, 11, 6, "AddAlternativesLayout"); 
    AddAlternativesLayout->setResizeMode( QLayout::Fixed );
	
	AddAlternativesLayout->addWidget(addAlternativesUi,0,0);
	
	connect(addAlternativesUi->m_bOk, SIGNAL(clicked()), this,
			SLOT(slotOkClicked()));
	connect(addAlternativesUi->m_bBrowse, SIGNAL(clicked()), this,
			SLOT(slotBrowseClicked()));
	connect(addAlternativesUi->m_bBrowseMan, SIGNAL(clicked()), this,
			SLOT(slotBrowseManClicked()));
	connect(addAlternativesUi->m_bCancel, SIGNAL( clicked() ), this,
			SLOT( close() ) );
	
	
	m_fileDialog = new KFileDialog ("", "", this, "Choose ALternative", TRUE);
	connect(m_fileDialog->okButton (), SIGNAL(clicked()), this,
			SLOT(slotOkFileClicked()));
	
	m_fileDialogMan = new KFileDialog ("", "", this, "Choose ALternative Man Page", TRUE);
	connect(m_fileDialogMan->okButton (), SIGNAL(clicked()), this,
			SLOT(slotOkFileManClicked()));
	
	m_Path = addAlternativesUi->m_Path;
	m_PathMan = addAlternativesUi->m_PathMan;
	m_Priority = addAlternativesUi->m_Priority;
	
	resize( 435, 185 );
}

AddAlternatives::~AddAlternatives()
{
	delete m_fileDialog;
}

void AddAlternatives::slotBrowseClicked()
{
	m_fileDialog->show();
}
void AddAlternatives::slotOkFileClicked()
{
	KURL url = m_fileDialog->selectedURL();
	m_Path->setText(url.path());
}

void AddAlternatives::slotBrowseManClicked()
{
	m_fileDialogMan->show();
}
void AddAlternatives::slotOkFileManClicked()
{
	KURL url = m_fileDialogMan->selectedURL();
	m_PathMan->setText(url.path());
}



void AddAlternatives::slotOkClicked()
{
	if(m_Path->text() != "")
	{
		
		Item *item = m_treeItem->getItem();
		Alternative *a = new Alternative(item);
		
		a->setPath(m_Path->text());
		a->setPriority(m_Priority->value());
		if (m_PathMan->text() != "")
		{
			a->addSlave(m_PathMan->text());
		}
		
		item->addAlternative(a);
		
		
		AltItemElement *altItem = new AltItemElement(m_kalt->optionsList(), a);
		
		m_treeItem->getAltController()->addAltItem(altItem);
		
		
		QString priority;
		priority.setNum(a->getPriority());
		
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
		m_treeItem->setChanged(TRUE);
		if (!close())
		{
			cout << "je veux pas me fermer et je t'emmerde !!" << endl;
		}
	}
}

#include "addalternatives.moc"