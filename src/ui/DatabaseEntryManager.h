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
    explicit DatabaseEntryManager(DatabaseEntryDto* entryDto, const Database* database, const DatabaseEntry* entry, const DatabaseGroup* group, QWidget *parent = nullptr);
    ~DatabaseEntryManager();

private slots:
    void togglePasswordVisibility(bool visible);
    void copyUsernameToClipboard(const QModelIndex& index);
    void copyPasswordToClipboard(const QModelIndex& index);
    void accept() override;

private:
    Ui::DatabaseEntryManager* ui;
    DatabaseEntryDto* _entryDto = nullptr;
    const Database* _database = nullptr;
    const DatabaseEntry* _entry = nullptr;
    const DatabaseGroup* _group = nullptr;
    void setDataFields();
    void setHistoryTable();
    void getUsername();
    void getPassword();
    void getNotes();
};

#endif // DATABASEENTRYMANAGER_H
