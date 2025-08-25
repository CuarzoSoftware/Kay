#ifndef CZ_AKRENDEREVENT_H
#define CZ_AKRENDEREVENT_H

#include <CZ/Events/CZEvent.h>
#include <CZ/AK/AK.h>
#include <CZ/skia/core/SkRegion.h>
#include <CZ/Ream/RSurface.h>

/**
 * @brief Render event.
 * @ingroup CZEvents
 */
class CZ::AKRenderEvent : public CZEvent
{
public:
    CZ_EVENT_DECLARE_COPY
    AKRenderEvent(
        const AKTarget &target,
        const SkRegion &damage,
        const SkIRect &rect,
        std::shared_ptr<RPass> pass,
        bool opaque) noexcept :
        CZEvent(Type::Render),
        target(target),
        damage(damage),
        rect(rect),
        pass(pass),
        opaque(opaque)
    {}

    const AKTarget &target;
    const SkRegion &damage;
    const SkIRect &rect;
    std::shared_ptr<RPass> pass;
    bool opaque;
};

#endif // CZ_AKRENDEREVENT_H
