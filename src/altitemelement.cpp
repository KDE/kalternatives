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

 
#include "altitemelement.h"
#include "altcontroller.h"
#include "altparser.h"

#include <qtimer.h> 
#include <klocale.h>
#include <kdebug.h>
/******************************* AltItemElement ********************/

AltItemElement::AltItemElement(KListView *parent, Alternative *alternative)
: QCheckListItem(parent, "", QCheckListItem::RadioButton),
  m_alt(alternative),
  m_parent(parent),
  m_bisBroken(alternative->isBroken()),
  m_path(alternative->getPath())
{
	setOn(alternative->isSelected());
	setEnabled(!m_bisBroken);
	m_desc = "";
}

AltItemElement::~AltItemElement()
{
	delete m_alt;
}


void AltItemElement::searchDescription()
{
	QString exec = m_path;
	int posSlash = exec.findRev("/");
	
	if (posSlash != -1)
	{
		exec.remove(0, posSlash+1);
	}
	
	if (!exec.isEmpty())
	{
		KProcess *procdesc = new KProcess();
		*procdesc << "whatis";
		*procdesc << exec;
		
		connect(procdesc, SIGNAL(receivedStdout(KProcess *, char *, int)), this,
				SLOT(slotGetDescription(KProcess *, char *, int)));
		//connect(procdesc, SIGNAL( receivedStderr(KProcess *, char *, int) ), this,
			//	SLOT(slotGetDescription(KProcess *, char *, int)));
		connect(procdesc, SIGNAL( processExited(KProcess *)), this,
				SLOT(slotDescriptionTermined(KProcess *)));
		procdesc->start(KProcess::NotifyOnExit,/*KProcess::Block,*/ KProcess::AllOutput);
	}
}


void AltItemElement::slotDescriptionTermined(KProcess *proc)
{
	if (!proc->exitStatus()) 
	{
		int pos = m_desc.find("\n");
		if (pos != -1)
		{
			m_desc.truncate(pos);
		}
		
		pos = m_desc.find("]");
		if (pos != -1)
		{
			m_desc.remove(0, pos+1);
		}
		
		pos = m_desc.find(")");
		if (pos != -1)
		{
			m_desc.remove(0, pos+1);
		}
		
		pos = m_desc.find("-");
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

void AltItemElement::slotGetDescription(KProcess *, char *buffer, int buflen)
{
	m_desc += QString::fromLatin1(buffer, buflen);
}



#include "altitemelement.moc"
