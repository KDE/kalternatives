/***************************************************************************
 *   Copyright (C) 2008 by Pino Toscano <pino@kde.org>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "alternativemodels.h"
#include "altparser.h"

#include <QFile>
#include <QFont>
#include <QList>

#include <kdebug.h>
#include <klocale.h>
#include <kprocess.h>

#include <algorithm>

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
        : AltNode(0, Type) {}

    virtual QList<AltNode*> children() const
        {
            QList<AltNode*> c;
            std::copy(m_children.begin(), m_children.end(), std::back_inserter(c));
            return c;
        }
    virtual int childCount() const
        { return m_children.count(); }

    QList<AltItemNode*> m_children;
};

struct AltItemNode : public AltNode
{
    enum { Type = 2 };

    AltItemNode(Item *i, AltRootNode *p)
        : AltNode(p, Type), item(i), changed(false), nbrAltChanged(false) {}

    virtual QList<AltNode*> children() const
        {
            QList<AltNode*> c;
            std::copy(m_children.begin(), m_children.end(), std::back_inserter(c));
            return c;
        }
    virtual int childCount() const
        { return m_children.count(); }

    Item *item;
    QList<AltAlternativeNode*> m_children;
    bool changed : 1;
    bool nbrAltChanged : 1;
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
    return n->type > 0 && n->type == T::Type ? static_cast<T*>(n) : 0;
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
    : q_ptr(0)
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
    AlternativeItemsModelPrivate(AltFilesManager *manager);

    virtual void load();
    virtual AltNode* root() const { return const_cast<AltRootNode *>(&m_root); }

    Q_DECLARE_PUBLIC(AlternativeItemsModel)

    // slots
    void itemChanged(Item *item, int);

    AltFilesManager *altManager;
    AltRootNode m_root;
};

AlternativeItemsModelPrivate::AlternativeItemsModelPrivate(AltFilesManager *manager)
    : AlternativesBaseModelPrivate(), altManager(manager)
{
}

void AlternativeItemsModelPrivate::load()
{
    Q3PtrList<Item> *itemslist = altManager->getGlobalAlternativeList();
    for (Item *i = itemslist->first(); i; i = itemslist->next())
    {
        AltItemNode *newItem = new AltItemNode(i, &m_root);
        m_root.m_children.append(newItem);
    }
}

void AlternativeItemsModelPrivate::itemChanged(Item *item, int change)
{
    Q_Q(AlternativeItemsModel);
    AlternativeAltModel::ItemChangeType changeType = (AlternativeAltModel::ItemChangeType)change;
    Q_FOREACH (AltItemNode *node, m_root.m_children)
    {
        if (node->item == item)
        {
            switch (changeType)
            {
                case AlternativeAltModel::SelectionItemChange:
                    node->changed = true;
                    break;
                case AlternativeAltModel::AltNumItemChange:
                    node->nbrAltChanged = true;
                    break;
            }
            const QModelIndex index = indexForItem(node, 0);
            emit q->dataChanged(index, index);
            break;
        }
    }
}


AlternativeItemsModel::AlternativeItemsModel(AltFilesManager *manager, QObject *parent)
    : AlternativesBaseModel(*new AlternativeItemsModelPrivate(manager), parent)
{
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

    AltNode *n = static_cast<AltNode *>(index.internalPointer());
    if (AltItemNode *n_i = altnode_cast<AltItemNode>(n))
    {
        switch (role)
        {
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
                return n_i->item->getName();
            case Qt::ForegroundRole:
                if (n_i->changed || n_i->nbrAltChanged)
                    return Qt::red;
                break;
            case Qt::FontRole:
                if (n_i->changed || n_i->nbrAltChanged)
                {
                    QFont f;
                    f.setBold(true);
                    return f;
                }
                break;
            case AltItemRole:
                return qVariantFromValue(n_i->item);
        }
    }
    return QVariant();
}

QVariant AlternativeItemsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal)
        return QVariant();

    switch (role)
    {
        case Qt::DisplayRole:
            if (section == 0)
                return i18n("Alternatives");
            break;
    }
    return QVariant();
}

void AlternativeItemsModel::save()
{
#ifdef __GNUC__
#warning fully implement saving
#endif
    Q_D(AlternativeItemsModel);
    QModelIndexList changedIndexes;
    const int rows = d->m_root.m_children.count();
    for (int i = 0; i < rows; ++i)
    {
        AltItemNode *node = d->m_root.m_children.at(i);
        Item *item = node->item;
        bool itemChanged = false;

        if (node->nbrAltChanged)
        {
#if 0
            QString parentPath = d->altManager->getAltDir() + '/' + item->getName();

            QFile origFile(parentPath);
            if (origFile.exists())
            {
                origFile.remove();
            }

            if (origFile.open(QIODevice::WriteOnly))
            {
                QTextStream stream(&origFile);

                stream << item->getMode() << endl;
                stream << item->getPath() << endl;

                SlaveList *slaveList = item->getSlaves();
                Slave *slave = slaveList->first();
                for (; slave; slave = slaveList->next())
                {
                    stream << slave->slname << endl;
                    stream << slave->slpath << endl;
                }

                stream << endl;

                AltsPtrList *altItemList = item->getAlternatives();
                Q_FOREACH (Alternative *a, *altItemList)
                {
                    stream << a->getPath() << endl;
                    stream << a->getPriority() << endl;

                    QStringList *slaveList = a->getSlaves();
                    QStringList::Iterator it = slaveList->begin();
                    for ( ; it != slaveList->end(); ++it )
                    {
                        stream << *it << endl;
                    }
                }
                origFile.close();
            }
#endif
            node->nbrAltChanged = false;
            itemChanged = true;
        }
        if (node->changed)
        {
            AltsPtrList *altItemList = item->getAlternatives();
            Q_FOREACH (Alternative *a, *altItemList)
            {
#if 0
                if (altItem->isOn())
                {
                    if (!a->select())
                    {
                        kDebug() << a->getSelectError() << endl;
                    }
                }
#endif
            }
            node->changed = false;
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


class AlternativeAltModelPrivate : public AlternativesBaseModelPrivate
{
public:
    AlternativeAltModelPrivate(AltFilesManager *manager, bool readOnly);
    ~AlternativeAltModelPrivate();

    virtual void load();
    virtual AltNode* root() const { return const_cast<AltItemNode *>(&m_root); }

    AltAlternativeNode* findHigherPriority(int *index) const;
    void searchDescription(Alternative *alternative) const;

    AltFilesManager *altManager;
    AltItemNode m_root;
    bool m_readOnly;
};

AlternativeAltModelPrivate::AlternativeAltModelPrivate(AltFilesManager *manager, bool readOnly)
    : AlternativesBaseModelPrivate(), altManager(manager), m_root(0, 0), m_readOnly(readOnly)
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
    const int num = m_root.m_children.count();
    if (!num)
    {
        *index = 0;
        return 0;
    }

    int id = 0;
    AltAlternativeNode* n = m_root.m_children.at(id);
    int priority = n->alternative->getPriority();
    for (int i = 1; i < num; ++i)
    {
        AltAlternativeNode* tmp = m_root.m_children.at(i);
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
        QString output = QString::fromLatin1(procOutput.constData(), procOutput.count());
        int pos = output.indexOf('\n');
        if (pos != -1)
        {
            output.truncate(pos);
        }
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
        alternative->setDescription(output);
    }
}


AlternativeAltModel::AlternativeAltModel(AltFilesManager *manager, bool readOnly, QObject *parent)
    : AlternativesBaseModel(*new AlternativeAltModelPrivate(manager, readOnly), parent)
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
            case Qt::ToolTipRole:
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
            case Qt::EditRole:
                if (index.column() == 0)
                    return n_a->selected;
                break;
            case Qt::CheckStateRole:
                if (index.column() == 0)
                    return n_a->selected ? Qt::Checked : Qt::Unchecked;
                break;
            case AltAlternativeRole:
                return qVariantFromValue(n_a->alternative);
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
        Q_UNUSED(n_a)
        Qt::ItemFlags f = Qt::ItemIsSelectable;
        if (!d->m_readOnly && !n_a->alternative->isBroken())
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
                    Q_FOREACH (AltAlternativeNode* node, d->m_root.m_children)
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
                    emit itemChanged(d->m_root.item, SelectionItemChange);
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
    qDeleteAll(d->m_root.m_children);
    d->m_root.m_children.clear();
    d->m_root.item = item;
    if (d->m_root.item)
    {
        AltsPtrList *alts = d->m_root.item->getAlternatives();
        Q_FOREACH (Alternative* a, *alts)
        {
            AltAlternativeNode *node = new AltAlternativeNode(a, &d->m_root);
            d->m_root.m_children.append(node);
            node->selected = a->isSelected();
        }
    }
    reset();
}

void AlternativeAltModel::addAlternative(Alternative *alt)
{
    if (!alt)
        return;

    Q_D(AlternativeAltModel);
    if (alt->getParent() != d->m_root.item)
        return;

    const int childCount = d->m_root.m_children.count();
    beginInsertRows(QModelIndex(), childCount, childCount);
    d->m_root.item->addAlternative(alt);
    d->m_root.m_children.append(new AltAlternativeNode(alt, &d->m_root));
    endInsertRows();
    emit itemChanged(d->m_root.item, AltNumItemChange);
}

void AlternativeAltModel::removeAlternative(Alternative *alt)
{
    if (!alt)
        return;

    Q_D(AlternativeAltModel);
    if (alt->getParent() != d->m_root.item)
        return;

    int altId = 0;
    const int childCount = d->m_root.m_children.count();
    for ( ; altId < childCount; ++altId)
    {
        if (d->m_root.m_children.at(altId)->alternative == alt)
            break;
    }
    if (altId == childCount)
        return;

    const bool wasSelected = d->m_root.m_children.at(altId)->selected;
    beginRemoveRows(QModelIndex(), altId, altId);
    d->m_root.item->delAlternativeByPath(alt->getPath());
    delete d->m_root.m_children.at(altId);
    d->m_root.m_children.removeAt(altId);
    endRemoveRows();
    if (wasSelected && !d->m_root.m_children.isEmpty())
    {
        int row = 0;
        AltAlternativeNode *node = d->findHigherPriority(&row);
        node->selected = true;
        const QModelIndex changedIndex = createIndex(row, 0, node);
        emit dataChanged(changedIndex, changedIndex);
    }
    emit itemChanged(d->m_root.item, AltNumItemChange);
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

#include "alternativemodels.moc"
