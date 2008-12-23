/***************************************************************************
 *   Copyright (C) 2004 by Juanjo Álvarez Martinez <juanjo@juanjoalvarez.net> *
 *   Copyright (C) 2008 by Pino Toscano <pino@kde.org>                     *
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

#ifndef _ALTPARSER_H_
#define _ALTPARSER_H_

#include <qmetatype.h>
#include <qstringlist.h>

class Item;

struct Slave
{
    QString slname;
    QString slpath;
};

class Alternative
{
    QString m_altPath;
    int m_priority;
    QString m_description;
    Item *m_parent;
    QStringList m_altSlaves;
public:
    Alternative(Item *parentarg);
    Alternative(const Alternative &alt);
    ~Alternative();
    Alternative& operator=(const Alternative &alt);

    Item* getParent() const { return m_parent; }
    QString getPath() const { return m_altPath; }
    void setPath(const QString &patharg) { m_altPath = patharg; }
    int getPriority() const { return m_priority; }
    void setDescription(const QString &desc) { m_description = desc; }
    QString getDescription() const { return m_description; }
    void setPriority(int priorityarg) { m_priority = priorityarg; }
    QStringList getSlaves() const { return m_altSlaves; }
    void setSlaves(const QStringList &slaves);
    void addSlave(const QString &slave) { m_altSlaves.append(slave); }
    uint countSlaves() const { return m_altSlaves.count(); }
    QString getSlave(int pos) const { return m_altSlaves.at(pos); }
    bool isSelected() const;
    bool isBroken() const;
    bool select(QString *selectError = 0);

    static QString prettyDescription(Alternative *);
};

typedef QList<Slave *> SlaveList;
typedef QList<Alternative *> AltsPtrList;

class Item
{
public:
    enum ItemMode
    {
        AutoMode,
        ManualMode
    };
private:
    QString m_name;
    ItemMode m_mode;
    QString m_path;
    SlaveList *m_itemSlaves;
    AltsPtrList *m_itemAlts;
public:
    Item();
    // Deep copy constructor:
    Item(const Item &item);
    ~Item();
    Item& operator=(const Item &item);

    Alternative* getSelected() const;
    QString getName() const { return m_name; }
    void setName(const QString &namearg) { m_name = namearg; }
    ItemMode getMode() const { return m_mode; }
    void setMode(ItemMode modearg) { m_mode = modearg; }
    QString getPath() const { return m_path; }
    void setPath(const QString &patharg) { m_path = patharg; }
    SlaveList *getSlaves() const { return m_itemSlaves; }
    void setSlaves(SlaveList *slaves);
    void addSlave(const QString &namearg, const QString &patharg);
    void delSlave(const QString &namearg);
    void delSlaveByPath(const QString &patharg);
    AltsPtrList *getAlternatives() const { return m_itemAlts; }
    Alternative *getAlternative(const QString &altpath);
    void setAlternatives(AltsPtrList &alts);
    int countAlternatives() const { return m_itemAlts->count(); }
    void delAlternativeByPath(const QString &patharg);
    void delAlternativeByPriority(int priorityarg);
    void addAlternative(Alternative *altarg) { m_itemAlts->append(altarg); }
    bool isBroken() const;
};

typedef QList<Item *> ItemPtrList;

class AltFilesManager
{
    ItemPtrList *m_itemlist;
    QString m_altdir;
    QString m_errorMsg;
    bool m_parseOk;

    bool parseAltFiles(QString &errorstr);
public:
    AltFilesManager(const QString &altdir);
    ~AltFilesManager();

    ItemPtrList* getGlobalAlternativeList() const { return this->m_itemlist; }
    bool parsingOk() const { return m_parseOk; }
    QString getErrorMsg() const { return m_errorMsg; }
    Item* getItem (const QString &name) const;
    //FIXME: Put in a #ifdef
    void debugPrintAlts() const;
	QString getAltDir() { return m_altdir ;}
};

Q_DECLARE_METATYPE(Item::ItemMode)

#endif // _KALTERNATIVES_H_
