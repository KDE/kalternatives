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

 
#include "altitemelement.h"
#include "altcontroller.h"
#include "altparser.h"

#include <iostream>
using namespace std;

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
//#ifdef DEBIAN
	findDescriptionThread = new FindDescriptionThread(this);
	m_desc = "";
//#endif
}

AltItemElement::~AltItemElement()
{
	delete m_alt;
}

//#ifdef DEBIAN

void AltItemElement::searchDescription()
{
	if (!findDescriptionThread->running ())
	{
		m_mutex.lock();
		findDescriptionThread->start();
		m_mutex.unlock();
	}
}

void AltItemElement::setDescription(QString desc) 
{
	m_mutex.lock();
	m_desc = desc; 
	desc.truncate(desc.find("\n"));
	setText( 3, desc);
	m_mutex.unlock();
}

//#endif

/********************************* FindDescriptionThread ******************************/
//#ifdef DEBIAN
FindDescriptionThread::FindDescriptionThread(AltItemElement *altItem):
m_altItem(altItem)
{
}

FindDescriptionThread::~FindDescriptionThread()
{
	if (m_altItem) delete m_altItem;
}

void FindDescriptionThread::run()
{
	QString tmp = getDescriptionProcess();
	m_altItem->setDescription(tmp);
	/*sleep(3);
	m_altItem->setDescription("blub");*/
}

void FindDescriptionThread::slotGetDescription(KProcess *, char *buffer, int buflen)
{
	if (m_descTmp != "")
	{
		m_descTmp += QString::fromLatin1(buffer, buflen);
	}
	else
	{
		m_descTmp =  QString::fromLatin1(buffer, buflen);
	}
	
	int posDesc = m_descTmp.findRev("Description:");
	if (posDesc != -1)
	{
		m_descTmp.remove(0, posDesc+12);
	}
}

void FindDescriptionThread::slotGetExecutable(KProcess *, char *buffer, int buflen)
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

QString FindDescriptionThread::getDescriptionProcess()
{
	
	m_exec = "";
	KProcess *proc = new KProcess();
	
	*proc << "dpkg";
	*proc << "-S" << m_altItem->getPath();
	
	connect(proc, SIGNAL(receivedStdout(KProcess *, char *, int)), this,
			SLOT(slotGetExecutable(KProcess *, char *, int)));
	connect(proc, SIGNAL( receivedStderr(KProcess *, char *, int) ), this,
			SLOT(slotGetExecutable(KProcess *, char *, int)));
	
    proc->start(KProcess::NotifyOnExit, KProcess::AllOutput);
	
	proc->wait();
	
	m_descTmp = "";

	if (m_exec != "")
	{
		KProcess *procdesc = new KProcess();
		
		*procdesc << "dpkg";
		*procdesc << "-p" << m_exec ;
	
		connect(procdesc, SIGNAL(receivedStdout(KProcess *, char *, int)), this,
				SLOT(slotGetDescription(KProcess *, char *, int)));
		connect(procdesc, SIGNAL( receivedStderr(KProcess *, char *, int) ), this,
				SLOT(slotGetDescription(KProcess *, char *, int)));
	
		procdesc->start(KProcess::NotifyOnExit, KProcess::AllOutput);
	
		procdesc->wait();
	}
	
	return m_descTmp;
}

//#endif

#include "altitemelement.moc"
