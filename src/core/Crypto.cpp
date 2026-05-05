#include "Crypto.h"
#include "../config/Constants.h"
#include <QtTypes>
#include <sodium.h>
#include <QString>
#include <QStringConverter>

QByteArray Crypto::encrypt(const SecureBuffer<std::byte>& plaintext, const SecureBuffer<std::byte>& key, const QByteArray& nonce, const QByteArray& associatedData)
{
    QByteArray ciphertext;
    ciphertext.resize(plaintext.size() + crypto_aead_aes256gcm_ABYTES);

    quint64 ciphertextLength;
    int result = crypto_aead_aes256gcm_encrypt(
        reinterpret_cast<uchar*>(ciphertext.data()),
        &ciphertextLength,
        reinterpret_cast<const uchar*>(plaintext.data()),
        plaintext.size(),
        reinterpret_cast<const uchar*>(associatedData.constData()),
        associatedData.size(),
        nullptr,
        reinterpret_cast<const uchar*>(nonce.constData()),
        reinterpret_cast<const uchar*>(key.data())
    );
    if (result != 0) {
        ciphertext.clear();
        throw std::runtime_error("AES-256-GCM encryption failed");
    }

    return ciphertext;
}

SecureBuffer<std::byte> Crypto::decrypt(const QByteArray& ciphertext, const SecureBuffer<std::byte>& key, const QByteArray& nonce, const QByteArray& associatedData)
{
    if (ciphertext.size() < crypto_aead_aes256gcm_ABYTES)
        throw std::runtime_error("Ciphertext invalid");

    SecureBuffer<std::byte> plaintext(ciphertext.size() - crypto_aead_aes256gcm_ABYTES);

    quint64 plaintextLength;
    int result = crypto_aead_aes256gcm_decrypt(
        reinterpret_cast<uchar*>(plaintext.data()),
        &plaintextLength,
        nullptr,
        reinterpret_cast<const uchar*>(ciphertext.constData()),
        ciphertext.size(),
        reinterpret_cast<const uchar*>(associatedData.constData()),
        associatedData.size(),
        reinterpret_cast<const uchar*>(nonce.constData()),
        reinterpret_cast<const uchar*>(key.data())
    );
    if (result != 0)
        throw std::runtime_error("AES-256-GCM decryption failed (forged/invalid)");

    return plaintext;
}

SecureBuffer<std::byte> Crypto::deriveKey(const SecureBuffer<std::byte>& password, const QByteArray& salt, quint64 memoryKib, quint32 iterations, quint32 parallelism)
{
    SecureBuffer<std::byte> key(Config::constants::KEY_BYTES);

    int result = crypto_pwhash(
        reinterpret_cast<uchar*>(key.data()),
        key.size(),
        reinterpret_cast<const char*>(password.data()),
        password.size(),
        reinterpret_cast<const uchar*>(salt.constData()),
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
            reinterpret_cast<uchar*>(dummyOutput.data()),
            Config::constants::SALT_BYTES,
            dummyPassword.toUtf8(),
            dummyPassword.size(),
            reinterpret_cast<const uchar*>(dummySalt.constData()),
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

void Crypto::zeroMemory(void* ptr, qsizetype count) { sodium_memzero(ptr, count); }

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
    if (input.size() == 0) return SecureBuffer<std::byte>(0);

    SecureBuffer<std::byte> tempBuffer(input.size() * 4);
    size_t outIdx = 0;

    for (qsizetype i = 0; i < input.size(); i++) {
        char32_t cp = input[i].unicode();

        if (input[i].isHighSurrogate() && (i + 1) < input.size() && input[i+1].isLowSurrogate()) {
            cp = QChar::surrogateToUcs4(input[i], input[i+1]);
            i++;
        }

        if (cp <= 0x7F) {
            tempBuffer[outIdx++] = static_cast<std::byte>(cp);
        }
        else if (cp <= 0x7FF) {
            tempBuffer[outIdx++] = static_cast<std::byte>(0xC0 | ((cp >> 6) & 0x1F));
            tempBuffer[outIdx++] = static_cast<std::byte>(0x80 | (cp & 0x3F));
        }
        else if (cp <= 0xFFFF) {
            tempBuffer[outIdx++] = static_cast<std::byte>(0xE0 | ((cp >> 12) & 0x0F));
            tempBuffer[outIdx++] = static_cast<std::byte>(0x80 | ((cp >> 6) & 0x3F));
            tempBuffer[outIdx++] = static_cast<std::byte>(0x80 | (cp & 0x3F));
        }
        else {
            tempBuffer[outIdx++] = static_cast<std::byte>(0xF0 | ((cp >> 18) & 0x07));
            tempBuffer[outIdx++] = static_cast<std::byte>(0x80 | ((cp >> 12) & 0x3F));
            tempBuffer[outIdx++] = static_cast<std::byte>(0x80 | ((cp >> 6) & 0x3F));
            tempBuffer[outIdx++] = static_cast<std::byte>(0x80 | (cp & 0x3F));
        }
    }

    SecureBuffer<std::byte> output(outIdx);
    std::memcpy(output.data(), tempBuffer.data(), output.byteSize());
    return output;
}

SecureBuffer<QChar> Crypto::byteToQChar(const SecureBuffer<std::byte>& input)
{
    if (input.size() == 0) return SecureBuffer<QChar>(0);

    SecureBuffer<QChar> tempBuffer(input.size());
    size_t outIdx = 0;

    for (qsizetype i = 0; i < input.size();) {
        uint32_t cp = 0;
        uint8_t b = static_cast<uint8_t>(input[i]);

        if (b <= 0x7F) {
            cp = b;
            i += 1;
        }
        else if ((b & 0xE0) == 0xC0 && (i + 1) < input.size()) {
            cp = (b & 0x1F) << 6;
            cp |= (static_cast<uint8_t>(input[i + 1]) & 0x3F);
            i += 2;
        }
        else if ((b & 0xF0) == 0xE0 && (i + 2) < input.size()) {
            cp = (b & 0x0F) << 12;
            cp |= (static_cast<uint8_t>(input[i + 1]) & 0x3F) << 6;
            cp |= (static_cast<uint8_t>(input[i + 2]) & 0x3F);
            i += 3;
        }
        else if ((b & 0xF8) == 0xF0 && (i + 3) < input.size()) {
            cp = (b & 0x07) << 18;
            cp |= (static_cast<uint8_t>(input[i + 1]) & 0x3F) << 12;
            cp |= (static_cast<uint8_t>(input[i + 2]) & 0x3F) << 6;
            cp |= (static_cast<uint8_t>(input[i + 3]) & 0x3F);
            i += 4;
        }
        else {
            i++;
            continue;
        }

        if (cp < 0x10000) {
            tempBuffer[outIdx++] = QChar(static_cast<ushort>(cp));
        } else {
            tempBuffer[outIdx++] = QChar(QChar::highSurrogate(cp));
            tempBuffer[outIdx++] = QChar(QChar::lowSurrogate(cp));
        }
    }

    SecureBuffer<QChar> output(outIdx);
    std::memcpy(output.data(), tempBuffer.data(), output.byteSize());
    return output;
}
