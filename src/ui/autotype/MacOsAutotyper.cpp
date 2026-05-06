#include "MacOsAutotyper.h"

Autotyper* Autotyper::create() { return new MacOsAutotyper(); }

bool MacOsAutotyper::isAvailable() const { return false; }

void MacOsAutotyper::typeSequence(const SecureBuffer<QChar>& sequence, uint64_t delay) const { /* TO DO */ }

void MacOsAutotyper::typeSequence(const QString& sequence, uint64_t delay) const { /* TO DO */ }
