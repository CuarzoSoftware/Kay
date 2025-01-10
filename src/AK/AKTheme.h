#ifndef AKTHEME_H
#define AKTHEME_H

#include <AK/AK.h>
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

    static inline SkColor   SystemRed     { 0xFFFC2125 };
    static inline SkColor   SystemGreen   { 0xFF29C732 };
    static inline SkColor   SystemBlue    { 0xFF0A60FF };
    static inline SkColor   SystemOrange  { 0xFFFD8208 };
    static inline SkColor   SystemYellow  { 0xFFFEC309 };
    static inline SkColor   SystemBrown   { 0xFF90714C };
    static inline SkColor   SystemPink    { 0xFFFB0D44 };
    static inline SkColor   SystemPurple  { 0xFF9D33D6 };
    static inline SkColor   SystemTeal    { 0xFF4A9DB7 };
    static inline SkColor   SystemIndigo  { 0xFF453CCC };
    static inline SkColor   SystemMint    { 0xFF18BDB0 };
    static inline SkColor   SystemCyan    { 0xFF47B0EC };
    static inline SkColor   SystemGray    { 0xFF7B7B81 };
    static inline SkColor   LinkColor     { 0xFF094FD1 };

    /* AKButton */

    static inline SkScalar  ButtonPressedBackgroundDarkness         { 0.95f };
    static inline SkScalar  ButtonContentPressedOpacity             { 0.85f };
    static inline SkScalar  ButtonDisabledOpacity                   { 0.3f };
    static inline SkFont    ButtonFont;
    static inline SkRect    ButtonPadding                           { 16.f, 2.f, 16.f, 2.f };
    static inline SkRect    ButtonPlainHThreePatchSideSrcRect       { SkRect::MakeWH(8.f, 24.f) };
    static inline SkRect    ButtonTintedHThreePatchSideSrcRect      { ButtonPlainHThreePatchSideSrcRect };
    static inline SkRect    ButtonPlainHThreePatchCenterSrcRect     { SkRect::MakeXYWH(8.f, 0.f, 1.f, 24.f) };
    static inline SkRect    ButtonTintedHThreePatchCenterSrcRect    { ButtonPlainHThreePatchCenterSrcRect };
    virtual SkRegion        buttonPlainOpaqueRegion                 (Int32 width) noexcept;
    virtual SkRegion        buttonTintedOpaqueRegion                (Int32 width) noexcept;
    virtual sk_sp<SkImage>  buttonPlainHThreePatchImage             (AKTarget *target) noexcept;
    virtual sk_sp<SkImage>  buttonTintedHThreePatchImage            (AKTarget *target) noexcept;
protected:

    /* AKButton */

    std::unordered_map<Int32,sk_sp<SkImage>> m_buttonPlainHThreePatchImage;
    std::unordered_map<Int32,sk_sp<SkImage>> m_buttonTintedHThreePatchImage;
};

#endif // AKTHEME_H
