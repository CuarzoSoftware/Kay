#ifndef AKRENDEREVENT_H
#define AKRENDEREVENT_H

#include <AK/events/AKEvent.h>
#include <AK/AKTime.h>

#include <skia/core/SkRegion.h>

/**
 * @brief Render event.
 * @ingroup AKEvents
 */
class AK::AKRenderEvent : public AKEvent
{
public:
    AKRenderEvent(
        const AKTarget &target,
        const SkRegion &damage,
        const SkIRect &rect,
        AKPainter &painter,
        UInt32 serial = AKTime::nextSerial(),
        UInt32 ms = AKTime::ms(),
        UInt64 us = AKTime::us()) noexcept :
        AKEvent(Render, serial, ms, us),
        target(target),
        damage(damage),
        rect(rect),
        painter(painter)
    {}

    const AKTarget &target;
    const SkRegion &damage;
    const SkIRect &rect;
    AKPainter &painter;
};

#endif // AKRENDEREVENT_H
