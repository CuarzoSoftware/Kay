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
#include <map>
#include <tuple>

namespace CZ
{
namespace AKAsset
{
    /**
     * @brief A baked 9-patch image for solid rounded-rect containers.
     * @ingroup AKCore
     */
    struct RRect9Patch
    {
        /// The baked 9-patch image.
        std::shared_ptr<RImage> image;
        /// The stretchable center cell within the image, in logical pixels.
        SkIRect center;
    };
}
};

/**
 * @brief Centralized visual style for built-in components.
 *
 * AKTheme gathers everything Kay's built-in components use to draw themselves:
 *
 * - **Colors**: adaptive (light, dark) macOS-style system colors as CZAdaptiveColor
 *   static members, plus a few Cuarzo-specific ones.
 * - **Fonts**: the default Skia font and text styles (initialized in the constructor,
 *   using the "Inter" family).
 * - **Metrics**: per-component constants such as paddings, radii and scroll physics.
 * - **Baked assets**: lazily created, per-scale cached images (nine/three-patches,
 *   shadows, window buttons, masks) exposed through virtual factory methods and
 *   CZAssetManager members.
 *
 * The active theme is returned by CZ::theme(), which lazily creates the default one.
 * To customize the look, subclass AKTheme (e.g. overriding the virtual asset factories
 * or adjusting the static members) and install it with CZ::setTheme().
 *
 * @ingroup AKCore
 */
class CZ::AKTheme
{
public:    
    /** @brief Constructs the theme, initializing fonts, text styles, the icon font and asset factories. */
    AKTheme() noexcept;

    /** @brief Destructor. */
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
    /// Secondary adaptable gray.
    static inline CZAdaptiveColor SystemGray2 { 0xFFAEAEB2, 0xFF636366 };
    /// Tertiary adaptable gray.
    static inline CZAdaptiveColor SystemGray3 { 0xFFC7C7CC, 0xFF48484A };
    /// Quaternary adaptable gray.
    static inline CZAdaptiveColor SystemGray4 { 0xFFD1D1D6, 0xFF3A3A3C };
    /// Low-emphasis adaptable gray.
    static inline CZAdaptiveColor SystemGray5 { 0xFFE5E5EA, 0xFF2C2C2E };
    /// Lightest adaptable gray.
    static inline CZAdaptiveColor SystemGray6 { 0xFFF2F2F7, 0xFF1C1C1E };

    /// Pure black (does not adapt).
    static inline CZAdaptiveColor Black { 0xFF000000, 0xFF000000 };
    /// Pure white (does not adapt).
    static inline CZAdaptiveColor White { 0xFFFFFFFF, 0xFFFFFFFF };

    /// Background for the overall view.
    static inline CZAdaptiveColor SystemBackground { 0xFFFFFFFF, 0xFF000000 };
    /// Background for grouped content.
    static inline CZAdaptiveColor SecondarySystemBackground { 0xFFF2F2F7, 0xFF1C1C1E };
    /// Background for content nested inside secondary surfaces.
    static inline CZAdaptiveColor TertiarySystemBackground { 0xFFFFFFFF, 0xFF2C2C2E };
    /// Background for grouped table-style layouts.
    static inline CZAdaptiveColor SystemGroupedBackground { 0xFFF2F2F7, 0xFF000000 };
    /// Grouped background for secondary surfaces.
    static inline CZAdaptiveColor SecondarySystemGroupedBackground { 0xFFFFFFFF, 0xFF1C1C1E };
    /// Grouped background for nested tertiary surfaces.
    static inline CZAdaptiveColor TertiarySystemGroupedBackground { 0xFFF2F2F7, 0xFF2C2C2E };

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

    /// Legacy always-light text (fixed; does not adapt).
    static inline CZAdaptiveColor LightText { 0x99FFFFFF, 0x99FFFFFF };
    /// Legacy always-dark text (fixed; does not adapt).
    static inline CZAdaptiveColor DarkText { 0xFF000000, 0xFF000000 };

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
    /// Separator that fully obscures content behind it.
    static inline CZAdaptiveColor OpaqueSeparator { 0xFFC6C6C8, 0xFF38383A };
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

    /// Outline drawn around the keyboard-focused component.
    static inline SkColor   FocusOutlineColor   { 0xFF86b4fa };
    /// Icon tint for the light color scheme.
    static inline SkColor   IconLightScheme     { 0xFF444444 };
    /// Icon tint for the dark color scheme.
    static inline SkColor   IconDarkScheme      { 0xFFFFFFFF };

    /* Fonts */

    /// Default Skia font ("Inter", regular, size 12).
    static inline SkFont    DefaultFont;
    /// Default paragraph text style ("Inter", regular, size 11, black).
    skia::textlayout::TextStyle   DefaultTextStyle;
    /// Text style for button labels (DefaultTextStyle with semi-bold weight).
    skia::textlayout::TextStyle   ButtonTextStyle;

    /* Renderables */

    /// Opacity factor applied to renderables while their window is inactive. Defaults to 0.5.
    static inline SkScalar  RenderableInactiveOpacityFactor { 0.5f };

    /* AKButton */

    /// Border radius of AKButton in logical pixels. Defaults to 8.
    static inline Int32     AKButtonBorderRadius                    { 8 };

    /* AKTextField */

    /// Opacity of a disabled AKTextField. Defaults to 0.3.
    static inline SkScalar  TextFieldDisabledOpacity                { 0.3f };
    /// Content padding of AKTextField (left, top, right, bottom). Defaults to (16, 2, 16, 2).
    static inline SkRect    TextFieldPadding                        { 16.f, 2.f, 16.f, 2.f };
    /// Source rect of the stretchable center cell within textFieldRoundHThreePatchImage().
    static inline SkRect    TextFieldRoundHThreePatchCenterSrcRect  { SkRect::MakeXYWH(8.f, 0.f, 1.f, 24.f) };
    /// Source rect of a side cell within textFieldRoundHThreePatchImage().
    static inline SkRect    TextFieldRoundHThreePatchSideSrcRect    { SkRect::MakeWH(8.f, 24.f) };
    /** @brief Baked horizontal 3-patch for the rounded AKTextField background (cached per scale). */
    virtual std::shared_ptr<RImage>  textFieldRoundHThreePatchImage          (Int32 scale) noexcept;

    /* AKTextCaret */

    /// Source rect of the stretchable center cell within textCaretVThreePatchImage().
    static inline SkRect    TextCaretVThreePatchCenterSrcRect       { SkRect::MakeXYWH(0.f, 1.f, 2.f, 1.f) };
    /// Source rect of a side cell within textCaretVThreePatchImage().
    static inline SkRect    TextCaretVThreePatchSideSrcRect         { SkRect::MakeWH(2.f, 2.f) };
    /** @brief Baked vertical 3-patch for the AKTextCaret (cached per scale). */
    virtual std::shared_ptr<RImage>  textCaretVThreePatchImage      (Int32 scale) noexcept;

    /* AKScroll */
    
    /// Kinetic scrolling speed factor. Defaults to 1.5.
    static inline SkScalar  ScrollKineticSpeed                      { 1.5f };
    /// Kinetic scrolling friction. Defaults to 0.15.
    static inline SkScalar  ScrollKineticFriction                   { 0.15f };
    /// Kinetic scrolling inertia. Defaults to 10.
    static inline SkScalar  ScrollKineticInertia                    { 10.f };
    /// Maximum overscroll bounce offset in logical pixels. Defaults to 96.
    static inline SkScalar  ScrollBounceOffsetLimit                 { 96.f };
    /// Scroll bar handle width in logical pixels. Defaults to 7.
    static inline SkScalar  ScrollBarHandleWidth                    { 7.f };
    /// Scroll bar handle width while hovered, in logical pixels. Defaults to 11.
    static inline SkScalar  ScrollBarHandleWidthHover               { 11.f };
    /**
     * @brief Baked 3-patch of a round-capped line of the given diameter, used for the scroll bar handle.
     *
     * Cached per orientation/diameter/scale. The side and center source rects are returned
     * through @p outSideSrc and @p outCenterSrc.
     */
    virtual std::shared_ptr<RImage>  roundLineThreePatchImage       (CZOrientation orientation, Int32 diam, Int32 scale, SkRect *outSideSrc, SkRect *outCenterSrc) noexcept;
    /**
     * @brief Baked 3-patch for the scroll bar rail with the given background and border colors.
     *
     * Cached per orientation/scale/colors. The side and center source rects are returned
     * through @p outSideSrc and @p outCenterSrc.
     */
    virtual std::shared_ptr<RImage>  scrollRailThreePatchImage      (CZOrientation orientation, Int32 scale, SkColor railBackground, SkColor railBorder, SkRect *outSideSrc, SkRect *outCenterSrc) noexcept;

    /* AKEdgeShadow */

    /// Thickness of AKEdgeShadow in logical pixels. Defaults to 2.
    static inline Int32     EdgeShadowRadius                        { 2 };
    /// Tint color of AKEdgeShadow. Defaults to half-transparent black.
    static inline SkColor   EdgeShadowColor                         { 0x80000000 };
    /** @brief Baked edge shadow gradient used by AKEdgeShadow (cached per scale). */
    virtual std::shared_ptr<RImage>  edgeShadowImage                (Int32 scale) noexcept;

    /* Window Button */

    /// Size of an AKWindowButton in logical pixels. Defaults to 12x12.
    static inline SkISize   WindowButtonSize                        { 12, 12 };
    /// Gap between window buttons in logical pixels. Defaults to 8.
    static inline Int32     WindowButtonGap                         { 8 };
    /** @brief Baked AKWindowButton image for the given type and state (cached per scale/type/state). */
    virtual std::shared_ptr<RImage>  windowButtonImage              (Int32 scale, AKWindowButton::Type type, AKWindowButton::State state);

    /* First quadrant of a circle white-filled */
    /// Cache of white-filled first-quadrant circle masks, keyed by radius and scale.
    CZAssetManager<RImage, Int32 /*radius*/, Int32 /*scale*/> FirstQuadrantCircleMask;

    /* Icon Font (could be nullptr) */
    /// The theme's icon font ("Material Icons Round" by default). Can be `nullptr` if loading failed.
    std::shared_ptr<AKIconFont> iconFont;

    /* Solid round container 9-patch */
    /// Cache of solid rounded-rect container 9-patches, keyed by corner radii, scale, fill and stroke.
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

    /* Scroll Rail Patch [orientation, scale, background, border] */
    std::map<std::tuple<CZOrientation, Int32, SkColor, SkColor>,
        std::shared_ptr<RImage>> m_scrollRailThreePatchImage;
};

#endif // CZ_AKTHEME_H
