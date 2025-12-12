#include "MainWindow.h"
#include "ui_MainWindow.h"

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
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::newDatabase()
{

}

void MainWindow::openDatabase()
{

}

void MainWindow::saveDatabase()
{

}

void MainWindow::saveDatabaseAs()
{

}

void MainWindow::openDatabaseSettings()
{

}

void MainWindow::lockDatabase()
{

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
