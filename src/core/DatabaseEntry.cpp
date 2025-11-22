#include "DatabaseEntry.h"
#include "Crypto.h"
#include <QJsonArray>

DatabaseEntry::DatabaseEntry() {}

DatabaseEntry::DatabaseEntry(const QJsonObject& obj)
{
    _uid = QUuid(obj["uuid"].toString());
    _group = QUuid(obj["group"].toString());
    _title = obj["title"].toString();
    _notes = obj["notes"].toString();
    if (_uid.isNull() || _group.isNull() || _title.isEmpty())
        throw std::runtime_error("Invalid or corrupted data");
    this->setPassword(SecureQByteArray(obj["password"].toString().toUtf8()));
}

QUuid DatabaseEntry::uid() const { return _uid; }

QUuid DatabaseEntry::group() const { return _group; }

void DatabaseEntry::setGroup(const QUuid& group) { _group = group; }

void DatabaseEntry::setGroup(const DatabaseGroup& group) { _group = group.uid(); }

QString DatabaseEntry::title() const { return _title; }

void DatabaseEntry::setTitle(const QString& title) { _title = title; }

QString DatabaseEntry::notes() const { return _notes; }

void DatabaseEntry::setNotes(const QString& notes) { _notes = notes; }

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
}

QJsonObject DatabaseEntry::toJson() const
{
    QJsonObject obj;
    obj["uuid"] = _uid.toRfc4122().data();
    obj["title"] = _title;
    obj["password"] = _password.data();
    obj["notes"] = _notes;
    return obj;
}
