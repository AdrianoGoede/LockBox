#include "DatabaseEntryTableProxyModel.h"
#include "DatabaseEntryTableModel.h"
#include <QIODevice>

DatabaseEntryTableProxyModel::DatabaseEntryTableProxyModel(QObject* parent) : QSortFilterProxyModel(parent) {}

QUuid DatabaseEntryTableProxyModel::groupFilter() const { return _groupFilter; }

void DatabaseEntryTableProxyModel::setGroupFilter(const QUuid& uid)
{
    if (uid != _groupFilter) {
        beginFilterChange();
        _groupFilter = uid;
        endFilterChange();
    }
}

QString DatabaseEntryTableProxyModel::titleFilter() const { return _titleFilter; }

void DatabaseEntryTableProxyModel::setTitleFilter(const QString& title)
{
    if (title != _titleFilter) {
        beginFilterChange();
        _titleFilter = title;
        endFilterChange();
    }
}

QDateTime DatabaseEntryTableProxyModel::createdFromFilter() const { return _createdFromFilter; }

void DatabaseEntryTableProxyModel::setCreatedFromFilter(const QDateTime& timestamp)
{
    if (timestamp != _createdFromFilter) {
        beginFilterChange();
        _createdFromFilter = timestamp;
        endFilterChange();
    }
}

QDateTime DatabaseEntryTableProxyModel::createdToFilter() const { return _createdToFilter; }

void DatabaseEntryTableProxyModel::setCreatedToFilter(const QDateTime& timestamp)
{
    if (timestamp != _createdToFilter) {
        beginFilterChange();
        _createdToFilter = timestamp;
        endFilterChange();
    }
}

QDateTime DatabaseEntryTableProxyModel::modifiedFromFilter() const { return _modifedFromFilter; }

void DatabaseEntryTableProxyModel::setModifiedFromFilter(const QDateTime& timestamp)
{
    if (timestamp != _modifedFromFilter) {
        beginFilterChange();
        _modifedFromFilter = timestamp;
        endFilterChange();
    }
}

QDateTime DatabaseEntryTableProxyModel::modifiedToFilter() const { return _modifiedToFilter; }

void DatabaseEntryTableProxyModel::setModifiedToFilter(const QDateTime& timestamp)
{
    if (timestamp != _modifiedToFilter) {
        beginFilterChange();
        _modifiedToFilter = timestamp;
        endFilterChange();
    }
}

bool DatabaseEntryTableProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    const QUuid dbEntryUid = index.data(Qt::UserRole + 1).value<QUuid>();
    if (dbEntryUid.isNull()) return false;

    const DatabaseEntryTableModel* sourceModel = qobject_cast<const DatabaseEntryTableModel*>(this->sourceModel());
    if (!sourceModel) return false;
    const Database* database = sourceModel->database();
    if (!database) return false;
    const DatabaseEntry* dbEntry = database->entry(dbEntryUid);
    if (!dbEntry) return false;

    if (!_groupFilter.isNull() && dbEntry->group() != _groupFilter) return false;
    if (!dbEntry->title().contains(_titleFilter)) return false;
    if (dbEntry->createdAt() < _createdFromFilter || dbEntry->createdAt() > _createdToFilter) return false;
    if (dbEntry->modifiedAt() < _modifedFromFilter || dbEntry->modifiedAt() > _modifiedToFilter) return false;

    return true;
}
