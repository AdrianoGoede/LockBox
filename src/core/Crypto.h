#ifndef CRYPTO_H
#define CRYPTO_H

#include "SecureQByteArray.h"
#include <chrono>

namespace Crypto {
    void encrypt(const SecureQByteArray& plaintext, const SecureQByteArray& key, SecureQByteArray& ciphertext, QByteArray& nonce);
    void decrypt(const QByteArray& ciphertext, const SecureQByteArray& key, const QByteArray& nonce, SecureQByteArray& plaintext);
    void deriveKey(const SecureQByteArray& password, const QByteArray& salt, std::chrono::milliseconds delay, SecureQByteArray& key);
    void generateKey(SecureQByteArray& key);
    void generateNonce(QByteArray& nonce);
    void generateSalt(QByteArray& salt);
}

#endif // CRYPTO_H
