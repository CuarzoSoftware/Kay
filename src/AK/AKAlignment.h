#ifndef AKALIGNMENT_H
#define AKALIGNMENT_H

#include <AK/AK.h>

namespace AK
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

#endif // AKALIGNMENT_H
