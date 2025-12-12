#include "DatabaseEntry.h"
#include "Crypto.h"
#include <QJsonArray>

DatabaseEntry::DatabaseEntry() : _uid(QUuid::createUuid()), _createdAt(QDateTime::currentDateTimeUtc()), _modifiedAt(_createdAt) {}

DatabaseEntry::DatabaseEntry(const QJsonObject& obj)
{
    _uid = QUuid(obj["uuid"].toString());
    _group = QUuid(obj["group"].toString());
    _title = obj["title"].toString();
    _username = obj["username"].toString();
    _notes = obj["notes"].toString();
    if (_uid.isNull() || _group.isNull() || _title.isEmpty())
        throw std::runtime_error("Invalid or corrupted data");
    this->setPassword(SecureQByteArray(obj["password"].toString().toUtf8()));
    _createdAt = QDateTime::fromSecsSinceEpoch(obj["created"].toInt());
    _modifiedAt = QDateTime::fromSecsSinceEpoch(obj["modified"].toInt());
}

QUuid DatabaseEntry::uid() const { return _uid; }

QUuid DatabaseEntry::group() const { return _group; }

void DatabaseEntry::setGroup(const QUuid& group)
{
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

SecureQByteArray DatabaseEntry::password() const
{
    SecureQByteArray result;
    Crypto::decrypt(_password, _key, _nonce, result);
    return result;
}

void DatabaseEntry::setPassword(const SecureQByteArray& password)
{
    Crypto::generateKey(_key);
    Crypto::encrypt(password, _key, _password, _nonce);
    _modifiedAt = QDateTime::currentDateTimeUtc();
}

QJsonObject DatabaseEntry::toJson() const
{
    QJsonObject obj;
    obj["uuid"] = QString(_uid.toRfc4122().data());
    obj["title"] = _title;
    obj["username"] = _username;
    obj["password"] = password().data();
    obj["notes"] = _notes;
    obj["created"] = _createdAt.toSecsSinceEpoch();
    obj["modified"] = _modifiedAt.toSecsSinceEpoch();
    return obj;
}
