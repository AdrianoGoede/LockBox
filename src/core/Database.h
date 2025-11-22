#ifndef DATABASE_H
#define DATABASE_H

#include <QMap>
#include <QUuid>
#include <QFile>
#include <chrono>
#include <QString>
#include <QJsonObject>
#include "DatabaseGroup.h"
#include "DatabaseEntry.h"
#include "SecureQByteArray.h"
#include "../config/Constants.h"

class Database
{
public:
    Database(const QString& filePath, const SecureQByteArray& password, std::chrono::milliseconds unlockDelay = Config::constants::DEFAULT_UNLOCK_DELAY);
    ~Database();
    void create(const SecureQByteArray& password, std::chrono::milliseconds unlockDelay);
    void load(const SecureQByteArray& password);
    void save();

private:
    std::unique_ptr<QFile> _dbFile = nullptr;
    int _kdfMemory, _kdfIterations, _kdfParallelism, _compressionLevel;
    QByteArray _kdfSalt, _cryptoNonce;
    SecureQByteArray _masterKey;
    QMap<QUuid, DatabaseGroup> _dbGroups;
    QMap<QUuid, DatabaseEntry> _dbEntries;
    void loadHeader(const QJsonObject& header);
    void loadData(const QByteArray& data);
};

#endif // DATABASE_H
