#ifndef DATABASEENTRYMANAGER_H
#define DATABASEENTRYMANAGER_H

#include <QDialog>
#include "../core/DatabaseEntry.h"
#include "../core/DatabaseGroup.h"

namespace Ui {
    class DatabaseEntryManager;
}

class DatabaseEntryManager : public QDialog
{
    Q_OBJECT

public:
    explicit DatabaseEntryManager(DatabaseEntry* entry, const DatabaseGroup* group, const DatabaseEntry* existingEntry = nullptr, QWidget *parent = nullptr);
    ~DatabaseEntryManager();

private slots:
    void togglePasswordVisibility(bool visible);
    void accept() override;

private:
    Ui::DatabaseEntryManager* ui;
    DatabaseEntry* _entry = nullptr;
    const DatabaseEntry* _existingEntry = nullptr;
    const DatabaseGroup* _group = nullptr;
};

#endif // DATABASEENTRYMANAGER_H
