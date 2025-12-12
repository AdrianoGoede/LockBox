#include "PasswordGenerator.h"
#include "ui_PasswordGenerator.h"
#include "../config/Constants.h"
#include "../core/Crypto.h"
#include <QSet>

PasswordGenerator::PasswordGenerator(SecureQByteArray* out, QWidget *parent) : QDialog(parent), ui(new Ui::PasswordGenerator), _out(out)
{
    ui->setupUi(this);
    connect(ui->pbGenerate, &QAbstractButton::clicked, this, &PasswordGenerator::generate);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

PasswordGenerator::~PasswordGenerator() { delete ui; }

void PasswordGenerator::accept()
{
    if (!_out) return;
    _out->wipe();
    _out->resize(ui->lePassword->text().size() + 1);
    _out->append(ui->lePassword->text().toUtf8());
}

void PasswordGenerator::generate()
{
    switch (ui->twOptions->currentIndex()) {
        case 0: generatePassword(); break;
        case 1: generatePassphrase(); break;
    }
}

void PasswordGenerator::generatePassword()
{
    const QVector<char> charset = buildCharset();
    SecureQByteArray password;
    Crypto::generateRandomPassword(charset, ui->hsPasswordLength->value(), password);
    ui->lePassword->setText(password);
}

void PasswordGenerator::generatePassphrase()
{

}

QVector<char> PasswordGenerator::buildCharset()
{
    QSet<char> result;

    if (ui->pbPasswdUppercaseLetters->isChecked())
        for (const char& chr : Config::constants::PASSWD_GEN_UPPERCASE_LETTERS)
            result.insert(chr);
    if (ui->pbPasswdLowercaseLetters->isChecked())
        for (const char& chr : Config::constants::PASSWD_GEN_LOWERCASE_LETTERS)
            result.insert(chr);
    if (ui->pbPasswdNumbers->isChecked())
        for (const char& chr : Config::constants::PASSWD_GEN_NUMBERS)
            result.insert(chr);
    if (ui->pbPasswdPunctuation->isChecked())
        for (const char& chr : Config::constants::PASSWD_GEN_PUNCTUATION)
            result.insert(chr);
    if (ui->pbPasswdSpecialChars1->isChecked())
        for (const char& chr : Config::constants::PASSWD_GEN_SPECIAL_CHARS1)
            result.insert(chr);
    if (ui->pbPasswdSpecialChars2->isChecked())
        for (const char& chr : Config::constants::PASSWD_GEN_SPECIAL_CHARS2)
            result.insert(chr);
    if (ui->pbPasswdSpecialChars3->isChecked())
        for (const char& chr : Config::constants::PASSWD_GEN_SPECIAL_CHARS3)
            result.insert(chr);
    if (ui->pbPasswdSpecialChars4->isChecked())
        for (const char& chr : Config::constants::PASSWD_GEN_SPECIAL_CHARS4)
            result.insert(chr);

    for (const char chr : ui->lePasswdExtraChars->text().toUtf8())
        result.insert(chr);

    return QVector<char>(result.cbegin(), result.cend());
}
