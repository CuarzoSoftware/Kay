#include <Theme.h>

#include <AK/AKSceneTarget.h>
#include <AK/AKSurface.h>

#include <include/gpu/ganesh/GrDirectContext.h>
#include <include/effects/SkGradientShader.h>
#include <include/effects/SkImageFilters.h>
#include <include/effects/SkBlurMaskFilter.h>
#include <include/core/SkCanvas.h>

sk_sp<SkImage> Theme::dockHThreePatchImage(Int32 scale) noexcept
{
    auto it = m_dockHThreePatchImage.find(scale);

    if (it != m_dockHThreePatchImage.end())
        return it->second;

    auto surface = AKSurface::Make(
        SkISize::Make(DockHThreePatchCenterSrcRect.width() + DockHThreePatchSideSrcRect.width(),
                      DockHThreePatchCenterSrcRect.height() + DockHThreePatchSideSrcRect.height()),
        scale);

    surface->surface()->recordingContext()->asDirectContext()->resetContext();
    SkCanvas &c { *surface->surface()->getCanvas() };
    c.scale(surface->scale(), surface->scale());
    c.clear(SK_ColorTRANSPARENT);

    SkPaint paint;
    paint.setAntiAlias(true);

    const SkRect roundRect { SkRect::MakeXYWH(
        SkScalar(DockShadowRadius),
        SkScalar(DockShadowRadius - DockShadowOffsetY),
        DockHThreePatchSideSrcRect.width() * 3.f,
        DockHeight
    )};

    // Shadow
    SkRect shadowRect { roundRect };
    shadowRect.offset(0, DockShadowOffsetY);
    paint.setMaskFilter(SkMaskFilter::MakeBlur(SkBlurStyle::kNormal_SkBlurStyle, SkScalar(DockShadowRadius) / 3.f));
    paint.setColor(SkColorSetARGB(128, 0, 0, 0));
    c.drawRoundRect(shadowRect, DockBorderRadius, DockBorderRadius, paint);

    // Fill
    paint.setBlendMode(SkBlendMode::kSrc);
    paint.setMaskFilter(nullptr);
    paint.setColor(SkColorSetARGB(240, 250, 250, 250));
    c.drawRoundRect(roundRect, DockBorderRadius, DockBorderRadius, paint);

    // Stroke
    paint.setStroke(true);
    paint.setStrokeWidth(0.5);
    paint.setBlendMode(SkBlendMode::kSrcOver);
    paint.setColor(SkColorSetARGB(48, 0, 0, 0));
    c.drawRoundRect(roundRect, DockBorderRadius, DockBorderRadius, paint);
    c.drawRoundRect(roundRect, DockBorderRadius, DockBorderRadius, paint);

    surface->surface()->recordingContext()->asDirectContext()->flush();
    sk_sp<SkImage> result { surface->releaseImage() };
    m_dockHThreePatchImage[scale] = result;
    return result;
}
