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

    _header.compressionLevel = Config::constants::DEFAULT_COMPRESSION_LEVEL;
    _header.kdfMemory = Config::constants::DEFAULT_KDF_MEMORY;
    _header.kdfIterations = Config::constants::DEFAULT_KDF_ITERATIONS;
    _header.kdfParallelism = Config::constants::DEFAULT_KDF_PARALLELISM;

    _settings.saveOnModification = Config::constants::DEFAULT_SAVE_ON_MODIFICATION;
    _settings.saveOnLocking = Config::constants::DEFAULT_SAVE_ON_LOCKING;
    _settings.lockOnMinimize = Config::constants::DEFAULT_LOCK_ON_MINIMIZE;
    _settings.lockOnScreenLocking = Config::constants::DEFAULT_LOCK_ON_SCREEN_LOCKING;
    _settings.clearClipboardAfter = Config::constants::DEFAULT_CLEAR_CLIPBOARD_AFTER;
    _settings.lockAfter = Config::constants::DEFAULT_LOCK_AFTER;

    Crypto::generateSalt(_header.kdfSalt);
    Crypto::tuneArgon2idParams(unlockDelay, _header.kdfMemory, _header.kdfIterations, _header.kdfParallelism);
    Crypto::deriveKey(password, _header.kdfSalt, _header.kdfMemory, _header.kdfIterations, _header.kdfParallelism, _masterKey);

    DatabaseGroup rootGroup;
    rootGroup.setTitle("Root");
    addGroup(rootGroup);

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
    loadBody(QByteArray::fromBase64(doc.object()["body"].toString().toUtf8()));
}

void Database::save()
{
    QJsonArray dataGroups;
    for (const DatabaseGroup& group : _dbGroups)
        dataGroups.append(group.toJson());
    QJsonArray dataEntries;
    for (const DatabaseEntry& entry : _dbEntries)
        dataEntries.append(entry.toJson());
    QJsonArray dataEntryHistory;
    for (const QList<DatabaseEntryHistoryItem>& itemList : _entryHistory)
        for (const DatabaseEntryHistoryItem& item : itemList)
            dataEntryHistory.append(item.toJson());

    QJsonObject bodyObj {
        { "settings", QJsonObject {
            { "saveOnModification", _settings.saveOnModification },
            { "saveOnLocking", _settings.saveOnLocking },
            { "lockOnMinimize", _settings.lockOnMinimize },
            { "lockOnScreenLocking", _settings.lockOnScreenLocking },
            { "clearClipboardAfter", _settings.clearClipboardAfter },
            { "lockAfter", _settings.lockAfter }
        }},
        { "data", QJsonObject {
            { "groups", dataGroups },
            { "entries", dataEntries },
            { "entryHistory", dataEntryHistory }
        }}
    };

    QByteArray encryptedData;
    Crypto::encrypt(
        SecureQByteArray(
            qCompress(QJsonDocument(bodyObj).toJson(QJsonDocument::JsonFormat::Compact),
            _header.compressionLevel)
        ),
        _masterKey,
        encryptedData,
        _header.cryptoNonce
    );

    QJsonObject jsonObj;
    jsonObj["header"] = QJsonObject{
        { "kdf", QJsonObject {
            { "memory", QJsonValue::fromVariant(_header.kdfMemory) },
            { "iterations", QJsonValue::fromVariant(_header.kdfIterations) },
            { "parallelism", QJsonValue::fromVariant(_header.kdfParallelism) },
            { "salt", QString(_header.kdfSalt.toBase64()) }
        }},
        { "crypto", QJsonObject {
            { "nonce", QString(_header.cryptoNonce.toBase64()) }
        }},
        { "compression", QJsonObject {
            { "level", QJsonValue::fromVariant(_header.compressionLevel) }
        }}
    };
    jsonObj["body"] = QString(encryptedData.toBase64());

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

    emit entryAdded((_dbEntryKeys.size() - 1), entry.uid());
}

void Database::addGroup(const DatabaseGroup& group)
{
    if (_dbGroups.contains(group.uid()))
        throw std::runtime_error("Group already exists");
    if (group.title().trimmed().isEmpty())
        throw std::runtime_error("Group must have a name");

    _dbGroupKeys.append(group.uid());
    _dbGroups[group.uid()] = group;
    emit groupAdded((_dbGroupKeys.size() - 1), group.uid());
}

void Database::editEntry(const DatabaseEntry& entry)
{
    if (!_dbEntries.contains(entry.uid()))
        throw std::runtime_error("Entry does not exist");
    recordHistory(entry.uid());
    _dbEntries[entry.uid()] = entry;
    emit entryEdited(_dbEntryKeys.indexOf(entry.uid()), entry.uid());
}

void Database::editGroup(const DatabaseGroup& group)
{
    if (!_dbGroups.contains(group.uid()))
        throw std::runtime_error("Group does not exist!");
    _dbGroups[group.uid()] = group;
    emit groupEdited(_dbGroupKeys.indexOf(group.uid()), group.uid());
}

void Database::removeEntry(const QUuid& uid)
{
    qsizetype row = _dbEntryKeys.indexOf(uid);
    if (row < 0) return;
    _dbEntryKeys.removeAt(row);
    _dbEntries.remove(uid);
    _entryHistory.remove(uid);
    emit entryRemoved(row, uid);
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
    emit groupRemoved(row, uid);
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

QList<DatabaseEntryHistoryItem> Database::entryHistory(const QUuid& entryUid) const { return _entryHistory.value(entryUid, QList<DatabaseEntryHistoryItem>()); }

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
    _header.kdfMemory = obj["memory"].toInt(Config::constants::DEFAULT_KDF_MEMORY);
    _header.kdfIterations = obj["iterations"].toInt(Config::constants::DEFAULT_KDF_ITERATIONS);
    _header.kdfParallelism = obj["parallelism"].toInt(Config::constants::DEFAULT_KDF_PARALLELISM);
    _header.kdfSalt = QByteArray::fromBase64(obj["salt"].toString().toUtf8());

    obj = header["compression"].toObject();
    if (obj.isEmpty()) throw std::runtime_error("Invalid or corrupted database file");
    _header.compressionLevel = obj["level"].toInt(Config::constants::DEFAULT_COMPRESSION_LEVEL);

    obj = header["crypto"].toObject();
    if (obj.isEmpty()) throw std::runtime_error("Invalid or corrupted database file");
    _header.cryptoNonce = QByteArray::fromBase64(obj["nonce"].toString().toUtf8());
    if (_header.cryptoNonce.isEmpty()) throw std::runtime_error("Invalid or corrupted database file");

    Crypto::deriveKey(password, _header.kdfSalt, _header.kdfMemory, _header.kdfIterations, _header.kdfParallelism, _masterKey);
}

void Database::loadBody(const QByteArray& body)
{
    SecureQByteArray plaintext;
    Crypto::decrypt(body, _masterKey, _header.cryptoNonce, plaintext);

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(qUncompress(plaintext), &parseError);
    if (parseError.error != QJsonParseError::ParseError::NoError)
        throw std::runtime_error("Invalid or corrupted database file");

    QJsonObject obj = doc.object()["settings"].toObject();
    loadSettings(obj);

    obj = doc.object()["data"].toObject();
    loadData(obj);
}

void Database::loadSettings(const QJsonObject& settings)
{
    _settings.saveOnModification = settings["saveOnModification"].toBool(Config::constants::DEFAULT_SAVE_ON_MODIFICATION);
    _settings.saveOnLocking = settings["saveOnLocking"].toBool(Config::constants::DEFAULT_SAVE_ON_LOCKING);
    _settings.lockOnMinimize = settings["lockOnMinimize"].toBool(Config::constants::DEFAULT_LOCK_ON_MINIMIZE);
    _settings.lockOnScreenLocking = settings["lockOnScreenLocking"].toBool(Config::constants::DEFAULT_LOCK_ON_SCREEN_LOCKING);
    _settings.clearClipboardAfter = settings["clearClipboardAfter"].toInt(Config::constants::DEFAULT_CLEAR_CLIPBOARD_AFTER);
    _settings.lockAfter = settings["lockAfter"].toInt(Config::constants::DEFAULT_LOCK_AFTER);
}

void Database::loadData(const QJsonObject& data)
{
    QJsonArray array = data["groups"].toArray();
    for (const QJsonValueRef& groupRef : array) {
        DatabaseGroup group(groupRef.toObject());
        _dbGroupKeys.append(group.uid());
        _dbGroups[group.uid()] = std::move(group);
    }

    array = data["entries"].toArray();
    for (const QJsonValueRef& entryRef : array) {
        DatabaseEntry entry(entryRef.toObject());
        _dbEntryKeys.append(entry.uid());
        _dbEntries[entry.uid()] = std::move(entry);
    }

    array = data["entryHistory"].toArray();
    for (const QJsonValueRef& entryRef : array) {
        DatabaseEntryHistoryItem item(entryRef.toObject());
        if (!_entryHistory.contains(item.entryUid()))
            _entryHistory[item.entryUid()] = QList<DatabaseEntryHistoryItem>();
        _entryHistory[item.entryUid()].append(DatabaseEntryHistoryItem(entryRef.toObject()));
    }
}

void Database::recordHistory(const QUuid& entryUid)
{
    if (!_entryHistory.contains(entryUid))
        _entryHistory[entryUid] = QList<DatabaseEntryHistoryItem>();
    _entryHistory[entryUid].append(DatabaseEntryHistoryItem(_dbEntries[entryUid]));
    while (_entryHistory[entryUid].size() > Config::constants::MAX_DB_ENTRY_HISTORY_ITEMS)
        _entryHistory[entryUid].removeFirst();
}
