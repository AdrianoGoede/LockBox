#include "Crypto.h"
#include "../config/Constants.h"
#include <sodium.h>
#include <QString>

void Crypto::encrypt(const SecureQByteArray& plaintext, const SecureQByteArray& key, QByteArray& ciphertext, QByteArray& nonce)
{
    ciphertext.clear();
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
        ciphertext.clear();
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

void Crypto::deriveKey(const SecureQByteArray& password, const QByteArray& salt, quint64 memoryKiB, quint32 iterations, quint32 parallelism, SecureQByteArray& key)
{
    if (sodium_init() < 0) throw std::runtime_error("libsodium init failed");

    key.wipe();
    key.resize(Config::constants::KEY_BYTES, 0);

    int result = crypto_pwhash(
        reinterpret_cast<u_char*>(key.data()),
        key.size(),
        password.constData(),
        password.size(),
        reinterpret_cast<const u_char*>(salt.constData()),
        iterations,
        (memoryKiB * 1024ULL),
        crypto_pwhash_ALG_ARGON2ID13
    );

    if (result != 0) {
        key.wipe();
        throw std::runtime_error("Argon2id benchmark failed");
    }
}

void Crypto::tuneArgon2idParams(std::chrono::milliseconds targetDelay, quint64& memoryKiB, quint32& iterations, quint32& parallelism)
{
    if (sodium_init() < 0) throw std::runtime_error("libsodium init failed");

    memoryKiB = (Config::constants::DEFAULT_KDF_MEMORY);
    iterations = Config::constants::DEFAULT_KDF_ITERATIONS;
    parallelism = Config::constants::DEFAULT_KDF_PARALLELISM;

    QByteArray dummySalt(Config::constants::SALT_BYTES, 'x');
    QByteArray dummyOutput(Config::constants::SALT_BYTES, 0);
    QString dummyPassword("test");

    auto benchmark = [&]() -> std::chrono::milliseconds {
        auto start = std::chrono::high_resolution_clock::now();
        int result = crypto_pwhash(
            reinterpret_cast<u_char*>(dummyOutput.data()),
            Config::constants::SALT_BYTES,
            dummyPassword.toUtf8(),
            dummyPassword.size(),
            reinterpret_cast<const u_char*>(dummySalt.constData()),
            iterations,
            (memoryKiB * 1024ULL),
            crypto_pwhash_ALG_ARGON2ID13
        );
        auto end = std::chrono::high_resolution_clock::now();

        if (result != 0)
            throw std::runtime_error("Argon2id benchmark failed");
        return std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    };

    while (benchmark() < targetDelay && memoryKiB <= Config::constants::MAX_KDF_MEMORY)
        memoryKiB *= 2;
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

void Crypto::generateRandomPassword(const QVector<char>& charset, qsizetype length, SecureQByteArray& out)
{
    if (sodium_init() < 0) throw std::runtime_error("libsodium initialization failed");
    out.wipe();
    out.resize(length);
    for (int i = 0; i < length; i++) {
        uint32_t index = randombytes_uniform(static_cast<uint32_t>(charset.size()));
        out[i] = charset.at(index);
    }
}

QVector<quint32> Crypto::generateRandomUnsignedIntegers(quint32 upperBound, qsizetype count)
{
    QVector<quint32> result;
    result.reserve(count);
    for (qsizetype i = 0; i < count; i++)
        result.append(randombytes_uniform(upperBound));
    return result;
}
