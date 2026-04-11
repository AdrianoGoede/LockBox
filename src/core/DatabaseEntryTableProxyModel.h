#ifndef DATABASEENTRYTABLEPROXYMODEL_H
#define DATABASEENTRYTABLEPROXYMODEL_H

#include <QUuid>
#include <QObject>
#include <QDateTime>
#include <QMimeData>
#include <QSortFilterProxyModel>

class DatabaseEntryTableProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QUuid _groupFilter READ groupFilter WRITE setGroupFilter)
    Q_PROPERTY(QString _titleFilter READ titleFilter WRITE setTitleFilter)

public:
    explicit DatabaseEntryTableProxyModel(QObject* parent = nullptr);
    QUuid groupFilter() const;
    void setGroupFilter(const QUuid& uid);
    QString titleFilter() const;
    void setTitleFilter(const QString& title);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

private:
    QUuid _groupFilter;
    QString _titleFilter;
};

#endif // DATABASEENTRYTABLEPROXYMODEL_H
