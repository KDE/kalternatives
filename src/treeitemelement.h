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

#ifndef _TREEITEMELEMENT_H_
#define _TREEITEMELEMENT_H_

#include <qstring.h>
#include <qptrlist.h>
#include <qlistview.h>
#include <klistview.h>

class AltItemElement;
class Item;


typedef QPtrList<AltItemElement> AltItemList;

class TreeItemElement : public QListViewItem
{
    Item   *m_item;
    QString m_name;
	bool    m_changed;
	AltItemElement *m_altItemChanged;
	AltItemList *m_altItemslist;
	
public:
    TreeItemElement(KListView *parent, Item *itemarg);
    ~TreeItemElement();

    QString getName() const { return m_name; }
    Item *getItem() const { return m_item; }
	void setChanged(bool c) { m_changed = c; }
	bool isChanged() const { return m_changed; }
	void setAltItemChanged(AltItemElement *alt) { m_altItemChanged = alt; }
	AltItemElement *getAltItemChanged() const { return m_altItemChanged; }
	void addAltItem(AltItemElement *altItem) {m_altItemslist->append(altItem);}
	AltItemList *getAltItemList() {return m_altItemslist;}
};



#endif //_TREEITEMELEMENT_H_
