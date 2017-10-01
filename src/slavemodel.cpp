/***************************************************************************
 *   Copyright (C) 2008 by Pino Toscano <pino@kde.org>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "slavemodel.h"
#include "altparser.h"

#include <kdebug.h>
#include <klocalizedstring.h>

SlaveModel::SlaveModel(QObject *parent)
    : QAbstractItemModel(parent)
    , m_item(0), m_alt(0)
{
}

SlaveModel::~SlaveModel()
{
}

int SlaveModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 2;
}

QVariant SlaveModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || !m_item)
        return QVariant();

    switch (role)
    {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
            if (index.column() == 0)
                return m_item->getSlaves()->at(index.row())->slname;
            else if (index.column() == 1)
                return m_alt ? m_alt->getSlaves().at(index.row()) : QString();
            break;
    }
    return QVariant();
}

bool SlaveModel::hasChildren(const QModelIndex &parent) const
{
    return rowCount(parent) > 0;
}

QVariant SlaveModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal)
    {
        switch (role)
        {
            case Qt::DisplayRole:
                if (section == 0)
                    return i18nc("Slave name", "Name");
                else if (section == 1)
                    return i18nc("Slave path", "Path");
                break;
        }
    }
    else
    {
        if (role == Qt::DisplayRole)
            return section;
    }
    return QVariant();
}

QModelIndex SlaveModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() || !m_item || column < 0 || column >= 2
        || row < 0 || (row >= m_item->getSlaves()->count()))
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex SlaveModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}

int SlaveModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : (m_item ? m_item->getSlaves()->count(): 0);
}

void SlaveModel::setItem(Item *item)
{
    if (item == m_item)
        return;

    m_item = item;
    m_alt = 0;
    reset();
}

void SlaveModel::setAlternative(Alternative *alt)
{
    if (!alt || !m_item || (alt->getParent() != m_item)
        || (alt->slavesCount() != m_item->getSlaves()->count()))
        return;

    m_alt = alt;
    emit dataChanged(createIndex(0, 1), createIndex(m_alt->slavesCount() - 1, 1));
}

#include <slavemodel.moc>
