#include "DatabaseEntryManager.h"
#include "ui_DatabaseEntryManager.h"

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

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

DatabaseEntryManager::~DatabaseEntryManager() { delete ui; }

void DatabaseEntryManager::accept()
{

}
