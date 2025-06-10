#ifndef AKTARGET_H
#define AKTARGET_H

#include <skia/core/SkSurface.h>
#include <skia/core/SkRect.h>
#include <AK/AKObject.h>
#include <AK/AKTransform.h>

class AK::AKTarget : public AKObject
{
public:
    AKTarget() = default;
    virtual const SkRect &viewport() const noexcept = 0;
    virtual AKTransform transform() const noexcept = 0;
    virtual const SkVector &xyScale() const noexcept = 0;
    virtual UInt32 fbId() const noexcept = 0;
    virtual sk_sp<SkSurface> surface() const noexcept = 0;
};

#endif // AKTARGET_H
