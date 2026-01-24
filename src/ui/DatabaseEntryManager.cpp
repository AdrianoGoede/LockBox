#include "DatabaseEntryManager.h"
#include "ui_DatabaseEntryManager.h"
#include "../core/HistoryActionButtonDelegate.h"
#include "MainWindow.h"
#include <QMessageBox>

DatabaseEntryManager::DatabaseEntryManager(DatabaseEntry* entry, const DatabaseGroup* group, const DatabaseEntry* existingEntry, const QList<DatabaseEntryHistoryItem>* history, QWidget* parent) : QDialog(parent), ui(new Ui::DatabaseEntryManager), _entry(entry), _existingEntry(existingEntry), _group(group), _history(history)
{
    if (!entry || !group)
        throw std::runtime_error("Entry instance and group must be informed");

    ui->setupUi(this);
    setDataFields();
    setHistoryTable();

    connect(ui->pbPasswordShow, &QAbstractButton::clicked, this, &DatabaseEntryManager::togglePasswordVisibility);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

DatabaseEntryManager::~DatabaseEntryManager() { delete ui; }

void DatabaseEntryManager::togglePasswordVisibility(bool visible) { ui->lePassword->setEchoMode(visible ? QLineEdit::EchoMode::Normal : QLineEdit::EchoMode::Password); }

void DatabaseEntryManager::copyUsernameToClipboard(const QModelIndex& index)
{
    if (!index.isValid() || !_history || index.row() >= _history->size()) return;
    const DatabaseEntryHistoryItem& item = _history->at(index.row());
    const MainWindow* parent = qobject_cast<const MainWindow*>(this->parent());
    if (!parent) return;
    parent->copyTextToClipboard(item.username().toUtf8());
}

void DatabaseEntryManager::copyPasswordToClipboard(const QModelIndex& index)
{
    if (!index.isValid() || !_history || index.row() >= _history->size()) return;
    const DatabaseEntryHistoryItem& item = _history->at(index.row());
    const MainWindow* parent = qobject_cast<const MainWindow*>(this->parent());
    if (!parent) return;
    parent->copyTextToClipboard(item.password());
}

void DatabaseEntryManager::accept()
{
    if (ui->leTitle->text().trimmed().isEmpty()) {
        QMessageBox::critical(
            this,
            "Error",
            "The entry must have a title!",
            QMessageBox::StandardButton::Ok,
            QMessageBox::StandardButton::Ok
        );
        return;
    }

    if (_existingEntry)
        _entry->setUid(_existingEntry->uid());
    _entry->setTitle(ui->leTitle->text());
    _entry->setGroup(_existingEntry ? _existingEntry->group() : _group->uid());
    _entry->setUsername(ui->leUsername->text());
    _entry->setPassword(SecureQByteArray(ui->lePassword->text().toUtf8()));
    _entry->setNotes(ui->teNotes->toPlainText());
    QDialog::accept();
}

void DatabaseEntryManager::setDataFields()
{
    this->setWindowTitle(_existingEntry ? "Edit Entry" : "Create Entry");
    ui->leTitle->setText(_existingEntry ? _existingEntry->title() : QString());
    ui->leUsername->setText(_existingEntry ? _existingEntry->username() : QString());
    ui->lePassword->setText(_existingEntry ? _existingEntry->password() : QString());
    ui->teNotes->setText(_existingEntry ? _existingEntry->notes() : QString());
}

void DatabaseEntryManager::setHistoryTable()
{
    ui->twHistory->setColumnCount(3);
    ui->twHistory->setHorizontalHeaderLabels({"Changed At", "Username", "Password"});
    ui->twHistory->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);
    ui->twHistory->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->twHistory->verticalHeader()->setVisible(false);

    if (_history) {
        for (const DatabaseEntryHistoryItem& item : *_history) {
            int row = ui->twHistory->rowCount();
            ui->twHistory->insertRow(row);
            ui->twHistory->setItem(row, 0, new QTableWidgetItem(item.createdAt().toString(Qt::DateFormat::RFC2822Date)));
        }

        HistoryActionButtonDelegate* usernameButton = new HistoryActionButtonDelegate(QIcon::fromTheme("user-offline"), this);
        connect(usernameButton, &HistoryActionButtonDelegate::clicked, this, &DatabaseEntryManager::copyUsernameToClipboard);
        ui->twHistory->setItemDelegateForColumn(1, usernameButton);

        HistoryActionButtonDelegate* passwordButton = new HistoryActionButtonDelegate(QIcon::fromTheme("system-lock-screen"), this);
        connect(passwordButton, &HistoryActionButtonDelegate::clicked, this, &DatabaseEntryManager::copyPasswordToClipboard);
        ui->twHistory->setItemDelegateForColumn(2, passwordButton);
    }
}
