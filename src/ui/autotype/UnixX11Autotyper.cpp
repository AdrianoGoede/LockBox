#include "UnixX11Autotyper.h"
#include <X11/extensions/XTest.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>
#include <X11/Xutil.h>

Autotyper* Autotyper::create() { return new UnixX11Autotyper(); }

UnixX11Autotyper::UnixX11Autotyper() { _display = XOpenDisplay(nullptr); }

UnixX11Autotyper::~UnixX11Autotyper() { if (_display) XCloseDisplay(_display); }

bool UnixX11Autotyper::isAvailable() const
{
    if (!_display) return false;
    int eventBase, errorBase, majorVersion, minorVersion;
    return (XTestQueryExtension(_display, &eventBase, &errorBase, &majorVersion, &minorVersion) && qEnvironmentVariableIsEmpty("WAYLAND_DISPLAY"));
}

void UnixX11Autotyper::typeSequence(const SecureBuffer<QChar>& sequence, uint64_t delay) const
{
    if (!_display) return;

    QThread::msleep(delay);
    for (size_t i = 0; i < sequence.size(); i++)
        typeQChar(sequence[i]);

    XFlush(_display);
}

void UnixX11Autotyper::typeSequence(const QString& sequence, uint64_t delay) const
{
    if (!_display) return;

    QThread::msleep(delay);
    for (const QChar& qchar : sequence)
        typeQChar(qchar);

    XFlush(_display);
}

void UnixX11Autotyper::typeQChar(QChar qchar) const
{
    char16_t unicode = qchar.unicode();

    KeySym keySym = (unicode < 0x100 ? unicode : (0x01000000 + unicode));
    KeyCode keyCode = findOrRemapKey(keySym);
    if (keyCode == 0) return;

    KeySym lower, upper;
    XConvertCase(keySym, &lower, &upper);
    uint32_t modifier = ((lower != upper && keySym == upper) ? XK_Shift_L : 0);

    sendKey(keyCode, modifier, true);
    sendKey(keyCode, modifier, false);
}

KeyCode UnixX11Autotyper::findOrRemapKey(KeySym keySym) const
{
    KeyCode keyCode = XKeysymToKeycode(_display, keySym);
    if (keyCode != 0) return keyCode;

    int minKeyCode, maxKeyCode;
    XDisplayKeycodes(_display, &minKeyCode, &maxKeyCode);

    keyCode = maxKeyCode;
    KeySym remapped = keySym;
    XChangeKeyboardMapping(_display, keyCode, 1, &remapped, 1);
    XSync(_display, false);

    QThread::msleep(10);
    return keyCode;
}

void UnixX11Autotyper::sendKey(KeyCode keycode, unsigned int modifiers, bool press) const
{
    if (modifiers)
        XTestFakeKeyEvent(_display, XKeysymToKeycode(_display, modifiers), press, 0);
    XTestFakeKeyEvent(_display, keycode, press, 0);

    if (modifiers)
        XTestFakeKeyEvent(_display, XKeysymToKeycode(_display, modifiers), !press, 0);
    XSync(_display, false);
}