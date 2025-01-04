#ifndef AKPOINTEREVENT_H
#define AKPOINTEREVENT_H

#include <AK/events/AKInputEvent.h>

/**
 * @brief Base class for pointer events.
 *
 * All pointer events share the same AKEvent::Type::Pointer type.
 */
class AK::AKPointerEvent : public AKInputEvent
{
protected:
    AKPointerEvent(Subtype subtype, UInt32 serial, UInt32 ms, UInt64 us, AKInputDevice *device) :
        AKInputEvent(Type::Pointer, subtype, serial, ms, us, device)
    {}
};

#endif // AKPOINTEREVENT_H
