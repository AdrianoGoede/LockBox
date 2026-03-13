#ifndef CRYPTO_H
#define CRYPTO_H

#include "SecureQByteArray.h"
#include <QVector>
#include <chrono>

namespace Crypto {
    void encrypt(const SecureQByteArray& plaintext, const SecureQByteArray& key, QByteArray& ciphertext, QByteArray& nonce);
    void decrypt(const QByteArray& ciphertext, const SecureQByteArray& key, const QByteArray& nonce, SecureQByteArray& plaintext);
    void deriveKey(const SecureQByteArray& password, const QByteArray& salt, quint64 memoryKiB, quint32 iterations, quint32 parallelism, SecureQByteArray& key);
    void tuneArgon2idParams(std::chrono::milliseconds targetDelay, quint64& memoryKiB, quint32& iterations, quint32& parallelism);
    void generateKey(SecureQByteArray& key);
    void generateRandomPassword(const QVector<char>& charset, qsizetype length, SecureQByteArray& out);
    void generateNonce(QByteArray& nonce);
    void generateSalt(QByteArray& salt);
    QVector<quint32> generateRandomUnsignedIntegers(quint32 upperBound,qsizetype count);
}

#endif // CRYPTO_H
