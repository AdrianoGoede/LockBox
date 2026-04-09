#include "DatabaseEntryHistoryItem.h"
#include "Crypto.h"

DatabaseEntryHistoryItem::DatabaseEntryHistoryItem(QDataStream& in) { in >> _itemUid >> _usernameNonce >> _username >> _passwordNonce >> _password >> _createdAt; }

QUuid DatabaseEntryHistoryItem::itemUid() const { return _itemUid; }

SecureBuffer<QChar> DatabaseEntryHistoryItem::username(const SecureBuffer<std::byte>& entryKey, const QByteArray& entryAad) const { return Crypto::byteToQChar(Crypto::decrypt(_username, entryKey, _usernameNonce, entryAad)); }

SecureBuffer<QChar> DatabaseEntryHistoryItem::password(const SecureBuffer<std::byte>& entryKey, const QByteArray& entryAad) const { return Crypto::byteToQChar(Crypto::decrypt(_password, entryKey, _passwordNonce, entryAad)); }

QDateTime DatabaseEntryHistoryItem::createdAt() const { return _createdAt; }

DatabaseEntryHistoryItem::DatabaseEntryHistoryItem(const QByteArray& usernameNonce, const QByteArray& username, const QByteArray& passwordNonce, const QByteArray& password)
    : _itemUid(QUuid::createUuid())
    , _usernameNonce(usernameNonce)
    , _username(username)
    , _passwordNonce(passwordNonce)
    , _password(password)
    , _createdAt(QDateTime::currentDateTimeUtc())
{}

void DatabaseEntryHistoryItem::toBinary(QDataStream& out) const { out << _itemUid << _usernameNonce << _username << _passwordNonce << _password << _createdAt; }
