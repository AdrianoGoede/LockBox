#include "Crypto.h"
#include "../config/Constants.h"
#include <sodium.h>
#include <QString>
#include <QStringConverter>

QByteArray Crypto::encrypt(const SecureBuffer<std::byte>& plaintext, const SecureBuffer<std::byte>& key, const QByteArray& nonce)
{
    QByteArray ciphertext;
    ciphertext.resize(plaintext.size() + crypto_aead_aes256gcm_ABYTES);

    quint64 ciphertextLength;
    int result = crypto_aead_aes256gcm_encrypt(
        reinterpret_cast<u_char*>(ciphertext.data()),
        &ciphertextLength,
        reinterpret_cast<const u_char*>(plaintext.data()),
        plaintext.size(),
        nullptr,
        0,
        nullptr,
        reinterpret_cast<const u_char*>(nonce.constData()),
        reinterpret_cast<const u_char*>(key.data())
    );
    if (result != 0) {
        ciphertext.clear();
        throw std::runtime_error("AES-256-GCM encryption failed");
    }

    return ciphertext;
}

SecureBuffer<std::byte> Crypto::decrypt(const QByteArray& ciphertext, const SecureBuffer<std::byte>& key, const QByteArray& nonce)
{
    if (ciphertext.size() < crypto_aead_aes256gcm_ABYTES)
        throw std::runtime_error("Ciphertext invalid");

    SecureBuffer<std::byte> plaintext(ciphertext.size() - crypto_aead_aes256gcm_ABYTES);

    quint64 plaintextLength;
    int result = crypto_aead_aes256gcm_decrypt(
        reinterpret_cast<u_char*>(plaintext.data()),
        &plaintextLength,
        nullptr,
        reinterpret_cast<const u_char*>(ciphertext.constData()),
        ciphertext.size(),
        nullptr,
        0,
        reinterpret_cast<const u_char*>(nonce.constData()),
        reinterpret_cast<const u_char*>(key.data())
    );
    if (result != 0)
        throw std::runtime_error("AES-256-GCM decryption failed (forged/invalid)");

    return plaintext;
}

SecureBuffer<std::byte> Crypto::deriveKey(const SecureBuffer<std::byte>& password, const QByteArray& salt, quint64 memoryKib, quint32 iterations, quint32 parallelism)
{
    SecureBuffer<std::byte> key(Config::constants::KEY_BYTES);

    int result = crypto_pwhash(
        reinterpret_cast<u_char*>(key.data()),
        key.size(),
        reinterpret_cast<const char*>(password.data()),
        password.size(),
        reinterpret_cast<const u_char*>(salt.constData()),
        iterations,
        (memoryKib * 1024ULL),
        crypto_pwhash_ALG_ARGON2ID13
    );

    if (result != 0)
        throw std::runtime_error("Argon2id benchmark failed");
    return key;
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

QByteArray Crypto::generateNonce()
{
    QByteArray nonce;
    nonce.resize(Config::constants::NONCE_BYTES);
    randombytes_buf(nonce.data(), nonce.size());
    return nonce;
}

QByteArray Crypto::generateSalt()
{
    QByteArray salt;
    salt.resize(Config::constants::SALT_BYTES);
    randombytes_buf(salt.data(), salt.size());
    return salt;
}

SecureBuffer<std::byte> Crypto::generateKey()
{
    SecureBuffer<std::byte> key(Config::constants::KEY_BYTES);
    randombytes_buf(key.data(), key.byteSize());
    return key;
}

SecureBuffer<QChar> Crypto::generateRandomPassword(const QVector<QChar>& charset, qsizetype length)
{
    SecureBuffer<QChar> result(length);
    for (qsizetype i = 0; i < length; i++) {
        qsizetype index = randombytes_uniform(charset.size());
        result[i] = charset.at(index);
    }
    return result;
}

SecureBuffer<quint32> Crypto::generateRandomUnsignedIntegers(quint32 upperBound, qsizetype count)
{
    SecureBuffer<quint32> result(count);
    for (qsizetype i = 0; i < count; i++)
        result[i] = randombytes_uniform(upperBound);
    return result;
}

SecureBuffer<std::byte> Crypto::qCharToByte(const SecureBuffer<QChar>& input)
{
    return SecureBuffer<std::byte>(0); // TO DO!!
}

SecureBuffer<QChar> Crypto::byteToQChar(const SecureBuffer<std::byte>& input)
{
    return SecureBuffer<QChar>(0); // TO DO!!
}
