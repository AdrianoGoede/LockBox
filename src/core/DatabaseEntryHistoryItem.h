#ifndef DATABASEENTRYHISTORYITEM_H
#define DATABASEENTRYHISTORYITEM_H

#include "SecureBuffer.h"
#include <QUuid>
#include <QDateTime>
#include <QDataStream>

class DatabaseEntryHistoryItem
{
public:
    explicit DatabaseEntryHistoryItem(QDataStream& in);
    explicit DatabaseEntryHistoryItem(const QByteArray& usernameNonce, const QByteArray& username, const QByteArray& passwordNonce, const QByteArray& password);
    QUuid itemUid() const;
    SecureBuffer<QChar> username(const SecureBuffer<std::byte>& entryKey, const QByteArray& entryAad) const;
    SecureBuffer<QChar> password(const SecureBuffer<std::byte>& entryKey, const QByteArray& entryAad) const;
    QDateTime createdAt() const;
    void toBinary(QDataStream& out) const;

private:
    QUuid _itemUid;
    QByteArray _usernameNonce, _username, _passwordNonce, _password;
    QDateTime _createdAt;
};

#endif // DATABASEENTRYHISTORYITEM_H
