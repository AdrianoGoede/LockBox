#ifndef UNIXX11AUTOTYPER_H
#define UNIXX11AUTOTYPER_H

#include "Autotyper.h"
#include "../../core/SecureBuffer.h"
#include <QString>
#include <QThread>
#include <X11/Xlib.h>

class UnixX11Autotyper : public Autotyper
{
public:
    UnixX11Autotyper();
    ~UnixX11Autotyper();
    bool isAvailable() const override;
    void typeSequence(const SecureBuffer<QChar>& sequence, uint64_t delay) const override;
    void typeSequence(const QString& sequence, uint64_t delay) const override;

private:
    Display* _display = nullptr;
    void typeQChar(QChar qchar) const;
    void sendKey(KeyCode keycode, unsigned int modifiers, bool press) const;
};

#endif // UNIXX11AUTOTYPER_H