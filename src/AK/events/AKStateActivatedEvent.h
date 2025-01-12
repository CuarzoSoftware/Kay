#ifndef AKSTATEACTIVATEDEVENT_H
#define AKSTATEACTIVATEDEVENT_H

#include <AK/events/AKStateEvent.h>
#include <AK/AKTime.h>

class AK::AKStateActivatedEvent : public AKStateEvent
{
public:
    AKStateActivatedEvent(UInt32 serial = AKTime::nextSerial(),
                          UInt32 ms = AKTime::ms(), UInt64 us = AKTime::us()) noexcept :
        AKStateEvent(Subtype::Activated, serial, ms, us) {}
};

#endif // AKSTATEACTIVATEDEVENT_H
