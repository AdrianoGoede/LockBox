#ifndef MEMORYWRITER_H
#define MEMORYWRITER_H

#include <QIODevice>

class MemoryWriter : public QIODevice {
public:
    MemoryWriter(void* ptr, qsizetype size) : _ptr(reinterpret_cast<char*>(ptr)), _size(size) { open(QIODevice::OpenModeFlag::WriteOnly); }
    bool isSequential() const override { return false; }
    qsizetype size() const override { return _size; }
    qsizetype pos() const override { return _pos; }
    qint64 readData(char*, qint64) override { return -1; }
    bool seek(qsizetype pos) override {
        if (pos < 0 || pos > _size)
            return false;
        _pos = pos;
        return true;
    }
    qint64 writeData(const char* data, qint64 len) override {
        if (_pos + len > _size)
            len = _size - _pos;
        if (len <= 0) return 0;
        std::memcpy((_ptr + _pos), data, len);
        _pos += len;
        return len;
    }

private:
    char* _ptr;
    qsizetype _size, _pos = 0;
};

#endif // MEMORYWRITER_H
