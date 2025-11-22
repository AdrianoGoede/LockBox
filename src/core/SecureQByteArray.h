#ifndef SECUREQBYTEARRAY_H
#define SECUREQBYTEARRAY_H

#include <QByteArray>

class SecureQByteArray : public QByteArray
{
public:
    SecureQByteArray() = default;
    explicit SecureQByteArray(const QByteArray&) = delete;
    SecureQByteArray(QByteArray&& other) noexcept : QByteArray(std::move(other)) {}
    ~SecureQByteArray();
    void wipe();
};

#endif // SECUREQBYTEARRAY_H
