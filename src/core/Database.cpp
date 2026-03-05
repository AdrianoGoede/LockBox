#include "Database.h"
#include "Crypto.h"
#include "../config/Constants.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSaveFile>

Database::Database(const NewDbConfig& newDbConfig, const DatabaseSettings& newDatabaseSettings, QObject* parent) : QObject{parent}, _filePath{newDbConfig.dbFilePath}
{
    setSettings(newDatabaseSettings);
    DatabaseGroup rootGroup;
    rootGroup.setTitle("Root");
    addGroup(rootGroup);
    save();
}

Database::Database(const QString& filePath, const SecureQByteArray& password, QObject* parent) : QObject{parent}, _filePath{filePath}
{
    QFile file(filePath);
    if (!file.open(QIODevice::OpenModeFlag::ReadOnly))
        throw std::runtime_error(QString("Could not open database file: %1").arg(file.errorString()).toUtf8());

    QByteArray payload = file.readAll();
    file.close();

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
            { "compressionLevel", QJsonValue::fromVariant(_compressionLevel) },
            { "saveOnModification", QJsonValue::fromVariant(_saveOnModification) },
            { "saveOnLocking", QJsonValue::fromVariant(_saveOnLocking) },
            { "clearClipboardAfter", QJsonValue::fromVariant(_clearClipboardAfter) },
            { "lockAfter", QJsonValue::fromVariant(_lockAfter) }
        }},
        { "data", QJsonObject {
            { "groups", QJsonValue::fromVariant(dataGroups) },
            { "entries", QJsonValue::fromVariant(dataEntries) },
            { "entryHistory", QJsonValue::fromVariant(dataEntryHistory) }
        }}
    };

    QByteArray encryptedData;
    Crypto::encrypt(
        SecureQByteArray(
            qCompress(QJsonDocument(bodyObj).toJson(QJsonDocument::JsonFormat::Compact),
            _compressionLevel)
        ),
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
        }}
    };
    jsonObj["body"] = QString(encryptedData.toBase64());

    QByteArray data = QJsonDocument(jsonObj).toJson(QJsonDocument::JsonFormat::Compact);
    QSaveFile file(_filePath);
    if (!file.open(QIODevice::OpenModeFlag::WriteOnly))
        throw std::runtime_error(QString("Could not open database file for saving: %1").arg(file.errorString()).toStdString());
    if (file.write(data) != data.size())
        throw std::runtime_error(QString("Could not write to databse file: %1").arg(file.errorString()).toStdString());
    if (!file.commit())
        throw std::runtime_error(QString("Could not save databse file: %1").arg(file.errorString()).toStdString());
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

void Database::moveEntry(const QUuid& entry, const QUuid& group)
{
    if (!_dbEntries.contains(entry))
        throw std::runtime_error("Entry does not exist");
    if (!_dbGroups.contains(group))
        throw std::runtime_error("Group does not exist");
    _dbEntries[entry].setGroup(group);
    emit entryMoved(_dbEntryKeys.indexOf(entry), entry);
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

void Database::moveGroup(const QUuid& group, const QUuid& newParent)
{
    if (!_dbGroups.contains(group))
        throw std::runtime_error("Group does not exist");
    if (!_dbGroups.contains(newParent))
        throw std::runtime_error("New parent group does not exist");
    _dbGroups[group].setParent(newParent);
    emit groupMoved(_dbGroupKeys.indexOf(group), group);
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

DatabaseSettings Database::settings() const
{
    return DatabaseSettings {
        _kdfMemory,
        _kdfIterations,
        _kdfParallelism,
        _compressionLevel,
        _saveOnModification,
        _saveOnLocking,
        _clearClipboardAfter,
        _lockAfter,
        SecureQByteArray()
    };
}

void Database::setSettings(const DatabaseSettings& settings)
{
    if (settings.kdfMemory < Config::constants::MIN_KDF_MEMORY || settings.kdfMemory > Config::constants::MAX_KDF_MEMORY)
        throw std::runtime_error(QString("KDF memory must be between %1 and %2 KiB").arg(Config::constants::MIN_KDF_MEMORY / 1024).arg(Config::constants::MAX_KDF_MEMORY / 1024).toStdString());
    if (settings.kdfIterations < Config::constants::MIN_KDF_ITERATIONS || settings.kdfIterations > Config::constants::MAX_KDF_ITERATIONS)
        throw std::runtime_error(QString("KDF iterations must be between %1 and %2").arg(Config::constants::MIN_KDF_ITERATIONS).arg(Config::constants::MAX_KDF_ITERATIONS).toStdString());
    if (settings.kdfParallelism < Config::constants::MIN_KDF_PARALLELISM || settings.kdfParallelism > Config::constants::MAX_KDF_PARALLELISM)
        throw std::runtime_error(QString("KDF paralellism must be between %1 and %2").arg(Config::constants::MIN_KDF_PARALLELISM).arg(Config::constants::MAX_KDF_PARALLELISM).toStdString());
    if (settings.compressionLevel < Config::constants::MIN_COMPRESSION_LEVEL || settings.compressionLevel > Config::constants::MAX_COMPRESSION_LEVEL)
        throw std::runtime_error(QString("Compression level must be between %1 and %2").arg(Config::constants::MIN_COMPRESSION_LEVEL).arg(Config::constants::MAX_COMPRESSION_LEVEL).toStdString());
    if (settings.clearClipboardAfter != 0 && (settings.clearClipboardAfter < Config::constants::MIN_CLIPBOARD_TIME || settings.clearClipboardAfter > Config::constants::MAX_CLIPBOARD_TIME))
        throw std::runtime_error(QString("Clipboard clearing time must be between %1 and %2").arg(Config::constants::MIN_CLIPBOARD_TIME).arg(Config::constants::MAX_CLIPBOARD_TIME).toStdString());
    if (settings.lockAfter != 0 && (settings.lockAfter < Config::constants::MIN_CLIPBOARD_TIME || settings.lockAfter > Config::constants::MAX_CLIPBOARD_TIME))
        throw std::runtime_error(QString("Clipboard clearing time must be between %1 and %2").arg(Config::constants::MIN_CLIPBOARD_TIME).arg(Config::constants::MAX_CLIPBOARD_TIME).toStdString());
    if (!settings.password.isEmpty() && (settings.password.size() < Config::constants::MIN_PASSWORD_LENGTH || settings.password.size() > Config::constants::MAX_PASSWORD_LENGTH))
        throw std::runtime_error(QString("Password length must be from %1 to %2").arg(Config::constants::MIN_PASSWORD_LENGTH).arg(Config::constants::MAX_PASSWORD_LENGTH).toStdString());

    _kdfMemory = settings.kdfMemory;
    _kdfIterations = settings.kdfIterations;
    _kdfParallelism = settings.kdfParallelism;
    _compressionLevel = settings.compressionLevel;
    _saveOnModification = settings.saveOnModification;
    _saveOnLocking = settings.saveOnLocking;
    _clearClipboardAfter = settings.clearClipboardAfter;
    _lockAfter = settings.lockAfter;

    if (!settings.password.isEmpty()) {
        Crypto::generateSalt(_kdfSalt);
        Crypto::deriveKey(settings.password, _kdfSalt, _kdfMemory, _kdfIterations, _kdfParallelism, _masterKey);
    }
}

void Database::handleDatabaseStateChange() { if (_saveOnModification) save(); }

void Database::loadHeader(const QJsonObject& header, const SecureQByteArray& password)
{
    QJsonObject obj = header["kdf"].toObject();
    if (obj.isEmpty()) throw std::runtime_error("Invalid or corrupted database file");
    _kdfMemory = obj["memory"].toInt(Config::constants::DEFAULT_KDF_MEMORY);
    _kdfIterations = obj["iterations"].toInt(Config::constants::DEFAULT_KDF_ITERATIONS);
    _kdfParallelism = obj["parallelism"].toInt(Config::constants::DEFAULT_KDF_PARALLELISM);
    _kdfSalt = QByteArray::fromBase64(obj["salt"].toString().toUtf8());

    obj = header["crypto"].toObject();
    if (obj.isEmpty()) throw std::runtime_error("Invalid or corrupted database file");
    _cryptoNonce = QByteArray::fromBase64(obj["nonce"].toString().toUtf8());
    if (_cryptoNonce.isEmpty()) throw std::runtime_error("Invalid or corrupted database file");

    Crypto::deriveKey(password, _kdfSalt, _kdfMemory, _kdfIterations, _kdfParallelism, _masterKey);
}

void Database::loadBody(const QByteArray& body)
{
    SecureQByteArray plaintext;
    Crypto::decrypt(body, _masterKey, _cryptoNonce, plaintext);

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
    _compressionLevel = settings["compressionLevel"].toInt(Config::constants::DEFAULT_COMPRESSION_LEVEL);
    _saveOnModification = settings["saveOnModification"].toBool(Config::constants::DEFAULT_SAVE_ON_MODIFICATION);
    _saveOnLocking = settings["saveOnLocking"].toBool(Config::constants::DEFAULT_SAVE_ON_LOCKING);
    _clearClipboardAfter = settings["clearClipboardAfter"].toInt(Config::constants::DEFAULT_CLIPBOARD_TIME);
    _lockAfter = settings["lockAfter"].toInt(Config::constants::DEFAULT_LOCK_AFTER);
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
