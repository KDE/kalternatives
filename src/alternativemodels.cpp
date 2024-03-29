/***************************************************************************
 *   Copyright (C) 2008-2009 by Pino Toscano <pino@kde.org>                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "alternativemodels.h"
#include "altparser.h"

#include <QComboBox>
#include <QFile>
#include <QFont>
#include <QList>

#include <kiconloader.h>
#include <klocalizedstring.h>
#include <kprocess.h>

#include <algorithm>

#include <config-kalternatives.h>
#include <kalternatives_debug.h>

enum ItemChangeType
{
    SelectionItemChange = 1,
    AltNumItemChange = 2,
    ModeItemChange = 4
};
Q_DECLARE_FLAGS(ItemChanges, ItemChangeType)
Q_DECLARE_OPERATORS_FOR_FLAGS(ItemChanges)

namespace
{

struct AltNode
{
    enum { Type = 0 };

    AltNode(AltNode *pp, int t)
        : parent(pp), type(t) {}
    virtual ~AltNode()
        {}

    virtual QList<AltNode*> children() const
        { return QList<AltNode*>(); }
    virtual int childCount() const
        { return 0; }

    AltNode *parent;
    int type : 3;
};

struct AltAlternativeNode;
struct AltItemNode;

struct AltRootNode : public AltNode
{
    enum { Type = 1 };

    AltRootNode()
        : AltNode(Q_NULLPTR, Type) {}
    ~AltRootNode() Q_DECL_OVERRIDE
        { qDeleteAll(m_children); }

    QList<AltNode*> children() const Q_DECL_OVERRIDE
        {
            QList<AltNode*> c;
            std::copy(m_children.begin(), m_children.end(), std::back_inserter(c));
            return c;
        }
    int childCount() const Q_DECL_OVERRIDE
        { return m_children.count(); }

    QList<AltItemNode*> m_children;
};

struct AltItemNode : public AltNode
{
    enum { Type = 2 };

    AltItemNode(Item *i, AltRootNode *p)
        : AltNode(p, Type), item(i)
        , changed(false), nbrAltChanged(false), modeChanged(false)
    {}
    ~AltItemNode() Q_DECL_OVERRIDE
        { qDeleteAll(m_children); }

    QList<AltNode*> children() const Q_DECL_OVERRIDE
        {
            QList<AltNode*> c;
            std::copy(m_children.begin(), m_children.end(), std::back_inserter(c));
            return c;
        }
    int childCount() const Q_DECL_OVERRIDE
        { return m_children.count(); }

    Item *item;
    QList<AltAlternativeNode*> m_children;
    bool changed : 1;
    bool nbrAltChanged : 1;
    bool modeChanged : 1;
};

struct AltAlternativeNode : public AltNode
{
    enum { Type = 3 };

    AltAlternativeNode(Alternative *a, AltItemNode *p)
        : AltNode(p, Type), alternative(a), selected(false)
    {}

    Alternative *alternative;
    bool selected : 1;
};

template <class T>
T* altnode_cast(AltNode *n)
{
    return n->type > 0 && n->type == T::Type ? static_cast<T*>(n) : Q_NULLPTR;
}

template <>
AltNode* altnode_cast(AltNode *n)
{
    return n;
}

}

class AlternativesBaseModelPrivate
{
public:
    AlternativesBaseModelPrivate();
    virtual ~AlternativesBaseModelPrivate();

    Q_DECLARE_PUBLIC(AlternativesBaseModel)

    virtual void load() = 0;
    virtual AltNode* root() const = 0;
    QModelIndex indexForItem(AltNode *n, int col) const;

    AlternativesBaseModel *q_ptr;
};

AlternativesBaseModelPrivate::AlternativesBaseModelPrivate()
    : q_ptr(Q_NULLPTR)
{
}

AlternativesBaseModelPrivate::~AlternativesBaseModelPrivate()
{
}

QModelIndex AlternativesBaseModelPrivate::indexForItem(AltNode *n, int col) const
{
    if (n->parent)
    {
        const QList<AltNode*> children = n->parent->children();
        const int id = children.indexOf(n);
        if (id >= 0 && id < children.count())
           return q_ptr->createIndex(id, col, n);
    }
    return QModelIndex();
}


AlternativesBaseModel::AlternativesBaseModel(AlternativesBaseModelPrivate &dd, QObject *parent)
    : QAbstractItemModel(parent), d_ptr(&dd)
{
    d_ptr->q_ptr = this;
    d_ptr->load();
}

AlternativesBaseModel::~AlternativesBaseModel()
{
    delete d_ptr;
}

bool AlternativesBaseModel::hasChildren(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return true;

    AltNode *n = static_cast<AltNode *>(parent.internalPointer());
    return n->childCount() > 0;
}

QModelIndex AlternativesBaseModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column < 0 || column >= columnCount(parent))
        return QModelIndex();

    AltNode *n = parent.isValid() ? static_cast<AltNode *>(parent.internalPointer()) : d_ptr->root();
    const QList<AltNode*> children = n->children();
    if (row < children.count())
        return createIndex(row, column, children.at(row));

    return QModelIndex();
}

QModelIndex AlternativesBaseModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    AltNode *n = static_cast<AltNode *>(index.internalPointer());
    return d_ptr->indexForItem(n->parent, index.column());

}

int AlternativesBaseModel::rowCount(const QModelIndex &parent) const
{
    AltNode *n = parent.isValid() ? static_cast<AltNode *>(parent.internalPointer()) : d_ptr->root();
    return n->childCount();
}


class AlternativeItemsModelPrivate : public AlternativesBaseModelPrivate
{
public:
    AlternativeItemsModelPrivate(const QString &appName);
    ~AlternativeItemsModelPrivate();

    void load() Q_DECL_OVERRIDE;
    AltNode* root() const Q_DECL_OVERRIDE { return const_cast<AltRootNode *>(&m_root); }

    Q_DECLARE_PUBLIC(AlternativeItemsModel)

    void itemChanged(AltItemNode *node, ItemChanges changes);
    void loadItemNode(AltItemNode *node);

    bool isChanged(AltItemNode *node) const;
    AltAlternativeNode* findSelectedAlternative(AltItemNode *node, int *index) const;

    AltFilesManager *altManager;
    AltRootNode m_root;
    QString m_appName;
    KIconLoader *iconLoader;
    QIcon brokenAltIcon;
};

AlternativeItemsModelPrivate::AlternativeItemsModelPrivate(const QString &appName)
    : AlternativesBaseModelPrivate(), altManager(Q_NULLPTR)
    , m_appName(appName), iconLoader(new KIconLoader(m_appName))
    , brokenAltIcon(KDE::icon("alternative-broken", iconLoader))
{
#if defined(DISTRO_DPKG)
    altManager = new AltFilesManager("/var/lib/dpkg/alternatives");
#elif defined(DISTRO_RPM_2)
    altManager = new AltFilesManager("/var/lib/alternatives");
#elif defined(DISTRO_RPM)
    altManager = new AltFilesManager("/var/lib/rpm/alternatives");
#else
    qCritical(KALT_LOG) << "Unsupported distribution for KAlternatives.";
#endif
    if (altManager && !altManager->parsingOk())
    {
        qCDebug(KALT_LOG) << altManager->getErrorMsg();
        delete altManager;
        altManager = Q_NULLPTR;
    }
}

AlternativeItemsModelPrivate::~AlternativeItemsModelPrivate()
{
    delete altManager;
}

void AlternativeItemsModelPrivate::load()
{
    if (!altManager)
        return;

    ItemPtrList *itemslist = altManager->getGlobalAlternativeList();
    Q_FOREACH (Item *i, *itemslist)
    {
        AltItemNode *newItem = new AltItemNode(i, &m_root);
        m_root.m_children.append(newItem);
    }
}

void AlternativeItemsModelPrivate::itemChanged(AltItemNode *node, ItemChanges changes)
{
    Q_Q(AlternativeItemsModel);
    if (changes & SelectionItemChange)
    {
        node->changed = true;
    }
    if (changes & AltNumItemChange)
    {
        node->nbrAltChanged = true;
    }
    if (changes & ModeItemChange)
    {
        node->modeChanged = true;
    }
    const QModelIndex index = indexForItem(node, 0);
    emit q->dataChanged(index, index);
}

void AlternativeItemsModelPrivate::loadItemNode(AltItemNode *node)
{
    if (!node->m_children.isEmpty())
        return;

    AltsPtrList *alts = node->item->getAlternatives();
    Q_FOREACH (Alternative* a, *alts)
    {
        AltAlternativeNode *altnode = new AltAlternativeNode(a, node);
        node->m_children.append(altnode);
        altnode->selected = a->isSelected();
    }
}

bool AlternativeItemsModelPrivate::isChanged(AltItemNode *node) const
{
    return node->changed || node->nbrAltChanged || node->modeChanged;
}

AltAlternativeNode* AlternativeItemsModelPrivate::findSelectedAlternative(AltItemNode *node, int *index) const
{
    const int num = node->m_children.count();
    for (int i = 0; i < num; ++i)
    {
        AltAlternativeNode *altnode = node->m_children.at(i);
        if (altnode->selected)
        {
            *index = i;
            return altnode;
        }
    }
    *index = -1;
    return Q_NULLPTR;
}


AlternativeItemsModel::AlternativeItemsModel(const QString &appName, QObject *parent)
    : AlternativesBaseModel(*new AlternativeItemsModelPrivate(appName), parent)
{
    Q_D(AlternativeItemsModel);
    d->iconLoader->setParent(this);
}

AlternativeItemsModel::~AlternativeItemsModel()
{
}

int AlternativeItemsModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

QVariant AlternativeItemsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    Q_D(const AlternativeItemsModel);
    AltNode *n = static_cast<AltNode *>(index.internalPointer());
    if (AltItemNode *n_i = altnode_cast<AltItemNode>(n))
    {
        switch (role)
        {
            case Qt::DisplayRole:
                return n_i->item->getName();
            case Qt::ToolTipRole:
            {
                QString tip = n_i->item->getName();
                if (n_i->item->isBroken())
                {
                    tip += "\n\n";
                    tip += i18n("Broken alternative group.");
                }
                return tip;
            }
            case Qt::ForegroundRole:
                if (d->isChanged(n_i))
                    return QColor(Qt::red);
                break;
            case Qt::FontRole:
                if (d->isChanged(n_i))
                {
                    QFont f;
                    f.setBold(true);
                    return f;
                }
                break;
            case Qt::DecorationRole:
                if (n_i->item->isBroken())
                    return d->brokenAltIcon;
                break;
            case AltItemRole:
                return QVariant::fromValue(n_i->item);
        }
    }
    return QVariant();
}

Qt::ItemFlags AlternativeItemsModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    AltNode *n = static_cast<AltNode *>(index.internalPointer());
    if (AltItemNode *n_i = altnode_cast<AltItemNode>(n))
    {
        Qt::ItemFlags f = Qt::ItemIsSelectable;
        if (!n_i->item->isBroken())
            f |= Qt::ItemIsEnabled;
        return f;
    }
    return Qt::NoItemFlags;
}

bool AlternativeItemsModel::hasChildren(const QModelIndex &parent) const
{
    return parent.isValid() ? false : AlternativesBaseModel::hasChildren(parent);
}

QVariant AlternativeItemsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal)
        return QVariant();

    switch (role)
    {
        case Qt::DisplayRole:
            if (section == 0)
                return i18nc("Groups of alternatives", "Groups");
            break;
    }
    return QVariant();
}

QModelIndex AlternativeItemsModel::index(int row, int column, const QModelIndex &parent) const
{
    return parent.isValid() ? QModelIndex() : AlternativesBaseModel::index(row, column, parent);
}

int AlternativeItemsModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : AlternativesBaseModel::rowCount(parent);
}

void AlternativeItemsModel::save()
{
    Q_D(AlternativeItemsModel);
    if (!d->altManager)
        return;

    QModelIndexList changedIndexes;
    const int rows = d->m_root.m_children.count();
    for (int i = 0; i < rows; ++i)
    {
        AltItemNode *node = d->m_root.m_children.at(i);
        Item *item = node->item;
        bool itemChanged = false;

        if (node->changed)
        {
            int index = 0;
            AltAlternativeNode* altnode = d->findSelectedAlternative(node, &index);
            Q_ASSERT(altnode);
            QString selectError;
            if (!altnode->alternative->select(&selectError))
            {
                qCDebug(KALT_LOG) << selectError << Qt::endl;
                return;
            }
            node->changed = false;
            itemChanged = true;
        }
        if (node->nbrAltChanged || node->modeChanged)
        {
            QString parentPath = d->altManager->getAltDir() + '/' + item->getName();

            QFile origFile(parentPath);
            if (origFile.exists())
            {
                origFile.remove();
            }

            if (origFile.open(QIODevice::WriteOnly))
            {
                QTextStream stream(&origFile);

                stream << Item::modeString(item->getMode()) << Qt::endl;
                stream << item->getPath() << Qt::endl;

                SlaveList *slaveList = item->getSlaves();
                Q_FOREACH (Slave *slave, *slaveList)
                {
                    stream << slave->slname << Qt::endl;
                    stream << slave->slpath << Qt::endl;
                }

                stream << Qt::endl;

                AltsPtrList *altItemList = item->getAlternatives();
                Q_FOREACH (Alternative *a, *altItemList)
                {
                    stream << a->getPath() << Qt::endl;
                    stream << a->getPriority() << Qt::endl;

                    Q_FOREACH (const QString &slave, a->getSlaves())
                    {
                        stream << slave << Qt::endl;
                    }
                }

                stream << Qt::endl;

                origFile.close();
            }
            node->nbrAltChanged = false;
            node->modeChanged = false;
            itemChanged = true;
        }
        if (itemChanged)
        {
            changedIndexes.append(createIndex(i, 0, node));
        }
    }
    Q_FOREACH (const QModelIndex &index, changedIndexes)
    {
        emit dataChanged(index, index);
    }
}

bool AlternativeItemsModel::isSupported() const
{
    Q_D(const AlternativeItemsModel);
    return d->altManager;
}


class AlternativeAltModelPrivate : public AlternativesBaseModelPrivate
{
public:
    AlternativeAltModelPrivate(AlternativeItemsModel *itemModel, bool readOnly);
    ~AlternativeAltModelPrivate();

    void load() Q_DECL_OVERRIDE;
    AltNode* root() const Q_DECL_OVERRIDE { return m_root; }

    Q_DECLARE_PUBLIC(AlternativeAltModel)

    AltAlternativeNode* findHigherPriority(int *index) const;
    void searchDescription(Alternative *alternative) const;

    void statusChanged(int index);

    AlternativeItemsModelPrivate *parentModel;
    AltItemNode m_nullRoot;
    AltItemNode *m_root;
    bool m_readOnly;
};

AlternativeAltModelPrivate::AlternativeAltModelPrivate(AlternativeItemsModel *itemModel, bool readOnly)
    : AlternativesBaseModelPrivate()
    , parentModel(itemModel->d_func()), m_nullRoot(Q_NULLPTR, 0), m_root(&m_nullRoot), m_readOnly(readOnly)
{
}

AlternativeAltModelPrivate::~AlternativeAltModelPrivate()
{
}

void AlternativeAltModelPrivate::load()
{
}

AltAlternativeNode* AlternativeAltModelPrivate::findHigherPriority(int *index) const
{
    const int num = m_root->m_children.count();
    if (!num)
    {
        *index = 0;
        return Q_NULLPTR;
    }

    int id = 0;
    AltAlternativeNode* n = m_root->m_children.at(id);
    int priority = n->alternative->getPriority();
    for (int i = 1; i < num; ++i)
    {
        AltAlternativeNode* tmp = m_root->m_children.at(i);
        if (tmp->alternative->getPriority() > priority)
        {
            id = i;
            n = tmp;
            priority = n->alternative->getPriority();
        }
    }
    *index = id;
    return n;
}

static bool extractDescriptionFor(const QString &outputLine, const QString &name, QString *desc)
{
    QString output = outputLine;
    int pos = output.indexOf('(');
    // look for the name of the search result, and discard it
    // in case it is not exactly what we requested
    if (pos == -1 || (output.left(pos -1) != name))
        return false;

    pos = output.indexOf(']');
    if (pos != -1)
    {
        output.remove(0, pos + 1);
    }
    pos = output.indexOf(')');
    if (pos != -1)
    {
        output.remove(0, pos + 1);
    }
    pos = output.indexOf('-');
    if (pos != -1)
    {
        output.remove(0, pos + 2);
    }
    *desc = output;
    return true;
}

void AlternativeAltModelPrivate::searchDescription(Alternative *alternative) const
{
    QString exec = alternative->getPath();
    const int slashPos = exec.lastIndexOf('/');
    if (slashPos != -1)
        exec.remove(0, slashPos + 1);

    KProcess proc;
    proc.setProgram("whatis", QStringList() << exec);
    proc.setOutputChannelMode(KProcess::SeparateChannels);
    proc.setEnv("COLUMNS", QString::number(300));
    proc.start();
    proc.waitForStarted();
    proc.waitForFinished();
    if (proc.exitCode() == 0)
    {
        const QByteArray procOutput = proc.readAllStandardOutput();
        const QStringList outputLines = QString::fromLocal8Bit(procOutput.constData()).split('\n', Qt::SkipEmptyParts);
        Q_FOREACH (const QString &outLine, outputLines)
        {
            QString description;
            if (extractDescriptionFor(outLine, exec, &description))
            {
                alternative->setDescription(description);
                break;
            }
        }
    }
}

void AlternativeAltModelPrivate::statusChanged(int index)
{
    if (m_root == &m_nullRoot)
        return;

    Q_Q(AlternativeAltModel);
    QComboBox *combo = q->sender() ? qobject_cast<QComboBox *>(q->sender()) : Q_NULLPTR;
    if (!combo)
        return;

    const Item::ItemMode mode = static_cast<Item::ItemMode>(combo->itemData(index).toInt());
    m_root->item->setMode(mode);
    ItemChanges changes = ModeItemChange;
    if (mode == Item::AutoMode)
    {
        int selectedIndex = 0;
        int higherPriorityIndex = 0;
        AltAlternativeNode *selectedaltnode = parentModel->findSelectedAlternative(m_root, &selectedIndex);
        AltAlternativeNode *newaltnode = findHigherPriority(&higherPriorityIndex);
        if (selectedIndex != higherPriorityIndex)
        {
            QModelIndexList indexes;
            selectedaltnode->selected = false;
            indexes.append(q->createIndex(selectedIndex, 0, selectedaltnode));
            newaltnode->selected = true;
            indexes.append(q->createIndex(higherPriorityIndex, 0, newaltnode));
            m_root->changed = true;
            Q_FOREACH (const QModelIndex &index, indexes)
            {
                emit q->dataChanged(index, index);
            }
            changes |= SelectionItemChange;
        }
    }
    parentModel->itemChanged(m_root, changes);
}


AlternativeAltModel::AlternativeAltModel(AlternativeItemsModel *itemModel, bool readOnly, QObject *parent)
    : AlternativesBaseModel(*new AlternativeAltModelPrivate(itemModel, readOnly), parent)
{
}

AlternativeAltModel::~AlternativeAltModel()
{
}

int AlternativeAltModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 3;
}

QVariant AlternativeAltModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    Q_D(const AlternativeAltModel);
    AltNode *n = static_cast<AltNode *>(index.internalPointer());
    if (AltAlternativeNode *n_a = altnode_cast<AltAlternativeNode>(n))
    {
        switch (role)
        {
            case Qt::DisplayRole:
            {
                switch (index.column())
                {
                    case 0:
                        return n_a->alternative->getPath();
                    case 1:
                        return n_a->alternative->getPriority();
                    case 2:
                        if (n_a->alternative->getDescription().isEmpty())
                            d->searchDescription(n_a->alternative);
                        return Alternative::prettyDescription(n_a->alternative);
                }
                break;
            }
            case Qt::ToolTipRole:
            {
                if (n_a->alternative->getDescription().isEmpty())
                    d->searchDescription(n_a->alternative);
                KLocalizedString tip = n_a->alternative->isBroken()
                                     ? ki18nc("%1 is the alternative path, %2 its description",
                                              "%1\n(broken)\n\n%2")
                                     : ki18nc("%1 is the alternative path, %2 its description",
                                              "%1\n\n%2");
                return tip.subs(n_a->alternative->getPath())
                          .subs(Alternative::prettyDescription(n_a->alternative))
                          .toString();
            }
            case Qt::EditRole:
                if (index.column() == 0)
                    return n_a->selected;
                break;
            case Qt::CheckStateRole:
                if (index.column() == 0)
                    return n_a->selected ? Qt::Checked : Qt::Unchecked;
                break;
            case Qt::DecorationRole:
                if (index.column() == 0 && n_a->alternative->isBroken())
                    return d->parentModel->brokenAltIcon;
                break;
            case AltAlternativeRole:
                return QVariant::fromValue(n_a->alternative);
        }
    }
    return QVariant();
}

Qt::ItemFlags AlternativeAltModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    Q_D(const AlternativeAltModel);
    AltNode *n = static_cast<AltNode *>(index.internalPointer());
    if (AltAlternativeNode *n_a = altnode_cast<AltAlternativeNode>(n))
    {
        Qt::ItemFlags f = Qt::ItemIsSelectable;
        if (!n_a->alternative->isBroken())
            f |= Qt::ItemIsEnabled;
        switch (index.column())
        {
            case 0:
                return f | Qt::ItemIsUserCheckable;
            default:
                return f;
        }
    }
    return Qt::NoItemFlags;
}

QVariant AlternativeAltModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal)
        return QVariant();

    switch (role)
    {
        case Qt::DisplayRole:
            switch (section)
            {
                case 0:
                    return i18n("Option");
                case 1:
                    return i18n("Priority");
                case 2:
                    return i18n("Description");
            }
            break;
    }
    return QVariant();
}

QModelIndex AlternativeAltModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

bool AlternativeAltModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    AltAlternativeNode *n = altnode_cast<AltAlternativeNode>(static_cast<AltNode *>(index.internalPointer()));
    if (!n)
        return false;

    Q_D(AlternativeAltModel);
    switch (role)
    {
        case Qt::CheckStateRole:
        {
            if (d->m_readOnly || n->alternative->isBroken())
                break;

            const bool newValue = value.toBool();
            if (newValue)
            {
                if (!n->selected)
                {
                    QModelIndexList changedIndexes;
                    Q_FOREACH (AltAlternativeNode* node, d->m_root->m_children)
                    {
                        if (node->selected)
                        {
                            node->selected = false;
                            changedIndexes.append(d->indexForItem(node, index.column()));
                        }
                    }
                    n->selected = true;
                    changedIndexes.append(index);
                    Q_FOREACH (const QModelIndex &changedIndex, changedIndexes)
                    {
                        emit dataChanged(changedIndex, changedIndex);
                    }
                    ItemChanges changes = SelectionItemChange;
                    // when changing option, set the alternative to "manual" mode
                    if (d->m_root->item->getMode() == Item::AutoMode)
                    {
                        d->m_root->item->setMode(Item::ManualMode);
                        changes |= ModeItemChange;
                    }
                    d->parentModel->itemChanged(d->m_root, changes);
                    return true;
                }
            }
            break;
        }
    }
    return false;
}

void AlternativeAltModel::setItem(Item *item)
{
    Q_D(AlternativeAltModel);
    beginResetModel();
    d->m_root = &d->m_nullRoot;
    Q_FOREACH (AltItemNode *n, d->parentModel->m_root.m_children)
    {
        if (n->item == item)
        {
            d->m_root = n;
            break;
        }
    }
    if (d->m_root->item)
    {
        d->parentModel->loadItemNode(d->m_root);
    }
    endResetModel();
}

void AlternativeAltModel::addAlternative(Alternative *alt)
{
    if (!alt)
        return;

    Q_D(AlternativeAltModel);
    if (alt->getParent() != d->m_root->item)
        return;

    const int childCount = d->m_root->m_children.count();
    beginInsertRows(QModelIndex(), childCount, childCount);
    d->m_root->item->addAlternative(alt);
    d->m_root->m_children.append(new AltAlternativeNode(alt, d->m_root));
    endInsertRows();
    d->parentModel->itemChanged(d->m_root, AltNumItemChange);
}

void AlternativeAltModel::removeAlternative(Alternative *alt)
{
    if (!alt)
        return;

    Q_D(AlternativeAltModel);
    if (alt->getParent() != d->m_root->item)
        return;

    int altId = 0;
    const int childCount = d->m_root->m_children.count();
    for ( ; altId < childCount; ++altId)
    {
        if (d->m_root->m_children.at(altId)->alternative == alt)
            break;
    }
    if (altId == childCount)
        return;

    const bool wasSelected = d->m_root->m_children.at(altId)->selected;
    beginRemoveRows(QModelIndex(), altId, altId);
    d->m_root->item->delAlternativeByPath(alt->getPath());
    delete d->m_root->m_children.at(altId);
    d->m_root->m_children.removeAt(altId);
    endRemoveRows();
    if (wasSelected && !d->m_root->m_children.isEmpty())
    {
        int row = 0;
        AltAlternativeNode *node = d->findHigherPriority(&row);
        node->selected = true;
        const QModelIndex changedIndex = createIndex(row, 0, node);
        emit dataChanged(changedIndex, changedIndex);
    }
    d->parentModel->itemChanged(d->m_root, AltNumItemChange);
}


AlternativeItemProxyModel::AlternativeItemProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent), m_showSingle(false)
{
}

AlternativeItemProxyModel::~AlternativeItemProxyModel()
{
}

void AlternativeItemProxyModel::setShowSingleAlternative(bool show)
{
    if (show == m_showSingle)
        return;

    m_showSingle = show;
    invalidateFilter();
}

bool AlternativeItemProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    Item *item = sourceModel()->index(source_row, 0, source_parent).data(AltItemRole).value<Item *>();
    return !item || m_showSingle || item->countAlternatives() > 1;
}

#include <moc_alternativemodels.cpp>
