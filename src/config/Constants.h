#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QtGlobal>
#include <QString>
#include <sodium.h>
#include <chrono>

namespace Config::constants {
    inline constexpr char FILE_FILTER[]                             = "LockBox Database (*.lbdb)";
    inline constexpr int MIN_PASSWORD_LENGTH                        = 8;
    inline constexpr int MAX_PASSWORD_LENGTH                        = 100;
    inline constexpr std::chrono::milliseconds DEFAULT_UNLOCK_DELAY = (std::chrono::milliseconds)1000;

    inline constexpr int SALT_BYTES                                 = crypto_pwhash_argon2id_SALTBYTES;
    inline constexpr int NONCE_BYTES                                = crypto_aead_aes256gcm_NPUBBYTES;
    inline constexpr int KEY_BYTES                                  = crypto_aead_aes256gcm_KEYBYTES;

    inline constexpr int MIN_COMPRESSION_LEVEL                      = 0;
    inline constexpr int MAX_COMPRESSION_LEVEL                      = 9;
    inline constexpr int DEFAULT_COMPRESSION_LEVEL                  = 6;
    inline constexpr int DEFAULT_KDF_MEMORY                         = 64;
    inline constexpr int DEFAULT_KDF_ITERATIONS                     = 3;
    inline constexpr int DEFAULT_KDF_PARALLELISM                    = 4;
    inline constexpr int MIN_CLIPBOARD_TIME                         = 5;
    inline constexpr int MAX_CLIPBOARD_TIME                         = 60;
    inline constexpr int DEFAULT_CLIPBOARD_TIME                     = 10;
    inline constexpr int MIN_LOCK_AFTER                             = 30;
    inline constexpr int MAX_LOCK_AFTER                             = 600;
    inline constexpr int DEFAULT_LOCK_AFTER                         = 60;
    inline constexpr bool DEFAULT_SAVE_ON_MODIFICATION              = false;
    inline constexpr bool DEFAULT_SAVE_ON_LOCKING                   = false;
    inline constexpr bool DEFAULT_LOCK_ON_MINIMIZE                  = false;
    inline constexpr bool DEFAULT_LOCK_ON_SCREEN_LOCKING            = false;

    inline constexpr char PASSWD_GEN_UPPERCASE_LETTERS[]            = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    inline constexpr char PASSWD_GEN_LOWERCASE_LETTERS[]            = "abcdefghijklmnopqrstuvwxyz";
    inline constexpr char PASSWD_GEN_NUMBERS[]                      = "0123456789";
    inline constexpr char PASSWD_GEN_PUNCTUATION[]                  = ".,:;?!";
    inline constexpr char PASSWD_GEN_SPECIAL_CHARS1[]               = "@#$%&";
    inline constexpr char PASSWD_GEN_SPECIAL_CHARS2[]               = "/|\\_-";
    inline constexpr char PASSWD_GEN_SPECIAL_CHARS3[]               = "<>*+-=";
    inline constexpr char PASSWD_GEN_SPECIAL_CHARS4[]               = "{[()]}";

    inline constexpr int MAX_DB_ENTRY_HISTORY_ITEMS                 = 20;
}

#endif // CONSTANTS_H
