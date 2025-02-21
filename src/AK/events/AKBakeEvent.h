#ifndef AKBAKEEVENT_H
#define AKBAKEEVENT_H

#include <include/core/SkCanvas.h>
#include <include/core/SkRegion.h>
#include <AK/events/AKEvent.h>
#include <AK/AKSurface.h>
#include <AK/AKChanges.h>
#include <AK/AKTime.h>

/**
 * @brief Render event.
 * @ingroup AKEvents
 */
class AK::AKBakeEvent : public AKEvent
{
public:
    AKBakeEvent(
        const AKChanges &changes,
        const AKTarget &target,
        const SkRegion &clip,
        SkRegion &damage,
        SkRegion &opaque,
        AKSurface &surface,
        UInt32 serial = AKTime::nextSerial(),
        UInt32 ms = AKTime::ms(),
        UInt64 us = AKTime::us()) noexcept :
        AKEvent(Bake, serial, ms, us),
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
    AKSurface &surface;

    SkCanvas &canvas() const noexcept
    {
        return *surface.surface()->getCanvas();
    }
};





#endif // AKBAKEEVENT_H
