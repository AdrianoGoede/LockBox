#include "DatabaseEntryHistoryItem.h"
#include "Crypto.h"

DatabaseEntryHistoryItem::DatabaseEntryHistoryItem(const QJsonObject& obj)
{
    _itemUid = QUuid::fromString(obj["itemUid"].toString());
    _entryUid = QUuid::fromString(obj["entryUid"].toString());
    _username = obj["username"].toString();
    setPassword(SecureQByteArray(obj["password"].toString().toUtf8()));
    _createdAt = QDateTime::fromSecsSinceEpoch(obj["createdAt"].toInteger());
}

DatabaseEntryHistoryItem::DatabaseEntryHistoryItem(const DatabaseEntry& entry) : _itemUid(QUuid::createUuid()), _entryUid(entry.uid()), _username(entry.username()), _createdAt(QDateTime::currentDateTimeUtc()) { setPassword(entry.password()); }

QUuid DatabaseEntryHistoryItem::itemUid() const { return _itemUid; }

QUuid DatabaseEntryHistoryItem::entryUid() const { return _entryUid; }

QDateTime DatabaseEntryHistoryItem::createdAt() const { return _createdAt; }

QString DatabaseEntryHistoryItem::username() const { return _username; }

SecureQByteArray DatabaseEntryHistoryItem::password() const
{
    SecureQByteArray password;
    Crypto::decrypt(_encryptedPassword, _key, _nonce, password);
    return password;
}

QJsonObject DatabaseEntryHistoryItem::toJson() const
{
    QJsonObject obj;
    obj["itemUid"] = _itemUid.toString(QUuid::StringFormat::WithoutBraces);
    obj["entryUid"] = _entryUid.toString(QUuid::StringFormat::WithoutBraces);
    obj["username"] = _username;
    obj["password"] = password().data();
    obj["createdAt"] = _createdAt.toSecsSinceEpoch();
    return obj;
}

void DatabaseEntryHistoryItem::setPassword(const SecureQByteArray& password)
{
    Crypto::generateKey(_key);
    Crypto::encrypt(password, _key, _encryptedPassword, _nonce);
}
