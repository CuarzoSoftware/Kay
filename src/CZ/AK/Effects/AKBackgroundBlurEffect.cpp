#include <CZ/skia/core/SkCanvas.h>
#include <CZ/skia/effects/SkImageFilters.h>
#include <CZ/skia/gpu/ganesh/GrDirectContext.h>
#include <CZ/skia/core/SkRRect.h>
#include <CZ/AK/Effects/AKBackgroundBlurEffect.h>
#include <CZ/AK/Events/AKRenderEvent.h>
#include <CZ/AK/AKApp.h>
#include <CZ/AK/AKTarget.h>
#include <CZ/AK/AKTheme.h>
#include <CZ/AK/AKLog.h>
#include <CZ/Ream/RImage.h>
#include <CZ/Ream/RPass.h>

using namespace CZ;

AKBackgroundBlurEffect::AKBackgroundBlurEffect(AKNode *target) noexcept :
    AKBackgroundEffect(Behind)
{
    if (target)
        target->addBackgroundEffect(this);

    bdt.setEnabled(true);
    bdt.setScale(0.5f);
    bdt.setDamageOutset(42);
}

bool AKBackgroundBlurEffect::setColorScheme(CZColorScheme scheme) noexcept
{
    const auto changed { m_colorScheme != scheme };
    bool needsRepaint { false };

    if (scheme == CZColorScheme::Dark)
        needsRepaint = m_colorScheme != CZColorScheme::Dark;
    else
        needsRepaint = m_colorScheme == CZColorScheme::Dark;

    if (needsRepaint)
        addChange(CHColorScheme);

    return changed;
}

void AKBackgroundBlurEffect::setFullSize() noexcept
{
    if (m_areaType == FullSize)
        return;

    m_areaType = FullSize;
    addChange(CHArea);
}

void AKBackgroundBlurEffect::setRegion(const SkRegion &region) noexcept
{
    m_areaType = Region;
    m_userRegion = region;
    addChange(CHArea);
}

void AKBackgroundBlurEffect::clearClip() noexcept
{
    if (m_clipType == NoClip)
        return;

    m_clipType = NoClip;
    addChange(CHClip);
}

void AKBackgroundBlurEffect::setRoundRectClip(const CZRRect &rRect) noexcept
{
    if (m_clipType == RoundRect && rRect == m_rRectClip)
        return;

    m_clipType = RoundRect;
    m_rRectClip = rRect;
    addChange(CHClip);
}

void AKBackgroundBlurEffect::setPathClip(const SkPath &path) noexcept
{
    m_clipType = Path;
    m_pathClip = path;
    addChange(CHClip);
}

void AKBackgroundBlurEffect::targetNodeRectCalculated()
{
    onTargetLayoutUpdated.notify();

    if (areaType() == FullSize)
        m_finalRegion.setRect(SkIRect::MakeSize(targetNode()->worldRect().size()));
    else // Region
        m_finalRegion = m_userRegion;

    const auto bounds { m_finalRegion.getBounds() };
    const bool changedSize { effectRect.size() != bounds.size() };

    if (!changes().testAnyOf(CHArea, CHClip, CHColorScheme) && !changedSize)
        return;

    if (changes().test(CHColorScheme))
        addDamage(AK_IRECT_INF);

    if (clipType() == NoClip)
    {
        effectRect = m_finalRegion.getBounds();
        bdt.setCaptureRect(SkIRect::MakeSize(effectRect.size()));

        SkRegion inverse;
        m_finalRegion.translate(-effectRect.x(), -effectRect.y(), &inverse);
        inverse.op(SkIRect::MakeSize(effectRect.size()), SkRegion::kReverseDifference_Op);

        SkRegion paintAnyway;
        SkRegion::Iterator it(inverse);

        while (!it.done())
        {
            paintAnyway.op(it.rect().makeOutset(5, 5), SkRegion::Op::kUnion_Op);
            it.next();
        }

        bdt.setPaintAnyway(paintAnyway);
    }
    else if (clipType() == RoundRect)
    {
        m_finalRegion.op(roundRectClip(), SkRegion::kIntersect_Op);
        effectRect = m_finalRegion.getBounds();
        bdt.setCaptureRect(SkIRect::MakeSize(effectRect.size()));

        SkRegion inverse { m_finalRegion };
        inverse.op(effectRect, SkRegion::kReverseDifference_Op);

        SkRegion corners;
        if (roundRectClip().fRadTL > 0)
            corners.op(SkIRect::MakeXYWH(
                roundRectClip().x(),
                roundRectClip().y(),
                roundRectClip().fRadTL,
                roundRectClip().fRadTL),
                SkRegion::Op::kUnion_Op);

        if (roundRectClip().fRadTR > 0)
            corners.op(SkIRect::MakeXYWH(
                roundRectClip().fRight - roundRectClip().fRadTR,
                roundRectClip().y(),
                roundRectClip().fRadTR,
                roundRectClip().fRadTR),
                SkRegion::Op::kUnion_Op);

        if (roundRectClip().fRadBR > 0)
            corners.op(SkIRect::MakeXYWH(
                roundRectClip().fRight - roundRectClip().fRadBR,
                roundRectClip().fBottom - roundRectClip().fRadBR,
                roundRectClip().fRadBR,
                roundRectClip().fRadBR),
                SkRegion::Op::kUnion_Op);

        if (roundRectClip().fRadBL > 0)
            corners.op(SkIRect::MakeXYWH(
                roundRectClip().x(),
                roundRectClip().fBottom - roundRectClip().fRadBL,
                roundRectClip().fRadBL,
                roundRectClip().fRadBL),
                SkRegion::Op::kUnion_Op);

        corners.op(m_finalRegion, SkRegion::kIntersect_Op);
        inverse.op(corners, SkRegion::kUnion_Op);
        inverse.translate(-effectRect.x(), -effectRect.y());

        SkRegion paintAnyway;
        SkRegion::Iterator it(inverse);

        while (!it.done())
        {
            paintAnyway.op(it.rect().makeOutset(5, 5), SkRegion::Op::kUnion_Op);
            it.next();
        }
        bdt.setPaintAnyway(paintAnyway);
    }
    else // Path
    {       
        m_finalRegion.op(pathClip().getBounds().round(), SkRegion::kIntersect_Op);
        effectRect = m_finalRegion.getBounds();
        bdt.setCaptureRect(SkIRect::MakeSize(effectRect.size()));

        // TODO: Calculate a more compact region

        SkRegion inverse;
        m_finalRegion.translate(-effectRect.x(), -effectRect.y(), &inverse);

        SkRegion paintAnyway;
        SkRegion::Iterator it(inverse);

        while (!it.done())
        {
            paintAnyway.op(it.rect().makeOutset(5, 5), SkRegion::Op::kUnion_Op);
            it.next();
        }

        bdt.setPaintAnyway(paintAnyway);
    }
}

void AKBackgroundBlurEffect::renderEvent(const AKRenderEvent &p)
{
    if (!bdt.currentSurface() || p.damage.isEmpty() || p.rect.isEmpty())
        return;

    const auto schemeChanged { changes().test(CHColorScheme) };
    bool reblur { !bdt.capturedDamage.isEmpty() || schemeChanged };
    bool copyAll { schemeChanged };
    SkScalar bdtScale { bdt.scale() };
    SkScalar scale { bdtScale * 0.5f};
    SkScalar blur2Scale { scale * 0.5f };
    SkISize copySize = p.rect.size();
    const int m { SkScalarRoundToInt(1.f / (scale * 0.5f)) };
    const int modW { copySize.fWidth % m };
    const int modH { copySize.fHeight % m };

    if (modW != 0)
        copySize.fWidth -= modW;
    if (modH != 0)
        copySize.fHeight -= modH;

    if (m_blur)
    {
        copyAll |= m_blur->resize(copySize, scale, true);
        reblur |= copyAll;
    }
    else
    {
        copyAll = reblur = true;
        m_blur = RSurface::Make(copySize, scale, false);
    }

    if (m_blur2)
        reblur |= m_blur2->resize(copySize, blur2Scale, true);
    else
        m_blur2 = RSurface::Make(copySize, blur2Scale, false);

    if (reblur)
    {
        // H Pass: bdt => x0.5 blur destination (no saturation)
        auto pass { m_blur->beginPass(RPassCap_Painter) };
        auto *painter { pass->getPainter() };

        RDrawImageInfo info {};
        info.image = bdt.currentSurface()->image();
        info.srcScale = bdtScale;
        info.dst = m_blur->geometry().viewport.roundOut();
        info.src = SkRect::Make(p.rect);

        if (copyAll)
            painter->drawImageEffect(info, RPainter::VibrancyH);
        else
        {
            bdt.capturedDamage.translate(-p.rect.x(), -p.rect.y());
            bdt.capturedDamage.op(info.dst, SkRegion::Op::kIntersect_Op);
            painter->drawImageEffect(info, RPainter::VibrancyH, &bdt.capturedDamage);
        }

        // V Pass: 0.5 blur => 0.25 blur + saturation
        auto fx { colorScheme() == CZColorScheme::Dark ? RPainter::VibrancyDarkV : RPainter::VibrancyLightV };
        pass = m_blur2->beginPass(RPassCap_Painter);
        painter = pass->getPainter();

        info = {};
        info.image = m_blur->image();
        info.src = m_blur->geometry().dst;
        info.srcScale = 1.f;
        info.dst = m_blur2->geometry().viewport.roundOut();

        if (copyAll)
            painter->drawImageEffect(info, fx);
        else
            painter->drawImageEffect(info, fx, &bdt.capturedDamage);
    }

    if (clipType() == NoClip)
    {
        SkRegion damage;
        m_finalRegion.translate(p.rect.x() - effectRect.x(), p.rect.y() - effectRect.y(), &damage);
        damage.op(p.damage, SkRegion::Op::kIntersect_Op);

        RDrawImageInfo info {};
        info.image = m_blur2->image();
        info.src = m_blur2->geometry().dst;
        info.dst = p.rect;

        auto *painter { p.pass->getPainter() };
        painter->setBlendMode(RBlendMode::Src);
        painter->drawImage(info, &damage);
    }
    else if (clipType() == RoundRect)
    {
        SkRegion damage;
        m_finalRegion.translate(p.rect.x() - effectRect.x(), p.rect.y() - effectRect.y(), &damage);
        damage.op(p.damage, SkRegion::Op::kIntersect_Op);

        SkRegion cornerDamage;
        constexpr CZTransform maskTransforms[4] {
            CZTransform::Normal,
            CZTransform::Rotated90,
            CZTransform::Rotated180,
            CZTransform::Rotated270
        };
        const SkIRect cornerDstRects[4] {
            SkIRect::MakeXYWH(
                roundRectClip().x() + targetNode()->sceneRect().x(),
                roundRectClip().y() + targetNode()->sceneRect().y(),
                roundRectClip().fRadTL,
                roundRectClip().fRadTL),
            SkIRect::MakeXYWH(
                roundRectClip().fRight - roundRectClip().fRadTR + targetNode()->sceneRect().x(),
                roundRectClip().y() + targetNode()->sceneRect().y(),
                roundRectClip().fRadTR,
                roundRectClip().fRadTR),
            SkIRect::MakeXYWH(
                roundRectClip().fRight - roundRectClip().fRadBR + targetNode()->sceneRect().x(),
                roundRectClip().fBottom - roundRectClip().fRadBR + targetNode()->sceneRect().y(),
                roundRectClip().fRadBR,
                roundRectClip().fRadBR),
            SkIRect::MakeXYWH(
                roundRectClip().x() + targetNode()->sceneRect().x(),
                roundRectClip().fBottom - roundRectClip().fRadBL + targetNode()->sceneRect().y(),
                roundRectClip().fRadBL,
                roundRectClip().fRadBL)
        };

        RDrawImageInfo imageInfo {};
        imageInfo.image = m_blur2->image();
        imageInfo.srcScale = 1.f;
        imageInfo.src = m_blur2->geometry().dst;
        imageInfo.dst = p.rect;

        RDrawImageInfo maskInfo {};

        auto *painter { p.pass->getPainter() };
        painter->setBlendMode(RBlendMode::SrcOver);

        // Draw round corners
        for (size_t i = 0; i < 4; i++)
        {
            if (cornerDstRects[i].isEmpty())
                continue;

            cornerDamage.op(cornerDstRects[i], damage, SkRegion::kIntersect_Op);

            if (cornerDamage.isEmpty())
                continue;

            m_firstQuarterCircleMasks[i] = theme()->FirstQuadrantCircleMask.get(cornerDstRects[i].width(), targetNode()->scale());
            maskInfo.image = m_firstQuarterCircleMasks[i];
            maskInfo.dst = cornerDstRects[i];
            maskInfo.src = SkRect::Make(maskInfo.image->size());
            maskInfo.srcTransform = maskTransforms[i];

            painter->drawImage(imageInfo, &cornerDamage, &maskInfo);

            damage.op(cornerDstRects[i], SkRegion::kDifference_Op);
        }

        // Draw the non-corner stuff
        painter->setBlendMode(RBlendMode::Src);
        painter->drawImage(imageInfo, &damage);
    }
    else if (clipType() == Path)
    {
        constexpr int mod { 2 };
        SkIRect pathLocalRect;
        pathLocalRect = m_pathClip.getBounds().round();
        pathLocalRect.setXYWH(
            pathLocalRect.x(),
            pathLocalRect.y(),
            pathLocalRect.width() + mod - (pathLocalRect.width() % mod),
            pathLocalRect.height() + mod - (pathLocalRect.height() % mod));

        auto &c { *p.pass->getCanvas() };
        c.save();

        SkRegion damage;
        m_finalRegion.translate(p.rect.x() - effectRect.x(), p.rect.y() - effectRect.y(), &damage);
        damage.op(p.damage, SkRegion::Op::kIntersect_Op);

        SkPath damagePath;
        damage.getBoundaryPath(&damagePath);
        damagePath.setIsVolatile(true);
        c.translate(
            p.rect.x() - pathLocalRect.x(),
            p.rect.y() - pathLocalRect.y());
        c.clipPath(m_pathClip, false);
        c.translate(
            pathLocalRect.x() - p.rect.x(),
            pathLocalRect.y() - p.rect.y());
        c.clipPath(damagePath, true);

        SkPaint paint;
        paint.setBlendMode(SkBlendMode::kSrc);
        paint.setAntiAlias(true);
        c.drawImageRect(m_blur2->image()->skImage(),
                        m_blur2->geometry().dst,
                        SkRect::Make(p.rect),
                        SkFilterMode::kLinear,
                        &paint,
                        SkCanvas::kFast_SrcRectConstraint);
        c.restore();
    }
}
