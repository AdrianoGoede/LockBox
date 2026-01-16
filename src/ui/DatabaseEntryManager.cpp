#include "DatabaseEntryManager.h"
#include "ui_DatabaseEntryManager.h"
#include <QMessageBox>

DatabaseEntryManager::DatabaseEntryManager(DatabaseEntry* entry, const DatabaseGroup* group, const DatabaseEntry* existingEntry, QWidget* parent) : QDialog(parent), ui(new Ui::DatabaseEntryManager), _entry(entry), _existingEntry(existingEntry), _group(group)
{
    if (!entry || !group)
        throw std::runtime_error("Entry instance and group must be informed");

    ui->setupUi(this);
    this->setWindowTitle(existingEntry ? "Edit Entry" : "Create Entry");
    ui->leTitle->setText(existingEntry ? existingEntry->title() : QString());
    ui->leUsername->setText(existingEntry ? existingEntry->username() : QString());
    ui->lePassword->setText(existingEntry ? existingEntry->password() : QString());
    ui->teNotes->setText(existingEntry ? existingEntry->notes() : QString());

    connect(ui->pbPasswordShow, &QAbstractButton::clicked, this, &DatabaseEntryManager::togglePasswordVisibility);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

DatabaseEntryManager::~DatabaseEntryManager() { delete ui; }

void DatabaseEntryManager::togglePasswordVisibility(bool visible) { ui->lePassword->setEchoMode(visible ? QLineEdit::EchoMode::Normal : QLineEdit::EchoMode::Password); }

void DatabaseEntryManager::accept()
{
    if (ui->leTitle->text().trimmed().isEmpty()) {
        QMessageBox::critical(
            this,
            "Error",
            "The entry must have a title!",
            QMessageBox::StandardButton::Ok,
            QMessageBox::StandardButton::Ok
        );
        return;
    }

    if (_existingEntry)
        _entry->setUid(_existingEntry->uid());
    _entry->setTitle(ui->leTitle->text());
    _entry->setGroup(_existingEntry ? _existingEntry->group() : _group->uid());
    _entry->setUsername(ui->leUsername->text());
    _entry->setPassword(SecureQByteArray(ui->lePassword->text().toUtf8()));
    _entry->setNotes(ui->teNotes->toPlainText());
    QDialog::accept();
}
