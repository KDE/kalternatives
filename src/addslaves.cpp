/***************************************************************************
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
 
#include "addslaves.h"
#include "addslavesui.h"
#include "addalternatives.h"

#include <kpushbutton.h>
#include <qlayout.h>
#include <kurl.h>
 
#include <iostream>
using namespace std;

AddSlaves::AddSlaves(AddAlternatives *addAlternatives):
QWidget(addAlternatives, 0, WDestructiveClose | WType_Modal), m_addAlternatives(addAlternatives)
{
	AddSlavesUi *addSlavesUi = new AddSlavesUi(this);
	QGridLayout *AddSalvesLayout = new QGridLayout( this, 1, 1, 11, 6, "AddSlavesLayout"); 
	AddSalvesLayout->setResizeMode( QLayout::Fixed );
	
	AddSalvesLayout->addWidget(addSlavesUi,0,0);
	
	connect(addSlavesUi->m_bOk, SIGNAL(clicked()), this,
			 SLOT(slotOkClicked()));
	connect(addSlavesUi->m_bBrowse, SIGNAL(clicked()), this,
			SLOT(slotBrowseClicked()));
	connect(addSlavesUi->m_bCancel, SIGNAL( clicked() ), this,
			SLOT( close() ) );
	
	m_fileDialog = new KFileDialog ("", "", this, "Choose ALternative", TRUE);
	connect(m_fileDialog->okButton (), SIGNAL(clicked()), this,
			SLOT(slotOkFileClicked()));
	
	m_Path = addSlavesUi->m_Path;
	
	resize( 435, 185 );
}

AddSlaves::~AddSlaves()
{
	if(m_fileDialog) delete m_fileDialog;
	if(m_Path) delete m_Path;
}

void AddSlaves::slotBrowseClicked()
{
	m_fileDialog->show();
}
void AddSlaves::slotOkFileClicked()
{
	KURL url = m_fileDialog->selectedURL();
	m_Path->setText(url.path());
}

void AddSlaves::slotOkClicked()
{
	if(m_Path->text() != "")
	{
		m_addAlternatives->addSlave(m_Path->text());
		close();
	}
}

#include "addslaves.moc"
