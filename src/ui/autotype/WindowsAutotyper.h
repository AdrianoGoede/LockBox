#ifndef WINDOWSAUTOTYPER_H
#define WINDOWSAUTOTYPER_H

#include "Autotyper.h"
#include "../../core/SecureBuffer.h"

class WindowsAutotyper : public Autotyper
{
public:
    WindowsAutotyper() = default;
    ~WindowsAutotyper() = default;
    bool isAvailable() const override;
    void typeSequence(const SecureBuffer<QChar>& sequence, uint64_t delay) const override;
    void typeSequence(const QString& sequence, uint64_t delay) const override;

private:
    void typeQChar(const QChar& qchar) const;
    void typeControlQChar(const QChar& qchar) const;
    void typeUnicodeQChar(const QChar& qchar) const;
};

#endif // WINDOWSAUTOTYPER_H
