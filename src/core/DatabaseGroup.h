#ifndef DATABASEGROUP_H
#define DATABASEGROUP_H

#include <QMap>
#include <QUuid>
#include <QJsonObject>

struct DatabaseGroupDto {
    QUuid parent;
    QString title;
};

class DatabaseGroup
{
public:
    DatabaseGroup();
    DatabaseGroup(const DatabaseGroupDto& groupDto);
    DatabaseGroup(const QJsonObject& jsonObj);
    QUuid uid() const;
    QUuid parent() const;
    void setParent(const QUuid& parent);
    QString title() const;
    void setTitle(const QString& title);
    QJsonObject toJson() const;

private:
    QUuid _uid, _parent;
    QString _title;
};

#endif // DATABASEGROUP_H
