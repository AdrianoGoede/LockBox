#ifndef DATABASEENTRYHISTORYITEM_H
#define DATABASEENTRYHISTORYITEM_H

#include "DatabaseEntry.h"
#include "SecureQByteArray.h"
#include <QUuid>
#include <QJsonObject>

class DatabaseEntryHistoryItem
{
public:
    explicit DatabaseEntryHistoryItem(const QJsonObject& obj);
    explicit DatabaseEntryHistoryItem(const DatabaseEntry& entry);
    QUuid itemUid() const;
    QUuid entryUid() const;
    QDateTime createdAt() const;
    QString username() const;
    SecureQByteArray password() const;
    QJsonObject toJson() const;

private:
    QUuid _itemUid, _entryUid;
    QString _username;
    QByteArray _nonce;
    SecureQByteArray _encryptedPassword, _key;
    QDateTime _createdAt;
    void setPassword(const SecureQByteArray& password);
};

#endif // DATABASEENTRYHISTORYITEM_H
