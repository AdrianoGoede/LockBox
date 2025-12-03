#ifndef DATABASE_H
#define DATABASE_H

#include <QHash>
#include <QUuid>
#include <QFile>
#include <QList>
#include <chrono>
#include <QString>
#include <QObject>
#include <QJsonObject>
#include "DatabaseGroup.h"
#include "DatabaseEntry.h"
#include "SecureQByteArray.h"
#include "../config/Constants.h"

class Database : public QObject
{
    Q_OBJECT

public:
    Database(const QString& filePath, const SecureQByteArray& password, std::chrono::milliseconds unlockDelay = Config::constants::DEFAULT_UNLOCK_DELAY);
    ~Database();
    void create(const SecureQByteArray& password, std::chrono::milliseconds unlockDelay);
    void load(const SecureQByteArray& password);
    void save();
    void addEntry(const QUuid& group, const QString& title, const QString& notes, const SecureQByteArray& password);
    void addGroup(const QUuid& parent, const QString& title);
    void removeEntry(const QUuid& uid);
    void removeGroup(const QUuid& uid);
    size_t entryCount() const;
    size_t groupCount() const;
    const DatabaseEntry& entry(const QUuid& uid) const;
    const DatabaseEntry& entry(int index) const;
    const DatabaseGroup& group(const QUuid& uid) const;
    const DatabaseGroup& group(int index) const;
    qsizetype indexOfEntry(const QUuid& uid) const;
    qsizetype indexOfGroup(const QUuid& uid) const;

signals:
    void entryAdded(qsizetype row);
    void entryRemoved(qsizetype row);
    void entryChanged(qsizetype row);
    void groupAdded(qsizetype row);
    void groupRemoved(qsizetype row);
    void groupChanged(qsizetype row);
    void databaseCleared();

private:
    std::unique_ptr<QFile> _dbFile = nullptr;
    quint64 _kdfMemory;
    quint32 _kdfIterations, _kdfParallelism, _compressionLevel;
    QByteArray _kdfSalt, _cryptoNonce;
    SecureQByteArray _masterKey;
    QHash<QUuid, DatabaseGroup> _dbGroups;
    QHash<QUuid, DatabaseEntry> _dbEntries;
    QList<QUuid> _dbGroupKeys, _dbEntryKeys;
    void loadHeader(const QJsonObject& header);
    void loadData(const QByteArray& data);
};

#endif // DATABASE_H
