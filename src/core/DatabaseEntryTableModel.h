#ifndef DATABASEENTRYTABLEMODEL_H
#define DATABASEENTRYTABLEMODEL_H

#include "Database.h"
#include <QAbstractTableModel>
#include <QObject>

enum DatabaseEntryModelColumns {
    Title = 0,
    CreatedAt = 1,
    ModifiedAt = 2,
    Uid = 3,
    ColumnCount
};

class DatabaseEntryTableModel : public QAbstractTableModel
{
public:
    explicit DatabaseEntryTableModel(QObject *parent = nullptr);
    void setDatabase(Database* database);
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    void addEntry(const QUuid& group, const QString& title, const QString& notes, const SecureQByteArray& password);
    void removeEntry(const QUuid& uid);

private:
    Database* _database = nullptr;
};

#endif // DATABASEENTRYTABLEMODEL_H
