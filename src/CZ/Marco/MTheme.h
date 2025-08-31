#ifndef MTHEME_H
#define MTHEME_H

#include <CZ/Marco/Marco.h>
#include <CZ/AK/AKTheme.h>
#include <CZ/Core/CZBitset.h>

class CZ::MTheme : public AKTheme
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
    virtual std::shared_ptr<RImage> csdBorderRadiusMask(Int32 scale) noexcept;
    virtual std::shared_ptr<RImage> csdShadowActive(Int32 scale, const SkISize &windowSize, CZBitset<ShadowClamp> &sides) noexcept;
    virtual std::shared_ptr<RImage> csdShadowInactive(Int32 scale, const SkISize &windowSize, CZBitset<ShadowClamp> &sides) noexcept;
protected:
    std::unordered_map<Int32,std::shared_ptr<RImage>> m_csdBorderRadiusMask;
    std::unordered_map<Int32,std::shared_ptr<RImage>> m_csdShadowActive;
    std::unordered_map<Int32,std::shared_ptr<RImage>> m_csdShadowInactive;
};

#endif // MTHEME_H
