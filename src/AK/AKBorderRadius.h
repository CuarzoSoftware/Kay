#ifndef AKBORDERRADIUS_H
#define AKBORDERRADIUS_H

#include <AK/AK.h>

namespace AK
{
    struct AKBorderRadius
    {
        static constexpr AKBorderRadius Make(Int32 rad) noexcept
        {
            return AKBorderRadius{rad, rad, rad, rad};
        }

        static constexpr AKBorderRadius MakeTB(Int32 top, Int32 bottom) noexcept
        {
            return AKBorderRadius{top, top, bottom, bottom};
        }

        static constexpr AKBorderRadius MakeLR(Int32 left, Int32 right) noexcept
        {
            return AKBorderRadius{left, right, left, right};
        }

        constexpr bool operator==(const AKBorderRadius &other) noexcept
        {
            return fTL == other.fTL &&
                   fTR == other.fTR &&
                   fBL == other.fBL &&
                   fBR == other.fBR;
        }

        constexpr bool operator!=(const AKBorderRadius &other) noexcept
        {
            return fTL != other.fTL ||
                   fTR != other.fTR ||
                   fBL != other.fBL ||
                   fBR != other.fBR;
        }

        Int32 fTL;
        Int32 fTR;
        Int32 fBL;
        Int32 fBR;
    };
}

#endif // AKBORDERRADIUS_H
