#ifndef MACOSAUTOTYPER_H
#define MACOSAUTOTYPER_H

#include "Autotyper.h"
#include "../../core/SecureBuffer.h"

class MacOsAutotyper : public Autotyper
{
public:
    MacOsAutotyper() = default;
    ~MacOsAutotyper() = default;
    bool isAvailable() const override;
    void typeSequence(const SecureBuffer<QChar>& sequence, uint64_t delay) const override;
    void typeSequence(const QString& sequence, uint64_t delay) const override;
};

#endif // MACOSAUTOTYPER_H
