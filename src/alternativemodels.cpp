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

#include <QComboBox>
#include <QFile>
#include <QFont>
#include <QList>

#include <kdebug.h>
#include <klocale.h>
#include <kprocess.h>

#include <algorithm>

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
        : AltNode(0, Type) {}
    virtual ~AltRootNode()
        { qDeleteAll(m_children); }

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
        : AltNode(p, Type), item(i)
        , changed(false), nbrAltChanged(false), modeChanged(false)
    {}
    virtual ~AltItemNode()
        { qDeleteAll(m_children); }

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

    void itemChanged(AltItemNode *node, ItemChanges changes);
    void loadItemNode(AltItemNode *node);

    bool isChanged(AltItemNode *node) const;
    AltAlternativeNode* findSelectedAlternative(AltItemNode *node, int *index) const;

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
    return 0;
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

    Q_D(const AlternativeItemsModel);
    AltNode *n = static_cast<AltNode *>(index.internalPointer());
    if (AltItemNode *n_i = altnode_cast<AltItemNode>(n))
    {
        switch (role)
        {
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
                return n_i->item->getName();
            case Qt::ForegroundRole:
                if (d->isChanged(n_i))
                    return Qt::red;
                break;
            case Qt::FontRole:
                if (d->isChanged(n_i))
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
                return i18n("Alternatives");
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
            if (!altnode->alternative->select())
            {
                kDebug() << altnode->alternative->getSelectError() << endl;
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

                stream << endl;

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


class AlternativeAltModelPrivate : public AlternativesBaseModelPrivate
{
public:
    AlternativeAltModelPrivate(AlternativeItemsModel *itemModel, bool readOnly);
    ~AlternativeAltModelPrivate();

    virtual void load();
    virtual AltNode* root() const { return m_root; }

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
    , parentModel(itemModel->d_func()), m_nullRoot(0, 0), m_root(&m_nullRoot), m_readOnly(readOnly)
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
        return 0;
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

void AlternativeAltModelPrivate::statusChanged(int index)
{
    if (m_root == &m_nullRoot)
        return;

    Q_Q(AlternativeAltModel);
    QComboBox *combo = q->sender() ? qobject_cast<QComboBox *>(q->sender()) : 0;
    if (!combo)
        return;

    const QString mode = combo->itemData(index, Qt::DisplayRole).toString(); // ### use a better id
    m_root->item->setMode(mode);
    ItemChanges changes = ModeItemChange;
    if (mode == "auto")
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
                    if (d->m_root->item->getMode() == "auto")
                    {
                        d->m_root->item->setMode("manual");
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
    reset();
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

#include "alternativemodels.moc"
