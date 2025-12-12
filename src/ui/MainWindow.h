#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

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
    void copyEntryUsername();
    void copyEntryPassword();
    void autotypeEntry();
    void newGroup();
    void editGroup();
    void deleteGroup();
    void openPasswordGenerator();
    void openAppSettings();
    void openRepo();
    void openAboutPage();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
