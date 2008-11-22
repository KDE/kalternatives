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
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02111-1307, USA.          *
 ***************************************************************************/

 
#include "altitemelement.h"
#include "altcontroller.h"
#include "altparser.h"
#include "treeitemelement.h"

#include <qtimer.h> 
#include <klocale.h>
#include <kdebug.h>
#include <k3listview.h>
#include <k3process.h>

/******************************* AltItemElement ********************/

AltItemElement::AltItemElement(TreeItemElement *parent, Alternative *alternative)
: Q3CheckListItem(parent, "", Q3CheckListItem::RadioButton),
  m_alt(alternative),
  m_parent(qobject_cast<K3ListView*>(parent->listView())),
  m_bisBroken(alternative->isBroken()),
  m_path(alternative->getPath())
{
	setOn(alternative->isSelected());
	setEnabled(!m_bisBroken);
	m_desc = "";
}

AltItemElement::~AltItemElement()
{
}


void AltItemElement::searchDescription()
{
	QString exec = m_path;
	int posSlash = exec.lastIndexOf('/');
	
	if (posSlash != -1)
	{
		exec.remove(0, posSlash+1);
	}
	
	if (!exec.isEmpty())
	{
		K3Process *procdesc = new K3Process();
		*procdesc << "whatis";
		*procdesc << exec;
		
		connect(procdesc, SIGNAL(receivedStdout(K3Process *, char *, int)), this,
				SLOT(slotGetDescription(K3Process *, char *, int)));
		//connect(procdesc, SIGNAL( receivedStderr(K3Process *, char *, int) ), this,
			//	SLOT(slotGetDescription(K3Process *, char *, int)));
		connect(procdesc, SIGNAL( processExited(K3Process *)), this,
				SLOT(slotDescriptionTermined(K3Process *)));
		procdesc->start(K3Process::NotifyOnExit,/*K3Process::Block,*/ K3Process::AllOutput);
	}
}


void AltItemElement::slotDescriptionTermined(K3Process *proc)
{
	if (!proc->exitStatus()) 
	{
		int pos = m_desc.indexOf('\n');
		if (pos != -1)
		{
			m_desc.truncate(pos);
		}
		
		pos = m_desc.indexOf(']');
		if (pos != -1)
		{
			m_desc.remove(0, pos+1);
		}
		
		pos = m_desc.indexOf(')');
		if (pos != -1)
		{
			m_desc.remove(0, pos+1);
		}
		
		pos = m_desc.indexOf('-');
		if (pos != -1)
		{
			m_desc.remove(0, pos+2);
		}
	}
	else
	{
		m_desc = i18n( "no description" );
	}
	setText( 3, m_desc);
}

void AltItemElement::slotGetDescription(K3Process *, char *buffer, int buflen)
{
	m_desc += QString::fromLatin1(buffer, buflen);
}



#include "altitemelement.moc"
