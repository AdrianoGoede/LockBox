#ifndef DATABASEGROUP_H
#define DATABASEGROUP_H

#include <QMap>
#include <QUuid>
#include <QDataStream>

struct DatabaseGroupDto {
    QUuid parent;
    QString title;
};

class DatabaseGroup
{
public:
    DatabaseGroup();
    DatabaseGroup(const DatabaseGroupDto& groupDto);
    DatabaseGroup(QDataStream& in);
    QUuid uid() const;
    QUuid parent() const;
    void setParent(const QUuid& parent);
    QString title() const;
    void setTitle(const QString& title);
    void toBinary(QDataStream& out) const;

private:
    QUuid _uid, _parent;
    QString _title;
};

#endif // DATABASEGROUP_H
