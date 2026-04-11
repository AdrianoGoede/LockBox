#ifndef PASSWORDDIALOG_H
#define PASSWORDDIALOG_H

#include <QDialog>
#include "../core/SecureBuffer.h"

namespace Ui {
    class PasswordDialog;
}

class PasswordDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PasswordDialog(SecureBuffer<QChar>& out, QWidget* parent = nullptr);
    ~PasswordDialog();

private slots:
    void setPasswordVisibility(bool visible);
    void accept() override;

private:
    Ui::PasswordDialog* ui;
    SecureBuffer<QChar>& _out;
};

#endif // PASSWORDDIALOG_H
