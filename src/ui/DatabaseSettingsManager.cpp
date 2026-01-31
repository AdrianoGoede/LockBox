#include "DatabaseSettingsManager.h"
#include "ui_DatabaseSettingsManager.h"

DatabaseSettingsManager::DatabaseSettingsManager(const Database* database, QWidget* parent) : QDialog(parent), ui(new Ui::DatabaseSettingsManager), _database(database)
{
    if (!_database)
        throw std::runtime_error("Database cannot be null");
    ui->setupUi(this);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &DatabaseSettingsManager::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &DatabaseSettingsManager::reject);
}

DatabaseSettingsManager::~DatabaseSettingsManager() { delete ui; }
