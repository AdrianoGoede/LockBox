#ifndef PASSWORDGENERATOR_H
#define PASSWORDGENERATOR_H

#include <QDialog>
#include <QVector>
#include "../core/SecureQByteArray.h"

namespace Ui {
    class PasswordGenerator;
}

class PasswordGenerator : public QDialog
{
    Q_OBJECT

public:
    explicit PasswordGenerator(SecureQByteArray* out = nullptr, QWidget *parent = nullptr);
    ~PasswordGenerator();

public slots:
    void accept() override;

private slots:
    void generate();

private:
    Ui::PasswordGenerator* ui;
    SecureQByteArray* _out = nullptr;
    void setDefaultWordLists();
    void generatePassword();
    void generatePassphrase();
    QVector<char> buildCharset();
};

#endif // PASSWORDGENERATOR_H
