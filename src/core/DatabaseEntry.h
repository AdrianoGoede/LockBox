#ifndef DATABASEENTRY_H
#define DATABASEENTRY_H

#include <QUuid>
#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include "SecureBuffer.h"
#include "DatabaseGroup.h"
#include "DatabaseEntryHistoryItem.h"

struct DatabaseEntryDto {
    QUuid group;
    QString title, username, notes;
    SecureBuffer<QChar> password;
};

class DatabaseEntry
{
public:
    DatabaseEntry();
    DatabaseEntry(const DatabaseEntryDto& entryDto, const SecureBuffer<std::byte>& masterKey);
    DatabaseEntry(const QJsonObject& obj);
    QUuid uid() const;
    void setUid(const QUuid& uid);
    QUuid group() const;
    void setGroup(const QUuid& group);
    void setGroup(const DatabaseGroup& group);
    QString title() const;
    void setTitle(const QString& title);
    QString username() const;
    void setUsername(const QString& name);
    QString notes() const;
    void setNotes(const QString& notes);
    QDateTime createdAt() const;
    QDateTime modifiedAt() const;
    SecureBuffer<QChar> password(const SecureBuffer<std::byte>& masterKey) const;
    void setPassword(const SecureBuffer<QChar>& password, const SecureBuffer<std::byte>& masterKey);
    void recordHistory();
    const QVector<DatabaseEntryHistoryItem>& history() const;
    const DatabaseEntryHistoryItem& getHistoryItem(const QUuid& itemUid) const;
    QJsonObject toJson() const;

private:
    QUuid _uid;
    QUuid _group;
    QString _title, _username, _notes;
    QDateTime _createdAt, _modifiedAt;
    QByteArray _keyNonce, _key, _passwordNonce, _password;
    QVector<DatabaseEntryHistoryItem> _history;
};

#endif // DATABASEENTRY_H
