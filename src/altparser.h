/***************************************************************************
 *   Copyright (C) 2004 by Juanjo Álvarez                                  *
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

#ifndef _ALTPARSER_H_
#define _ALTPARSER_H_


//FIXME: Cosas pa mirar:
//Comprobar entradas NULL y demás (sobre todo antes de los delete)
//
//Quitar el stdio.h

#include <qstring.h>
#include <qobject.h>
#include <qptrlist.h>
#include <qstringlist.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

class Item;

struct Slave
{
    QString name;
    QString path;
};

class Alternative
{
    QString path;
    QString selectError;
    int priority;
    Item *parent;
    QStringList *slaves;
public:
    Alternative(Item *parentarg);
    Alternative(const Alternative &alt);
    ~Alternative();
    Alternative& operator=(const Alternative &alt);

    Item* getParent() const { return parent; }
    QString getPath() const { return path; }
    void setPath(const QString &patharg) { path = patharg; }
    int getPriority() const { return priority; }
    void setPriority(int priorityarg) { priority = priorityarg; }
    QStringList* getSlaves() const { return slaves; }
    void setSlaves(QStringList *slaves);
    void addSlave(const QString &slave) { slaves->append(slave); }
    uint countSlaves() const { return slaves->count(); }
    QString getSlave(int pos) const { return *(slaves->at(pos)); }
    bool isSelected() const;
    bool isBroken() const;
    bool select();
    QString getSelectError() const { return selectError; }
};

typedef QPtrList<Slave> SlaveList;
typedef QPtrList<Alternative> AltsPtrList;

class Item 
{
    QString name;
    QString mode;
    QString path;
    SlaveList *slaves;
    AltsPtrList *alts;
public:
    Item();
    // Deep copy constructor:
    Item(const Item &item);
    ~Item();
    Item& operator=(const Item &item);

    Alternative* getSelected() const;
    QString getName() const { return name; }
    void setName(const QString &namearg) { name = namearg; }
    QString getMode() const { return mode; }
    //Check the input (FIXME)
    void setMode(const QString &modearg) { mode = modearg; }
    QString getPath() const { return path; }
    void setPath(const QString &patharg) { path = patharg; }
    SlaveList *getSlaves() const { return slaves; } 
    void setSlaves(SlaveList *slaves);
    void addSlave(const QString &namearg, const QString &patharg);
    void delSlave(const QString &namearg);
    void delSlaveByPath(const QString &patharg);
    AltsPtrList *getAlternatives() const { return alts; }
    Alternative *getAlternative(const QString &altpath);
    void setAlternatives(AltsPtrList &alts);
    int countAlternatives() const { return alts->count(); }
    void delAlternativeByPath(const QString &patharg);
    void delAlternativeByPriority(int priorityarg);
    void addAlternative(Alternative *altarg) { alts->append(altarg); }
    bool isBroken() const;
};

typedef QPtrList<Item> ItemPtrList;

class AltFilesManager
{
    ItemPtrList *itemlist;
    QString altdir;
    QString errorMsg;
    bool parseOk;

    bool parseAltFiles(QString &errorstr);
public:
    AltFilesManager(const QString &altdir);
    ~AltFilesManager();

    ItemPtrList* getGlobalAlternativeList() const { return this->itemlist; }
    bool parsingOk() const { return parseOk; }
    QString getErrorMsg() const { return errorMsg; }
    Item* getItem (const QString &name) const;
    //FIXME: Put in a #ifdef
    void debugPrintAlts() const;
//protected:
    //virtual int compareItems(Item i1, Item i2);
};
#endif // _KALTERNATIVES_H_
