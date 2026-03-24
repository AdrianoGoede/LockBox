#include "PasswordGenerator.h"
#include "ui_PasswordGenerator.h"
#include "../core/SecureBuffer.h"
#include "../config/Constants.h"
#include "../core/Crypto.h"
#include <QStringBuilder>
#include <QFileDialog>
#include <QMessageBox>
#include <QSet>
#include <QDir>

PasswordGenerator::PasswordGenerator(SecureBuffer<QChar>& out, QWidget* parent) : QDialog(parent), ui(new Ui::PasswordGenerator), _out(out)
{
    ui->setupUi(this);
    setPasswordTab();
    setPassphraseTab();
    connect(ui->pbGenerate, &QAbstractButton::clicked, this, &PasswordGenerator::generate);
}

PasswordGenerator::~PasswordGenerator() { delete ui; }

void PasswordGenerator::accept()
{
    QString input = ui->lePassword->text();
    ui->lePassword->setText(QString(input.size(), 'X'));
    ui->lePassword->clear();

    _out = SecureBuffer<QChar>(input.size());
    std::memcpy(_out.data(), input.constData(), _out.byteSize());
    input.fill('X', input.size());
    input.clear();
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
    QSet<QChar> result;

    if (ui->pbPasswdUppercaseLetters->isChecked())
        for (const char& chr : Config::constants::PASSWD_GEN_UPPERCASE_LETTERS)
            result.insert(QChar(chr));
    if (ui->pbPasswdLowercaseLetters->isChecked())
        for (const char& chr : Config::constants::PASSWD_GEN_LOWERCASE_LETTERS)
            result.insert(QChar(chr));
    if (ui->pbPasswdNumbers->isChecked())
        for (const char& chr : Config::constants::PASSWD_GEN_NUMBERS)
            result.insert(QChar(chr));
    if (ui->pbPasswdPunctuation->isChecked())
        for (const char& chr : Config::constants::PASSWD_GEN_PUNCTUATION)
            result.insert(QChar(chr));
    if (ui->pbPasswdSpecialChars1->isChecked())
        for (const char& chr : Config::constants::PASSWD_GEN_SPECIAL_CHARS1)
            result.insert(QChar(chr));
    if (ui->pbPasswdSpecialChars2->isChecked())
        for (const char& chr : Config::constants::PASSWD_GEN_SPECIAL_CHARS2)
            result.insert(QChar(chr));
    if (ui->pbPasswdSpecialChars3->isChecked())
        for (const char& chr : Config::constants::PASSWD_GEN_SPECIAL_CHARS3)
            result.insert(QChar(chr));
    if (ui->pbPasswdSpecialChars4->isChecked())
        for (const char& chr : Config::constants::PASSWD_GEN_SPECIAL_CHARS4)
            result.insert(QChar(chr));
    for (const char chr : ui->lePasswdExtraChars->text().toUtf8())
        result.insert(QChar(chr));

    _charset = QVector<QChar>(result.cbegin(), result.cend());
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
    SecureBuffer<QChar> password = Crypto::generateRandomPassword(_charset, ui->hsPasswordLength->value());
    ui->lePassword->setText(QString(password.data(), static_cast<int>(password.size())));
}

void PasswordGenerator::generatePassphrase()
{
    QString separator = ui->lePassphraseSeparator->text();
    separator = (separator.isEmpty() ? " " : separator);
    SecureBuffer<quint32> randomNums = Crypto::generateRandomUnsignedIntegers(_wordlist.size(), ui->hsPassphraseWordCount->value());

    qsizetype passphraseLength = 0;
    for (qsizetype i = 0; i < randomNums.size(); i++)
        passphraseLength += _wordlist.at(randomNums[i]).size();
    passphraseLength += ((separator.size() * (randomNums.size() - 1)));

    qsizetype passphraseIndex = 0;
    SecureBuffer<QChar> passphrase(passphraseLength);

    for (qsizetype randomNum = 0; randomNum < randomNums.size(); randomNum++) {
        if (randomNum != 0)
            for (QChar sep : separator)
                passphrase[passphraseIndex++] = sep;

        const QString& word = _wordlist.at(randomNums[randomNum]);
        for (qsizetype chr = 0; chr < word.size(); chr++) {
            switch (chr) {
                case 0:  passphrase[passphraseIndex++] = (ui->cbPassphraseWordCase->currentIndex() == 0 ? word.at(chr).toLower() : word.at(chr).toUpper()); break;
                default: passphrase[passphraseIndex++] = (ui->cbPassphraseWordCase->currentIndex() == 1 ? word.at(chr).toUpper() : word.at(chr).toLower()); break;
            }
        }
    }

    ui->lePassword->setText(QString(passphrase.data(), static_cast<int>(passphrase.size())));
}
