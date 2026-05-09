#include "DatabaseSettingsManager.h"
#include "ui_DatabaseSettingsManager.h"
#include "../config/Constants.h"
#include "../core/Crypto.h"
#include "PasswordDialog.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QFuture>

DatabaseSettingsManager::DatabaseSettingsManager(DatabaseSettings& settings, const DatabaseSettings* existingSettings, QWidget* parent) : QDialog(parent), ui(new Ui::DatabaseSettingsManager), _settings(settings), _existingSettings(existingSettings)
{
    ui->setupUi(this);
    ui->sbClearClipboardAfter->setMinimum(Config::constants::MIN_CLIPBOARD_TIME);
    ui->sbClearClipboardAfter->setMaximum(Config::constants::MAX_CLIPBOARD_TIME);
    ui->sbLockAfterInactivity->setMinimum(Config::constants::MIN_LOCK_AFTER);
    ui->sbLockAfterInactivity->setMaximum(Config::constants::MAX_LOCK_AFTER);
    ui->sbSetUnlockTime->setMinimum(Config::constants::MIN_UNLOCK_TIME);
    ui->sbSetUnlockTime->setMaximum(Config::constants::MAX_UNLOCK_TIME);
    ui->sbSetUnlockTime->setValue(Config::constants::DEFAULT_UNLOCK_TIME);
    ui->sbKdfMemory->setMinimum(Config::constants::MIN_KDF_MEMORY / 1024);
    ui->sbKdfMemory->setMaximum(Config::constants::MAX_KDF_MEMORY / 1024);
    ui->sbKdfIterations->setMinimum(Config::constants::MIN_KDF_ITERATIONS);
    ui->sbKdfIterations->setMaximum(Config::constants::MAX_KDF_ITERATIONS);
    ui->sbKdfParallelism->setMinimum(Config::constants::MIN_KDF_PARALLELISM);
    ui->sbKdfParallelism->setMaximum(Config::constants::MAX_KDF_PARALLELISM);
    setKdfSettingsEnabled(existingSettings == nullptr);

    connect(ui->cbClearClipboardAfter, &QCheckBox::checkStateChanged, ui->sbClearClipboardAfter, &QSpinBox::setEnabled);
    connect(ui->cbLockAfterInactivity, &QCheckBox::checkStateChanged, ui->sbLockAfterInactivity, &QSpinBox::setEnabled);
    connect(ui->pbChangePassword, &QAbstractButton::clicked, this, &DatabaseSettingsManager::changePassword);
    connect(ui->pbSetUnlockTime, &QAbstractButton::clicked, this, &DatabaseSettingsManager::setUnlockTime);

    ui->cbAutosave->setChecked(_existingSettings ? _existingSettings->saveOnModification : Config::constants::DEFAULT_SAVE_ON_MODIFICATION);
    ui->cbClearClipboardAfter->setChecked(_existingSettings ? (_existingSettings->clearClipboardAfter > 0) : (Config::constants::DEFAULT_CLIPBOARD_TIME > 0));
    ui->sbClearClipboardAfter->setValue(_existingSettings ? _existingSettings->clearClipboardAfter : Config::constants::DEFAULT_CLIPBOARD_TIME);
    ui->cbLockAfterInactivity->setChecked(_existingSettings ? (_existingSettings->lockAfter > 0) : (Config::constants::DEFAULT_LOCK_AFTER > 0));
    ui->sbLockAfterInactivity->setValue(_existingSettings ? _existingSettings->lockAfter : Config::constants::DEFAULT_LOCK_AFTER);
    ui->sbKdfMemory->setValue((_existingSettings ? _existingSettings->kdfMemory : Config::constants::DEFAULT_KDF_MEMORY) / 1024);
    ui->sbKdfIterations->setValue(_existingSettings ? _existingSettings->kdfIterations : Config::constants::DEFAULT_KDF_ITERATIONS);
    ui->sbKdfParallelism->setValue(_existingSettings ? _existingSettings->kdfParallelism : Config::constants::DEFAULT_KDF_PARALLELISM);
}

DatabaseSettingsManager::~DatabaseSettingsManager() { delete ui; }

void DatabaseSettingsManager::changePassword()
{
    PasswordDialog passwordGenerator(_settings.password, this);
    if (passwordGenerator.exec() == QDialog::DialogCode::Accepted) {
        setKdfSettingsEnabled(true);
        ui->pbChangePassword->setText("New password set!");
    }
    else {
        _settings.password = SecureBuffer<QChar>();
        setKdfSettingsEnabled(false);
    }
}

void DatabaseSettingsManager::setUnlockTime()
{
    setKdfParamTuningRunning(true);

    _kdfTuningCancelled = std::make_shared<std::atomic<bool>>(false);
    std::shared_ptr<std::atomic<bool>> cancelled = _kdfTuningCancelled;

    _kdfTuningFuture = QtConcurrent::run([=]() {
        std::chrono::milliseconds msecs(ui->sbSetUnlockTime->value() * 1000);
        quint64 memory;
        quint32 iterations, parallelism;

        try {
            Crypto::tuneArgon2idParams(msecs, memory, iterations, parallelism, cancelled);
        }
        catch (...) {
            if (*cancelled) return;
            QMetaObject::invokeMethod(this, [=] {
                QMessageBox::critical(this, "Error", "KDF parameter tuning failed", QMessageBox::StandardButton::Ok);
            });
            return;
        }

        if (*cancelled)
            return;

        QMetaObject::invokeMethod(this, [=] {
            ui->sbKdfMemory->setValue(memory / 1024);
            ui->sbKdfIterations->setValue(iterations);
            ui->sbKdfParallelism->setValue(parallelism);
            ui->sbKdfIterations->setValue(iterations);
            setKdfParamTuningRunning(false);
        });
    });
}

void DatabaseSettingsManager::accept()
{
    _settings.kdfMemory = (ui->sbKdfMemory->value() * 1024);
    _settings.kdfIterations = ui->sbKdfIterations->value();
    _settings.kdfParallelism = ui->sbKdfParallelism->value();
    _settings.saveOnModification = ui->cbAutosave->isChecked();
    _settings.clearClipboardAfter = (ui->cbClearClipboardAfter->isChecked() ? ui->sbClearClipboardAfter->value() : 0);
    _settings.lockAfter = (ui->cbLockAfterInactivity->isChecked() ? ui->sbLockAfterInactivity->value() : 0);
    QDialog::accept();
}

void DatabaseSettingsManager::reject()
{
    _settings.password = SecureBuffer<QChar>();
    if (_kdfTuningCancelled)
        _kdfTuningCancelled->store(true);
    if (_kdfTuningFuture.isRunning())
        _kdfTuningFuture.waitForFinished();
    QDialog::reject();
}

void DatabaseSettingsManager::setKdfSettingsEnabled(bool enabled)
{
    ui->pbChangePassword->setEnabled(!enabled);
    ui->pbSetUnlockTime->setEnabled(enabled);
    ui->sbSetUnlockTime->setEnabled(enabled);
    ui->sbKdfMemory->setEnabled(enabled);
    ui->sbKdfIterations->setEnabled(enabled);
    ui->sbKdfParallelism->setEnabled(enabled);
}

void DatabaseSettingsManager::setKdfParamTuningRunning(bool running)
{
    ui->pbSetUnlockTime->setEnabled(!running);
    ui->pbSetUnlockTime->setText(running ? " Calculating KDF params..." : " Set Unlock Time");
    ui->sbSetUnlockTime->setEnabled(!running);
    ui->sbKdfMemory->setEnabled(!running);
    ui->sbKdfIterations->setEnabled(!running);
    ui->sbKdfParallelism->setEnabled(!running);
    ui->sbKdfIterations->setEnabled(!running);

    if (QPushButton* button = ui->buttonBox->button(QDialogButtonBox::StandardButton::Save))
        button->setEnabled(!running);
}
