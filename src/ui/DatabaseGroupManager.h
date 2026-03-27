#ifndef DATABASEGROUPMANAGER_H
#define DATABASEGROUPMANAGER_H

#include <QDialog>
#include <QAbstractButton>
#include "../core/DatabaseGroup.h"

namespace Ui {
    class DatabaseGroupManager;
}

class DatabaseGroupManager : public QDialog
{
    Q_OBJECT

public:
    explicit DatabaseGroupManager(DatabaseGroupDto* groupDto, const DatabaseGroup* group = nullptr, const DatabaseGroup* parentGroup = nullptr, QWidget* parent = nullptr);
    ~DatabaseGroupManager();

private slots:
    void accept() override;

private:
    Ui::DatabaseGroupManager* ui;
    DatabaseGroupDto* _groupDto;
    const DatabaseGroup* _group = nullptr;
    const DatabaseGroup* _parentGroup = nullptr;
};

#endif // DATABASEGROUPMANAGER_H
