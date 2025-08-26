#include <CZ/AK/Nodes/AKBakeable.h>
#include <CZ/AK/Events/AKRenderEvent.h>
#include <CZ/AK/AKLog.h>
#include <CZ/Ream/RSurface.h>
#include <CZ/Ream/RPass.h>

using namespace CZ;

std::shared_ptr<RSurface> AKBakeable::surface() const noexcept
{
    return m_surface;
}

void AKBakeable::renderEvent(const AKRenderEvent &e)
{
    if (!m_surface)
        return;

    auto *p { e.pass->getPainter() };

    RDrawImageInfo info {};
    info.image = m_surface->image();
    info.src = m_surface->geometry().dst;
    info.srcTransform = m_surface->geometry().transform;
    info.dst = e.rect;

    p->drawImage(info, &e.damage);
}

bool AKBakeable::event(const CZEvent &event) noexcept
{
    switch (event.type())
    {
    case CZEvent::Type::Bake:
        bakeEvent((const AKBakeEvent&)event);
        break;
    default:
        return AKRenderable::event(event);
    }

    return true;
}
