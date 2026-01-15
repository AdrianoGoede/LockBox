#include "Database.h"
#include "Crypto.h"
#include "../config/Constants.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

Database::Database(const QString& filePath, const SecureQByteArray& password, std::chrono::milliseconds unlockDelay, QObject* parent) : QObject{parent}
{
    _dbFile = std::make_unique<QFile>(filePath);
    if (_dbFile->exists())
        this->load(password);
    else
        this->create(password, unlockDelay);
}

Database::~Database() { if (_dbFile && _dbFile->isOpen()) _dbFile->close(); }

void Database::create(const SecureQByteArray& password, std::chrono::milliseconds unlockDelay)
{
    if (!_dbFile->open(QIODevice::OpenModeFlag::ReadWrite))
        throw std::runtime_error(QString("Could not create database file: %1").arg(_dbFile->errorString()).toUtf8());

    _compressionLevel = Config::constants::DEFAULT_COMPRESSION_LEVEL;
    _kdfMemory = Config::constants::DEFAULT_KDF_MEMORY;
    _kdfIterations = Config::constants::DEFAULT_KDF_ITERATIONS;
    _kdfParallelism = Config::constants::DEFAULT_KDF_PARALLELISM;

    Crypto::generateSalt(_kdfSalt);
    Crypto::tuneArgon2idParams(unlockDelay, _kdfMemory, _kdfIterations, _kdfParallelism);
    Crypto::deriveKey(password, _kdfSalt, _kdfMemory, _kdfIterations, _kdfParallelism, _masterKey);

    save();
}

void Database::load(const SecureQByteArray& password)
{
    if (!_dbFile->open(QIODevice::OpenModeFlag::ReadWrite))
        throw std::runtime_error(QString("Could not open database file: %1").arg(_dbFile->errorString()).toUtf8());

    SecureQByteArray payload(_dbFile->readAll());

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(payload, &parseError);
    if (parseError.error != QJsonParseError::ParseError::NoError)
        throw std::runtime_error(parseError.errorString().toStdString());

    loadHeader(doc.object()["header"].toObject(), password);
    loadData(QByteArray::fromBase64(doc.object()["data"].toString().toUtf8()));
}

void Database::save()
{
    QJsonArray dataGroups;
    for (const DatabaseGroup& group : _dbGroups)
        dataGroups.append(group.toJson());
    QJsonArray dataEntries;
    for (const DatabaseEntry& entry : _dbEntries)
        dataEntries.append(entry.toJson());
    QJsonObject dataObj {
        { "groups", dataGroups },
        { "entries", dataEntries }
    };

    QByteArray encryptedData;
    Crypto::encrypt(
        SecureQByteArray(qCompress(QJsonDocument(dataObj).toJson(QJsonDocument::JsonFormat::Compact), _compressionLevel)),
        _masterKey,
        encryptedData,
        _cryptoNonce
    );

    QJsonObject jsonObj;
    jsonObj["header"] = QJsonObject{
        { "kdf", QJsonObject {
            { "memory", QJsonValue::fromVariant(_kdfMemory) },
            { "iterations", QJsonValue::fromVariant(_kdfIterations) },
            { "parallelism", QJsonValue::fromVariant(_kdfParallelism) },
            { "salt", QString(_kdfSalt.toBase64()) }
        }},
        { "crypto", QJsonObject {
            { "nonce", QString(_cryptoNonce.toBase64()) }
        }},
        { "compression", QJsonObject {
            { "level", QJsonValue::fromVariant(_compressionLevel) }
        }}
    };
    jsonObj["data"] = QString(encryptedData.toBase64());

    try {
        _dbFile->startTransaction();
        _dbFile->resize(0);
        _dbFile->write(QJsonDocument(jsonObj).toJson(QJsonDocument::JsonFormat::Compact));
        _dbFile->flush();
        _dbFile->commitTransaction();
    }
    catch (...) {
        _dbFile->rollbackTransaction();
        throw;
    }
}

void Database::addEntry(const DatabaseEntry& entry)
{
    if (_dbEntries.contains(entry.uid()))
        throw std::runtime_error("Entry already exists");
    if (entry.title().trimmed().isEmpty())
        throw std::runtime_error("Entry must have a title");
    if (entry.group().isNull())
        throw std::runtime_error("Entry must have a valid parent");

    _dbEntryKeys.append(entry.uid());
    _dbEntries[entry.uid()] = entry;

    emit entryAdded(_dbEntryKeys.size() - 1);
}

void Database::addGroup(const DatabaseGroup& group)
{
    if (_dbGroups.contains(group.uid()))
        throw std::runtime_error("Group already exists");
    if (group.title().trimmed().isEmpty())
        throw std::runtime_error("Group must have a name");

    _dbGroupKeys.append(group.uid());
    _dbGroups[group.uid()] = group;

    emit groupAdded(_dbGroupKeys.size() - 1);
}

void Database::editEntry(const DatabaseEntry& entry)
{
    if (!_dbEntries.contains(entry.uid()))
        throw std::runtime_error("Entry does not exist");
    _dbEntries[entry.uid()] = entry;
}

void Database::editGroup(const DatabaseGroup& group)
{
    if (!_dbGroups.contains(group.uid()))
        throw std::runtime_error("Group does not exist!");
    _dbGroups[group.uid()] = group;
}

void Database::removeEntry(const QUuid& uid)
{
    qsizetype row = _dbEntryKeys.indexOf(uid);
    if (row < 0) return;
    _dbEntryKeys.removeAt(row);
    _dbEntries.remove(uid);
    emit entryRemoved(row);
}

void Database::removeGroup(const QUuid& uid)
{
    qsizetype row = _dbGroupKeys.indexOf(uid);
    if (row < 0) return;

    for (const QUuid& entry : entriesOfGroup(uid)) {
        _dbEntryKeys.removeOne(entry);
        _dbEntries.remove(entry);
    }

    _dbGroupKeys.removeAt(row);
    _dbGroups.remove(uid);
    emit groupRemoved(row);
}

size_t Database::entryCount() const { return _dbEntries.size(); }

size_t Database::groupCount() const { return _dbGroups.size(); }

const DatabaseEntry& Database::entry(const QUuid& uid) const
{
    if (!_dbEntries.contains(uid))
        throw std::runtime_error("Entry not found");
    return _dbEntries.find(uid).value();
}

const DatabaseEntry& Database::entry(int index) const
{
    if (index >= _dbEntryKeys.size())
        throw std::runtime_error("Index out of range");
    const QUuid& uid = _dbEntryKeys[index];
    return _dbEntries.find(uid).value();
}

const DatabaseGroup& Database::group(const QUuid &uid) const
{
    if (!_dbGroups.contains(uid))
        throw std::runtime_error("Group not found");
    return _dbGroups.find(uid).value();
}

const DatabaseGroup& Database::group(int index) const
{
    if (index >= _dbGroupKeys.size())
        throw std::runtime_error("Index out of range");
    const QUuid& uid = _dbGroupKeys[index];
    return _dbGroups.find(uid).value();
}

QVector<const DatabaseGroup*> Database::childrenOfGroup(const DatabaseGroup* group) const
{
    QVector<const DatabaseGroup*> children;
    QUuid parentUuid = (group ? group->uid() : QUuid());
    for (const DatabaseGroup& child : _dbGroups)
        if (child.parent() == parentUuid)
            children.append(&child);
    return children;
}

QVector<QUuid> Database::entriesOfGroup(const QUuid& group) const
{
    QVector<QUuid> results;
    for (const DatabaseEntry& entry : _dbEntries) {
        if (entry.group() == group)
            results.append(entry.uid());
    }
    return results;
}

qsizetype Database::indexOfEntry(const QUuid& uid) const { return _dbEntryKeys.indexOf(uid); }

qsizetype Database::indexOfGroup(const QUuid& uid) const { return _dbGroupKeys.indexOf(uid); }

void Database::loadHeader(const QJsonObject& header, const SecureQByteArray& password)
{
    QJsonObject obj = header["kdf"].toObject();
    if (obj.isEmpty()) throw std::runtime_error("Invalid or corrupted database file");
    _kdfMemory = obj["memory"].toInt(Config::constants::DEFAULT_KDF_MEMORY);
    _kdfIterations = obj["iterations"].toInt(Config::constants::DEFAULT_KDF_ITERATIONS);
    _kdfParallelism = obj["parallelism"].toInt(Config::constants::DEFAULT_KDF_PARALLELISM);
    _kdfSalt = QByteArray::fromBase64(obj["salt"].toString().toUtf8());

    obj = header["compression"].toObject();
    if (obj.isEmpty()) throw std::runtime_error("Invalid or corrupted database file");
    _compressionLevel = obj["level"].toInt(Config::constants::DEFAULT_COMPRESSION_LEVEL);

    obj = header["crypto"].toObject();
    if (obj.isEmpty()) throw std::runtime_error("Invalid or corrupted database file");
    _cryptoNonce = QByteArray::fromBase64(obj["nonce"].toString().toUtf8());
    if (_cryptoNonce.isEmpty()) throw std::runtime_error("Invalid or corrupted database file");

    Crypto::deriveKey(password, _kdfSalt, _kdfMemory, _kdfIterations, _kdfParallelism, _masterKey);
}

void Database::loadData(const QByteArray& data)
{
    SecureQByteArray plaintext;
    Crypto::decrypt(data, _masterKey, _cryptoNonce, plaintext);

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(qUncompress(plaintext), &parseError);
    if (parseError.error != QJsonParseError::ParseError::NoError)
        throw std::runtime_error("Invalid or corrupted database file");

    QJsonArray array = doc.object()["groups"].toArray();
    for (const QJsonValueRef& groupRef : array) {
        DatabaseGroup group(groupRef.toObject());
        _dbGroupKeys.append(group.uid());
        _dbGroups[group.uid()] = std::move(group);
    }

    array = doc.object()["entries"].toArray();
    for (const QJsonValueRef &entryRef : array) {
        DatabaseEntry entry(entryRef.toObject());
        _dbEntryKeys.append(entry.uid());
        _dbEntries[entry.uid()] = std::move(entry);
    }
}
