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
        m_alt(alternative),
        m_parent(parent),
        m_bisBroken(alternative->isBroken()),
        m_balreadyEnabled(alternative->isSelected()),
        m_bisNode(false),
        m_path(alternative->getPath())
{
    setOn(m_balreadyEnabled);
    setEnabled(!m_bisBroken);
}

AltItemElement::~AltItemElement()
{
    //Don't delete the alt because it is still being used in the AltFilesManager
    //delete m_alt;
}

void AltItemElement::stateChange(bool on)
{
    if(m_balreadyEnabled && (!isOn()) )
        m_balreadyEnabled = 0;
    QCheckListItem::stateChange(on);
}


/******************************* TreeItemElement ********************/

TreeItemElement::TreeItemElement(KListView *parent, Item *itemarg)
: QCheckListItem(parent, itemarg->getName(), QCheckListItem::RadioButtonController),
  m_item(itemarg),
  m_name(itemarg->getName()),
  m_bisNode(1)
{
    m_altList = new QPtrList<AltItemElement>;
    m_altList->setAutoDelete(1);
}

TreeItemElement::~TreeItemElement()
{
    delete m_altList;
}

void TreeItemElement::setData()
{
    if(m_item->isBroken())
        setEnabled(0);

    Alternative *a;
    AltItemElement *ael;
    for(a = m_item->getAlternatives()->first(); a; a = m_item->getAlternatives()->next())
    {
        ael = new AltItemElement(this, a);
        m_altList->append(ael);
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
    m_itemWidgetsList = new QPtrList<TreeItemElement>;
    m_itemWidgetsList->setAutoDelete(1);
    m_bChanged = 0;
}

ItemsWidget::~ItemsWidget()
{
    delete m_itemWidgetsList;
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
        m_itemWidgetsList->append(treeit);
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
    m_bChanged = 1;
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
    if (user == 0) m_bisRoot = true;
    else m_bisRoot = false;
    m_icons = new KIconLoader();
    QWidget *centralWidget = new QWidget(this, "centralW");
    setCentralWidget(centralWidget);

    if(QFile::exists("/var/lib/rpm/alternatives") && QFile::exists("/etc/mandrakelinux-release"))
    {
        // Mandrake
        m_distro = MANDRAKE;
        m_mgr = new AltFilesManager("/var/lib/rpm/alternatives");
    }
    else if(QFile::exists("/var/lib/dpkg/alternatives"))
    {
        // Debian
        m_distro = DEBIAN;
        m_mgr = new AltFilesManager("/var/lib/dpkg/alternatives");
    }

    else
    {
        // Crap ;)
        KMessageBox::sorry(this, i18n("Kalternatives only work on Debian- or Mandrake-based systems"), i18n("Wrong System Type"));
        QTimer::singleShot(0, this, SLOT(die()));
    }

    m_iw = new ItemsWidget(centralWidget);
    m_iw->updatedata(m_mgr);

    m_apply = new KPushButton(QIconSet(m_icons->loadIcon("ok", KIcon::Small)), i18n("&Apply"), centralWidget);
    m_apply->setEnabled(0);
    m_expand = new KPushButton(i18n("&Expand All"), centralWidget);
    m_collapse = new KPushButton(i18n("C&ollapse All"), centralWidget);
    m_about = new KPushButton(QIconSet(m_icons->loadIcon("about_kde", KIcon::Small)), i18n("A&bout"), centralWidget);
    m_help = new KPushButton(QIconSet(m_icons->loadIcon("help", KIcon::Small)), i18n("&Help"), centralWidget);
    m_close = new KPushButton(QIconSet(m_icons->loadIcon("exit", KIcon::Small)), i18n("&Close"), centralWidget);

    connect(m_close, SIGNAL(clicked()),   this, SLOT(slotCloseClicked()));
    connect(m_collapse, SIGNAL(clicked()),this, SLOT(slotCollapseClicked()));
    connect(m_expand, SIGNAL(clicked()),  this, SLOT(slotExpandClicked()));
    connect(m_about, SIGNAL(clicked()),   this, SLOT(slotAboutClicked()));
    connect(m_iw, SIGNAL(iwChanged()), this, SLOT(slotSelectionChanged()));
    connect(m_apply, SIGNAL(clicked()), this, SLOT(slotApplyClicked()));


    QHBoxLayout *l = new QHBoxLayout(centralWidget, 10);
    l->addWidget(m_iw, 10);
    QVBoxLayout *buttonBox = new QVBoxLayout;
    l->addLayout(buttonBox);

    buttonBox->addWidget(m_apply);
    buttonBox->addWidget(m_expand);
    buttonBox->addWidget(m_collapse);
    buttonBox->addStretch(1);
    buttonBox->addWidget(m_about);
    buttonBox->addWidget(m_help);
    buttonBox->addWidget(m_close);

    //FIXME: Remove as kcm
    setMinimumSize(QSize(300,300));
    resize(QSize(420,360));

    if(!m_bisRoot)
    {
        if(KMessageBox::warningContinueCancel(this,
            i18n("You are running this program from a non-privileged user account which usually means that you will be unable to apply any selected changes using the Apply button. If you want to commit your changes to the alternatives system please run the program as the root user."), i18n("Non-Privileged User")) == KMessageBox::Cancel)
            QTimer::singleShot(0, this, SLOT(die()));
    }



    // set the shell's ui resource file
    //setXMLFile("kalternativesui.rc");

    //AltFilesManager *a = new AltFilesManager("/var/lib/rpm/alternatives");

    setCaption(i18n("Alternatives Manager"));
    connect(m_iw, SIGNAL(iwChanged()), this, SLOT(slotSelectionChanged()));
}

kalternatives::~kalternatives()
{
    if(m_icons)delete m_icons;
    if(m_mgr) delete m_mgr;
    if(m_iw) delete m_iw;
}

void kalternatives::die()
{
    delete this;
}

void kalternatives::slotSelectionChanged()
{
    if(m_iw->getChanged() && m_bisRoot)
        m_apply->setEnabled(1);
}

void kalternatives::slotCloseClicked()
{
    queryClose();
    app->quit();
}

void kalternatives::slotCollapseClicked()
{
    QPtrListIterator<TreeItemElement> it(*(m_iw->getItemWidgetsList()));
    TreeItemElement *i;
    while( (i = it.current()) != 0)
    {
        ++it;
        m_iw->setOpen(i, 0);
    }
}

void kalternatives::slotExpandClicked()
{
    QPtrListIterator<TreeItemElement> it(*(m_iw->getItemWidgetsList()));
    TreeItemElement *i;
    while( (i = it.current()) != 0)
    {
        ++it;
        m_iw->setOpen(i, 1);
    }
}

void kalternatives::slotAboutClicked()
{
    KAboutDialog *dlg = new KAboutDialog;
    dlg->setTitle(i18n("KDE Mandrake/Debian alternatives-system manager"));
    dlg->setAuthor("Juanjo Alvarez Martinez", "juanjo@juanjoalvarez.net",
                "http://juanjoalvarez.net", "\n\nKalternatives -- Mandrake/Debian alternatives-system manager");
    dlg->setVersion(KALT_VERSION);
    dlg->show();
}

void kalternatives::slotApplyClicked()
{
    if(!m_bisRoot) {
        KMessageBox::information(this, i18n("Non-root user"),
        i18n("You are not the root user; if you want your changes to be applied you have to run this program as root."), i18n("&Ok"));
        return;
    }

    QPtrList<AltItemElement> *forChangeList = getChangedList();
    if (forChangeList->count() == 0) {
        KMessageBox::information(this, i18n("You did not change any alternatives"), i18n("No change selected"));
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
        if ( countChanged() == 0 )
            m_apply->setEnabled(0);
    }
    delete forChangeList;
}

QPtrList<AltItemElement> *kalternatives::getChangedList()
{
    QPtrList<AltItemElement> *forChangeList = new QPtrList<AltItemElement>;
    QPtrListIterator<TreeItemElement> ittree(*(m_iw->getItemWidgetsList()));
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
    // Don't forget to delete it!!
    return forChangeList;
}

int kalternatives::countChanged()
{
    QPtrList<AltItemElement> *fc = getChangedList();
    int count = fc->count();
    delete fc;
    return count;
}


bool kalternatives::queryClose()
{
  if (m_bisRoot)
  {
    if (countChanged() != 0)
    {
        if (KMessageBox::warningYesNo(this,
            i18n("Some changes were not applied; do you want to apply them now?"),
            i18n("Unapplied changes")) != KMessageBox::No);
                slotApplyClicked();
    }
  }
  return 1;
}

//#include "moc_kalternatives.cpp"
