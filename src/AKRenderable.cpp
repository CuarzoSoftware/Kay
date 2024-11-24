#include <AKRenderable.h>

using namespace AK;

void AKRenderable::addDamage(const SkRegion &region) noexcept
{
    for (auto &it : m_targets)
        it.second.clientDamage.op(region, SkRegion::Op::kUnion_Op);
}
