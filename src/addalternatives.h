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

#ifndef _ADDALTERNATIVES_H_
#define _ADDALTERNATIVES_H_

#include <kfiledialog.h>
#include <klineedit.h> 
#include <knuminput.h> 
#include <qwidget.h>
#include <ktextedit.h>

class TreeItemElement;
class Kalternatives;

class AddAlternatives : public QWidget
{
	Q_OBJECT
	
	KFileDialog *m_fileDialog;
	TreeItemElement *m_treeItem;
	Kalternatives *m_kalt;
	KLineEdit* m_Path;
	KIntSpinBox* m_Priority;
	KTextEdit* m_textSlave;
	int m_countSlave;
	
public:
	AddAlternatives(TreeItemElement *treeItem, Kalternatives *kalt, int countSlaves);
	virtual ~AddAlternatives();
	
	void addSlave(QString text){m_textSlave->append(text);}
	
private slots:
	void slotOkFileClicked();
	void slotOkClicked();
	void slotBrowseClicked();
	void slotAddSlaveClicked();	
};

#endif //ADDALTERNATIVES_H_
