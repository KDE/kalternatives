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
 
 #include "treeitemelement.h"
 #include "altitemelement.h"
 #include "altparser.h"

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

 