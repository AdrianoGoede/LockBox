#include "DatabaseEntry.h"
#include "../config/Constants.h"
#include "Crypto.h"

DatabaseEntry::DatabaseEntry() : _uid(QUuid::createUuid()), _createdAt(QDateTime::currentDateTimeUtc()), _modifiedAt(_createdAt) {}

DatabaseEntry::DatabaseEntry(const DatabaseEntryDto& entryDto, const SecureBuffer<std::byte>& masterKey)
    : _uid(QUuid::createUuid())
    , _group(entryDto.group)
    , _title(entryDto.title)
    , _createdAt(QDateTime::currentDateTimeUtc())
    , _modifiedAt(_createdAt)
{
    _entryKeyNonce = Crypto::generateNonce();
    SecureBuffer<std::byte> key = Crypto::generateKey();
    _entryKey = Crypto::encrypt(key, masterKey, _entryKeyNonce, _uid.toByteArray(QUuid::StringFormat::WithoutBraces));

    setUsernameOnInit(entryDto.username, key);
    setPasswordOnInit(entryDto.password, key);
    setNotesOnInit(entryDto.notes, key);
}

DatabaseEntry::DatabaseEntry(QDataStream& in)
{
    in >> _uid
       >> _group
       >> _title
       >> _createdAt
       >> _modifiedAt
       >> _entryKeyNonce
       >> _entryKey
       >> _usernameNonce
       >> _username
       >> _passwordNonce
       >> _password
       >> _notesNonce
       >> _notes;

    quint32 size;
    in >> size;
    for (quint32 i = 0; i < size; i++)
        _history.append(DatabaseEntryHistoryItem(in));
}

QUuid DatabaseEntry::uid() const { return _uid; }

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

SecureBuffer<QChar> DatabaseEntry::username(const SecureBuffer<std::byte>& masterKey) const
{
    SecureBuffer<std::byte> entryKey = Crypto::decrypt(_entryKey, masterKey, _entryKeyNonce, _uid.toByteArray(QUuid::StringFormat::WithoutBraces));
    SecureBuffer<std::byte> username = Crypto::decrypt(_username, entryKey, _usernameNonce, _uid.toByteArray(QUuid::StringFormat::WithoutBraces));
    return Crypto::byteToQChar(username);
}

void DatabaseEntry::setUsername(const SecureBuffer<QChar>& name, const SecureBuffer<std::byte>& masterKey)
{
    SecureBuffer<std::byte> entryKey = Crypto::decrypt(_entryKey, masterKey, _entryKeyNonce, _uid.toByteArray(QUuid::StringFormat::WithoutBraces));
    QByteArray usernameNonce = Crypto::generateNonce();
    QByteArray newUsername = Crypto::encrypt(Crypto::qCharToByte(name), entryKey, usernameNonce, _uid.toByteArray(QUuid::StringFormat::WithoutBraces));

    _usernameNonce = usernameNonce;
    _username = newUsername;
    _modifiedAt = QDateTime::currentDateTimeUtc();
}

SecureBuffer<QChar> DatabaseEntry::notes(const SecureBuffer<std::byte>& masterKey) const
{
    SecureBuffer<std::byte> entryKey = Crypto::decrypt(_entryKey, masterKey, _entryKeyNonce, _uid.toByteArray(QUuid::StringFormat::WithoutBraces));
    SecureBuffer<std::byte> notes = Crypto::decrypt(_notes, entryKey, _notesNonce, _uid.toByteArray(QUuid::StringFormat::WithoutBraces));
    return Crypto::byteToQChar(notes);
}

void DatabaseEntry::setNotes(const SecureBuffer<QChar>& notes, const SecureBuffer<std::byte>& masterKey)
{
    SecureBuffer<std::byte> entryKey = Crypto::decrypt(_entryKey, masterKey, _entryKeyNonce, _uid.toByteArray(QUuid::StringFormat::WithoutBraces));
    QByteArray notesNonce = Crypto::generateNonce();
    QByteArray newNotes = Crypto::encrypt(Crypto::qCharToByte(notes), entryKey, notesNonce, _uid.toByteArray(QUuid::StringFormat::WithoutBraces));

    _notesNonce = notesNonce;
    _notes = newNotes;
    _modifiedAt = QDateTime::currentDateTimeUtc();
}

QDateTime DatabaseEntry::createdAt() const { return _createdAt; }

QDateTime DatabaseEntry::modifiedAt() const { return _modifiedAt; }

SecureBuffer<QChar> DatabaseEntry::password(const SecureBuffer<std::byte>& masterKey) const
{
    SecureBuffer<std::byte> entryKey = Crypto::decrypt(_entryKey, masterKey, _entryKeyNonce, _uid.toByteArray(QUuid::StringFormat::WithoutBraces));
    SecureBuffer<std::byte> password = Crypto::decrypt(_password, entryKey, _passwordNonce, _uid.toByteArray(QUuid::StringFormat::WithoutBraces));
    return Crypto::byteToQChar(password);
}

void DatabaseEntry::setPassword(const SecureBuffer<QChar>& password, const SecureBuffer<std::byte>& masterKey)
{
    SecureBuffer<std::byte> entryKey = Crypto::decrypt(_entryKey, masterKey, _entryKeyNonce, _uid.toByteArray(QUuid::StringFormat::WithoutBraces));
    QByteArray passwordNonce = Crypto::generateNonce();
    QByteArray newPassword = Crypto::encrypt(Crypto::qCharToByte(password), entryKey, passwordNonce, _uid.toByteArray(QUuid::StringFormat::WithoutBraces));

    _passwordNonce = passwordNonce;
    _password = newPassword;
    _modifiedAt = QDateTime::currentDateTimeUtc();
}

void DatabaseEntry::recordHistory()
{
    _history.append(DatabaseEntryHistoryItem(_usernameNonce, _username, _passwordNonce, _password));
    while (_history.size() > Config::constants::MAX_DB_ENTRY_HISTORY_ITEMS)
        _history.removeFirst();
}

const QVector<DatabaseEntryHistoryItem>& DatabaseEntry::history() const { return _history; }

SecureBuffer<QChar> DatabaseEntry::historyItemUsername(const QUuid& itemUid, const SecureBuffer<std::byte>& masterKey) const
{
    for (const DatabaseEntryHistoryItem& item : _history)
        if (item.itemUid() == itemUid) {
            SecureBuffer<std::byte> entryKey = Crypto::decrypt(_entryKey, masterKey, _entryKeyNonce, _uid.toByteArray(QUuid::StringFormat::WithoutBraces));
            return item.username(entryKey, _uid.toByteArray(QUuid::StringFormat::WithoutBraces));
        }
    throw std::runtime_error("History item does not exist");
}

SecureBuffer<QChar> DatabaseEntry::historyItemPassword(const QUuid& itemUid, const SecureBuffer<std::byte>& masterKey) const
{
    for (const DatabaseEntryHistoryItem& item : _history)
        if (item.itemUid() == itemUid) {
            SecureBuffer<std::byte> entryKey = Crypto::decrypt(_entryKey, masterKey, _entryKeyNonce, _uid.toByteArray(QUuid::StringFormat::WithoutBraces));
            return item.password(entryKey, _uid.toByteArray(QUuid::StringFormat::WithoutBraces));
        }
    throw std::runtime_error("History item does not exist");
}

void DatabaseEntry::toBinary(QDataStream& out) const
{
    out << _uid
        << _group
        << _title
        << _createdAt
        << _modifiedAt
        << _entryKeyNonce
        << _entryKey
        << _usernameNonce
        << _username
        << _passwordNonce
        << _password
        << _notesNonce
        << _notes;

    out << static_cast<quint32>(_history.size());
    for (const DatabaseEntryHistoryItem& item : _history)
        item.toBinary(out);
}

void DatabaseEntry::setUsernameOnInit(const SecureBuffer<QChar>& name, const SecureBuffer<std::byte>& entryKey)
{
    _usernameNonce = Crypto::generateNonce();
    _username = Crypto::encrypt(Crypto::qCharToByte(name), entryKey, _usernameNonce, _uid.toByteArray(QUuid::StringFormat::WithoutBraces));
}

void DatabaseEntry::setPasswordOnInit(const SecureBuffer<QChar>& password, const SecureBuffer<std::byte>& entryKey)
{
    _passwordNonce = Crypto::generateNonce();
    _password = Crypto::encrypt(Crypto::qCharToByte(password), entryKey, _passwordNonce, _uid.toByteArray(QUuid::StringFormat::WithoutBraces));
}

void DatabaseEntry::setNotesOnInit(const SecureBuffer<QChar>& notes, const SecureBuffer<std::byte>& entryKey)
{
    _notesNonce = Crypto::generateNonce();
    _notes = Crypto::encrypt(Crypto::qCharToByte(notes), entryKey, _notesNonce, _uid.toByteArray(QUuid::StringFormat::WithoutBraces));
}
