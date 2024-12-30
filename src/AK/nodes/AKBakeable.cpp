#include <AK/nodes/AKBakeable.h>
#include <AK/AKSurface.h>
#include <AK/AKPainter.h>

using namespace AK;

std::shared_ptr<AKSurface> AKBakeable::getSurface(AKTarget *target) const noexcept
{
    auto it = m_targets.find(target);
    return it == m_targets.end() ? nullptr : it->second.bake;
}

void AKBakeable::onRender(AKPainter *painter, const SkRegion &damage)
{
    if (!t || !t->bake->image())
        return;

    painter->bindTextureMode({
        .texture = t->bake->image(),
        .pos = SkIPoint(rect().x(), rect().y()),
        .srcRect = SkRect::MakeWH(rect().width(), rect().height()),
        .dstSize = rect().size(),
        .srcTransform = AKTransform::Normal,
        .srcScale = SkScalar(t->bake->scale())
    });

    painter->drawRegion(damage);
}
