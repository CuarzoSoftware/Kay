#include <AK/nodes/AKBakeable.h>
#include <AK/events/AKRenderEvent.h>
#include <AK/AKSurface.h>
#include <AK/AKPainter.h>

using namespace AK;

std::shared_ptr<AKSurface> AKBakeable::surface() const noexcept
{
    return m_surface;
}

void AKBakeable::renderEvent(const AKRenderEvent &p)
{
    if (!m_surface)
        return;

    p.painter.bindTextureMode({
        .texture = m_surface->image(),
        .pos = p.rect.topLeft(),
        .srcRect = SkRect::MakeWH(p.rect.width(), p.rect.height()),
        .dstSize = p.rect.size(),
        .srcTransform = AKTransform::Normal,
        .srcScale = SkScalar(m_surface->scale())
    });

    p.painter.drawRegion(p.damage);
}

bool AKBakeable::event(const AKEvent &event)
{
    switch (event.type())
    {
    case AKEvent::BakeEvent:
        bakeEvent((const AKBakeEvent&)event);
        break;
    default:
        return AKRenderable::event(event);
    }

    return true;
}
