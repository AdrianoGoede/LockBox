#include "DatabaseGroupTreeModel.h"
#include <QIcon>

DatabaseGroupTreeModel::DatabaseGroupTreeModel(QObject* parent) : QAbstractItemModel(parent) {}

void DatabaseGroupTreeModel::setDatabase(Database* database)
{
    beginResetModel();

    if (_database)
        disconnect(_database, nullptr, this, nullptr);
    _database = database;

    if (_database) {
        connect(_database, &Database::groupAdded, this, &DatabaseGroupTreeModel::groupAdded);
        connect(_database, &Database::groupEdited, this, &DatabaseGroupTreeModel::groupEdited);
        connect(_database, &Database::groupRemoved, this, &DatabaseGroupTreeModel::groupRemoved);
    }

    endResetModel();
}

QModelIndex DatabaseGroupTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!_database || !hasIndex(row, column, parent))
        return {};

    const DatabaseGroup* parentGroup = (parent.isValid() ? static_cast<const DatabaseGroup*>(parent.internalPointer()) : nullptr);
    QVector<const DatabaseGroup*> children = _database->childrenOfGroup(parentGroup);
    if (row >= children.size())
        return {};

    return createIndex(row, column, const_cast<DatabaseGroup*>(children.at(row)));
}

QModelIndex DatabaseGroupTreeModel::parent(const QModelIndex& index) const
{
    if (!_database || !index.isValid())
        return {};

    const DatabaseGroup* childGroup = static_cast<const DatabaseGroup*>(index.internalPointer());
    QUuid parentUuid = childGroup->parent();
    if (parentUuid.isNull())
        return {};

    const DatabaseGroup& parentGroup = _database->group(parentUuid);
    QUuid grandParentUuid = parentGroup.parent();
    const DatabaseGroup* grandParentGroup = (!grandParentUuid.isNull() ? &_database->group(grandParentUuid) : nullptr);
    QVector<const DatabaseGroup*> siblings = _database->childrenOfGroup(grandParentGroup);
    int row = siblings.indexOf(&parentGroup);

    return (row < 0 ? QModelIndex() : createIndex(
        row,
        0,
        const_cast<DatabaseGroup*>(&parentGroup)
    ));
}

int DatabaseGroupTreeModel::rowCount(const QModelIndex& parent) const
{
    if (!_database) return 0;
    const DatabaseGroup* parentGroup = (parent.isValid() ? static_cast<const DatabaseGroup*>(parent.internalPointer()) : nullptr);
    return _database->childrenOfGroup(parentGroup).size();
}

int DatabaseGroupTreeModel::columnCount(const QModelIndex& parent) const { return 1; }

QVariant DatabaseGroupTreeModel::data(const QModelIndex& index, int role) const
{
    if (!_database || !index.isValid())
        return QVariant();

    const DatabaseGroup* group = static_cast<const DatabaseGroup*>(index.internalPointer());

    switch (role) {
        case Qt::ItemDataRole::DisplayRole: case Qt::ItemDataRole::EditRole: return group->title();
        case Qt::ItemDataRole::DecorationRole: return QIcon::fromTheme("folder");
        case (Qt::ItemDataRole::UserRole + 1): return QVariant::fromValue(group);
        default: return QVariant();
    }
}

void DatabaseGroupTreeModel::groupAdded(qsizetype row, QUuid groupUuid)
{
    beginInsertRows(QModelIndex(), _database->entryCount(), _database->entryCount());
    endInsertRows();
}

void DatabaseGroupTreeModel::groupEdited(qsizetype row, QUuid groupUuid)
{
    if (row < 0 || row > rowCount())
        return;
    QModelIndex topLeft = index(row, 0);
    QModelIndex bottomRight = index(row, (columnCount() - 1));
    emit dataChanged(topLeft, bottomRight, { Qt::DisplayRole, Qt::EditRole });
}

void DatabaseGroupTreeModel::groupRemoved(qsizetype row, QUuid groupUuid)
{
    beginRemoveRows(QModelIndex(), row, row);
    endRemoveRows();
}
