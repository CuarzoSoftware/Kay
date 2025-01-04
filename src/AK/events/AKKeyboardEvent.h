#ifndef AKKEYBOARDEVENT_H
#define AKKEYBOARDEVENT_H

#include <AK/events/AKInputEvent.h>

/**
 * @brief Base class for keyboard events.
 *
 * All keyboard events share the same AKEvent::Type::Keyboard type.
 */
class AK::AKKeyboardEvent : public AKInputEvent
{
protected:
    AKKeyboardEvent(Subtype subtype, UInt32 serial, UInt32 ms, UInt64 us, AKInputDevice *device) noexcept :
        AKInputEvent(Type::Keyboard, subtype, serial, ms, us, device)
    {}
};

#endif // AKKEYBOARDEVENT_H
