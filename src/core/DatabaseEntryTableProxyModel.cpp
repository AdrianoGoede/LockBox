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
    if (title.toLower() != _titleFilter) {
        beginFilterChange();
        _titleFilter = title.toLower();
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
    if (!dbEntry->title().toLower().contains(_titleFilter)) return false;

    return true;
}
