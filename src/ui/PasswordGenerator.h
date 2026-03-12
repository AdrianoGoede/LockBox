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
    explicit PasswordGenerator(SecureQByteArray* out = nullptr, QWidget* parent = nullptr);
    ~PasswordGenerator();

public slots:
    void accept() override;

private slots:
    void generate();
    void buildCharset();
    void buildWordlist();
    void addWordlist();
    void removeWordlist();
    void togglePasswordVisibility(bool visible);
    void handlePasswordLengthChange(int value);
    void handleWordlistSelectionChange();

private:
    Ui::PasswordGenerator* ui;
    SecureQByteArray* _out = nullptr;
    QVector<char> _charset;
    QVector<QString> _wordlist;
    void setPasswordTab();
    void setPassphraseTab();
    void generatePassword();
    void generatePassphrase();
};

#endif // PASSWORDGENERATOR_H
