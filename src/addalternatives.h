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

#ifndef _ADDALTERNATIVES_H_
#define _ADDALTERNATIVES_H_

#include "ui_addalternativesui.h"

#include <kdialog.h>
#include <ktextedit.h>

class TreeItemElement;
class Kalternatives;

class AddAlternatives : public KDialog, private Ui::AddAlternatives
{
	Q_OBJECT
	
	TreeItemElement *m_treeItem;
	Kalternatives *m_kalt;
	int m_countSlave;
	
public:
	AddAlternatives(TreeItemElement *treeItem, Kalternatives *kalt, int countSlaves);
	virtual ~AddAlternatives();
	
	void addSlave(const QString& text){m_textSlave->append(text);}
	
protected slots:
	void slotOkClicked();
	void slotAddSlaveClicked();	
};

#endif //ADDALTERNATIVES_H_
