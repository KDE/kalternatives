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

#include <kmainwindow.h>
#include <qlistview.h>
#include <qapplication.h>
#include <klistview.h>
#include <kpushbutton.h>
#include <kiconloader.h>
#include "altparser.h"

#define KALT_VERSION "0.10"

class TreeItemElement;
class Alternative;
class Item;

class AltItemElement : public QCheckListItem
{
    Alternative *alt;
    TreeItemElement *parent;
    bool bisBroken;
    bool balreadyEnabled;
    bool bisNode;
    QString path;

public:
    AltItemElement(TreeItemElement *parent, Alternative *alternative);
    ~AltItemElement();

    bool isBroken() const { return bisBroken; }
    bool alreadyEnabled() const { return balreadyEnabled; }
    void setAlreadyEnabled(bool b) { balreadyEnabled = b; }
    bool isNode() const { return bisNode; }
    TreeItemElement *getParent() const { return parent; }
    Alternative *getAlternative() { return alt; }

protected:
    virtual void stateChange(bool);
    virtual int rtti() const { return 1001; }
};

class TreeItemElement : public QCheckListItem
{
    Item *item;
    QString name;
    bool bisNode;
    QPtrList<AltItemElement> *altList;

public:
    TreeItemElement(KListView *parent, Item *itemarg);
    ~TreeItemElement();

    void setData();
    bool isNode() const { return bisNode; }
    Item *getItem() const { return item; }
    QPtrList<AltItemElement> *getAltList() const { return altList; }

protected:
    virtual void setup();
    virtual int rtti() const { return 1002; }

};

class ItemsWidget : public KListView
{
    Q_OBJECT

    QPtrList<TreeItemElement> *itemWidgetsList;
    bool changed;

public:
    ItemsWidget(QWidget *parent);
    ~ItemsWidget();

    void updatedata(AltFilesManager *mgr);
    QPtrList<TreeItemElement> *getItemWidgetsList() const { return itemWidgetsList; }
    bool getChanged() const { return changed; }

public slots:
    void slotItemClicked(QListViewItem *);

signals:
    void iwChanged();
};


/*
 * @short Application Main Window
 * @author Juanjo Álvarez<juanjux@yahoo.es>
 * @version 0.10
 */
class kalternatives : public KMainWindow
{
    Q_OBJECT

    bool isRoot;
    int distro;
    KIconLoader *icons;
    AltFilesManager *mgr;
    ItemsWidget *iw;
    KPushButton *apply;
    KPushButton *expand;
    KPushButton *collapse;
    KPushButton *about;
    KPushButton *help;
    KPushButton *close;

    void start();
    QPtrList<AltItemElement> *getChangedList();
    int countChanged();

public:
    kalternatives();
    virtual ~kalternatives();

protected:
    virtual bool queryClose();

private slots:
    void slotApplyClicked();
    void slotExpandClicked();
    void slotCollapseClicked();
    void slotCloseClicked();
    void slotAboutClicked();
    void slotSelectionChanged();
    void die();
};

#endif // _KALTERNATIVES_H_
