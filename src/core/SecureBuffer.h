#ifndef SECUREBUFFER_H
#define SECUREBUFFER_H

#include <new>
#include <sodium.h>
#include <type_traits>

template <typename T>
class SecureBuffer
{
    static_assert(std::is_trivially_copyable<T>::value, "SecureBuffer only supports trivially copyable types");

public:
    SecureBuffer(size_t size = 0) : _size(size) {
        if (_size > 0) {
            _buffer = static_cast<T*>(sodium_malloc(_size * sizeof(T)));
            if (!_buffer) throw std::bad_alloc();
        }
    };

    SecureBuffer(SecureBuffer&& other) noexcept : _buffer(other._buffer), _size(other._size) {
        other._buffer = nullptr;
        other._size = 0;
    }

    SecureBuffer& operator=(SecureBuffer&& other) noexcept {
        if (this != &other) {
            if (_buffer) sodium_free(_buffer);
            _buffer = other._buffer;
            _size = other._size;
            other._buffer = nullptr;
            other._size = 0;
        }
        return *this;
    }

    ~SecureBuffer() { if (_buffer) sodium_free(_buffer); }

    T* data() { return _buffer; }
    const T* data() const { return _buffer; }
    size_t size() const { return _size; }
    size_t byteSize() const { return (_size * sizeof(T)); }
    bool isEmpty() const { return (_size == 0); }
    T& operator[](size_t i) { return _buffer[i]; }
    const T& operator[](size_t i) const { return _buffer[i]; }

    SecureBuffer(const SecureBuffer&) = delete;
    SecureBuffer& operator=(const SecureBuffer&) = delete;

private:
    T* _buffer = nullptr;
    size_t _size = 0;
};

#endif // SECUREBUFFER_H
