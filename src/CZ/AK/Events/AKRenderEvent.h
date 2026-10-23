#ifndef CZ_AKRENDEREVENT_H
#define CZ_AKRENDEREVENT_H

#include <CZ/Core/Events/CZEvent.h>
#include <CZ/AK/AK.h>
#include <CZ/skia/core/SkRegion.h>
#include <CZ/Ream/RSurface.h>

/**
 * @brief Render event, sent to AKRenderable nodes to draw into the current target.
 *
 * Received through AKRenderable::renderEvent() during AKScene::render().
 * Unlike bake events, the destination coords are relative to the current target surface.
 *
 * @ingroup AKEvents
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

    /**
     * @brief The target currently being rendered.
     */
    const AKTarget &target;

    /**
     * @brief The region to repaint within rect (restrict all drawing to it).
     */
    const SkRegion &damage;

    /**
     * @brief The node rect relative to the current target surface.
     */
    const SkIRect &rect;

    /**
     * @brief The Ream pass to render with (see RPass::getPainter()).
     */
    std::shared_ptr<RPass> pass;

    /**
     * @brief Whether damage only contains rects from the opaque region (blending should be disabled).
     */
    bool opaque;
};

#endif // CZ_AKRENDEREVENT_H
