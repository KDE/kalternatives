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
 
#include "treeitemelement.h"
#include "altparser.h"
#include "altcontroller.h"
#include <qfont.h> 
#include <qpainter.h> 

#include <iostream>
using namespace std;
 
TreeItemElement::TreeItemElement(KListView *parent, Item *itemarg, AltController *altControl )
: QListViewItem(parent, itemarg->getName()),
  m_item(itemarg),
  m_name(itemarg->getName()),
  m_changed(FALSE),
  m_nbrAltChanged(FALSE),
  m_altControl(altControl)
{
}


TreeItemElement::~TreeItemElement()
{
}

void TreeItemElement::paintCell( QPainter * p, const QColorGroup & cg, int column, int width, int align )
{
	QColor color;
	
	if (m_changed || m_nbrAltChanged)
	{
		color=QColor("red");
		QFont f = p->font();
		f.setBold(true);
		p->setFont(f);
	}
	 
	// the pallet of colors is saved
	QColorGroup _cg( cg );
	QColor oldText=_cg.text();
 
	// modification of the pallet of colors by defining 
	//our new color as color of text
	_cg.setColor( QColorGroup::Text, color );
 
	QListViewItem::paintCell( p, _cg, column, width, align );
 
	// restoration of the pallet of "standard" colors
	_cg.setColor( QColorGroup::Text, oldText );
 
}
