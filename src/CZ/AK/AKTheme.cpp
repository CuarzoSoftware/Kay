#include <CZ/AK/AKApp.h>
#include <CZ/AK/AKTheme.h>
#include <CZ/AK/AKTarget.h>
#include <CZ/AK/AKIconFont.h>
#include <CZ/AK/AKLog.h>

#include <CZ/Ream/RCore.h>
#include <CZ/Ream/RSurface.h>
#include <CZ/Ream/RPass.h>
#include <CZ/Ream/RImage.h>
#include <CZ/Ream/RDevice.h>

#include <CZ/skia/gpu/ganesh/GrRecordingContext.h>
#include <CZ/skia/gpu/ganesh/GrDirectContext.h>
#include <CZ/skia/effects/SkGradientShader.h>
#include <CZ/skia/effects/SkImageFilters.h>
#include <CZ/skia/core/SkMaskFilter.h>
#include <CZ/skia/core/SkBlurTypes.h>
#include <CZ/skia/core/SkCanvas.h>

using namespace CZ;

CZ::AKTheme::AKTheme() noexcept
{
    DefaultFont.setTypeface(
        AKApp::Get()->fontManager()->matchFamilyStyle("Inter",
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

    iconFont = AKIconFont::Make("Material Icons Round", AKFontsDir() / "MaterialIconsRound-Regular.codepoints");
}

SkRegion CZ::AKTheme::buttonPlainOpaqueRegion(Int32 width) noexcept
{
    SkRegion region;
    region.op(SkIRect(3, 7, width - 3, 24 - 7), SkRegion::kUnion_Op);
    region.op(SkIRect(7, 3, width - 7, 24 - 3), SkRegion::kUnion_Op);
    return region;
}

SkRegion CZ::AKTheme::buttonTintedOpaqueRegion(Int32 width) noexcept
{
    SkRegion region;
    region.op(SkIRect(3, 7, width - 3, 24 - 7), SkRegion::kUnion_Op);
    region.op(SkIRect(7, 3, width - 7, 24 - 3), SkRegion::kUnion_Op);
    return region;
}

std::shared_ptr<RImage> CZ::AKTheme::buttonPlainHThreePatchImage(Int32 scale) noexcept
{
    const auto it { m_buttonPlainHThreePatchImage.find(scale) };

    if (it != m_buttonPlainHThreePatchImage.end())
        return it->second;

    auto surface = RSurface::Make(
        SkISize(ButtonPlainHThreePatchSideSrcRect.width() + ButtonPlainHThreePatchCenterSrcRect.width(),
               ButtonPlainHThreePatchSideSrcRect.height()),
        scale, true);

    auto pass { surface->beginPass(RPassCap_SkCanvas) };
    auto &c { *pass->getCanvas() };
    // c.scale(scale, scale);
    c.clear(SK_ColorTRANSPARENT);

    SkPaint paint;
    const float borderRadius { 5.f };
    SkRect roundRect { SkRect::MakeWH(surface->geometry().viewport.width() * 2, surface->geometry().viewport.height()) };

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

    m_buttonPlainHThreePatchImage[scale] = surface->image();
    return surface->image();
}

std::shared_ptr<RImage> CZ::AKTheme::buttonTintedHThreePatchImage(Int32 scale) noexcept
{
    const auto it { m_buttonTintedHThreePatchImage.find(scale) };

    if (it != m_buttonTintedHThreePatchImage.end())
        return it->second;

    auto surface = RSurface::Make(
        SkISize(ButtonTintedHThreePatchSideSrcRect.width() + ButtonTintedHThreePatchCenterSrcRect.width(),
               ButtonTintedHThreePatchSideSrcRect.height()),
                scale, true);

    auto pass { surface->beginPass(RPassCap_SkCanvas) };
    auto &c { *pass->getCanvas() };
    // c.scale(surface->scale(), surface->scale());
    c.clear(SK_ColorTRANSPARENT);

    SkPaint paint;
    const float borderRadius { 5.f };
    SkRect roundRect { SkRect::MakeWH(surface->geometry().viewport.width() * 2, surface->geometry().viewport.height()) };

    const SkPoint gradPoints[] { SkPoint(0.f, 2.f), SkPoint(0.f, surface->geometry().viewport.height() - 4.f) };
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

    m_buttonTintedHThreePatchImage[scale] = surface->image();
    return surface->image();
}

std::shared_ptr<RImage> CZ::AKTheme::textFieldRoundHThreePatchImage(Int32 scale) noexcept
{
    const auto it { m_textFieldRoundHThreePatchImage.find(scale) };

    if (it != m_textFieldRoundHThreePatchImage.end())
        return it->second;

    auto surface = RSurface::Make(SkISize(TextFieldRoundHThreePatchSideSrcRect.width() + TextFieldRoundHThreePatchCenterSrcRect.width(),
                                          TextFieldRoundHThreePatchSideSrcRect.height()),
                                   scale, true);

    auto pass { surface->beginPass(RPassCap_SkCanvas) };
    auto &c { *pass->getCanvas() };

    // c.scale(surface->scale(), surface->scale());
    c.clear(SK_ColorTRANSPARENT);

    SkPaint paint;
    const float borderRadius { 5.f };
    SkRect roundRect { SkRect::MakeWH(surface->geometry().viewport.width() * 2, surface->geometry().viewport.height()) };

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

    m_textFieldRoundHThreePatchImage[scale] = surface->image();
    return surface->image();
}

std::shared_ptr<RImage> CZ::AKTheme::textCaretVThreePatchImage(Int32 scale) noexcept
{
    const auto it { m_textCaretVThreePatchImage.find(scale) };

    if (it != m_textCaretVThreePatchImage.end())
        return it->second;

    auto surface = RSurface::Make(SkISize(TextCaretVThreePatchSideSrcRect.width(),
                                          TextCaretVThreePatchSideSrcRect.height() + TextCaretVThreePatchCenterSrcRect.height()),
                                   scale, true);

    auto pass { surface->beginPass(RPassCap_SkCanvas) };
    auto &c { *pass->getCanvas() };

    // c.scale(surface->scale(), surface->scale());
    c.clear(SK_ColorTRANSPARENT);

    const SkScalar borderRadius { surface->geometry().viewport.width() * 0.5f };
    SkRect roundRect { SkRect::MakeWH(surface->geometry().viewport.width(), surface->geometry().viewport.height() * 2) };
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setBlendMode(SkBlendMode::kSrc);
    paint.setStroke(false);
    paint.setColor(SK_ColorWHITE);
    c.drawRoundRect(roundRect, borderRadius, borderRadius, paint);

    m_textCaretVThreePatchImage[scale] = surface->image();
    return surface->image();
}

std::shared_ptr<RImage> CZ::AKTheme::roundLineThreePatchImage(CZOrientation orientation, Int32 diam, Int32 scale, SkRect *outSideSrc, SkRect *outCenterSrc) noexcept
{
    if (diam <= 0 || scale <= 0 || outSideSrc == nullptr || outCenterSrc == nullptr)
        return nullptr;

    const SkScalar rad { SkScalar(diam) * 0.5f };

    if (orientation == CZOrientation::V)
    {
        outSideSrc->setXYWH(0, 1.f, diam, rad);
        outCenterSrc->setXYWH(0, rad + 1.f, diam, 1.f);
    }
    else
    {
        outSideSrc->setXYWH(1.f, 0.f, rad, diam);
        outCenterSrc->setXYWH(rad + 1.f, 0.f, 1.f, diam);
    }

    auto &orientationMap { m_roundLineThreePatchImage[orientation] };
    auto &scaleMap { orientationMap[scale] };

    auto it { scaleMap.find(diam) };

    if (it != scaleMap.end())
        return it->second;

    auto surface = RSurface::Make(
        SkISize(
            orientation == CZOrientation::V ? diam : rad + 2.f,
            orientation == CZOrientation::V ? rad + 2.f : diam),
        scale, true);

    auto pass { surface->beginPass(RPassCap_SkCanvas) };
    auto &c { *pass->getCanvas() };

    // c.scale(surface->scale(), surface->scale());
    c.clear(SK_ColorTRANSPARENT);

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setBlendMode(SkBlendMode::kSrc);
    paint.setStroke(false);
    paint.setColor(SK_ColorWHITE);

    if (orientation == CZOrientation::V)
        c.drawCircle(SkPoint(rad, rad + 1.f), rad, paint);
    else
        c.drawCircle(SkPoint(rad + 1.f, rad), rad, paint);

    c.drawRect(*outCenterSrc, paint);

    scaleMap[diam] = surface->image();
    return surface->image();
}

std::shared_ptr<RImage> CZ::AKTheme::scrollRailThreePatchImage(CZOrientation orientation, Int32 scale, SkRect *outSideSrc, SkRect *outCenterSrc) noexcept
{
    if (scale <= 0 || outSideSrc == nullptr || outCenterSrc == nullptr)
        return nullptr;

    if (orientation == CZOrientation::V)
    {
        outSideSrc->setXYWH(0.f, 0.f, 1.f, 2.f);
        outCenterSrc->setXYWH(0, 2.f, 1.f, 1.f);
    }
    else
    {
        outSideSrc->setXYWH(0.f, 0.f, 2.f, 1.f);
        outCenterSrc->setXYWH(2.f, 0.f, 1.f, 1.f);
    }

    auto &orientationMap { m_scrollRailThreePatchImage[orientation] };
    auto it { orientationMap.find(scale) };

    if (it != orientationMap.end())
        return it->second;

    auto surface = RSurface::Make(
        SkISize(
            orientation == CZOrientation::V ? 1.f : 3.f,
            orientation == CZOrientation::V ? 3.f : 1.f),
        scale, true);

    auto pass { surface->beginPass(RPassCap_SkCanvas) };
    auto &c { *pass->getCanvas() };

    // c.scale(surface->scale(), surface->scale());
    c.clear(SkColorSetARGB(255, 251, 251, 251));

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setBlendMode(SkBlendMode::kSrcOver);
    paint.setStroke(true);
    paint.setStrokeWidth(1.f);
    paint.setColor(SkColorSetARGB(25, 0, 0, 0));

    if (orientation == CZOrientation::V)
        c.drawLine(-5.f, 0.f, 5.f, 0.f, paint);
    else
        c.drawLine(0.f, -5.f, 0.f, 5.f, paint);

    orientationMap[scale] = surface->image();
    return surface->image();
}

std::shared_ptr<RImage> CZ::AKTheme::edgeShadowImage(Int32 scale) noexcept
{
    const auto it { m_edgeShadowImage.find(scale) };

    if (it != m_edgeShadowImage.end())
        return it->second;

    auto surface = RSurface::Make(SkISize(1, EdgeShadowRadius), scale, true);
    auto pass { surface->beginPass(RPassCap_SkCanvas) };
    auto &c { *pass->getCanvas() };

    // c.scale(surface->scale(), surface->scale());
    c.clear(SK_ColorTRANSPARENT);

    SkPaint paint;
    paint.setColor(SK_ColorWHITE);
    paint.setBlendMode(SkBlendMode::kSrc);
    paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, SkScalar(EdgeShadowRadius)/3.f));
    c.drawIRect(SkIRect::MakeXYWH(-100, -100, 200, 100), paint);

    m_edgeShadowImage[scale] = surface->image();
    return surface->image();
}

std::shared_ptr<RImage> CZ::AKTheme::windowButtonImage(Int32 scale, AKWindowButton::Type type, AKWindowButton::State state)
{
    CZ_UNUSED(scale)

    // Being lazy and always using scale 2

    auto &typeMap { m_windowButtons[scale] };
    auto &stateMap { typeMap[type] };
    auto it { stateMap.find(state) };

    if (it != stateMap.end())
        return it->second;

    std::shared_ptr<RImage> image;

    auto *mainDevice { RCore::Get()->mainDevice() };

    RImageConstraints cons {};
    cons.allocator = mainDevice;
    cons.caps[mainDevice] = RImageCap_Src | RImageCap_SkImage;

    auto fmt { mainDevice->textureFormats().formats().find(DRM_FORMAT_ARGB8888) };
    assert(fmt != mainDevice->textureFormats().formats().end());

    switch (state)
    {
    case AKWindowButton::State::Disabled:
        image = RImage::LoadFile(AKAssetsDir() / "window_button_disabled.png", *fmt, {}, &cons);
        break;
    case AKWindowButton::State::Normal:
        switch (type)
        {
            case CZ::AKWindowButton::Type::Close:
                image = RImage::LoadFile(AKAssetsDir() / "window_button_close_normal.png", *fmt, {}, &cons);
                break;
            case CZ::AKWindowButton::Type::Minimize:
                image = RImage::LoadFile(AKAssetsDir() / "window_button_minimize_normal.png", *fmt, {}, &cons);
                break;
            case CZ::AKWindowButton::Type::Maximize:
            case CZ::AKWindowButton::Type::Fullscreen:
            case CZ::AKWindowButton::Type::UnsetFullscreen:
                image = RImage::LoadFile(AKAssetsDir() / "window_button_maximize_normal.png", *fmt, {}, &cons);
                break;
        }
        break;
    case AKWindowButton::State::Hover:
        switch (type)
        {
        case CZ::AKWindowButton::Type::Close:
            image = RImage::LoadFile(AKAssetsDir() / "window_button_close_hover.png", *fmt, {}, &cons);
            break;
        case CZ::AKWindowButton::Type::Minimize:
            image = RImage::LoadFile(AKAssetsDir() / "window_button_minimize_hover.png", *fmt, {}, &cons);
            break;
        case CZ::AKWindowButton::Type::Maximize:
            image = RImage::LoadFile(AKAssetsDir() / "window_button_maximize_hover.png", *fmt, {}, &cons);
            break;
        case CZ::AKWindowButton::Type::Fullscreen:
            image = RImage::LoadFile(AKAssetsDir() / "window_button_fullscreen_hover.png", *fmt, {}, &cons);
            break;
        case CZ::AKWindowButton::Type::UnsetFullscreen:
            image = RImage::LoadFile(AKAssetsDir() / "window_button_unset_fullscreen_hover.png", *fmt, {}, &cons);
            break;
        }
        break;
    case AKWindowButton::State::Pressed:
        switch (type)
        {
        case CZ::AKWindowButton::Type::Close:
            image = RImage::LoadFile(AKAssetsDir() / "window_button_close_pressed.png", *fmt, {}, &cons);
            break;
        case CZ::AKWindowButton::Type::Minimize:
            image = RImage::LoadFile(AKAssetsDir() / "window_button_minimize_pressed.png", *fmt, {}, &cons);
            break;
        case CZ::AKWindowButton::Type::Maximize:
            image = RImage::LoadFile(AKAssetsDir() / "window_button_maximize_pressed.png", *fmt, {}, &cons);
            break;
        case CZ::AKWindowButton::Type::Fullscreen:
            image = RImage::LoadFile(AKAssetsDir() / "window_button_fullscreen_pressed.png", *fmt, {}, &cons);
            break;
        case CZ::AKWindowButton::Type::UnsetFullscreen:
            image = RImage::LoadFile(AKAssetsDir() / "window_button_unset_fullscreen_pressed.png", *fmt, {}, &cons);
            break;
        }
        break;
    default:
        image = RImage::LoadFile(AKAssetsDir() / "window_button_disabled.png", *fmt, {}, &cons);
        break;
    }

    if (image)
    {
        stateMap[state] = image;
    }

    return image;
}

std::shared_ptr<RImage> CZ::AKTheme::topLeftRoundCornerMask(Int32 radius, Int32 scale) noexcept
{
    assert(radius > 0 && scale > 0);

    auto &scalesMap { m_topLeftRoundCornerMasks[scale] };

    const auto &it { scalesMap.find(radius) };

    if (it != scalesMap.end())
        return it->second;

    auto surface = RSurface::Make(SkISize(radius, radius), scale, true);
    auto pass { surface->beginPass(RPassCap_SkCanvas) };
    auto &c { *pass->getCanvas() };

    // c.scale(surface->scale(), surface->scale());
    c.clear(SK_ColorTRANSPARENT);

    SkPaint paint;
    paint.setStroke(false);
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorWHITE);
    paint.setBlendMode(SkBlendMode::kSrc);
    c.drawCircle(SkPoint::Make(radius, radius), radius, paint);

    scalesMap[radius] = surface->image();
    return surface->image();
}

