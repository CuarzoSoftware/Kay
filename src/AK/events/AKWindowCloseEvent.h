#ifndef AKWINDOWCLOSEEVENT_H
#define AKWINDOWCLOSEEVENT_H

#include <AK/events/AKWindowEvent.h>
#include <AK/AKWindowState.h>
#include <AK/AKBitset.h>
#include <AK/AKTime.h>

class AK::AKWindowCloseEvent : public AKWindowEvent
{
public:
    AKEVENT_DECLARE_COPY

    /**
     * @brief Event constructor.
     */
    AKWindowCloseEvent(UInt32 serial = AKTime::nextSerial(),
                       UInt32 ms = AKTime::ms(), UInt64 us = AKTime::us()) noexcept :
        AKWindowEvent(WindowClose, serial, ms, us) {}
};

#endif // AKWINDOWCLOSEEVENT_H
