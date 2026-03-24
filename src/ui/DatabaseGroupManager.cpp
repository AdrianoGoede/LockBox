#include "DatabaseGroupManager.h"
#include "ui_DatabaseGroupManager.h"
#include <QMessageBox>

DatabaseGroupManager::DatabaseGroupManager(DatabaseGroup* group, const DatabaseGroup* existingGroup, const DatabaseGroup* parentGroup, QWidget* parent) : QDialog(parent), ui(new Ui::DatabaseGroupManager), _group(group), _existingGroup(existingGroup), _parentGroup(parentGroup)
{
    if (!_group)
        throw std::runtime_error("Group cannot be null");

    ui->setupUi(this);
    this->setWindowTitle(_group->title().isEmpty() ? "New Group" : "Edit Group");
    ui->leGroupParent->setText(_parentGroup ? _parentGroup->title() : "Root");
    ui->leGroupName->setText(_existingGroup ? _existingGroup->title() : QString());
}

DatabaseGroupManager::~DatabaseGroupManager() { delete ui; }

void DatabaseGroupManager::accept()
{
    if (ui->leGroupName->text().trimmed().isEmpty()) {
        QMessageBox::critical(
            this,
            "Error",
            "The group must have a name!",
            QMessageBox::StandardButton::Ok,
            QMessageBox::StandardButton::Ok
        );
        return;
    }

    _group->setParent(_parentGroup ? _parentGroup->uid() : QUuid());
    _group->setTitle(ui->leGroupName->text().trimmed());
    QDialog::accept();
}
