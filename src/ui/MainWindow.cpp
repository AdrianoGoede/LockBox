#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "NewDatabase.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->actionDatabaseNew, &QAction::triggered, this, &MainWindow::newDatabase);
    connect(ui->actionDatabaseOpen, &QAction::triggered, this, &MainWindow::openDatabase);
    connect(ui->actionDatabaseSave, &QAction::triggered, this, &MainWindow::saveDatabase);
    connect(ui->actionDatabaseSaveAs, &QAction::triggered, this, &MainWindow::saveDatabaseAs);
    connect(ui->actionDatabaseSettings, &QAction::triggered, this, &MainWindow::openDatabaseSettings);
    connect(ui->actionDatabaseLock, &QAction::triggered, this, &MainWindow::lockDatabase);
    connect(ui->actionEntriesNew, &QAction::triggered, this, &MainWindow::newEntry);
    connect(ui->actionEntriesEdit, &QAction::triggered, this, &MainWindow::editEntry);
    connect(ui->actionEntriesDelete, &QAction::triggered, this, &MainWindow::deleteEntry);
    connect(ui->actionEntriesCopyUsername, &QAction::triggered, this, &MainWindow::copyEntryUsername);
    connect(ui->actionEntriesCopyPassword, &QAction::triggered, this, &MainWindow::copyEntryPassword);
    connect(ui->actionGroupsNew, &QAction::triggered, this, &MainWindow::newGroup);
    connect(ui->actionGroupsEdit, &QAction::triggered, this, &MainWindow::editGroup);
    connect(ui->actionGroupsDelete, &QAction::triggered, this, &MainWindow::deleteGroup);
    connect(ui->actionToolsPasswordGenerator, &QAction::triggered, this, &MainWindow::openPasswordGenerator);
    connect(ui->actionToolsAppSettings, &QAction::triggered, this, &MainWindow::openAppSettings);
    connect(ui->actionHelpGithubRepo, &QAction::triggered, this, &MainWindow::openRepo);
    connect(ui->actionHelpAbout, &QAction::triggered, this, &MainWindow::openAboutPage);

    connect(ui->pbSave, &QAbstractButton::clicked, this, &MainWindow::saveDatabase);
    connect(ui->pbLock, &QAbstractButton::clicked, this, &MainWindow::lockDatabase);
    connect(ui->pbAddEntry, &QAbstractButton::clicked, this, &MainWindow::newEntry);
    connect(ui->pbEditEntry, &QAbstractButton::clicked, this, &MainWindow::editEntry);
    connect(ui->pbDeleteEntry, &QAbstractButton::clicked, this, &MainWindow::deleteEntry);
    connect(ui->pbCopyUsername, &QAbstractButton::clicked, this, &MainWindow::copyEntryUsername);
    connect(ui->pbCopyPassword, &QAbstractButton::clicked, this, &MainWindow::copyEntryPassword);
    connect(ui->pbAutotype, &QAbstractButton::clicked, this, &MainWindow::autotypeEntry);
    connect(ui->pbPasswordGenerator, &QAbstractButton::clicked, this, &MainWindow::openPasswordGenerator);
    connect(ui->pbDatabaseSettings, &QAbstractButton::clicked, this, &MainWindow::openDatabaseSettings);
    connect(ui->pbAppSettings, &QAbstractButton::clicked, this, &MainWindow::openAppSettings);

    connect(ui->leEntryTitleFilter, &QLineEdit::textChanged, this, &MainWindow::filterEntryTitle);
    connect(ui->dteCreatedFromFilter, &QDateTimeEdit::dateTimeChanged, this, &MainWindow::filterEntryCreatedAfter);
    connect(ui->dteCreatedToFilter, &QDateTimeEdit::dateTimeChanged, this, &MainWindow::filterEntryCreatedBefore);
    connect(ui->dteModifiedFromFilter, &QDateTimeEdit::dateTimeChanged, this, &MainWindow::filterEntryModifiedAfter);
    connect(ui->dteModifiedToFilter, &QDateTimeEdit::dateTimeChanged, this, &MainWindow::filterEntryModifiedBefore);

    _entriesModel = new DatabaseEntryTableModel(this);
    _entriesProxyModel = new DatabaseEntryTableProxyModel(this);
    _entriesProxyModel->setSourceModel(_entriesModel);
    ui->tvEntries->setModel(_entriesProxyModel);
    ui->tvEntries->setSortingEnabled(true);
    ui->tvEntries->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->dteCreatedFromFilter->setDateTime(QDateTime::fromSecsSinceEpoch(0));
    ui->dteCreatedToFilter->setDateTime(QDateTime::fromSecsSinceEpoch(0).addYears(100));
    ui->dteModifiedFromFilter->setDateTime(QDateTime::fromSecsSinceEpoch(0));
    ui->dteModifiedToFilter->setDateTime(QDateTime::fromSecsSinceEpoch(0).addYears(100));
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::newDatabase()
{
    try {
        NewDbConfig config;
        NewDatabase newDbForm(config, this);

        if (newDbForm.exec() == QDialog::DialogCode::Accepted) {
            _database = std::make_unique<Database>(config.dbFilePath, config.password, config.unlockDelay, this);
            _entriesModel->setDatabase(_database.get());
            toggleDatabaseOpenState();
        }
    }
    catch (const std::runtime_error& error) {
        QMessageBox::critical(
            this,
            "Error",
            error.what(),
            QMessageBox::StandardButton::Ok
        );
    }
}

void MainWindow::openDatabase()
{
    try {
        if (_database) {
            QMessageBox::StandardButton button = QMessageBox::question(
                this,
                "?",
                "Save the currently open database?",
                (QMessageBox::StandardButton::Yes | QMessageBox::Button::No | QMessageBox::Button::Cancel),
                QMessageBox::StandardButton::Yes
            );

            if (button == QMessageBox::StandardButton::Cancel)
                return;
            else if (button == QMessageBox::StandardButton::Yes)
                _database->save();
        }

        QString path = QFileDialog::getOpenFileName(
            this,
            "Select file",
            QDir::currentPath(),
            QString(Config::constants::FILE_FILTER)
        );

        if (path.isEmpty())
            return;

        bool ok;
        SecureQByteArray password(QInputDialog::getText(
            this,
            "Enter the password",
            QString(),
            QLineEdit::EchoMode::Password,
            QString(),
            &ok
        ).toUtf8());

        if (!ok)
            return;
        else if (password.isEmpty()) {
            throw std::runtime_error("Password cannot be empty!");
            return;
        }

        _database = std::make_unique<Database>(path, password);
        _entriesModel->setDatabase(_database.get());
        toggleDatabaseOpenState();
    }
    catch (const std::runtime_error& error) {
        QMessageBox::critical(
            this,
            "Error",
            error.what(),
            QMessageBox::StandardButton::Ok
        );
    }
}

void MainWindow::saveDatabase()
{
    try {
        _database->save();
    }
    catch (const std::runtime_error& error) {
        QMessageBox::critical(
            this,
            "Error",
            error.what(),
            QMessageBox::StandardButton::Ok
        );
    }
}

void MainWindow::saveDatabaseAs()
{

}

void MainWindow::openDatabaseSettings()
{

}

void MainWindow::lockDatabase()
{
    QMessageBox::StandardButton button = QMessageBox::question(
        this,
        "?",
        "Unsaved changes will be lost, do you want to save?",
        (QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No | QMessageBox::StandardButton::Cancel),
        QMessageBox::StandardButton::Yes
    );

    try {
        if (button == QMessageBox::StandardButton::Cancel)
            return;
        else if (button == QMessageBox::StandardButton::Yes)
            _database->save();
        _database = nullptr;
        toggleDatabaseOpenState();
    }
    catch (const std::runtime_error& error) {
        QMessageBox::critical(
            this,
            "Error",
            error.what(),
            QMessageBox::StandardButton::Ok
        );
    }
}

void MainWindow::newEntry()
{

}

void MainWindow::editEntry()
{

}

void MainWindow::deleteEntry()
{

}

void MainWindow::filterEntryTitle(const QString& filter)
{
    if (_entriesProxyModel)
        _entriesProxyModel->setTitleFilter(filter);
}

void MainWindow::filterEntryCreatedAfter(const QDateTime& filter)
{
    if (_entriesProxyModel)
        _entriesProxyModel->setCreatedFromFilter(filter);
}

void MainWindow::filterEntryCreatedBefore(const QDateTime& filter)
{
    if (_entriesProxyModel)
        _entriesProxyModel->setCreatedToFilter(filter);
}

void MainWindow::filterEntryModifiedAfter(const QDateTime& filter)
{
    if (_entriesProxyModel)
        _entriesProxyModel->setModifiedFromFilter(filter);
}

void MainWindow::filterEntryModifiedBefore(const QDateTime& filter)
{
    if (_entriesProxyModel)
        _entriesProxyModel->setModifiedToFilter(filter);
}

void MainWindow::copyEntryUsername()
{

}

void MainWindow::copyEntryPassword()
{

}

void MainWindow::autotypeEntry()
{

}

void MainWindow::newGroup()
{

}

void MainWindow::editGroup()
{

}

void MainWindow::deleteGroup()
{

}

void MainWindow::openPasswordGenerator()
{

}

void MainWindow::openAppSettings()
{

}

void MainWindow::openRepo()
{

}

void MainWindow::openAboutPage()
{

}

void MainWindow::toggleDatabaseOpenState()
{
    ui->actionDatabaseSave->setEnabled(!ui->actionDatabaseSave->isEnabled());
    ui->actionDatabaseSaveAs->setEnabled(!ui->actionDatabaseSaveAs->isEnabled());
    ui->actionDatabaseLock->setEnabled(!ui->actionDatabaseLock->isEnabled());
    ui->pbSave->setEnabled(!ui->pbSave->isEnabled());
    ui->pbLock->setEnabled(!ui->pbLock->isEnabled());

    ui->leEntryTitleFilter->setEnabled(!ui->leEntryTitleFilter->isEnabled());
    ui->dteCreatedFromFilter->setEnabled(!ui->dteCreatedFromFilter->isEnabled());
    ui->dteCreatedToFilter->setEnabled(!ui->dteCreatedToFilter->isEnabled());
    ui->dteModifiedFromFilter->setEnabled(!ui->dteModifiedFromFilter->isEnabled());
    ui->dteModifiedToFilter->setEnabled(!ui->dteModifiedToFilter->isEnabled());
}
