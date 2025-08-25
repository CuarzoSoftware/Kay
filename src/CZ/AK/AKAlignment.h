#ifndef CZ_AKALIGNMENT_H
#define CZ_AKALIGNMENT_H

#include <CZ/AK/AK.h>

namespace CZ
{
    enum AKAlignment : UInt32
    {
        AKAlignTop          = static_cast<UInt32>(1) << 0,
        AKAlignBottom       = static_cast<UInt32>(1) << 1,
        AKAlignLeft         = static_cast<UInt32>(1) << 2,
        AKAlignRight        = static_cast<UInt32>(1) << 3,
        AKAlignTopLeft      = AKAlignTop | AKAlignLeft,
        AKAlignTopRight     = AKAlignTop | AKAlignRight,
        AKAlignBottomLeft   = AKAlignBottom | AKAlignLeft,
        AKAlignBottomRight  = AKAlignBottom | AKAlignRight,
        AKAlignCenter       = AKAlignTop | AKAlignBottom | AKAlignLeft | AKAlignRight
    };
};

#endif // CZ_AKALIGNMENT_H
