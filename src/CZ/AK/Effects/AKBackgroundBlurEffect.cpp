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
#include <cstdlib>

using namespace CZ;

/*
 * CZ_AK_BLUR_DEBUG: a bitmask to find blur artifacts (e.g. dark frames while a panel slides in).
 * - 1: Always reblur the whole effect (not only the captured damage).
 * - 2: Fill the blur buffers with magenta before reblurring: texels not rewritten that frame show pink.
 * - 4: Log every reblur (effect rect, valid area, damage).
 */
enum BlurDebug : UInt32
{
    BlurDebugFullReblur = 1,
    BlurDebugMagenta    = 2,
    BlurDebugLog        = 4,
};

static UInt32 BlurDebugFlags() noexcept
{
    static const UInt32 flags { [] {
        const char *env { getenv("CZ_AK_BLUR_DEBUG") };
        return env ? UInt32(std::strtoul(env, nullptr, 10)) : 0u;
    }() };
    return flags;
}

static void FillMagenta(RPainter &painter, const SkIRect &rect) noexcept
{
    painter.setBlendMode(RBlendMode::Src);
    painter.setColor(SK_ColorMAGENTA);
    painter.drawColor(SkRegion(rect));
}

AKBackgroundBlurEffect::AKBackgroundBlurEffect(AKNode *target) noexcept :
    AKBackgroundEffect(Behind)
{
    if (target)
        target->addBackgroundEffect(this);

    bdt.setEnabled(true);
    updateTrackerScale();

    // Reblur when the node's color scheme changes (AKNode::setColorScheme emits this, e.g. from a
    // propagated CZColorSchemeEvent). The scheme itself is read from colorScheme() at render time.
    onColorSchemeChanged.subscribe(this, [this](CZColorScheme){ addChange(CHColorScheme); });
}

/*
 * The blur passes use fixed kernels: sigma 6 texels horizontally (reading the captured backdrop at
 * 1/n scale) and sigma 3 texels vertically (reading the 1/2n result), i.e. sigma 6n logical units.
 * n is the integer divisor closest to radius/6 and the remaining factor stretches the tap step, so
 * any radius costs the same per pixel, with fewer pixels as it grows.
 */
static Int32 ScaleDivisor(SkScalar radius) noexcept
{
    return std::clamp(SkScalarRoundToInt(radius / 6.f), 1, 16); // 1/16: the tracker's min scale
}

void AKBackgroundBlurEffect::updateTrackerScale() noexcept
{
    bdt.setScale(1.f / SkScalar(ScaleDivisor(m_blurRadius)));

    // Captured damage spreads up to the kernel reach (the vertical pass reaches ~4.3 sigma, but its
    // outer taps weigh little)
    bdt.setDamageOutset(SkScalarCeilToInt(m_blurRadius * 3.5f));
}

void AKBackgroundBlurEffect::setBlurRadius(SkScalar radius) noexcept
{
    radius = std::clamp(radius, 6.f, 96.f);

    if (radius == m_blurRadius)
        return;

    m_blurRadius = radius;
    updateTrackerScale();
    addChange(CHMaterial);
}

void AKBackgroundBlurEffect::setMaterial(CZColorScheme scheme, const Material &material) noexcept
{
    Material &current { scheme == CZColorScheme::Dark ? m_darkMaterial : m_lightMaterial };

    if (current.saturation == material.saturation && current.tint == material.tint)
        return;

    current = material;
    addChange(CHMaterial);
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

    if (!changes().testAnyOf(CHArea, CHClip, CHColorScheme, CHMaterial) && !changedSize)
        return;

    if (changes().testAnyOf(CHColorScheme, CHMaterial))
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

    const auto schemeChanged { changes().testAnyOf(CHColorScheme, CHMaterial) };
    const UInt32 debug { BlurDebugFlags() };

    // Where the backdrop is sampled: under the effect, or pushed inside the screen while it sticks
    // out (see AKBackgroundDamageTracker::setCaptureInsideTarget())
    const SkIRect src { p.rect.makeOffset(bdt.sampleOffset().x(), bdt.sampleOffset().y()) };

    // The blur buffers hold the backdrop of the last sampled rect: once it moves (e.g. a panel
    // sliding in), all of it is stale, not only the captured damage
    const bool moved { src != m_prevSrc };
    m_prevSrc = src;

    bool reblur { !bdt.capturedDamage.isEmpty() || schemeChanged || moved || (debug & BlurDebugFullReblur) };
    bool copyAll { schemeChanged || moved || (debug & BlurDebugFullReblur) };
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

        // The tap stretch completing the radius the scale divisor doesn't reach (see ScaleDivisor())
        RImageEffectParams params {};
        params.tapStep = m_blurRadius * bdtScale / 6.f;

        RDrawImageInfo info {};
        info.image = bdt.currentSurface()->image();
        info.srcScale = bdtScale;
        info.dst = m_blur->geometry().viewport.roundOut();
        info.src = SkRect::Make(src);

        // Only the part of the effect within the target (screen) is ever captured: e.g. a dock
        // sliding in from below the screen (unless sampled pushed inside, see src above, then all of
        // it is valid). Taps beyond it are mirrored back (see validRect), so the
        // blur continues the screen's content past its edges instead of reading what was never
        // captured (dark or stale).
        const SkRect hDst { SkRect::Make(info.dst) };
        SkRect valid { info.src };

        const SkRect &captured { bdt.currentSurface()->geometry().viewport }; // Target-sized, at 0,0

        if (!valid.intersect(SkRect::MakeWH(captured.width(), captured.height())))
            valid.setXYWH(info.src.centerX(), info.src.centerY(), 1.f, 1.f); // Fully off-screen: nothing valid

        const auto mapRect = [](const SkRect &r, const SkRect &from, const SkRect &to) {
            const SkScalar sx { to.width() / std::max(1.f, from.width()) };
            const SkScalar sy { to.height() / std::max(1.f, from.height()) };
            return SkRect::MakeLTRB(to.x() + (r.left() - from.x()) * sx,   to.y() + (r.top() - from.y()) * sy,
                                    to.x() + (r.right() - from.x()) * sx,  to.y() + (r.bottom() - from.y()) * sy);
        };

        params.validRect = mapRect(valid, info.src, hDst); // In H dst coordinates

        if (debug & BlurDebugLog)
        {
            const SkIRect dmg { bdt.capturedDamage.getBounds() };
            AKLog(CZInfo, "[BlurDebug] {} rect ({},{} {}x{}) src ({},{}) captured {}x{} valid ({},{} {}x{}) copyAll {} damage ({},{} {}x{}) rects {}",
                  (const void*)this, p.rect.x(), p.rect.y(), p.rect.width(), p.rect.height(), src.x(), src.y(),
                  captured.width(), captured.height(), valid.x(), valid.y(), valid.width(), valid.height(),
                  copyAll, dmg.x(), dmg.y(), dmg.width(), dmg.height(), bdt.capturedDamage.computeRegionComplexity());
        }

        if (debug & BlurDebugMagenta)
            FillMagenta(*painter, info.dst);

        if (copyAll)
            painter->drawImageEffect(info, RPainter::VibrancyH, nullptr, params);
        else
        {
            bdt.capturedDamage.translate(-src.x(), -src.y()); // Relative to the sampled rect
            bdt.capturedDamage.op(info.dst, SkRegion::Op::kIntersect_Op);
            painter->drawImageEffect(info, RPainter::VibrancyH, &bdt.capturedDamage, params);
        }

        // V Pass: x0.5 => x0.25 blur, then the material's saturation and tint (one color matrix)
        const Material &mat { material(colorScheme()) };
        params.colorMatrix = RImageEffectParams::MakeVibrancyMatrix(mat.saturation, mat.tint);
        pass = m_blur2->beginPass(RPassCap_Painter);
        painter = pass->getPainter();

        info = {};
        info.image = m_blur->image();
        info.src = m_blur->geometry().dst;
        info.srcScale = 1.f;
        info.dst = m_blur2->geometry().viewport.roundOut();

        // The same valid area, in V dst coordinates: the H pass never writes the rest (it may hold
        // anything from previous frames)
        params.validRect = mapRect(*params.validRect, hDst, SkRect::Make(info.dst));

        if (debug & BlurDebugMagenta)
            FillMagenta(*painter, info.dst);

        if (copyAll)
            painter->drawImageEffect(info, RPainter::VibrancyV, nullptr, params);
        else
        {
            // One output texel beyond the damage too, even past the screen edges (the captured
            // damage never is): the final upscale blends each on-screen pixel with its neighbor
            // texels, which would otherwise keep stale content (a dark band along the screen edge
            // while e.g. the dock slides in). Their mirrored taps make them valid.
            const Int32 texel { SkScalarCeilToInt(1.f / blur2Scale) };
            SkRegion vDamage;

            for (SkRegion::Iterator it(bdt.capturedDamage); !it.done(); it.next())
                vDamage.op(it.rect().makeOutset(texel, texel), SkRegion::kUnion_Op);

            vDamage.op(info.dst, SkRegion::kIntersect_Op);
            painter->drawImageEffect(info, RPainter::VibrancyV, &vDamage, params);
        }

        pass.reset();
        onBlurred.notify();
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

        // Src (faster) only while fully opaque: faded, it must blend with what's below.
        auto *painter { p.pass->getPainter() };
        painter->setBlendMode(opacity() < 1.f ? RBlendMode::SrcOver : RBlendMode::Src);
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

        // Draw the non-corner stuff (Src only while fully opaque, see above)
        painter->setBlendMode(opacity() < 1.f ? RBlendMode::SrcOver : RBlendMode::Src);
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
        c.clipPath(m_pathClip, true); // Antialiased edges (e.g. a tooltip's rounded shape and caret)
        c.translate(
            pathLocalRect.x() - p.rect.x(),
            pathLocalRect.y() - p.rect.y());
        c.clipPath(damagePath, true);

        // Src (faster) only while fully opaque: faded, it must blend with what's below (which the
        // scene then paints too, see AKScene)
        SkPaint paint;
        paint.setBlendMode(opacity() < 1.f ? SkBlendMode::kSrcOver : SkBlendMode::kSrc);
        paint.setAlphaf(std::clamp(opacity(), 0.f, 1.f));
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

bool AKBackgroundBlurEffect::drawBlurred(SkCanvas &canvas) const noexcept
{
    if (!m_blur2 || effectRect.isEmpty() || m_finalRegion.isEmpty())
        return false;

    auto image { m_blur2->image() ? m_blur2->image()->skImage() : nullptr };

    if (!image)
        return false;

    canvas.save();

    // The area (local coordinates)...
    SkPath area;
    m_finalRegion.getBoundaryPath(&area);
    canvas.clipPath(area);

    // ...and the shape, antialiased
    if (clipType() == RoundRect)
    {
        const SkVector radii[4] {
            SkVector::Make(m_rRectClip.fRadTL, m_rRectClip.fRadTL),
            SkVector::Make(m_rRectClip.fRadTR, m_rRectClip.fRadTR),
            SkVector::Make(m_rRectClip.fRadBR, m_rRectClip.fRadBR),
            SkVector::Make(m_rRectClip.fRadBL, m_rRectClip.fRadBL) };
        SkRRect rRect;
        rRect.setRectRadii(SkRect::Make(static_cast<const SkIRect&>(m_rRectClip)), radii);
        canvas.clipRRect(rRect, true);
    }
    else if (clipType() == Path)
        canvas.clipPath(m_pathClip, true);

    SkPaint paint;
    paint.setAntiAlias(true);
    canvas.drawImageRect(image, m_blur2->geometry().dst, SkRect::Make(effectRect),
                         SkFilterMode::kLinear, &paint, SkCanvas::kFast_SrcRectConstraint);
    canvas.restore();
    return true;
}
