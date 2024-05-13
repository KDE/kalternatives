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
    ~AlternativesBaseModel() Q_DECL_OVERRIDE;

    // QAbstractItemModel interface
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

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
    AlternativeItemsModel(const QString &appName, QObject *parent = Q_NULLPTR);
    ~AlternativeItemsModel() Q_DECL_OVERRIDE;

    // QAbstractItemModel interface
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

    void save();
    bool isSupported() const;

private:
    Q_DECLARE_PRIVATE(AlternativeItemsModel)
};


class AlternativeAltModel : public AlternativesBaseModel
{
    Q_OBJECT
public:
    AlternativeAltModel(AlternativeItemsModel *itemModel, bool readOnly, QObject *parent = Q_NULLPTR);
    ~AlternativeAltModel() Q_DECL_OVERRIDE;

    // QAbstractItemModel interface
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) Q_DECL_OVERRIDE;

    void setItem(Item *item);
    void addAlternative(Alternative *alt);
    void removeAlternative(Alternative *alt);

private:
    Q_DECLARE_PRIVATE(AlternativeAltModel)
    Q_PRIVATE_SLOT(d_func(), void statusChanged(int))
};


class AlternativeItemProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    AlternativeItemProxyModel(QObject *parent = Q_NULLPTR);
    ~AlternativeItemProxyModel() Q_DECL_OVERRIDE;

    void setShowSingleAlternative(bool show);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const Q_DECL_OVERRIDE;

private:
    bool m_showSingle;
};

#endif
