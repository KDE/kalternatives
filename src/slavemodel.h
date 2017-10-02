/***************************************************************************
 *   Copyright (C) 2008 by Pino Toscano <pino@kde.org>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef SLAVEMODEL_H
#define SLAVEMODEL_H

#include <QAbstractItemModel>

class Alternative;
class Item;

class SlaveModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    SlaveModel(QObject *parent = Q_NULLPTR);
    ~SlaveModel() Q_DECL_OVERRIDE;

    // QAbstractItemModel interface
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

    void setItem(Item *item);
    void setAlternative(Alternative *alt);

protected:
    Item *m_item;
    Alternative *m_alt;
};

#endif
