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

#include "altparser.h"
#include <qfileinfo.h>
#include <qfile.h>
#include <qdir.h>
#include <qstringlist.h>

#include <klocale.h>

#include <unistd.h>
#include <string.h>
#include <errno.h>

Alternative::Alternative(Item *parentarg) : m_parent(parentarg)
{
    m_priority = 1;
    m_altSlaves = new QStringList;
}

// Copy constructor
Alternative::Alternative(const Alternative &alt) :
    m_altPath(alt.getPath()),
    m_priority(alt.getPriority()),
    m_description(alt.getDescription()),
    m_parent(alt.getParent())
{
    m_altSlaves = new QStringList( *(alt.m_altSlaves) );
}

Alternative::~Alternative()
{
    if(m_altSlaves)delete m_altSlaves;
}

Alternative& Alternative::operator=(const Alternative &alt)
{
    if(this != &alt)
    {
        if(m_altSlaves)delete m_altSlaves;
        m_altPath = alt.getPath();
        m_priority = alt.getPriority();
        m_description = alt.getDescription();
        m_parent = alt.getParent();
        m_altSlaves = new QStringList( *(alt.m_altSlaves) );
    }
    return (*this);
}

void Alternative::setSlaves(QStringList *slaves)
{
    if(m_altSlaves) delete m_altSlaves;
    m_altSlaves = slaves;
}

bool Alternative::isSelected() const
{
    if(m_parent->isBroken()) return false;
    QFileInfo file("/etc/alternatives/"+m_parent->getName());
    if(!file.isSymLink()) return false;
    if(file.readLink() == m_altPath) return 1;
    return 0;
}


bool Alternative::isBroken() const
{
    return !QFile::exists(m_altPath);
}

bool Alternative::select(QString *selectError)
{
// This method was 19 lines in Python in the original kalternatives :-D
    if(isSelected()) return true;
    if(isBroken())
    {
        if (selectError)
            *selectError = QString("Broken alternative: Unexisting path %1").arg(m_altPath);
        return false;
    }

    // Remove the current link:
    QString parentPath = QString("/etc/alternatives/%1").arg(m_parent->getName());
    QFile origlink(parentPath);
    if(!origlink.remove())
    {
        if (selectError)
            *selectError = QString("Could not delete alternative link %1: %2").arg(parentPath).arg(origlink.errorString());
        return false;
    }

    // Then we do the main link:
    const QByteArray m_altPathAscii = m_altPath.toAscii();
    const QByteArray parentPathAscii = parentPath.toAscii();
    if(symlink(m_altPathAscii.constData(), parentPathAscii.constData()) == -1)
    {
        if (selectError)
            *selectError = QString(strerror(errno));
        return false;
    }

    // And finally the slaves
    SlaveList *parslaves = m_parent->getSlaves();
    if(parslaves->count() == 0 || m_altSlaves->count() == 0) return true;
    int count = 0;
    QStringList::iterator sl;
    Slave *parsl;
    for( sl = m_altSlaves->begin(); sl != m_altSlaves->end(); ++sl)
    {
        parsl = parslaves->at(count);
        QString parstr = QString("/etc/alternatives/%1").arg(parsl->slname);
        QFile parlink(parstr);
        if(!parlink.remove())
        {
            if (selectError)
                *selectError = QString("Could not delete slave alternative link %1: %2").arg(parstr).arg(parlink.errorString());
            return false;
        }
        const QByteArray slAscii = (*sl).toAscii();
        const QByteArray parstrAscii = parstr.toAscii();
        if(symlink(slAscii.constData(), parstrAscii.constData()) == -1)
        {
            if (selectError)
                *selectError = QString(strerror(errno));
            return false;
        }
        ++count;
    }
    return true;
}

QString Alternative::prettyDescription(Alternative *alt)
{
    return alt->getDescription().isEmpty() ? i18n("no description") : alt->getDescription();
}


//*************************************** Item

Item::Item()
{
    m_mode = "auto";
    m_itemSlaves = new SlaveList;
    m_itemAlts = new AltsPtrList;
}

// Deep copy
Item::Item(const Item &item) :
    m_name(item.m_name),
    m_mode(item.m_mode),
    m_path(item.m_path)
{
    m_itemSlaves = new SlaveList;
    m_itemAlts = new AltsPtrList;
    Slave *slave;
    Slave *slavecopy;
    Q_FOREACH (slave, *item.m_itemSlaves)
    {
        slavecopy = new Slave;
        slavecopy->slname = slave->slname;
        slavecopy->slpath = slave->slpath;
        m_itemSlaves->append(slavecopy);
    }

    Alternative *alt;
    Alternative *altcopy;
    Q_FOREACH (alt, *item.m_itemAlts)
    {
        // The Alternative class already has a deep copy constructor:
        altcopy = new Alternative( (*alt) );
        m_itemAlts->append(altcopy);
    }
}

Item& Item::operator=(const Item &item)
{
    if(this != &item)
    {
        if(m_itemSlaves)delete m_itemSlaves;
        if(m_itemAlts)delete m_itemAlts;
        m_name = item.m_name;
        m_mode = item.m_mode;
        m_path = item.m_path;
        m_itemSlaves = new SlaveList;
        m_itemAlts = new AltsPtrList;
        Slave *slave;
        Slave *slavecopy;
        Q_FOREACH (slave, *item.m_itemSlaves)
        {
            slavecopy = new Slave;
            slavecopy->slname = slave->slname;
            slavecopy->slpath = slave->slpath;
            m_itemSlaves->append(slavecopy);
        }

        Alternative *alt;
        Alternative *altcopy;
        Q_FOREACH (alt, *item.m_itemAlts)
        {
            altcopy = new Alternative( (*alt) );
            m_itemAlts->append(altcopy);
        }
    }
    return (*this);
}



Item::~Item()
{
    qDeleteAll(*m_itemSlaves);
    if(m_itemSlaves)delete m_itemSlaves;
    qDeleteAll(*m_itemAlts);
    if(m_itemAlts)delete m_itemAlts;
}

Alternative *Item::getSelected() const
{
    Alternative *a;
    Q_FOREACH (a, *m_itemAlts)
    {
        if(a->isSelected())
        {
            return a;
        }
    }
    return NULL;
}

void Item::setSlaves(SlaveList *slaves)
{
    qDeleteAll(*m_itemSlaves);
    if(this->m_itemSlaves)delete this->m_itemSlaves;
    this->m_itemSlaves = slaves;
}

void Item::addSlave(const QString &namearg, const QString &patharg)
{
    Slave *s = new Slave;
    s->slname = namearg;
    s->slpath = patharg;
    m_itemSlaves->append(s);
}

void Item::delSlave(const QString &namearg)
{
    QMutableListIterator<Slave *> it(*m_itemSlaves);

    Slave *s;
    while (it.hasNext())
    {
        s = it.next();
        if(s->slname == namearg)
        {
            it.remove();
            delete s;
            break;
        }
    }
}
void Item::delSlaveByPath(const QString &patharg)
{
    QMutableListIterator<Slave *> it(*m_itemSlaves);

    Slave *s;
    while (it.hasNext())
    {
        s = it.next();
        if(s->slpath == patharg)
        {
            it.remove();
            delete s;
            break;
        }
    }
}

Alternative *Item::getAlternative(const QString &altpath)
{
    Alternative *a;
    Q_FOREACH (a, *m_itemAlts)
    {
        if(a->getPath() == altpath)
        {
            return a;
        }
    }
    return NULL;
}

void Item::setAlternatives(AltsPtrList &alts)
{
    qDeleteAll(*m_itemAlts);
    if(this->m_itemAlts)delete this->m_itemAlts;
    this->m_itemAlts = &alts;
}

void Item::delAlternativeByPath(const QString &patharg)
{
    QMutableListIterator<Alternative *> it(*m_itemAlts);

    Alternative *a;
    while (it.hasNext())
    {
        a = it.next();
        if(a->getPath() == patharg)
        {
            it.remove();
            delete a;
            break;
        }
    }
}

void Item::delAlternativeByPriority(int priorityarg)
{
    QMutableListIterator<Alternative *> it(*m_itemAlts);

    Alternative *a;
    while (it.hasNext())
    {
        a = it.next();
        if(a->getPriority() == priorityarg)
        {
            it.remove();
            delete a;
            break;
        }
    }
}

bool Item::isBroken() const
{
    return !QFile::exists(m_path);
}

/********************** AltFIlesManager ************/

AltFilesManager::AltFilesManager(const QString &altdirarg) :
    m_altdir(altdirarg)
{
    m_itemlist = new ItemPtrList;
    m_parseOk = true;
    m_errorMsg = "";
    if(!parseAltFiles(m_errorMsg))
    {
        m_parseOk = false;
    }
    //debugPrintAlts();
}

AltFilesManager::~AltFilesManager()
{
    qDeleteAll(*m_itemlist);
    delete m_itemlist;
}

Item* AltFilesManager::getItem(const QString &name) const
{
    Q_FOREACH (Item *i, *m_itemlist)
    {
        if(i->getName() == name)
        {
            return i;
        }
    }
    return NULL;
}

bool AltFilesManager::parseAltFiles(QString &errorstr)
{
    QDir d(m_altdir);
    QStringList fileList = d.entryList();
    QStringList lines;
    QFile altFile;
    QString line, tmp;
    int nslaves;
    unsigned int index, slavesend;

    for( QStringList::Iterator it = fileList.begin(); it != fileList.end(); ++it)
    {
        Item *item = new Item;
        if(*it == "." || *it == "..") {
            delete item;
            continue;
        }

        item->setName(*it);
        altFile.setFileName(m_altdir+"/"+*it);

        if(!altFile.open(QIODevice::ReadOnly))
        {
            errorstr = altFile.errorString();
            delete item;
            return false;
        }

        lines.clear();
        while ( !altFile.atEnd() )
        {
            QByteArray data(9999, '\0');
            if(!altFile.readLine(data.data(), data.count()))
            {
                errorstr = altFile.errorString();
                delete item;
                return false;
            }
            lines.append(QString(data));
        }

        line = lines[0];
        tmp = line.left(line.length()-1);
        item->setMode(tmp);

        line = lines[1];
        tmp = line.left(line.length()-1);
        item->setPath(tmp);

        index = 2;
        line = lines[index];
        nslaves = 0;
        SlaveList *slaves = new SlaveList;

        while(line != "\n")
        {
            tmp = line.left(line.length()-1);
            Slave *slave = new Slave;
            nslaves++;
            slave->slname = tmp;

            line = lines[++index];
            tmp = line.left(line.length()-1);
            slave->slpath = tmp;

            slaves->append(slave);
            line = lines[++index];
        }

        item->setSlaves(slaves);

        ++index;
        while(index < lines.count()-1)
        {
            line = lines[index];
            Alternative *a = new Alternative(item);
            tmp = line.left(line.length()-1);
            a->setPath(tmp);

            if(line=="\n") {
                //File end (with a \n)
                delete a;
                break;
            }

            if(++index == lines.count())
            {
                item->addAlternative(a);
                break;
            }


            line = lines[index];
            tmp = line.left(line.length()-1);
            a->setPriority(tmp.toInt());

            if(++index == lines.count())
            {
                item->addAlternative(a);
                break;
            }

            line = lines[index];
            if(line != "\n" and nslaves > 0)
            {
                slavesend = index+nslaves;
                while(index < slavesend)
                {
                    line = lines[index];
                    tmp = line.left(line.length()-1);
                    a->addSlave(tmp);
                    ++index;
                }
            }
			else
			{
				if (nslaves >0)
				{
					++index;
				}
			}
            item->addAlternative(a);
        }
        m_itemlist->append(item);
        altFile.close();
    }

    return true;
}

//FIXME: This must be in a son of Q3Ptrlist!
/*
int AltFilesManager::compareItems(Item i1, Item i2)
{
    return i1.getPath().compare(i2.getPath());
}
*/

/*
void AltFilesManager::debugPrintAlts() const
{
    printf("----------------------------------\n");
    Item *item;
    for(item = m_itemlist->first(); item; item = m_itemlist->next())
    {
        printf("\nItem: %s\n", item->getName().ascii());
        printf("\tMode: %s\n", item->getMode().ascii());
        printf("\tPath: %s\n", item->getPath().ascii());
        if(item->getSlaves()->count() == 0)
            printf("\tNo slaves\n");
        else
        {
            Slave *slave;
            SlaveList *slaves = item->getSlaves();
            for(slave = slaves->first(); slave; slave = slaves->next())
            {
                printf("\tSlave name: %s\n", slave->name.ascii());
                printf("\tSlave path: %s\n", slave->path.ascii());
            }
        }
        printf("\tAlternatives:\n");
        if(item->getAlternatives()->count() == 0)
            printf("\t\tNO ALTERNATIVES!");
        else
        {
            Alternative *a;
            AltsPtrList *alts = item->getAlternatives();
            for(a = alts->first(); a; a = alts->next())
            {
                printf("\t\tPath: %s\n", a->getPath().ascii());
                printf("\t\tPriority: %d\n", a->getPriority());
                printf("\t\tSlaves:\n");
                if(a->getSlaves()->count() == 0)
                    printf("\t\t\tNo slaves\n");
                else
                {
                    QStringList altslaves = *(a->getSlaves());
                    QStringList::iterator sl;
                    for( sl = altslaves.begin(); sl != altslaves.end(); ++sl)
                    {
                        printf("\t\t\t%s\n", (*sl).ascii());
                    }
                }
            }
        }
    }
}
*/
/*
// ************************************** Test
int main(int argc, char **argv)
{
    AltFilesManager a("/var/lib/rpm/alternatives");
    if(!a.parsingOk())
        printf("ERROR PARSING ALT FILES: %s\n", a.getErrorMsg().ascii());
    else
        printf("\nOK, Finished parsing\n");

    Item *item= a.getItem("vi");
    if(item == NULL) return 0;
    printf("Nombre item: %s\n", item->getName().ascii());
    printf("Path item: %s\n", item->getPath().ascii());
    Alternative *alt = item->getSelected();
    if(alt == NULL) return 0;
    printf("Selected alt: %s\n", alt->getPath().ascii());

    Alternative *vimminimal = item->getAlternative("/bin/vim-minimal");
    if(vimminimal == NULL) { printf("NULL!\n"); return 0; }
    printf("Not selected alt: %s\n", vimminimal->getPath().ascii());

    printf("Selecting vim-minimal instead of vim-enhanced as vi\n");
    if(!vimminimal->select())
    {
        printf("ERROR: %s\n", vimminimal->getSelectError().ascii());
    }

    printf("Now selecting vim-enhanced...\n");
    Alternative *vimen = item->getAlternative("/usr/bin/vim-enhanced");
    if(vimen == NULL) { printf("NULL!\n"); return 0; }
    if(!vimen->select())
    {
        printf("ERROR: %s\n", alt->getSelectError().ascii());
    }
    return 0;
  }
*/
