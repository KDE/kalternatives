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

#include "altcontroller.h"
#include "altitemelement.h"
#include "treeitemelement.h"

AltController::AltController()
{
	m_altItemslist = new AltItemList;
}

AltController::~AltController()
{
	 if(m_altItemslist) delete m_altItemslist;
}


void AltController::setBoutonOnOff(KListView *list, AltItemElement *altItem)
{
	QListViewItemIterator it( list );
	AltItemElement *alt;
	while ( it.current() ) 
	{
		if((alt = dynamic_cast<AltItemElement *>(it.current())))
		{
			if((alt!=altItem) && alt->isOn())
			{
    			alt->setState(QCheckListItem::Off);
			}
		}
		++it;
	}
	
}
//#include "altcontroller.moc"
