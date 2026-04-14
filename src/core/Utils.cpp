#include "Utils.h"
#include "Crypto.h"

void Utils::clearQString(QString& string)
{
    Crypto::zeroMemory(string.data(), (string.size() * sizeof(QChar)));
    string.clear();
}

void Utils::clearQLineEdit(QLineEdit* lineEdit)
{
    if (!lineEdit) return;
    QString text = lineEdit->text();
    lineEdit->setText(QString(text.size(), 'X'));
    lineEdit->clear();
    clearQString(text);
}

void Utils::clearQTextEdit(QTextEdit* textEdit)
{
    if (!textEdit) return;
    QString text = textEdit->toPlainText();
    textEdit->setPlainText(QString(text.size(), 'X'));
    textEdit->clear();
    clearQString(text);
}
