#ifndef DATABASESETTINGSMANAGER_H
#define DATABASESETTINGSMANAGER_H

#include <QDialog>
#include "../core/Database.h"

namespace Ui {
    class DatabaseSettingsManager;
}

class DatabaseSettingsManager : public QDialog
{
    Q_OBJECT

public:
    explicit DatabaseSettingsManager(const Database* database, QWidget *parent = nullptr);
    ~DatabaseSettingsManager();

private:
    Ui::DatabaseSettingsManager* ui;
    const Database* _database = nullptr;
};

#endif // DATABASESETTINGSMANAGER_H
