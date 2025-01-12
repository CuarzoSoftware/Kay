#ifndef AKSTATEDEACTIVATEDEVENT_H
#define AKSTATEDEACTIVATEDEVENT_H

#include <AK/events/AKStateEvent.h>
#include <AK/AKTime.h>

class AK::AKStateDeactivatedEvent : public AKStateEvent
{
public:
    AKStateDeactivatedEvent(UInt32 serial = AKTime::nextSerial(),
                          UInt32 ms = AKTime::ms(), UInt64 us = AKTime::us()) noexcept :
        AKStateEvent(Subtype::Deactivated, serial, ms, us) {}
};


#endif // AKSTATEDEACTIVATEDEVENT_H
