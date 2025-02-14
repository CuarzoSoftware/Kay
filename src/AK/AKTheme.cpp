#include <include/gpu/GrDirectContext.h>
#include <include/core/SkBlurTypes.h>
#include <include/core/SkMaskFilter.h>
#include <include/core/SkCanvas.h>
#include <include/core/SkPaint.h>
#include <include/effects/SkGradientShader.h>
#include <AK/AKTheme.h>
#include <AK/AKTarget.h>
#include <AK/AKSurface.h>

#include <include/core/SkOpenTypeSVGDecoder.h>
AK::AKTheme::AKTheme() noexcept
{
    ButtonFont.setTypeface(
        SkTypeface::MakeFromName("Inter",
        SkFontStyle(
            SkFontStyle::kSemiBold_Weight,
            SkFontStyle::Width::kNormal_Width,
            SkFontStyle::Slant::kUpright_Slant)));
    ButtonFont.setSize(12);
}

SkRegion AK::AKTheme::buttonPlainOpaqueRegion(Int32 width) noexcept
{
    SkRegion region;
    region.op(SkIRect(3, 7, width - 3, 24 - 7), SkRegion::kUnion_Op);
    region.op(SkIRect(7, 3, width - 7, 24 - 3), SkRegion::kUnion_Op);
    return region;
}

SkRegion AK::AKTheme::buttonTintedOpaqueRegion(Int32 width) noexcept
{
    SkRegion region;
    region.op(SkIRect(3, 7, width - 3, 24 - 7), SkRegion::kUnion_Op);
    region.op(SkIRect(7, 3, width - 7, 24 - 3), SkRegion::kUnion_Op);
    return region;
}

sk_sp<SkImage> AK::AKTheme::buttonPlainHThreePatchImage(Int32 scale) noexcept
{
    const auto it { m_buttonPlainHThreePatchImage.find(scale) };

    if (it != m_buttonPlainHThreePatchImage.end())
        return it->second;

    auto surface = AKSurface::Make(
        SkSize(ButtonPlainHThreePatchSideSrcRect.width() + ButtonPlainHThreePatchCenterSrcRect.width(),
               ButtonPlainHThreePatchSideSrcRect.height()),
        scale, true);

    surface->surface()->recordingContext()->asDirectContext()->resetContext();
    SkCanvas &c { *surface->surface()->getCanvas() };
    c.scale(surface->scale(), surface->scale());
    c.clear(SK_ColorTRANSPARENT);

    SkPaint paint;
    const float borderRadius { 5.f };
    SkRect roundRect { SkRect::MakeWH(surface->size().width() * 2, surface->size().height()) };

    // Shadow
    paint.setAntiAlias(true);
    roundRect.inset(2.5f, 2.5f);
    roundRect.offset(0.f, 0.5f);
    paint.setBlendMode(SkBlendMode::kSrc);
    paint.setColor(SkColorSetARGB(82, 0, 0, 0));
    paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 1.f));
    c.drawRoundRect(roundRect, borderRadius, borderRadius, paint);
    paint.setMaskFilter(nullptr);
    roundRect.offset(0.f, -0.5f);

    // Border
    paint.setBlendMode(SkBlendMode::kSrcOver);
    paint.setStroke(true);
    paint.setStrokeWidth(1.f);
    paint.setColor(SkColorSetARGB(9, 0, 0, 0));
    c.drawRoundRect(roundRect, borderRadius, borderRadius, paint);

    // Fill
    paint.setBlendMode(SkBlendMode::kSrc);
    paint.setStroke(false);
    paint.setColor(SK_ColorWHITE);
    c.drawRoundRect(roundRect, borderRadius, borderRadius, paint);
    surface->surface()->flush();

    sk_sp<SkImage> result { surface->releaseImage() };
    m_buttonPlainHThreePatchImage[scale] = result;
    return result;
}

sk_sp<SkImage> AK::AKTheme::buttonTintedHThreePatchImage(Int32 scale) noexcept
{
    const auto it { m_buttonTintedHThreePatchImage.find(scale) };

    if (it != m_buttonTintedHThreePatchImage.end())
        return it->second;

    auto surface = AKSurface::Make(
        SkSize(ButtonTintedHThreePatchSideSrcRect.width() + ButtonTintedHThreePatchCenterSrcRect.width(),
               ButtonTintedHThreePatchSideSrcRect.height()),
                scale, true);

    surface->surface()->recordingContext()->asDirectContext()->resetContext();
    SkCanvas &c { *surface->surface()->getCanvas() };
    c.scale(surface->scale(), surface->scale());
    c.clear(SK_ColorTRANSPARENT);

    SkPaint paint;
    const float borderRadius { 5.f };
    SkRect roundRect { SkRect::MakeWH(surface->size().width() * 2, surface->size().height()) };

    const SkPoint gradPoints[] { SkPoint(0.f, 2.f), SkPoint(0.f, surface->size().height() - 4.f) };
    const SkColor4f gradColors[] { SkColor4f::FromColor(0xFFFFFFFF), SkColor4f::FromColor(SkColorSetARGB(255, 239, 239, 239)) };
    const SkScalar gradPos[] { 0.f, 1.f };

    // Shadow
    paint.setShader(SkGradientShader::MakeLinear(gradPoints, gradColors, SkColorSpace::MakeSRGB(), gradPos, 2, SkTileMode::kClamp));
    paint.setAntiAlias(true);
    roundRect.inset(2.5f, 2.5f);
    roundRect.offset(0.f, 0.5f);
    paint.setBlendMode(SkBlendMode::kSrc);
    paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 1.f));
    c.drawRoundRect(roundRect, borderRadius, borderRadius, paint);
    paint.setMaskFilter(nullptr);
    roundRect.offset(0.f, -0.5f);

    // Stroke
    paint.setBlendMode(SkBlendMode::kSrcOver);
    paint.setStroke(true);
    paint.setStrokeWidth(1.f);
    c.drawRoundRect(roundRect, borderRadius, borderRadius, paint);

    // Fill
    paint.setBlendMode(SkBlendMode::kSrc);
    paint.setStroke(false);
    c.drawRoundRect(roundRect, borderRadius, borderRadius, paint);
    surface->surface()->flush();

    sk_sp<SkImage> result { surface->releaseImage() };
    m_buttonTintedHThreePatchImage[scale] = result;
    return result;
}

sk_sp<SkImage> AK::AKTheme::textFieldRoundHThreePatchImage(Int32 scale) noexcept
{
    const auto it { m_textFieldRoundHThreePatchImage.find(scale) };

    if (it != m_textFieldRoundHThreePatchImage.end())
        return it->second;

    auto surface = AKSurface::Make(SkSize(TextFieldRoundHThreePatchSideSrcRect.width() + TextFieldRoundHThreePatchCenterSrcRect.width(),
                                          TextFieldRoundHThreePatchSideSrcRect.height()),
                                   scale, true);

    surface->surface()->recordingContext()->asDirectContext()->resetContext();
    SkCanvas &c { *surface->surface()->getCanvas() };
    c.scale(surface->scale(), surface->scale());
    c.clear(SK_ColorTRANSPARENT);

    SkPaint paint;
    const float borderRadius { 5.f };
    SkRect roundRect { SkRect::MakeWH(surface->size().width() * 2, surface->size().height()) };

    // Shadow
    paint.setAntiAlias(true);
    roundRect.inset(2.5f, 2.5f);
    roundRect.offset(0.f, 0.5f);
    paint.setBlendMode(SkBlendMode::kSrc);
    paint.setColor(SkColorSetARGB(82, 0, 0, 0));
    paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 1.f));
    c.drawRoundRect(roundRect, borderRadius, borderRadius, paint);
    paint.setMaskFilter(nullptr);
    roundRect.offset(0.f, -0.5f);

    // Border
    paint.setBlendMode(SkBlendMode::kSrcOver);
    paint.setStroke(true);
    paint.setStrokeWidth(1.f);
    paint.setColor(SkColorSetARGB(9, 0, 0, 0));
    c.drawRoundRect(roundRect, borderRadius, borderRadius, paint);

    // Fill
    paint.setBlendMode(SkBlendMode::kSrc);
    paint.setStroke(false);
    paint.setColor(SK_ColorWHITE);
    c.drawRoundRect(roundRect, borderRadius, borderRadius, paint);
    surface->surface()->flush();

    sk_sp<SkImage> result { surface->releaseImage() };
    m_textFieldRoundHThreePatchImage[scale] = result;
    return result;
}

sk_sp<SkImage> AK::AKTheme::textCaretVThreePatchImage(Int32 scale) noexcept
{
    const auto it { m_textCaretVThreePatchImage.find(scale) };

    if (it != m_textCaretVThreePatchImage.end())
        return it->second;

    auto surface = AKSurface::Make(SkSize(TextCaretVThreePatchSideSrcRect.width(),
                                          TextCaretVThreePatchSideSrcRect.height() + TextCaretVThreePatchCenterSrcRect.height()),
                                   scale, true);

    surface->surface()->recordingContext()->asDirectContext()->resetContext();
    SkCanvas &c { *surface->surface()->getCanvas() };
    c.scale(surface->scale(), surface->scale());
    c.clear(SK_ColorTRANSPARENT);

    const SkScalar borderRadius { surface->size().width() * 0.5f };
    SkRect roundRect { SkRect::MakeWH(surface->size().width(), surface->size().height() * 2) };
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setBlendMode(SkBlendMode::kSrc);
    paint.setStroke(false);
    paint.setColor(SK_ColorWHITE);
    c.drawRoundRect(roundRect, borderRadius, borderRadius, paint);
    surface->surface()->flush();

    sk_sp<SkImage> result { surface->releaseImage() };
    m_textCaretVThreePatchImage[scale] = result;
    return result;
}

sk_sp<SkImage> AK::AKTheme::edgeShadowImage(Int32 scale) noexcept
{
    const auto it { m_edgeShadowImage.find(scale) };

    if (it != m_edgeShadowImage.end())
        return it->second;

    auto surface = AKSurface::Make(SkSize(1, EdgeShadowRadius), scale, true);
    surface->surface()->recordingContext()->asDirectContext()->resetContext();
    SkCanvas &c { *surface->surface()->getCanvas() };
    c.scale(surface->scale(), surface->scale());
    c.clear(SK_ColorTRANSPARENT);

    SkPaint paint;
    paint.setColor(SK_ColorWHITE);
    paint.setBlendMode(SkBlendMode::kSrc);
    paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, SkScalar(EdgeShadowRadius)/3.f));
    c.drawIRect(SkIRect::MakeXYWH(-100, -100, 200, 100), paint);

    surface->surface()->flush();

    sk_sp<SkImage> result { surface->releaseImage() };
    m_edgeShadowImage[scale] = result;
    return result;
}

