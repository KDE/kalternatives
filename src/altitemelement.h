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

#ifndef _ALTITEMELEMENT_H_
#define _ALTITEMELEMENT_H_

#include <qlistview.h>
#include <klistview.h>
#include <qstring.h>
#include <qmutex.h>
#include <qthread.h>
#include <kprocess.h>

class Alternative;
class AltController;
class FindDescriptionThread;

class AltItemElement :  public QObject, public QCheckListItem
{
	Q_OBJECT
	
	Alternative *m_alt;
	KListView *m_parent;
	bool m_bisBroken;
	QString m_path;
	QString m_desc;
	FindDescriptionThread *findDescriptionThread;
	QMutex m_mutex;
	
public:
    AltItemElement(KListView *parent, Alternative *alternative );
    ~AltItemElement();

    bool isBroken() const { return m_bisBroken; }
    KListView *getParent() const { return m_parent; }
    Alternative *getAlternative() { return m_alt; }
	QString getPath() const {return m_path; }
	QString getDescription() const {return m_desc;}
	void searchDescription();
	void setDescription(QString desc);
	
private slots:
	void slotKillThread();
};

class FindDescriptionThread : public QObject, public QThread
{
	Q_OBJECT
	
	AltItemElement *m_altItem;
	QString m_descTmp;
	QString m_exec;
public:
	FindDescriptionThread(AltItemElement *altItem);
	virtual ~FindDescriptionThread();
	
	virtual void run();
	QString getDescriptionProcess();
private slots:
	void slotGetDescription(KProcess *proc, char *buffer, int buflen);
	void slotGetExecutable(KProcess *proc, char *buffer, int buflen);
};

#endif //_ALTITEMELEMENT_H_
