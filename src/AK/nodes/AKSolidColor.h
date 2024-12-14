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
        AKRenderable(parent)
    {
        const SkColor4f col4 { SkColor4f::FromColor(color) };
        setColor(col4);
        setOpacity(col4.fA);
    }

protected:
    using AKRenderable::addDamage;
    using AKRenderable::opaqueRegion;
    using AKRenderable::setColorHint;
    void onRender(AKPainter *painter, const SkRegion &damage) override;
    void onLayoutUpdate() override;
};

#endif // AKSOLIDCOLOR_H
