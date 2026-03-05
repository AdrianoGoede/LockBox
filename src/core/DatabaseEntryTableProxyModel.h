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
    Q_PROPERTY(QDateTime _createdFromFilter READ createdFromFilter WRITE setCreatedFromFilter)
    Q_PROPERTY(QDateTime _createdToFilter READ createdToFilter WRITE setCreatedToFilter)
    Q_PROPERTY(QDateTime _modifedFromFilter READ modifiedFromFilter WRITE setModifiedFromFilter)
    Q_PROPERTY(QDateTime _modifiedToFilter READ modifiedToFilter WRITE setModifiedToFilter)

public:
    explicit DatabaseEntryTableProxyModel(QObject* parent = nullptr);
    QUuid groupFilter() const;
    void setGroupFilter(const QUuid& uid);
    QString titleFilter() const;
    void setTitleFilter(const QString& title);
    QDateTime createdFromFilter() const;
    void setCreatedFromFilter(const QDateTime& timestamp);
    QDateTime createdToFilter() const;
    void setCreatedToFilter(const QDateTime& timestamp);
    QDateTime modifiedFromFilter() const;
    void setModifiedFromFilter(const QDateTime& timestamp);
    QDateTime modifiedToFilter() const;
    void setModifiedToFilter(const QDateTime& timestamp);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

private:
    QUuid _groupFilter;
    QString _titleFilter;
    QDateTime _createdFromFilter, _createdToFilter, _modifedFromFilter, _modifiedToFilter;
};

#endif // DATABASEENTRYTABLEPROXYMODEL_H
