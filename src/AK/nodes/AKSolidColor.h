#ifndef AKSOLIDCOLOR_H
#define AKSOLIDCOLOR_H

#include <AK/nodes/AKRenderable.h>
#include <AK/AKBitset.h>
#include <AK/AKBrush.h>
#include <AK/AKPen.h>

class AK::AKSolidColor : public AKRenderable
{
public:
    AKSolidColor(SkColor color, AKNode *parent = nullptr) noexcept :
        AKRenderable(SolidColor, parent)
    {
        setColorWithAlpha(color);
    }

protected:
    using AKRenderable::addDamage;
    using AKRenderable::opaqueRegion;
    void onRender(AKPainter *painter, const SkRegion &damage, const SkIRect &rect) override;
};

#endif // AKSOLIDCOLOR_H
