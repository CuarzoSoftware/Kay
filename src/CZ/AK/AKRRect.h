#ifndef CZ_AKRRECT_H
#define CZ_AKRRECT_H

#include <CZ/skia/core/SkRect.h>
#include <CZ/AK/AK.h>

class CZ::AKRRect : public SkIRect
{
public:
    AKRRect(const SkIRect &rect = SkIRect::MakeEmpty(), Int32 radTL = 0, Int32 radTR = 0, Int32 radBR = 0, Int32 radBL = 0) noexcept :
        SkIRect(rect), fRadTL(radTL), fRadTR(radTR), fRadBR(radBR), fRadBL(radBL)
    {}

    bool isValid() const noexcept
    {
        return
            width() >= 0 &&
            height() >= 0 &&
            fRadTL >= 0 &&
            fRadTR >= 0 &&
            fRadBR >= 0 &&
            fRadBL >= 0 &&
            fRadTL + fRadTR <= width() &&
            fRadBL + fRadBR <= width() &&
            fRadTL + fRadBL <= height() &&
            fRadTR + fRadBR <= height();
    }

    constexpr bool operator==(const AKRRect& other) const
    {
        return fLeft == other.fLeft  &&
               fRight == other.fRight  &&
               fBottom == other.fBottom  &&
               fTop == other.fTop  &&
               fRadTL == other.fRadTL &&
               fRadTR == other.fRadTR &&
               fRadBR == other.fRadBR &&
               fRadBL == other.fRadBL;
    }

    constexpr bool operator!=(const AKRRect& other) const
    {
        return !(operator==(other));
    }

    Int32 fRadTL { 0 };
    Int32 fRadTR { 0 };
    Int32 fRadBR { 0 };
    Int32 fRadBL { 0 };
};

#endif // CZ_AKRRECT_H
