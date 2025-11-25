#include <CZ/Core/Events/CZWindowStateEvent.h>
#include <CZ/Core/Events/CZEvent.h>
#include <CZ/Core/Events/CZColorSchemeEvent.h>
#include <CZ/AK/Nodes/AKRenderable.h>
#include <CZ/AK/AKTarget.h>
#include <CZ/AK/AKTheme.h>
#include <CZ/Ream/RPass.h>

using namespace CZ;

AKRenderable::AKRenderable(RenderableHint hint, AKNode *parent) noexcept :
    AKNode(parent),
    m_renderableHint(hint)
{
    m_caps |= RenderableBit;
}

void AKRenderable::addDamage(const SkRegion &region) noexcept
{
    for (auto &it : m_targets)
        it.second.damage.op(region, SkRegion::Op::kUnion_Op);
}

void AKRenderable::addDamage(const SkIRect &rect) noexcept
{
    for (auto &it : m_targets)
        it.second.damage.op(rect, SkRegion::Op::kUnion_Op);
}

const SkRegion &AKRenderable::damage() const noexcept
{
    static const SkRegion emptyRegion;
    // All targets have the same damage to it doesn't matter which one is chosen
    return m_targets.empty() ? emptyRegion : m_targets.begin()->second.damage;
}

bool AKRenderable::event(const CZEvent &event) noexcept
{
    switch (event.type())
    {
    case CZEvent::Type::Render:
        renderEvent((const AKRenderEvent&)event);
        break;
    default:
        return AKNode::event(event);
    }

    return true;
}

void AKRenderable::windowStateEvent(const CZWindowStateEvent &event)
{
    AKNode::windowStateEvent(event);

    if (diminishOpacityOnInactive() && event.changes.has(CZWinActivated))
    {
        addDamage(AK_IRECT_INF);
        repaint();
        event.accept();
    }
}

void AKRenderable::handleCommonChanges() noexcept
{
    const auto &c { changes() };

    if (m_renderableHint == RenderableHint::SolidColor)
    {
        if (c.testAnyOf(CHColor, CHOpacity, CHColorFactor, CHCustomBlendModeEnabled) || (customBlendModeEnabled() && c.test(CHCustomBlendMode)))
            addDamage(AK_IRECT_INF);

        if (customBlendModeEnabled())
        {
            m_colorHint = customBlendMode() == RBlendMode::Src ? ColorHint::Opaque : ColorHint::Translucent;
        }
        else
            m_colorHint = (opacity() * color4f().fA * colorFactor().fA) < 1.f ? ColorHint::Translucent : ColorHint::Opaque;
    }
    else // Image
    {
        if (c.testAnyOf(CHOpacity, CHColorFactor, CHCustomBlendModeEnabled, CHReplaceImageColorEnabled) ||
            (replaceImageColorEnabled() && c.test(CHColor)) ||
            (customBlendModeEnabled() && c.test(CHCustomBlendMode)))
            addDamage(AK_IRECT_INF);

        m_colorHint = (opacity() < 1.f || colorFactor().fA < 1.f) ? ColorHint::Translucent : ColorHint::UseRegion;
    }

    if (diminishOpacityOnInactive() && !activated())
        m_colorHint = ColorHint::Translucent;
}
