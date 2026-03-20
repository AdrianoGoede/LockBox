#ifndef CRYPTO_H
#define CRYPTO_H

#include "SecureQByteArray.h"
#include "SecureBuffer.h"
#include <QVector>
#include <chrono>

namespace Crypto {
    QByteArray encrypt(const SecureBuffer<std::byte>& plaintext, const SecureBuffer<std::byte>& key, const QByteArray& nonce);
    SecureBuffer<std::byte> decrypt(const QByteArray& ciphertext, const SecureBuffer<std::byte>& key, const QByteArray& nonce);
    SecureBuffer<std::byte> deriveKey(const SecureBuffer<std::byte>& password, const QByteArray& salt, quint64 memoryKib, quint32 iterations, quint32 parallelism);
    void tuneArgon2idParams(std::chrono::milliseconds targetDelay, quint64& memoryKiB, quint32& iterations, quint32& parallelism);
    SecureBuffer<std::byte> generateKey();
    SecureBuffer<QChar> generateRandomPassword(const QVector<QChar>& charset, qsizetype length);
    QByteArray generateNonce();
    QByteArray generateSalt();
    SecureBuffer<quint32> generateRandomUnsignedIntegers(quint32 upperBound,qsizetype count);
    SecureBuffer<std::byte> qCharToByte(const SecureBuffer<QChar>& input);
    SecureBuffer<QChar> byteToQChar(const SecureBuffer<std::byte>& input);
}

#endif // CRYPTO_H
