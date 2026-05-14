# LockBox

A secure, open source desktop password manager for Linux and Windows, built with C++17 and Qt 6.

## Architecture Overview

The codebase is organized into three layers:

**Core (`src/core/`)** — cryptography, database engine, data models, and Qt item models. No UI dependencies.

**UI (`src/ui/`)** — Qt dialogs and the main window. Depends on core, never the other way around.

**Config (`src/config/`)** — compile-time constants for KDF parameters, password constraints, and UI limits.

### Primitives

**AES-256-GCM** — authenticated encryption with associated data (AEAD). 
Chosen over plain AES-CBC because authentication is built in — 
ciphertext tampering is detected without a separate HMAC step. 
Hardware acceleration (AES-NI) makes it fast on any modern CPU.

**Argon2id** — memory-hard password hashing, winner of the Password 
Hashing Competition. Chosen over bcrypt and PBKDF2 because memory 
hardness resists GPU and ASIC-based brute force attacks. The hybrid 
id variant provides resistance against both side-channel and 
time-memory trade-off attacks.

### Key Classes

`SecureBuffer<T>` — a non-copyable, move-only buffer backed by `sodium_malloc`. Provides guard pages, stack canaries, `mlock` protection against swapping to disk, and guaranteed zeroing on destruction. Used for all sensitive data throughout the codebase.

`Crypto` — a stateless namespace wrapping libsodium. Provides AES-256-GCM encryption/decryption, Argon2id key derivation, secure random generation, and UTF-8 conversion between `SecureBuffer<QChar>` and `SecureBuffer<std::byte>`.

`Database` — the vault engine. Owns the master key as a `SecureBuffer<std::byte>`, manages entries and groups, handles Argon2id key derivation on open, and serializes the vault to a custom binary format using `QDataStream`. Saving uses `QSaveFile` for atomic writes.

`DatabaseEntry` — holds encrypted field data. Never stores plaintext. All sensitive fields are decrypted on demand by passing the master key at call time.

`Autotyper` — abstract base class for platform-specific keyboard injection. Implemented by `UnixX11Autotyper` (Linux/X11) and `WindowsAutotyper` (Windows). The correct implementation is selected at compile time via CMake.

---

## Cryptography

### Why libsodium

libsodium was chosen for the following reasons:

- Extensively battle-tested, open source, and independently audited
- Misuse-resistant API — it is difficult to use incorrectly by design
- For this application only two primitives are needed: AES-256-GCM and Argon2id. libsodium provides both with a clean, minimal API that does not require managing low-level cipher state
- `sodium_malloc` provides hardened memory allocation with no additional dependencies
- Cross-platform with consistent behavior across Linux and Windows

### File Format

The vault file is structured as follows:

```
[ kdfMemory | kdfIterations | kdfParallelism | kdfSalt | nonce | encryptedBody ]
```

The header fields (`kdfMemory`, `kdfIterations`, `kdfParallelism`, `kdfSalt`, `nonce`) are serialized in plaintext and passed as associated data to the AES-256-GCM encryption of the body. This means any tampering with the KDF parameters or nonce is detected and rejected during decryption — an attacker cannot weaken the key derivation parameters without invalidating the authentication tag.

### Key Wrapping

Each entry has its own randomly generated 256-bit entry key. This key is encrypted with the master key using AES-256-GCM, with the entry's UUID as associated data. The sensitive fields — username, password, notes — are then encrypted with the entry key, each with its own nonce and the entry UUID as associated data.

```
master password
      │
      ▼
  Argon2id (memory-hard KDF)
      │
      ▼
  master key (256-bit, sodium_malloc)
      │
      ▼
  entry key (256-bit, per-entry, AES-256-GCM encrypted)
      │
      ├── username  (AES-256-GCM, entry UUID as AAD)
      ├── password  (AES-256-GCM, entry UUID as AAD)
      └── notes     (AES-256-GCM, entry UUID as AAD)
```

This design has two important properties. First, a compromise of one entry key does not compromise the rest of the vault. Second, changing the master password only requires re-wrapping the entry keys — the field ciphertexts are never re-encrypted, making password changes O(n entries) rather than O(n fields).

The entry UUID as associated data binds each ciphertext to its entry, preventing an attacker from copying an encrypted field from one entry to another.

### KDF Parameter Tuning

Argon2id parameters (memory, iterations, parallelism) are tunable via a benchmark that targets a user-specified unlock time. The tuning runs a two-phase algorithm: first maximizing memory usage within half the time budget, then increasing iterations to consume the remaining budget. Parallelism is set to the number of logical CPU cores. The benchmark runs off the main thread to keep the UI responsive.

---

## Building

**Dependencies:** Qt 6, libsodium, libX11 + libXtst (Linux only)

```bash
# Debian/Ubuntu
sudo apt install qt6-base-dev libsodium-dev libx11-dev libxtst-dev

cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

---

## Third Party

| Library | License |
|---|---|
| [Qt 6](https://www.qt.io) | LGPL v3 |
| [libsodium](https://libsodium.org) | ISC |
| [Feather Icons](https://feathericons.com) | MIT |

---

## To-Do

**Security**
- [ ] Implement `MemoryReader` custom `QIODevice` to match `MemoryWriter` — currently `loadData` copies the decrypted vault body through an unprotected `QBuffer`
- [X] Move `sodium_init()` to `main()` with an explicit abort on failure
- [ ] Add bounds checking (`assert`) on `SecureBuffer::operator[]` in debug builds

**Core**
- [ ] `recordHistory()` should be called automatically inside `setPassword`, `setUsername`, `setNotes` rather than manually by the caller
- [ ] Add `QDataStream::status()` validation in all deserialization constructors (`DatabaseEntry`, `DatabaseGroup`, `DatabaseEntryHistoryItem`)
- [X] Fix `groupAdded` in `DatabaseGroupTreeModel` — uses `entryCount()` instead of `groupCount()`
- [ ] Fix `groupAdded` and `groupRemoved` to pass the correct parent `QModelIndex` for nested groups
- [ ] Add cycle detection in `DatabaseGroupTreeModel::dropMimeData` to prevent dropping a group onto its own descendant
- [X] Add `begin()`/`end()` iterators to `SecureBuffer` to support range-for

**UI**
- [X] Run `tuneArgon2idParams` off the main thread in `DatabaseSettingsManager` — currently blocks the UI
- [ ] Add unsaved changes indicator in the window title
- [ ] Add password strength indicator in `DatabaseEntryManager` and `PasswordGenerator`
- [ ] Add URL field to `DatabaseEntry` and `DatabaseEntryManager`
- [ ] Add global search across all groups
- [ ] Add system tray icon with quick lock/unlock
- [ ] Fix exception safety in `removeWordlist()` — items deleted before `buildWordlist()` succeeds

**Platform**
- [ ] Implement autotype for macOS (`MacAutotyper`) using `CGEventCreateKeyboardEvent`
- [X] Restore remapped keycode after use in `UnixX11Autotyper::typeQChar`
- [ ] Register `.lbdb` file association on Linux (`.desktop` + MIME type) and Windows (registry)
- [ ] Replace `QDir::currentPath()` with `QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)` in file dialogs
