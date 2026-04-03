#include "DatabaseEntryTableModel.h"
#include <QPushButton>

DatabaseEntryTableModel::DatabaseEntryTableModel(QObject* parent) : QAbstractTableModel{parent} {}

const Database *DatabaseEntryTableModel::database() const { return _database; }

void DatabaseEntryTableModel::setDatabase(const Database* database)
{
    beginResetModel();

    if (_database)
        disconnect(_database, nullptr, this, nullptr);
    _database = database;

    if (_database) {
        connect(_database, &Database::entryAdded, this, &DatabaseEntryTableModel::entryAdded);
        connect(_database, &Database::entryEdited, this, &DatabaseEntryTableModel::entryEdited);
        connect(_database, &Database::entryRemoved, this, &DatabaseEntryTableModel::entryRemoved);
        connect(_database, &Database::entryMoved, this, &DatabaseEntryTableModel::entryEdited);
    }

    endResetModel();
}

int DatabaseEntryTableModel::rowCount(const QModelIndex& parent) const { return (_database ? _database->entryCount() : 0); }

int DatabaseEntryTableModel::columnCount(const QModelIndex& parent) const { return DatabaseEntryModelColumns::ColumnCount; }

QVariant DatabaseEntryTableModel::data(const QModelIndex& index, int role) const
{
    if (!_database || !index.isValid() || index.row() >= _database->entryCount())
        return QVariant();

    const DatabaseEntry* entry = _database->entry(index.row());
    if (!entry) return QVariant();

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case DatabaseEntryModelColumns::Title: return entry->title();
            case DatabaseEntryModelColumns::CreatedAt: return entry->createdAt();
            case DatabaseEntryModelColumns::ModifiedAt: return entry->modifiedAt();
        }
    }
    else if (role == Qt::UserRole + 1)
        return QVariant::fromValue(entry->uid());

    return QVariant();
}

QVariant DatabaseEntryTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return QVariant();

    switch (section) {
        case DatabaseEntryModelColumns::Title: return "Title";
        case DatabaseEntryModelColumns::CreatedAt: return "Creation";
        case DatabaseEntryModelColumns::ModifiedAt: return "Modified At";
        case DatabaseEntryModelColumns::Manage: return "Manage";
        case DatabaseEntryModelColumns::CopyUsername: return "User";
        case DatabaseEntryModelColumns::CopyPassword: return "Passw.";
        case DatabaseEntryModelColumns::PerformAutotype: return "Autotype";
        default: return QVariant();
    }
}

Qt::ItemFlags DatabaseEntryTableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    if (index.isValid())
        return (flags | Qt::ItemIsDragEnabled);
    return flags;
}

Qt::DropActions DatabaseEntryTableModel::supportedDragActions() const { return Qt::MoveAction; }

QStringList DatabaseEntryTableModel::mimeTypes() const {
    return {
        "application/x-qabstractitemmodeldatalist",
        "text/uri-list",
        "application/x-custom-entry-uuid"
    };
}

QMimeData* DatabaseEntryTableModel::mimeData(const QModelIndexList& indexes) const
{
    QMimeData* mimeData = new QMimeData();
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    if (!indexes.isEmpty()) {
        QModelIndex index = indexes.first();
        const DatabaseEntry* entry = _database->entry(index.row());
        if (entry)
            stream << entry->uid().toString(QUuid::WithoutBraces);
    }

    mimeData->setData("application/x-custom-entry-uuid", encodedData);
    return mimeData;
}

void DatabaseEntryTableModel::entryAdded(qsizetype row, QUuid entryUuid)
{
    beginInsertRows(QModelIndex(), (_database->entryCount() - 1), (_database->entryCount() - 1));
    endInsertRows();
}

void DatabaseEntryTableModel::entryEdited(qsizetype row, QUuid entryUuid)
{
    if (row < 0 || row > rowCount())
        return;
    QModelIndex topLeft = index(row, 0);
    QModelIndex bottomRight = index(row, (columnCount() - 1));
    emit dataChanged(topLeft, bottomRight, { Qt::DisplayRole, Qt::EditRole });
}

void DatabaseEntryTableModel::entryRemoved(qsizetype row, QUuid entryUuid)
{
    beginRemoveRows(QModelIndex(), row, row);
    endRemoveRows();
}
