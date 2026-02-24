#ifndef NEWDATABASE_H
#define NEWDATABASE_H

#include <QDialog>
#include "../core/Database.h"

namespace Ui {
    class NewDatabase;
}

class NewDatabase : public QDialog
{
    Q_OBJECT

public:
    explicit NewDatabase(NewDbConfig& newDbConfig, QWidget* parent = nullptr);
    ~NewDatabase();

public slots:
    void accept() override;

private slots:
    void selectFilePath();
    void generatePassword();
    void togglePasswordVisibility(bool visible);

private:
    Ui::NewDatabase* ui;
    NewDbConfig& _dbConfig;
};

#endif // NEWDATABASE_H
