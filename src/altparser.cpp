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

#include "altparser.h"
#include <qfileinfo.h>
#include <qfile.h>
#include <qdir.h>
#include <qstringlist.h>

#include <unistd.h>
#include <string.h>
#include <errno.h>

Alternative::Alternative(Item *parentarg) : parent(parentarg)
{
    priority = 1;
    slaves = new QStringList;
}

// Copy constructor
Alternative::Alternative(const Alternative &alt) : 
    path(alt.getPath()), 
    priority(alt.getPriority()), 
    parent(alt.parent)
{
    slaves = new QStringList( *(alt.slaves) );
}

Alternative::~Alternative()
{
    if(slaves)delete slaves;
}

Alternative& Alternative::operator=(const Alternative &alt)
{
    if(this != &alt)
    {
        if(slaves)delete slaves;
        path = alt.getPath();
        priority = alt.getPriority();
        parent = alt.parent;
        slaves = new QStringList( *(alt.slaves) );
    }
    return (*this);
}

void Alternative::setSlaves(QStringList *slaves)
{
    if(this->slaves)delete this->slaves;
    this->slaves = slaves;
}

bool Alternative::isSelected() const
{
    if(parent->isBroken()) return false;
    QFileInfo file("/etc/alternatives/"+parent->getName());
    if(!file.isSymLink()) return false;
    if(file.readLink() == path) return 1;
    return 0;
}


bool Alternative::isBroken() const
{
    return !QFile::exists(path);
}

bool Alternative::select()
{
// This method was 19 lines in Python in the original kalternatives :-D
    if(isSelected()) return true;
    if(isBroken())
    {
        selectError = QString("Broken alternative: Unexisting path %1").arg(path);
        return false;
    }

    // Remove the current link:
    QString parentPath = QString("/etc/alternatives/%1").arg(parent->getName());
    QFile origlink(parentPath);
    if(!origlink.remove())
    {
        selectError = QString("Could not delete alternative link %1: %2").arg(parentPath).arg(origlink.errorString());
        return false;
    }

    // Then we do the main link:
    if(symlink(path.ascii(), parentPath.ascii()) == -1)
    {
        selectError = QString(strerror(errno));
        return false;
    }

    // And finally the slaves
    SlaveList *parslaves = parent->getSlaves();
    if(parslaves->count() == 0 || slaves->count() == 0) return true;
    int count = 0;
    QStringList::iterator sl;
    Slave *parsl;
    for( sl = slaves->begin(); sl != slaves->end(); ++sl)
    {
        parsl = parslaves->at(count);
        QString parstr = QString("/etc/alternatives/%1").arg(parsl->name);
        QFile parlink(parstr);
        if(!parlink.remove())
        {
            selectError = QString("Could not delete slave alternative link %1: %2").arg(parstr).arg(parlink.errorString());
            return false;
        }
        if(symlink( (*sl).ascii(), parstr.ascii()) == -1)
        {
            selectError = QString(strerror(errno));
            return false;
        }
        ++count;
    }
    return true;
}


//*************************************** Item

Item::Item()
{
    mode = "auto";
    slaves = new SlaveList;
    alts = new AltsPtrList;
    slaves->setAutoDelete(1);
    alts->setAutoDelete(1);
}

// Deep copy
Item::Item(const Item &item) : 
    name(item.name), 
    mode(item.mode), 
    path(item.path)
{
    slaves = new SlaveList;
    alts = new AltsPtrList;
    slaves->setAutoDelete(1);
    alts->setAutoDelete(1);
    Slave *slave;
    Slave *slavecopy;
    for(slave = item.slaves->first(); slave; slave = item.slaves->next())
    {
        slavecopy = new Slave;
        slavecopy->name = slave->name;
        slavecopy->path = slave->path;        
        slaves->append(slavecopy);
    }

    Alternative *alt;
    Alternative *altcopy;
    for(alt = item.alts->first(); alt; alt = item.alts->next())
    {
        // The Alternative class already has a deep copy constructor:
        altcopy = new Alternative( (*alt) );
        alts->append(altcopy);
    }
}

Item& Item::operator=(const Item &item)
{
    if(this != &item)
    {
        if(slaves)delete slaves;
        if(alts)delete alts;
        name = item.name;
        mode = item.mode;
        path = item.path;
        slaves = new SlaveList;
        alts = new AltsPtrList;
        slaves->setAutoDelete(1);
        alts->setAutoDelete(1);
        Slave *slave;
        Slave *slavecopy;
        for(slave = item.slaves->first(); slave; slave = item.slaves->next())
        {
            slavecopy = new Slave;
            slavecopy->name = slave->name;
            slavecopy->path = slave->path;
            slaves->append(slavecopy);
        }
        
        Alternative *alt;
        Alternative *altcopy;
        for(alt = item.alts->first(); alt; alt = item.alts->next())
        {
            altcopy = new Alternative( (*alt) );
            alts->append(altcopy);
        }
    }
    return (*this);
}

        

Item::~Item()
{
    if(slaves)delete slaves;
    if(alts)delete alts;
    /*
    Slave *slave;
    for(slave = slaves->first(); slave; slave = slaves->next())
        delete slave;
    delete slaves;
    
    Alternative *a;
    for(a = alts->first(); a; a = alts->next())
        delete a;
    delete alts;
    */
}

Alternative *Item::getSelected() const
{
    Alternative *a;
    for(a = alts->first(); a; a = alts->next())
    {
        if(a->isSelected())
        {
            return a;
            break;
        }
    }
    return NULL;
}

void Item::setSlaves(SlaveList *slaves)
{
    if(this->slaves)delete this->slaves;
    this->slaves = slaves;
}

void Item::addSlave(const QString &namearg, const QString &patharg)
{
    Slave *s = new Slave;
    s->name = namearg;
    s->path = patharg;
    slaves->append(s);
}

void Item::delSlave(const QString &namearg)
{
    QPtrListIterator<Slave> it(*slaves);

    Slave *s;
    while( (s = it.current()) != 0)
    {
        ++it;
        if(s->name == namearg) 
        {
            slaves->remove(s);
            break;
        }
    }
}
void Item::delSlaveByPath(const QString &patharg)
{
    QPtrListIterator<Slave> it(*slaves);
    
    Slave *s;
    while( (s = it.current()) != 0)
    {
        ++it;
        if(s->path == patharg)
        {
            slaves->remove(s);
            break;
        }
    }
}

Alternative *Item::getAlternative(const QString &altpath)
{
    Alternative *a;
    for(a = alts->first(); a; a = alts->next())
    {
        if(a->getPath() == altpath)
        {
            return a;
            break;
        }
    }
    return NULL;
}

void Item::setAlternatives(AltsPtrList &alts)
{
    if(this->alts)delete this->alts;
    this->alts = &alts;
}

void Item::delAlternativeByPath(const QString &patharg)
{
    QPtrListIterator<Alternative> it(*alts);

    Alternative *a;
    while( (a = it.current()) != 0)
    {
        ++it;
        if(a->getPath() == patharg)
        {
            alts->remove(a);
            break;
        }
    }
}

void Item::delAlternativeByPriority(int priorityarg)
{
    QPtrListIterator<Alternative> it(*alts);
    
    Alternative *a;
    while( (a = it.current()) != 0)
    {
        ++it;
        if(a->getPriority() == priorityarg)
        {
            alts->remove(a);
            break;
        }
    }
}

bool Item::isBroken() const
{
    return !QFile::exists(path);
}

/********************** AltFIlesManager ************/

AltFilesManager::AltFilesManager(const QString &altdirarg) : 
    altdir(altdirarg)
{
    itemlist = new ItemPtrList;
    itemlist->setAutoDelete(1);
    parseOk = true;
    errorMsg = "";
    if(!parseAltFiles(errorMsg))
    {
        parseOk = false;
    }
    //debugPrintAlts();
}

AltFilesManager::~AltFilesManager()
{
    if(itemlist)delete itemlist;
    /*
    Item *item;
    for(item = itemlist->first(); item; item = itemlist->next())
    {
        delete item;
    }    

    delete itemlist;
    */
}

Item* AltFilesManager::getItem(const QString &name) const
{
    QPtrListIterator<Item> it(*itemlist);

    Item *i;
    while( (i = it.current()) !=  0)
    {
        ++it;
        if(i->getName() == name) 
        {
            return i;
            break;
        }
    }
    return NULL;
}

bool AltFilesManager::parseAltFiles(QString &errorstr)
{
    QDir d(altdir);
    QStringList fileList = d.entryList();
    QStringList lines;
    QFile altFile;
    QString line, tmp;
    int nslaves;
    unsigned int index, slavesend;

    for( QStringList::Iterator it = fileList.begin(); it != fileList.end(); ++it)
    {
        Item *item = new Item;
        if(*it == "." || *it == "..")continue;

        item->setName(*it);
        altFile.setName(altdir+"/"+*it);

        if(!altFile.open( IO_ReadOnly ))
        {
            errorstr = altFile.errorString();
            return false;
        }

        lines.clear();
        while ( !altFile.atEnd() )
        {
            if(!altFile.readLine(line, 9999))
            {
                errorstr = altFile.errorString();
                return false;
            }
            lines.append(line);
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
            slave->name = tmp;
            
            line = lines[++index];
            tmp = line.left(line.length()-1);
            slave->path = tmp;

            slaves->append(slave);
            line = lines[++index];
        }

        item->setSlaves(slaves);
        line = lines[++index];

        while(index < lines.count()-1)
        {
            Alternative *a = new Alternative(item);
            tmp = line.left(line.length()-1);
            a->setPath(tmp);
            if(line=="\n")
                //File end (with a \n)
                break;
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

            item->addAlternative(a);
        }
        itemlist->append(item);
        altFile.close();
    }

    return true;
}

//FIXME: This must be in a son of qptrlist!
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
    for(item = itemlist->first(); item; item = itemlist->next())
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
