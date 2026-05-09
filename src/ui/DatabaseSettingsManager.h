#ifndef DATABASESETTINGSMANAGER_H
#define DATABASESETTINGSMANAGER_H

#include <QDialog>
#include <QtConcurrent/QtConcurrent>
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
    std::shared_ptr<std::atomic<bool>> _kdfTuningCancelled = nullptr;
    QFuture<void> _kdfTuningFuture;
    void setKdfSettingsEnabled(bool enabled);
    void setKdfParamTuningRunning(bool running);
};

#endif // DATABASESETTINGSMANAGER_H
