#include "Database.h"
#include "Crypto.h"
#include "MemoryWriter.h"
#include "SizeOnlyDevice.h"
#include "../config/Constants.h"
#include <QDataStream>
#include <QSaveFile>
#include <QBuffer>

Database::Database(const QString& filePath, const DatabaseSettings& newDatabaseSettings, QObject* parent) : QObject(parent), _filePath(filePath)
{
    setSettings(newDatabaseSettings);
    DatabaseGroupDto rootGroup;
    rootGroup.title = "Root";
    addGroup(rootGroup);
    save();
}

Database::Database(const QString& filePath, const SecureBuffer<QChar>& password, QObject* parent) : QObject(parent), _filePath(filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::OpenModeFlag::ReadOnly))
        throw std::runtime_error(QString("Could not open database file: %1").arg(file.errorString()).toUtf8());
    QByteArray payload = file.readAll();
    file.close();

    QDataStream stream(&payload, QIODevice::OpenModeFlag::ReadOnly);
    stream.setVersion(QDataStream::Version::Qt_6_0);

    QByteArray encryptedBody;
    stream >> _kdfMemory >> _kdfIterations >> _kdfParallelism >> _kdfSalt >> _cryptoNonce >> encryptedBody;
    _masterKey = Crypto::deriveKey(Crypto::qCharToByte(password), _kdfSalt, _kdfMemory, _kdfIterations, _kdfParallelism);

    QByteArray associatedData;
    QDataStream associatedDataStream(&associatedData, QIODevice::OpenModeFlag::WriteOnly);
    associatedDataStream.setVersion(QDataStream::Version::Qt_6_0);
    associatedDataStream << _kdfMemory << _kdfIterations << _kdfParallelism << _kdfSalt << _cryptoNonce;

    loadData(Crypto::decrypt(encryptedBody, _masterKey, _cryptoNonce, associatedData));
}

void Database::save()
{
    _cryptoNonce = Crypto::generateNonce();

    QByteArray associatedData;
    QDataStream stream(&associatedData, QIODevice::OpenModeFlag::WriteOnly);
    stream.setVersion(QDataStream::Version::Qt_6_0);
    stream << _kdfMemory << _kdfIterations << _kdfParallelism << _kdfSalt << _cryptoNonce;

    QSaveFile file(_filePath);
    if (!file.open(QIODevice::OpenModeFlag::WriteOnly))
        throw std::runtime_error(QString("Could not open database file for saving: %1").arg(file.errorString()).toStdString());

    stream.resetStatus();
    stream.setDevice(&file);
    stream << _kdfMemory << _kdfIterations << _kdfParallelism << _kdfSalt << _cryptoNonce << encryptedBody(associatedData);

    if (!file.commit())
        throw std::runtime_error(QString("Could not save databse file: %1").arg(file.errorString()).toStdString());
}

void Database::saveAs(const QString& path)
{
    QString oldPath = _filePath;

    try {
        _filePath = path;
        save();
    }
    catch (...) {
        _filePath = oldPath;
        throw;
    }
}

void Database::addEntry(const DatabaseEntryDto& entryDto)
{
    DatabaseEntry entry(entryDto, _masterKey);
    _dbEntryKeys.append(entry.uid());
    _dbEntries[entry.uid()] = entry;
    emit entryAdded((_dbEntryKeys.size() - 1), entry.uid());
}

void Database::addGroup(const DatabaseGroupDto& groupDto)
{
    if (groupDto.title.trimmed().isEmpty())
        throw std::runtime_error("Group must have a name");

    DatabaseGroup group(groupDto);
    _dbGroupKeys.append(group.uid());
    _dbGroups[group.uid()] = group;
    emit groupAdded((_dbGroupKeys.size() - 1), group.uid());
}

void Database::editEntry(const QUuid& entryUid, const DatabaseEntryDto& entryDto)
{
    if (!_dbEntries.contains(entryUid))
        throw std::runtime_error("Entry does not exist");

    DatabaseEntry& entry = _dbEntries[entryUid];
    entry.recordHistory();

    entry.setTitle(entryDto.title);
    entry.setUsername(entryDto.username);
    entry.setNotes(entryDto.notes);
    entry.setPassword(entryDto.password, _masterKey);

    emit entryEdited(_dbEntryKeys.indexOf(entry.uid()), entry.uid());
}

void Database::editGroup(const QUuid& groupUid, const DatabaseGroupDto& groupDto)
{
    if (!_dbGroups.contains(groupUid))
        throw std::runtime_error("Group does not exist!");

    DatabaseGroup& group = _dbGroups[groupUid];
    group.setTitle(groupDto.title);

    emit groupEdited(_dbGroupKeys.indexOf(groupUid), groupUid);
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

SecureBuffer<QChar> Database::entryPassword(const QUuid& entryUid) const
{
    if (!_dbEntries.contains(entryUid))
        throw std::runtime_error("Entry does not exist");
    const DatabaseEntry& entry = _dbEntries.find(entryUid).value();
    return entry.password(_masterKey);
}

SecureBuffer<QChar> Database::entryHistoryItemPassword(const QUuid& entryUid, const QUuid& historyItemUid) const
{
    if (!_dbEntries.contains(entryUid))
        throw std::runtime_error("Entry does not exist");
    const DatabaseEntry& entry = _dbEntries.find(entryUid).value();
    const DatabaseEntryHistoryItem& item = entry.getHistoryItem(historyItemUid);
    return item.password(_masterKey);
}

const DatabaseEntry* Database::entry(const QUuid& uid) const
{
    if (!_dbEntries.contains(uid))
        return nullptr;
    return &_dbEntries.find(uid).value();
}

const DatabaseEntry* Database::entry(int index) const
{
    if (index >= _dbEntryKeys.size())
        return nullptr;
    const QUuid& uid = _dbEntryKeys[index];
    return &_dbEntries.find(uid).value();
}

const DatabaseGroup* Database::group(const QUuid &uid) const
{
    if (!_dbGroups.contains(uid))
        return nullptr;
    return &_dbGroups.find(uid).value();
}

const DatabaseGroup* Database::group(int index) const
{
    if (index >= _dbGroupKeys.size())
        return nullptr;
    const QUuid& uid = _dbGroupKeys[index];
    return &_dbGroups.find(uid).value();
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

DatabaseSettings Database::settings() const
{
    return DatabaseSettings {
        _kdfMemory,
        _kdfIterations,
        _kdfParallelism,
        _saveOnModification,
        _saveOnLocking,
        _clearClipboardAfter,
        _lockAfter,
        SecureBuffer<QChar>()
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
    if (settings.clearClipboardAfter != 0 && (settings.clearClipboardAfter < Config::constants::MIN_CLIPBOARD_TIME || settings.clearClipboardAfter > Config::constants::MAX_CLIPBOARD_TIME))
        throw std::runtime_error(QString("Clipboard clearing time must be between %1 and %2").arg(Config::constants::MIN_CLIPBOARD_TIME).arg(Config::constants::MAX_CLIPBOARD_TIME).toStdString());
    if (settings.lockAfter != 0 && (settings.lockAfter < Config::constants::MIN_CLIPBOARD_TIME || settings.lockAfter > Config::constants::MAX_CLIPBOARD_TIME))
        throw std::runtime_error(QString("Clipboard clearing time must be between %1 and %2").arg(Config::constants::MIN_CLIPBOARD_TIME).arg(Config::constants::MAX_CLIPBOARD_TIME).toStdString());
    if (!settings.password.isEmpty() && (settings.password.size() < Config::constants::MIN_PASSWORD_LENGTH || settings.password.size() > Config::constants::MAX_PASSWORD_LENGTH))
        throw std::runtime_error(QString("Password length must be from %1 to %2").arg(Config::constants::MIN_PASSWORD_LENGTH).arg(Config::constants::MAX_PASSWORD_LENGTH).toStdString());

    _kdfMemory = settings.kdfMemory;
    _kdfIterations = settings.kdfIterations;
    _kdfParallelism = settings.kdfParallelism;
    _saveOnModification = settings.saveOnModification;
    _saveOnLocking = settings.saveOnLocking;
    _clearClipboardAfter = settings.clearClipboardAfter;
    _lockAfter = settings.lockAfter;

    if (!settings.password.isEmpty()) {
        _kdfSalt = Crypto::generateSalt();
        _masterKey = Crypto::deriveKey(Crypto::qCharToByte(settings.password), _kdfSalt, _kdfMemory, _kdfIterations, _kdfParallelism);
    }
}

void Database::handleDatabaseStateChange() { if (_saveOnModification) save(); }

void Database::loadData(const SecureBuffer<std::byte>& data)
{
    QBuffer buffer;
    buffer.setData(reinterpret_cast<const char*>(data.data()), data.size());
    if (!buffer.open(QIODevice::OpenModeFlag::ReadOnly))
        throw std::runtime_error("Buffer could not be opened for reading");
    QDataStream stream(&buffer);
    stream.setVersion(QDataStream::Version::Qt_6_0);

    stream >> _saveOnModification >> _saveOnLocking >> _clearClipboardAfter >> _lockAfter;
    qsizetype size;

    stream >> size;
    for (qsizetype i = 0; i < size; i++) {
        DatabaseGroup group(stream);
        _dbGroupKeys.append(group.uid());
        _dbGroups[group.uid()] = std::move(group);
    }

    stream >> size;
    for (qsizetype i = 0; i < size; i++) {
        DatabaseEntry entry(stream);
        _dbEntryKeys.append(entry.uid());
        _dbEntries[entry.uid()] = std::move(entry);
    }
}

qsizetype Database::calculateBodySize() const
{
    SizeOnlyDevice dummyBuffer;
    QDataStream probe(&dummyBuffer);
    probe.setVersion(QDataStream::Version::Qt_6_0);

    probe << _saveOnModification << _saveOnLocking << _clearClipboardAfter << _lockAfter;

    probe << _dbGroupKeys.size();
    for (const QUuid& groupUid : _dbGroupKeys) {
        const DatabaseGroup& group = _dbGroups[groupUid];
        group.toBinary(probe);
    }

    probe << _dbEntryKeys.size();
    for (const QUuid& entryUid : _dbEntryKeys) {
        const DatabaseEntry& entry = _dbEntries[entryUid];
        entry.toBinary(probe);
    }

    return dummyBuffer.size();
}

QByteArray Database::encryptedBody(const QByteArray& associatedData)
{
    SecureBuffer<std::byte> plaintextData(calculateBodySize());
    MemoryWriter writer(plaintextData.data(), plaintextData.size());
    QDataStream stream(&writer);
    stream.setVersion(QDataStream::Version::Qt_6_0);

    stream << _saveOnModification << _saveOnLocking << _clearClipboardAfter << _lockAfter;

    stream << _dbGroupKeys.size();
    for (const QUuid& groupUid : _dbGroupKeys) {
        const DatabaseGroup& group = _dbGroups[groupUid];
        group.toBinary(stream);
    }

    stream << _dbEntryKeys.size();
    for (const QUuid& entryUid : _dbEntryKeys) {
        const DatabaseEntry& entry = _dbEntries[entryUid];
        entry.toBinary(stream);
    }

    return Crypto::encrypt(plaintextData, _masterKey, _cryptoNonce, associatedData);
}
