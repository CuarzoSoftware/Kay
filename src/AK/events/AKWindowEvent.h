#ifndef AKSTATEEVENT_H
#define AKSTATEEVENT_H

#include <AK/events/AKEvent.h>

/**
 * @brief Base class for window events.
 * @ingroup AKEvents
 */
class AK::AKWindowEvent : public AKEvent
{
protected:
    AKWindowEvent(Type type, UInt32 serial, UInt32 ms, UInt64 us) noexcept :
        AKEvent(type, serial, ms, us) {}
};

#endif // AKSTATEEVENT_H
