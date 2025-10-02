#ifndef CZ_AKKEYMAP_H
#define CZ_AKKEYMAP_H

#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-compose.h>
#include <CZ/AK/AKObject.h>
#include <CZ/Events/CZKeyboardEnterEvent.h>
#include <CZ/Events/CZKeyboardKeyEvent.h>
#include <CZ/Events/CZKeyboardModifiersEvent.h>
#include <CZ/Events/CZKeyboardLeaveEvent.h>
#include <CZ/Core/CZWeak.h>

class CZ::AKKeyboard : public AKObject
{
public:
    struct EventHistory
    {
        CZKeyboardEnterEvent enter;
        CZKeyboardKeyEvent key;
        CZKeyboardModifiersEvent modifiers;
        CZKeyboardLeaveEvent leave;
    };

    const EventHistory &history() const noexcept { return m_history; }
    AKScene *focus() const noexcept { return m_focus; }
    CZSignal<> onFocusChanged;

protected:
    AKKeyboard() noexcept;

private:
    friend class AKApp;
    friend class AKScene;
    CZWeak<AKScene> m_focus;
    EventHistory m_history {};
};

#endif // CZ_AKKEYMAP_H
