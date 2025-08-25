#include <CZ/skia/core/SkCanvas.h>
#include <CZ/AK/Nodes/AKRoundContainer.h>
#include <CZ/AK/Events/AKBakeEvent.h>
#include <CZ/skia/core/SkPaint.h>
#include <CZ/Ream/RSurface.h>

using namespace CZ;

void AKRoundContainer::bakeEvent(const AKBakeEvent &event)
{
    borderRadius().addDamage(
        event.surface.size(),
        &event.clip,
        &event.damage);

    bakeChildren(event);

    borderRadius().clipCorners(
        &event.canvas(),
        &event.clip,
        &event.damage,
        &event.opaque);
}
