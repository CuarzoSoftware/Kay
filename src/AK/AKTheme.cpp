#include <AK/AKApplication.h>
#include <AK/AKTheme.h>
#include <AK/AKSceneTarget.h>
#include <AK/AKSurface.h>
#include <AK/utils/AKImageLoader.h>
#include <AK/AKLog.h>

#include <include/gpu/ganesh/GrRecordingContext.h>
#include <include/gpu/ganesh/GrDirectContext.h>
#include <include/effects/SkGradientShader.h>
#include <include/effects/SkImageFilters.h>
#include <include/core/SkMaskFilter.h>
#include <include/core/SkBlurTypes.h>
#include <include/core/SkCanvas.h>

AK::AKTheme::AKTheme() noexcept
{
    DefaultFont.setTypeface(
        akApp()->fontManager()->matchFamilyStyle("Inter",
        SkFontStyle(
        SkFontStyle::kNormal_Weight,
        SkFontStyle::Width::kNormal_Width,
        SkFontStyle::Slant::kUpright_Slant)));
    DefaultFont.setSize(12);

    SkPaint defaultTextStylePaint;
    defaultTextStylePaint.setColor(SK_ColorBLACK);
    defaultTextStylePaint.setAntiAlias(true);
    DefaultTextStyle.setForegroundPaint(defaultTextStylePaint);
    DefaultTextStyle.setFontSize(12);
    DefaultTextStyle.setFontFamilies(std::vector<SkString>({SkString("Inter")}));
    DefaultTextStyle.setFontStyle(
        SkFontStyle(
            SkFontStyle::Weight::kNormal_Weight,
            SkFontStyle::Width::kNormal_Width,
            SkFontStyle::Slant::kUpright_Slant));
    ButtonTextStyle = DefaultTextStyle;
    ButtonTextStyle.setFontStyle(
        SkFontStyle(
            SkFontStyle::Weight::kSemiBold_Weight,
            SkFontStyle::Width::kNormal_Width,
            SkFontStyle::Slant::kUpright_Slant));
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
        SkISize(ButtonPlainHThreePatchSideSrcRect.width() + ButtonPlainHThreePatchCenterSrcRect.width(),
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
    surface->surface()->recordingContext()->asDirectContext()->flush();

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
        SkISize(ButtonTintedHThreePatchSideSrcRect.width() + ButtonTintedHThreePatchCenterSrcRect.width(),
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
    surface->surface()->recordingContext()->asDirectContext()->flush();

    sk_sp<SkImage> result { surface->releaseImage() };
    m_buttonTintedHThreePatchImage[scale] = result;
    return result;
}

sk_sp<SkImage> AK::AKTheme::textFieldRoundHThreePatchImage(Int32 scale) noexcept
{
    const auto it { m_textFieldRoundHThreePatchImage.find(scale) };

    if (it != m_textFieldRoundHThreePatchImage.end())
        return it->second;

    auto surface = AKSurface::Make(SkISize(TextFieldRoundHThreePatchSideSrcRect.width() + TextFieldRoundHThreePatchCenterSrcRect.width(),
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
    surface->surface()->recordingContext()->asDirectContext()->flush();

    sk_sp<SkImage> result { surface->releaseImage() };
    m_textFieldRoundHThreePatchImage[scale] = result;
    return result;
}

sk_sp<SkImage> AK::AKTheme::textCaretVThreePatchImage(Int32 scale) noexcept
{
    const auto it { m_textCaretVThreePatchImage.find(scale) };

    if (it != m_textCaretVThreePatchImage.end())
        return it->second;

    auto surface = AKSurface::Make(SkISize(TextCaretVThreePatchSideSrcRect.width(),
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
    surface->surface()->recordingContext()->asDirectContext()->flush();

    sk_sp<SkImage> result { surface->releaseImage() };
    m_textCaretVThreePatchImage[scale] = result;
    return result;
}

sk_sp<SkImage> AK::AKTheme::edgeShadowImage(Int32 scale) noexcept
{
    const auto it { m_edgeShadowImage.find(scale) };

    if (it != m_edgeShadowImage.end())
        return it->second;

    auto surface = AKSurface::Make(SkISize(1, EdgeShadowRadius), scale, true);
    surface->surface()->recordingContext()->asDirectContext()->resetContext();
    SkCanvas &c { *surface->surface()->getCanvas() };
    c.scale(surface->scale(), surface->scale());
    c.clear(SK_ColorTRANSPARENT);

    SkPaint paint;
    paint.setColor(SK_ColorWHITE);
    paint.setBlendMode(SkBlendMode::kSrc);
    paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, SkScalar(EdgeShadowRadius)/3.f));
    c.drawIRect(SkIRect::MakeXYWH(-100, -100, 200, 100), paint);

    surface->surface()->recordingContext()->asDirectContext()->flush();

    sk_sp<SkImage> result { surface->releaseImage() };
    m_edgeShadowImage[scale] = result;
    return result;
}

sk_sp<SkImage> AK::AKTheme::windowButtonImage(Int32 scale, AKWindowButton::Type type, AKWindowButton::State state)
{
    AK_UNUSED(scale)

    // Being lazy and always using scale 2

    auto &typeMap { m_windowButtons[scale] };
    auto &stateMap { typeMap[type] };
    auto it { stateMap.find(state) };

    if (it != stateMap.end())
        return it->second;

    sk_sp<SkImage> image;

    switch (state)
    {
    case AKWindowButton::State::Disabled:
        image = AKImageLoader::loadFile(std::filesystem::path(AK_ASSETS_DIR) / "window_button_disabled.png");
        break;
    case AKWindowButton::State::Normal:
        switch (type)
        {
            case AK::AKWindowButton::Type::Close:
                image = AKImageLoader::loadFile(std::filesystem::path(AK_ASSETS_DIR) / "window_button_close_normal.png");
                break;
            case AK::AKWindowButton::Type::Minimize:
                image = AKImageLoader::loadFile(std::filesystem::path(AK_ASSETS_DIR) / "window_button_minimize_normal.png");
                break;
            case AK::AKWindowButton::Type::Maximize:
            case AK::AKWindowButton::Type::Fullscreen:
            case AK::AKWindowButton::Type::UnsetFullscreen:
                image = AKImageLoader::loadFile(std::filesystem::path(AK_ASSETS_DIR) / "window_button_maximize_normal.png");
                break;
        }
        break;
    case AKWindowButton::State::Hover:
        switch (type)
        {
        case AK::AKWindowButton::Type::Close:
            image = AKImageLoader::loadFile(std::filesystem::path(AK_ASSETS_DIR) / "window_button_close_hover.png");
            break;
        case AK::AKWindowButton::Type::Minimize:
            image = AKImageLoader::loadFile(std::filesystem::path(AK_ASSETS_DIR) / "window_button_minimize_hover.png");
            break;
        case AK::AKWindowButton::Type::Maximize:
            image = AKImageLoader::loadFile(std::filesystem::path(AK_ASSETS_DIR) / "window_button_maximize_hover.png");
            break;
        case AK::AKWindowButton::Type::Fullscreen:
            image = AKImageLoader::loadFile(std::filesystem::path(AK_ASSETS_DIR) / "window_button_fullscreen_hover.png");
            break;
        case AK::AKWindowButton::Type::UnsetFullscreen:
            image = AKImageLoader::loadFile(std::filesystem::path(AK_ASSETS_DIR) / "window_button_unset_fullscreen_hover.png");
            break;
        }
        break;
    case AKWindowButton::State::Pressed:
        switch (type)
        {
        case AK::AKWindowButton::Type::Close:
            image = AKImageLoader::loadFile(std::filesystem::path(AK_ASSETS_DIR) / "window_button_close_pressed.png");
            break;
        case AK::AKWindowButton::Type::Minimize:
            image = AKImageLoader::loadFile(std::filesystem::path(AK_ASSETS_DIR) / "window_button_minimize_pressed.png");
            break;
        case AK::AKWindowButton::Type::Maximize:
            image = AKImageLoader::loadFile(std::filesystem::path(AK_ASSETS_DIR) / "window_button_maximize_pressed.png");
            break;
        case AK::AKWindowButton::Type::Fullscreen:
            image = AKImageLoader::loadFile(std::filesystem::path(AK_ASSETS_DIR) / "window_button_fullscreen_pressed.png");
            break;
        case AK::AKWindowButton::Type::UnsetFullscreen:
            image = AKImageLoader::loadFile(std::filesystem::path(AK_ASSETS_DIR) / "window_button_unset_fullscreen_pressed.png");
            break;
        }
        break;
    default:
        image = AKImageLoader::loadFile(std::filesystem::path(AK_ASSETS_DIR) / "window_button_disabled.png");
        break;
    }

    if (image)
    {
        stateMap[state] = image;
    }

    return image;
}

sk_sp<SkImage> AK::AKTheme::topLeftRoundCornerMask(Int32 radius, Int32 scale) noexcept
{
    assert(radius > 0 && scale > 0);

    auto &scalesMap { m_topLeftRoundCornerMasks[scale] };

    const auto &it { scalesMap.find(radius) };

    if (it != scalesMap.end())
        return it->second;

    auto surface = AKSurface::Make(SkISize(radius, radius), scale, true);
    surface->surface()->recordingContext()->asDirectContext()->resetContext();
    SkCanvas &c { *surface->surface()->getCanvas() };
    c.scale(surface->scale(), surface->scale());
    c.clear(SK_ColorTRANSPARENT);

    SkPaint paint;
    paint.setStroke(false);
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorWHITE);
    paint.setBlendMode(SkBlendMode::kSrc);
    c.drawCircle(SkPoint::Make(radius, radius), radius, paint);
    surface->surface()->recordingContext()->asDirectContext()->flush();

    sk_sp<SkImage> result { surface->releaseImage() };
    scalesMap[radius] = result;
    return result;
}

