#include "DatabaseSettingsManager.h"
#include "ui_DatabaseSettingsManager.h"
#include "../config/Constants.h"
#include <QInputDialog>

DatabaseSettingsManager::DatabaseSettingsManager(DatabaseSettings& settings, QWidget* parent) : QDialog(parent), ui(new Ui::DatabaseSettingsManager), _settings(settings)
{
    ui->setupUi(this);
    ui->sbCompressionLevel->setMinimum(Config::constants::MIN_COMPRESSION_LEVEL);
    ui->sbCompressionLevel->setMaximum(Config::constants::MAX_COMPRESSION_LEVEL);
    ui->sbClearClipboardAfter->setMinimum(Config::constants::MIN_CLIPBOARD_TIME);
    ui->sbClearClipboardAfter->setMaximum(Config::constants::MAX_CLIPBOARD_TIME);
    ui->sbLockAfterInactivity->setMinimum(Config::constants::MIN_LOCK_AFTER);
    ui->sbLockAfterInactivity->setMaximum(Config::constants::MAX_LOCK_AFTER);

    connect(ui->cbClearClipboardAfter, &QCheckBox::checkStateChanged, ui->sbClearClipboardAfter, &QSpinBox::setEnabled);
    connect(ui->cbLockAfterInactivity, &QCheckBox::checkStateChanged, ui->sbLockAfterInactivity, &QSpinBox::setEnabled);
    connect(ui->pbChangePassword, &QAbstractButton::clicked, this, &DatabaseSettingsManager::changePassword);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    ui->cbAutosave->setChecked(_settings.saveOnModification);
    ui->sbCompressionLevel->setValue(_settings.compressionLevel);
    ui->cbClearClipboardAfter->setChecked(_settings.clearClipboardAfter > 0);
    ui->sbClearClipboardAfter->setValue(_settings.clearClipboardAfter);
    ui->cbLockAfterInactivity->setChecked(_settings.lockAfter > 0);
    ui->sbLockAfterInactivity->setValue(_settings.lockAfter);
}

DatabaseSettingsManager::~DatabaseSettingsManager() { delete ui; }

void DatabaseSettingsManager::changePassword()
{
    bool ok;
    _settings.password.wipe();
    _settings.password.append(QInputDialog::getText(
        this,
        "Enter the new password",
        QString(),
        QLineEdit::EchoMode::Password,
        QString(),
        &ok
    ).toUtf8());
    if (!ok)
        _settings.password.wipe();
}

void DatabaseSettingsManager::accept()
{
    _settings.compressionLevel = ui->sbCompressionLevel->value();
    _settings.saveOnModification = ui->cbAutosave->isChecked();
    _settings.clearClipboardAfter = (ui->cbClearClipboardAfter->isChecked() ? ui->sbClearClipboardAfter->value() : 0);
    _settings.lockAfter = (ui->cbLockAfterInactivity->isChecked() ? ui->sbLockAfterInactivity->value() : 0);
    QDialog::accept();
}

void DatabaseSettingsManager::reject()
{
    _settings.password.wipe();
    QDialog::reject();
}
