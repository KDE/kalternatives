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

#ifndef _ALTCONTROLLER_H_
#define _ALTCONTROLLER_H_

#include <q3ptrlist.h>

class K3ListView;
class AltItemElement;

typedef Q3PtrList<AltItemElement> AltItemList;


class AltController
{
	AltItemList *m_altItemslist;
	
public:
	AltController();
	~AltController();

	void setBoutonOnOff(K3ListView *list, AltItemElement *altItem);
	void addAltItem(AltItemElement *altItem) {m_altItemslist->append(altItem);}
	AltItemList *getAltItemList() {return m_altItemslist;}
};
#endif // _ALTCONTROLLER_H_
