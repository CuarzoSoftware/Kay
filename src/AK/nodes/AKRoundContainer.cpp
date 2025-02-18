#include <include/core/SkCanvas.h>
#include <AK/nodes/AKRoundContainer.h>
#include <AK/AKBrush.h>
#include <AK/AKSurface.h>

using namespace AK;

void AKRoundContainer::onBake(const BakeEvent &event)
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
