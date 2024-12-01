#ifndef AKBORDERRADIUS_H
#define AKBORDERRADIUS_H

#include <include/core/SkScalar.h>

namespace AK
{
    struct AKBorderRadius
    {
        static constexpr AKBorderRadius Make(SkScalar rad) noexcept
        {
            return AKBorderRadius{rad, rad, rad, rad};
        }

        static constexpr AKBorderRadius MakeTB(SkScalar top, SkScalar bottom) noexcept
        {
            return AKBorderRadius{top, top, bottom, bottom};
        }

        static constexpr AKBorderRadius MakeLR(SkScalar left, SkScalar right) noexcept
        {
            return AKBorderRadius{left, right, left, right};
        }

        SkScalar fTopLeft;
        SkScalar fTopRight;
        SkScalar fBottomLeft;
        SkScalar fBottomRight;
    };
}

#endif // AKBORDERRADIUS_H
