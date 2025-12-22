#include "DatabaseEntryTableModel.h"

DatabaseEntryTableModel::DatabaseEntryTableModel(QObject* parent) : QAbstractTableModel{parent} {}

void DatabaseEntryTableModel::setDatabase(Database* database)
{
    beginResetModel();
    _database = database;
    endResetModel();
}

int DatabaseEntryTableModel::rowCount(const QModelIndex& parent) const { return (_database ? _database->entryCount() : 0); }

int DatabaseEntryTableModel::columnCount(const QModelIndex& parent) const { return DatabaseEntryModelColumns::ColumnCount; }

QVariant DatabaseEntryTableModel::data(const QModelIndex& index, int role) const
{
    if (!_database || !index.isValid() || index.row() >= _database->entryCount())
        return QVariant();

    const DatabaseEntry& entry = _database->entry(index.row());

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case DatabaseEntryModelColumns::Uid: return entry.uid().toString(QUuid::StringFormat::WithoutBraces);
            case DatabaseEntryModelColumns::Title: return entry.title();
            case DatabaseEntryModelColumns::CreatedAt: return entry.createdAt();
            case DatabaseEntryModelColumns::ModifiedAt: return entry.modifiedAt();
        }
    }
    else if (role == Qt::UserRole + 1)
        return QVariant::fromValue(&entry);

    return QVariant();
}

QVariant DatabaseEntryTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return QVariant();

    switch (section) {
        case DatabaseEntryModelColumns::Title: return "Title";
        case DatabaseEntryModelColumns::CreatedAt: return "Created At";
        case DatabaseEntryModelColumns::ModifiedAt: return "Modified At";
        case DatabaseEntryModelColumns::Uid: return "Entry UUID";
        default: return QVariant();
    }
}

void DatabaseEntryTableModel::addEntry(const QUuid& group, const QString& title, const QString& notes, const SecureQByteArray& password)
{
    if (!_database) return;
    beginInsertRows(QModelIndex(), _database->entryCount(), _database->entryCount());
    _database->addEntry(group, title, notes, password);
    endInsertRows();
}

void DatabaseEntryTableModel::removeEntry(const QUuid& uid)
{
    if (!_database) return;
    qsizetype index = _database->indexOfEntry(uid);
    if (index < 0) return;

    beginRemoveRows(QModelIndex(), index, index);
    _database->removeEntry(uid);
    endRemoveRows();
}
