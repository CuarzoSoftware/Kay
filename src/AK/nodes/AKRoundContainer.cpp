#include <include/core/SkCanvas.h>
#include <AK/nodes/AKRoundContainer.h>
#include <AK/AKBrush.h>
#include <AK/AKSurface.h>

using namespace AK;

void AKRoundContainer::onBake(OnBakeParams *params)
{
    borderRadius().addDamage(
        params->surface->size(),
        params->clip,
        params->damage);

    bakeChildren(params);

    borderRadius().clipCorners(
        params->surface->surface()->getCanvas(),
        params->damage,
        params->opaque);    
}
