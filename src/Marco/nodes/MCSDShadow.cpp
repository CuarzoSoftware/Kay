#include <AK/AKLog.h>
#include <Marco/private/MSurfacePrivate.h>
#include <Marco/nodes/MCSDShadow.h>
#include <Marco/roles/MSurface.h>
#include <Marco/roles/MToplevel.h>
#include <Marco/MApplication.h>
#include <AK/events/AKRenderEvent.h>
#include <AK/events/AKLayoutEvent.h>

using namespace AK;

MCSDShadow::MCSDShadow(MToplevel *toplevel) noexcept :
    AKRenderable(AKRenderable::Texture, &toplevel->MSurface::imp()->root),
    m_toplevel(toplevel)
{
    SkRegion reg;
    setInputRegion(&reg);
    layout().setPositionType(YGPositionTypeAbsolute);
    layout().setWidthPercent(100);
    layout().setHeightPercent(100);
}

void MCSDShadow::layoutEvent(const AKLayoutEvent &e)
{
    if (!m_toplevel || !e.changes().check(AKLayoutEvent::Changes::Size | AKLayoutEvent::Changes::Scale))
        return;

    if (m_toplevel->activated())
    {
        m_image = app()->theme()->csdShadowActive(
            scale(),
            SkISize(layout().calculatedWidth() -  m_toplevel->builtinDecorationMargins().fLeft - m_toplevel->builtinDecorationMargins().fRight,
                    layout().calculatedHeight() - m_toplevel->builtinDecorationMargins().fTop - m_toplevel->builtinDecorationMargins().fBottom),
            m_clampSides);
    }
    else
    {
        m_image = app()->theme()->csdShadowInactive(
            scale(),
            SkISize(layout().calculatedWidth() -  m_toplevel->builtinDecorationMargins().fLeft - m_toplevel->builtinDecorationMargins().fRight,
                    layout().calculatedHeight() - m_toplevel->builtinDecorationMargins().fTop - m_toplevel->builtinDecorationMargins().fBottom),
            m_clampSides);
    }

    addDamage(AK_IRECT_INF);
}

void MCSDShadow::renderEvent(const AKRenderEvent &p)
{
    if (!m_toplevel || !m_image || p.damage.isEmpty())
        return;

    /* Remove invisible region */

    SkIRect rect = p.rect;
    rect.fRight++;

    const auto &margins { m_toplevel->builtinDecorationMargins() };
    SkIRect centerV = SkIRect(
        margins.fLeft,
        margins.fTop,
        rect.width() - margins.fRight,
        rect.height() - margins.fBottom);

    SkIRect centerH = centerV;
    centerV.inset(MTheme::CSDBorderRadius, 1);
    centerH.inset(1, MTheme::CSDBorderRadius);

    SkRegion maskedDamage = p.damage;
    maskedDamage.op(centerV, SkRegion::Op::kDifference_Op);
    maskedDamage.op(centerH, SkRegion::Op::kDifference_Op);

    if (maskedDamage.isEmpty())
        return;

    const Int32 halfWidth { rect.width()/2 };
    SkRegion finalDamage { maskedDamage };
    AKPainter::TextureParams params;
    params.texture = m_image;
    params.srcScale = scale();

    /* No clamp */
    if (m_clampSides.get() == 0)
    {
        /* Left side */
        params.dstSize = SkISize::Make(halfWidth, rect.height());
        params.srcRect = SkRect::MakeWH(halfWidth, rect.height()),
        params.pos = {0, 0};
        params.srcTransform = AKTransform::Normal;
        finalDamage.op(SkIRect::MakeWH(halfWidth, rect.height()), SkRegion::Op::kIntersect_Op);
        p.painter.bindTextureMode(params);
        p.painter.drawRegion(finalDamage);

        /* Right mirrored side */
        finalDamage = maskedDamage;
        finalDamage.op(SkIRect::MakeXYWH(halfWidth, 0, halfWidth, rect.height()), SkRegion::Op::kIntersect_Op);
        params.pos.fX = halfWidth;
        params.srcTransform = AKTransform::Flipped;
        p.painter.bindTextureMode(params);
        p.painter.drawRegion(finalDamage);
    }
    else
    {
        const Int32 B { (activated() ? MTheme::CSDShadowActiveOffsetY : MTheme::CSDShadowInactiveOffsetY) + 1  };
        const Int32 T { (activated() ?
            (MTheme::CSDShadowActiveRadius - MTheme::CSDShadowActiveOffsetY) :
            (MTheme::CSDShadowInactiveRadius - MTheme::CSDShadowInactiveOffsetY)) + (2 * MTheme::CSDBorderRadius) + 1 };
        const Int32 L { margins.fLeft + 2 };

        /* Top Left */
        params.dstSize = { margins.fLeft + L, margins.fTop  + T };
        finalDamage = maskedDamage;
        finalDamage.op(SkIRect::MakeSize(params.dstSize), SkRegion::Op::kIntersect_Op);
        params.srcRect.setWH(params.dstSize.width(), params.dstSize.height()),
        params.pos = {0, 0};
        params.srcTransform = AKTransform::Normal;
        p.painter.bindTextureMode(params);
        p.painter.drawRegion(finalDamage);

        /* Top Right */
        finalDamage = maskedDamage;
        params.pos = {rect.width() - margins.fLeft - L, 0};
        finalDamage.op(SkIRect::MakeXYWH(params.pos.x(), 0, params.dstSize.width(), params.dstSize.height()), SkRegion::Op::kIntersect_Op);
        params.srcTransform = AKTransform::Flipped;
        p.painter.bindTextureMode(params);
        p.painter.drawRegion(finalDamage);

        /* Top */
        finalDamage = maskedDamage;
        params.pos = {margins.fLeft + L, 0};
        params.dstSize.fWidth = rect.width() - 2 * ( margins.fLeft + L);
        params.srcRect.setXYWH(params.pos.x(), 0, 1, params.dstSize.height()),
        finalDamage.op(SkIRect::MakeXYWH(params.pos.x(), 0, params.dstSize.width(), params.dstSize.height()), SkRegion::Op::kIntersect_Op);
        params.srcTransform = AKTransform::Normal;
        p.painter.bindTextureMode(params);
        p.painter.drawRegion(finalDamage);

        /* Left */
        params.pos = { 0, margins.fTop + T };
        params.dstSize.fWidth = margins.fLeft + L;
        params.dstSize.fHeight = rect.height() - margins.fTop - margins.fBottom - T - B;
        finalDamage = maskedDamage;
        finalDamage.op(SkIRect::MakeXYWH(0, params.pos.y(), params.dstSize.width(), params.dstSize.height()), SkRegion::Op::kIntersect_Op);
        params.srcRect.setXYWH(0, params.pos.y(), params.dstSize.width(), 0.5);
        params.srcTransform = AKTransform::Normal;
        p.painter.bindTextureMode(params);
        p.painter.drawRegion(finalDamage);

        /* Right */
        params.pos.fX = rect.width() - margins.fLeft - L;
        finalDamage = maskedDamage;
        finalDamage.op(SkIRect::MakeXYWH(params.pos.x(), params.pos.y(), params.dstSize.width(), params.dstSize.height()), SkRegion::Op::kIntersect_Op);
        params.srcTransform = AKTransform::Flipped;
        p.painter.bindTextureMode(params);
        p.painter.drawRegion(finalDamage);

        /* Bottom Left */
        const Int32 bottom { m_image->height()/scale() - margins.bottom() - B };
        params.pos = { 0, rect.height() - margins.fBottom - B };
        params.dstSize.fHeight = margins.fBottom + B;
        finalDamage = maskedDamage;
        finalDamage.op(SkIRect::MakeXYWH(0, params.pos.y(), params.dstSize.width(), params.dstSize.height()), SkRegion::Op::kIntersect_Op);
        params.srcRect.setXYWH(0, bottom, params.dstSize.width(), params.dstSize.height());
        params.srcTransform = AKTransform::Normal;
        p.painter.bindTextureMode(params);
        p.painter.drawRegion(finalDamage);

        /* Bottom Right */
        params.pos.fX = rect.width() - margins.fLeft - L;
        finalDamage = maskedDamage;
        finalDamage.op(SkIRect::MakeXYWH(params.pos.x(), params.pos.y(), params.dstSize.width(), params.dstSize.height()), SkRegion::Op::kIntersect_Op);
        params.srcRect.setXYWH(0, bottom, params.dstSize.width(), params.dstSize.height());
        params.srcTransform = AKTransform::Flipped;
        p.painter.bindTextureMode(params);
        p.painter.drawRegion(finalDamage);

        /* Bottom */
        params.pos.fX = margins.fLeft + L;
        params.dstSize.fWidth = rect.width() - (margins.fLeft  + L)*2;
        finalDamage = maskedDamage;
        finalDamage.op(SkIRect::MakeXYWH(params.pos.x(), params.pos.y(), params.dstSize.width(), params.dstSize.height()), SkRegion::Op::kIntersect_Op);
        params.srcRect.setXYWH(params.pos.x(), bottom, params.dstSize.width(), params.dstSize.height());
        params.srcTransform = AKTransform::Normal;
        p.painter.bindTextureMode(params);
        p.painter.drawRegion(finalDamage);
    }
}
