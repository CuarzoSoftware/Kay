#ifndef AKDESTROYEVENT_H
#define AKDESTROYEVENT_H

#include <AK/events/AKEvent.h>
#include <AK/AKTime.h>

/**
 * @brief Queued after calling AKObject::destroyLater().
 *
 * Unlike others, this event is not notified through AKObject::event().
 *
 * @ingroup AKEvents
 */
class AK::AKDestroyEvent : public AKEvent
{
public:
    AKDestroyEvent() noexcept : AKEvent(Destroy, AKTime::nextSerial(), AKTime::ms(), AKTime::us()) {}
};

#endif // AKDESTROYEVENT_H
