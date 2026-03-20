#include "DatabaseEntry.h"
#include "../config/Constants.h"
#include "Crypto.h"
#include <QJsonArray>

DatabaseEntry::DatabaseEntry() : _uid(QUuid::createUuid()), _createdAt(QDateTime::currentDateTimeUtc()), _modifiedAt(_createdAt) {}

DatabaseEntry::DatabaseEntry(const QJsonObject& obj)
{
    _uid = QUuid::fromString(obj["uuid"].toString());
    _group = QUuid::fromString(obj["group"].toString());
    _title = obj["title"].toString();
    _username = obj["username"].toString();
    _notes = obj["notes"].toString();
    if (_uid.isNull() || _group.isNull() || _title.isEmpty())
        throw std::runtime_error("Invalid or corrupted data");
    _createdAt = QDateTime::fromSecsSinceEpoch(obj["created"].toInteger());
    _modifiedAt = QDateTime::fromSecsSinceEpoch(obj["modified"].toInteger());

    _keyNonce = QByteArray::fromBase64(obj["keyNonce"].toString().toUtf8());
    _key = QByteArray::fromBase64(obj["key"].toString().toUtf8());
    _passwordNonce = QByteArray::fromBase64(obj["passwordNonce"].toString().toUtf8());
    _password = QByteArray::fromBase64(obj["password"].toString().toUtf8());

    QJsonArray history = obj["history"].toArray(QJsonArray());
    for (const QJsonValueRef item : history)
        _history.append(DatabaseEntryHistoryItem(item.toObject()));
}

QUuid DatabaseEntry::uid() const { return _uid; }

void DatabaseEntry::setUid(const QUuid& uid)
{
    if (uid.isNull())
        throw std::runtime_error("Entry UUID must be valid");
    _uid = uid;
    _modifiedAt = QDateTime::currentDateTimeUtc();
}

QUuid DatabaseEntry::group() const { return _group; }

void DatabaseEntry::setGroup(const QUuid& group)
{
    if (group.isNull())
        throw std::runtime_error("Entry must have a valid group");
    _group = group;
    _modifiedAt = QDateTime::currentDateTimeUtc();
}

void DatabaseEntry::setGroup(const DatabaseGroup& group)
{
    _group = group.uid();
    _modifiedAt = QDateTime::currentDateTimeUtc();
}

QString DatabaseEntry::title() const { return _title; }

void DatabaseEntry::setTitle(const QString& title)
{
    _title = title;
    _modifiedAt = QDateTime::currentDateTimeUtc();
}

QString DatabaseEntry::username() const { return _username; }

void DatabaseEntry::setUsername(const QString& name)
{
    recordHistory();
    _username = name;
    _modifiedAt = QDateTime::currentDateTimeUtc();
}

QString DatabaseEntry::notes() const { return _notes; }

void DatabaseEntry::setNotes(const QString& notes)
{
    _notes = notes;
    _modifiedAt = QDateTime::currentDateTimeUtc();
}

QDateTime DatabaseEntry::createdAt() const { return _createdAt; }

QDateTime DatabaseEntry::modifiedAt() const { return _modifiedAt; }

SecureBuffer<QChar> DatabaseEntry::password(const SecureBuffer<std::byte>& masterKey) const
{
    SecureBuffer<std::byte> entryKey = Crypto::decrypt(_key, masterKey, _keyNonce);
    SecureBuffer<std::byte> password = Crypto::decrypt(_password, entryKey, _passwordNonce);
    return Crypto::byteToQChar(password);
}

void DatabaseEntry::setPassword(const SecureBuffer<QChar>& password, const SecureBuffer<std::byte>& masterKey)
{
    recordHistory();

    SecureBuffer<std::byte> entryKey = Crypto::generateKey();
    QByteArray passwordNonce = Crypto::generateNonce();
    QByteArray newPassword = Crypto::encrypt(Crypto::qCharToByte(password), entryKey, passwordNonce);

    QByteArray keyNonce = Crypto::generateNonce();
    QByteArray encryptedKey = Crypto::encrypt(entryKey, masterKey, keyNonce);

    _keyNonce = keyNonce;
    _key = encryptedKey;
    _passwordNonce = passwordNonce;
    _password = newPassword;
}

const QVector<DatabaseEntryHistoryItem>& DatabaseEntry::history() const { return _history; }

const DatabaseEntryHistoryItem& DatabaseEntry::getHistoryItem(const QUuid& itemUid) const
{
    for (const DatabaseEntryHistoryItem& item : _history)
        if (item.itemUid() == itemUid)
            return item;
    throw std::runtime_error("History item does not exist");
}

QJsonObject DatabaseEntry::toJson() const
{
    QJsonObject obj;
    obj["uuid"] = _uid.toString(QUuid::StringFormat::WithoutBraces);
    obj["group"] = _group.toString(QUuid::StringFormat::WithoutBraces);
    obj["title"] = _title;
    obj["username"] = _username;
    obj["notes"] = _notes;
    obj["created"] = _createdAt.toSecsSinceEpoch();
    obj["modified"] = _modifiedAt.toSecsSinceEpoch();

    obj["keyNonce"] = QString(_keyNonce.toBase64());
    obj["key"] = QString(_key.toBase64());
    obj["passwordNonce"] = QString(_passwordNonce.toBase64());
    obj["password"] = QString(_password.toBase64());

    QJsonArray history;
    for (const DatabaseEntryHistoryItem& item : _history)
        history.append(item.toJson());
    obj["history"] = history;

    return obj;
}

void DatabaseEntry::recordHistory()
{
    _history.append(DatabaseEntryHistoryItem(_username, _keyNonce, _key, _passwordNonce, _password));
    while (_history.size() > Config::constants::MAX_DB_ENTRY_HISTORY_ITEMS)
        _history.removeFirst();
}
