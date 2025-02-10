#include <AK/nodes/AKBakeable.h>
#include <AK/AKSurface.h>
#include <AK/AKPainter.h>

using namespace AK;

std::shared_ptr<AKSurface> AKBakeable::surface() const noexcept
{
    return m_surface;
}

void AKBakeable::onRender(AKPainter *painter, const SkRegion &damage, const SkIRect &rect)
{
    if (!m_surface)
        return;

    painter->bindTextureMode({
        .texture = m_surface->image(),
        .pos = rect.topLeft(),
        .srcRect = SkRect::MakeWH(rect.width(), rect.height()),
        .dstSize = rect.size(),
        .srcTransform = AKTransform::Normal,
        .srcScale = SkScalar(m_surface->scale())
    });

    painter->drawRegion(damage);
}
