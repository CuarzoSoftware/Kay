#ifndef MTHEME_H
#define MTHEME_H

#include <Marco/Marco.h>
#include <AK/AKTheme.h>
#include <AK/AKBitset.h>

class AK::MTheme : public AKTheme
{
public:
    MTheme() noexcept;

    /* CSD */

    enum ShadowClamp
    {
        ShadowClampX = 1 << 0,
        ShadowClampY = 1 << 1
    };

    static inline Int32 CSDMoveOutset { 54 };
    static inline Int32 CSDResizeOutset { 6 };
    static inline Int32 CSDBorderRadius { 10 };
    constexpr static Int32 CSDShadowActiveRadius { 48 };
    constexpr static Int32 CSDShadowActiveOffsetY { 18 };
    constexpr static Int32 CSDShadowInactiveRadius { 27 };
    constexpr static Int32 CSDShadowInactiveOffsetY { 8 };
    virtual sk_sp<SkImage> csdBorderRadiusMask(Int32 scale) noexcept;
    virtual sk_sp<SkImage> csdShadowActive(Int32 scale, const SkISize &windowSize, AKBitset<ShadowClamp> &sides) noexcept;
    virtual sk_sp<SkImage> csdShadowInactive(Int32 scale, const SkISize &windowSize, AKBitset<ShadowClamp> &sides) noexcept;
protected:
    std::unordered_map<Int32,sk_sp<SkImage>> m_csdBorderRadiusMask;
    std::unordered_map<Int32,sk_sp<SkImage>> m_csdShadowActive;
    std::unordered_map<Int32,sk_sp<SkImage>> m_csdShadowInactive;
};

#endif // MTHEME_H
