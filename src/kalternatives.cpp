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

#include "main.h"
#include "kalternatives.h"
#include "altparser.h"

#include <qobject.h>
#include <qlabel.h>
#include <qheader.h>
#include <qfile.h>
#include <qtimer.h>
#include <qiconset.h>
#include <qlayout.h>

#include <kmainwindow.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kicontheme.h>
#include <kaboutdialog.h>

#include <unistd.h>
#include <sys/types.h>
//FIXME: Quitar (debug)
#include <stdio.h>

/******************************* AltItemElement ********************/

AltItemElement::AltItemElement(TreeItemElement *parent, Alternative *alternative)
: QCheckListItem(parent, alternative->getPath(),
        QCheckListItem::RadioButton),
        alt(alternative),
        parent(parent),
        bisBroken(alternative->isBroken()),
        balreadyEnabled(alternative->isSelected()),
        bisNode(false),
        path(alternative->getPath())
{
    setOn(balreadyEnabled);
    setEnabled(!bisBroken);
}

AltItemElement::~AltItemElement()
{
    //Don't delete the alt because it is still being used in the AltFilesManager
    //delete alt;
}

void AltItemElement::stateChange(bool on)
{
    if(balreadyEnabled && (!isOn()) )
        balreadyEnabled = 0;
    QCheckListItem::stateChange(on);
}


/******************************* TreeItemElement ********************/

TreeItemElement::TreeItemElement(KListView *parent, Item *itemarg)
: QCheckListItem(parent, itemarg->getName(), QCheckListItem::RadioButtonController),
    item(itemarg), name(itemarg->getName()), bisNode(1)
{
    altList = new QPtrList<AltItemElement>;
    altList->setAutoDelete(1);
}

TreeItemElement::~TreeItemElement()
{
    delete altList;
}

void TreeItemElement::setData()
{
    if(item->isBroken())
        setEnabled(0);

    Alternative *a;
    AltItemElement *ael;
    for(a = item->getAlternatives()->first(); a; a = item->getAlternatives()->next())
    {
        ael = new AltItemElement(this, a);
        altList->append(ael);
    }
}

void TreeItemElement::setup()
{
    setExpandable(1);
    QCheckListItem::setup();
}

/*********************************** ItemsWidget *************************************/
ItemsWidget::ItemsWidget(QWidget *parent) : KListView(parent)
{
    addColumn("Alternatives");
    header()->hide();
    setRootIsDecorated(1);
    setShowToolTips(1);
    setItemsMovable(0);
    setAcceptDrops(0);

    connect(this, SIGNAL(clicked(QListViewItem *)), this, SLOT(slotItemClicked(QListViewItem *)));
    itemWidgetsList = new QPtrList<TreeItemElement>;
    itemWidgetsList->setAutoDelete(1);
    changed = 0;
}

ItemsWidget::~ItemsWidget()
{
    delete itemWidgetsList;
}

void ItemsWidget::updatedata(AltFilesManager *mgr)
{
    QPtrList<Item> *itemslist = mgr->getGlobalAlternativeList();
    Item *i;
    TreeItemElement *treeit;
    for(i = itemslist->first(); i; i = itemslist->next())
    {
        treeit = new TreeItemElement(this, i);
        treeit->setData();
        treeit->setOpen(0);
        itemWidgetsList->append(treeit);
    }
    setMinimumSize(QSize(200,280));
}

void ItemsWidget::slotItemClicked(QListViewItem *qlit)
{
    if(!qlit) return;
    // 1001 = AltItemElement
    if( qlit->rtti() != 1001 )
        return;

    AltItemElement *it = (AltItemElement *)qlit;
    if( !it->isOn() || it->alreadyEnabled())
        return;
    changed = 1;
    it->setAlreadyEnabled(1);
    emit iwChanged();
}
/*********************************** Main Window *************************************/
kalternatives::kalternatives()
    : KMainWindow( 0, "kalternatives" )
{
    new QLabel( "Hello World", this, "hello label" );
    int user = getuid();
    //FIXME: This won't be needed as kcm
    if (user == 0) isRoot = true;
    else isRoot = false;
    icons = new KIconLoader();
    QWidget *centralWidget = new QWidget(this, "centralW");
    setCentralWidget(centralWidget);

    if(QFile::exists("/var/lib/rpm/alternatives") && QFile::exists("/etc/mandrakelinux-release"))
    {
        // Mandrake
        distro = 1;
        mgr = new AltFilesManager("/var/lib/rpm/alternatives");
    }
    else if(QFile::exists("/var/lib/dpkg/alternatives"))
    {
        // Debian
        distro = 0;
        mgr = new AltFilesManager("/var/lib/dpkg/alternatives");
    }

    else
    {
        // Crap ;)
        KMessageBox::sorry(this, i18n("Kalternatives only work on Debian or Mandrake based systems"), i18n("Wrong System Type"));
        QTimer::singleShot(0, this, SLOT(die()));
    }

    iw = new ItemsWidget(centralWidget);
    iw->updatedata(mgr);

    apply = new KPushButton(QIconSet(icons->loadIcon("ok", KIcon::Small)), i18n("&Apply"), centralWidget);
    apply->setEnabled(0);
    expand = new KPushButton(i18n("&Expand All"), centralWidget);
    collapse = new KPushButton(i18n("C&ollapse All"), centralWidget);
    about = new KPushButton(QIconSet(icons->loadIcon("about_kde", KIcon::Small)), i18n("A&bout"), centralWidget);
    help = new KPushButton(QIconSet(icons->loadIcon("help", KIcon::Small)), i18n("&Help"), centralWidget);
    close = new KPushButton(QIconSet(icons->loadIcon("exit", KIcon::Small)), i18n("&Close"), centralWidget);

    connect(close, SIGNAL(clicked()),   this, SLOT(slotCloseClicked()));
    connect(collapse, SIGNAL(clicked()),this, SLOT(slotCollapseClicked()));
    connect(expand, SIGNAL(clicked()),  this, SLOT(slotExpandClicked()));
    connect(about, SIGNAL(clicked()),   this, SLOT(slotAboutClicked()));
    connect(iw, SIGNAL(iwChanged()), this, SLOT(slotSelectionChanged()));
    connect(apply, SIGNAL(clicked()), this, SLOT(slotApplyClicked()));


    QHBoxLayout *l = new QHBoxLayout(centralWidget, 10);
    l->addWidget(iw, 10);
    QVBoxLayout *buttonBox = new QVBoxLayout;
    l->addLayout(buttonBox);

    buttonBox->addWidget(apply);
    buttonBox->addWidget(expand);
    buttonBox->addWidget(collapse);
    buttonBox->addStretch(1);
    buttonBox->addWidget(about);
    buttonBox->addWidget(help);
    buttonBox->addWidget(close);

    //FIXME: Remove as kcm
    setMinimumSize(QSize(300,300));
    resize(QSize(420,360));

    if(!isRoot)
    {
        if(KMessageBox::warningContinueCancel(this,
                    i18n("You are running this program from a non-privileged user account which usually means that you will be unable to apply any selected changes using the Apply button. If you want to commit your changes to the alternatives system please run the program as the root user."), i18n("Non Privileged User")) == KMessageBox::Cancel)
            QTimer::singleShot(0, this, SLOT(die()));
    }



    // set the shell's ui resource file
    //setXMLFile("kalternativesui.rc");

    //AltFilesManager *a = new AltFilesManager("/var/lib/rpm/alternatives");

    setCaption(i18n("Alternatives Manager"));
    connect(iw, SIGNAL(iwChanged()), this, SLOT(slotSelectionChanged()));
}

kalternatives::~kalternatives()
{
    if(icons)delete icons;
    if(mgr) delete mgr;
    if(iw) delete iw;
}

void kalternatives::die()
{
    delete this;
}

void kalternatives::slotSelectionChanged()
{
    if(iw->getChanged() && isRoot)
        apply->setEnabled(1);
}

void kalternatives::slotCloseClicked()
{
    queryClose();
    app->quit();
}

void kalternatives::slotCollapseClicked()
{
    QPtrListIterator<TreeItemElement> it(*(iw->getItemWidgetsList()));
    TreeItemElement *i;
    while( (i = it.current()) != 0)
    {
        ++it;
        iw->setOpen(i, 0);
    }
}

void kalternatives::slotExpandClicked()
{
    QPtrListIterator<TreeItemElement> it(*(iw->getItemWidgetsList()));
    TreeItemElement *i;
    while( (i = it.current()) != 0)
    {
        ++it;
        iw->setOpen(i, 1);
    }
}

void kalternatives::slotAboutClicked()
{
    KAboutDialog *dlg = new KAboutDialog;
    dlg->setTitle(i18n("KDE Mandrake/Debian alternatives-system manager"));
    dlg->setAuthor("Juanjo Alvarez Martinez", "juanjo@juanjoalvarez.net",
                  "http://juanjoalvarez.net", "\n\nKalternatives -- Mandrake/Debian alternatives system manager");
    dlg->setVersion(KALT_VERSION);
    dlg->show();
}

void kalternatives::slotApplyClicked()
{
    if(!isRoot) {
        KMessageBox::information(this, i18n("Non root user"),
        i18n("You are not the root user. If you want your changed to be applied you have to run this program as root"), i18n("&Ok"));
        return;
    }

    QPtrList<AltItemElement> *forChangeList = getChangedList();
    if (forChangeList->count() == 0) {
        KMessageBox::information(this, i18n("You didn't change any alternative"), i18n("No change selected"));
        return;
    }
    QPtrListIterator<AltItemElement> altit(*forChangeList);
    QString confstr;
    AltItemElement *altItem;

    while ( (altItem = altit.current()) != 0 )
    {
        ++altit;
        if (!altItem->getAlternative()->select()) {
            AltItemElement *alt2;
            QPtrListIterator<AltItemElement> it2(*( altItem->getParent()->getAltList() ));
            while( (alt2 = it2.current()) != 0)
            {
                ++it2;
                if (alt2->getAlternative()->isSelected()) {
                    alt2->setOn(1);
                }

            }
            if ( KMessageBox::warningContinueCancel(this,
                i18n("There was a problem changing to the %1 alternative:\n %2")
                .arg(altItem->getAlternative()->getPath())
                .arg(altItem->getAlternative()->getSelectError()),
                i18n("Error")) == KMessageBox::Cancel )
                break;
        }
        if ( getChangedList()->count() == 0 )
            apply->setEnabled(0);
    }
}

QPtrList<AltItemElement> *kalternatives::getChangedList()
{
    QPtrList<AltItemElement> *forChangeList = new QPtrList<AltItemElement>;
    QPtrListIterator<TreeItemElement> ittree(*(iw->getItemWidgetsList()));
    TreeItemElement *node;
    while ( (node = ittree.current()) != 0 )
    {
        ++ittree;
        QPtrListIterator<AltItemElement> itnode(*(node->getAltList()));
        AltItemElement *alt;
        while( (alt = itnode.current()) != 0)
        {
            ++itnode;
            if (alt->isOn() && !(alt->getAlternative()->isSelected()))
                forChangeList->append(alt);
        }
    }
    return forChangeList;
}

bool kalternatives::queryClose()
{
  if (isRoot)
  {
    if (getChangedList()->count() != 0)
    {
        if (KMessageBox::warningYesNo(this,
            i18n("Some changes were not applied. Do you want to apply them now?"),
            i18n("Unapplied changes")) != KMessageBox::No);
                slotApplyClicked();
    }
  }
  return 1;
}

//#include "moc_kalternatives.cpp"
