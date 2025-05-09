#include <AK/events/AKWindowStateEvent.h>
#include <AK/events/AKEvent.h>
#include <AK/nodes/AKRenderable.h>
#include <AK/AKSceneTarget.h>

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

const SkRegion &AKRenderable::damage() const noexcept
{
    static const SkRegion emptyRegion;
    return m_targets.empty() ? emptyRegion : m_targets.begin()->second.clientDamage;
}

bool AKRenderable::event(const AKEvent &event)
{
    switch (event.type())
    {
    case AKEvent::Render:
        renderEvent((const AKRenderEvent&)event);
        break;
    default:
        return AKNode::event(event);
    }

    return true;
}

void AKRenderable::windowStateEvent(const AKWindowStateEvent &event)
{
    AKNode::windowStateEvent(event);

    if (diminishOpacityOnInactive() && event.changes().check(AKActivated))
    {
        addDamage(AK_IRECT_INF);
        repaint();
        event.accept();
    }
}

void AKRenderable::handleCommonChanges() noexcept
{
    const auto &c { changes() };

    if (m_renderableHint == SolidColor)
    {
        if (c.testAnyOf(CHColor, CHOpacity, CHColorFactor, CHCustomBlendFuncEnabled) ||
            (customBlendFuncEnabled() && c.test(CHCustomBlendFunc)))
            addDamage(AK_IRECT_INF);

        m_colorHint = opacity() < 1.f || colorFactor().fA < 1.f ? ColorHint::Translucent : ColorHint::Opaque;
    }
    else
    {
        if (c.testAnyOf(CHOpacity, CHColorFactor, CHCustomBlendFuncEnabled, CHCustomTextureColorEnabled) ||
            (customTextureColorEnabled() && c.test(CHColor)) ||
            (customBlendFuncEnabled() && c.test(CHCustomBlendFunc)))
            addDamage(AK_IRECT_INF);

        m_colorHint = opacity() < 1.f || colorFactor().fA < 1.f ? ColorHint::Translucent : ColorHint::UseRegion;
    }

    if (diminishOpacityOnInactive() && !activated())
        m_colorHint = ColorHint::Translucent;
}
