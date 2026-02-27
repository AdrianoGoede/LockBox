#ifndef DATABASEGROUPTREEMODEL_H
#define DATABASEGROUPTREEMODEL_H

#include <QAbstractItemModel>
#include <QMimeData>
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
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    Qt::DropActions supportedDropActions() const override;
    Qt::DropActions supportedDragActions() const override;
    QStringList mimeTypes() const override;
    QMimeData* mimeData(const QModelIndexList &indexes) const override;
    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;

private slots:
    void groupAdded(qsizetype row, QUuid groupUuid);
    void groupEdited(qsizetype row, QUuid groupUuid);
    void groupRemoved(qsizetype row, QUuid groupUuid);

private:
    Database* _database = nullptr;
};

#endif // DATABASEGROUPTREEMODEL_H
