#ifndef CZ_AKBAKEEVENT_H
#define CZ_AKBAKEEVENT_H

#include <CZ/skia/core/SkCanvas.h>
#include <CZ/skia/core/SkRegion.h>
#include <CZ/Events/CZEvent.h>
#include <CZ/AK/AKChanges.h>
#include <CZ/Ream/RSurface.h>

/**
 * @brief Render event.
 * @ingroup CZEvents
 */
class CZ::AKBakeEvent : public CZEvent
{
public:
    CZ_EVENT_DECLARE_COPY

    AKBakeEvent(
        const AKChanges &changes,
        const AKTarget &target,
        const SkRegion &clip,
        SkRegion &damage,
        SkRegion &opaque,
        std::shared_ptr<RSurface> surface) noexcept :
        CZEvent(Type::Bake),
        changes(changes),
        target(target),
        clip(clip),
        damage(damage),
        opaque(opaque),
        surface(surface)
    {}

    const AKChanges &changes;
    const AKTarget &target;

    /**
     * @brief Region in node-local coordinates that is not currently occluded (never nullptr).
     */
    const SkRegion &clip;

    /**
     * @brief Region in node-local coordinates used by both the AKScene to explicitly indicate regions that need
     * to be repainted, and to indicate new damage generated during the onBake() event (never nullptr).
     * The resulting damage should be the union of the incoming damage and the newly generated damage.
     */
    SkRegion &damage;

    /**
     * @brief The resulting opaque region during onBake() in node-local coordinates, persistent across calls (never nullptr).
     * Should be updated only when required.
     */
    SkRegion &opaque;

    /**
     * @brief The surface to render to.
     */
    std::shared_ptr<RSurface> surface;
};

#endif // CZ_AKBAKEEVENT_H
