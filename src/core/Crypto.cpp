#include "Crypto.h"
#include "../config/Constants.h"
#include <sodium.h>

void Crypto::encrypt(const SecureQByteArray& plaintext, const SecureQByteArray& key, SecureQByteArray& ciphertext, QByteArray& nonce)
{
    ciphertext.wipe();
    ciphertext.resize((plaintext.size() + crypto_aead_aes256gcm_ABYTES), 0);
    generateNonce(nonce);

    unsigned long long ciphertextLength;
    int result = crypto_aead_aes256gcm_encrypt(
        reinterpret_cast<u_char*>(ciphertext.data()),
        &ciphertextLength,
        reinterpret_cast<const u_char*>(plaintext.constData()),
        plaintext.size(),
        nullptr,
        0,
        nullptr,
        reinterpret_cast<const u_char*>(nonce.constData()),
        reinterpret_cast<const u_char*>(key.constData())
    );

    if (result != 0) {
        nonce.clear();
        throw std::runtime_error("AES-256-GCM encryption failed");
    }
}

void Crypto::decrypt(const QByteArray& ciphertext, const SecureQByteArray& key, const QByteArray& nonce, SecureQByteArray& plaintext)
{
    if (ciphertext.size() < crypto_aead_aes256gcm_ABYTES)
        throw std::runtime_error("Ciphertext invalid");

    plaintext.wipe();
    plaintext.resize((ciphertext.size() - crypto_aead_aes256gcm_ABYTES), 0);

    unsigned long long plaintextLength;
    int result = crypto_aead_aes256gcm_decrypt(
        reinterpret_cast<u_char*>(plaintext.data()),
        &plaintextLength,
        nullptr,
        reinterpret_cast<const u_char*>(ciphertext.constData()),
        ciphertext.size(),
        nullptr,
        0,
        reinterpret_cast<const u_char*>(nonce.constData()),
        reinterpret_cast<const u_char*>(key.constData())
    );

    if (result != 0) {
        plaintext.wipe();
        throw std::runtime_error("AES-256-GCM decryption failed (forged/invalid)");
    }
}

void Crypto::deriveKey(const SecureQByteArray& password, const QByteArray& salt, std::chrono::milliseconds delay, SecureQByteArray& key)
{
    key.wipe();
    // To do
}

void Crypto::generateNonce(QByteArray& nonce)
{
    if (sodium_init() < 0) throw std::runtime_error("libsodium initialization failed");
    nonce.clear();
    nonce.resize(Config::constants::NONCE_BYTES, 0);
    randombytes_buf(nonce.data(), nonce.size());
}

void Crypto::generateSalt(QByteArray& salt)
{
    if (sodium_init() < 0) throw std::runtime_error("libsodium initialization failed");
    salt.clear();
    salt.resize(Config::constants::SALT_BYTES, 0);
    randombytes_buf(salt.data(), salt.size());
}

void Crypto::generateKey(SecureQByteArray& key)
{
    if (sodium_init() < 0) throw std::runtime_error("libsodium initialization failed");
    key.wipe();
    key.resize(Config::constants::KEY_BYTES);
    randombytes_buf(key.data(), key.size());
}
