#include "DatabaseEntryHistoryItem.h"
#include "Crypto.h"

DatabaseEntryHistoryItem::DatabaseEntryHistoryItem(const QJsonObject& obj)
{
    _itemUid = QUuid::fromString(obj["itemUid"].toString());
    _username = obj["username"].toString();
    _createdAt = QDateTime::fromSecsSinceEpoch(obj["createdAt"].toInteger());

    _keyNonce = QByteArray::fromBase64(obj["keyNonce"].toString().toUtf8());
    _key = QByteArray::fromBase64(obj["key"].toString().toUtf8());
    _passwordNonce = QByteArray::fromBase64(obj["passwordNonce"].toString().toUtf8());
    _password = QByteArray::fromBase64(obj["password"].toString().toUtf8());
}

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

QJsonObject DatabaseEntryHistoryItem::toJson() const
{
    QJsonObject obj;
    obj["itemUid"] = _itemUid.toString(QUuid::StringFormat::WithoutBraces);
    obj["username"] = _username;
    obj["keyNonce"] = QString(_keyNonce.toBase64());
    obj["key"] = QString(_key.toBase64());
    obj["passwordNonce"] = QString(_passwordNonce.toBase64());
    obj["password"] = QString(_password.toBase64());
    obj["createdAt"] = _createdAt.toSecsSinceEpoch();
    return obj;
}
