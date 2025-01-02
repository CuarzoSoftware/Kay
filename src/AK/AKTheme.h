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

    /* AKButton */

    const SkFont &buttonFont() const noexcept { return m_buttonFont; }
    const SkRect &buttonPadding() const noexcept { return m_buttonPadding; }
    SkScalar buttonDisabledOpacity() const noexcept { return m_buttonDisabledOpacity; }
    const SkRect &buttonPlainHThreePatchSideSrcRect() const noexcept { return m_buttonPlainHThreePatchSideSrcRect; }
    const SkRect &buttonPlainHThreePatchCenterSrcRect() const noexcept { return m_buttonPlainHThreePatchCenterSrcRect; }
    const SkRect &buttonTintedHThreePatchSideSrcRect() const noexcept { return m_buttonTintedHThreePatchSideSrcRect; }
    const SkRect &buttonTintedHThreePatchCenterSrcRect() const noexcept { return m_buttonTintedHThreePatchCenterSrcRect; }
    virtual SkRegion buttonPlainOpaqueRegion(Int32 width) noexcept;
    virtual SkRegion buttonTintedOpaqueRegion(Int32 width) noexcept;
    virtual sk_sp<SkImage> buttonPlainHThreePatchImage(AKTarget *target);
    virtual sk_sp<SkImage> buttonTintedHThreePatchImage(AKTarget *target);
protected:

    /* AKButton */

    SkFont m_buttonFont;
    SkRect m_buttonPadding { 16.f, 2.f, 16.f, 2.f };
    SkScalar m_buttonDisabledOpacity { 0.3f };
    SkRect m_buttonPlainHThreePatchSideSrcRect { SkRect::MakeWH(8.f, 24.f) };
    SkRect m_buttonTintedHThreePatchSideSrcRect { m_buttonPlainHThreePatchSideSrcRect };
    SkRect m_buttonPlainHThreePatchCenterSrcRect { SkRect::MakeXYWH(8.f, 0.f, 1.f, 24.f) };
    SkRect m_buttonTintedHThreePatchCenterSrcRect { m_buttonPlainHThreePatchCenterSrcRect };
    std::unordered_map<Int32,sk_sp<SkImage>> m_buttonPlainHThreePatchImage;
    std::unordered_map<Int32,sk_sp<SkImage>> m_buttonTintedHThreePatchImage;
};

#endif // AKTHEME_H
