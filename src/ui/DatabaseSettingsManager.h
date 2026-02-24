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
    explicit DatabaseSettingsManager(DatabaseSettings& settings, const DatabaseSettings* existingSettings, QWidget* parent = nullptr);
    ~DatabaseSettingsManager();

private slots:
    void changePassword();
    void setUnlockTime();
    void accept() override;
    void reject() override;

private:
    Ui::DatabaseSettingsManager* ui;
    DatabaseSettings& _settings;
    const DatabaseSettings* _existingSettings = nullptr;
    void setKdfSettingsEnabled(bool enabled);
};

#endif // DATABASESETTINGSMANAGER_H
