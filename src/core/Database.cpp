#include "Database.h"
#include "Crypto.h"
#include "../config/Constants.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

Database::Database(const QString& filePath, const SecureQByteArray& password, std::chrono::milliseconds unlockDelay)
{
    _dbFile = std::make_unique<QFile>(new QFile(filePath));
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

    // Crypto::deriveKey(password, _kdfSalt, unlockDelay, _masterKey);
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

    loadHeader(doc.object()["header"].toObject());
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

    SecureQByteArray encryptedData;
    Crypto::encrypt(
        SecureQByteArray(qCompress(QJsonDocument(dataObj).toJson(QJsonDocument::JsonFormat::Compact), _compressionLevel)),
        _masterKey,
        encryptedData,
        _cryptoNonce
    );

    QJsonObject jsonObj;
    jsonObj["header"] = QJsonObject{
        { "kdf", QJsonObject {
            { "memory", _kdfMemory },
            { "iterations", _kdfIterations },
            { "parallelism", _kdfParallelism },
            { "salt", QString(_kdfSalt.toBase64()) }
        }},
        { "crypto", QJsonObject {
            { "nonce", QString(_cryptoNonce.toBase64()) }
        }},
        { "compression", QJsonObject {
            { "level", _compressionLevel }
        }}
    };
    jsonObj["data"] = QString(encryptedData.toBase64());

    try {
        _dbFile->startTransaction();
        _dbFile->write(QJsonDocument(jsonObj).toJson(QJsonDocument::JsonFormat::Compact));
        _dbFile->commitTransaction();
    }
    catch (...) {
        _dbFile->rollbackTransaction();
        throw;
    }
}

void Database::loadHeader(const QJsonObject& header)
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
        _dbGroups[group.uid()] = group;
    }

    array = doc.object()["entries"].toArray();
    for (const QJsonValueRef& entryRef : array) {
        DatabaseEntry entry(entryRef.toObject());
        _dbEntries[entry.uid()] = entry;
    }
}
