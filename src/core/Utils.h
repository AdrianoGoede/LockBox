#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QLineEdit>
#include <QTextEdit>

namespace Utils {
    void clearQString(QString& string);
    void clearQLineEdit(QLineEdit* lineEdit);
    void clearQTextEdit(QTextEdit* textEdit);
}

#endif // UTILS_H
