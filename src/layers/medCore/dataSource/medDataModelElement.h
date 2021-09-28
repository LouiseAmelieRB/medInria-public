#pragma once
/*=========================================================================

 medInria

 Copyright (c) INRIA 2013 - 2021. All rights reserved.
 See LICENSE.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include <QAbstractItemModel>

#include <medDataModelItem.h>

#include <medCoreExport.h>

class medDataModel;
struct medDataModelElementPrivate;

class MEDCORE_EXPORT medDataModelElement : public QAbstractItemModel
{

    Q_OBJECT

public:	
    medDataModelElement(medDataModel *parent, QString const & sourceIntanceId);
    virtual ~medDataModelElement();

    // ////////////////////////////////////////////////////////////////////////
    // Pure Virtual Override
    QVariant	data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QModelIndex	index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex	parent(const QModelIndex &index) const override;

    int	columnCount(const QModelIndex &parent = QModelIndex()) const override;
    int	rowCount(const QModelIndex &parent = QModelIndex()) const override;


    // ////////////////////////////////////////////////////////////////////////
    // Simple Virtual Override
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;


    bool canFetchMore(const QModelIndex& parent) const override;
    void fetchMore(const QModelIndex& parent) override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;


    // ////////////////////////////////////////////////////////////////////////
    // Simple methods
    //void setColumnAttributes(int p_iLevel, QStringList &attributes); //maybe developed because not const ?
    int  getColumnInsideLevel(int level, int section);
    bool fetch(QString uri);


public slots:
    void itemPressed(QModelIndex const &index);



private:    
    medDataModelItem* getItem(const QModelIndex &index) const;
    QModelIndex getIndex(QString iid, QModelIndex const &parent = QModelIndex()) const;
    bool fetchColumnNames(const QModelIndex &index);
    void populateLevel(QModelIndex const &index, QString const &key);
    void populateLevelV2(QModelIndex const &index, QString const & uri);
    void addRowRanges(QMap<int, QVariantList> &entriesToAdd, const QModelIndex & index);
    void computeRowRangesToAdd(QVariantList &entries, medDataModelItem * pItem, QMap<int, QVariantList> &entriesToAdd);
    void removeRowRanges(QVector<QPair<int, int>> &rangeToRemove, const QModelIndex & index);
    void computeRowRangesToRemove(medDataModelItem * pItem, QVariantList &entries, QVector<QPair<int, int>> &rangeToRemove);
    bool currentLevelFetchable(medDataModelItem * pItemCurrent);
    //bool hasChildItemWithIID(medDataModelItem *pi_Item, QString iid);

    bool itemStillExist(QVariantList &entries, medDataModelItem * pItem);


signals:


private:
    medDataModelElementPrivate* d;


};