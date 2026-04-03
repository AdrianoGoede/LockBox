#include "DatabaseEntryHistoryItem.h"
#include "Crypto.h"

DatabaseEntryHistoryItem::DatabaseEntryHistoryItem(QDataStream& in) { in >> _itemUid >> _username >> _keyNonce >> _key >> _passwordNonce >> _password >> _createdAt; }

DatabaseEntryHistoryItem::DatabaseEntryHistoryItem(const QString& username, const QByteArray& keyNonce, const QByteArray& key, const QByteArray& passwordNonce, const QByteArray& password)
    : _itemUid(QUuid::createUuid())
    , _username(username)
    , _keyNonce(keyNonce)
    , _key(key)
    , _passwordNonce(passwordNonce)
    , _password(password)
    , _createdAt(QDateTime::currentDateTime())
{}

QUuid DatabaseEntryHistoryItem::itemUid() const { return _itemUid; }

QDateTime DatabaseEntryHistoryItem::createdAt() const { return _createdAt; }

QString DatabaseEntryHistoryItem::username() const { return _username; }

SecureBuffer<QChar> DatabaseEntryHistoryItem::password(const SecureBuffer<std::byte>& masterKey) const
{
    SecureBuffer<std::byte> entryKey = Crypto::decrypt(_key, masterKey, _keyNonce);
    SecureBuffer<std::byte> password = Crypto::decrypt(_password, entryKey, _passwordNonce);
    return Crypto::byteToQChar(password);
}

void DatabaseEntryHistoryItem::toBinary(QDataStream& out) const { out << _itemUid << _username << _keyNonce << _key << _passwordNonce << _password << _createdAt; }
