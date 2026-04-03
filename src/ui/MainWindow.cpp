#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "../core/EntryActionButtonDelegate.h"
#include "../core/SecureBuffer.h"
#include "../config/Constants.h"
#include "DatabaseGroupManager.h"
#include "DatabaseEntryManager.h"
#include "DatabaseSettingsManager.h"
#include "PasswordGenerator.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QClipboard>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    configureMenuBar();
    configureButtonBar();
    configureFilterBar();
    configureEntryTable();
    configureGroupsTree();
    setDefaultFilters();
    setTimers();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::copyTextToClipboard(const QString& text) const
{
    QClipboard* clipboard = QGuiApplication::clipboard();
    if (clipboard && !text.isEmpty()) {
        clipboard->setText(text);
        QTimer::singleShot(_clipboardTime, clipboard, [clipboard, text]() {
            if (clipboard->text().toUtf8() == text)
                clipboard->clear();
        });
    }
}

void MainWindow::newDatabase()
{
    try {
        QString filePath = QFileDialog::getSaveFileName(
            this,
            "Select file",
            QDir::currentPath(),
            QString(Config::constants::FILE_FILTER)
        );
        if (filePath.isEmpty()) return;

        DatabaseSettings settings;

        bool passwdOk;
        QString password = QInputDialog::getText(
            this,
            "Enter the new password",
            QString(),
            QLineEdit::EchoMode::Password,
            QString(),
            &passwdOk
        );
        if (!passwdOk) return;
        settings.password = SecureBuffer<QChar>(password.size());
        std::memcpy(settings.password.data(), password.constData(), settings.password.byteSize());

        DatabaseSettingsManager manager(settings, nullptr, this);
        if (manager.exec() != QDialog::DialogCode::Accepted) return;

        _database = std::make_unique<Database>(filePath, settings, this);
        _groupsModel->setDatabase(_database.get());
        _entriesModel->setDatabase(_database.get());

        if (settings.clearClipboardAfter > 0)
            _clipboardTime = (settings.clearClipboardAfter * 1000);
        if (settings.lockAfter > 0) {
            _inactivityTimer.setInterval(settings.lockAfter * 1000);
            _inactivityTimer.start();
        }

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
        QString passwordInput = QInputDialog::getText(
            this,
            "Enter the password",
            QString(),
            QLineEdit::EchoMode::Password,
            QString(),
            &ok
        );

        if (!ok)
            return;
        else if (passwordInput.isEmpty())
            throw std::runtime_error("Password cannot be empty!");

        SecureBuffer<QChar> password(passwordInput.size());
        std::memcpy(password.data(), passwordInput.constData(), password.byteSize());

        _database = std::make_unique<Database>(path, password);
        _groupsModel->setDatabase(_database.get());
        _entriesModel->setDatabase(_database.get());

        DatabaseSettings settings = _database->settings();
        if (settings.clearClipboardAfter > 0)
            _clipboardTime = (settings.clearClipboardAfter * 1000);
        if (settings.lockAfter > 0) {
            _inactivityTimer.setInterval(settings.lockAfter * 1000);
            _inactivityTimer.start();
        }

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
    try {
        QString path = QFileDialog::getSaveFileName(
            this,
            "Save As",
            QDir::currentPath(),
            QString(Config::constants::FILE_FILTER)
        );
        if (!path.isEmpty())
            _database->saveAs(path);
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

void MainWindow::openDatabaseSettings()
{
    try {
        if (!_database) return;
        DatabaseSettings settings;
        DatabaseSettings existingSettings = _database->settings();
        DatabaseSettingsManager manager(settings, &existingSettings, this);

        if (manager.exec() == QDialog::DialogCode::Accepted) {
            _inactivityTimer.stop();
            _database->setSettings(settings);
            if (settings.clearClipboardAfter > 0)
                _clipboardTime = (settings.clearClipboardAfter * 1000);
            if (settings.lockAfter > 0) {
                _inactivityTimer.setInterval(settings.lockAfter * 1000);
                _inactivityTimer.start();
            }
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

void MainWindow::lockDatabase(bool ask)
{
    QMessageBox::StandardButton button = (ask ? QMessageBox::question(
        this,
        "?",
        "Unsaved changes will be lost, do you want to save?",
        (QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No | QMessageBox::StandardButton::Cancel),
        QMessageBox::StandardButton::Yes
    ) : QMessageBox::StandardButton::Yes);

    try {
        if (button == QMessageBox::StandardButton::Cancel)
            return;
        else if (button == QMessageBox::StandardButton::Yes)
            _database->save();
        _groupsModel->setDatabase(nullptr);
        _entriesModel->setDatabase(nullptr);
        _database.reset();
        _inactivityTimer.stop();
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

void MainWindow::newEntry() { openEntryManager(QUuid(0)); }

void MainWindow::editEntry()
{
    QModelIndex index = ui->tvEntries->currentIndex();
    if (!index.isValid()) return;
    QUuid entryUid = index.data(Qt::UserRole + 1).value<QUuid>();
    if (entryUid.isNull()) return;
    openEntryManager(entryUid);
}

void MainWindow::deleteEntry()
{
    QModelIndex index = ui->tvEntries->currentIndex();
    if (!index.isValid()) return;
    QUuid entryUid = index.data(Qt::UserRole + 1).value<QUuid>();
    if (entryUid.isNull()) return;
    const DatabaseEntry* entry = _database->entry(entryUid);

    QMessageBox::StandardButton button = QMessageBox::question(
        this,
        "?",
        QString("Are you sure you want to delete entry '%1'?").arg(entry->title().trimmed()),
        (QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No),
        QMessageBox::StandardButton::No
    );
    if (button == QMessageBox::StandardButton::Yes)
        _database->removeEntry(entry->uid());
}

void MainWindow::filterEntryTitle(const QString& filter) { _entriesProxyModel->setTitleFilter(filter); }

void MainWindow::filterEntryCreatedAfter(const QDateTime& filter) { _entriesProxyModel->setCreatedFromFilter(filter); }

void MainWindow::filterEntryCreatedBefore(const QDateTime& filter) { _entriesProxyModel->setCreatedToFilter(filter); }

void MainWindow::filterEntryModifiedAfter(const QDateTime& filter) { _entriesProxyModel->setModifiedFromFilter(filter); }

void MainWindow::filterEntryModifiedBefore(const QDateTime& filter) { _entriesProxyModel->setModifiedToFilter(filter); }

void MainWindow::filterEntriesByGroup(const QModelIndex& current, const QModelIndex& previous)
{
    ui->actionGroupsNew->setEnabled(current.isValid());
    ui->actionGroupsEdit->setEnabled(current.isValid());
    ui->actionGroupsDelete->setEnabled(current.isValid());

    if (!current.isValid()) return;
    const DatabaseGroup* selectedGroup = static_cast<const DatabaseGroup*>(current.internalPointer());
    if (!selectedGroup) return;
    _entriesProxyModel->setGroupFilter(selectedGroup->uid());
}

void MainWindow::openEntryManager(const QUuid& entryUid)
{
    try {
        const DatabaseEntry* entry = (!entryUid.isNull() ? _database->entry(entryUid) : nullptr);
        QModelIndex index = ui->tvGroups->currentIndex();
        if (!index.isValid()) return;
        const DatabaseGroup* group = (!entry ? static_cast<const DatabaseGroup*>(index.internalPointer()) : nullptr);

        DatabaseEntryDto entryDto;
        DatabaseEntryManager manager(&entryDto, _database.get(), entry, group, this);

        if (manager.exec() == QDialog::DialogCode::Accepted) {
            if (entryUid.isNull())
                _database->addEntry(entryDto);
            else
                _database->editEntry(entryUid, entryDto);
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

void MainWindow::copyEntryUsername(const QUuid& entryUid)
{
    if (entryUid.isNull() || !_database) return;
    if (const DatabaseEntry* entry = _database->entry(entryUid))
        copyTextToClipboard(entry->username());
}

void MainWindow::copyEntryPassword(const QUuid& entryUid)
{
    if (entryUid.isNull() || !_database) return;
    if (const DatabaseEntry* entry = _database->entry(entryUid)) {
        SecureBuffer<QChar> password = _database->entryPassword(entryUid);
        copyTextToClipboard(QString(password.data(), password.size()));
    }
}

void MainWindow::autotypeEntry()
{

}

void MainWindow::newGroup()
{
    try {
        QModelIndex index = ui->tvGroups->currentIndex();
        if (!index.isValid()) return;
        QUuid groupUid = index.data(Qt::UserRole + 1).value<QUuid>();
        if (groupUid.isNull()) return;
        const DatabaseGroup* parentGroup = _database->group(groupUid);
        if (!parentGroup) return;
        DatabaseGroupDto groupDto;
        DatabaseGroupManager manager(&groupDto, nullptr, parentGroup, this);

        if (manager.exec() == QDialog::DialogCode::Accepted)
            _database->addGroup(groupDto);
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

void MainWindow::editGroup()
{
    try {
        QModelIndex index = ui->tvGroups->currentIndex();
        if (!index.isValid()) return;
        QUuid groupUid = index.data(Qt::UserRole + 1).value<QUuid>();
        if (groupUid.isNull()) return;
        const DatabaseGroup* group = _database->group(groupUid);
        if (!group) return;
        const DatabaseGroup* parentGroup = (!group->parent().isNull() ? _database->group(group->parent()) : nullptr);
        DatabaseGroupDto groupDto;
        DatabaseGroupManager manager(&groupDto, group, parentGroup, this);

        if (manager.exec() == QDialog::DialogCode::Accepted)
            _database->editGroup(group->uid(), groupDto);
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

void MainWindow::deleteGroup()
{
    try {
        QModelIndex index = ui->tvGroups->currentIndex();
        if (!index.isValid()) return;
        const DatabaseGroup* group = static_cast<const DatabaseGroup*>(index.internalPointer());
        if (!group) return;

        QMessageBox::StandardButton button = QMessageBox::question(
            this,
            "?",
            QString("Are you sure you want to delete group '%1' and all it's entries?").arg(group->title().trimmed()),
            (QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No),
            QMessageBox::StandardButton::No
        );
        if (button == QMessageBox::StandardButton::Yes)
            _database->removeGroup(group->uid());
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

void MainWindow::openPasswordGenerator()
{
    SecureBuffer<QChar> buffer = SecureBuffer<QChar>(0);
    PasswordGenerator generator(buffer, this);
    generator.exec();
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

void MainWindow::handleInactivityTimeout()
{
    try {
        closeChildDialogs();
        lockDatabase(false);
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

void MainWindow::configureMenuBar()
{
    connect(ui->actionDatabaseNew, &QAction::triggered, this, &MainWindow::newDatabase);
    connect(ui->actionDatabaseOpen, &QAction::triggered, this, &MainWindow::openDatabase);
    connect(ui->actionDatabaseSave, &QAction::triggered, this, &MainWindow::saveDatabase);
    connect(ui->actionDatabaseSaveAs, &QAction::triggered, this, &MainWindow::saveDatabaseAs);
    connect(ui->actionDatabaseSettings, &QAction::triggered, this, &MainWindow::openDatabaseSettings);
    connect(ui->actionDatabaseLock, &QAction::triggered, this, &MainWindow::lockDatabase);
    connect(ui->actionEntriesNew, &QAction::triggered, this, &MainWindow::newEntry);
    connect(ui->actionEntriesEdit, &QAction::triggered, this, &MainWindow::editEntry);
    connect(ui->actionEntriesDelete, &QAction::triggered, this, &MainWindow::deleteEntry);
    connect(ui->actionGroupsNew, &QAction::triggered, this, &MainWindow::newGroup);
    connect(ui->actionGroupsEdit, &QAction::triggered, this, &MainWindow::editGroup);
    connect(ui->actionGroupsDelete, &QAction::triggered, this, &MainWindow::deleteGroup);
    connect(ui->actionToolsPasswordGenerator, &QAction::triggered, this, &MainWindow::openPasswordGenerator);
    connect(ui->actionToolsAppSettings, &QAction::triggered, this, &MainWindow::openAppSettings);
    connect(ui->actionHelpGithubRepo, &QAction::triggered, this, &MainWindow::openRepo);
    connect(ui->actionHelpAbout, &QAction::triggered, this, &MainWindow::openAboutPage);
}

void MainWindow::configureButtonBar()
{
    connect(ui->pbSave, &QAbstractButton::clicked, this, &MainWindow::saveDatabase);
    connect(ui->pbLock, &QAbstractButton::clicked, this, &MainWindow::lockDatabase);
    connect(ui->pbAddEntry, &QAbstractButton::clicked, this, &MainWindow::newEntry);
    connect(ui->pbEditEntry, &QAbstractButton::clicked, this, &MainWindow::editEntry);
    connect(ui->pbDeleteEntry, &QAbstractButton::clicked, this, &MainWindow::deleteEntry);
    connect(ui->pbPasswordGenerator, &QAbstractButton::clicked, this, &MainWindow::openPasswordGenerator);
    connect(ui->pbDatabaseSettings, &QAbstractButton::clicked, this, &MainWindow::openDatabaseSettings);
    connect(ui->pbAppSettings, &QAbstractButton::clicked, this, &MainWindow::openAppSettings);
}

void MainWindow::configureFilterBar()
{
    connect(ui->leEntryTitleFilter, &QLineEdit::textChanged, this, &MainWindow::filterEntryTitle);
    connect(ui->dteCreatedFromFilter, &QDateTimeEdit::dateTimeChanged, this, &MainWindow::filterEntryCreatedAfter);
    connect(ui->dteCreatedToFilter, &QDateTimeEdit::dateTimeChanged, this, &MainWindow::filterEntryCreatedBefore);
    connect(ui->dteModifiedFromFilter, &QDateTimeEdit::dateTimeChanged, this, &MainWindow::filterEntryModifiedAfter);
    connect(ui->dteModifiedToFilter, &QDateTimeEdit::dateTimeChanged, this, &MainWindow::filterEntryModifiedBefore);
}

void MainWindow::configureGroupsTree()
{
    _groupsModel = new DatabaseGroupTreeModel(this);
    ui->tvGroups->setModel(_groupsModel);
    ui->tvGroups->setAcceptDrops(true);
    ui->tvGroups->setDropIndicatorShown(true);
    connect(ui->tvGroups->selectionModel(), &QItemSelectionModel::currentChanged, this, &MainWindow::filterEntriesByGroup);
}

void MainWindow::configureEntryTable()
{
    _entriesModel = new DatabaseEntryTableModel(this);
    _entriesProxyModel = new DatabaseEntryTableProxyModel(this);
    _entriesProxyModel->setSourceModel(_entriesModel);
    ui->tvEntries->setModel(_entriesProxyModel);
    ui->tvEntries->setSortingEnabled(true);
    ui->tvEntries->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);

    EntryActionButtonDelegate* manageEntryButtonDelegate = new EntryActionButtonDelegate(QIcon::fromTheme("zoom-in"), this);
    ui->tvEntries->setItemDelegateForColumn(3, manageEntryButtonDelegate);
    ui->tvEntries->setColumnWidth(3, 90);
    ui->tvEntries->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeMode::Fixed);
    connect(manageEntryButtonDelegate, &EntryActionButtonDelegate::clicked, this, &MainWindow::openEntryManager);

    EntryActionButtonDelegate* copyUsernameButtonDelegate = new EntryActionButtonDelegate(QIcon::fromTheme("user-offline"), this);
    ui->tvEntries->setItemDelegateForColumn(4, copyUsernameButtonDelegate);
    ui->tvEntries->setColumnWidth(4, 90);
    ui->tvEntries->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeMode::Fixed);
    connect(copyUsernameButtonDelegate, &EntryActionButtonDelegate::clicked, this, &MainWindow::copyEntryUsername);

    EntryActionButtonDelegate* copyPasswordButtonDelegate = new EntryActionButtonDelegate(QIcon::fromTheme("system-lock-screen"), this);
    ui->tvEntries->setItemDelegateForColumn(5, copyPasswordButtonDelegate);
    ui->tvEntries->setColumnWidth(5, 90);
    ui->tvEntries->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeMode::Fixed);
    connect(copyPasswordButtonDelegate, &EntryActionButtonDelegate::clicked, this, &MainWindow::copyEntryPassword);

    EntryActionButtonDelegate* performAutotypeButtonDelegate = new EntryActionButtonDelegate(QIcon::fromTheme("input-keyboard"), this);
    ui->tvEntries->setItemDelegateForColumn(6, performAutotypeButtonDelegate);
    ui->tvEntries->setColumnWidth(6, 90);
    ui->tvEntries->horizontalHeader()->setSectionResizeMode(6, QHeaderView::ResizeMode::Fixed);
    connect(performAutotypeButtonDelegate, &EntryActionButtonDelegate::clicked, this, &MainWindow::autotypeEntry);
}

void MainWindow::setDefaultFilters()
{
    ui->dteCreatedFromFilter->setDateTime(QDateTime::fromSecsSinceEpoch(0));
    ui->dteCreatedToFilter->setDateTime(QDateTime::fromSecsSinceEpoch(0).addYears(100));
    ui->dteModifiedFromFilter->setDateTime(QDateTime::fromSecsSinceEpoch(0));
    ui->dteModifiedToFilter->setDateTime(QDateTime::fromSecsSinceEpoch(0).addYears(100));
}

void MainWindow::setTimers()
{
    connect(&_inactivityTimer, &QTimer::timeout, this, &MainWindow::handleInactivityTimeout);
    qApp->installEventFilter(this);
}

void MainWindow::toggleDatabaseOpenState()
{
    ui->actionDatabaseSave->setEnabled(!ui->actionDatabaseSave->isEnabled());
    ui->actionDatabaseSaveAs->setEnabled(!ui->actionDatabaseSaveAs->isEnabled());
    ui->actionDatabaseSettings->setEnabled(!ui->actionDatabaseSettings->isEnabled());
    ui->actionDatabaseLock->setEnabled(!ui->actionDatabaseLock->isEnabled());

    ui->menuEntries->setEnabled(!ui->menuEntries->isEnabled());
    ui->menuGroups->setEnabled(!ui->menuGroups->isEnabled());

    ui->pbSave->setEnabled(!ui->pbSave->isEnabled());
    ui->pbLock->setEnabled(!ui->pbLock->isEnabled());
    ui->pbAddEntry->setEnabled(!ui->pbAddEntry->isEnabled());
    ui->pbEditEntry->setEnabled(!ui->pbEditEntry->isEnabled());
    ui->pbDeleteEntry->setEnabled(!ui->pbDeleteEntry->isEnabled());
    ui->pbDatabaseSettings->setEnabled(!ui->pbDatabaseSettings->isEnabled());

    ui->leEntryTitleFilter->setEnabled(!ui->leEntryTitleFilter->isEnabled());
    ui->dteCreatedFromFilter->setEnabled(!ui->dteCreatedFromFilter->isEnabled());
    ui->dteCreatedToFilter->setEnabled(!ui->dteCreatedToFilter->isEnabled());
    ui->dteModifiedFromFilter->setEnabled(!ui->dteModifiedFromFilter->isEnabled());
    ui->dteModifiedToFilter->setEnabled(!ui->dteModifiedToFilter->isEnabled());
}

void MainWindow::closeChildDialogs()
{
    for (QObject* child : this->children()) {
        if (QDialog* dialog = qobject_cast<QDialog*>(child))
            dialog->reject();
    }
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{
    switch (event->type()) {
        case QEvent::KeyPress:
        case QEvent::KeyRelease:
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseMove:
        case QEvent::Wheel:
        case QEvent::TouchBegin:
        case QEvent::TouchUpdate:
        case QEvent::TouchEnd:
        case QEvent::FocusIn:
        case QEvent::FocusOut: {
            if (_inactivityTimer.isActive()) {
                _inactivityTimer.stop();
                _inactivityTimer.start();
            }
        } break;
        default: break;
    }

    return QMainWindow::eventFilter(obj, event);
}
