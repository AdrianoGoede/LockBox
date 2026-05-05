#include "WindowsAutotyper.h"
#include <QThread>
#include <windows.h>

Autotyper* Autotyper::create() { return new WindowsAutotyper(); }

bool WindowsAutotyper::isAvailable() const
{
    DWORD sessionId = 0;
    ProcessIdToSessionId(GetCurrentProcessId(), &sessionId);
    return (sessionId != 0);
}

void WindowsAutotyper::typeSequence(const SecureBuffer<QChar>& sequence, uint64_t delay) const
{
    QThread::msleep(delay);
    for (const QChar& qchar : sequence)
        typeQChar(qchar);
}

void WindowsAutotyper::typeSequence(const QString& sequence, uint64_t delay) const
{
    QThread::msleep(delay);
    for (const QChar& qchar : sequence)
        typeQChar(qchar);
}

void WindowsAutotyper::typeQChar(const QChar& qchar) const
{
    if (qchar == '\t' || qchar == '\n' || qchar == '\r')
        typeControlQChar(qchar);
    else
        typeUnicodeQChar(qchar);
}

void WindowsAutotyper::typeControlQChar(const QChar& qchar) const
{
    WORD vk = (qchar == '\t' ? VK_TAB : VK_RETURN);
    INPUT inputs[2] = {};

    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = vk;
    inputs[0].ki.dwFlags = 0;

    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = vk;
    inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(2, inputs, sizeof(INPUT));
    QThread::msleep(5);
}

void WindowsAutotyper::typeUnicodeQChar(const QChar& qchar) const
{
    char16_t unicode = qchar.unicode();
    INPUT inputs[2] = {};

    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = 0;
    inputs[0].ki.wScan = unicode;
    inputs[0].ki.dwFlags = KEYEVENTF_UNICODE;

    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = 0;
    inputs[1].ki.wScan = unicode;
    inputs[1].ki.dwFlags = (KEYEVENTF_UNICODE | KEYEVENTF_KEYUP);

    SendInput(2, inputs, sizeof(INPUT));
    QThread::msleep(5);
}