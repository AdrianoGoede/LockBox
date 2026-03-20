#ifndef DATABASEENTRYHISTORYITEM_H
#define DATABASEENTRYHISTORYITEM_H

#include "SecureBuffer.h"
#include <QUuid>
#include <QJsonObject>

class DatabaseEntryHistoryItem
{
public:
    explicit DatabaseEntryHistoryItem(const QJsonObject& obj);
    explicit DatabaseEntryHistoryItem(const QString& username, const QByteArray& keyNonce, const QByteArray& key, const QByteArray& passwordNonce, const QByteArray& password);
    QUuid itemUid() const;
    QDateTime createdAt() const;
    QString username() const;
    SecureBuffer<QChar> password(const SecureBuffer<std::byte>& masterKey) const;
    QJsonObject toJson() const;

private:
    QUuid _itemUid;
    QString _username;
    QByteArray _keyNonce, _key, _passwordNonce, _password;
    QDateTime _createdAt;
};

#endif // DATABASEENTRYHISTORYITEM_H
