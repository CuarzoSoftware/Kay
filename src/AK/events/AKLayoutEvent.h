#ifndef AKLAYOUTEVENT_H
#define AKLAYOUTEVENT_H

#include <AK/events/AKEvent.h>
#include <AK/AKTime.h>
#include <AK/AKBitset.h>

/**
 * @brief Layout change event.
 * @ingroup AKEvents
 */
class AK::AKLayoutEvent : public AKEvent
{
public:
    AKEVENT_DECLARE_COPY

    enum Changes
    {
        Pos = 1 << 0,
        Size = 1 << 1,
        Scale = 1 << 2
    };

    AKLayoutEvent(AKBitset<Changes> changes, UInt32 serial = AKTime::nextSerial(), UInt32 ms = AKTime::ms(), UInt64 us = AKTime::us()) noexcept :
        AKEvent(Layout, serial, ms, us),
        m_changes(changes)
    {}

    AKBitset<Changes> changes() const noexcept
    {
        return m_changes;
    }

    void setChanges(AKBitset<Changes> changes) noexcept
    {
        m_changes = changes;
    }

private:
    AKBitset<Changes> m_changes;
};

#endif // AKLAYOUTEVENT_H
