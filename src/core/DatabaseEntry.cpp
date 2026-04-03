#include "DatabaseEntry.h"
#include "../config/Constants.h"
#include "Crypto.h"

DatabaseEntry::DatabaseEntry() : _uid(QUuid::createUuid()), _createdAt(QDateTime::currentDateTimeUtc()), _modifiedAt(_createdAt) {}

DatabaseEntry::DatabaseEntry(const DatabaseEntryDto& entryDto, const SecureBuffer<std::byte>& masterKey)
    : _uid(QUuid::createUuid())
    , _group(entryDto.group)
    , _title(entryDto.title)
    , _username(entryDto.username)
    , _notes(entryDto.notes)
    , _createdAt(QDateTime::currentDateTimeUtc())
    , _modifiedAt(_createdAt)
{
    setPassword(entryDto.password, masterKey);
}

DatabaseEntry::DatabaseEntry(QDataStream& in)
{
    in >> _uid
       >> _group
       >> _title
       >> _username
       >> _notes
       >> _createdAt
       >> _modifiedAt
       >> _keyNonce
       >> _key
       >> _passwordNonce
       >> _password;

    quint32 size;
    in >> size;
    for (quint32 i = 0; i < size; i++)
        _history.append(DatabaseEntryHistoryItem(in));
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

void DatabaseEntry::recordHistory()
{
    _history.append(DatabaseEntryHistoryItem(_username, _keyNonce, _key, _passwordNonce, _password));
    while (_history.size() > Config::constants::MAX_DB_ENTRY_HISTORY_ITEMS)
        _history.removeFirst();
}

const QVector<DatabaseEntryHistoryItem>& DatabaseEntry::history() const { return _history; }

const DatabaseEntryHistoryItem& DatabaseEntry::getHistoryItem(const QUuid& itemUid) const
{
    for (const DatabaseEntryHistoryItem& item : _history)
        if (item.itemUid() == itemUid)
            return item;
    throw std::runtime_error("History item does not exist");
}

void DatabaseEntry::toBinary(QDataStream& out) const
{
    out << _uid
        << _group
        << _title
        << _username
        << _notes
        << _createdAt
        << _modifiedAt
        << _keyNonce
        << _key
        << _passwordNonce
        << _password;

    out << static_cast<quint32>(_history.size());
    for (const DatabaseEntryHistoryItem& item : _history)
        item.toBinary(out);
}
