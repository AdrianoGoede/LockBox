#include "PasswordDialog.h"
#include "ui_PasswordDialog.h"
#include "../config/Constants.h"
#include "../core/Utils.h"
#include <QMessageBox>

PasswordDialog::PasswordDialog(SecureBuffer<QChar>& out, QWidget* parent) : QDialog(parent), ui(new Ui::PasswordDialog), _out(out)
{
    ui->setupUi(this);
    ui->lePassword->setMaxLength(Config::constants::MAX_PASSWORD_LENGTH);
    connect(ui->pbPasswordVisibility, &QAbstractButton::clicked, this, &PasswordDialog::setPasswordVisibility);
}

PasswordDialog::~PasswordDialog()
{
    Utils::clearQLineEdit(ui->lePassword);
    delete ui;
}

void PasswordDialog::setPasswordVisibility(bool visible) { ui->lePassword->setEchoMode(visible ? QLineEdit::EchoMode::Normal : QLineEdit::EchoMode::Password); }

void PasswordDialog::accept()
{
    QString password = ui->lePassword->text();

    if (password.size() >= Config::constants::MIN_PASSWORD_LENGTH && password.size() <= Config::constants::MAX_PASSWORD_LENGTH) {
        _out = SecureBuffer<QChar>(password.size());
        std::memcpy(_out.data(), password.constData(), _out.byteSize());
        Utils::clearQString(password);
        QDialog::accept();
    }
    else {
        Utils::clearQString(password);
        QMessageBox::critical(
            this,
            QString(),
            QString("The password must be between %1 and %2 characters long").arg(Config::constants::MIN_PASSWORD_LENGTH).arg(Config::constants::MAX_PASSWORD_LENGTH),
            QMessageBox::StandardButton::Ok,
            QMessageBox::StandardButton::Ok
        );
    }
}
