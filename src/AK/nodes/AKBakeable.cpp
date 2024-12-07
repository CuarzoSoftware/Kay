#include <AK/nodes/AKBakeable.h>
#include <AK/AKSurface.h>
#include <AK/AKPainter.h>

using namespace AK;

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
        .srcScale = t->bake->scale().x()
    });

    painter->drawRegion(damage);
}
