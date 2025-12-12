#ifndef NEWDATABASE_H
#define NEWDATABASE_H

#include <QDialog>
#include <chrono>
#include "../core/SecureQByteArray.h"

namespace Ui {
    class NewDatabase;
}

struct NewDbConfig {
    QString dbFilePath;
    std::chrono::milliseconds unlockDelay;
    SecureQByteArray password;
};

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
