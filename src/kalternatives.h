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

#define KALT_VERSION "0.11"
#define DEBIAN 0
#define MANDRAKE 1

class TreeItemElement;
class Alternative;
class Item;

class AltItemElement : public QCheckListItem
{
    Alternative *m_alt;
    TreeItemElement *m_parent;
    bool m_bisBroken;
    bool m_balreadyEnabled;
    bool m_bisNode;
    QString m_path;

public:
    AltItemElement(TreeItemElement *parent, Alternative *alternative);
    ~AltItemElement();

    bool isBroken() const { return m_bisBroken; }
    bool alreadyEnabled() const { return m_balreadyEnabled; }
    void setAlreadyEnabled(bool b) { m_balreadyEnabled = b; }
    bool isNode() const { return m_bisNode; }
    TreeItemElement *getParent() const { return m_parent; }
    Alternative *getAlternative() { return m_alt; }

protected:
    virtual void stateChange(bool);
    virtual int rtti() const { return 1001; }
};

class TreeItemElement : public QCheckListItem
{
    Item   *m_item;
    QString m_name;
    bool    m_bisNode;
    QPtrList<AltItemElement> *m_altList;

public:
    TreeItemElement(KListView *parent, Item *itemarg);
    ~TreeItemElement();

    void setData();
    bool isNode() const { return m_bisNode; }
    Item *getItem() const { return m_item; }
    QPtrList<AltItemElement> *getAltList() const { return m_altList; }

protected:
    virtual void setup();
    virtual int rtti() const { return 1002; }

};

class ItemsWidget : public KListView
{
    Q_OBJECT

    QPtrList<TreeItemElement> *m_itemWidgetsList;
    bool m_bChanged;

public:
    ItemsWidget(QWidget *parent);
    ~ItemsWidget();

    void updatedata(AltFilesManager *mgr);
    QPtrList<TreeItemElement> *getItemWidgetsList() const { return m_itemWidgetsList; }
    bool getChanged() const { return m_bChanged; }

public slots:
    void slotItemClicked(QListViewItem *);

signals:
    void iwChanged();
};


/*
 * @short Application Main Window
 * @author Juanjo Álvarez<juanjux@yahoo.es>
 * @version 0.11
 */
class kalternatives : public KMainWindow
{
    Q_OBJECT

    bool m_bisRoot;
    int m_distro;
    KIconLoader *m_icons;
    AltFilesManager *m_mgr;
    ItemsWidget *m_iw;
    KPushButton *m_apply;
    KPushButton *m_expand;
    KPushButton *m_collapse;
    KPushButton *m_about;
    KPushButton *m_help;
    KPushButton *m_close;

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
