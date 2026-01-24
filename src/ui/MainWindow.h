#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "../core/Database.h"
#include "../core/DatabaseGroupTreeModel.h"
#include "../core/DatabaseEntryTableModel.h"
#include "../core/DatabaseEntryTableProxyModel.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void copyTextToClipboard(const QByteArray& text, int seconds = Config::constants::DEFAULT_CLIPBOARD_TIME) const;

private slots:
    void newDatabase();
    void openDatabase();
    void saveDatabase();
    void saveDatabaseAs();
    void openDatabaseSettings();
    void lockDatabase();
    void newEntry();
    void editEntry();
    void deleteEntry();
    void filterEntryTitle(const QString& filter);
    void filterEntryCreatedAfter(const QDateTime& filter);
    void filterEntryCreatedBefore(const QDateTime& filter);
    void filterEntryModifiedAfter(const QDateTime& filter);
    void filterEntryModifiedBefore(const QDateTime& filter);
    void filterEntriesByGroup(const QModelIndex& current, const QModelIndex& previous);
    void openEntryManager(const DatabaseEntry* existingEntry);
    void copyEntryUsername(const DatabaseEntry* entry = nullptr);
    void copyEntryPassword(const DatabaseEntry* entry = nullptr);
    void autotypeEntry();
    void newGroup();
    void editGroup();
    void deleteGroup();
    void openPasswordGenerator();
    void openAppSettings();
    void openRepo();
    void openAboutPage();

private:
    Ui::MainWindow* ui;
    std::unique_ptr<Database> _database = nullptr;
    DatabaseGroupTreeModel* _groupsModel = nullptr;
    DatabaseEntryTableModel* _entriesModel = nullptr;
    DatabaseEntryTableProxyModel* _entriesProxyModel = nullptr;
    void configureMenuBar();
    void configureButtonBar();
    void configureFilterBar();
    void configureGroupsTree();
    void configureEntryTable();
    void setDefaultFilters();
    void toggleDatabaseOpenState();
};

#endif // MAINWINDOW_H
