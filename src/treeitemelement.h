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

#ifndef _TREEITEMELEMENT_H_
#define _TREEITEMELEMENT_H_

#include <q3listview.h>

class K3ListView;
class AltController;
class Item;


class TreeItemElement : public Q3ListViewItem
{
    Item   *m_item;
    QString m_name;
	bool    m_changed;
	bool    m_nbrAltChanged;
	AltController *m_altControl;
	
public:
    TreeItemElement(K3ListView *parent, Item *itemarg, AltController *altControl);
    ~TreeItemElement();

    QString getName() const { return m_name; }
    Item *getItem() const { return m_item; }
	void setChanged(bool c) { m_changed = c; }
	bool isChanged() const { return m_changed; }
	void setNbrAltChanged(bool c) { m_nbrAltChanged = c; }
	bool isNbrAltChanged() const { return m_nbrAltChanged; }
	AltController *getAltController() {return m_altControl;}
	
	virtual void paintCell( QPainter * p, const QColorGroup & cg, int column, int width, int align );
};

#endif //_TREEITEMELEMENT_H_
