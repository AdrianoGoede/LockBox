#ifndef SIZEONLYDEVICE_H
#define SIZEONLYDEVICE_H

#include <QIODevice>

class SizeOnlyDevice : public QIODevice
{
    Q_OBJECT

public:
    SizeOnlyDevice() { open(QIODevice::OpenModeFlag::WriteOnly); }
    bool isSequential() const override { return false; }
    qsizetype size() const override { return _size; }

protected:
    qsizetype readData(char*, qsizetype) override { return -1; }
    qsizetype writeData(const char*, qsizetype len) override { _size += len; return len; }

private:
    qsizetype _size = 0;
};

#endif // SIZEONLYDEVICE_H
