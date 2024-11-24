#include <AKRenderable.h>

using namespace AK;

void AKRenderable::addDamage(const SkRegion &region) noexcept
{
    for (auto &it : m_targets)
        it.second.clientDamage.op(region, SkRegion::Op::kUnion_Op);
}

void AKRenderable::addDamage(const SkIRect &rect) noexcept
{
    for (auto &it : m_targets)
        it.second.clientDamage.op(rect, SkRegion::Op::kUnion_Op);
}
