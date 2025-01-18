#include <AK/events/AKEvent.h>
#include <AK/nodes/AKRenderable.h>
#include <AK/AKTarget.h>

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
    return currentTarget() == nullptr ? emptyRegion : m_targets[currentTarget()].clientDamage;
}

void AKRenderable::onEvent(const AKEvent &event)
{
    AKNode::onEvent(event);

    if (diminishOpacityOnInactive() && event.type() == AKEvent::Type::State)
    {
        if (event.subtype() == AKEvent::Subtype::Activated || event.subtype() == AKEvent::Subtype::Deactivated)
        {
            addDamage(AK_IRECT_INF);
            repaint();
            return;
        }
    }
}

void AKRenderable::handleCommonChanges() noexcept
{
    const auto &c { changes() };

    if (m_renderableHint == SolidColor)
    {
        if (c.test(Chg_Color) ||
            c.test(Chg_Opacity) ||
            c.test(Chg_ColorFactor) ||
            c.test(Chg_CustomBlendFuncEnabled) ||
            (customBlendFuncEnabled() && c.test(Chg_CustomBlendFunc)))
            addDamage(AK_IRECT_INF);

        m_colorHint = opacity() < 1.f || colorFactor().fA < 1.f ? ColorHint::Translucent : ColorHint::Opaque;
    }
    else
    {
        if (c.test(Chg_Opacity) ||
            c.test(Chg_ColorFactor) ||
            c.test(Chg_CustomBlendFuncEnabled) ||
            c.test(Chg_CustomTextureColorEnabled) ||
            (customTextureColorEnabled() && c.test(Chg_Color)) ||
            (customBlendFuncEnabled() && c.test(Chg_CustomBlendFunc)))
            addDamage(AK_IRECT_INF);

        m_colorHint = opacity() < 1.f || colorFactor().fA < 1.f ? ColorHint::Translucent : ColorHint::UseRegion;
    }

    if (diminishOpacityOnInactive() && !activated())
        m_colorHint = ColorHint::Translucent;
}
