#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QtGlobal>
#include <sodium.h>
#include <chrono>

namespace Config::constants {
    inline constexpr int MIN_PASSWORD_LENGTH = 8;
    inline constexpr int MAX_PASSWORD_LENGTH = 100;
    inline constexpr std::chrono::milliseconds DEFAULT_UNLOCK_DELAY = (std::chrono::milliseconds)1000;

    inline constexpr int SALT_BYTES = crypto_pwhash_argon2id_SALTBYTES;
    inline constexpr int NONCE_BYTES = crypto_aead_aes256gcm_NPUBBYTES;
    inline constexpr int KEY_BYTES = crypto_aead_aes256gcm_KEYBYTES;

    inline constexpr int DEFAULT_COMPRESSION_LEVEL = 9;
    inline constexpr int DEFAULT_KDF_MEMORY = 64;
    inline constexpr int DEFAULT_KDF_ITERATIONS = 3;
    inline constexpr int DEFAULT_KDF_PARALLELISM = 4;
}

#endif // CONSTANTS_H
