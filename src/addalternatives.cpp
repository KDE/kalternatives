/***************************************************************************
 *   Copyright (C) 2004 by Mario Bensi                                    *
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

#include "addalternatives.h"
#include "treeitemelement.h"
#include "kalternatives.h"
#include "altcontroller.h"
#include "altitemelement.h"
#include "altparser.h"

#include <qstringlist.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kstandardguiitem.h>
#include <kurlrequesterdialog.h>

AddAlternatives::AddAlternatives(TreeItemElement *treeItem, Kalternatives *kalt, int countSlaves)
	: KDialog(kalt), m_treeItem(treeItem), m_kalt(kalt), m_countSlave(countSlaves)
{
	setupUi(mainWidget());
	
	setButtons(Ok | Cancel | User1);
	setButtonGuiItem(User1, KGuiItem(i18n("&Add Slave"), "list-add"));
	setCaption(i18n("Add Alternative"));
	showButtonSeparator(true);
	
	m_Path->setWindowTitle( i18n( "Choose Alternative" ) );
	m_Path->setFilter( i18n( "*|All Files" ) );
	m_Path->setMode( KFile::File | KFile::LocalOnly );
	
	connect(this, SIGNAL(user1Clicked()), this, SLOT(slotAddSlaveClicked()));
}

AddAlternatives::~AddAlternatives()
{
}

void AddAlternatives::slotAddSlaveClicked()
{
	KUrlRequesterDialog d(QString(), i18n("Select the path to the new slave."), this);
	d.setCaption(i18n("Add Slave"));
	d.urlRequester()->setWindowTitle(i18n("Choose Slave"));
	d.urlRequester()->setFilter(i18n("*|All Files"));
	d.urlRequester()->setMode(KFile::File | KFile::LocalOnly);
	if (d.exec() != QDialog::Accepted)
		return;

	const KUrl url = d.selectedUrl();
	if (!url.isEmpty())
	{
		addSlave(url.toLocalFile());
	}
}

void AddAlternatives::slotOkClicked()
{
	if(!m_Path->url().isEmpty())
	{
		
		Item *item = m_treeItem->getItem();
		Alternative *a = new Alternative(item);
		
		a->setPath(m_Path->url().toLocalFile());
		a->setPriority(m_Priority->value());
		
		int countSlave = 0;
		
		const QString text = m_textSlave->toPlainText();
		if (!text.isEmpty())
		{
			QStringList slaveList = text.split('\n', QString::SkipEmptyParts);
			QStringList::Iterator it = slaveList.begin();
			for ( ; it != slaveList.end(); ++it ) 
			{
				a->addSlave(*it);
				countSlave++;
			}
		}
		
		if (countSlave == m_countSlave)
		{
			item->addAlternative(a);
		
		
			AltItemElement *altItem = new AltItemElement(m_treeItem, a);
		
			m_treeItem->getAltController()->addAltItem(altItem);
		
		
			QString priority;
			priority.setNum(a->getPriority());
		
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
			m_treeItem->setNbrAltChanged(TRUE);
			emit m_kalt->configChanged();
			close();
		}
		else
		{
			KMessageBox::sorry(this, i18n("The number of slaves is not good."), i18n("Number Of Slaves"));
		}
	}
}

#include "addalternatives.moc"
