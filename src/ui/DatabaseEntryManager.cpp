#include "DatabaseEntryManager.h"
#include "ui_DatabaseEntryManager.h"
#include "../core/HistoryActionButtonDelegate.h"
#include "MainWindow.h"
#include <QMessageBox>

DatabaseEntryManager::DatabaseEntryManager(DatabaseEntryDto* entryDto, const Database* database, const DatabaseEntry* entry, const DatabaseGroup* group, QWidget* parent) : QDialog(parent), ui(new Ui::DatabaseEntryManager), _entryDto(entryDto), _database(database), _entry(entry), _group(group)
{
    if (!entryDto)
        throw std::runtime_error("entryDto cannot be null");
    if (!_database)
        throw std::runtime_error("database cannot be null");
    if (!_entry && !_group)
        throw std::runtime_error("group or existing entry required");

    ui->setupUi(this);
    setDataFields();
    setHistoryTable();

    connect(ui->pbPasswordShow, &QAbstractButton::clicked, this, &DatabaseEntryManager::togglePasswordVisibility);
}

DatabaseEntryManager::~DatabaseEntryManager() { delete ui; }

void DatabaseEntryManager::togglePasswordVisibility(bool visible) { ui->lePassword->setEchoMode(visible ? QLineEdit::EchoMode::Normal : QLineEdit::EchoMode::Password); }

void DatabaseEntryManager::copyUsernameToClipboard(const QModelIndex& index)
{
    if (!index.isValid() || !_entry || index.row() >= _entry->history().size()) return;
    const DatabaseEntryHistoryItem& item = _entry->history().at(index.row());
    SecureBuffer<QChar> username = _database->entryHistoryItemUsername(_entry->uid(), item.itemUid());
    emit copyToClipboardRequested(QString(username.data(), username.size()));
}

void DatabaseEntryManager::copyPasswordToClipboard(const QModelIndex& index)
{
    if (!index.isValid() || !_entry || index.row() >= _entry->history().size()) return;
    const DatabaseEntryHistoryItem& item = _entry->history().at(index.row());
    SecureBuffer<QChar> password = _database->entryHistoryItemPassword(_entry->uid(), item.itemUid());
    emit copyToClipboardRequested(QString(password.data(), password.size()));
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

    _entryDto->title = ui->leTitle->text();
    _entryDto->group = (_entry ? _entry->group() : _group->uid());

    getUsername();
    getPassword();
    getNotes();

    QDialog::accept();
}

void DatabaseEntryManager::setDataFields()
{
    SecureBuffer<QChar> username = (_entry ? _database->entryUsername(_entry->uid()) : SecureBuffer<QChar>());
    SecureBuffer<QChar> password = (_entry ? _database->entryPassword(_entry->uid()) : SecureBuffer<QChar>());
    SecureBuffer<QChar> notes = (_entry ? _database->entryNotes(_entry->uid()) : SecureBuffer<QChar>());

    this->setWindowTitle(_entry ? "Edit Entry" : "Create Entry");
    ui->leTitle->setText(_entry ? _entry->title() : QString());
    ui->leUsername->setText(QString(username.data(), username.size()));
    ui->lePassword->setText(QString(password.data(), password.size()));
    ui->teNotes->setText(QString(notes.data(), notes.size()));
}

void DatabaseEntryManager::setHistoryTable()
{
    ui->twHistory->setColumnCount(3);
    ui->twHistory->setHorizontalHeaderLabels({"Changed At", "Username", "Password"});
    ui->twHistory->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);
    ui->twHistory->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->twHistory->verticalHeader()->setVisible(false);

    if (_entry) {
        for (const DatabaseEntryHistoryItem& item : _entry->history()) {
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

void DatabaseEntryManager::getUsername()
{
    QString username = ui->leUsername->text();
    ui->leUsername->setText(QString(username.size(), 'X'));
    ui->leUsername->clear();

    _entryDto->username = SecureBuffer<QChar>(username.size());
    std::memcpy(_entryDto->username.data(), username.constData(), _entryDto->username.byteSize());
    username.fill('X', username.size());
    username.clear();
}

void DatabaseEntryManager::getPassword()
{
    QString password = ui->lePassword->text();
    ui->lePassword->setText(QString(password.size(), 'X'));
    ui->lePassword->clear();

    _entryDto->password = SecureBuffer<QChar>(password.size());
    std::memcpy(_entryDto->password.data(), password.constData(), _entryDto->password.byteSize());
    password.fill('X', password.size());
    password.clear();
}

void DatabaseEntryManager::getNotes()
{
    QString notes = ui->teNotes->toPlainText();
    ui->teNotes->setText(QString(notes.size(), 'X'));
    ui->teNotes->clear();

    _entryDto->notes = SecureBuffer<QChar>(notes.size());
    std::memcpy(_entryDto->notes.data(), notes.constData(), _entryDto->notes.byteSize());
    notes.fill('X', notes.size());
    notes.clear();
}
