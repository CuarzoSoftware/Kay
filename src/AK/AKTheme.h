#ifndef AKTHEME_H
#define AKTHEME_H

#include "modules/skparagraph/include/FontCollection.h"
#include <AK/AK.h>
#include <modules/skparagraph/include/TextStyle.h>
#include <include/core/SkImage.h>
#include <include/core/SkRegion.h>
#include <include/core/SkFont.h>
#include <unordered_map>

class AK::AKTheme
{
public:
    AKTheme() noexcept;
    virtual ~AKTheme() {}

    /* Colors */

    static inline SkColor   SystemRed           { 0xFFFC2125 };
    static inline SkColor   SystemGreen         { 0xFF29C732 };
    static inline SkColor   SystemBlue          { 0xFF0A60FF };
    static inline SkColor   SystemOrange        { 0xFFFD8208 };
    static inline SkColor   SystemYellow        { 0xFFFEC309 };
    static inline SkColor   SystemBrown         { 0xFF90714C };
    static inline SkColor   SystemPink          { 0xFFFB0D44 };
    static inline SkColor   SystemPurple        { 0xFF9D33D6 };
    static inline SkColor   SystemTeal          { 0xFF4A9DB7 };
    static inline SkColor   SystemIndigo        { 0xFF453CCC };
    static inline SkColor   SystemMint          { 0xFF18BDB0 };
    static inline SkColor   SystemCyan          { 0xFF47B0EC };
    static inline SkColor   SystemGray          { 0xFF7B7B81 };
    static inline SkColor   LinkColor           { 0xFF094FD1 };
    static inline SkColor   FocusOutlineColor   { 0xFF86b4fa };

    /* Fonts */

    static inline SkFont    DefaultFont;
    skia::textlayout::TextStyle   DefaultTextStyle;
    skia::textlayout::TextStyle   ButtonTextStyle;

    /* Renderables */

    static inline SkScalar  RenderableInactiveOpacityFactor { 0.5f };

    /* AKButton */

    static inline SkScalar  ButtonPressedBackgroundDarkness         { 0.95f };
    static inline SkScalar  ButtonContentPressedOpacity             { 0.85f };
    static inline SkScalar  ButtonDisabledOpacity                   { 0.3f };
    static inline SkRect    ButtonPadding                           { 16.f, 2.f, 16.f, 2.f };
    static inline SkRect    ButtonPlainHThreePatchSideSrcRect       { SkRect::MakeWH(8.f, 24.f) };
    static inline SkRect    ButtonTintedHThreePatchSideSrcRect      { ButtonPlainHThreePatchSideSrcRect };
    static inline SkRect    ButtonPlainHThreePatchCenterSrcRect     { SkRect::MakeXYWH(8.f, 0.f, 1.f, 24.f) };
    static inline SkRect    ButtonTintedHThreePatchCenterSrcRect    { ButtonPlainHThreePatchCenterSrcRect };
    virtual SkRegion        buttonPlainOpaqueRegion                 (Int32 width) noexcept;
    virtual SkRegion        buttonTintedOpaqueRegion                (Int32 width) noexcept;
    virtual sk_sp<SkImage>  buttonPlainHThreePatchImage             (Int32 scale) noexcept;
    virtual sk_sp<SkImage>  buttonTintedHThreePatchImage            (Int32 scale) noexcept;

    /* AKTextField */

    static inline SkScalar  TextFieldDisabledOpacity                { 0.3f };
    static inline SkRect    TextFieldPadding                        { 16.f, 2.f, 16.f, 2.f };
    static inline SkRect    TextFieldRoundHThreePatchCenterSrcRect  { SkRect::MakeXYWH(8.f, 0.f, 1.f, 24.f) };
    static inline SkRect    TextFieldRoundHThreePatchSideSrcRect    { SkRect::MakeWH(8.f, 24.f) };
    virtual sk_sp<SkImage>  textFieldRoundHThreePatchImage          (Int32 scale) noexcept;

    /* AKTextCaret */

    static inline SkRect    TextCaretVThreePatchCenterSrcRect       { SkRect::MakeXYWH(0.f, 1.f, 2.f, 1.f) };
    static inline SkRect    TextCaretVThreePatchSideSrcRect         { SkRect::MakeWH(2.f, 2.f) };
    virtual sk_sp<SkImage>  textCaretVThreePatchImage               (Int32 scale) noexcept;

    /* AKEdgeShadow */
    static inline Int32     EdgeShadowRadius                        { 2 };
    static inline SkColor   EdgeShadowColor                         { 0x80000000 };
    virtual sk_sp<SkImage>  edgeShadowImage                         (Int32 scale) noexcept;

protected:

    /* AKButton */

    std::unordered_map<Int32,sk_sp<SkImage>> m_buttonPlainHThreePatchImage;
    std::unordered_map<Int32,sk_sp<SkImage>> m_buttonTintedHThreePatchImage;

    /* AKTextField */

    std::unordered_map<Int32,sk_sp<SkImage>> m_textFieldRoundHThreePatchImage;
    std::unordered_map<Int32,sk_sp<SkImage>> m_textCaretVThreePatchImage;

    /* AKEdgeShadow */
    std::unordered_map<Int32,sk_sp<SkImage>> m_edgeShadowImage;
};

#endif // AKTHEME_H
