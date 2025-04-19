#ifndef AKRRECT_H
#define AKRRECT_H

#include <include/core/SkRect.h>
#include <AK/AK.h>

class AK::AKRRect : public SkIRect
{
public:
    AKRRect(const SkIRect &rect = SkIRect::MakeEmpty(), Int32 radTL = 0, Int32 radTR = 0, Int32 radBR = 0, Int32 radBL = 0) noexcept :
        SkIRect(rect), fRadTL(radTL), fRadTR(radTR), fRadBR(radBR), fRadBL(radBL)
    {}

    Int32 fRadTL { 0 };
    Int32 fRadTR { 0 };
    Int32 fRadBR { 0 };
    Int32 fRadBL { 0 };
};

#endif // AKRRECT_H
