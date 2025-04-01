#include <Marco/MTheme.h>
#include <AK/AKTarget.h>
#include <AK/AKSurface.h>

#include <include/gpu/ganesh/GrDirectContext.h>
#include <include/effects/SkGradientShader.h>
#include <include/effects/SkImageFilters.h>
#include <include/core/SkCanvas.h>

using namespace AK;

MTheme::MTheme() noexcept {}

sk_sp<SkImage> MTheme::csdBorderRadiusMask(Int32 scale) noexcept
{
    auto it = m_csdBorderRadiusMask.find(scale);

    if (it != m_csdBorderRadiusMask.end())
        return it->second;

    auto surface = AKSurface::Make(
        SkISize::Make(CSDBorderRadius, CSDBorderRadius),
        scale);

    surface->surface()->recordingContext()->asDirectContext()->resetContext();
    SkCanvas &c { *surface->surface()->getCanvas() };
    c.scale(surface->scale(), surface->scale());
    c.clear(SK_ColorTRANSPARENT);

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorBLACK);
    c.drawCircle(SkPoint::Make(CSDBorderRadius, CSDBorderRadius), CSDBorderRadius, paint);
    surface->surface()->recordingContext()->asDirectContext()->flush();

    sk_sp<SkImage> result { surface->releaseImage() };
    m_csdBorderRadiusMask[scale] = result;
    return result;
}

sk_sp<SkImage> MTheme::csdShadowActive(Int32 scale, const SkISize &winSize, AK::AKBitset<ShadowClamp> &sides) noexcept
{
    if (winSize.isEmpty())
    {
        sides.set(0);
        return sk_sp<SkImage>();
    }

    const Int32 shadowRadius { CSDShadowActiveRadius };
    const Int32 shadowOffsetY { CSDShadowActiveOffsetY };
    const SkIRect shadowMargins {
        shadowRadius,
        shadowRadius - shadowOffsetY,
        shadowRadius,
        shadowRadius + shadowOffsetY
    };

    SkISize windowSize { winSize };

    // TODO:
    const SkISize minClampSize { shadowMargins.left() * 2 + 2, shadowMargins.bottom() * 2 };

    if (windowSize.width() < minClampSize.width() || windowSize.height() < minClampSize.height())
        sides.set(0);
    else
    {
        sides.set(ShadowClampX | ShadowClampY);

        auto it = m_csdShadowActive.find(scale);

        if (it != m_csdShadowActive.end())
            return it->second;

        windowSize = { minClampSize.width() + 1,  minClampSize.height() + 1 };
    }

    const SkISize surfaceSize(
        windowSize.width() * 0.5f + shadowMargins.left(),
        windowSize.height() + shadowMargins.top() + shadowMargins.bottom());
    auto surface = AKSurface::Make(surfaceSize, scale);
    surface->surface()->recordingContext()->asDirectContext()->resetContext();
    SkCanvas *c = surface->surface()->getCanvas();
    c->clear(SK_ColorTRANSPARENT);
    c->scale(surface->scale(), surface->scale());

    /* Shadow */
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setBlendMode(SkBlendMode::kSrc);
    paint.setImageFilter(SkImageFilters::DropShadowOnly(0, shadowOffsetY, Float32(shadowRadius)/3.f, Float32(shadowRadius)/3.f, 0x69000000, nullptr));
    SkRect rrect = SkRect::MakeXYWH(shadowMargins.left(), shadowMargins.top(), windowSize.width(), windowSize.height());
    c->drawRoundRect(rrect, CSDBorderRadius, CSDBorderRadius, paint);

    /* Black border */
    paint.setImageFilter(nullptr);
    paint.setAntiAlias(true);
    paint.setStrokeWidth(1.f);
    paint.setColor(0x33000000);
    paint.setStroke(true);
    paint.setBlendMode(SkBlendMode::kSrcOver);
    c->drawRoundRect(rrect, CSDBorderRadius, CSDBorderRadius, paint);

    /* Clear center */
    paint.setStroke(false);
    paint.setBlendMode(SkBlendMode::kClear);
    c->drawRoundRect(rrect, CSDBorderRadius, CSDBorderRadius, paint);

    /* White border */
    paint.setStrokeWidth(0.25f);
    paint.setColor(0xFFFFFFFF);
    paint.setStroke(true);
    paint.setBlendMode(SkBlendMode::kSrcOver);

    SkPoint gPoints[2] { SkPoint(0, rrect.fTop), SkPoint(0, rrect.fTop + CSDBorderRadius * 0.5f)};
    SkColor gColors[2] { 0xFAFFFFFF, 0x00FFFFFF };
    SkScalar gPos[2] { 0.f, 1.f };
    paint.setShader(SkGradientShader::MakeLinear(gPoints, gColors, gPos, 2, SkTileMode::kClamp));

    c->save();
    c->clipRect(SkRect(rrect.fLeft, rrect.fTop - 1, rrect.fRight, rrect.fTop + CSDBorderRadius));
    rrect.inset(0.5f, 0.5f);
    c->drawRoundRect(rrect, CSDBorderRadius, CSDBorderRadius, paint);
    c->restore();
    surface->surface()->recordingContext()->asDirectContext()->flush();

    if (sides.get() == 0)
        return sk_sp<SkImage>(surface->releaseImage());
    else
    {
        sk_sp<SkImage> result { surface->releaseImage() };
        m_csdShadowActive[scale] = result;
        return result;
    }
}

sk_sp<SkImage> MTheme::csdShadowInactive(Int32 scale, const SkISize &winSize, AK::AKBitset<ShadowClamp> &sides) noexcept
{
    if (winSize.isEmpty())
    {
        sides.set(0);
        return sk_sp<SkImage>();
    }

    const Int32 shadowRadius { CSDShadowInactiveRadius };
    const Int32 shadowOffsetY { CSDShadowInactiveOffsetY };
    const SkIRect shadowMargins {
        shadowRadius,
        shadowRadius - shadowOffsetY,
        shadowRadius,
        shadowRadius + shadowOffsetY
    };

    SkISize windowSize { winSize };

    // TODO:
    const SkISize minClampSize { shadowMargins.left() * 2 + 2, shadowMargins.bottom() * 2 };

    if (windowSize.width() < minClampSize.width() || windowSize.height() < minClampSize.height())
        sides.set(0);
    else
    {
        sides.set(ShadowClampX | ShadowClampY);

        auto it = m_csdShadowInactive.find(scale);

        if (it != m_csdShadowInactive.end())
            return it->second;

        windowSize = { minClampSize.width() + 1,  minClampSize.height() + 1 };
    }

    const SkISize surfaceSize(
        windowSize.width() * 0.5f + shadowMargins.left(),
        windowSize.height() + shadowMargins.top() + shadowMargins.bottom());
    auto surface = AKSurface::Make(surfaceSize, scale);
    surface->surface()->recordingContext()->asDirectContext()->resetContext();
    SkCanvas *c = surface->surface()->getCanvas();
    c->clear(SK_ColorTRANSPARENT);
    c->scale(surface->scale(), surface->scale());

    /* Shadow */
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setBlendMode(SkBlendMode::kSrc);
    paint.setImageFilter(SkImageFilters::DropShadowOnly(0, shadowOffsetY, Float32(shadowRadius)/3.f, Float32(shadowRadius)/3.f, 0x33000000, nullptr));
    SkRect rrect = SkRect::MakeXYWH(shadowMargins.left(), shadowMargins.top(), windowSize.width(), windowSize.height());
    c->drawRoundRect(rrect, CSDBorderRadius, CSDBorderRadius, paint);

    /* Black border */
    paint.setImageFilter(nullptr);
    paint.setAntiAlias(true);
    paint.setStrokeWidth(1.f);
    paint.setColor(0x22000000);
    paint.setStroke(true);
    paint.setBlendMode(SkBlendMode::kSrcOver);
    c->drawRoundRect(rrect, CSDBorderRadius, CSDBorderRadius, paint);

    /* Clear center */
    paint.setStroke(false);
    paint.setBlendMode(SkBlendMode::kClear);
    c->drawRoundRect(rrect, CSDBorderRadius, CSDBorderRadius, paint);

    /* White border */
    paint.setStrokeWidth(0.25f);
    paint.setColor(0xFFFFFFFF);
    paint.setStroke(true);
    paint.setBlendMode(SkBlendMode::kSrcOver);

    SkPoint gPoints[2] { SkPoint(0, rrect.fTop), SkPoint(0, rrect.fTop + CSDBorderRadius * 0.5f)};
    SkColor gColors[2] { 0xFAFFFFFF, 0x00FFFFFF };
    SkScalar gPos[2] { 0.f, 1.f };
    paint.setShader(SkGradientShader::MakeLinear(gPoints, gColors, gPos, 2, SkTileMode::kClamp));

    c->save();
    c->clipRect(SkRect(rrect.fLeft, rrect.fTop - 1, rrect.fRight, rrect.fTop + CSDBorderRadius));
    rrect.inset(0.5f, 0.5f);
    c->drawRoundRect(rrect, CSDBorderRadius, CSDBorderRadius, paint);
    c->restore();
    surface->surface()->recordingContext()->asDirectContext()->flush();

    if (sides.get() == 0)
        return sk_sp<SkImage>(surface->releaseImage());
    else
    {
        sk_sp<SkImage> result { surface->releaseImage() };
        m_csdShadowInactive[scale] = result;
        return result;
    }
}
