#ifndef MKEYBOARD_H
#define MKEYBOARD_H

#include <Marco/Marco.h>
#include <AK/events/AKKeyboardKeyEvent.h>
#include <AK/events/AKKeyboardEnterEvent.h>
#include <AK/events/AKKeyboardLeaveEvent.h>
#include <AK/events/AKKeyboardModifiersEvent.h>
#include <AK/input/AKKeyboard.h>
#include <AK/AKWeak.h>

class AK::MKeyboard : public AKKeyboard
{
public:
    MKeyboard() = default;

    struct EventHistory
    {
        AKKeyboardKeyEvent key;
        AKKeyboardEnterEvent enter;
        AKKeyboardLeaveEvent leave;
        AKKeyboardModifiersEvent modifiers;
    };

    MSurface *focus() const noexcept { return m_focus; };
    const EventHistory &eventHistory() const noexcept { return m_eventHistory; };

private:
    friend class MApplication;
    EventHistory m_eventHistory;
    AKWeak<MSurface> m_focus;
};

#endif // MKEYBOARD_H
