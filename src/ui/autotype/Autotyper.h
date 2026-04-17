#ifndef AUTOTYPER_H
#define AUTOTYPER_H

#include "../../core/SecureBuffer.h"
#include <QString>

class Autotyper
{
public:
    static Autotyper* create();
    virtual ~Autotyper() = default;
    virtual bool isAvailable() const = 0;
    virtual void typeSequence(const SecureBuffer<QChar>& sequence, uint64_t delay = 1000) const = 0;
    virtual void typeSequence(const QString& sequence, uint64_t delay = 1000) const = 0;
};

#endif // AUTOTYPER_H
