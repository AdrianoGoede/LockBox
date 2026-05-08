#include "DatabaseGroupManager.h"
#include "ui_DatabaseGroupManager.h"
#include <QMessageBox>

DatabaseGroupManager::DatabaseGroupManager(DatabaseGroupDto* groupDto, const DatabaseGroup* group, const DatabaseGroup* parentGroup, QWidget* parent) : QDialog(parent), ui(new Ui::DatabaseGroupManager), _groupDto(groupDto), _group(group), _parentGroup(parentGroup)
{
    if (!_groupDto)
        throw std::runtime_error("Group DTO cannot be null");

    ui->setupUi(this);
    this->setWindowTitle(_group ? "Edit Group" : "New Group");
    ui->leGroupParent->setText(_parentGroup ? _parentGroup->title() : QString());
    ui->leGroupName->setText(_group ? _group->title() : QString());
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

    _groupDto->parent = (_parentGroup ? _parentGroup->uid() : QUuid(0));
    _groupDto->title = ui->leGroupName->text().trimmed();
    QDialog::accept();
}
