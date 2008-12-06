/***************************************************************************
 *   Copyright (C) 2008 by Pino Toscano <pino@kde.org>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef _ALTERNATIVEMODELS_H_
#define _ALTERNATIVEMODELS_H_

#include <QSortFilterProxyModel>
#include <QVariant>

class Alternative;
class AltFilesManager;
class Item;
class AlternativesBaseModelPrivate;
class AlternativeItemsModelPrivate;
class AlternativeAltModelPrivate;

enum
{
    AltItemRole = 0x00ff0001,
    AltAlternativeRole = 0x00ff0002
};

class AlternativesBaseModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    ~AlternativesBaseModel();

    // QAbstractItemModel interface
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

protected:
    AlternativesBaseModel(AlternativesBaseModelPrivate &dd, QObject *parent);
    Q_DECLARE_PRIVATE(AlternativesBaseModel)
    AlternativesBaseModelPrivate *d_ptr;
};


class AlternativeItemsModel : public AlternativesBaseModel
{
    Q_OBJECT

    friend class AlternativeAltModelPrivate;
public:
    AlternativeItemsModel(AltFilesManager *manager, QObject *parent = 0);
    ~AlternativeItemsModel();

    // QAbstractItemModel interface
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    void save();

private:
    Q_DECLARE_PRIVATE(AlternativeItemsModel)
};


class AlternativeAltModel : public AlternativesBaseModel
{
    Q_OBJECT
public:
    enum ItemChangeType
    {
        SelectionItemChange,
        AltNumItemChange,
    };

    AlternativeAltModel(AlternativeItemsModel *itemModel, bool readOnly, QObject *parent = 0);
    ~AlternativeAltModel();

    // QAbstractItemModel interface
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QModelIndex parent(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    void setItem(Item *item);
    void addAlternative(Alternative *alt);
    void removeAlternative(Alternative *alt);

private:
    Q_DECLARE_PRIVATE(AlternativeAltModel)
};


class AlternativeItemProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    AlternativeItemProxyModel(QObject *parent = 0);
    ~AlternativeItemProxyModel();

    void setShowSingleAlternative(bool show);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

private:
    bool m_showSingle;
};

Q_DECLARE_METATYPE(Alternative*)
Q_DECLARE_METATYPE(Item*)

#endif
