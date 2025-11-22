#include "MainWidget.h"
#include "./ui_MainWidget.h"
#include "NewDatabase.h"
#include <QFileDialog>

MainWidget::MainWidget(QWidget *parent) : QWidget(parent), ui(new Ui::MainWidget)
{
    ui->setupUi(this);
    connect(ui->PbNewDb, &QAbstractButton::clicked, this, &MainWidget::newDatabase);
    connect(ui->PbOpenDb, &QAbstractButton::clicked, this, &MainWidget::openDatabase);
    connect(ui->PbAbout, &QAbstractButton::clicked, this, &MainWidget::about);
}

MainWidget::~MainWidget() { delete ui; }

void MainWidget::newDatabase()
{
    NewDbConfig config;
    NewDatabase newDbForm(config, this);

    if (newDbForm.exec() == QDialog::DialogCode::Accepted) {

    }
}

void MainWidget::openDatabase()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        QString("Select the database file"),
        QDir::currentPath(),
        QString("LockBox Database (*.lbdb)")
    );

    if (!filePath.isEmpty()) {

    }
}

void MainWidget::about() {}
