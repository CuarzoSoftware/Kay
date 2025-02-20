#ifndef AKKEYBOARDEVENT_H
#define AKKEYBOARDEVENT_H

#include <AK/events/AKInputEvent.h>

/**
 * @brief Base class for keyboard events.
 * @ingroup AKEvents
 *
 * All keyboard events share the same AKEvent::Type::Keyboard type.
 */
class AK::AKKeyboardEvent : public AKInputEvent
{
protected:
    AKKeyboardEvent(Type type, UInt32 serial, UInt32 ms, UInt64 us, AKInputDevice *device) noexcept :
        AKInputEvent(type, serial, ms, us, device)
    {}
};

#endif // AKKEYBOARDEVENT_H
