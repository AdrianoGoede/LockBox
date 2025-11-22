#include "SecureQByteArray.h"
#include "sodium/utils.h"

SecureQByteArray::~SecureQByteArray() { this->wipe(); }

void SecureQByteArray::wipe()
{
    if (!this->isEmpty())
        sodium_memzero((void*)this->data(), this->size());
    this->clear();
}
