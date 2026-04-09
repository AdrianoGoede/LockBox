#ifndef DATABASEENTRY_H
#define DATABASEENTRY_H

#include <QUuid>
#include <QString>
#include <QDateTime>
#include <QDataStream>
#include "SecureBuffer.h"
#include "DatabaseGroup.h"
#include "DatabaseEntryHistoryItem.h"

struct DatabaseEntryDto {
    QUuid group;
    QString title;
    SecureBuffer<QChar> username, password, notes;
};

class DatabaseEntry
{
public:
    DatabaseEntry();
    DatabaseEntry(const DatabaseEntryDto& entryDto, const SecureBuffer<std::byte>& masterKey);
    DatabaseEntry(QDataStream& in);
    QUuid uid() const;
    QUuid group() const;
    void setGroup(const QUuid& group);
    void setGroup(const DatabaseGroup& group);
    QString title() const;
    void setTitle(const QString& title);
    SecureBuffer<QChar> username(const SecureBuffer<std::byte>& masterKey) const;
    void setUsername(const SecureBuffer<QChar>& name, const SecureBuffer<std::byte>& masterKey);
    SecureBuffer<QChar> notes(const SecureBuffer<std::byte>& masterKey) const;
    void setNotes(const SecureBuffer<QChar>& notes, const SecureBuffer<std::byte>& masterKey);
    QDateTime createdAt() const;
    QDateTime modifiedAt() const;
    SecureBuffer<QChar> password(const SecureBuffer<std::byte>& masterKey) const;
    void setPassword(const SecureBuffer<QChar>& password, const SecureBuffer<std::byte>& masterKey);
    void recordHistory();
    const QVector<DatabaseEntryHistoryItem>& history() const;
    SecureBuffer<QChar> historyItemUsername(const QUuid& itemUid, const SecureBuffer<std::byte>& masterKey) const;
    SecureBuffer<QChar> historyItemPassword(const QUuid& itemUid, const SecureBuffer<std::byte>& masterKey) const;
    void toBinary(QDataStream& out) const;

private:
    QUuid _uid;
    QUuid _group;
    QString _title;
    QDateTime _createdAt, _modifiedAt;
    QByteArray _entryKeyNonce, _entryKey;
    QByteArray _usernameNonce, _username;
    QByteArray _passwordNonce, _password;
    QByteArray _notesNonce, _notes;
    QVector<DatabaseEntryHistoryItem> _history;
    void setUsernameOnInit(const SecureBuffer<QChar>& name, const SecureBuffer<std::byte>& entryKey);
    void setPasswordOnInit(const SecureBuffer<QChar>& password, const SecureBuffer<std::byte>& entryKey);
    void setNotesOnInit(const SecureBuffer<QChar>& notes, const SecureBuffer<std::byte>& entryKey);
};

#endif // DATABASEENTRY_H
