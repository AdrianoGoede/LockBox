#ifndef DATABASEENTRYMANAGER_H
#define DATABASEENTRYMANAGER_H

#include <QDialog>
#include "../core/Database.h"
#include "../core/DatabaseEntry.h"

namespace Ui {
    class DatabaseEntryManager;
}

class DatabaseEntryManager : public QDialog
{
    Q_OBJECT

public:
    explicit DatabaseEntryManager(DatabaseEntry* entryDto, const Database* database, const QUuid& entryUid, const QUuid& groupUid);
    ~DatabaseEntryManager();

private slots:
    void togglePasswordVisibility(bool visible);
    void copyUsernameToClipboard(const QModelIndex& index);
    void copyPasswordToClipboard(const QModelIndex& index);
    void accept() override;

private:
    Ui::DatabaseEntryManager* ui;
    DatabaseEntry* _entryDto = nullptr;
    const Database* _database = nullptr;
    QUuid _entryUid;
    const DatabaseEntry* _entry = nullptr;
    const DatabaseGroup* _group = nullptr;
    void setDataFields();
    void setHistoryTable();
};

#endif // DATABASEENTRYMANAGER_H
