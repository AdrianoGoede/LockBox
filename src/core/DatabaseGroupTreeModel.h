#ifndef DATABASEGROUPTREEMODEL_H
#define DATABASEGROUPTREEMODEL_H

#include <QAbstractItemModel>
#include "Database.h"

class DatabaseGroupTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit DatabaseGroupTreeModel(QObject* parent = nullptr);
    void setDatabase(Database* database);
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    void addGroup(const DatabaseGroup& group);
    void editGroup(const DatabaseGroup& group);
    void removeGroup(const QUuid& uid);

private:
    Database* _database = nullptr;
};

#endif // DATABASEGROUPTREEMODEL_H
