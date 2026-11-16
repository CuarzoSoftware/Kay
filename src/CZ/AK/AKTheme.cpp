#include <CZ/AK/AKApp.h>
#include <CZ/AK/AKTheme.h>
#include <CZ/AK/AKTarget.h>
#include <CZ/AK/AKIconFont.h>
#include <CZ/AK/AKLog.h>
#include <CZ/AssetBroker/ABCore.h>
#include <CZ/AssetBroker/ABGenerators.h>

#include <CZ/Ream/RCore.h>
#include <CZ/Ream/RSurface.h>
#include <CZ/Ream/RPass.h>
#include <CZ/Ream/RImage.h>
#include <CZ/Ream/RDevice.h>
#include <CZ/Ream/SK/RSKFormat.h>

#include <CZ/skia/core/SkPixelRef.h>
#include <CZ/skia/gpu/ganesh/GrRecordingContext.h>
#include <CZ/skia/gpu/ganesh/GrDirectContext.h>
#include <CZ/skia/effects/SkGradientShader.h>
#include <CZ/skia/effects/SkImageFilters.h>
#include <CZ/skia/core/SkMaskFilter.h>
#include <CZ/skia/core/SkBlurTypes.h>
#include <CZ/skia/core/SkCanvas.h>
#include <CZ/skia/core/SkRRect.h>
#include <CZ/skia/core/SkBitmap.h>

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
    DefaultTextStyle.setFontSize(11);
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

    RRect9Patch.setFactory([this](Int32 radTL, Int32 radTR, Int32 radBR, Int32 radBL, Int32 scale, SkColor backgroundColor, Int32 strokeWidth, SkColor strokeColor) -> std::shared_ptr<AKAsset::RRect9Patch>
    {
        auto *ab { assetBroker() };

        if (!ab)
            return {};

        if (scale <= 0) scale = 1;
        if (strokeWidth < 0 || SkColorGetA(strokeColor) == 0) strokeWidth = 0;

        radTL = std::max(0, radTL);
        radTR = std::max(0, radTR);
        radBR = std::max(0, radBR);
        radBL = std::max(0, radBL);

        // Each 9-slice cell must fit both of its corners and the stroke; the center stays a 1x1
        // stretchable pixel. Corners may differ, so the four cells can have different sizes.
        const Int32 leftW { std::max({ radTL, radBL, strokeWidth }) };
        const Int32 topH  { std::max({ radTL, radTR, strokeWidth }) };
        const SkIRect center { SkIRect::MakeXYWH(leftW, topH, 1, 1) };

        // Drawn (and shared with other apps) by the AssetBroker
        auto image { ab->generate({ ABGenerators::AKRRect9Patch,
            { radTL, radTR, radBR, radBL, scale, backgroundColor, strokeWidth, strokeColor }, {} }) };

        if (!image)
            return {};

        return std::make_shared<AKAsset::RRect9Patch>(image, center);
    });

    FirstQuadrantCircleMask.setFactory([this](Int32 radius, Int32 scale) -> std::shared_ptr<RImage>
    {
        auto *ab { assetBroker() };

        if (!ab)
            return {};

        if (radius <= 0) radius = 1;
        if (scale <= 0) scale = 1;

        return ab->generate({ ABGenerators::AKCircleQuadrant, { radius, scale }, {} });
    });
}

ABCore *CZ::AKTheme::assetBroker() noexcept
{
    // Created on first use: it needs the CZCore and RCore (and keeps the connection to the service)
    if (!m_assetBroker)
        m_assetBroker = ABCore::GetOrMake();

    return m_assetBroker.get();
}

std::shared_ptr<RImage> CZ::AKTheme::textFieldRoundHThreePatchImage(Int32 scale) noexcept
{
    const auto it { m_textFieldRoundHThreePatchImage.find(scale) };

    if (it != m_textFieldRoundHThreePatchImage.end())
        return it->second;

    auto *ab { assetBroker() };

    if (!ab)
        return {};

    const Int32 width { Int32(TextFieldRoundHThreePatchSideSrcRect.width() + TextFieldRoundHThreePatchCenterSrcRect.width()) };
    const Int32 height { Int32(TextFieldRoundHThreePatchSideSrcRect.height()) };
    auto image { ab->generate({ ABGenerators::AKTextFieldRoundH3Patch, { width, height, scale }, {} }) };

    if (image)
        m_textFieldRoundHThreePatchImage[scale] = image;

    return image;
}

std::shared_ptr<RImage> CZ::AKTheme::textCaretVThreePatchImage(Int32 scale) noexcept
{
    const auto it { m_textCaretVThreePatchImage.find(scale) };

    if (it != m_textCaretVThreePatchImage.end())
        return it->second;

    auto *ab { assetBroker() };

    if (!ab)
        return {};

    const Int32 width { Int32(TextCaretVThreePatchSideSrcRect.width()) };
    const Int32 height { Int32(TextCaretVThreePatchSideSrcRect.height() + TextCaretVThreePatchCenterSrcRect.height()) };
    auto image { ab->generate({ ABGenerators::AKTextCaretV3Patch, { width, height, scale }, {} }) };

    if (image)
        m_textCaretVThreePatchImage[scale] = image;

    return image;
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

    auto *ab { assetBroker() };

    if (!ab)
        return {};

    auto image { ab->generate({ ABGenerators::AKRoundLine3Patch, { orientation == CZOrientation::V ? 1 : 0, diam, scale }, {} }) };

    if (image)
        scaleMap[diam] = image;

    return image;
}

std::shared_ptr<RImage> CZ::AKTheme::scrollRailThreePatchImage(CZOrientation orientation, Int32 scale, SkColor railBackground, SkColor railBorder, SkRect *outSideSrc, SkRect *outCenterSrc) noexcept
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

    const std::tuple<CZOrientation, Int32, SkColor, SkColor> key { orientation, scale, railBackground, railBorder };
    auto it { m_scrollRailThreePatchImage.find(key) };

    if (it != m_scrollRailThreePatchImage.end())
        return it->second;

    auto *ab { assetBroker() };

    if (!ab)
        return {};

    auto image { ab->generate({ ABGenerators::AKScrollRail3Patch,
        { orientation == CZOrientation::V ? 1 : 0, scale, railBackground, railBorder }, {} }) };

    if (image)
        m_scrollRailThreePatchImage[key] = image;

    return image;
}

std::shared_ptr<RImage> CZ::AKTheme::edgeShadowImage(Int32 scale) noexcept
{
    const auto it { m_edgeShadowImage.find(scale) };

    if (it != m_edgeShadowImage.end())
        return it->second;

    auto *ab { assetBroker() };

    if (!ab)
        return {};

    auto image { ab->generate({ ABGenerators::AKEdgeShadow, { EdgeShadowRadius, scale }, {} }) };

    if (image)
        m_edgeShadowImage[scale] = image;

    return image;
}

static const char *WindowButtonFile(AKWindowButton::Type type, AKWindowButton::State state) noexcept
{
    using Type = AKWindowButton::Type;
    using State = AKWindowButton::State;

    switch (state)
    {
    case State::Normal:
        switch (type)
        {
        case Type::Close:           return "window_button_close_normal.png";
        case Type::Minimize:        return "window_button_minimize_normal.png";
        case Type::Maximize:
        case Type::Fullscreen:
        case Type::UnsetFullscreen: return "window_button_maximize_normal.png";
        }
        break;
    case State::Hover:
        switch (type)
        {
        case Type::Close:           return "window_button_close_hover.png";
        case Type::Minimize:        return "window_button_minimize_hover.png";
        case Type::Maximize:        return "window_button_maximize_hover.png";
        case Type::Fullscreen:      return "window_button_fullscreen_hover.png";
        case Type::UnsetFullscreen: return "window_button_unset_fullscreen_hover.png";
        }
        break;
    case State::Pressed:
        switch (type)
        {
        case Type::Close:           return "window_button_close_pressed.png";
        case Type::Minimize:        return "window_button_minimize_pressed.png";
        case Type::Maximize:        return "window_button_maximize_pressed.png";
        case Type::Fullscreen:      return "window_button_fullscreen_pressed.png";
        case Type::UnsetFullscreen: return "window_button_unset_fullscreen_pressed.png";
        }
        break;
    default:
        break;
    }

    return "window_button_disabled.png";
}

std::shared_ptr<RImage> CZ::AKTheme::windowButtonImage(Int32 scale, AKWindowButton::Type type, AKWindowButton::State state)
{
    // Being lazy and always using scale 2 (the PNGs are 2x)

    auto &typeMap { m_windowButtons[scale] };
    auto &stateMap { typeMap[type] };
    auto it { stateMap.find(state) };

    if (it != stateMap.end())
        return it->second;

    auto *ab { assetBroker() };

    if (!ab)
        return {};

    // Loaded (and shared with other apps) by the AssetBroker
    auto image { ab->image(AKAssetsDir() / WindowButtonFile(type, state)) };

    if (image)
        stateMap[state] = image;

    return image;
}
