#ifndef DATABASEENTRYTABLEMODEL_H
#define DATABASEENTRYTABLEMODEL_H

#include "Database.h"
#include <QAbstractTableModel>
#include <QObject>
#include <QMimeData>

enum DatabaseEntryModelColumns {
    Title = 0,
    ModifiedAt = 1,
    Manage = 2,
    CopyUsername = 3,
    CopyPassword = 4,
    PerformAutotype = 5,
    ColumnCount
};

class DatabaseEntryTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit DatabaseEntryTableModel(QObject *parent = nullptr);
    const Database* database() const;
    void setDatabase(const Database* database);
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    Qt::DropActions supportedDragActions() const override;
    QStringList mimeTypes() const override;
    QMimeData* mimeData(const QModelIndexList& indexes) const override;

private slots:
    void entryAdded(qsizetype row, QUuid entryUuid);
    void entryEdited(qsizetype row, QUuid entryUuid);
    void entryRemoved(qsizetype row, QUuid entryUuid);

private:
    const Database* _database = nullptr;
};

#endif // DATABASEENTRYTABLEMODEL_H
