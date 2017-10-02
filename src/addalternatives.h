/***************************************************************************
 *   Copyright (C) 2004 by Mario Bensi <nef@ipsquad.net>                   *
 *   Copyright (C) 2004, 2008 by Pino Toscano <pino@kde.org>               *
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

#include <qdialog.h>
#include <qlist.h>

class QDialogButtonBox;

class Alternative;
class Item;
class SlaveWidget;

class AddAlternatives : public QDialog, private Ui::AddAlternatives
{
	Q_OBJECT
	
	QDialogButtonBox* m_buttons;
	Item* m_item;
	Alternative* m_alternative;
	QList<SlaveWidget *> m_slaveWidgets;
	
public:
	AddAlternatives(Item* item, QWidget *parent = Q_NULLPTR);
	virtual ~AddAlternatives();
	
	virtual QSize sizeHint() const;
	
	Alternative* alternative() const { return m_alternative; }
	
private slots:
	void slotCheckSlaves();
	void slotOkClicked();
};

#endif //ADDALTERNATIVES_H_
