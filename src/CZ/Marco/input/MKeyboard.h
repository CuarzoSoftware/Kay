#ifndef MKEYBOARD_H
#define MKEYBOARD_H

#include <CZ/Marco/Marco.h>
#include <CZ/Events/CZKeyboardKeyEvent.h>
#include <CZ/Events/CZKeyboardEnterEvent.h>
#include <CZ/Events/CZKeyboardLeaveEvent.h>
#include <CZ/Events/CZKeyboardModifiersEvent.h>
#include <CZ/AK/Input/AKKeyboard.h>
#include <CZ/Core/CZWeak.h>

class CZ::MKeyboard : public AKKeyboard
{
public:
    MKeyboard() = default;

    struct EventHistory
    {
        CZKeyboardKeyEvent key;
        CZKeyboardEnterEvent enter;
        CZKeyboardLeaveEvent leave;
        CZKeyboardModifiersEvent modifiers;
    };

    MSurface *focus() const noexcept { return m_focus; };
    const EventHistory &eventHistory() const noexcept { return m_eventHistory; };

private:
    friend class MApplication;
    EventHistory m_eventHistory;
    CZWeak<MSurface> m_focus;
};

#endif // MKEYBOARD_H
