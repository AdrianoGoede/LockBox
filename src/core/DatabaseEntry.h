#ifndef DATABASEENTRY_H
#define DATABASEENTRY_H

#include <QUuid>
#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include "DatabaseGroup.h"
#include "SecureQByteArray.h"

class DatabaseEntry
{
public:
    DatabaseEntry();
    DatabaseEntry(const QJsonObject& obj);
    QUuid uid() const;
    QUuid group() const;
    void setGroup(const QUuid& group);
    void setGroup(const DatabaseGroup& group);
    QString title() const;
    void setTitle(const QString& title);
    QString notes() const;
    void setNotes(const QString& notes);
    QDateTime createdAt() const;
    QDateTime modifiedAt() const;
    SecureQByteArray password() const;
    void setPassword(const SecureQByteArray& password);
    QJsonObject toJson() const;

private:
    QUuid _uid;
    QUuid _group;
    QString _title, _notes;
    QDateTime _createdAt, _modifiedAt;
    QByteArray _nonce;
    SecureQByteArray _password, _key;
};

#endif // DATABASEENTRY_H
