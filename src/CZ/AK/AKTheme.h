#ifndef CZ_AKTHEME_H
#define CZ_AKTHEME_H

#include <CZ/skia/modules/skparagraph/include/FontCollection.h>
#include <CZ/skia/modules/skparagraph/include/TextStyle.h>
#include <CZ/skia/core/SkImage.h>
#include <CZ/skia/core/SkRegion.h>
#include <CZ/skia/core/SkFont.h>
#include <CZ/AK/Nodes/AKWindowButton.h>
#include <CZ/AK/AK.h>
#include <CZ/Ream/Ream.h>
#include <CZ/Core/CZOrientation.h>
#include <CZ/Core/CZAssetManager.h>
#include <CZ/Core/CZAdaptive.h>
#include <unordered_map>

namespace CZ
{
namespace AKAsset
{
    struct RRect9Patch
    {
        std::shared_ptr<RImage> image;
        SkIRect center;
    };
}
};

class CZ::AKTheme
{
public:    
    AKTheme() noexcept;
    virtual ~AKTheme() {}

    /* Colors */

    /* macOS system colors (light, dark). Source: https://swiftuicolors.com/macos-colors */

    /// Red accent for errors and destructive actions.
    static inline CZAdaptiveColor SystemRed { 0xFFFF383C, 0xFFFF4245 };
    /// Orange accent.
    static inline CZAdaptiveColor SystemOrange { 0xFFFF8D28, 0xFFFF9230 };
    /// Yellow accent for cautions and warnings.
    static inline CZAdaptiveColor SystemYellow { 0xFFFFCC00, 0xFFFFD600 };
    /// Green accent for success and confirmation.
    static inline CZAdaptiveColor SystemGreen { 0xFF34C759, 0xFF30D158 };
    /// Mint accent.
    static inline CZAdaptiveColor SystemMint { 0xFF00C8B3, 0xFF00DAC3 };
    /// Teal accent.
    static inline CZAdaptiveColor SystemTeal { 0xFF00C3D0, 0xFF00D2E0 };
    /// Cyan accent.
    static inline CZAdaptiveColor SystemCyan { 0xFF00C0E8, 0xFF3CD3FE };
    /// Blue accent, the default system tint.
    static inline CZAdaptiveColor SystemBlue { 0xFF0088FF, 0xFF0091FF };
    /// Indigo accent.
    static inline CZAdaptiveColor SystemIndigo { 0xFF6155F5, 0xFF6D7CFF };
    /// Purple accent.
    static inline CZAdaptiveColor SystemPurple { 0xFFCB30E0, 0xFFDB34F2 };
    /// Pink accent.
    static inline CZAdaptiveColor SystemPink { 0xFFFF2D55, 0xFFFF375F };
    /// Brown accent.
    static inline CZAdaptiveColor SystemBrown { 0xFFAC7F5E, 0xFFB78A66 };
    /// Neutral gray accent.
    static inline CZAdaptiveColor SystemGray { 0xFF8E8E93, 0xFF98989D };

    static inline CZAdaptiveColor SystemBackground { 0xFFFFFFFF, 0xFF000000 };

    /// Primary text/label color.
    static inline CZAdaptiveColor Label { 0xD9000000, 0xD9FFFFFF };
    /// Secondary text for subtitles and less prominent labels.
    static inline CZAdaptiveColor SecondaryLabel { 0x80000000, 0x8CFFFFFF };
    /// Tertiary text for hints and disabled labels.
    static inline CZAdaptiveColor TertiaryLabel { 0x42000000, 0x40FFFFFF };
    /// Quaternary text for faint, watermark-level labels.
    static inline CZAdaptiveColor QuaternaryLabel { 0x1A000000, 0x1AFFFFFF };
    /// Quinary text, the faintest and most decorative.
    static inline CZAdaptiveColor QuinaryLabel { 0x0D000000, 0x0DFFFFFF };

    /// Editable document text.
    static inline CZAdaptiveColor Text { 0xFF000000, 0xFFFFFFFF };
    /// Placeholder shown in empty text fields.
    static inline CZAdaptiveColor PlaceholderText { 0x80000000, 0x8CFFFFFF };
    /// Text within a selection.
    static inline CZAdaptiveColor SelectedText { 0xFF000000, 0xFFFFFFFF };
    /// Background of text/content areas (editors).
    static inline CZAdaptiveColor TextBackground { 0xFFFFFFFF, 0xFF1E1E1E };
    /// Highlight behind selected text (focused view).
    static inline CZAdaptiveColor SelectedTextBackground { 0xFFB3D7FF, 0xFF3F638B };
    /// Selected text when the view is not focused.
    static inline CZAdaptiveColor UnemphasizedSelectedText { 0xFF000000, 0xFFFFFFFF };
    /// Selection highlight when the view is not focused.
    static inline CZAdaptiveColor UnemphasizedSelectedTextBackground { 0xFFDCDCDC, 0xFF464646 };

    /// Hyperlink text.
    static inline CZAdaptiveColor Link { 0xFF0068DA, 0xFF419CFF };
    /// Text caret / insertion point.
    static inline CZAdaptiveColor TextInsertionPoint { 0xFF007AFF, 0xFF007AFF };

    /// Fill for small/thin UI shapes (e.g. toggles).
    static inline CZAdaptiveColor SystemFill { 0x1A000000, 0x1AFFFFFF };
    /// Fill for medium UI shapes.
    static inline CZAdaptiveColor SecondarySystemFill { 0x14000000, 0x14FFFFFF };
    /// Fill for large UI shapes (e.g. input areas).
    static inline CZAdaptiveColor TertiarySystemFill { 0x0D000000, 0x0DFFFFFF };
    /// Fill for extra-large UI areas.
    static inline CZAdaptiveColor QuaternarySystemFill { 0x08000000, 0x08FFFFFF };
    /// Faintest fill for the largest UI areas.
    static inline CZAdaptiveColor QuinarySystemFill { 0x03000000, 0x03FFFFFF };

    /// Divider line between content.
    static inline CZAdaptiveColor Separator { 0x1A000000, 0x1AFFFFFF };
    /// Grid lines in tables and outlines.
    static inline CZAdaptiveColor Grid { 0xFFE6E6E6, 0xFF1A1A1A };
    /// Drop-shadow color.
    static inline CZAdaptiveColor Shadow { 0xFF000000, 0xFF000000 };
    /// Bright bevel/highlight edge.
    static inline CZAdaptiveColor Highlight { 0xFFFFFFFF, 0xFFB4B4B4 };
    /// Highlight behind find/search matches.
    static inline CZAdaptiveColor FindHighlight { 0xFFFFFF00, 0xFFFFFF00 };
    /// Focus ring around the keyboard-focused control.
    static inline CZAdaptiveColor KeyboardFocusIndicator { 0x800067F4, 0x801AA9FF };

    /// The accent/tint applied to controls (buttons, switches).
    static inline CZAdaptiveColor ControlAccent { 0xFF007AFF, 0xFF007AFF };
    /// Control surface/face color.
    static inline CZAdaptiveColor ControlColor { 0xFFFFFFFF, 0x40FFFFFF };
    /// Background behind controls and content.
    static inline CZAdaptiveColor ControlBackground { 0xFFFFFFFF, 0xFF1E1E1E };
    /// Label text of an enabled control.
    static inline CZAdaptiveColor ControlText { 0xD9000000, 0xD9FFFFFF };
    /// Label text of a disabled control.
    static inline CZAdaptiveColor DisabledControlText { 0x40000000, 0x40FFFFFF };

    /// Background of selected rows/items in a focused list.
    static inline CZAdaptiveColor SelectedContentBackground { 0xFF0064E1, 0xFF0059D1 };
    /// Selected-item background when the list is unfocused.
    static inline CZAdaptiveColor UnemphasizedSelectedContentBackground { 0xFFDCDCDC, 0xFF464646 };
    /// Background of a selected control.
    static inline CZAdaptiveColor SelectedControl { 0xFFB3D7FF, 0xFF3F638B };
    /// Text of a selected control.
    static inline CZAdaptiveColor SelectedControlText { 0xD9000000, 0xD9FFFFFF };
    /// Text over an emphasized (tinted) selection.
    static inline CZAdaptiveColor AlternateSelectedControlText { 0xFFFFFFFF, 0xFFFFFFFF };
    /// Text of the highlighted menu item.
    static inline CZAdaptiveColor SelectedMenuItemText { 0xFFFFFFFF, 0xFFFFFFFF };
    /// Table/section header text.
    static inline CZAdaptiveColor HeaderText { 0xD9000000, 0xFFFFFFFF };

    /// Window background surface.
    static inline CZAdaptiveColor WindowBackground { 0xFFFFFFFF, 0xFF1E1E1E };
    /// Text drawn on the window frame/titlebar.
    static inline CZAdaptiveColor WindowFrameText { 0xD9000000, 0xD9FFFFFF };
    /// Backdrop revealed under scrollable page content.
    static inline CZAdaptiveColor UnderPageBackground { 0xE6969696, 0xFF282828 };

    /* Non-macOS Cuarzo colors */

    static inline SkColor   FocusOutlineColor   { 0xFF86b4fa };
    static inline SkColor   IconLightScheme     { 0xFF444444 };
    static inline SkColor   IconDarkScheme      { 0xFFFFFFFF };

    /* Fonts */

    static inline SkFont    DefaultFont;
    skia::textlayout::TextStyle   DefaultTextStyle;
    skia::textlayout::TextStyle   ButtonTextStyle;

    /* Renderables */

    static inline SkScalar  RenderableInactiveOpacityFactor { 0.5f };

    /* AKButton */

    static inline Int32     AKButtonBorderRadius                    { 8 };

    /* AKTextField */

    static inline SkScalar  TextFieldDisabledOpacity                { 0.3f };
    static inline SkRect    TextFieldPadding                        { 16.f, 2.f, 16.f, 2.f };
    static inline SkRect    TextFieldRoundHThreePatchCenterSrcRect  { SkRect::MakeXYWH(8.f, 0.f, 1.f, 24.f) };
    static inline SkRect    TextFieldRoundHThreePatchSideSrcRect    { SkRect::MakeWH(8.f, 24.f) };
    virtual std::shared_ptr<RImage>  textFieldRoundHThreePatchImage          (Int32 scale) noexcept;

    /* AKTextCaret */

    static inline SkRect    TextCaretVThreePatchCenterSrcRect       { SkRect::MakeXYWH(0.f, 1.f, 2.f, 1.f) };
    static inline SkRect    TextCaretVThreePatchSideSrcRect         { SkRect::MakeWH(2.f, 2.f) };
    virtual std::shared_ptr<RImage>  textCaretVThreePatchImage      (Int32 scale) noexcept;

    /* AKScroll */
    
    static inline SkScalar  ScrollKineticSpeed                      { 1.5f };
    static inline SkScalar  ScrollKineticFriction                   { 0.15f };
    static inline SkScalar  ScrollKineticInertia                    { 10.f };
    static inline SkScalar  ScrollBounceOffsetLimit                 { 96.f };
    static inline SkScalar  ScrollBarHandleWidth                    { 7.f };
    static inline SkScalar  ScrollBarHandleWidthHover               { 11.f };
    virtual std::shared_ptr<RImage>  roundLineThreePatchImage       (CZOrientation orientation, Int32 diam, Int32 scale, SkRect *outSideSrc, SkRect *outCenterSrc) noexcept;
    virtual std::shared_ptr<RImage>  scrollRailThreePatchImage      (CZOrientation orientation, Int32 scale, SkRect *outSideSrc, SkRect *outCenterSrc) noexcept;

    /* AKEdgeShadow */

    static inline Int32     EdgeShadowRadius                        { 2 };
    static inline SkColor   EdgeShadowColor                         { 0x80000000 };
    virtual std::shared_ptr<RImage>  edgeShadowImage                (Int32 scale) noexcept;

    /* Window Button */

    static inline SkISize   WindowButtonSize                        { 12, 12 };
    static inline Int32     WindowButtonGap                         { 8 };
    virtual std::shared_ptr<RImage>  windowButtonImage              (Int32 scale, AKWindowButton::Type type, AKWindowButton::State state);

    /* First quadrant of a circle white-filled */
    CZAssetManager<RImage, Int32 /*radius*/, Int32 /*scale*/> FirstQuadrantCircleMask;

    /* Icon Font (could be nullptr) */
    std::shared_ptr<AKIconFont> iconFont;

    /* Solid round container 9-patch */
    CZAssetManager<AKAsset::RRect9Patch, Int32 /*radTL*/, Int32 /*radTR*/, Int32 /*radBR*/, Int32 /*radBL*/, Int32 /*scale*/, SkColor /*backgroundColor*/, Int32 /*strokeWidth*/, SkColor /*strokeColor*/> RRect9Patch;

protected:

    /* AKButton */
    std::unordered_map<Int32,std::shared_ptr<RImage>> m_buttonPlainHThreePatchImage;
    std::unordered_map<Int32,std::shared_ptr<RImage>> m_buttonTintedHThreePatchImage;

    /* AKTextField */
    std::unordered_map<Int32,std::shared_ptr<RImage>> m_textFieldRoundHThreePatchImage;
    std::unordered_map<Int32,std::shared_ptr<RImage>> m_textCaretVThreePatchImage;

    /* AKEdgeShadow */
    std::unordered_map<Int32,std::shared_ptr<RImage>> m_edgeShadowImage;

    /* AKWindowButton [scale][type][state] */
    std::unordered_map<Int32,
        std::unordered_map<AKWindowButton::Type,
            std::unordered_map<AKWindowButton::State,
                std::shared_ptr<RImage>>>> m_windowButtons;

    /* Round Line Patch [orientation][scale][diam] */
    std::unordered_map<CZOrientation,
        std::unordered_map<Int32,
            std::unordered_map<Int32,
                std::shared_ptr<RImage>>>> m_roundLineThreePatchImage;

    /* Round Line Patch [orientation][scale] */
    std::unordered_map<CZOrientation,
        std::unordered_map<Int32,
            std::shared_ptr<RImage>>> m_scrollRailThreePatchImage;
};

#endif // CZ_AKTHEME_H
