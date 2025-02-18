#ifndef AKSTATEEVENT_H
#define AKSTATEEVENT_H

#include <AK/events/AKEvent.h>

/**
 * @brief Base class for state events.
 * @ingroup AKEvents
 */
class AK::AKStateEvent : public AKEvent
{
protected:
    AKStateEvent(Subtype subtype, UInt32 serial, UInt32 ms, UInt64 us) noexcept :
        AKEvent(Type::State, subtype, serial, ms, us) {}
};

#endif // AKSTATEEVENT_H
