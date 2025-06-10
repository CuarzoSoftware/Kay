#ifndef AKCOLORS_H
#define AKCOLORS_H

#include <skia/core/SkColor.h>

namespace AK
{
    namespace AKColor
    {
        static inline constexpr SkColor Gray            { 0xFF9E9E9E };
        static inline constexpr SkColor GrayLighten5    { 0xFFFAFAFA };
        static inline constexpr SkColor GrayLighten4    { 0xFFF5F5F5 };
        static inline constexpr SkColor GrayLighten3    { 0xFFEEEEEE };
        static inline constexpr SkColor GrayLighten2    { 0xFFE0E0E0 };
        static inline constexpr SkColor GrayLighten1    { 0xFFBDBDBD };
        static inline constexpr SkColor GrayDarken1     { 0xFF757575 };
        static inline constexpr SkColor GrayDarken2     { 0xFF616161 };
        static inline constexpr SkColor GrayDarken3     { 0xFF424242 };
        static inline constexpr SkColor GrayDarken4     { 0xFF212121 };
    }
}

#endif // AKCOLORS_H
