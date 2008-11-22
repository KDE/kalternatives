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

#ifndef _ALTITEMELEMENT_H_
#define _ALTITEMELEMENT_H_

#include <q3listview.h>

class K3ListView;
class K3Process;
class Alternative;
class AltController;
class TreeItemElement;

class AltItemElement :  public QObject, public Q3CheckListItem
{
	Q_OBJECT
	
	Alternative *m_alt;
	K3ListView *m_parent;
	bool m_bisBroken;
	QString m_path;
	QString m_desc;
	
public:
    AltItemElement(TreeItemElement *parent, Alternative *alternative );
    ~AltItemElement();

    bool isBroken() const { return m_bisBroken; }
    K3ListView *getParent() const { return m_parent; }
    Alternative *getAlternative() { return m_alt; }
	QString getPath() const {return m_path; }
	QString getDescription() const {return m_desc;}
	void searchDescription();
	
private slots:
	void slotDescriptionTermined(K3Process *);
	void slotGetDescription(K3Process *proc, char *buffer, int buflen);
};

#endif //_ALTITEMELEMENT_H_
