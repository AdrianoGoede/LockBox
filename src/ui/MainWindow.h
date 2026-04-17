#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "autotype/Autotyper.h"
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
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void newDatabase();
    void openDatabase();
    void saveDatabase();
    void saveDatabaseAs();
    void openDatabaseSettings();
    void lockDatabase(bool ask = true);
    void newEntry();
    void editEntry();
    void deleteEntry();
    void filterEntryTitle(const QString& filter);
    void filterEntriesByGroup(const QModelIndex& current, const QModelIndex& previous);
    void openEntryManager(const QUuid& entryUid);
    void copyEntryUsername(const QUuid& entryUid);
    void copyEntryPassword(const QUuid& entryUid);
    void autotypeEntry();
    void newGroup();
    void editGroup();
    void deleteGroup();
    void openPasswordGenerator();
    void openRepo();
    void openAboutPage();
    void copyTextToClipboard(const QString& text) const;
    void handleInactivityTimeout();

private:
    Ui::MainWindow* ui;
    std::unique_ptr<Database> _database = nullptr;
    DatabaseGroupTreeModel* _groupsModel = nullptr;
    DatabaseEntryTableModel* _entriesModel = nullptr;
    DatabaseEntryTableProxyModel* _entriesProxyModel = nullptr;
    std::unique_ptr<Autotyper> _autotyper = nullptr;
    QTimer _inactivityTimer;
    quint32 _clipboardTime;
    void configureMenuBar();
    void configureButtonBar();
    void configureFilterBar();
    void configureGroupsTree();
    void configureEntryTable();
    void setTimers();
    void toggleDatabaseOpenState();
    void closeChildDialogs();
    bool eventFilter(QObject* obj, QEvent* event) override;
};

#endif // MAINWINDOW_H
