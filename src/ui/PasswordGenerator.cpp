#include "PasswordGenerator.h"
#include "ui_PasswordGenerator.h"
#include "../config/Constants.h"
#include "../core/Crypto.h"
#include <QStringBuilder>
#include <QFileDialog>
#include <QMessageBox>
#include <QSet>
#include <QDir>

PasswordGenerator::PasswordGenerator(SecureQByteArray* out, QWidget *parent) : QDialog(parent), ui(new Ui::PasswordGenerator), _out(out)
{
    ui->setupUi(this);
    setPasswordTab();
    setPassphraseTab();
    connect(ui->pbGenerate, &QAbstractButton::clicked, this, &PasswordGenerator::generate);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

PasswordGenerator::~PasswordGenerator() { delete ui; }

void PasswordGenerator::accept()
{
    if (!_out) return;
    _out->wipe();
    _out->resize(ui->lePassword->text().size() + 1);
    _out->append(ui->lePassword->text().toUtf8());
}

void PasswordGenerator::generate()
{
    switch (ui->twOptions->currentIndex()) {
        case 0: generatePassword(); break;
        case 1: generatePassphrase(); break;
    }
}

void PasswordGenerator::buildCharset()
{
    QSet<char> result;

    if (ui->pbPasswdUppercaseLetters->isChecked())
        for (const char& chr : Config::constants::PASSWD_GEN_UPPERCASE_LETTERS)
            result.insert(chr);
    if (ui->pbPasswdLowercaseLetters->isChecked())
        for (const char& chr : Config::constants::PASSWD_GEN_LOWERCASE_LETTERS)
            result.insert(chr);
    if (ui->pbPasswdNumbers->isChecked())
        for (const char& chr : Config::constants::PASSWD_GEN_NUMBERS)
            result.insert(chr);
    if (ui->pbPasswdPunctuation->isChecked())
        for (const char& chr : Config::constants::PASSWD_GEN_PUNCTUATION)
            result.insert(chr);
    if (ui->pbPasswdSpecialChars1->isChecked())
        for (const char& chr : Config::constants::PASSWD_GEN_SPECIAL_CHARS1)
            result.insert(chr);
    if (ui->pbPasswdSpecialChars2->isChecked())
        for (const char& chr : Config::constants::PASSWD_GEN_SPECIAL_CHARS2)
            result.insert(chr);
    if (ui->pbPasswdSpecialChars3->isChecked())
        for (const char& chr : Config::constants::PASSWD_GEN_SPECIAL_CHARS3)
            result.insert(chr);
    if (ui->pbPasswdSpecialChars4->isChecked())
        for (const char& chr : Config::constants::PASSWD_GEN_SPECIAL_CHARS4)
            result.insert(chr);
    for (const char chr : ui->lePasswdExtraChars->text().toUtf8())
        result.insert(chr);

    _charset = QVector<char>(result.cbegin(), result.cend());
}

void PasswordGenerator::buildWordlist()
{
    QSet<QString> words;
    for (qsizetype i = 0; i < ui->lwPassphraseWordlists->count(); i++) {
        QFile file(ui->lwPassphraseWordlists->item(i)->text());
        if (!file.open(QIODevice::OpenModeFlag::ReadOnly | QIODevice::OpenModeFlag::Text))
            throw std::runtime_error(QString("Could not open file on row %1: %2").arg(i).arg(file.errorString()).toStdString());

        QTextStream stream(&file);
        while (!stream.atEnd()) {
            QString line = stream.readLine();
            int tabIndex = line.lastIndexOf('\t');
            words.insert((tabIndex != -1) ? line.mid(tabIndex + 1) : line);
        }
    }
    _wordlist = QVector<QString>(words.cbegin(), words.cend());
}

void PasswordGenerator::addWordlist()
{
    QStringList paths = QFileDialog::getOpenFileNames(this, "Select wordlist files", QDir::currentPath());
    for (const QString& path : paths)
        ui->lwPassphraseWordlists->addItem(path);
}

void PasswordGenerator::removeWordlist()
{
    QList<QListWidgetItem*> selectedItems = ui->lwPassphraseWordlists->selectedItems();
    if (selectedItems.isEmpty()) return;

    QMessageBox::Button button = QMessageBox::question(
        this,
        QString(),
        "Are you sure you want to remove the selected wordlists",
        (QMessageBox::Button::Yes | QMessageBox::Button::No),
        QMessageBox::Button::No
    );
    if (button != QMessageBox::Button::Yes) return;

    for (const QListWidgetItem* item : selectedItems)
        delete item;
    buildWordlist();
}

void PasswordGenerator::togglePasswordVisibility(bool visible) { ui->lePassword->setEchoMode(visible ? QLineEdit::EchoMode::Normal : QLineEdit::EchoMode::Password); }

void PasswordGenerator::handlePasswordLengthChange(int value) { ui->lbPasswordLength->setText(QString("Length: %1 characters").arg(value)); }

void PasswordGenerator::handlePassphraseLengthChange(int value){ ui->lbPassphraseWordCount->setText(QString("Length: %1 words").arg(value)); }

void PasswordGenerator::handleWordlistSelectionChange()
{
    QList<QListWidgetItem*> selectedItems = ui->lwPassphraseWordlists->selectedItems();
    ui->pbPassphraseRemoveWordlist->setEnabled(!selectedItems.isEmpty());
}

void PasswordGenerator::setPasswordTab()
{
    buildCharset();

    connect(ui->pbToggleVisibility, &QAbstractButton::clicked, this, &PasswordGenerator::togglePasswordVisibility);
    connect(ui->hsPasswordLength, &QAbstractSlider::valueChanged, this, &PasswordGenerator::handlePasswordLengthChange);
    ui->hsPasswordLength->setMinimum(Config::constants::MIN_PASSWORD_LENGTH);
    ui->hsPasswordLength->setMaximum(Config::constants::MAX_PASSWORD_LENGTH);

    for (const QObject* child : ui->tbPassword->children()) {
        if (const QPushButton* button = qobject_cast<const QPushButton*>(child))
            connect(button, &QAbstractButton::clicked, this, &PasswordGenerator::buildCharset);
    }
}

void PasswordGenerator::setPassphraseTab()
{
    QDir directory(Config::constants::WORDLISTS_RESOURCES_DIRECTORY);
    QStringList files = directory.entryList(QDir::Filter::Files);
    for (const QString& file : files)
        ui->lwPassphraseWordlists->addItem(QString("%1/%2").arg(Config::constants::WORDLISTS_RESOURCES_DIRECTORY).arg(file));

    buildWordlist();
    connect(ui->lwPassphraseWordlists, &QListWidget::itemSelectionChanged, this, &PasswordGenerator::handleWordlistSelectionChange);
    connect(ui->pbPassphraseAddWordlist, &QAbstractButton::clicked, this, &PasswordGenerator::addWordlist);
    connect(ui->pbPassphraseRemoveWordlist, &QAbstractButton::clicked, this, &PasswordGenerator::removeWordlist);

    connect(ui->hsPassphraseWordCount, &QAbstractSlider::valueChanged, this, &PasswordGenerator::handlePassphraseLengthChange);
    ui->hsPassphraseWordCount->setMinimum(Config::constants::MIN_PASSPHRASE_LENGTH);
    ui->hsPassphraseWordCount->setMaximum(Config::constants::MAX_PASSPHRASE_LENGTH);
}

void PasswordGenerator::generatePassword()
{
    SecureQByteArray password;
    Crypto::generateRandomPassword(_charset, ui->hsPasswordLength->value(), password);
    ui->lePassword->setText(password);
}

void PasswordGenerator::generatePassphrase()
{
    QStringList words;
    words.reserve(ui->hsPassphraseWordCount->value());

    QVector<quint32> indexes = Crypto::generateRandomUnsignedIntegers(_wordlist.size(), ui->hsPassphraseWordCount->value());
    for (quint32 index : indexes) {
        QString word(_wordlist.at(index).toUtf8());
        switch (ui->cbPassphraseWordCase->currentIndex()) {
            case 0: word = word.toLower(); break;
            case 1: word = word.toUpper(); break;
            case 2: {
                word = word.toLower();
                word[0] = word[0].toUpper();
            }; break;
        }
        words.append(word);
    }

    QString separator = ui->lePassphraseSeparator->text();
    ui->lePassword->setText(words.join(separator.isEmpty() ? " " : separator));
}
