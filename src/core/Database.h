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
#include "DatabaseEntryHistoryItem.h"
#include "../config/Constants.h"

struct DatabaseSettings {
    quint32 compressionLevel;
    bool saveOnModification, saveOnLocking;
    int clearClipboardAfter, lockAfter;
    SecureQByteArray password;
};

class Database : public QObject
{
    Q_OBJECT

public:
    Database(const QString& filePath, const SecureQByteArray& password, std::chrono::milliseconds unlockDelay = Config::constants::DEFAULT_UNLOCK_DELAY, QObject* parent = nullptr);
    ~Database();
    void create(const SecureQByteArray& password, std::chrono::milliseconds unlockDelay);
    void load(const SecureQByteArray& password);
    void save();
    void addEntry(const DatabaseEntry& entry);
    void addGroup(const DatabaseGroup& group);
    void editEntry(const DatabaseEntry& entry);
    void editGroup(const DatabaseGroup& group);
    void removeEntry(const QUuid& uid);
    void removeGroup(const QUuid& uid);
    size_t entryCount() const;
    size_t groupCount() const;
    const DatabaseEntry& entry(const QUuid& uid) const;
    const DatabaseEntry& entry(int index) const;
    const DatabaseGroup& group(const QUuid& uid) const;
    const DatabaseGroup& group(int index) const;
    QList<DatabaseEntryHistoryItem> entryHistory(const QUuid& entryUid) const;
    QVector<const DatabaseGroup*> childrenOfGroup(const DatabaseGroup* group) const;
    QVector<QUuid> entriesOfGroup(const QUuid& group) const;
    qsizetype indexOfEntry(const QUuid& uid) const;
    qsizetype indexOfGroup(const QUuid& uid) const;
    DatabaseSettings settings() const;
    void setSettings(const DatabaseSettings& settings);
    void changePassword(const SecureQByteArray& password);

signals:
    void entryAdded(qsizetype row, QUuid entryUuid);
    void entryRemoved(qsizetype row, QUuid entryUuid);
    void entryEdited(qsizetype row, QUuid entryUuid);
    void groupAdded(qsizetype row, QUuid groupUuid);
    void groupRemoved(qsizetype row, QUuid groupUuid);
    void groupEdited(qsizetype row, QUuid groupUuid);
    void databaseStateChanged();
    void databaseCleared();

private slots:
    void handleDatabaseStateChange();

private:
    std::unique_ptr<QFile> _dbFile = nullptr;
    SecureQByteArray _masterKey;
    quint64 _kdfMemory;
    quint32 _kdfIterations, _kdfParallelism, _compressionLevel;
    QByteArray _kdfSalt, _cryptoNonce;
    bool _saveOnModification, _saveOnLocking;
    int _clearClipboardAfter, _lockAfter;
    QHash<QUuid, DatabaseGroup> _dbGroups;
    QHash<QUuid, DatabaseEntry> _dbEntries;
    QList<QUuid> _dbGroupKeys, _dbEntryKeys;
    QHash<QUuid, QList<DatabaseEntryHistoryItem>> _entryHistory;
    void loadHeader(const QJsonObject& header, const SecureQByteArray& password);
    void loadBody(const QByteArray& body);
    void loadSettings(const QJsonObject& settings);
    void loadData(const QJsonObject& data);
    void recordHistory(const QUuid& entryUid);
};

#endif // DATABASE_H
