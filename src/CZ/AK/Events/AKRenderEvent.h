#ifndef CZ_AKRENDEREVENT_H
#define CZ_AKRENDEREVENT_H

#include <CZ/Events/CZEvent.h>
#include <CZ/AK/AK.h>
#include <CZ/skia/core/SkRegion.h>
#include <CZ/Ream/RSurface.h>

/**
 * @brief Render event.
 *
 * Unlike bake events, the destination coords are relative to the current target surface.
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

    // The current target
    const AKTarget &target;

    // Damage within rect
    const SkRegion &damage;

    // Node rect relative to the current target surface
    const SkIRect &rect;

    // The pass to use for rendering
    std::shared_ptr<RPass> pass;

    // Indicates if damage only contains rects from the opaque region (blending should be disabled)
    bool opaque;
};

#endif // CZ_AKRENDEREVENT_H
