/***************************************************************************
 *   Copyright (C) 2004 by Juanjo                                          *
 *   juanjux@yahoo.es                                                      *
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

#ifndef _KALTERNATIVES_H_
#define _KALTERNATIVES_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "mainwindow.h"
#include <qlistview.h>
#include <qapplication.h>
#include <klistview.h>
#include <kpushbutton.h>
#include <kiconloader.h>
#include "altparser.h"
#include <qstring.h> 
#include <qstringlist.h> 
#include <qmutex.h>
#include <qthread.h>
#include <kfiledialog.h>
#include "addalternativesui.h"

#define KALT_VERSION "0.12"


class TreeItemElement;
class Alternative;
class Item;
class KProcess;
class AltItemElement;
class Kalternatives;

class AltController
{
	AltItemElement *m_altItem;
	TreeItemElement *m_treeItem;
public:
	AltController(TreeItemElement *treeItem);
	~AltController();
	
	void setAltItem(AltItemElement *altItem){m_altItem = altItem;}
	void setBoutonOnOff(KListView *list);
};

class AltItemElement : public QCheckListItem
{
	Alternative *m_alt;
	KListView *m_parent;
	bool m_bisBroken;
	QString m_path;
	AltController *m_altControl;
	QString m_desc;
	
public:
    AltItemElement(KListView *parent, Alternative *alternative, AltController *altControl );
    ~AltItemElement();

    bool isBroken() const { return m_bisBroken; }
    KListView *getParent() const { return m_parent; }
    Alternative *getAlternative() { return m_alt; }
	QString getPath() const {return m_path; }
	AltController *getAltController() {return m_altControl;}
	QString getDescription() const {return m_desc; }
	void setDescription(QString desc) {m_desc = desc; }
};

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

class MyThread : public QThread 
{
	QString m_path;
	AltItemElement *m_altItem;
	Kalternatives *m_kalt;
public:
	MyThread(QString path, AltItemElement *altItem, Kalternatives *kalt);
	virtual ~MyThread();
	
	virtual void run();
};


class AddAlternatives : public QObject
{
	Q_OBJECT
	
	KFileDialog *m_fileDialog;
	KFileDialog *m_fileDialogMan;
	AddAlternativesUi *m_addAlternativesUi;
	TreeItemElement *m_treeItem;
	Kalternatives *m_kalt;
	MainWindow *m_parent;
public:
	AddAlternatives(MainWindow *parent, TreeItemElement *treeItem, Kalternatives *kalt);
	virtual ~AddAlternatives();
	void init();
	
private slots:
	void slotOkFileClicked();
	void slotOkClicked();
	void slotBrowseClicked();
	void slotBrowseManClicked();
	void slotOkFileManClicked();
	
};


/*
 * @short Application Main Window
 * @author Juanjo Álvarez<juanjux@yahooKMainWindo.es>
 * @version 0.11
 */
class Kalternatives : public QObject
{
    Q_OBJECT

    bool m_bisRoot;
    AltFilesManager *m_mgr;
	QString m_desc;
	QString m_exec;;
	QMutex m_mutex;
	
	void updateData(AltFilesManager *mgr);
	void clearList(KListView* list);
	void init();
	
public:
    Kalternatives();
    virtual ~Kalternatives();

	QString getDescription(QString path);
	MainWindow *m_mainwindow;

private slots:
	void slotSelectAlternativesClicked(QListViewItem *);
    void slotApplyClicked();
	void slotHideAlternativesClicked();
    void slotAboutClicked();
    void die();
	void slotGetDescription(KProcess *proc, char *buffer, int buflen);
	void slotGetExecutable(KProcess *proc, char *buffer, int buflen);
	void slotOptionClicked(QListViewItem *option);
	void slotAddClicked();
	void slotDeleteClicked();
	void slotPropertiesClicked();
};

#endif // _KALTERNATIVES_H_
