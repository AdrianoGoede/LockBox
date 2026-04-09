#ifndef DATABASE_H
#define DATABASE_H

#include <QHash>
#include <QUuid>
#include <QList>
#include <QFile>
#include <QString>
#include <QObject>
#include "SecureBuffer.h"
#include "DatabaseGroup.h"
#include "DatabaseEntry.h"

struct DatabaseSettings {
    quint64 kdfMemory;
    quint32 kdfIterations, kdfParallelism;
    bool saveOnModification, saveOnLocking;
    int clearClipboardAfter, lockAfter;
    SecureBuffer<QChar> password;
};

class Database : public QObject
{
    Q_OBJECT

public:
    Database(const QString& filePath, const DatabaseSettings& newDatabaseSettings, QObject* parent = nullptr);
    Database(const QString& filePath, const SecureBuffer<QChar>& password, QObject* parent = nullptr);
    ~Database() = default;
    void save();
    void saveAs(const QString& path);
    void addEntry(const DatabaseEntryDto& entryDto);
    void addGroup(const DatabaseGroupDto& groupDto);
    void editEntry(const QUuid& entryUid, const DatabaseEntryDto& entryDto);
    void editGroup(const QUuid& groupUid, const DatabaseGroupDto& groupDto);
    void moveEntry(const QUuid& entry, const QUuid& group);
    void removeEntry(const QUuid& uid);
    void moveGroup(const QUuid& group, const QUuid& newParent);
    void removeGroup(const QUuid& uid);
    size_t entryCount() const;
    size_t groupCount() const;
    SecureBuffer<QChar> entryUsername(const QUuid& entryUid) const;
    SecureBuffer<QChar> entryPassword(const QUuid& entryUid) const;
    SecureBuffer<QChar> entryNotes(const QUuid& entryUid) const;
    SecureBuffer<QChar> entryHistoryItemUsername(const QUuid& entryUid, const QUuid& historyItemUid) const;
    SecureBuffer<QChar> entryHistoryItemPassword(const QUuid& entryUid, const QUuid& historyItemUid) const;
    const DatabaseEntry* entry(const QUuid& uid) const;
    const DatabaseEntry* entry(int index) const;
    const DatabaseGroup* group(const QUuid& uid) const;
    const DatabaseGroup* group(int index) const;
    QVector<const DatabaseGroup*> childrenOfGroup(const DatabaseGroup* group) const;
    QVector<QUuid> entriesOfGroup(const QUuid& group) const;
    qsizetype indexOfEntry(const QUuid& uid) const;
    qsizetype indexOfGroup(const QUuid& uid) const;
    DatabaseSettings settings() const;
    void setSettings(const DatabaseSettings& settings);

signals:
    void entryAdded(qsizetype row, QUuid entryUuid);
    void entryRemoved(qsizetype row, QUuid entryUuid);
    void entryEdited(qsizetype row, QUuid entryUuid);
    void entryMoved(qsizetype row, QUuid entryUuid);
    void groupAdded(qsizetype row, QUuid groupUuid);
    void groupRemoved(qsizetype row, QUuid groupUuid);
    void groupEdited(qsizetype row, QUuid groupUuid);
    void groupMoved(qsizetype row, QUuid groupUuid);
    void databaseCleared();

private slots:
    void handleDatabaseStateChange();

private:
    QString _filePath;
    SecureBuffer<std::byte> _masterKey;
    quint64 _kdfMemory;
    quint32 _kdfIterations, _kdfParallelism;
    QByteArray _kdfSalt, _cryptoNonce;
    bool _saveOnModification, _saveOnLocking;
    int _clearClipboardAfter, _lockAfter;
    QHash<QUuid, DatabaseGroup> _dbGroups;
    QHash<QUuid, DatabaseEntry> _dbEntries;
    QList<QUuid> _dbGroupKeys, _dbEntryKeys;
    void loadData(const SecureBuffer<std::byte>& data);
    qsizetype calculateBodySize() const;
    QByteArray encryptedBody(const QByteArray& associatedData);
};

#endif // DATABASE_H
