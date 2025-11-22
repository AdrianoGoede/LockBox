#include "NewDatabase.h"
#include "./src/ui/ui_NewDatabase.h"
#include <QFileDialog>
#include <QMessageBox>

NewDatabase::NewDatabase(NewDbConfig& newDbConfig, QWidget* parent) : QDialog(parent), ui(new Ui::NewDatabase), _dbConfig(newDbConfig)
{
    ui->setupUi(this);
    connect(ui->pbFilePath, &QAbstractButton::clicked, this, &NewDatabase::selectFilePath);
    connect(ui->pbTogglePasswordShow, &QAbstractButton::clicked, this, &NewDatabase::togglePasswordVisibility);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

NewDatabase::~NewDatabase() { delete ui; }

void NewDatabase::accept()
{
    QString message;

    if (ui->leFilePath->text().size() == 0)
        message = QString("Enter the file name and path!");
    if (ui->lePassword->text().size() == 0)
        message = QString("Enter a valid password!");

    if (message.isEmpty()) {
        _dbConfig.dbFilePath = ui->leFilePath->text();
        _dbConfig.unlockDelay = ui->dsbUnlockDelay->value();
        _dbConfig.password.append(ui->lePassword->text().toUtf8());
    }
    else
        QMessageBox::critical(this, QString(), message, QMessageBox::StandardButton::Ok);
}

void NewDatabase::selectFilePath()
{
    ui->leFilePath->setText(QFileDialog::getSaveFileName(
        this,
        QString("Select the database file location"),
        QDir::currentPath(),
        QString("LockBox Database (*.lbdb)")
    ));
}

void NewDatabase::togglePasswordVisibility(bool visible) { ui->lePassword->setEchoMode(visible ? QLineEdit::EchoMode::Normal : QLineEdit::EchoMode::Password); }
